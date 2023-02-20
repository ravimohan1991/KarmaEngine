// Let me see how far I can push before removing ImGui's defines and smoothly mingle with Karma
// Dear ImGui (1.89.2) is Copyright (c) 2014-2023 Omar Cornut. This code is practically ImGui in Karma context!!

#include "KarmaGui.h"
#include "KarmaGuiInternal.h"

#ifndef KARMAGUI_DEFINE_MATH_OPERATORS
#define KARMAGUI_DEFINE_MATH_OPERATORS
#endif

// [Windows] On non-Visual Studio compilers, we default to KARMAGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS unless explicitly enabled
#if defined(KR_CORE_WINDOWS) && !defined(_MSC_VER) && !defined(KARMAGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS)
#define KARMAGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
#endif

// [Windows] OS specific includes (optional)
#if defined(KR_CORE_WINDOWS) && defined(KARMAGUI_DISABLE_DEFAULT_FILE_FUNCTIONS) && defined(IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS) && defined(KARMAGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#define IMGUI_DISABLE_WIN32_FUNCTIONS
#endif
#if defined(KR_CORE_WINDOWS) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef __MINGW32__
#include <Windows.h>        // _wfopen, OpenClipboard
#else
#include <windows.h>
#endif
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP) // UWP doesn't have all Win32 functions
#define IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
#define KARMAGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
#endif
#endif

// [Apple] OS specific includes
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

// Debug options
#define KARMAGUI_DEBUG_NAV_SCORING     0   // Display navigation scoring preview when hovering items. Display last moving direction matches when holding CTRL
#define KARMAGUI_DEBUG_NAV_RECTS       0   // Display the reference navigation rectangle for each window

#define KARMAGUI_DEBUG_INI_SETTINGS    0   // Save additional comments in .ini file (particularly helps for Docking, but makes saving slower)

KarmaGuiContext* GKarmaGui;

// When using CTRL+TAB (or Gamepad Square+L/R) we delay the visual a little in order to reduce visual noise doing a fast switch.
static const float NAV_WINDOWING_HIGHLIGHT_DELAY = 0.20f;    // Time before the highlight and screen dimming starts fading in
static const float NAV_WINDOWING_LIST_APPEAR_DELAY = 0.15f;    // Time before the window list starts to appear

// Window resizing from edges (when io.ConfigWindowsResizeFromEdges = true and KGGuiBackendFlags_HasMouseCursors is set in io.BackendFlags by backend)
static const float WINDOWS_HOVER_PADDING = 4.0f;     // Extend outside window for hovering/resizing (maxxed with TouchPadding) and inside windows for borders. Affect FindHoveredWindow().
static const float WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER = 0.04f;    // Reduce visual noise by only highlighting the border after a certain time.
static const float WINDOWS_MOUSE_WHEEL_SCROLL_LOCK_TIMER = 0.70f;    // Lock scrolled window (so it doesn't pick child windows that are scrolling through) for a certain time, unless mouse moved.

// Docking
static const float DOCKING_TRANSPARENT_PAYLOAD_ALPHA = 0.50f;    // For use with io.ConfigDockingTransparentPayload. Apply to Viewport _or_ WindowBg in host viewport.
static const float DOCKING_SPLITTER_SIZE = 2.0f;

// Allocators
static void* MallocWrapper(size_t size, void* user_data) { KG_UNUSED(user_data); return malloc(size); }
static void    FreeWrapper(void* ptr, void* user_data) { KG_UNUSED(user_data); free(ptr); }

namespace Karma
{
	KarmaGuiContext* GKarmaGui = nullptr;
	KarmaGuiMemAllocFunc    KarmaGuiInternal::GImAllocatorAllocFunc = MallocWrapper;
	KarmaGuiMemFreeFunc     KarmaGuiInternal::GImAllocatorFreeFunc = FreeWrapper;
	void* KarmaGuiInternal::GImAllocatorUserData = NULL;
}

//-----------------------------------------------------------------------------
// [SECTION] USER FACING STRUCTURES (KarmaGuiStyle, KarmaGuiIO)
//-----------------------------------------------------------------------------

KarmaGuiStyle::KarmaGuiStyle()
{
	Alpha = 1.0f;             // Global alpha applies to everything in Dear ImGui.
	DisabledAlpha = 0.60f;            // Additional alpha multiplier applied by BeginDisabled(). Multiply over current value of Alpha.
	WindowPadding = KGVec2(8, 8);      // Padding within a window
	WindowRounding = 0.0f;             // Radius of window corners rounding. Set to 0.0f to have rectangular windows. Large values tend to lead to variety of artifacts and are not recommended.
	WindowBorderSize = 1.0f;             // Thickness of border around windows. Generally set to 0.0f or 1.0f. Other values not well tested.
	WindowMinSize = KGVec2(32, 32);    // Minimum window size
	WindowTitleAlign = KGVec2(0.0f, 0.5f);// Alignment for title bar text
	WindowMenuButtonPosition = KGGuiDir_Left;    // Position of the collapsing/docking button in the title bar (left/right). Defaults to KGGuiDir_Left.
	ChildRounding = 0.0f;             // Radius of child window corners rounding. Set to 0.0f to have rectangular child windows
	ChildBorderSize = 1.0f;             // Thickness of border around child windows. Generally set to 0.0f or 1.0f. Other values not well tested.
	PopupRounding = 0.0f;             // Radius of popup window corners rounding. Set to 0.0f to have rectangular child windows
	PopupBorderSize = 1.0f;             // Thickness of border around popup or tooltip windows. Generally set to 0.0f or 1.0f. Other values not well tested.
	FramePadding = KGVec2(4, 3);      // Padding within a framed rectangle (used by most widgets)
	FrameRounding = 0.0f;             // Radius of frame corners rounding. Set to 0.0f to have rectangular frames (used by most widgets).
	FrameBorderSize = 0.0f;             // Thickness of border around frames. Generally set to 0.0f or 1.0f. Other values not well tested.
	ItemSpacing = KGVec2(8, 4);      // Horizontal and vertical spacing between widgets/lines
	ItemInnerSpacing = KGVec2(4, 4);      // Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label)
	CellPadding = KGVec2(4, 2);      // Padding within a table cell
	TouchExtraPadding = KGVec2(0, 0);      // Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
	IndentSpacing = 21.0f;            // Horizontal spacing when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
	ColumnsMinSpacing = 6.0f;             // Minimum horizontal spacing between two columns. Preferably > (FramePadding.x + 1).
	ScrollbarSize = 14.0f;            // Width of the vertical scrollbar, Height of the horizontal scrollbar
	ScrollbarRounding = 9.0f;             // Radius of grab corners rounding for scrollbar
	GrabMinSize = 12.0f;            // Minimum width/height of a grab box for slider/scrollbar
	GrabRounding = 0.0f;             // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
	LogSliderDeadzone = 4.0f;             // The size in pixels of the dead-zone around zero on logarithmic sliders that cross zero.
	TabRounding = 4.0f;             // Radius of upper corners of a tab. Set to 0.0f to have rectangular tabs.
	TabBorderSize = 0.0f;             // Thickness of border around tabs.
	TabMinWidthForCloseButton = 0.0f;           // Minimum width for close button to appear on an unselected tab when hovered. Set to 0.0f to always show when hovering, set to FLT_MAX to never show close button unless selected.
	ColorButtonPosition = KGGuiDir_Right;   // Side of the color button in the ColorEdit4 widget (left/right). Defaults to KGGuiDir_Right.
	ButtonTextAlign = KGVec2(0.5f, 0.5f);// Alignment of button text when button is larger than text.
	SelectableTextAlign = KGVec2(0.0f, 0.0f);// Alignment of selectable text. Defaults to (0.0f, 0.0f) (top-left aligned). It's generally important to keep this left-aligned if you want to lay multiple items on a same line.
	DisplayWindowPadding = KGVec2(19, 19);    // Window position are clamped to be visible within the display area or monitors by at least this amount. Only applies to regular windows.
	DisplaySafeAreaPadding = KGVec2(3, 3);      // If you cannot see the edge of your screen (e.g. on a TV) increase the safe area padding. Covers popups/tooltips as well regular windows.
	MouseCursorScale = 1.0f;             // Scale software rendered mouse cursor (when io.MouseDrawCursor is enabled). May be removed later.
	AntiAliasedLines = true;             // Enable anti-aliased lines/borders. Disable if you are really tight on CPU/GPU.
	AntiAliasedLinesUseTex = true;             // Enable anti-aliased lines/borders using textures where possible. Require backend to render with bilinear filtering (NOT point/nearest filtering).
	AntiAliasedFill = true;             // Enable anti-aliased filled shapes (rounded rectangles, circles, etc.).
	CurveTessellationTol = 1.25f;            // Tessellation tolerance when using PathBezierCurveTo() without a specific number of segments. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.
	CircleTessellationMaxError = 0.30f;         // Maximum error (in pixels) allowed when using AddCircle()/AddCircleFilled() or drawing rounded corner rectangles with no explicit segment count specified. Decrease for higher quality but more geometry.

	// Default theme
	Karma::KarmaGui::StyleColorsDark(this);
}

// To scale your entire UI (e.g. if you want your app to use High DPI or generally be DPI aware) you may use this helper function. Scaling the fonts is done separately and is up to you.
// Important: This operation is lossy because we round all sizes to integer. If you need to change your scale multiples, call this over a freshly initialized KarmaGuiStyle structure rather than scaling multiple times.
void KarmaGuiStyle::ScaleAllSizes(float scale_factor)
{
	WindowPadding = KGFloor(WindowPadding * scale_factor);
	WindowRounding = KGFloor(WindowRounding * scale_factor);
	WindowMinSize = KGFloor(WindowMinSize * scale_factor);
	ChildRounding = KGFloor(ChildRounding * scale_factor);
	PopupRounding = KGFloor(PopupRounding * scale_factor);
	FramePadding = KGFloor(FramePadding * scale_factor);
	FrameRounding = KGFloor(FrameRounding * scale_factor);
	ItemSpacing = KGFloor(ItemSpacing * scale_factor);
	ItemInnerSpacing = KGFloor(ItemInnerSpacing * scale_factor);
	CellPadding = KGFloor(CellPadding * scale_factor);
	TouchExtraPadding = KGFloor(TouchExtraPadding * scale_factor);
	IndentSpacing = KGFloor(IndentSpacing * scale_factor);
	ColumnsMinSpacing = KGFloor(ColumnsMinSpacing * scale_factor);
	ScrollbarSize = KGFloor(ScrollbarSize * scale_factor);
	ScrollbarRounding = KGFloor(ScrollbarRounding * scale_factor);
	GrabMinSize = KGFloor(GrabMinSize * scale_factor);
	GrabRounding = KGFloor(GrabRounding * scale_factor);
	LogSliderDeadzone = KGFloor(LogSliderDeadzone * scale_factor);
	TabRounding = KGFloor(TabRounding * scale_factor);
	TabMinWidthForCloseButton = (TabMinWidthForCloseButton != FLT_MAX) ? KGFloor(TabMinWidthForCloseButton * scale_factor) : FLT_MAX;
	DisplayWindowPadding = KGFloor(DisplayWindowPadding * scale_factor);
	DisplaySafeAreaPadding = KGFloor(DisplaySafeAreaPadding * scale_factor);
	MouseCursorScale = KGFloor(MouseCursorScale * scale_factor);
}

KarmaGuiIO::KarmaGuiIO()
{
	// Most fields are initialized with zero
	memset(this, 0, sizeof(*this));
	KG_STATIC_ASSERT(KG_ARRAYSIZE(KarmaGuiIO::MouseDown) == KGGuiMouseButton_COUNT && KG_ARRAYSIZE(KarmaGuiIO::MouseClicked) == KGGuiMouseButton_COUNT);

	// Settings
	ConfigFlags = KGGuiConfigFlags_None;
	BackendFlags = KGGuiBackendFlags_None;
	DisplaySize = KGVec2(-1.0f, -1.0f);
	DeltaTime = 1.0f / 60.0f;
	IniSavingRate = 5.0f;
	IniFilename = "kggui.ini"; // Important: "kggui.ini" is relative to current working dir, most apps will want to lock this to an absolute path (e.g. same path as executables).
	LogFilename = "imgui_log.txt";
	MouseDoubleClickTime = 0.30f;
	MouseDoubleClickMaxDist = 6.0f;
#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
	for (int i = 0; i < KGGuiKey_COUNT; i++)
		KeyMap[i] = -1;
#endif
	KeyRepeatDelay = 0.275f;
	KeyRepeatRate = 0.050f;
	HoverDelayNormal = 0.30f;
	HoverDelayShort = 0.10f;
	UserData = NULL;

	Fonts = NULL;
	FontGlobalScale = 1.0f;
	FontDefault = NULL;
	FontAllowUserScaling = false;
	DisplayFramebufferScale = KGVec2(1.0f, 1.0f);

	// Docking options (when KGGuiConfigFlags_DockingEnable is set)
	ConfigDockingNoSplit = false;
	ConfigDockingWithShift = false;
	ConfigDockingAlwaysTabBar = false;
	ConfigDockingTransparentPayload = false;

	// Viewport options (when KGGuiConfigFlags_ViewportsEnable is set)
	ConfigViewportsNoAutoMerge = false;
	ConfigViewportsNoTaskBarIcon = false;
	ConfigViewportsNoDecoration = true;
	ConfigViewportsNoDefaultParent = false;

	// Miscellaneous options
	MouseDrawCursor = false;
#ifdef __APPLE__
	ConfigMacOSXBehaviors = true;  // Set Mac OS X style defaults based on __APPLE__ compile time flag
#else
	ConfigMacOSXBehaviors = false;
#endif
	ConfigInputTrickleEventQueue = true;
	ConfigInputTextCursorBlink = true;
	ConfigInputTextEnterKeepActive = false;
	ConfigDragClickToInputText = false;
	ConfigWindowsResizeFromEdges = true;
	ConfigWindowsMoveFromTitleBarOnly = false;
	ConfigMemoryCompactTimer = 60.0f;

	// Platform Functions
	BackendPlatformName = BackendRendererName = NULL;
	BackendPlatformUserData = BackendRendererUserData = BackendLanguageUserData = NULL;
	GetClipboardTextFn = Karma::KarmaGuiInternal::GetClipboardTextFn_DefaultImpl;   // Platform dependent default implementations
	SetClipboardTextFn = Karma::KarmaGuiInternal::SetClipboardTextFn_DefaultImpl;
	ClipboardUserData = NULL;
	SetPlatformImeDataFn = Karma::KarmaGuiInternal::SetPlatformImeDataFn_DefaultImpl;

	// Input (NB: we already have memset zero the entire structure!)
	MousePos = KGVec2(-FLT_MAX, -FLT_MAX);
	MousePosPrev = KGVec2(-FLT_MAX, -FLT_MAX);
	MouseDragThreshold = 6.0f;
	for (int i = 0; i < KG_ARRAYSIZE(MouseDownDuration); i++) MouseDownDuration[i] = MouseDownDurationPrev[i] = -1.0f;
	for (int i = 0; i < KG_ARRAYSIZE(KeysData); i++) { KeysData[i].DownDuration = KeysData[i].DownDurationPrev = -1.0f; }
	AppAcceptingEvents = true;
	BackendUsingLegacyKeyArrays = (KGS8)-1;
	BackendUsingLegacyNavInputArray = true; // assume using legacy array until proven wrong
}

// Pass in translated ASCII characters for text input.
// - with glfw you can get those from the callback set in glfwSetCharCallback()
// - on Windows you can get those using ToAscii+keyboard state, or via the WM_CHAR message
// FIXME: Should in theory be called "AddCharacterEvent()" to be consistent with new API
void KarmaGuiIO::AddInputCharacter(unsigned int c)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KR_CORE_ASSERT(&g.IO == this, "Can only add events to current context.");
	if (c == 0 || !AppAcceptingEvents)
		return;

	KGGuiInputEvent e;
	e.Type = KGGuiInputEventType_Text;
	e.Source = KGGuiInputSource_Keyboard;
	e.Text.Char = c;
	g.InputEventsQueue.push_back(e);
}

// UTF16 strings use surrogate pairs to encode codepoints >= 0x10000, so
// we should save the high surrogate.
void KarmaGuiIO::AddInputCharacterUTF16(KGWchar16 c)
{
	if ((c == 0 && InputQueueSurrogate == 0) || !AppAcceptingEvents)
		return;

	if ((c & 0xFC00) == 0xD800) // High surrogate, must save
	{
		if (InputQueueSurrogate != 0)
			AddInputCharacter(KG_UNICODE_CODEPOINT_INVALID);
		InputQueueSurrogate = c;
		return;
	}

	KGWchar cp = c;
	if (InputQueueSurrogate != 0)
	{
		if ((c & 0xFC00) != 0xDC00) // Invalid low surrogate
		{
			AddInputCharacter(KG_UNICODE_CODEPOINT_INVALID);
		}
		else
		{
#if KG_UNICODE_CODEPOINT_MAX == 0xFFFF
			cp = KG_UNICODE_CODEPOINT_INVALID; // Codepoint will not fit in KGWchar
#else
			cp = (KGWchar)(((InputQueueSurrogate - 0xD800) << 10) + (c - 0xDC00) + 0x10000);
#endif
		}

		InputQueueSurrogate = 0;
	}
	AddInputCharacter((unsigned)cp);
}

void KarmaGuiIO::AddInputCharactersUTF8(const char* utf8_chars)
{
	if (!AppAcceptingEvents)
		return;
	while (*utf8_chars != 0)
	{
		unsigned int c = 0;
		utf8_chars += KGTextCharFromUtf8(&c, utf8_chars, NULL);
		if (c != 0)
			AddInputCharacter(c);
	}
}

// FIXME: Perhaps we could clear queued events as well?
void KarmaGuiIO::ClearInputCharacters()
{
	InputQueueCharacters.resize(0);
}

// FIXME: Perhaps we could clear queued events as well?
void KarmaGuiIO::ClearInputKeys()
{
#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
	memset(KeysDown, 0, sizeof(KeysDown));
#endif
	for (int n = 0; n < KG_ARRAYSIZE(KeysData); n++)
	{
		KeysData[n].Down = false;
		KeysData[n].DownDuration = -1.0f;
		KeysData[n].DownDurationPrev = -1.0f;
	}
	KeyCtrl = KeyShift = KeyAlt = KeySuper = false;
	KeyMods = KGGuiMod_None;
	MousePos = KGVec2(-FLT_MAX, -FLT_MAX);
	for (int n = 0; n < KG_ARRAYSIZE(MouseDown); n++)
	{
		MouseDown[n] = false;
		MouseDownDuration[n] = MouseDownDurationPrev[n] = -1.0f;
	}
	MouseWheel = MouseWheelH = 0.0f;
}

static KGGuiInputEvent* FindLatestInputEvent(KGGuiInputEventType type, int arg = -1)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	for (int n = g.InputEventsQueue.Size - 1; n >= 0; n--)
	{
		KGGuiInputEvent* e = &g.InputEventsQueue[n];
		if (e->Type != type)
			continue;
		if (type == KGGuiInputEventType_Key && e->Key.Key != arg)
			continue;
		if (type == KGGuiInputEventType_MouseButton && e->MouseButton.Button != arg)
			continue;
		return e;
	}
	return NULL;
}

// Queue a new key down/up event.
// - KarmaGuiKey key:       Translated key (as in, generally KGGuiKey_A matches the key end-user would use to emit an 'A' character)
// - bool down:          Is the key down? use false to signify a key release.
// - float analog_value: 0.0f..1.0f
void KarmaGuiIO::AddKeyAnalogEvent(KarmaGuiKey key, bool down, float analog_value)
{
	//if (e->Down) { KARMAGUI_DEBUG_LOG_IO("AddKeyEvent() Key='%s' %d, NativeKeycode = %d, NativeScancode = %d\n", KarmaGui::GetKeyName(e->Key), e->Down, e->NativeKeycode, e->NativeScancode); }
	if (key == KGGuiKey_None || !AppAcceptingEvents)
		return;
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KR_CORE_ASSERT(&g.IO == this, "Can only add events to current context.");
	KR_CORE_ASSERT(Karma::KarmaGuiInternal::IsNamedKeyOrModKey(key), ""); // Backend needs to pass a valid KGGuiKey_ constant. 0..511 values are legacy native key codes which are not accepted by this API.
	KR_CORE_ASSERT(!Karma::KarmaGuiInternal::IsAliasKey(key), ""); // Backend cannot submit KGGuiKey_MouseXXX values they are automatically inferred from AddMouseXXX() events.
	KR_CORE_ASSERT(key != KGGuiMod_Shortcut, ""); // We could easily support the translation here but it seems saner to not accept it (TestEngine perform a translation itself)

	// Verify that backend isn't mixing up using new io.AddKeyEvent() api and old io.KeysDown[] + io.KeyMap[] data.
#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
	KR_CORE_ASSERT((BackendUsingLegacyKeyArrays == -1 || BackendUsingLegacyKeyArrays == 0), "Backend needs to either only use io.AddKeyEvent(), either only fill legacy io.KeysDown[] + io.KeyMap[]. Not both!");
	if (BackendUsingLegacyKeyArrays == -1)
		for (int n = KGGuiKey_NamedKey_BEGIN; n < KGGuiKey_NamedKey_END; n++)
		{
			KR_CORE_ASSERT(KeyMap[n] == -1, "Backend needs to either only use io.AddKeyEvent(), either only fill legacy io.KeysDown[] + io.KeyMap[]. Not both!");
		}
	BackendUsingLegacyKeyArrays = 0;
#endif
	if (Karma::KarmaGuiInternal::IsGamepadKey(key))
		BackendUsingLegacyNavInputArray = false;

	// Filter duplicate (in particular: key mods and gamepad analog values are commonly spammed)
	const KGGuiInputEvent* latest_event = FindLatestInputEvent(KGGuiInputEventType_Key, (int)key);
	const KarmaGuiKeyData* key_data = Karma::KarmaGuiInternal::GetKeyData(key);
	const bool latest_key_down = latest_event ? latest_event->Key.Down : key_data->Down;
	const float latest_key_analog = latest_event ? latest_event->Key.AnalogValue : key_data->AnalogValue;
	if (latest_key_down == down && latest_key_analog == analog_value)
		return;

	// Add event
	KGGuiInputEvent e;
	e.Type = KGGuiInputEventType_Key;
	e.Source = Karma::KarmaGuiInternal::IsGamepadKey(key) ? KGGuiInputSource_Gamepad : KGGuiInputSource_Keyboard;
	e.Key.Key = key;
	e.Key.Down = down;
	e.Key.AnalogValue = analog_value;
	g.InputEventsQueue.push_back(e);
}

void KarmaGuiIO::AddKeyEvent(KarmaGuiKey key, bool down)
{
	if (!AppAcceptingEvents)
		return;
	AddKeyAnalogEvent(key, down, down ? 1.0f : 0.0f);
}

// [Optional] Call after AddKeyEvent().
// Specify native keycode, scancode + Specify index for legacy <1.87 IsKeyXXX() functions with native indices.
// If you are writing a backend in 2022 or don't use IsKeyXXX() with native values that are not KarmaGuiKey values, you can avoid calling this.
void KarmaGuiIO::SetKeyEventNativeData(KarmaGuiKey key, int native_keycode, int native_scancode, int native_legacy_index)
{
	if (key == KGGuiKey_None)
		return;
	KR_CORE_ASSERT(Karma::KarmaGuiInternal::IsNamedKey(key), ""); // >= 512
	KR_CORE_ASSERT(native_legacy_index == -1 || Karma::KarmaGuiInternal::IsLegacyKey((KarmaGuiKey)native_legacy_index), ""); // >= 0 && <= 511
	KG_UNUSED(native_keycode);  // Yet unused
	KG_UNUSED(native_scancode); // Yet unused

	// Build native->imgui map so old user code can still call key functions with native 0..511 values.
#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
	const int legacy_key = (native_legacy_index != -1) ? native_legacy_index : native_keycode;
	if (!Karma::KarmaGuiInternal::IsLegacyKey((KarmaGuiKey)legacy_key))
		return;
	KeyMap[legacy_key] = key;
	KeyMap[key] = legacy_key;
#else
	KG_UNUSED(key);
	KG_UNUSED(native_legacy_index);
#endif
}

// Set master flag for accepting key/mouse/text events (default to true). Useful if you have native dialog boxes that are interrupting your application loop/refresh, and you want to disable events being queued while your app is frozen.
void KarmaGuiIO::SetAppAcceptingEvents(bool accepting_events)
{
	AppAcceptingEvents = accepting_events;
}

// Queue a mouse move event
void KarmaGuiIO::AddMousePosEvent(float x, float y)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KR_CORE_ASSERT(&g.IO == this, "Can only add events to current context.");
	if (!AppAcceptingEvents)
		return;

	// Apply same flooring as UpdateMouseInputs()
	KGVec2 pos((x > -FLT_MAX) ? KGFloorSigned(x) : x, (y > -FLT_MAX) ? KGFloorSigned(y) : y);

	// Filter duplicate
	const KGGuiInputEvent* latest_event = FindLatestInputEvent(KGGuiInputEventType_MousePos);
	const KGVec2 latest_pos = latest_event ? KGVec2(latest_event->MousePos.PosX, latest_event->MousePos.PosY) : g.IO.MousePos;
	if (latest_pos.x == pos.x && latest_pos.y == pos.y)
		return;

	KGGuiInputEvent e;
	e.Type = KGGuiInputEventType_MousePos;
	e.Source = KGGuiInputSource_Mouse;
	e.MousePos.PosX = pos.x;
	e.MousePos.PosY = pos.y;
	g.InputEventsQueue.push_back(e);
}

void KarmaGuiIO::AddMouseButtonEvent(int mouse_button, bool down)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KR_CORE_ASSERT(&g.IO == this, "Can only add events to current context.");
	KR_CORE_ASSERT(mouse_button >= 0 && mouse_button < KGGuiMouseButton_COUNT, "");
	if (!AppAcceptingEvents)
		return;

	// Filter duplicate
	const KGGuiInputEvent* latest_event = FindLatestInputEvent(KGGuiInputEventType_MouseButton, (int)mouse_button);
	const bool latest_button_down = latest_event ? latest_event->MouseButton.Down : g.IO.MouseDown[mouse_button];
	if (latest_button_down == down)
		return;

	KGGuiInputEvent e;
	e.Type = KGGuiInputEventType_MouseButton;
	e.Source = KGGuiInputSource_Mouse;
	e.MouseButton.Button = mouse_button;
	e.MouseButton.Down = down;
	g.InputEventsQueue.push_back(e);
}

// Queue a mouse wheel event (most mouse/API will only have a Y component)
void KarmaGuiIO::AddMouseWheelEvent(float wheel_x, float wheel_y)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KR_CORE_ASSERT(&g.IO == this, "Can only add events to current context.");

	// Filter duplicate (unlike most events, wheel values are relative and easy to filter)
	if (!AppAcceptingEvents || (wheel_x == 0.0f && wheel_y == 0.0f))
		return;

	KGGuiInputEvent e;
	e.Type = KGGuiInputEventType_MouseWheel;
	e.Source = KGGuiInputSource_Mouse;
	e.MouseWheel.WheelX = wheel_x;
	e.MouseWheel.WheelY = wheel_y;
	g.InputEventsQueue.push_back(e);
}

void KarmaGuiIO::AddMouseViewportEvent(KGGuiID viewport_id)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KR_CORE_ASSERT(&g.IO == this, "Can only add events to current context.");
	KR_CORE_ASSERT(g.IO.BackendFlags & KGGuiBackendFlags_HasMouseHoveredViewport, "");
	if (!AppAcceptingEvents)
		return;

	// Filter duplicate
	const KGGuiInputEvent* latest_event = FindLatestInputEvent(KGGuiInputEventType_MouseViewport);
	const KGGuiID latest_viewport_id = latest_event ? latest_event->MouseViewport.HoveredViewportID : g.IO.MouseHoveredViewport;
	if (latest_viewport_id == viewport_id)
		return;

	KGGuiInputEvent e;
	e.Type = KGGuiInputEventType_MouseViewport;
	e.Source = KGGuiInputSource_Mouse;
	e.MouseViewport.HoveredViewportID = viewport_id;
	g.InputEventsQueue.push_back(e);
}

void KarmaGuiIO::AddFocusEvent(bool focused)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KR_CORE_ASSERT(&g.IO == this, "Can only add events to current context.");

	// Filter duplicate
	const KGGuiInputEvent* latest_event = FindLatestInputEvent(KGGuiInputEventType_Focus);
	const bool latest_focused = latest_event ? latest_event->AppFocused.Focused : !g.IO.AppFocusLost;
	if (latest_focused == focused)
		return;

	KGGuiInputEvent e;
	e.Type = KGGuiInputEventType_Focus;
	e.AppFocused.Focused = focused;
	g.InputEventsQueue.push_back(e);
}

//-----------------------------------------------------------------------------
// [SECTION] MISC HELPERS/UTILITIES (Geometry functions)
//-----------------------------------------------------------------------------

KGVec2 KGBezierCubicClosestPoint(const KGVec2& p1, const KGVec2& p2, const KGVec2& p3, const KGVec2& p4, const KGVec2& p, int num_segments)
{
	KR_CORE_ASSERT(num_segments > 0, ""); // Use KGBezierCubicClosestPointCasteljau()
	KGVec2 p_last = p1;
	KGVec2 p_closest;
	float p_closest_dist2 = FLT_MAX;
	float t_step = 1.0f / (float)num_segments;
	for (int i_step = 1; i_step <= num_segments; i_step++)
	{
		KGVec2 p_current = KGBezierCubicCalc(p1, p2, p3, p4, t_step * i_step);
		KGVec2 p_line = KGLineClosestPoint(p_last, p_current, p);
		float dist2 = KGLengthSqr(p - p_line);
		if (dist2 < p_closest_dist2)
		{
			p_closest = p_line;
			p_closest_dist2 = dist2;
		}
		p_last = p_current;
	}
	return p_closest;
}

// Closely mimics PathBezierToCasteljau() in imgui_draw.cpp
static void ImBezierCubicClosestPointCasteljauStep(const KGVec2& p, KGVec2& p_closest, KGVec2& p_last, float& p_closest_dist2, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tess_tol, int level)
{
	float dx = x4 - x1;
	float dy = y4 - y1;
	float d2 = ((x2 - x4) * dy - (y2 - y4) * dx);
	float d3 = ((x3 - x4) * dy - (y3 - y4) * dx);
	d2 = (d2 >= 0) ? d2 : -d2;
	d3 = (d3 >= 0) ? d3 : -d3;
	if ((d2 + d3) * (d2 + d3) < tess_tol * (dx * dx + dy * dy))
	{
		KGVec2 p_current(x4, y4);
		KGVec2 p_line = KGLineClosestPoint(p_last, p_current, p);
		float dist2 = KGLengthSqr(p - p_line);
		if (dist2 < p_closest_dist2)
		{
			p_closest = p_line;
			p_closest_dist2 = dist2;
		}
		p_last = p_current;
	}
	else if (level < 10)
	{
		float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
		float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
		float x34 = (x3 + x4) * 0.5f, y34 = (y3 + y4) * 0.5f;
		float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
		float x234 = (x23 + x34) * 0.5f, y234 = (y23 + y34) * 0.5f;
		float x1234 = (x123 + x234) * 0.5f, y1234 = (y123 + y234) * 0.5f;
		ImBezierCubicClosestPointCasteljauStep(p, p_closest, p_last, p_closest_dist2, x1, y1, x12, y12, x123, y123, x1234, y1234, tess_tol, level + 1);
		ImBezierCubicClosestPointCasteljauStep(p, p_closest, p_last, p_closest_dist2, x1234, y1234, x234, y234, x34, y34, x4, y4, tess_tol, level + 1);
	}
}

// tess_tol is generally the same value you would find in KarmaGui::GetStyle().CurveTessellationTol
// Because those ImXXX functions are lower-level than KarmaGui:: we cannot access this value automatically.
KGVec2 KGBezierCubicClosestPointCasteljau(const KGVec2& p1, const KGVec2& p2, const KGVec2& p3, const KGVec2& p4, const KGVec2& p, float tess_tol)
{
	KR_CORE_ASSERT(tess_tol > 0.0f, "");
	KGVec2 p_last = p1;
	KGVec2 p_closest;
	float p_closest_dist2 = FLT_MAX;
	ImBezierCubicClosestPointCasteljauStep(p, p_closest, p_last, p_closest_dist2, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, tess_tol, 0);
	return p_closest;
}

KGVec2 KGLineClosestPoint(const KGVec2& a, const KGVec2& b, const KGVec2& p)
{
	KGVec2 ap = p - a;
	KGVec2 ab_dir = b - a;
	float dot = ap.x * ab_dir.x + ap.y * ab_dir.y;
	if (dot < 0.0f)
		return a;
	float ab_len_sqr = ab_dir.x * ab_dir.x + ab_dir.y * ab_dir.y;
	if (dot > ab_len_sqr)
		return b;
	return a + ab_dir * dot / ab_len_sqr;
}

bool KGTriangleContainsPoint(const KGVec2& a, const KGVec2& b, const KGVec2& c, const KGVec2& p)
{
	bool b1 = ((p.x - b.x) * (a.y - b.y) - (p.y - b.y) * (a.x - b.x)) < 0.0f;
	bool b2 = ((p.x - c.x) * (b.y - c.y) - (p.y - c.y) * (b.x - c.x)) < 0.0f;
	bool b3 = ((p.x - a.x) * (c.y - a.y) - (p.y - a.y) * (c.x - a.x)) < 0.0f;
	return ((b1 == b2) && (b2 == b3));
}

void KGTriangleBarycentricCoords(const KGVec2& a, const KGVec2& b, const KGVec2& c, const KGVec2& p, float& out_u, float& out_v, float& out_w)
{
	KGVec2 v0 = b - a;
	KGVec2 v1 = c - a;
	KGVec2 v2 = p - a;
	const float denom = v0.x * v1.y - v1.x * v0.y;
	out_v = (v2.x * v1.y - v1.x * v2.y) / denom;
	out_w = (v0.x * v2.y - v2.x * v0.y) / denom;
	out_u = 1.0f - out_v - out_w;
}

KGVec2 KGTriangleClosestPoint(const KGVec2& a, const KGVec2& b, const KGVec2& c, const KGVec2& p)
{
	KGVec2 proj_ab = KGLineClosestPoint(a, b, p);
	KGVec2 proj_bc = KGLineClosestPoint(b, c, p);
	KGVec2 proj_ca = KGLineClosestPoint(c, a, p);
	float dist2_ab = KGLengthSqr(p - proj_ab);
	float dist2_bc = KGLengthSqr(p - proj_bc);
	float dist2_ca = KGLengthSqr(p - proj_ca);
	float m = KGMin(dist2_ab, KGMin(dist2_bc, dist2_ca));
	if (m == dist2_ab)
		return proj_ab;
	if (m == dist2_bc)
		return proj_bc;
	return proj_ca;
}

//-----------------------------------------------------------------------------
// [SECTION] MISC HELPERS/UTILITIES (String, Format, Hash functions)
//-----------------------------------------------------------------------------

// Consider using _stricmp/_strnicmp under Windows or strcasecmp/strncasecmp. We don't actually use either KGStricmp/KGStrnicmp in the codebase any more.
int KGStricmp(const char* str1, const char* str2)
{
	int d;
	while ((d = KGToUpper(*str2) - KGToUpper(*str1)) == 0 && *str1) { str1++; str2++; }
	return d;
}

int KGStrnicmp(const char* str1, const char* str2, size_t count)
{
	int d = 0;
	while (count > 0 && (d = KGToUpper(*str2) - KGToUpper(*str1)) == 0 && *str1) { str1++; str2++; count--; }
	return d;
}

void KGStrncpy(char* dst, const char* src, size_t count)
{
	if (count < 1)
		return;
	if (count > 1)
		strncpy(dst, src, count - 1);
	dst[count - 1] = 0;
}

char* KGStrdup(const char* str)
{
	size_t len = strlen(str);
	void* buf = KG_ALLOC(len + 1);
	return (char*)memcpy(buf, (const void*)str, len + 1);
}

char* KGStrdupcpy(char* dst, size_t* p_dst_size, const char* src)
{
	size_t dst_buf_size = p_dst_size ? *p_dst_size : strlen(dst) + 1;
	size_t src_size = strlen(src) + 1;
	if (dst_buf_size < src_size)
	{
		KG_FREE(dst);
		dst = (char*)KG_ALLOC(src_size);
		if (p_dst_size)
			*p_dst_size = src_size;
	}
	return (char*)memcpy(dst, (const void*)src, src_size);
}

const char* KGStrchrRange(const char* str, const char* str_end, char c)
{
	const char* p = (const char*)memchr(str, (int)c, str_end - str);
	return p;
}

int KGStrlenW(const KGWchar* str)
{
	//return (int)wcslen((const wchar_t*)str);  // FIXME-OPT: Could use this when wchar_t are 16-bit
	int n = 0;
	while (*str++) n++;
	return n;
}

// Find end-of-line. Return pointer will point to either first \n, either str_end.
const char* KGStreolRange(const char* str, const char* str_end)
{
	const char* p = (const char*)memchr(str, '\n', str_end - str);
	return p ? p : str_end;
}

const KGWchar* KGStrbolW(const KGWchar* buf_mid_line, const KGWchar* buf_begin) // find beginning-of-line
{
	while (buf_mid_line > buf_begin && buf_mid_line[-1] != '\n')
		buf_mid_line--;
	return buf_mid_line;
}

const char* KGStristr(const char* haystack, const char* haystack_end, const char* needle, const char* needle_end)
{
	if (!needle_end)
		needle_end = needle + strlen(needle);

	const char un0 = (char)KGToUpper(*needle);
	while ((!haystack_end && *haystack) || (haystack_end && haystack < haystack_end))
	{
		if (KGToUpper(*haystack) == un0)
		{
			const char* b = needle + 1;
			for (const char* a = haystack + 1; b < needle_end; a++, b++)
				if (KGToUpper(*a) != KGToUpper(*b))
					break;
			if (b == needle_end)
				return haystack;
		}
		haystack++;
	}
	return NULL;
}

// Trim str by offsetting contents when there's leading data + writing a \0 at the trailing position. We use this in situation where the cost is negligible.
void KGStrTrimBlanks(char* buf)
{
	char* p = buf;
	while (p[0] == ' ' || p[0] == '\t')     // Leading blanks
		p++;
	char* p_start = p;
	while (*p != 0)                         // Find end of string
		p++;
	while (p > p_start && (p[-1] == ' ' || p[-1] == '\t'))  // Trailing blanks
		p--;
	if (p_start != buf)                     // Copy memory if we had leading blanks
		memmove(buf, p_start, p - p_start);
	buf[p - p_start] = 0;                   // Zero terminate
}

const char* KGStrSkipBlank(const char* str)
{
	while (str[0] == ' ' || str[0] == '\t')
		str++;
	return str;
}

// A) MSVC version appears to return -1 on overflow, whereas glibc appears to return total count (which may be >= buf_size).
// Ideally we would test for only one of those limits at runtime depending on the behavior the vsnprintf(), but trying to deduct it at compile time sounds like a pandora can of worm.
// B) When buf==NULL vsnprintf() will return the output size.
#ifndef IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS

// We support stb_sprintf which is much faster (see: https://github.com/nothings/stb/blob/master/stb_sprintf.h)
// You may set IMGUI_USE_STB_SPRINTF to use our default wrapper, or set IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
// and setup the wrapper yourself. (FIXME-OPT: Some of our high-level operations such as KarmaGuiTextBuffer::appendfv() are
// designed using two-passes worst case, which probably could be improved using the stbsp_vsprintfcb() function.)
#ifdef IMGUI_USE_STB_SPRINTF
#define STB_SPRINTF_IMPLEMENTATION
#ifdef IMGUI_STB_SPRINTF_FILENAME
#include IMGUI_STB_SPRINTF_FILENAME
#else
#include "stb_sprintf.h"
#endif
#endif

#if defined(_MSC_VER) && !defined(vsnprintf)
#define vsnprintf _vsnprintf
#endif

int KGFormatString(char* buf, size_t buf_size, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
#ifdef IMGUI_USE_STB_SPRINTF
	int w = stbsp_vsnprintf(buf, (int)buf_size, fmt, args);
#else
	int w = vsnprintf(buf, buf_size, fmt, args);
#endif
	va_end(args);
	if (buf == NULL)
		return w;
	if (w == -1 || w >= (int)buf_size)
		w = (int)buf_size - 1;
	buf[w] = 0;
	return w;
}

int KGFormatStringV(char* buf, size_t buf_size, const char* fmt, va_list args)
{
#ifdef IMGUI_USE_STB_SPRINTF
	int w = stbsp_vsnprintf(buf, (int)buf_size, fmt, args);
#else
	int w = vsnprintf(buf, buf_size, fmt, args);
#endif
	if (buf == NULL)
		return w;
	if (w == -1 || w >= (int)buf_size)
		w = (int)buf_size - 1;
	buf[w] = 0;
	return w;
}
#endif // #ifdef IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS

void KGFormatStringToTempBuffer(const char** out_buf, const char** out_buf_end, const char* fmt, ...)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	va_list args;
	va_start(args, fmt);
	int buf_len = KGFormatStringV(g.TempBuffer.Data, g.TempBuffer.Size, fmt, args);
	*out_buf = g.TempBuffer.Data;
	if (out_buf_end) { *out_buf_end = g.TempBuffer.Data + buf_len; }
	va_end(args);
}

void KGFormatStringToTempBufferV(const char** out_buf, const char** out_buf_end, const char* fmt, va_list args)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	int buf_len = KGFormatStringV(g.TempBuffer.Data, g.TempBuffer.Size, fmt, args);
	*out_buf = g.TempBuffer.Data;
	if (out_buf_end) { *out_buf_end = g.TempBuffer.Data + buf_len; }
}

// CRC32 needs a 1KB lookup table (not cache friendly)
// Although the code to generate the table is simple and shorter than the table itself, using a const table allows us to easily:
// - avoid an unnecessary branch/memory tap, - keep the ImHashXXX functions usable by static constructors, - make it thread-safe.
static const KGU32 GCrc32LookupTable[256] =
{
	0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,
	0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
	0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
	0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,
	0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,
	0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
	0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
	0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,
	0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
	0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
	0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,
	0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
	0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
	0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
	0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
	0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D,
};

// Known size hash
// It is ok to call KGHashData on a string with known length but the ### operator won't be supported.
// FIXME-OPT: Replace with e.g. FNV1a hash? CRC32 pretty much randomly access 1KB. Need to do proper measurements.
KGGuiID KGHashData(const void* data_p, size_t data_size, KGU32 seed)
{
	KGU32 crc = ~seed;
	const unsigned char* data = (const unsigned char*)data_p;
	const KGU32* crc32_lut = GCrc32LookupTable;
	while (data_size-- != 0)
		crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ *data++];
	return ~crc;
}

// Zero-terminated string hash, with support for ### to reset back to seed value
// We support a syntax of "label###id" where only "###id" is included in the hash, and only "label" gets displayed.
// Because this syntax is rarely used we are optimizing for the common case.
// - If we reach ### in the string we discard the hash so far and reset to the seed.
// - We don't do 'current += 2; continue;' after handling ### to keep the code smaller/faster (measured ~10% diff in Debug build)
// FIXME-OPT: Replace with e.g. FNV1a hash? CRC32 pretty much randomly access 1KB. Need to do proper measurements.
KGGuiID KGHashStr(const char* data_p, size_t data_size, KGU32 seed)
{
	seed = ~seed;
	KGU32 crc = seed;
	const unsigned char* data = (const unsigned char*)data_p;
	const KGU32* crc32_lut = GCrc32LookupTable;
	if (data_size != 0)
	{
		while (data_size-- != 0)
		{
			unsigned char c = *data++;
			if (c == '#' && data_size >= 2 && data[0] == '#' && data[1] == '#')
				crc = seed;
			crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ c];
		}
	}
	else
	{
		while (unsigned char c = *data++)
		{
			if (c == '#' && data[0] == '#' && data[1] == '#')
				crc = seed;
			crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ c];
		}
	}
	return ~crc;
}

//-----------------------------------------------------------------------------
// [SECTION] MISC HELPERS/UTILITIES (File functions)
//-----------------------------------------------------------------------------

// Default file functions
#ifndef KARMAGUI_DISABLE_DEFAULT_FILE_FUNCTIONS

ImFileHandle KGFileOpen(const char* filename, const char* mode)
{
#if defined(KR_CORE_WINDOWS) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS) && !defined(__CYGWIN__) && !defined(__GNUC__)
	// We need a fopen() wrapper because MSVC/Windows fopen doesn't handle UTF-8 filenames.
	// Previously we used KGTextCountCharsFromUtf8/KGTextStrFromUtf8 here but we now need to support KGWchar16 and KGWchar32!
	const int filename_wsize = ::MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
	const int mode_wsize = ::MultiByteToWideChar(CP_UTF8, 0, mode, -1, NULL, 0);
	KGVector<KGWchar> buf;
	buf.resize(filename_wsize + mode_wsize);
	::MultiByteToWideChar(CP_UTF8, 0, filename, -1, (wchar_t*)&buf[0], filename_wsize);
	::MultiByteToWideChar(CP_UTF8, 0, mode, -1, (wchar_t*)&buf[filename_wsize], mode_wsize);
	return ::_wfopen((const wchar_t*)&buf[0], (const wchar_t*)&buf[filename_wsize]);
#else
	return fopen(filename, mode);
#endif
}

// We should in theory be using fseeko()/ftello() with off_t and _fseeki64()/_ftelli64() with __int64, waiting for the PR that does that in a very portable pre-C++11 zero-warnings way.
bool    KGFileClose(ImFileHandle f) { return fclose(f) == 0; }
KGU64   KGFileGetSize(ImFileHandle f) { long off = 0, sz = 0; return ((off = ftell(f)) != -1 && !fseek(f, 0, SEEK_END) && (sz = ftell(f)) != -1 && !fseek(f, off, SEEK_SET)) ? (KGU64)sz : (KGU64)-1; }
KGU64   KGFileRead(void* data, KGU64 sz, KGU64 count, ImFileHandle f) { return fread(data, (size_t)sz, (size_t)count, f); }
KGU64   KGFileWrite(const void* data, KGU64 sz, KGU64 count, ImFileHandle f) { return fwrite(data, (size_t)sz, (size_t)count, f); }
#endif // #ifndef KARMAGUI_DISABLE_DEFAULT_FILE_FUNCTIONS

// Helper: Load file content into memory
// Memory allocated with KG_ALLOC(), must be freed by user using KG_FREE() == KarmaGui::MemFree()
// This can't really be used with "rt" because fseek size won't match read size.
void* KGFileLoadToMemory(const char* filename, const char* mode, size_t* out_file_size, int padding_bytes)
{
	KR_CORE_ASSERT(filename && mode, "");
	if (out_file_size)
		*out_file_size = 0;

	ImFileHandle f;
	if ((f = KGFileOpen(filename, mode)) == NULL)
		return NULL;

	size_t file_size = (size_t)KGFileGetSize(f);
	if (file_size == (size_t)-1)
	{
		KGFileClose(f);
		return NULL;
	}

	void* file_data = KG_ALLOC(file_size + padding_bytes);
	if (file_data == NULL)
	{
		KGFileClose(f);
		return NULL;
	}
	if (KGFileRead(file_data, 1, file_size, f) != file_size)
	{
		KGFileClose(f);
		KG_FREE(file_data);
		return NULL;
	}
	if (padding_bytes > 0)
		memset((void*)(((char*)file_data) + file_size), 0, (size_t)padding_bytes);

	KGFileClose(f);
	if (out_file_size)
		*out_file_size = file_size;

	return file_data;
}

//-----------------------------------------------------------------------------
// [SECTION] MISC HELPERS/UTILITIES (ImText* functions)
//-----------------------------------------------------------------------------

// Convert UTF-8 to 32-bit character, process single character input.
// A nearly-branchless UTF-8 decoder, based on work of Christopher Wellons (https://github.com/skeeto/branchless-utf8).
// We handle UTF-8 decoding error by skipping forward.
int KGTextCharFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end)
{
	static const char lengths[32] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0 };
	static const int masks[] = { 0x00, 0x7f, 0x1f, 0x0f, 0x07 };
	static const uint32_t mins[] = { 0x400000, 0, 0x80, 0x800, 0x10000 };
	static const int shiftc[] = { 0, 18, 12, 6, 0 };
	static const int shifte[] = { 0, 6, 4, 2, 0 };
	int len = lengths[*(const unsigned char*)in_text >> 3];
	int wanted = len + !len;

	if (in_text_end == NULL)
		in_text_end = in_text + wanted; // Max length, nulls will be taken into account.

	// Copy at most 'len' bytes, stop copying at 0 or past in_text_end. Branch predictor does a good job here,
	// so it is fast even with excessive branching.
	unsigned char s[4];
	s[0] = in_text + 0 < in_text_end ? in_text[0] : 0;
	s[1] = in_text + 1 < in_text_end ? in_text[1] : 0;
	s[2] = in_text + 2 < in_text_end ? in_text[2] : 0;
	s[3] = in_text + 3 < in_text_end ? in_text[3] : 0;

	// Assume a four-byte character and load four bytes. Unused bits are shifted out.
	*out_char = (uint32_t)(s[0] & masks[len]) << 18;
	*out_char |= (uint32_t)(s[1] & 0x3f) << 12;
	*out_char |= (uint32_t)(s[2] & 0x3f) << 6;
	*out_char |= (uint32_t)(s[3] & 0x3f) << 0;
	*out_char >>= shiftc[len];

	// Accumulate the various error conditions.
	int e = 0;
	e = (*out_char < mins[len]) << 6; // non-canonical encoding
	e |= ((*out_char >> 11) == 0x1b) << 7;  // surrogate half?
	e |= (*out_char > KG_UNICODE_CODEPOINT_MAX) << 8;  // out of range?
	e |= (s[1] & 0xc0) >> 2;
	e |= (s[2] & 0xc0) >> 4;
	e |= (s[3]) >> 6;
	e ^= 0x2a; // top two bits of each tail byte correct?
	e >>= shifte[len];

	if (e)
	{
		// No bytes are consumed when *in_text == 0 || in_text == in_text_end.
		// One byte is consumed in case of invalid first byte of in_text.
		// All available bytes (at most `len` bytes) are consumed on incomplete/invalid second to last bytes.
		// Invalid or incomplete input may consume less bytes than wanted, therefore every byte has to be inspected in s.
		wanted = KGMin(wanted, !!s[0] + !!s[1] + !!s[2] + !!s[3]);
		*out_char = KG_UNICODE_CODEPOINT_INVALID;
	}

	return wanted;
}

int KGTextStrFromUtf8(KGWchar* buf, int buf_size, const char* in_text, const char* in_text_end, const char** in_text_remaining)
{
	KGWchar* buf_out = buf;
	KGWchar* buf_end = buf + buf_size;
	while (buf_out < buf_end - 1 && (!in_text_end || in_text < in_text_end) && *in_text)
	{
		unsigned int c;
		in_text += KGTextCharFromUtf8(&c, in_text, in_text_end);
		if (c == 0)
			break;
		*buf_out++ = (KGWchar)c;
	}
	*buf_out = 0;
	if (in_text_remaining)
		*in_text_remaining = in_text;
	return (int)(buf_out - buf);
}

int KGTextCountCharsFromUtf8(const char* in_text, const char* in_text_end)
{
	int char_count = 0;
	while ((!in_text_end || in_text < in_text_end) && *in_text)
	{
		unsigned int c;
		in_text += KGTextCharFromUtf8(&c, in_text, in_text_end);
		if (c == 0)
			break;
		char_count++;
	}
	return char_count;
}

// Based on stb_to_utf8() from github.com/nothings/stb/
static inline int ImTextCharToUtf8_inline(char* buf, int buf_size, unsigned int c)
{
	if (c < 0x80)
	{
		buf[0] = (char)c;
		return 1;
	}
	if (c < 0x800)
	{
		if (buf_size < 2) return 0;
		buf[0] = (char)(0xc0 + (c >> 6));
		buf[1] = (char)(0x80 + (c & 0x3f));
		return 2;
	}
	if (c < 0x10000)
	{
		if (buf_size < 3) return 0;
		buf[0] = (char)(0xe0 + (c >> 12));
		buf[1] = (char)(0x80 + ((c >> 6) & 0x3f));
		buf[2] = (char)(0x80 + ((c) & 0x3f));
		return 3;
	}
	if (c <= 0x10FFFF)
	{
		if (buf_size < 4) return 0;
		buf[0] = (char)(0xf0 + (c >> 18));
		buf[1] = (char)(0x80 + ((c >> 12) & 0x3f));
		buf[2] = (char)(0x80 + ((c >> 6) & 0x3f));
		buf[3] = (char)(0x80 + ((c) & 0x3f));
		return 4;
	}
	// Invalid code point, the max unicode is 0x10FFFF
	return 0;
}

const char* KGTextCharToUtf8(char out_buf[5], unsigned int c)
{
	int count = ImTextCharToUtf8_inline(out_buf, 5, c);
	out_buf[count] = 0;
	return out_buf;
}

// Not optimal but we very rarely use this function.
int KGTextCountUtf8BytesFromChar(const char* in_text, const char* in_text_end)
{
	unsigned int unused = 0;
	return KGTextCharFromUtf8(&unused, in_text, in_text_end);
}

static inline int KGTextCountUtf8BytesFromChar(unsigned int c)
{
	if (c < 0x80) return 1;
	if (c < 0x800) return 2;
	if (c < 0x10000) return 3;
	if (c <= 0x10FFFF) return 4;
	return 3;
}

int KGTextStrToUtf8(char* out_buf, int out_buf_size, const KGWchar* in_text, const KGWchar* in_text_end)
{
	char* buf_p = out_buf;
	const char* buf_end = out_buf + out_buf_size;
	while (buf_p < buf_end - 1 && (!in_text_end || in_text < in_text_end) && *in_text)
	{
		unsigned int c = (unsigned int)(*in_text++);
		if (c < 0x80)
			*buf_p++ = (char)c;
		else
			buf_p += ImTextCharToUtf8_inline(buf_p, (int)(buf_end - buf_p - 1), c);
	}
	*buf_p = 0;
	return (int)(buf_p - out_buf);
}

int KGTextCountUtf8BytesFromStr(const KGWchar* in_text, const KGWchar* in_text_end)
{
	int bytes_count = 0;
	while ((!in_text_end || in_text < in_text_end) && *in_text)
	{
		unsigned int c = (unsigned int)(*in_text++);
		if (c < 0x80)
			bytes_count++;
		else
			bytes_count += KGTextCountUtf8BytesFromChar(c);
	}
	return bytes_count;
}

//-----------------------------------------------------------------------------
// [SECTION] MISC HELPERS/UTILITIES (Color functions)
// Note: The Convert functions are early design which are not consistent with other API.
//-----------------------------------------------------------------------------

KGU32 KGAlphaBlendColors(KGU32 col_a, KGU32 col_b)
{
	float t = ((col_b >> KG_COL32_A_SHIFT) & 0xFF) / 255.f;
	int r = KGLerp((int)(col_a >> KG_COL32_R_SHIFT) & 0xFF, (int)(col_b >> KG_COL32_R_SHIFT) & 0xFF, t);
	int g = KGLerp((int)(col_a >> KG_COL32_G_SHIFT) & 0xFF, (int)(col_b >> KG_COL32_G_SHIFT) & 0xFF, t);
	int b = KGLerp((int)(col_a >> KG_COL32_B_SHIFT) & 0xFF, (int)(col_b >> KG_COL32_B_SHIFT) & 0xFF, t);
	return KG_COL32(r, g, b, 0xFF);
}

KGVec4 Karma::KarmaGui::ColorConvertU32ToFloat4(KGU32 in)
{
	float s = 1.0f / 255.0f;
	return KGVec4(
		((in >> KG_COL32_R_SHIFT) & 0xFF) * s,
		((in >> KG_COL32_G_SHIFT) & 0xFF) * s,
		((in >> KG_COL32_B_SHIFT) & 0xFF) * s,
		((in >> KG_COL32_A_SHIFT) & 0xFF) * s);
}

KGU32 Karma::KarmaGui::ColorConvertFloat4ToU32(const KGVec4& in)
{
	KGU32 out;
	out = ((KGU32)KG_F32_TO_INT8_SAT(in.x)) << KG_COL32_R_SHIFT;
	out |= ((KGU32)KG_F32_TO_INT8_SAT(in.y)) << KG_COL32_G_SHIFT;
	out |= ((KGU32)KG_F32_TO_INT8_SAT(in.z)) << KG_COL32_B_SHIFT;
	out |= ((KGU32)KG_F32_TO_INT8_SAT(in.w)) << KG_COL32_A_SHIFT;
	return out;
}

// Convert rgb floats ([0-1],[0-1],[0-1]) to hsv floats ([0-1],[0-1],[0-1]), from Foley & van Dam p592
// Optimized http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv
void Karma::KarmaGui::ColorConvertRGBtoHSV(float r, float g, float b, float& out_h, float& out_s, float& out_v)
{
	float K = 0.f;
	if (g < b)
	{
		KGSwap(g, b);
		K = -1.f;
	}
	if (r < g)
	{
		KGSwap(r, g);
		K = -2.f / 6.f - K;
	}

	const float chroma = r - (g < b ? g : b);
	out_h = KGFabs(K + (g - b) / (6.f * chroma + 1e-20f));
	out_s = chroma / (r + 1e-20f);
	out_v = r;
}

// Convert hsv floats ([0-1],[0-1],[0-1]) to rgb floats ([0-1],[0-1],[0-1]), from Foley & van Dam p593
// also http://en.wikipedia.org/wiki/HSL_and_HSV
void Karma::KarmaGui::ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g, float& out_b)
{
	if (s == 0.0f)
	{
		// gray
		out_r = out_g = out_b = v;
		return;
	}

	h = KGFmod(h, 1.0f) / (60.0f / 360.0f);
	int   i = (int)h;
	float f = h - (float)i;
	float p = v * (1.0f - s);
	float q = v * (1.0f - s * f);
	float t = v * (1.0f - s * (1.0f - f));

	switch (i)
	{
	case 0: out_r = v; out_g = t; out_b = p; break;
	case 1: out_r = q; out_g = v; out_b = p; break;
	case 2: out_r = p; out_g = v; out_b = t; break;
	case 3: out_r = p; out_g = q; out_b = v; break;
	case 4: out_r = t; out_g = p; out_b = v; break;
	case 5: default: out_r = v; out_g = p; out_b = q; break;
	}
}

//-----------------------------------------------------------------------------
// [SECTION] KarmaGuiStorage
// Helper: Key->value storage
//-----------------------------------------------------------------------------

// std::lower_bound but without the bullshit
KarmaGuiStorage::ImGuiStoragePair* Karma::KarmaGuiInternal::LowerBound(KGVector<KarmaGuiStorage::ImGuiStoragePair>& data, KGGuiID key)
{
	KarmaGuiStorage::ImGuiStoragePair* first = data.Data;
	KarmaGuiStorage::ImGuiStoragePair* last = data.Data + data.Size;
	size_t count = (size_t)(last - first);
	while (count > 0)
	{
		size_t count2 = count >> 1;
		KarmaGuiStorage::ImGuiStoragePair* mid = first + count2;
		if (mid->key < key)
		{
			first = ++mid;
			count -= count2 + 1;
		}
		else
		{
			count = count2;
		}
	}
	return first;
}

// For quicker full rebuild of a storage (instead of an incremental one), you may add all your contents and then sort once.
void KarmaGuiStorage::BuildSortByKey()
{
	struct StaticFunc
	{
		static int PairComparerByID(const void* lhs, const void* rhs)
		{
			// We can't just do a subtraction because qsort uses signed integers and subtracting our ID doesn't play well with that.
			if (((const ImGuiStoragePair*)lhs)->key > ((const ImGuiStoragePair*)rhs)->key) return +1;
			if (((const ImGuiStoragePair*)lhs)->key < ((const ImGuiStoragePair*)rhs)->key) return -1;
			return 0;
		}
	};
	KGQsort(Data.Data, (size_t)Data.Size, sizeof(ImGuiStoragePair), StaticFunc::PairComparerByID);
}

int KarmaGuiStorage::GetInt(KGGuiID key, int default_val) const
{
	ImGuiStoragePair* it = Karma::KarmaGuiInternal::LowerBound(const_cast<KGVector<ImGuiStoragePair>&>(Data), key);
	if (it == Data.end() || it->key != key)
		return default_val;
	return it->val_i;
}

bool KarmaGuiStorage::GetBool(KGGuiID key, bool default_val) const
{
	return GetInt(key, default_val ? 1 : 0) != 0;
}

float KarmaGuiStorage::GetFloat(KGGuiID key, float default_val) const
{
	ImGuiStoragePair* it = Karma::KarmaGuiInternal::LowerBound(const_cast<KGVector<ImGuiStoragePair>&>(Data), key);
	if (it == Data.end() || it->key != key)
		return default_val;
	return it->val_f;
}

void* KarmaGuiStorage::GetVoidPtr(KGGuiID key) const
{
	ImGuiStoragePair* it = Karma::KarmaGuiInternal::LowerBound(const_cast<KGVector<ImGuiStoragePair>&>(Data), key);
	if (it == Data.end() || it->key != key)
		return NULL;
	return it->val_p;
}

// References are only valid until a new value is added to the storage. Calling a Set***() function or a Get***Ref() function invalidates the pointer.
int* KarmaGuiStorage::GetIntRef(KGGuiID key, int default_val)
{
	ImGuiStoragePair* it = Karma::KarmaGuiInternal::LowerBound(Data, key);
	if (it == Data.end() || it->key != key)
		it = Data.insert(it, ImGuiStoragePair(key, default_val));
	return &it->val_i;
}

bool* KarmaGuiStorage::GetBoolRef(KGGuiID key, bool default_val)
{
	return (bool*)GetIntRef(key, default_val ? 1 : 0);
}

float* KarmaGuiStorage::GetFloatRef(KGGuiID key, float default_val)
{
	ImGuiStoragePair* it = Karma::KarmaGuiInternal::LowerBound(Data, key);
	if (it == Data.end() || it->key != key)
		it = Data.insert(it, ImGuiStoragePair(key, default_val));
	return &it->val_f;
}

void** KarmaGuiStorage::GetVoidPtrRef(KGGuiID key, void* default_val)
{
	ImGuiStoragePair* it = Karma::KarmaGuiInternal::LowerBound(Data, key);
	if (it == Data.end() || it->key != key)
		it = Data.insert(it, ImGuiStoragePair(key, default_val));
	return &it->val_p;
}

// FIXME-OPT: Need a way to reuse the result of lower_bound when doing GetInt()/SetInt() - not too bad because it only happens on explicit interaction (maximum one a frame)
void KarmaGuiStorage::SetInt(KGGuiID key, int val)
{
	ImGuiStoragePair* it = Karma::KarmaGuiInternal::LowerBound(Data, key);
	if (it == Data.end() || it->key != key)
	{
		Data.insert(it, ImGuiStoragePair(key, val));
		return;
	}
	it->val_i = val;
}

void KarmaGuiStorage::SetBool(KGGuiID key, bool val)
{
	SetInt(key, val ? 1 : 0);
}

void KarmaGuiStorage::SetFloat(KGGuiID key, float val)
{
	ImGuiStoragePair* it = Karma::KarmaGuiInternal::LowerBound(Data, key);
	if (it == Data.end() || it->key != key)
	{
		Data.insert(it, ImGuiStoragePair(key, val));
		return;
	}
	it->val_f = val;
}

void KarmaGuiStorage::SetVoidPtr(KGGuiID key, void* val)
{
	ImGuiStoragePair* it = Karma::KarmaGuiInternal::LowerBound(Data, key);
	if (it == Data.end() || it->key != key)
	{
		Data.insert(it, ImGuiStoragePair(key, val));
		return;
	}
	it->val_p = val;
}

void KarmaGuiStorage::SetAllInt(int v)
{
	for (int i = 0; i < Data.Size; i++)
		Data[i].val_i = v;
}

//-----------------------------------------------------------------------------
// [SECTION] KarmaGuiTextFilter
//-----------------------------------------------------------------------------

// Helper: Parse and apply text filters. In format "aaaaa[,bbbb][,ccccc]"
KarmaGuiTextFilter::KarmaGuiTextFilter(const char* default_filter) //-V1077
{
	InputBuf[0] = 0;
	CountGrep = 0;
	if (default_filter)
	{
		KGStrncpy(InputBuf, default_filter, KG_ARRAYSIZE(InputBuf));
		Build();
	}
}

bool KarmaGuiTextFilter::Draw(const char* label, float width)
{
	if (width != 0.0f)
		Karma::KarmaGui::SetNextItemWidth(width);
	bool value_changed = Karma::KarmaGui::InputText(label, InputBuf, KG_ARRAYSIZE(InputBuf));
	if (value_changed)
		Build();
	return value_changed;
}

void KarmaGuiTextFilter::ImGuiTextRange::split(char separator, KGVector<ImGuiTextRange>* out) const
{
	out->resize(0);
	const char* wb = b;
	const char* we = wb;
	while (we < e)
	{
		if (*we == separator)
		{
			out->push_back(ImGuiTextRange(wb, we));
			wb = we + 1;
		}
		we++;
	}
	if (wb != we)
		out->push_back(ImGuiTextRange(wb, we));
}

void KarmaGuiTextFilter::Build()
{
	Filters.resize(0);
	ImGuiTextRange input_range(InputBuf, InputBuf + strlen(InputBuf));
	input_range.split(',', &Filters);

	CountGrep = 0;
	for (int i = 0; i != Filters.Size; i++)
	{
		ImGuiTextRange& f = Filters[i];
		while (f.b < f.e && KGCharIsBlankA(f.b[0]))
			f.b++;
		while (f.e > f.b && KGCharIsBlankA(f.e[-1]))
			f.e--;
		if (f.empty())
			continue;
		if (Filters[i].b[0] != '-')
			CountGrep += 1;
	}
}

bool KarmaGuiTextFilter::PassFilter(const char* text, const char* text_end) const
{
	if (Filters.empty())
		return true;

	if (text == NULL)
		text = "";

	for (int i = 0; i != Filters.Size; i++)
	{
		const ImGuiTextRange& f = Filters[i];
		if (f.empty())
			continue;
		if (f.b[0] == '-')
		{
			// Subtract
			if (KGStristr(text, text_end, f.b + 1, f.e) != NULL)
				return false;
		}
		else
		{
			// Grep
			if (KGStristr(text, text_end, f.b, f.e) != NULL)
				return true;
		}
	}

	// Implicit * grep
	if (CountGrep == 0)
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// [SECTION] KarmaGuiTextBuffer, KGGuiTextIndex
//-----------------------------------------------------------------------------

// On some platform vsnprintf() takes va_list by reference and modifies it.
// va_copy is the 'correct' way to copy a va_list but Visual Studio prior to 2013 doesn't have it.
#ifndef va_copy
#if defined(__GNUC__) || defined(__clang__)
#define va_copy(dest, src) __builtin_va_copy(dest, src)
#else
#define va_copy(dest, src) (dest = src)
#endif
#endif

char KarmaGuiTextBuffer::EmptyString[1] = { 0 };

void KarmaGuiTextBuffer::append(const char* str, const char* str_end)
{
	int len = str_end ? (int)(str_end - str) : (int)strlen(str);

	// Add zero-terminator the first time
	const int write_off = (Buf.Size != 0) ? Buf.Size : 1;
	const int needed_sz = write_off + len;
	if (write_off + len >= Buf.Capacity)
	{
		int new_capacity = Buf.Capacity * 2;
		Buf.reserve(needed_sz > new_capacity ? needed_sz : new_capacity);
	}

	Buf.resize(needed_sz);
	memcpy(&Buf[write_off - 1], str, (size_t)len);
	Buf[write_off - 1 + len] = 0;
}

void KarmaGuiTextBuffer::appendf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	appendfv(fmt, args);
	va_end(args);
}

// Helper: Text buffer for logging/accumulating text
void KarmaGuiTextBuffer::appendfv(const char* fmt, va_list args)
{
	va_list args_copy;
	va_copy(args_copy, args);

	int len = KGFormatStringV(NULL, 0, fmt, args);         // FIXME-OPT: could do a first pass write attempt, likely successful on first pass.
	if (len <= 0)
	{
		va_end(args_copy);
		return;
	}

	// Add zero-terminator the first time
	const int write_off = (Buf.Size != 0) ? Buf.Size : 1;
	const int needed_sz = write_off + len;
	if (write_off + len >= Buf.Capacity)
	{
		int new_capacity = Buf.Capacity * 2;
		Buf.reserve(needed_sz > new_capacity ? needed_sz : new_capacity);
	}

	Buf.resize(needed_sz);
	KGFormatStringV(&Buf[write_off - 1], (size_t)len + 1, fmt, args_copy);
	va_end(args_copy);
}

void KGGuiTextIndex::append(const char* base, int old_size, int new_size)
{
	KR_CORE_ASSERT(old_size >= 0 && new_size >= old_size && new_size >= EndOffset, "");
	if (old_size == new_size)
		return;
	if (EndOffset == 0 || base[EndOffset - 1] == '\n')
		LineOffsets.push_back(EndOffset);
	const char* base_end = base + new_size;
	for (const char* p = base + old_size; (p = (const char*)memchr(p, '\n', base_end - p)) != 0; )
		if (++p < base_end) // Don't push a trailing offset on last \n
			LineOffsets.push_back((int)(intptr_t)(p - base));
	EndOffset = KGMax(EndOffset, new_size);
}

//-----------------------------------------------------------------------------
// [SECTION] KarmaGuiListClipper
// This is currently not as flexible/powerful as it should be and really confusing/spaghetti, mostly because we changed
// the API mid-way through development and support two ways to using the clipper, needs some rework (see TODO)
//-----------------------------------------------------------------------------

// FIXME-TABLE: This prevents us from using KarmaGuiListClipper _inside_ a table cell.
// The problem we have is that without a Begin/End scheme for rows using the clipper is ambiguous.
static bool GetSkipItemForListClipping()
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	return (g.CurrentTable ? g.CurrentTable->HostSkipItems : g.CurrentWindow->SkipItems);
}

static void ImGuiListClipper_SortAndFuseRanges(KGVector<KGGuiListClipperRange>& ranges, int offset = 0)
{
	if (ranges.Size - offset <= 1)
		return;

	// Helper to order ranges and fuse them together if possible (bubble sort is fine as we are only sorting 2-3 entries)
	for (int sort_end = ranges.Size - offset - 1; sort_end > 0; --sort_end)
		for (int i = offset; i < sort_end + offset; ++i)
			if (ranges[i].Min > ranges[i + 1].Min)
				KGSwap(ranges[i], ranges[i + 1]);

	// Now fuse ranges together as much as possible.
	for (int i = 1 + offset; i < ranges.Size; i++)
	{
		KR_CORE_ASSERT(!ranges[i].PosToIndexConvert && !ranges[i - 1].PosToIndexConvert, "");
		if (ranges[i - 1].Max < ranges[i].Min)
			continue;
		ranges[i - 1].Min = KGMin(ranges[i - 1].Min, ranges[i].Min);
		ranges[i - 1].Max = KGMax(ranges[i - 1].Max, ranges[i].Max);
		ranges.erase(ranges.Data + i);
		i--;
	}
}

static void ImGuiListClipper_SeekCursorAndSetupPrevLine(float pos_y, float line_height)
{
	// Set cursor position and a few other things so that SetScrollHereY() and Columns() can work when seeking cursor.
	// FIXME: It is problematic that we have to do that here, because custom/equivalent end-user code would stumble on the same issue.
	// The clipper should probably have a final step to display the last item in a regular manner, maybe with an opt-out flag for data sets which may have costly seek?
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	float off_y = pos_y - window->DC.CursorPos.y;
	window->DC.CursorPos.y = pos_y;
	window->DC.CursorMaxPos.y = KGMax(window->DC.CursorMaxPos.y, pos_y - g.Style.ItemSpacing.y);
	window->DC.CursorPosPrevLine.y = window->DC.CursorPos.y - line_height;  // Setting those fields so that SetScrollHereY() can properly function after the end of our clipper usage.
	window->DC.PrevLineSize.y = (line_height - g.Style.ItemSpacing.y);      // If we end up needing more accurate data (to e.g. use SameLine) we may as well make the clipper have a fourth step to let user process and display the last item in their list.
	if (KGGuiOldColumns* columns = window->DC.CurrentColumns)
		columns->LineMinY = window->DC.CursorPos.y;                         // Setting this so that cell Y position are set properly
	if (KGGuiTable* table = g.CurrentTable)
	{
		if (table->IsInsideRow)
			Karma::KarmaGuiInternal::TableEndRow(table);
		table->RowPosY2 = window->DC.CursorPos.y;
		const int row_increase = (int)((off_y / line_height) + 0.5f);
		//table->CurrentRow += row_increase; // Can't do without fixing TableEndRow()
		table->RowBgColorCounter += row_increase;
	}
}

static void ImGuiListClipper_SeekCursorForItem(KarmaGuiListClipper* clipper, int item_n)
{
	// StartPosY starts from ItemsFrozen hence the subtraction
	// Perform the add and multiply with double to allow seeking through larger ranges
	KGGuiListClipperData* data = (KGGuiListClipperData*)clipper->TempData;
	float pos_y = (float)((double)clipper->StartPosY + data->LossynessOffset + (double)(item_n - data->ItemsFrozen) * clipper->ItemsHeight);
	ImGuiListClipper_SeekCursorAndSetupPrevLine(pos_y, clipper->ItemsHeight);
}

KarmaGuiListClipper::KarmaGuiListClipper()
{
	memset(this, 0, sizeof(*this));
	ItemsCount = -1;
}

KarmaGuiListClipper::~KarmaGuiListClipper()
{
	End();
}

void KarmaGuiListClipper::Begin(int items_count, float items_height)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	//KR_CORE_INFO("Clipper: Begin(%d,%.2f) in '%s'\n", items_count, items_height, window->Name);

	if (KGGuiTable* table = g.CurrentTable)
		if (table->IsInsideRow)
			Karma::KarmaGuiInternal::TableEndRow(table);

	StartPosY = window->DC.CursorPos.y;
	ItemsHeight = items_height;
	ItemsCount = items_count;
	DisplayStart = -1;
	DisplayEnd = 0;

	// Acquire temporary buffer
	if (++g.ClipperTempDataStacked > g.ClipperTempData.Size)
		g.ClipperTempData.resize(g.ClipperTempDataStacked, KGGuiListClipperData());
	KGGuiListClipperData* data = &g.ClipperTempData[g.ClipperTempDataStacked - 1];
	data->Reset(this);
	data->LossynessOffset = window->DC.CursorStartPosLossyness.y;
	TempData = data;
}

void KarmaGuiListClipper::End()
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	if (KGGuiListClipperData* data = (KGGuiListClipperData*)TempData)
	{
		// In theory here we should assert that we are already at the right position, but it seems saner to just seek at the end and not assert/crash the user.
		// KR_CORE_INFO("Clipper: End() in '%s'\n", g.CurrentWindow->Name);
		if (ItemsCount >= 0 && ItemsCount < INT_MAX && DisplayStart >= 0)
			ImGuiListClipper_SeekCursorForItem(this, ItemsCount);

		// Restore temporary buffer and fix back pointers which may be invalidated when nesting
		KR_CORE_ASSERT(data->ListClipper == this, "");
		data->StepNo = data->Ranges.Size;
		if (--g.ClipperTempDataStacked > 0)
		{
			data = &g.ClipperTempData[g.ClipperTempDataStacked - 1];
			data->ListClipper->TempData = data;
		}
		TempData = NULL;
	}
	ItemsCount = -1;
}

void KarmaGuiListClipper::ForceDisplayRangeByIndices(int item_min, int item_max)
{
	KGGuiListClipperData* data = (KGGuiListClipperData*)TempData;
	KR_CORE_ASSERT(DisplayStart < 0, ""); // Only allowed after Begin() and if there has not been a specified range yet.
	KR_CORE_ASSERT(item_min <= item_max, "");
	if (item_min < item_max)
		data->Ranges.push_back(KGGuiListClipperRange::FromIndices(item_min, item_max));
}

static bool ImGuiListClipper_StepInternal(KarmaGuiListClipper* clipper)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	KGGuiListClipperData* data = (KGGuiListClipperData*)clipper->TempData;
	KR_CORE_ASSERT(data != NULL, "Called KarmaGuiListClipper::Step() too many times, or before KarmaGuiListClipper::Begin() ?");

	KGGuiTable* table = g.CurrentTable;
	if (table && table->IsInsideRow)
		Karma::KarmaGuiInternal::TableEndRow(table);

	// No items
	if (clipper->ItemsCount == 0 || GetSkipItemForListClipping())
		return false;

	// While we are in frozen row state, keep displaying items one by one, unclipped
	// FIXME: Could be stored as a table-agnostic state.
	if (data->StepNo == 0 && table != NULL && !table->IsUnfrozenRows)
	{
		clipper->DisplayStart = data->ItemsFrozen;
		clipper->DisplayEnd = KGMin(data->ItemsFrozen + 1, clipper->ItemsCount);
		if (clipper->DisplayStart < clipper->DisplayEnd)
			data->ItemsFrozen++;
		return true;
	}

	// Step 0: Let you process the first element (regardless of it being visible or not, so we can measure the element height)
	bool calc_clipping = false;
	if (data->StepNo == 0)
	{
		clipper->StartPosY = window->DC.CursorPos.y;
		if (clipper->ItemsHeight <= 0.0f)
		{
			// Submit the first item (or range) so we can measure its height (generally the first range is 0..1)
			data->Ranges.push_front(KGGuiListClipperRange::FromIndices(data->ItemsFrozen, data->ItemsFrozen + 1));
			clipper->DisplayStart = KGMax(data->Ranges[0].Min, data->ItemsFrozen);
			clipper->DisplayEnd = KGMin(data->Ranges[0].Max, clipper->ItemsCount);
			data->StepNo = 1;
			return true;
		}
		calc_clipping = true;   // If on the first step with known item height, calculate clipping.
	}

	// Step 1: Let the clipper infer height from first range
	if (clipper->ItemsHeight <= 0.0f)
	{
		KR_CORE_ASSERT(data->StepNo == 1, "");
		if (table)
		{
			KR_CORE_ASSERT((table->RowPosY1 == clipper->StartPosY) && (table->RowPosY2 == window->DC.CursorPos.y), "");
		}

		clipper->ItemsHeight = (window->DC.CursorPos.y - clipper->StartPosY) / (float)(clipper->DisplayEnd - clipper->DisplayStart);
		bool affected_by_floating_point_precision = KGIsFloatAboveGuaranteedIntegerPrecision(clipper->StartPosY) || KGIsFloatAboveGuaranteedIntegerPrecision(window->DC.CursorPos.y);
		if (affected_by_floating_point_precision)
			clipper->ItemsHeight = window->DC.PrevLineSize.y + g.Style.ItemSpacing.y; // FIXME: Technically wouldn't allow multi-line entries.

		KR_CORE_ASSERT(clipper->ItemsHeight > 0.0f, "Unable to calculate item height! First item hasn't moved the cursor vertically!");
		calc_clipping = true;   // If item height had to be calculated, calculate clipping afterwards.
	}

	// Step 0 or 1: Calculate the actual ranges of visible elements.
	const int already_submitted = clipper->DisplayEnd;
	if (calc_clipping)
	{
		if (g.LogEnabled)
		{
			// If logging is active, do not perform any clipping
			data->Ranges.push_back(KGGuiListClipperRange::FromIndices(0, clipper->ItemsCount));
		}
		else
		{
			// Add range selected to be included for navigation
			const bool is_nav_request = (g.NavMoveScoringItems && g.NavWindow && g.NavWindow->RootWindowForNav == window->RootWindowForNav);
			if (is_nav_request)
				data->Ranges.push_back(KGGuiListClipperRange::FromPositions(g.NavScoringNoClipRect.Min.y, g.NavScoringNoClipRect.Max.y, 0, 0));
			if (is_nav_request && (g.NavMoveFlags & KGGuiNavMoveFlags_Tabbing) && g.NavTabbingDir == -1)
				data->Ranges.push_back(KGGuiListClipperRange::FromIndices(clipper->ItemsCount - 1, clipper->ItemsCount));

			// Add focused/active item
			KGRect nav_rect_abs = Karma::KarmaGuiInternal::WindowRectRelToAbs(window, window->NavRectRel[0]);
			if (g.NavId != 0 && window->NavLastIds[0] == g.NavId)
				data->Ranges.push_back(KGGuiListClipperRange::FromPositions(nav_rect_abs.Min.y, nav_rect_abs.Max.y, 0, 0));

			// Add visible range
			const int off_min = (is_nav_request && g.NavMoveClipDir == KGGuiDir_Up) ? -1 : 0;
			const int off_max = (is_nav_request && g.NavMoveClipDir == KGGuiDir_Down) ? 1 : 0;
			data->Ranges.push_back(KGGuiListClipperRange::FromPositions(window->ClipRect.Min.y, window->ClipRect.Max.y, off_min, off_max));
		}

		// Convert position ranges to item index ranges
		// - Very important: when a starting position is after our maximum item, we set Min to (ItemsCount - 1). This allows us to handle most forms of wrapping.
		// - Due to how Selectable extra padding they tend to be "unaligned" with exact unit in the item list,
		//   which with the flooring/ceiling tend to lead to 2 items instead of one being submitted.
		for (int i = 0; i < data->Ranges.Size; i++)
			if (data->Ranges[i].PosToIndexConvert)
			{
				int m1 = (int)(((double)data->Ranges[i].Min - window->DC.CursorPos.y - data->LossynessOffset) / clipper->ItemsHeight);
				int m2 = (int)((((double)data->Ranges[i].Max - window->DC.CursorPos.y - data->LossynessOffset) / clipper->ItemsHeight) + 0.999999f);
				data->Ranges[i].Min = KGClamp(already_submitted + m1 + data->Ranges[i].PosToIndexOffsetMin, already_submitted, clipper->ItemsCount - 1);
				data->Ranges[i].Max = KGClamp(already_submitted + m2 + data->Ranges[i].PosToIndexOffsetMax, data->Ranges[i].Min + 1, clipper->ItemsCount);
				data->Ranges[i].PosToIndexConvert = false;
			}
		ImGuiListClipper_SortAndFuseRanges(data->Ranges, data->StepNo);
	}

	// Step 0+ (if item height is given in advance) or 1+: Display the next range in line.
	if (data->StepNo < data->Ranges.Size)
	{
		clipper->DisplayStart = KGMax(data->Ranges[data->StepNo].Min, already_submitted);
		clipper->DisplayEnd = KGMin(data->Ranges[data->StepNo].Max, clipper->ItemsCount);
		if (clipper->DisplayStart > already_submitted) //-V1051
			ImGuiListClipper_SeekCursorForItem(clipper, clipper->DisplayStart);
		data->StepNo++;
		return true;
	}

	// After the last step: Let the clipper validate that we have reached the expected Y position (corresponding to element DisplayEnd),
	// Advance the cursor to the end of the list and then returns 'false' to end the loop.
	if (clipper->ItemsCount < INT_MAX)
		ImGuiListClipper_SeekCursorForItem(clipper, clipper->ItemsCount);

	return false;
}

bool KarmaGuiListClipper::Step()
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	bool need_items_height = (ItemsHeight <= 0.0f);
	bool ret = ImGuiListClipper_StepInternal(this);
	if (ret && (DisplayStart == DisplayEnd))
		ret = false;
	if (g.CurrentTable && g.CurrentTable->IsUnfrozenRows == false);
		//KR_CORE_INFO("Clipper: Step(): inside frozen table row.");
	if (need_items_height && ItemsHeight > 0.0f);
		//KR_CORE_INFO("Clipper: Step(): computed ItemsHeight: %.2f.", ItemsHeight);
	if (ret)
	{
		//KR_CORE_INFO("Clipper: Step(): display %d to %d.", DisplayStart, DisplayEnd);
	}
	else
	{
		//KR_CORE_INFO("Clipper: Step(): End.");
		End();
	}
	return ret;
}

//-----------------------------------------------------------------------------
// [SECTION] STYLING
//-----------------------------------------------------------------------------

KarmaGuiStyle& Karma::KarmaGui::GetStyle()
{
	KR_CORE_ASSERT(Karma::GKarmaGui != NULL, "No current context. Did you call KarmaGui::CreateContext() and KarmaGui::SetCurrentContext() ?");
	return Karma::GKarmaGui->Style;
}

KGU32 Karma::KarmaGui::GetColorU32(KarmaGuiCol idx, float alpha_mul)
{
	KarmaGuiStyle& style = Karma::GKarmaGui->Style;
	KGVec4 c = style.Colors[idx];
	c.w *= style.Alpha * alpha_mul;
	return ColorConvertFloat4ToU32(c);
}

KGU32 Karma::KarmaGui::GetColorU32(const KGVec4& col)
{
	KarmaGuiStyle& style = Karma::GKarmaGui->Style;
	KGVec4 c = col;
	c.w *= style.Alpha;
	return ColorConvertFloat4ToU32(c);
}

const KGVec4& Karma::KarmaGui::GetStyleColorVec4(KarmaGuiCol idx)
{
	KarmaGuiStyle& style = Karma::GKarmaGui->Style;
	return style.Colors[idx];
}

KGU32 Karma::KarmaGui::GetColorU32(KGU32 col)
{
	KarmaGuiStyle& style = Karma::GKarmaGui->Style;
	if (style.Alpha >= 1.0f)
		return col;
	KGU32 a = (col & KG_COL32_A_MASK) >> KG_COL32_A_SHIFT;
	a = (KGU32)(a * style.Alpha); // We don't need to clamp 0..255 because Style.Alpha is in 0..1 range.
	return (col & ~KG_COL32_A_MASK) | (a << KG_COL32_A_SHIFT);
}

// FIXME: This may incur a round-trip (if the end user got their data from a float4) but eventually we aim to store the in-flight colors as KGU32
void Karma::KarmaGui::PushStyleColor(KarmaGuiCol idx, KGU32 col)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGGuiColorMod backup;
	backup.Col = idx;
	backup.BackupValue = g.Style.Colors[idx];
	g.ColorStack.push_back(backup);
	g.Style.Colors[idx] = ColorConvertU32ToFloat4(col);
}

void Karma::KarmaGui::PushStyleColor(KarmaGuiCol idx, const KGVec4& col)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGGuiColorMod backup;
	backup.Col = idx;
	backup.BackupValue = g.Style.Colors[idx];
	g.ColorStack.push_back(backup);
	g.Style.Colors[idx] = col;
}

void Karma::KarmaGui::PopStyleColor(int count)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	if (g.ColorStack.Size < count)
	{
		KR_CORE_ASSERT(g.ColorStack.Size > count, "Calling PopStyleColor() too many times: stack underflow.");
		count = g.ColorStack.Size;
	}
	while (count > 0)
	{
		KGGuiColorMod& backup = g.ColorStack.back();
		g.Style.Colors[backup.Col] = backup.BackupValue;
		g.ColorStack.pop_back();
		count--;
	}
}

struct ImGuiStyleVarInfo
{
	KarmaGuiDataType   Type;
	KGU32           Count;
	KGU32           Offset;
	void* GetVarPtr(KarmaGuiStyle* style) const { return (void*)((unsigned char*)style + Offset); }
};

static const KarmaGuiCol GWindowDockStyleColors[KGGuiWindowDockStyleCol_COUNT] =
{
	KGGuiCol_Text, KGGuiCol_Tab, KGGuiCol_TabHovered, KGGuiCol_TabActive, KGGuiCol_TabUnfocused, KGGuiCol_TabUnfocusedActive
};

static const ImGuiStyleVarInfo GStyleVarInfo[] =
{
	{ KGGuiDataType_Float, 1, (KGU32)KG_OFFSETOF(KarmaGuiStyle, Alpha) },               // KGGuiStyleVar_Alpha
	{ KGGuiDataType_Float, 1, (KGU32)KG_OFFSETOF(KarmaGuiStyle, DisabledAlpha) },       // KGGuiStyleVar_DisabledAlpha
	{ KGGuiDataType_Float, 2, (KGU32)KG_OFFSETOF(KarmaGuiStyle, WindowPadding) },       // KGGuiStyleVar_WindowPadding
	{ KGGuiDataType_Float, 1, (KGU32)KG_OFFSETOF(KarmaGuiStyle, WindowRounding) },      // KGGuiStyleVar_WindowRounding
	{ KGGuiDataType_Float, 1, (KGU32)KG_OFFSETOF(KarmaGuiStyle, WindowBorderSize) },    // KGGuiStyleVar_WindowBorderSize
	{ KGGuiDataType_Float, 2, (KGU32)KG_OFFSETOF(KarmaGuiStyle, WindowMinSize) },       // KGGuiStyleVar_WindowMinSize
	{ KGGuiDataType_Float, 2, (KGU32)KG_OFFSETOF(KarmaGuiStyle, WindowTitleAlign) },    // KGGuiStyleVar_WindowTitleAlign
	{ KGGuiDataType_Float, 1, (KGU32)KG_OFFSETOF(KarmaGuiStyle, ChildRounding) },       // KGGuiStyleVar_ChildRounding
	{ KGGuiDataType_Float, 1, (KGU32)KG_OFFSETOF(KarmaGuiStyle, ChildBorderSize) },     // KGGuiStyleVar_ChildBorderSize
	{ KGGuiDataType_Float, 1, (KGU32)KG_OFFSETOF(KarmaGuiStyle, PopupRounding) },       // KGGuiStyleVar_PopupRounding
	{ KGGuiDataType_Float, 1, (KGU32)KG_OFFSETOF(KarmaGuiStyle, PopupBorderSize) },     // KGGuiStyleVar_PopupBorderSize
	{ KGGuiDataType_Float, 2, (KGU32)KG_OFFSETOF(KarmaGuiStyle, FramePadding) },        // KGGuiStyleVar_FramePadding
	{ KGGuiDataType_Float, 1, (KGU32)KG_OFFSETOF(KarmaGuiStyle, FrameRounding) },       // KGGuiStyleVar_FrameRounding
	{ KGGuiDataType_Float, 1, (KGU32)KG_OFFSETOF(KarmaGuiStyle, FrameBorderSize) },     // KGGuiStyleVar_FrameBorderSize
	{ KGGuiDataType_Float, 2, (KGU32)KG_OFFSETOF(KarmaGuiStyle, ItemSpacing) },         // KGGuiStyleVar_ItemSpacing
	{ KGGuiDataType_Float, 2, (KGU32)KG_OFFSETOF(KarmaGuiStyle, ItemInnerSpacing) },    // KGGuiStyleVar_ItemInnerSpacing
	{ KGGuiDataType_Float, 1, (KGU32)KG_OFFSETOF(KarmaGuiStyle, IndentSpacing) },       // KGGuiStyleVar_IndentSpacing
	{ KGGuiDataType_Float, 2, (KGU32)KG_OFFSETOF(KarmaGuiStyle, CellPadding) },         // KGGuiStyleVar_CellPadding
	{ KGGuiDataType_Float, 1, (KGU32)KG_OFFSETOF(KarmaGuiStyle, ScrollbarSize) },       // KGGuiStyleVar_ScrollbarSize
	{ KGGuiDataType_Float, 1, (KGU32)KG_OFFSETOF(KarmaGuiStyle, ScrollbarRounding) },   // KGGuiStyleVar_ScrollbarRounding
	{ KGGuiDataType_Float, 1, (KGU32)KG_OFFSETOF(KarmaGuiStyle, GrabMinSize) },         // KGGuiStyleVar_GrabMinSize
	{ KGGuiDataType_Float, 1, (KGU32)KG_OFFSETOF(KarmaGuiStyle, GrabRounding) },        // KGGuiStyleVar_GrabRounding
	{ KGGuiDataType_Float, 1, (KGU32)KG_OFFSETOF(KarmaGuiStyle, TabRounding) },         // KGGuiStyleVar_TabRounding
	{ KGGuiDataType_Float, 2, (KGU32)KG_OFFSETOF(KarmaGuiStyle, ButtonTextAlign) },     // KGGuiStyleVar_ButtonTextAlign
	{ KGGuiDataType_Float, 2, (KGU32)KG_OFFSETOF(KarmaGuiStyle, SelectableTextAlign) }, // KGGuiStyleVar_SelectableTextAlign
};

static const ImGuiStyleVarInfo* GetStyleVarInfo(KarmaGuiStyleVar idx)
{
	KR_CORE_ASSERT(idx >= 0 && idx < KGGuiStyleVar_COUNT, "");
	KR_CORE_ASSERT(KG_ARRAYSIZE(GStyleVarInfo) == KGGuiStyleVar_COUNT, "");
	return &GStyleVarInfo[idx];
}

void Karma::KarmaGui::PushStyleVar(KarmaGuiStyleVar idx, float val)
{
	const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(idx);
	if (var_info->Type == KGGuiDataType_Float && var_info->Count == 1)
	{
		KarmaGuiContext& g = *Karma::GKarmaGui;
		float* pvar = (float*)var_info->GetVarPtr(&g.Style);
		g.StyleVarStack.push_back(KGGuiStyleMod(idx, *pvar));
		*pvar = val;
		return;
	}
	KR_CORE_ASSERT(0, "Called PushStyleVar() float variant but variable is not a float!");
}

void Karma::KarmaGui::PushStyleVar(KarmaGuiStyleVar idx, const KGVec2& val)
{
	const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(idx);
	if (var_info->Type == KGGuiDataType_Float && var_info->Count == 2)
	{
		KarmaGuiContext& g = *Karma::GKarmaGui;
		KGVec2* pvar = (KGVec2*)var_info->GetVarPtr(&g.Style);
		g.StyleVarStack.push_back(KGGuiStyleMod(idx, *pvar));
		*pvar = val;
		return;
	}
	KR_CORE_ASSERT(0, "Called PushStyleVar() KGVec2 variant but variable is not a KGVec2!");
}

void Karma::KarmaGui::PopStyleVar(int count)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	if (g.StyleVarStack.Size < count)
	{
		KR_CORE_ASSERT(g.StyleVarStack.Size > count, "Calling PopStyleVar() too many times: stack underflow.");
		count = g.StyleVarStack.Size;
	}
	while (count > 0)
	{
		// We avoid a generic memcpy(data, &backup.Backup.., GDataTypeSize[info->Type] * info->Count), the overhead in Debug is not worth it.
		KGGuiStyleMod& backup = g.StyleVarStack.back();
		const ImGuiStyleVarInfo* info = GetStyleVarInfo(backup.VarIdx);
		void* data = info->GetVarPtr(&g.Style);
		if (info->Type == KGGuiDataType_Float && info->Count == 1) { ((float*)data)[0] = backup.BackupFloat[0]; }
		else if (info->Type == KGGuiDataType_Float && info->Count == 2) { ((float*)data)[0] = backup.BackupFloat[0]; ((float*)data)[1] = backup.BackupFloat[1]; }
		g.StyleVarStack.pop_back();
		count--;
	}
}

const char* Karma::KarmaGui::GetStyleColorName(KarmaGuiCol idx)
{
	// Create switch-case from enum with regexp: KGGuiCol_{.*}, --> case KGGuiCol_\1: return "\1";
	switch (idx)
	{
	case KGGuiCol_Text: return "Text";
	case KGGuiCol_TextDisabled: return "TextDisabled";
	case KGGuiCol_WindowBg: return "WindowBg";
	case KGGuiCol_ChildBg: return "ChildBg";
	case KGGuiCol_PopupBg: return "PopupBg";
	case KGGuiCol_Border: return "Border";
	case KGGuiCol_BorderShadow: return "BorderShadow";
	case KGGuiCol_FrameBg: return "FrameBg";
	case KGGuiCol_FrameBgHovered: return "FrameBgHovered";
	case KGGuiCol_FrameBgActive: return "FrameBgActive";
	case KGGuiCol_TitleBg: return "TitleBg";
	case KGGuiCol_TitleBgActive: return "TitleBgActive";
	case KGGuiCol_TitleBgCollapsed: return "TitleBgCollapsed";
	case KGGuiCol_MenuBarBg: return "MenuBarBg";
	case KGGuiCol_ScrollbarBg: return "ScrollbarBg";
	case KGGuiCol_ScrollbarGrab: return "ScrollbarGrab";
	case KGGuiCol_ScrollbarGrabHovered: return "ScrollbarGrabHovered";
	case KGGuiCol_ScrollbarGrabActive: return "ScrollbarGrabActive";
	case KGGuiCol_CheckMark: return "CheckMark";
	case KGGuiCol_SliderGrab: return "SliderGrab";
	case KGGuiCol_SliderGrabActive: return "SliderGrabActive";
	case KGGuiCol_Button: return "Button";
	case KGGuiCol_ButtonHovered: return "ButtonHovered";
	case KGGuiCol_ButtonActive: return "ButtonActive";
	case KGGuiCol_Header: return "Header";
	case KGGuiCol_HeaderHovered: return "HeaderHovered";
	case KGGuiCol_HeaderActive: return "HeaderActive";
	case KGGuiCol_Separator: return "Separator";
	case KGGuiCol_SeparatorHovered: return "SeparatorHovered";
	case KGGuiCol_SeparatorActive: return "SeparatorActive";
	case KGGuiCol_ResizeGrip: return "ResizeGrip";
	case KGGuiCol_ResizeGripHovered: return "ResizeGripHovered";
	case KGGuiCol_ResizeGripActive: return "ResizeGripActive";
	case KGGuiCol_Tab: return "Tab";
	case KGGuiCol_TabHovered: return "TabHovered";
	case KGGuiCol_TabActive: return "TabActive";
	case KGGuiCol_TabUnfocused: return "TabUnfocused";
	case KGGuiCol_TabUnfocusedActive: return "TabUnfocusedActive";
	case KGGuiCol_DockingPreview: return "DockingPreview";
	case KGGuiCol_DockingEmptyBg: return "DockingEmptyBg";
	case KGGuiCol_PlotLines: return "PlotLines";
	case KGGuiCol_PlotLinesHovered: return "PlotLinesHovered";
	case KGGuiCol_PlotHistogram: return "PlotHistogram";
	case KGGuiCol_PlotHistogramHovered: return "PlotHistogramHovered";
	case KGGuiCol_TableHeaderBg: return "TableHeaderBg";
	case KGGuiCol_TableBorderStrong: return "TableBorderStrong";
	case KGGuiCol_TableBorderLight: return "TableBorderLight";
	case KGGuiCol_TableRowBg: return "TableRowBg";
	case KGGuiCol_TableRowBgAlt: return "TableRowBgAlt";
	case KGGuiCol_TextSelectedBg: return "TextSelectedBg";
	case KGGuiCol_DragDropTarget: return "DragDropTarget";
	case KGGuiCol_NavHighlight: return "NavHighlight";
	case KGGuiCol_NavWindowingHighlight: return "NavWindowingHighlight";
	case KGGuiCol_NavWindowingDimBg: return "NavWindowingDimBg";
	case KGGuiCol_ModalWindowDimBg: return "ModalWindowDimBg";
	}
	KR_CORE_ASSERT(0, "");
	return "Unknown";
}

//-----------------------------------------------------------------------------
// [SECTION] RENDER HELPERS
// Some of those (internal) functions are currently quite a legacy mess - their signature and behavior will change,
// we need a nicer separation between low-level functions and high-level functions relying on the ImGui context.
// Also see imgui_draw.cpp for some more which have been reworked to not rely on KarmaGui:: context.
//-----------------------------------------------------------------------------

const char* Karma::KarmaGuiInternal::FindRenderedTextEnd(const char* text, const char* text_end)
{
	const char* text_display_end = text;
	if (!text_end)
		text_end = (const char*)-1;

	while (text_display_end < text_end && *text_display_end != '\0' && (text_display_end[0] != '#' || text_display_end[1] != '#'))
		text_display_end++;
	return text_display_end;
}

// Internal ImGui functions to render text
// RenderText***() functions calls KGDrawList::AddText() calls ImBitmapFont::RenderText()
void Karma::KarmaGuiInternal::RenderText(KGVec2 pos, const char* text, const char* text_end, bool hide_text_after_hash)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;

	// Hide anything after a '##' string
	const char* text_display_end;
	if (hide_text_after_hash)
	{
		text_display_end = FindRenderedTextEnd(text, text_end);
	}
	else
	{
		if (!text_end)
			text_end = text + strlen(text); // FIXME-OPT
		text_display_end = text_end;
	}

	if (text != text_display_end)
	{
		window->DrawList->AddText(g.Font, g.FontSize, pos, Karma::KarmaGui::GetColorU32(KGGuiCol_Text), text, text_display_end);
		if (g.LogEnabled)
			LogRenderedText(&pos, text, text_display_end);
	}
}

void Karma::KarmaGuiInternal::RenderTextWrapped(KGVec2 pos, const char* text, const char* text_end, float wrap_width)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;

	if (!text_end)
		text_end = text + strlen(text); // FIXME-OPT

	if (text != text_end)
	{
		window->DrawList->AddText(g.Font, g.FontSize, pos, Karma::KarmaGui::GetColorU32(KGGuiCol_Text), text, text_end, wrap_width);
		if (g.LogEnabled)
			LogRenderedText(&pos, text, text_end);
	}
}

// Default clip_rect uses (pos_min,pos_max)
// Handle clipping on CPU immediately (vs typically let the GPU clip the triangles that are overlapping the clipping rectangle edges)
void Karma::KarmaGuiInternal::RenderTextClippedEx(KGDrawList* draw_list, const KGVec2& pos_min, const KGVec2& pos_max, const char* text, const char* text_display_end, const KGVec2* text_size_if_known, const KGVec2& align, const KGRect* clip_rect)
{
	// Perform CPU side clipping for single clipped element to avoid using scissor state
	KGVec2 pos = pos_min;
	const KGVec2 text_size = text_size_if_known ? *text_size_if_known : Karma::KarmaGui::CalcTextSize(text, text_display_end, false, 0.0f);

	const KGVec2* clip_min = clip_rect ? &clip_rect->Min : &pos_min;
	const KGVec2* clip_max = clip_rect ? &clip_rect->Max : &pos_max;
	bool need_clipping = (pos.x + text_size.x >= clip_max->x) || (pos.y + text_size.y >= clip_max->y);
	if (clip_rect) // If we had no explicit clipping rectangle then pos==clip_min
		need_clipping |= (pos.x < clip_min->x) || (pos.y < clip_min->y);

	// Align whole block. We should defer that to the better rendering function when we'll have support for individual line alignment.
	if (align.x > 0.0f) pos.x = KGMax(pos.x, pos.x + (pos_max.x - pos.x - text_size.x) * align.x);
	if (align.y > 0.0f) pos.y = KGMax(pos.y, pos.y + (pos_max.y - pos.y - text_size.y) * align.y);

	// Render
	if (need_clipping)
	{
		KGVec4 fine_clip_rect(clip_min->x, clip_min->y, clip_max->x, clip_max->y);
		draw_list->AddText(NULL, 0.0f, pos, Karma::KarmaGui::GetColorU32(KGGuiCol_Text), text, text_display_end, 0.0f, &fine_clip_rect);
	}
	else
	{
		draw_list->AddText(NULL, 0.0f, pos, Karma::KarmaGui::GetColorU32(KGGuiCol_Text), text, text_display_end, 0.0f, NULL);
	}
}

void Karma::KarmaGuiInternal::RenderTextClipped(const KGVec2& pos_min, const KGVec2& pos_max, const char* text, const char* text_end, const KGVec2* text_size_if_known, const KGVec2& align, const KGRect* clip_rect)
{
	// Hide anything after a '##' string
	const char* text_display_end = FindRenderedTextEnd(text, text_end);
	const int text_len = (int)(text_display_end - text);
	if (text_len == 0)
		return;

	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	RenderTextClippedEx(window->DrawList, pos_min, pos_max, text, text_display_end, text_size_if_known, align, clip_rect);
	if (g.LogEnabled)
		LogRenderedText(&pos_min, text, text_display_end);
}

// Another overly complex function until we reorganize everything into a nice all-in-one helper.
// This is made more complex because we have dissociated the layout rectangle (pos_min..pos_max) which define _where_ the ellipsis is, from actual clipping of text and limit of the ellipsis display.
// This is because in the context of tabs we selectively hide part of the text when the Close Button appears, but we don't want the ellipsis to move.
void Karma::KarmaGuiInternal::RenderTextEllipsis(KGDrawList* draw_list, const KGVec2& pos_min, const KGVec2& pos_max, float clip_max_x, float ellipsis_max_x, const char* text, const char* text_end_full, const KGVec2* text_size_if_known)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	if (text_end_full == NULL)
		text_end_full = FindRenderedTextEnd(text);
	const KGVec2 text_size = text_size_if_known ? *text_size_if_known : Karma::KarmaGui::CalcTextSize(text, text_end_full, false, 0.0f);

	//draw_list->AddLine(KGVec2(pos_max.x, pos_min.y - 4), KGVec2(pos_max.x, pos_max.y + 4), KG_COL32(0, 0, 255, 255));
	//draw_list->AddLine(KGVec2(ellipsis_max_x, pos_min.y-2), KGVec2(ellipsis_max_x, pos_max.y+2), KG_COL32(0, 255, 0, 255));
	//draw_list->AddLine(KGVec2(clip_max_x, pos_min.y), KGVec2(clip_max_x, pos_max.y), KG_COL32(255, 0, 0, 255));
	// FIXME: We could technically remove (last_glyph->AdvanceX - last_glyph->X1) from text_size.x here and save a few pixels.
	if (text_size.x > pos_max.x - pos_min.x)
	{
		// Hello wo...
		// |       |   |
		// min   max   ellipsis_max
		//          <-> this is generally some padding value

		const KGFont* font = draw_list->_Data->Font;
		const float font_size = draw_list->_Data->FontSize;
		const char* text_end_ellipsis = NULL;

		KGWchar ellipsis_char = font->EllipsisChar;
		int ellipsis_char_count = 1;
		if (ellipsis_char == (KGWchar)-1)
		{
			ellipsis_char = font->DotChar;
			ellipsis_char_count = 3;
		}
		const KGFontGlyph* glyph = font->FindGlyph(ellipsis_char);

		float ellipsis_glyph_width = glyph->X1;                 // Width of the glyph with no padding on either side
		float ellipsis_total_width = ellipsis_glyph_width;      // Full width of entire ellipsis

		if (ellipsis_char_count > 1)
		{
			// Full ellipsis size without free spacing after it.
			const float spacing_between_dots = 1.0f * (draw_list->_Data->FontSize / font->FontSize);
			ellipsis_glyph_width = glyph->X1 - glyph->X0 + spacing_between_dots;
			ellipsis_total_width = ellipsis_glyph_width * (float)ellipsis_char_count - spacing_between_dots;
		}

		// We can now claim the space between pos_max.x and ellipsis_max.x
		const float text_avail_width = KGMax((KGMax(pos_max.x, ellipsis_max_x) - ellipsis_total_width) - pos_min.x, 1.0f);
		float text_size_clipped_x = font->CalcTextSizeA(font_size, text_avail_width, 0.0f, text, text_end_full, &text_end_ellipsis).x;
		if (text == text_end_ellipsis && text_end_ellipsis < text_end_full)
		{
			// Always display at least 1 character if there's no room for character + ellipsis
			text_end_ellipsis = text + KGTextCountUtf8BytesFromChar(text, text_end_full);
			text_size_clipped_x = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, text, text_end_ellipsis).x;
		}
		while (text_end_ellipsis > text && KGCharIsBlankA(text_end_ellipsis[-1]))
		{
			// Trim trailing space before ellipsis (FIXME: Supporting non-ascii blanks would be nice, for this we need a function to backtrack in UTF-8 text)
			text_end_ellipsis--;
			text_size_clipped_x -= font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, text_end_ellipsis, text_end_ellipsis + 1).x; // Ascii blanks are always 1 byte
		}

		// Render text, render ellipsis
		RenderTextClippedEx(draw_list, pos_min, KGVec2(clip_max_x, pos_max.y), text, text_end_ellipsis, &text_size, KGVec2(0.0f, 0.0f));
		float ellipsis_x = pos_min.x + text_size_clipped_x;
		if (ellipsis_x + ellipsis_total_width <= ellipsis_max_x)
			for (int i = 0; i < ellipsis_char_count; i++)
			{
				font->RenderChar(draw_list, font_size, KGVec2(ellipsis_x, pos_min.y), Karma::KarmaGui::GetColorU32(KGGuiCol_Text), ellipsis_char);
				ellipsis_x += ellipsis_glyph_width;
			}
	}
	else
	{
		RenderTextClippedEx(draw_list, pos_min, KGVec2(clip_max_x, pos_max.y), text, text_end_full, &text_size, KGVec2(0.0f, 0.0f));
	}

	if (g.LogEnabled)
		LogRenderedText(&pos_min, text, text_end_full);
}

// Render a rectangle shaped with optional rounding and borders
void Karma::KarmaGuiInternal::RenderFrame(KGVec2 p_min, KGVec2 p_max, KGU32 fill_col, bool border, float rounding)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	window->DrawList->AddRectFilled(p_min, p_max, fill_col, rounding);
	const float border_size = g.Style.FrameBorderSize;
	if (border && border_size > 0.0f)
	{
		window->DrawList->AddRect(p_min + KGVec2(1, 1), p_max + KGVec2(1, 1), Karma::KarmaGui::GetColorU32(KGGuiCol_BorderShadow), rounding, 0, border_size);
		window->DrawList->AddRect(p_min, p_max, Karma::KarmaGui::GetColorU32(KGGuiCol_Border), rounding, 0, border_size);
	}
}

void Karma::KarmaGuiInternal::RenderFrameBorder(KGVec2 p_min, KGVec2 p_max, float rounding)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	const float border_size = g.Style.FrameBorderSize;
	if (border_size > 0.0f)
	{
		window->DrawList->AddRect(p_min + KGVec2(1, 1), p_max + KGVec2(1, 1), Karma::KarmaGui::GetColorU32(KGGuiCol_BorderShadow), rounding, 0, border_size);
		window->DrawList->AddRect(p_min, p_max, Karma::KarmaGui::GetColorU32(KGGuiCol_Border), rounding, 0, border_size);
	}
}

void Karma::KarmaGuiInternal::RenderNavHighlight(const KGRect& bb, KGGuiID id, KGGuiNavHighlightFlags flags)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	if (id != g.NavId)
		return;
	if (g.NavDisableHighlight && !(flags & KGGuiNavHighlightFlags_AlwaysDraw))
		return;
	KGGuiWindow* window = g.CurrentWindow;
	if (window->DC.NavHideHighlightOneFrame)
		return;

	float rounding = (flags & KGGuiNavHighlightFlags_NoRounding) ? 0.0f : g.Style.FrameRounding;
	KGRect display_rect = bb;
	display_rect.ClipWith(window->ClipRect);
	if (flags & KGGuiNavHighlightFlags_TypeDefault)
	{
		const float THICKNESS = 2.0f;
		const float DISTANCE = 3.0f + THICKNESS * 0.5f;
		display_rect.Expand(KGVec2(DISTANCE, DISTANCE));
		bool fully_visible = window->ClipRect.Contains(display_rect);
		if (!fully_visible)
			window->DrawList->PushClipRect(display_rect.Min, display_rect.Max);
		window->DrawList->AddRect(display_rect.Min + KGVec2(THICKNESS * 0.5f, THICKNESS * 0.5f), display_rect.Max - KGVec2(THICKNESS * 0.5f, THICKNESS * 0.5f), Karma::KarmaGui::GetColorU32(KGGuiCol_NavHighlight), rounding, 0, THICKNESS);
		if (!fully_visible)
			window->DrawList->PopClipRect();
	}
	if (flags & KGGuiNavHighlightFlags_TypeThin)
	{
		window->DrawList->AddRect(display_rect.Min, display_rect.Max, Karma::KarmaGui::GetColorU32(KGGuiCol_NavHighlight), rounding, 0, 1.0f);
	}
}

void Karma::KarmaGuiInternal::RenderMouseCursor(KGVec2 base_pos, float base_scale, KarmaGuiMouseCursor mouse_cursor, KGU32 col_fill, KGU32 col_border, KGU32 col_shadow)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KR_CORE_ASSERT(mouse_cursor > KGGuiMouseCursor_None && mouse_cursor < KGGuiMouseCursor_COUNT, "");
	KGFontAtlas* font_atlas = g.DrawListSharedData.Font->ContainerAtlas;
	for (int n = 0; n < g.Viewports.Size; n++)
	{
		// We scale cursor with current viewport/monitor, however Windows 10 for its own hardware cursor seems to be using a different scale factor.
		KGVec2 offset, size, uv[4];
		if (!font_atlas->GetMouseCursorTexData(mouse_cursor, &offset, &size, &uv[0], &uv[2]))
			continue;
		KGGuiViewportP* viewport = g.Viewports[n];
		const KGVec2 pos = base_pos - offset;
		const float scale = base_scale * viewport->DpiScale;
		if (!viewport->GetMainRect().Overlaps(KGRect(pos, pos + KGVec2(size.x + 2, size.y + 2) * scale)))
			continue;
		KGDrawList* draw_list = Karma::KarmaGui::GetForegroundDrawList(viewport);
		KGTextureID tex_id = font_atlas->TexID;
		draw_list->PushTextureID(tex_id);
		draw_list->AddImage(tex_id, pos + KGVec2(1, 0) * scale, pos + (KGVec2(1, 0) + size) * scale, uv[2], uv[3], col_shadow);
		draw_list->AddImage(tex_id, pos + KGVec2(2, 0) * scale, pos + (KGVec2(2, 0) + size) * scale, uv[2], uv[3], col_shadow);
		draw_list->AddImage(tex_id, pos, pos + size * scale, uv[2], uv[3], col_border);
		draw_list->AddImage(tex_id, pos, pos + size * scale, uv[0], uv[1], col_fill);
		draw_list->PopTextureID();
	}
}

//-----------------------------------------------------------------------------
// [SECTION] INITIALIZATION, SHUTDOWN
//-----------------------------------------------------------------------------

// Internal state access - if you want to share Dear ImGui state between modules (e.g. DLL) or allocate it yourself
// Note that we still point to some static data and members (such as GFontAtlas), so the state instance you end up using will point to the static data within its module
KarmaGuiContext* Karma::KarmaGui::GetCurrentContext()
{
	return GKarmaGui;
}

void Karma::KarmaGui::SetCurrentContext(KarmaGuiContext* ctx)
{
	GKarmaGui = ctx;
}

void Karma::KarmaGui::SetAllocatorFunctions(KarmaGuiMemAllocFunc alloc_func, KarmaGuiMemFreeFunc free_func, void* user_data)
{
	KarmaGuiInternal::GImAllocatorAllocFunc = alloc_func;
	KarmaGuiInternal::GImAllocatorFreeFunc = free_func;
	KarmaGuiInternal::GImAllocatorUserData = user_data;
}

// This is provided to facilitate copying allocators from one static/DLL boundary to another (e.g. retrieve default allocator of your executable address space)
void Karma::KarmaGui::GetAllocatorFunctions(KarmaGuiMemAllocFunc* p_alloc_func, KarmaGuiMemFreeFunc* p_free_func, void** p_user_data)
{
	*p_alloc_func = KarmaGuiInternal::GImAllocatorAllocFunc;
	*p_free_func = KarmaGuiInternal::GImAllocatorFreeFunc;
	*p_user_data = KarmaGuiInternal::GImAllocatorUserData;
}

KarmaGuiContext* Karma::KarmaGui::CreateContext(KGFontAtlas* shared_font_atlas)
{
	KarmaGuiContext* prev_ctx = GetCurrentContext();
	KarmaGuiContext* ctx = KG_NEW(KarmaGuiContext)(shared_font_atlas);
	SetCurrentContext(ctx);
	KarmaGuiInternal::Initialize();
	if (prev_ctx != NULL)
		SetCurrentContext(prev_ctx); // Restore previous context if any, else keep new one.
	return ctx;
}

void Karma::KarmaGui::DestroyContext(KarmaGuiContext* ctx)
{
	KarmaGuiContext* prev_ctx = GetCurrentContext();
	if (ctx == NULL) //-V1051
		ctx = prev_ctx;
	SetCurrentContext(ctx);
	KarmaGuiInternal::Shutdown();
	SetCurrentContext((prev_ctx != ctx) ? prev_ctx : NULL);
	KG_DELETE(ctx);
}

// IMPORTANT: ###xxx suffixes must be same in ALL languages
static const KGGuiLocEntry GLocalizationEntriesEnUS[] =
{
	{ ImGuiLocKey_TableSizeOne,         "Size column to fit###SizeOne"          },
	{ ImGuiLocKey_TableSizeAllFit,      "Size all columns to fit###SizeAll"     },
	{ ImGuiLocKey_TableSizeAllDefault,  "Size all columns to default###SizeAll" },
	{ ImGuiLocKey_TableResetOrder,      "Reset order###ResetOrder"              },
	{ ImGuiLocKey_WindowingMainMenuBar, "(Main menu bar)"                       },
	{ ImGuiLocKey_WindowingPopup,       "(Popup)"                               },
	{ ImGuiLocKey_WindowingUntitled,    "(Untitled)"                            },
	{ ImGuiLocKey_DockingHideTabBar,    "Hide tab bar###HideTabBar"             },
};

void Karma::KarmaGuiInternal::Initialize()
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(!g.Initialized && !g.SettingsLoaded, "");

	// Add .ini handle for KGGuiWindow and KGGuiTable types
	{
		KGGuiSettingsHandler ini_handler;
		ini_handler.TypeName = "Window";
		ini_handler.TypeHash = KGHashStr("Window");
		ini_handler.ClearAllFn = WindowSettingsHandler_ClearAll;
		ini_handler.ReadOpenFn = WindowSettingsHandler_ReadOpen;
		ini_handler.ReadLineFn = WindowSettingsHandler_ReadLine;
		ini_handler.ApplyAllFn = WindowSettingsHandler_ApplyAll;
		ini_handler.WriteAllFn = WindowSettingsHandler_WriteAll;
		AddSettingsHandler(&ini_handler);
	}
	TableSettingsAddSettingsHandler();

	// Setup default localization table
	LocalizeRegisterEntries(GLocalizationEntriesEnUS, KG_ARRAYSIZE(GLocalizationEntriesEnUS));

	// Create default viewport
	KGGuiViewportP* viewport = KG_NEW(KGGuiViewportP)();
	viewport->ID = KarmaGuiInternal::IMGUI_VIEWPORT_DEFAULT_ID;
	viewport->Idx = 0;
	viewport->PlatformWindowCreated = true;
	viewport->Flags = KGGuiViewportFlags_OwnedByApp;
	g.Viewports.push_back(viewport);
	g.TempBuffer.resize(1024 * 3 + 1, 0);
	g.PlatformIO.Viewports.push_back(g.Viewports[0]);

#ifdef KARMAGUI_HAS_DOCK
	// Initialize Docking
	DockContextInitialize(&g);
#endif

	g.Initialized = true;
}

// This function is merely here to free heap allocations.
void Karma::KarmaGuiInternal::Shutdown()
{
	// The fonts atlas can be used prior to calling NewFrame(), so we clear it even if g.Initialized is FALSE (which would happen if we never called NewFrame)
	KarmaGuiContext& g = *GKarmaGui;
	if (g.IO.Fonts && g.FontAtlasOwnedByContext)
	{
		g.IO.Fonts->Locked = false;
		KG_DELETE(g.IO.Fonts);
	}
	g.IO.Fonts = NULL;
	g.DrawListSharedData.TempBuffer.clear();

	// Cleanup of other data are conditional on actually having initialized Dear ImGui.
	if (!g.Initialized)
		return;

	// Save settings (unless we haven't attempted to load them: CreateContext/DestroyContext without a call to NewFrame shouldn't save an empty file)
	if (g.SettingsLoaded && g.IO.IniFilename != NULL)
		KarmaGui::SaveIniSettingsToDisk(g.IO.IniFilename);

	// Destroy platform windows
	KarmaGui::DestroyPlatformWindows();

	// Shutdown extensions
	DockContextShutdown(&g);

	CallContextHooks(&g, ImGuiContextHookType_Shutdown);

	// Clear everything else
	g.Windows.clear_delete();
	g.WindowsFocusOrder.clear();
	g.WindowsTempSortBuffer.clear();
	g.CurrentWindow = NULL;
	g.CurrentWindowStack.clear();
	g.WindowsById.Clear();
	g.NavWindow = NULL;
	g.HoveredWindow = g.HoveredWindowUnderMovingWindow = NULL;
	g.ActiveIdWindow = g.ActiveIdPreviousFrameWindow = NULL;
	g.MovingWindow = NULL;

	g.KeysRoutingTable.Clear();

	g.ColorStack.clear();
	g.StyleVarStack.clear();
	g.FontStack.clear();
	g.OpenPopupStack.clear();
	g.BeginPopupStack.clear();

	g.CurrentViewport = g.MouseViewport = g.MouseLastHoveredViewport = NULL;
	g.Viewports.clear_delete();

	g.TabBars.Clear();
	g.CurrentTabBarStack.clear();
	g.ShrinkWidthBuffer.clear();

	g.ClipperTempData.clear_destruct();

	g.Tables.Clear();
	g.TablesTempData.clear_destruct();
	g.DrawChannelsTempMergeBuffer.clear();

	g.ClipboardHandlerData.clear();
	g.MenusIdSubmittedThisFrame.clear();
	g.InputTextState.ClearFreeMemory();

	g.SettingsWindows.clear();
	g.SettingsHandlers.clear();

	if (g.LogFile)
	{
#ifndef IMGUI_DISABLE_TTY_FUNCTIONS
		if (g.LogFile != stdout)
#endif
			KGFileClose(g.LogFile);
		g.LogFile = NULL;
	}
	g.LogBuffer.clear();
	g.DebugLogBuf.clear();
	g.DebugLogIndex.clear();

	g.Initialized = false;
}

// No specific ordering/dependency support, will see as needed
KGGuiID Karma::KarmaGuiInternal::AddContextHook(KarmaGuiContext* ctx, const KGGuiContextHook* hook)
{
	KarmaGuiContext& g = *ctx;
	KR_CORE_ASSERT(hook->Callback != NULL && hook->HookId == 0 && hook->Type != ImGuiContextHookType_PendingRemoval_, "");
	g.Hooks.push_back(*hook);
	g.Hooks.back().HookId = ++g.HookIdNext;
	return g.HookIdNext;
}

// Deferred removal, avoiding issue with changing vector while iterating it
void Karma::KarmaGuiInternal::RemoveContextHook(KarmaGuiContext* ctx, KGGuiID hook_id)
{
	KarmaGuiContext& g = *ctx;
	KR_CORE_ASSERT(hook_id != 0, "");
	for (int n = 0; n < g.Hooks.Size; n++)
		if (g.Hooks[n].HookId == hook_id)
			g.Hooks[n].Type = ImGuiContextHookType_PendingRemoval_;
}

// Call context hooks (used by e.g. test engine)
// We assume a small number of hooks so all stored in same array
void Karma::KarmaGuiInternal::CallContextHooks(KarmaGuiContext* ctx, ImGuiContextHookType hook_type)
{
	KarmaGuiContext& g = *ctx;
	for (int n = 0; n < g.Hooks.Size; n++)
		if (g.Hooks[n].Type == hook_type)
			g.Hooks[n].Callback(&g, &g.Hooks[n]);
}

//-----------------------------------------------------------------------------
// [SECTION] MAIN CODE (most of the code! lots of stuff, needs tidying up!)
//-----------------------------------------------------------------------------

// KGGuiWindow is mostly a dumb struct. It merely has a constructor and a few helper methods
KGGuiWindow::KGGuiWindow(KarmaGuiContext* context, const char* name) : DrawListInst(NULL)
{
	memset(this, 0, sizeof(*this));
	Name = KGStrdup(name);
	NameBufLen = (int)strlen(name) + 1;
	ID = KGHashStr(name);
	IDStack.push_back(ID);
	ViewportAllowPlatformMonitorExtend = -1;
	ViewportPos = KGVec2(FLT_MAX, FLT_MAX);
	MoveId = GetID("#MOVE");
	TabId = GetID("#TAB");
	ScrollTarget = KGVec2(FLT_MAX, FLT_MAX);
	ScrollTargetCenterRatio = KGVec2(0.5f, 0.5f);
	AutoFitFramesX = AutoFitFramesY = -1;
	AutoPosLastDirection = KGGuiDir_None;
	SetWindowPosAllowFlags = SetWindowSizeAllowFlags = SetWindowCollapsedAllowFlags = SetWindowDockAllowFlags = KGGuiCond_Always | KGGuiCond_Once | KGGuiCond_FirstUseEver | KGGuiCond_Appearing;
	SetWindowPosVal = SetWindowPosPivot = KGVec2(FLT_MAX, FLT_MAX);
	LastFrameActive = -1;
	LastFrameJustFocused = -1;
	LastTimeActive = -1.0f;
	FontWindowScale = FontDpiScale = 1.0f;
	SettingsOffset = -1;
	DockOrder = -1;
	DrawList = &DrawListInst;
	DrawList->_Data = &context->DrawListSharedData;
	DrawList->_OwnerName = Name;
	bUseCustomBgColor = false;
	KG_PLACEMENT_NEW(&WindowClass) KarmaGuiWindowClass();
}

KGGuiWindow::~KGGuiWindow()
{
	KR_CORE_ASSERT(DrawList == &DrawListInst, "");
	KG_DELETE(Name);
	ColumnsStorage.clear_destruct();
}

KGGuiID KGGuiWindow::GetID(const char* str, const char* str_end)
{
	KGGuiID seed = IDStack.back();
	KGGuiID id = KGHashStr(str, str_end ? (str_end - str) : 0, seed);
	KarmaGuiContext& g = *Karma::GKarmaGui;
	if (g.DebugHookIdInfo == id)
		Karma::KarmaGuiInternal::DebugHookIdInfo(id, KGGuiDataType_String, str, str_end);
	return id;
}

float KGGuiWindow::TitleBarHeight() const
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	return (Flags & KGGuiWindowFlags_NoTitleBar) ? 0.0f : CalcFontSize() + g.Style.FramePadding.y * 2.0f;
}

float KGGuiWindow::CalcFontSize() const
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	float scale = g.FontBaseSize * FontWindowScale * FontDpiScale; if (ParentWindow) scale *= ParentWindow->FontWindowScale;
	return scale;
}

float KGGuiWindow::MenuBarHeight() const
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	return (Flags & KGGuiWindowFlags_MenuBar) ? DC.MenuBarOffset.y + CalcFontSize() + g.Style.FramePadding.y * 2.0f : 0.0f;
}

KGGuiID KGGuiWindow::GetID(const void* ptr)
{
	KGGuiID seed = IDStack.back();
	KGGuiID id = KGHashData(&ptr, sizeof(void*), seed);
	KarmaGuiContext& g = *Karma::GKarmaGui;
	if (g.DebugHookIdInfo == id)
		Karma::KarmaGuiInternal::DebugHookIdInfo(id, KGGuiDataType_Pointer, ptr, NULL);
	return id;
}

KGGuiID KGGuiWindow::GetID(int n)
{
	KGGuiID seed = IDStack.back();
	KGGuiID id = KGHashData(&n, sizeof(n), seed);
	KarmaGuiContext& g = *Karma::GKarmaGui;
	if (g.DebugHookIdInfo == id)
		Karma::KarmaGuiInternal::DebugHookIdInfo(id, KGGuiDataType_S32, (void*)(intptr_t)n, NULL);
	return id;
}

// This is only used in rare/specific situations to manufacture an ID out of nowhere.
KGGuiID KGGuiWindow::GetIDFromRectangle(const KGRect& r_abs)
{
	KGGuiID seed = IDStack.back();
	KGRect r_rel = Karma::KarmaGuiInternal::WindowRectAbsToRel(this, r_abs);
	KGGuiID id = KGHashData(&r_rel, sizeof(r_rel), seed);
	return id;
}

static void SetCurrentWindow(KGGuiWindow* window)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	g.CurrentWindow = window;
	g.CurrentTable = window && window->DC.CurrentTableIdx != -1 ? g.Tables.GetByIndex(window->DC.CurrentTableIdx) : NULL;
	if (window)
		g.FontSize = g.DrawListSharedData.FontSize = window->CalcFontSize();
}

void Karma::KarmaGuiInternal::GcCompactTransientMiscBuffers()
{
	KarmaGuiContext& g = *GKarmaGui;
	g.ItemFlagsStack.clear();
	g.GroupStack.clear();
	TableGcCompactSettings();
}

// Free up/compact internal window buffers, we can use this when a window becomes unused.
// Not freed:
// - KGGuiWindow, KGGuiWindowSettings, Name, StateStorage, ColumnsStorage (may hold useful data)
// This should have no noticeable visual effect. When the window reappear however, expect new allocation/buffer growth/copy cost.
void Karma::KarmaGuiInternal::GcCompactTransientWindowBuffers(KGGuiWindow* window)
{
	window->MemoryCompacted = true;
	window->MemoryDrawListIdxCapacity = window->DrawList->IdxBuffer.Capacity;
	window->MemoryDrawListVtxCapacity = window->DrawList->VtxBuffer.Capacity;
	window->IDStack.clear();
	window->DrawList->_ClearFreeMemory();
	window->DC.ChildWindows.clear();
	window->DC.ItemWidthStack.clear();
	window->DC.TextWrapPosStack.clear();
}

void Karma::KarmaGuiInternal::GcAwakeTransientWindowBuffers(KGGuiWindow* window)
{
	// We stored capacity of the KGDrawList buffer to reduce growth-caused allocation/copy when awakening.
	// The other buffers tends to amortize much faster.
	window->MemoryCompacted = false;
	window->DrawList->IdxBuffer.reserve(window->MemoryDrawListIdxCapacity);
	window->DrawList->VtxBuffer.reserve(window->MemoryDrawListVtxCapacity);
	window->MemoryDrawListIdxCapacity = window->MemoryDrawListVtxCapacity = 0;
}

KGGuiKeyOwnerData* Karma::KarmaGuiInternal::GetKeyOwnerData(KarmaGuiKey key)
{
	if (key & KGGuiMod_Mask_) key = ConvertSingleModFlagToKey(key);
	KR_CORE_ASSERT(IsNamedKey(key), "");
	return &Karma::GKarmaGui->KeysOwnerData[key - KGGuiKey_NamedKey_BEGIN];
}

KarmaGuiKeyChord  Karma::KarmaGuiInternal::ConvertShortcutMod(KarmaGuiKeyChord key_chord)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KR_CORE_ASSERT(key_chord & KGGuiMod_Shortcut, "");
	return (key_chord & ~KGGuiMod_Shortcut) | (g.IO.ConfigMacOSXBehaviors ? KGGuiMod_Super : KGGuiMod_Ctrl);
}

KarmaGuiKey Karma::KarmaGuiInternal::ConvertSingleModFlagToKey(KarmaGuiKey key)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	if (key == KGGuiMod_Ctrl) return KGGuiKey_ReservedForModCtrl;
	if (key == KGGuiMod_Shift) return KGGuiKey_ReservedForModShift;
	if (key == KGGuiMod_Alt) return KGGuiKey_ReservedForModAlt;
	if (key == KGGuiMod_Super) return KGGuiKey_ReservedForModSuper;
	if (key == KGGuiMod_Shortcut) return (g.IO.ConfigMacOSXBehaviors ? KGGuiKey_ReservedForModSuper : KGGuiKey_ReservedForModCtrl);
	return key;
}

KGGuiWindow* Karma::KarmaGuiInternal::GetCurrentWindowRead()
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	return g.CurrentWindow;
}

void Karma::KarmaGuiInternal::SetActiveID(KGGuiID id, KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;

	// While most behaved code would make an effort to not steal active id during window move/drag operations,
	// we at least need to be resilient to it. Cancelling the move is rather aggressive and users of 'master' branch
	// may prefer the weird ill-defined half working situation ('docking' did assert), so may need to rework that.
	if (g.MovingWindow != NULL && g.ActiveId == g.MovingWindow->MoveId)
	{
		KR_CORE_INFO("SetActiveID() cancel MovingWindow");
		g.MovingWindow = NULL;
	}

	// Set active id
	g.ActiveIdIsJustActivated = (g.ActiveId != id);
	if (g.ActiveIdIsJustActivated)
	{
		// Need to understand the log meaning
		KR_CORE_INFO("SetActiveID() old:{0} (window \"{1}\") -> new:{2}X (window \"{3}\")", g.ActiveId, g.ActiveIdWindow ? g.ActiveIdWindow->Name : "", id, window ? window->Name : "");
		g.ActiveIdTimer = 0.0f;
		g.ActiveIdHasBeenPressedBefore = false;
		g.ActiveIdHasBeenEditedBefore = false;
		g.ActiveIdMouseButton = -1;
		if (id != 0)
		{
			g.LastActiveId = id;
			g.LastActiveIdTimer = 0.0f;
		}
	}
	g.ActiveId = id;
	g.ActiveIdAllowOverlap = false;
	g.ActiveIdNoClearOnFocusLoss = false;
	g.ActiveIdWindow = window;
	g.ActiveIdHasBeenEditedThisFrame = false;
	if (id)
	{
		g.ActiveIdIsAlive = id;
		g.ActiveIdSource = (g.NavActivateId == id || g.NavActivateInputId == id || g.NavJustMovedToId == id) ? (KGGuiInputSource)KGGuiInputSource_Nav : KGGuiInputSource_Mouse;
	}

	// Clear declaration of inputs claimed by the widget
	// (Please note that this is WIP and not all keys/inputs are thoroughly declared by all widgets yet)
	g.ActiveIdUsingNavDirMask = 0x00;
	g.ActiveIdUsingAllKeyboardKeys = false;
#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
	g.ActiveIdUsingNavInputMask = 0x00;
#endif
}

void Karma::KarmaGuiInternal::ClearActiveID()
{
	SetActiveID(0, NULL); // g.ActiveId = 0;
}

void Karma::KarmaGuiInternal::SetHoveredID(KGGuiID id)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.HoveredId = id;
	g.HoveredIdAllowOverlap = false;
	if (id != 0 && g.HoveredIdPreviousFrame != id)
		g.HoveredIdTimer = g.HoveredIdNotActiveTimer = 0.0f;
}

KGGuiID Karma::KarmaGuiInternal::GetHoveredID()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.HoveredId ? g.HoveredId : g.HoveredIdPreviousFrame;
}

// This is called by ItemAdd().
// Code not using ItemAdd() may need to call this manually otherwise ActiveId will be cleared. In IMGUI_VERSION_NUM < 18717 this was called by GetID().
void Karma::KarmaGuiInternal::KeepAliveID(KGGuiID id)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.ActiveId == id)
		g.ActiveIdIsAlive = id;
	if (g.ActiveIdPreviousFrame == id)
		g.ActiveIdPreviousFrameIsAlive = true;
}

void Karma::KarmaGuiInternal::MarkItemEdited(KGGuiID id)
{
	// This marking is solely to be able to provide info for IsItemDeactivatedAfterEdit().
	// ActiveId might have been released by the time we call this (as in the typical press/release button behavior) but still need to fill the data.
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(g.ActiveId == id || g.ActiveId == 0 || g.DragDropActive, "");
	KG_UNUSED(id); // Avoid unused variable warnings when asserts are compiled out.
	//KR_CORE_ASSERT(g.CurrentWindow->DC.LastItemId == id);
	g.ActiveIdHasBeenEditedThisFrame = true;
	g.ActiveIdHasBeenEditedBefore = true;
	g.LastItemData.StatusFlags |= KGGuiItemStatusFlags_Edited;
}

static inline bool IsWindowContentHoverable(KGGuiWindow* window, KarmaGuiHoveredFlags flags)
{
	// An active popup disable hovering on other windows (apart from its own children)
	// FIXME-OPT: This could be cached/stored within the window.
	KarmaGuiContext& g = *Karma::GKarmaGui;
	if (g.NavWindow)
		if (KGGuiWindow* focused_root_window = g.NavWindow->RootWindowDockTree)
			if (focused_root_window->WasActive && focused_root_window != window->RootWindowDockTree)
			{
				// For the purpose of those flags we differentiate "standard popup" from "modal popup"
				// NB: The 'else' is important because Modal windows are also Popups.
				bool want_inhibit = false;
				if (focused_root_window->Flags & KGGuiWindowFlags_Modal)
					want_inhibit = true;
				else if ((focused_root_window->Flags & KGGuiWindowFlags_Popup) && !(flags & KGGuiHoveredFlags_AllowWhenBlockedByPopup))
					want_inhibit = true;

				// Inhibit hover unless the window is within the stack of our modal/popup
				if (want_inhibit)
					if (!Karma::KarmaGuiInternal::IsWindowWithinBeginStackOf(window->RootWindow, focused_root_window))
						return false;
			}

	// Filter by viewport
	if (window->Viewport != g.MouseViewport)
		if (g.MovingWindow == NULL || window->RootWindowDockTree != g.MovingWindow->RootWindowDockTree)
			return false;

	return true;
}

// This is roughly matching the behavior of internal-facing ItemHoverable()
// - we allow hovering to be true when ActiveId==window->MoveID, so that clicking on non-interactive items such as a Text() item still returns true with IsItemHovered()
// - this should work even for non-interactive items that have no ID, so we cannot use LastItemId
bool Karma::KarmaGui::IsItemHovered(KarmaGuiHoveredFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	if (g.NavDisableMouseHover && !g.NavDisableHighlight && !(flags & KGGuiHoveredFlags_NoNavOverride))
	{
		if ((g.LastItemData.InFlags & KGGuiItemFlags_Disabled) && !(flags & KGGuiHoveredFlags_AllowWhenDisabled))
			return false;
		if (!IsItemFocused())
			return false;
	}
	else
	{
		// Test for bounding box overlap, as updated as ItemAdd()
		KGGuiItemStatusFlags status_flags = g.LastItemData.StatusFlags;
		if (!(status_flags & KGGuiItemStatusFlags_HoveredRect))
			return false;
		KR_CORE_ASSERT((flags & (KGGuiHoveredFlags_AnyWindow | KGGuiHoveredFlags_RootWindow | KGGuiHoveredFlags_ChildWindows | KGGuiHoveredFlags_NoPopupHierarchy | KGGuiHoveredFlags_DockHierarchy)) == 0, "");   // Flags not supported by this function

		// Done with rectangle culling so we can perform heavier checks now
		// Test if we are hovering the right window (our window could be behind another window)
		// [2021/03/02] Reworked / reverted the revert, finally. Note we want e.g. BeginGroup/ItemAdd/EndGroup to work as well. (#3851)
		// [2017/10/16] Reverted commit 344d48be3 and testing RootWindow instead. I believe it is correct to NOT test for RootWindow but this leaves us unable
		// to use IsItemHovered() after EndChild() itself. Until a solution is found I believe reverting to the test from 2017/09/27 is safe since this was
		// the test that has been running for a long while.
		if (g.HoveredWindow != window && (status_flags & KGGuiItemStatusFlags_HoveredWindow) == 0)
			if ((flags & KGGuiHoveredFlags_AllowWhenOverlapped) == 0)
				return false;

		// Test if another item is active (e.g. being dragged)
		if ((flags & KGGuiHoveredFlags_AllowWhenBlockedByActiveItem) == 0)
			if (g.ActiveId != 0 && g.ActiveId != g.LastItemData.ID && !g.ActiveIdAllowOverlap)
				if (g.ActiveId != window->MoveId && g.ActiveId != window->TabId)
					return false;

		// Test if interactions on this window are blocked by an active popup or modal.
		// The KGGuiHoveredFlags_AllowWhenBlockedByPopup flag will be tested here.
		if (!IsWindowContentHoverable(window, flags) && !(g.LastItemData.InFlags & KGGuiItemFlags_NoWindowHoverableCheck))
			return false;

		// Test if the item is disabled
		if ((g.LastItemData.InFlags & KGGuiItemFlags_Disabled) && !(flags & KGGuiHoveredFlags_AllowWhenDisabled))
			return false;

		// Special handling for calling after Begin() which represent the title bar or tab.
		// When the window is skipped/collapsed (SkipItems==true) that last item (always ->MoveId submitted by Begin)
		// will never be overwritten so we need to detect the case.
		if (g.LastItemData.ID == window->MoveId && window->WriteAccessed)
			return false;
	}

	// Handle hover delay
	// (some ideas: https://www.nngroup.com/articles/timing-exposing-content)
	float delay;
	if (flags & KGGuiHoveredFlags_DelayNormal)
		delay = g.IO.HoverDelayNormal;
	else if (flags & KGGuiHoveredFlags_DelayShort)
		delay = g.IO.HoverDelayShort;
	else
		delay = 0.0f;
	if (delay > 0.0f)
	{
		KGGuiID hover_delay_id = (g.LastItemData.ID != 0) ? g.LastItemData.ID : window->GetIDFromRectangle(g.LastItemData.Rect);
		if ((flags & KGGuiHoveredFlags_NoSharedDelay) && (g.HoverDelayIdPreviousFrame != hover_delay_id))
			g.HoverDelayTimer = 0.0f;
		g.HoverDelayId = hover_delay_id;
		return g.HoverDelayTimer >= delay;
	}

	return true;
}

// Internal facing ItemHoverable() used when submitting widgets. Differs slightly from IsItemHovered().
bool Karma::KarmaGuiInternal::ItemHoverable(const KGRect& bb, KGGuiID id)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.HoveredId != 0 && g.HoveredId != id && !g.HoveredIdAllowOverlap)
		return false;

	KGGuiWindow* window = g.CurrentWindow;
	if (g.HoveredWindow != window)
		return false;
	if (g.ActiveId != 0 && g.ActiveId != id && !g.ActiveIdAllowOverlap)
		return false;
	if (!KarmaGui::IsMouseHoveringRect(bb.Min, bb.Max))
		return false;

	// Done with rectangle culling so we can perform heavier checks now.
	KGGuiItemFlags item_flags = (g.LastItemData.ID == id ? g.LastItemData.InFlags : g.CurrentItemFlags);
	if (!(item_flags & KGGuiItemFlags_NoWindowHoverableCheck) && !IsWindowContentHoverable(window, KGGuiHoveredFlags_None))
	{
		g.HoveredIdDisabled = true;
		return false;
	}

	// We exceptionally allow this function to be called with id==0 to allow using it for easy high-level
	// hover test in widgets code. We could also decide to split this function is two.
	if (id != 0)
		SetHoveredID(id);

	// When disabled we'll return false but still set HoveredId
	if (item_flags & KGGuiItemFlags_Disabled)
	{
		// Release active id if turning disabled
		if (g.ActiveId == id)
			ClearActiveID();
		g.HoveredIdDisabled = true;
		return false;
	}

	if (id != 0)
	{
		// [DEBUG] Item Picker tool!
		// We perform the check here because SetHoveredID() is not frequently called (1~ time a frame), making
		// the cost of this tool near-zero. We can get slightly better call-stack and support picking non-hovered
		// items if we performed the test in ItemAdd(), but that would incur a small runtime cost.
		if (g.DebugItemPickerActive && g.HoveredIdPreviousFrame == id)
		{
			KarmaGui::GetForegroundDrawList()->AddRect(bb.Min, bb.Max, KG_COL32(255, 255, 0, 255));
		}
		if (g.DebugItemPickerBreakId == id)
		{
			KR_CORE_ASSERT(0, "");
		}
	}

	if (g.NavDisableMouseHover)
		return false;

	return true;
}

// FIXME: This is inlined/duplicated in ItemAdd()
bool Karma::KarmaGuiInternal::IsClippedEx(const KGRect& bb, KGGuiID id)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	if (!bb.Overlaps(window->ClipRect))
		if (id == 0 || (id != g.ActiveId && id != g.NavId))
			if (!g.LogEnabled)
				return true;
	return false;
}

// This is also inlined in ItemAdd()
// Note: if KGGuiItemStatusFlags_HasDisplayRect is set, user needs to set window->DC.LastItemDisplayRect!
void Karma::KarmaGuiInternal::SetLastItemData(KGGuiID item_id, KGGuiItemFlags in_flags, KGGuiItemStatusFlags item_flags, const KGRect& item_rect)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.LastItemData.ID = item_id;
	g.LastItemData.InFlags = in_flags;
	g.LastItemData.StatusFlags = item_flags;
	g.LastItemData.Rect = item_rect;
}

float Karma::KarmaGuiInternal::CalcWrapWidthForPos(const KGVec2& pos, float wrap_pos_x)
{
	if (wrap_pos_x < 0.0f)
		return 0.0f;

	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	if (wrap_pos_x == 0.0f)
	{
		// We could decide to setup a default wrapping max point for auto-resizing windows,
		// or have auto-wrap (with unspecified wrapping pos) behave as a ContentSize extending function?
		//if (window->Hidden && (window->Flags & KGGuiWindowFlags_AlwaysAutoResize))
		//    wrap_pos_x = KGMax(window->WorkRect.Min.x + g.FontSize * 10.0f, window->WorkRect.Max.x);
		//else
		wrap_pos_x = window->WorkRect.Max.x;
	}
	else if (wrap_pos_x > 0.0f)
	{
		wrap_pos_x += window->Pos.x - window->Scroll.x; // wrap_pos_x is provided is window local space
	}

	return KGMax(wrap_pos_x - pos.x, 1.0f);
}

// KG_ALLOC() == KarmaGui::MemAlloc()
// May need to hook up with Karma
void* Karma::KarmaGui::MemAlloc(size_t size)
{
	if (KarmaGuiContext* ctx = GKarmaGui)
		ctx->IO.MetricsActiveAllocations++;
	return (*KarmaGuiInternal::GImAllocatorAllocFunc)(size, KarmaGuiInternal::GImAllocatorUserData);
}

// KG_FREE() == KarmaGui::MemFree()
void Karma::KarmaGui::MemFree(void* ptr)
{
	if (ptr)
		if (KarmaGuiContext* ctx = GKarmaGui)
			ctx->IO.MetricsActiveAllocations--;
	return (*KarmaGuiInternal::GImAllocatorFreeFunc)(ptr, KarmaGuiInternal::GImAllocatorUserData);
}

const char* Karma::KarmaGui::GetClipboardText()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.IO.GetClipboardTextFn ? g.IO.GetClipboardTextFn(g.IO.ClipboardUserData) : "";
}

void Karma::KarmaGui::SetClipboardText(const char* text)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.IO.SetClipboardTextFn)
		g.IO.SetClipboardTextFn(g.IO.ClipboardUserData, text);
}

KarmaGuiIO& Karma::KarmaGui::GetIO()
{
	KR_CORE_ASSERT(GKarmaGui != NULL, "No current context. Did you call KarmaGui::CreateContext() and KarmaGui::SetCurrentContext() ?");
	return GKarmaGui->IO;
}

KarmaGuiPlatformIO& Karma::KarmaGui::GetPlatformIO()
{
	KR_CORE_ASSERT(GKarmaGui != NULL, "No current context. Did you call KarmaGui::CreateContext() or KarmaGui::SetCurrentContext()?");
	return GKarmaGui->PlatformIO;
}

// Pass this to your backend rendering function! Valid after Render() and until the next call to NewFrame()
KGDrawData* Karma::KarmaGui::GetDrawData()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiViewportP* viewport = g.Viewports[0];
	return viewport->DrawDataP.Valid ? &viewport->DrawDataP : NULL;
}

double Karma::KarmaGui::GetTime()
{
	return GKarmaGui->Time;
}

int Karma::KarmaGui::GetFrameCount()
{
	return GKarmaGui->FrameCount;
}

static KGDrawList* GetViewportDrawList(KGGuiViewportP* viewport, size_t drawlist_no, const char* drawlist_name)
{
	// Create the draw list on demand, because they are not frequently used for all viewports
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KR_CORE_ASSERT(drawlist_no < KG_ARRAYSIZE(viewport->DrawLists), "");
	KGDrawList* draw_list = viewport->DrawLists[drawlist_no];
	if (draw_list == NULL)
	{
		draw_list = KG_NEW(KGDrawList)(&g.DrawListSharedData);
		draw_list->_OwnerName = drawlist_name;
		viewport->DrawLists[drawlist_no] = draw_list;
	}

	// Our KGDrawList system requires that there is always a command
	if (viewport->DrawListsLastFrame[drawlist_no] != g.FrameCount)
	{
		draw_list->_ResetForNewFrame();
		draw_list->PushTextureID(g.IO.Fonts->TexID);
		draw_list->PushClipRect(viewport->Pos, viewport->Pos + viewport->Size, false);
		viewport->DrawListsLastFrame[drawlist_no] = g.FrameCount;
	}
	return draw_list;
}

KGDrawList* Karma::KarmaGui::GetBackgroundDrawList(KarmaGuiViewport* viewport)
{
	return GetViewportDrawList((KGGuiViewportP*)viewport, 0, "##Background");
}

KGDrawList* Karma::KarmaGui::GetBackgroundDrawList()
{
	KarmaGuiContext& g = *GKarmaGui;
	return GetBackgroundDrawList(g.CurrentWindow->Viewport);
}

KGDrawList* Karma::KarmaGui::GetForegroundDrawList(KarmaGuiViewport* viewport)
{
	return GetViewportDrawList((KGGuiViewportP*)viewport, 1, "##Foreground");
}

KGDrawList* Karma::KarmaGui::GetForegroundDrawList()
{
	KarmaGuiContext& g = *GKarmaGui;
	return GetForegroundDrawList(g.CurrentWindow->Viewport);
}

KGDrawListSharedData* Karma::KarmaGui::GetDrawListSharedData()
{
	return &GKarmaGui->DrawListSharedData;
}

void Karma::KarmaGuiInternal::StartMouseMovingWindow(KGGuiWindow* window)
{
	// Set ActiveId even if the _NoMove flag is set. Without it, dragging away from a window with _NoMove would activate hover on other windows.
	// We _also_ call this when clicking in a window empty space when io.ConfigWindowsMoveFromTitleBarOnly is set, but clear g.MovingWindow afterward.
	// This is because we want ActiveId to be set even when the window is not permitted to move.
	KarmaGuiContext& g = *GKarmaGui;
	FocusWindow(window);
	SetActiveID(window->MoveId, window);
	g.NavDisableHighlight = true;
	g.ActiveIdClickOffset = g.IO.MouseClickedPos[0] - window->RootWindowDockTree->Pos;
	g.ActiveIdNoClearOnFocusLoss = true;
	SetActiveIdUsingAllKeyboardKeys();

	bool can_move_window = true;
	if ((window->Flags & KGGuiWindowFlags_NoMove) || (window->RootWindowDockTree->Flags & KGGuiWindowFlags_NoMove))
		can_move_window = false;
	if (KGGuiDockNode* node = window->DockNodeAsHost)
		if (node->VisibleWindow && (node->VisibleWindow->Flags & KGGuiWindowFlags_NoMove))
			can_move_window = false;
	if (can_move_window)
		g.MovingWindow = window;
}

// We use 'undock_floating_node == false' when dragging from title bar to allow moving groups of floating nodes without undocking them.
// - undock_floating_node == true: when dragging from a floating node within a hierarchy, always undock the node.
// - undock_floating_node == false: when dragging from a floating node within a hierarchy, move root window.
void Karma::KarmaGuiInternal::StartMouseMovingWindowOrNode(KGGuiWindow* window, KGGuiDockNode* node, bool undock_floating_node)
{
	KarmaGuiContext& g = *GKarmaGui;
	bool can_undock_node = false;
	if (node != NULL && node->VisibleWindow && (node->VisibleWindow->Flags & KGGuiWindowFlags_NoMove) == 0)
	{
		// Can undock if:
		// - part of a floating node hierarchy with more than one visible node (if only one is visible, we'll just move the whole hierarchy)
		// - part of a dockspace node hierarchy (trivia: undocking from a fixed/central node will create a new node and copy windows)
		KGGuiDockNode* root_node = DockNodeGetRootNode(node);
		if (root_node->OnlyNodeWithWindows != node || root_node->CentralNode != NULL)   // -V1051 PVS-Studio thinks node should be root_node and is wrong about that.
			if (undock_floating_node || root_node->IsDockSpace())
				can_undock_node = true;
	}

	const bool clicked = KarmaGui::IsMouseClicked(0);
	const bool dragging = KarmaGui::IsMouseDragging(0, g.IO.MouseDragThreshold * 1.70f);
	if (can_undock_node && dragging)
		DockContextQueueUndockNode(&g, node); // Will lead to DockNodeStartMouseMovingWindow() -> StartMouseMovingWindow() being called next frame
	else if (!can_undock_node && (clicked || dragging) && g.MovingWindow != window)
		StartMouseMovingWindow(window);
}

// Handle mouse moving window
// Note: moving window with the navigation keys (Square + d-pad / CTRL+TAB + Arrows) are processed in NavUpdateWindowing()
// FIXME: We don't have strong guarantee that g.MovingWindow stay synched with g.ActiveId == g.MovingWindow->MoveId.
// This is currently enforced by the fact that BeginDragDropSource() is setting all g.ActiveIdUsingXXXX flags to inhibit navigation inputs,
// but if we should more thoroughly test cases where g.ActiveId or g.MovingWindow gets changed and not the other.
void Karma::KarmaGuiInternal::UpdateMouseMovingWindowNewFrame()
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.MovingWindow != NULL)
	{
		// We actually want to move the root window. g.MovingWindow == window we clicked on (could be a child window).
		// We track it to preserve Focus and so that generally ActiveIdWindow == MovingWindow and ActiveId == MovingWindow->MoveId for consistency.
		KeepAliveID(g.ActiveId);
		KR_CORE_ASSERT(g.MovingWindow && g.MovingWindow->RootWindowDockTree, "");
		KGGuiWindow* moving_window = g.MovingWindow->RootWindowDockTree;

		// When a window stop being submitted while being dragged, it may will its viewport until next Begin()
		const bool window_disappared = ((!moving_window->WasActive && !moving_window->Active) || moving_window->Viewport == NULL);
		if (g.IO.MouseDown[0] && KarmaGui::IsMousePosValid(&g.IO.MousePos) && !window_disappared)
		{
			KGVec2 pos = g.IO.MousePos - g.ActiveIdClickOffset;
			if (moving_window->Pos.x != pos.x || moving_window->Pos.y != pos.y)
			{
				SetWindowPos(moving_window, pos, KGGuiCond_Always);
				if (moving_window->ViewportOwned) // Synchronize viewport immediately because some overlays may relies on clipping rectangle before we Begin() into the window.
				{
					moving_window->Viewport->Pos = pos;
					moving_window->Viewport->UpdateWorkRect();
				}
			}
			FocusWindow(g.MovingWindow);
		}
		else
		{
			if (!window_disappared)
			{
				// Try to merge the window back into the main viewport.
				// This works because MouseViewport should be != MovingWindow->Viewport on release (as per code in UpdateViewports)
				if (g.ConfigFlagsCurrFrame & KGGuiConfigFlags_ViewportsEnable)
					UpdateTryMergeWindowIntoHostViewport(moving_window, g.MouseViewport);

				// Restore the mouse viewport so that we don't hover the viewport _under_ the moved window during the frame we released the mouse button.
				if (!IsDragDropPayloadBeingAccepted())
					g.MouseViewport = moving_window->Viewport;

				// Clear the NoInput window flag set by the Viewport system
				moving_window->Viewport->Flags &= ~KGGuiViewportFlags_NoInputs; // FIXME-VIEWPORT: Test engine managed to crash here because Viewport was NULL.
			}

			g.MovingWindow = NULL;
			ClearActiveID();
		}
	}
	else
	{
		// When clicking/dragging from a window that has the _NoMove flag, we still set the ActiveId in order to prevent hovering others.
		if (g.ActiveIdWindow && g.ActiveIdWindow->MoveId == g.ActiveId)
		{
			KeepAliveID(g.ActiveId);
			if (!g.IO.MouseDown[0])
				ClearActiveID();
		}
	}
}

// Initiate moving window when clicking on empty space or title bar.
// Handle left-click and right-click focus.
void Karma::KarmaGuiInternal::UpdateMouseMovingWindowEndFrame()
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.ActiveId != 0 || g.HoveredId != 0)
		return;

	// Unless we just made a window/popup appear
	if (g.NavWindow && g.NavWindow->Appearing)
		return;

	// Click on empty space to focus window and start moving
	// (after we're done with all our widgets, so e.g. clicking on docking tab-bar which have set HoveredId already and not get us here!)
	if (g.IO.MouseClicked[0])
	{
		// Handle the edge case of a popup being closed while clicking in its empty space.
		// If we try to focus it, FocusWindow() > ClosePopupsOverWindow() will accidentally close any parent popups because they are not linked together any more.
		KGGuiWindow* root_window = g.HoveredWindow ? g.HoveredWindow->RootWindow : NULL;
		const bool is_closed_popup = root_window && (root_window->Flags & KGGuiWindowFlags_Popup) && !IsPopupOpen(root_window->PopupId, KGGuiPopupFlags_AnyPopupLevel);

		if (root_window != NULL && !is_closed_popup)
		{
			StartMouseMovingWindow(g.HoveredWindow); //-V595

			// Cancel moving if clicked outside of title bar
			if (g.IO.ConfigWindowsMoveFromTitleBarOnly)
				if (!(root_window->Flags & KGGuiWindowFlags_NoTitleBar) || root_window->DockIsActive)
					if (!root_window->TitleBarRect().Contains(g.IO.MouseClickedPos[0]))
						g.MovingWindow = NULL;

			// Cancel moving if clicked over an item which was disabled or inhibited by popups (note that we know HoveredId == 0 already)
			if (g.HoveredIdDisabled)
				g.MovingWindow = NULL;
		}
		else if (root_window == NULL && g.NavWindow != NULL && GetTopMostPopupModal() == NULL)
		{
			// Clicking on void disable focus
			FocusWindow(NULL);
		}
	}

	// With right mouse button we close popups without changing focus based on where the mouse is aimed
	// Instead, focus will be restored to the window under the bottom-most closed popup.
	// (The left mouse button path calls FocusWindow on the hovered window, which will lead NewFrame->ClosePopupsOverWindow to trigger)
	if (g.IO.MouseClicked[1])
	{
		// Find the top-most window between HoveredWindow and the top-most Modal Window.
		// This is where we can trim the popup stack.
		KGGuiWindow* modal = GetTopMostPopupModal();
		bool hovered_window_above_modal = g.HoveredWindow && (modal == NULL || IsWindowAbove(g.HoveredWindow, modal));
		ClosePopupsOverWindow(hovered_window_above_modal ? g.HoveredWindow : modal, true);
	}
}

// This is called during NewFrame()->UpdateViewportsNewFrame() only.
// Need to keep in sync with SetWindowPos()
static void TranslateWindow(KGGuiWindow* window, const KGVec2& delta)
{
	window->Pos += delta;
	window->ClipRect.Translate(delta);
	window->OuterRectClipped.Translate(delta);
	window->InnerRect.Translate(delta);
	window->DC.CursorPos += delta;
	window->DC.CursorStartPos += delta;
	window->DC.CursorMaxPos += delta;
	window->DC.IdealMaxPos += delta;
}

static void ScaleWindow(KGGuiWindow* window, float scale)
{
	KGVec2 origin = window->Viewport->Pos;
	window->Pos = KGFloor((window->Pos - origin) * scale + origin);
	window->Size = KGFloor(window->Size * scale);
	window->SizeFull = KGFloor(window->SizeFull * scale);
	window->ContentSize = KGFloor(window->ContentSize * scale);
}

static bool IsWindowActiveAndVisible(KGGuiWindow* window)
{
	return (window->Active) && (!window->Hidden);
}

// The reason this is exposed in imgui_internal.h is: on touch-based system that don't have hovering, we want to dispatch inputs to the right target (imgui vs imgui+app)
void Karma::KarmaGuiInternal::UpdateHoveredWindowAndCaptureFlags()
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiIO& io = g.IO;
	g.WindowsHoverPadding = KGMax(g.Style.TouchExtraPadding, KGVec2(WINDOWS_HOVER_PADDING, WINDOWS_HOVER_PADDING));

	// Find the window hovered by mouse:
	// - Child windows can extend beyond the limit of their parent so we need to derive HoveredRootWindow from HoveredWindow.
	// - When moving a window we can skip the search, which also conveniently bypasses the fact that window->WindowRectClipped is lagging as this point of the frame.
	// - We also support the moved window toggling the NoInputs flag after moving has started in order to be able to detect windows below it, which is useful for e.g. docking mechanisms.
	bool clear_hovered_windows = false;
	FindHoveredWindow();
	KR_CORE_ASSERT(g.HoveredWindow == NULL || g.HoveredWindow == g.MovingWindow || g.HoveredWindow->Viewport == g.MouseViewport, "");

	// Modal windows prevents mouse from hovering behind them.
	KGGuiWindow* modal_window = GetTopMostPopupModal();
	if (modal_window && g.HoveredWindow && !IsWindowWithinBeginStackOf(g.HoveredWindow->RootWindow, modal_window)) // FIXME-MERGE: RootWindowDockTree ?
		clear_hovered_windows = true;

	// Disabled mouse?
	if (io.ConfigFlags & KGGuiConfigFlags_NoMouse)
		clear_hovered_windows = true;

	// We track click ownership. When clicked outside of a window the click is owned by the application and
	// won't report hovering nor request capture even while dragging over our windows afterward.
	const bool has_open_popup = (g.OpenPopupStack.Size > 0);
	const bool has_open_modal = (modal_window != NULL);
	int mouse_earliest_down = -1;
	bool mouse_any_down = false;
	for (int i = 0; i < KG_ARRAYSIZE(io.MouseDown); i++)
	{
		if (io.MouseClicked[i])
		{
			io.MouseDownOwned[i] = (g.HoveredWindow != NULL) || has_open_popup;
			io.MouseDownOwnedUnlessPopupClose[i] = (g.HoveredWindow != NULL) || has_open_modal;
		}
		mouse_any_down |= io.MouseDown[i];
		if (io.MouseDown[i])
			if (mouse_earliest_down == -1 || io.MouseClickedTime[i] < io.MouseClickedTime[mouse_earliest_down])
				mouse_earliest_down = i;
	}
	const bool mouse_avail = (mouse_earliest_down == -1) || io.MouseDownOwned[mouse_earliest_down];
	const bool mouse_avail_unless_popup_close = (mouse_earliest_down == -1) || io.MouseDownOwnedUnlessPopupClose[mouse_earliest_down];

	// If mouse was first clicked outside of ImGui bounds we also cancel out hovering.
	// FIXME: For patterns of drag and drop across OS windows, we may need to rework/remove this test (first committed 311c0ca9 on 2015/02)
	const bool mouse_dragging_extern_payload = g.DragDropActive && (g.DragDropSourceFlags & KGGuiDragDropFlags_SourceExtern) != 0;
	if (!mouse_avail && !mouse_dragging_extern_payload)
		clear_hovered_windows = true;

	if (clear_hovered_windows)
		g.HoveredWindow = g.HoveredWindowUnderMovingWindow = NULL;

	// Update io.WantCaptureMouse for the user application (true = dispatch mouse info to Dear ImGui only, false = dispatch mouse to Dear ImGui + underlying app)
	// Update io.WantCaptureMouseAllowPopupClose (experimental) to give a chance for app to react to popup closure with a drag
	if (g.WantCaptureMouseNextFrame != -1)
	{
		io.WantCaptureMouse = io.WantCaptureMouseUnlessPopupClose = (g.WantCaptureMouseNextFrame != 0);
	}
	else
	{
		io.WantCaptureMouse = (mouse_avail && (g.HoveredWindow != NULL || mouse_any_down)) || has_open_popup;
		io.WantCaptureMouseUnlessPopupClose = (mouse_avail_unless_popup_close && (g.HoveredWindow != NULL || mouse_any_down)) || has_open_modal;
	}

	// Update io.WantCaptureKeyboard for the user application (true = dispatch keyboard info to Dear ImGui only, false = dispatch keyboard info to Dear ImGui + underlying app)
	if (g.WantCaptureKeyboardNextFrame != -1)
		io.WantCaptureKeyboard = (g.WantCaptureKeyboardNextFrame != 0);
	else
		io.WantCaptureKeyboard = (g.ActiveId != 0) || (modal_window != NULL);
	if (io.NavActive && (io.ConfigFlags & KGGuiConfigFlags_NavEnableKeyboard) && !(io.ConfigFlags & KGGuiConfigFlags_NavNoCaptureKeyboard))
		io.WantCaptureKeyboard = true;

	// Update io.WantTextInput flag, this is to allow systems without a keyboard (e.g. mobile, hand-held) to show a software keyboard if possible
	io.WantTextInput = (g.WantTextInputNextFrame != -1) ? (g.WantTextInputNextFrame != 0) : false;
}

void Karma::KarmaGui::NewFrame()
{
	KR_CORE_ASSERT(GKarmaGui != NULL, "No current context. Did you call KarmaGui::CreateContext() and KarmaGui::SetCurrentContext() ?");
	KarmaGuiContext& g = *GKarmaGui;

	// Remove pending delete hooks before frame start.
	// This deferred removal avoid issues of removal while iterating the hook vector
	for (int n = g.Hooks.Size - 1; n >= 0; n--)
		if (g.Hooks[n].Type == ImGuiContextHookType_PendingRemoval_)
			g.Hooks.erase(&g.Hooks[n]);

	KarmaGuiInternal::CallContextHooks(&g, ImGuiContextHookType_NewFramePre);

	// Check and assert for various common IO and Configuration mistakes
	g.ConfigFlagsLastFrame = g.ConfigFlagsCurrFrame;
	KarmaGuiInternal::ErrorCheckNewFrameSanityChecks();
	g.ConfigFlagsCurrFrame = g.IO.ConfigFlags;

	// Load settings on first frame, save settings when modified (after a delay)
	KarmaGuiInternal::UpdateSettings();

	g.Time += g.IO.DeltaTime;
	g.WithinFrameScope = true;
	g.FrameCount += 1;
	g.TooltipOverrideCount = 0;
	g.WindowsActiveCount = 0;
	g.MenusIdSubmittedThisFrame.resize(0);

	// Calculate frame-rate for the user, as a purely luxurious feature
	g.FramerateSecPerFrameAccum += g.IO.DeltaTime - g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx];
	g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx] = g.IO.DeltaTime;
	g.FramerateSecPerFrameIdx = (g.FramerateSecPerFrameIdx + 1) % KG_ARRAYSIZE(g.FramerateSecPerFrame);
	g.FramerateSecPerFrameCount = KGMin(g.FramerateSecPerFrameCount + 1, KG_ARRAYSIZE(g.FramerateSecPerFrame));
	g.IO.Framerate = (g.FramerateSecPerFrameAccum > 0.0f) ? (1.0f / (g.FramerateSecPerFrameAccum / (float)g.FramerateSecPerFrameCount)) : FLT_MAX;

	// Process input queue (trickle as many events as possible), turn events into writes to IO structure
	g.InputEventsTrail.resize(0);
	KarmaGuiInternal::UpdateInputEvents(g.IO.ConfigInputTrickleEventQueue);

	// Update viewports (after processing input queue, so io.MouseHoveredViewport is set)
	KarmaGuiInternal::UpdateViewportsNewFrame();

	// Setup current font and draw list shared data
	// FIXME-VIEWPORT: the concept of a single ClipRectFullscreen is not ideal!
	g.IO.Fonts->Locked = true;
	KarmaGuiInternal::SetCurrentFont(KarmaGuiInternal::GetDefaultFont());
	KR_CORE_ASSERT(g.Font->IsLoaded(), "");
	KGRect virtual_space(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (int n = 0; n < g.Viewports.Size; n++)
		virtual_space.Add(g.Viewports[n]->GetMainRect());
	g.DrawListSharedData.ClipRectFullscreen = virtual_space.ToVec4();
	g.DrawListSharedData.CurveTessellationTol = g.Style.CurveTessellationTol;
	g.DrawListSharedData.SetCircleTessellationMaxError(g.Style.CircleTessellationMaxError);
	g.DrawListSharedData.InitialFlags = KGDrawListFlags_None;
	if (g.Style.AntiAliasedLines)
		g.DrawListSharedData.InitialFlags |= KGDrawListFlags_AntiAliasedLines;
	if (g.Style.AntiAliasedLinesUseTex && !(g.Font->ContainerAtlas->Flags & KGFontAtlasFlags_NoBakedLines))
		g.DrawListSharedData.InitialFlags |= KGDrawListFlags_AntiAliasedLinesUseTex;
	if (g.Style.AntiAliasedFill)
		g.DrawListSharedData.InitialFlags |= KGDrawListFlags_AntiAliasedFill;
	if (g.IO.BackendFlags & KGGuiBackendFlags_RendererHasVtxOffset)
		g.DrawListSharedData.InitialFlags |= KGDrawListFlags_AllowVtxOffset;

	// Mark rendering data as invalid to prevent user who may have a handle on it to use it.
	for (int n = 0; n < g.Viewports.Size; n++)
	{
		KGGuiViewportP* viewport = g.Viewports[n];
		viewport->DrawData = NULL;
		viewport->DrawDataP.Clear();
	}

	// Drag and drop keep the source ID alive so even if the source disappear our state is consistent
	if (g.DragDropActive && g.DragDropPayload.SourceId == g.ActiveId)
		KarmaGuiInternal::KeepAliveID(g.DragDropPayload.SourceId);

	// Update HoveredId data
	if (!g.HoveredIdPreviousFrame)
		g.HoveredIdTimer = 0.0f;
	if (!g.HoveredIdPreviousFrame || (g.HoveredId && g.ActiveId == g.HoveredId))
		g.HoveredIdNotActiveTimer = 0.0f;
	if (g.HoveredId)
		g.HoveredIdTimer += g.IO.DeltaTime;
	if (g.HoveredId && g.ActiveId != g.HoveredId)
		g.HoveredIdNotActiveTimer += g.IO.DeltaTime;
	g.HoveredIdPreviousFrame = g.HoveredId;
	g.HoveredId = 0;
	g.HoveredIdAllowOverlap = false;
	g.HoveredIdDisabled = false;

	// Clear ActiveID if the item is not alive anymore.
	// In 1.87, the common most call to KeepAliveID() was moved from GetID() to ItemAdd().
	// As a result, custom widget using ButtonBehavior() _without_ ItemAdd() need to call KeepAliveID() themselves.
	if (g.ActiveId != 0 && g.ActiveIdIsAlive != g.ActiveId && g.ActiveIdPreviousFrame == g.ActiveId)
	{
		KR_CORE_INFO("NewFrame(): ClearActiveID() because it isn't marked alive or interesting anymore!");
		KarmaGuiInternal::ClearActiveID();
	}

	// Update ActiveId data (clear reference to active widget if the widget isn't alive anymore)
	if (g.ActiveId)
		g.ActiveIdTimer += g.IO.DeltaTime;
	g.LastActiveIdTimer += g.IO.DeltaTime;
	g.ActiveIdPreviousFrame = g.ActiveId;
	g.ActiveIdPreviousFrameWindow = g.ActiveIdWindow;
	g.ActiveIdPreviousFrameHasBeenEditedBefore = g.ActiveIdHasBeenEditedBefore;
	g.ActiveIdIsAlive = 0;
	g.ActiveIdHasBeenEditedThisFrame = false;
	g.ActiveIdPreviousFrameIsAlive = false;
	g.ActiveIdIsJustActivated = false;
	if (g.TempInputId != 0 && g.ActiveId != g.TempInputId)
		g.TempInputId = 0;
	if (g.ActiveId == 0)
	{
		g.ActiveIdUsingNavDirMask = 0x00;
		g.ActiveIdUsingAllKeyboardKeys = false;
#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
		g.ActiveIdUsingNavInputMask = 0x00;
#endif
	}

#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
	if (g.ActiveId == 0)
		g.ActiveIdUsingNavInputMask = 0;
	else if (g.ActiveIdUsingNavInputMask != 0)
	{
		// If your custom widget code used:                 { g.ActiveIdUsingNavInputMask |= (1 << KGGuiNavInput_Cancel); }
		// Since IMGUI_VERSION_NUM >= 18804 it should be:   { SetKeyOwner(KGGuiKey_Escape, g.ActiveId); SetKeyOwner(KGGuiKey_NavGamepadCancel, g.ActiveId); }
		if (g.ActiveIdUsingNavInputMask & (1 << KGGuiNavInput_Cancel))
			KarmaGuiInternal::SetKeyOwner(KGGuiKey_Escape, g.ActiveId);
		if (g.ActiveIdUsingNavInputMask & ~(1 << KGGuiNavInput_Cancel))
		{
			KR_CORE_ASSERT(0, ""); // Other values unsupported
		}
	}
#endif

	// Update hover delay for IsItemHovered() with delays and tooltips
	g.HoverDelayIdPreviousFrame = g.HoverDelayId;
	if (g.HoverDelayId != 0)
	{
		//if (g.IO.MouseDelta.x == 0.0f && g.IO.MouseDelta.y == 0.0f) // Need design/flags
		g.HoverDelayTimer += g.IO.DeltaTime;
		g.HoverDelayClearTimer = 0.0f;
		g.HoverDelayId = 0;
	}
	else if (g.HoverDelayTimer > 0.0f)
	{
		// This gives a little bit of leeway before clearing the hover timer, allowing mouse to cross gaps
		g.HoverDelayClearTimer += g.IO.DeltaTime;
		if (g.HoverDelayClearTimer >= KGMax(0.20f, g.IO.DeltaTime * 2.0f)) // ~6 frames at 30 Hz + allow for low framerate
			g.HoverDelayTimer = g.HoverDelayClearTimer = 0.0f; // May want a decaying timer, in which case need to clamp at max first, based on max of caller last requested timer.
	}

	// Drag and drop
	g.DragDropAcceptIdPrev = g.DragDropAcceptIdCurr;
	g.DragDropAcceptIdCurr = 0;
	g.DragDropAcceptIdCurrRectSurface = FLT_MAX;
	g.DragDropWithinSource = false;
	g.DragDropWithinTarget = false;
	g.DragDropHoldJustPressedId = 0;

	// Close popups on focus lost (currently wip/opt-in)
	//if (g.IO.AppFocusLost)
	//    ClosePopupsExceptModals();

	// Update keyboard input state
	KarmaGuiInternal::UpdateKeyboardInputs();

	//KR_CORE_ASSERT(g.IO.KeyCtrl == IsKeyDown(KGGuiKey_LeftCtrl) || IsKeyDown(KGGuiKey_RightCtrl));
	//KR_CORE_ASSERT(g.IO.KeyShift == IsKeyDown(KGGuiKey_LeftShift) || IsKeyDown(KGGuiKey_RightShift));
	//KR_CORE_ASSERT(g.IO.KeyAlt == IsKeyDown(KGGuiKey_LeftAlt) || IsKeyDown(KGGuiKey_RightAlt));
	//KR_CORE_ASSERT(g.IO.KeySuper == IsKeyDown(KGGuiKey_LeftSuper) || IsKeyDown(KGGuiKey_RightSuper));

	// Update gamepad/keyboard navigation
	KarmaGuiInternal::NavUpdate();

	// Update mouse input state
	KarmaGuiInternal::UpdateMouseInputs();

	// Undocking
	// (needs to be before UpdateMouseMovingWindowNewFrame so the window is already offset and following the mouse on the detaching frame)
	KarmaGuiInternal::DockContextNewFrameUpdateUndocking(&g);

	// Find hovered window
	// (needs to be before UpdateMouseMovingWindowNewFrame so we fill g.HoveredWindowUnderMovingWindow on the mouse release frame)
	KarmaGuiInternal::UpdateHoveredWindowAndCaptureFlags();

	// Handle user moving window with mouse (at the beginning of the frame to avoid input lag or sheering)
	KarmaGuiInternal::UpdateMouseMovingWindowNewFrame();

	// Background darkening/whitening
	if (KarmaGuiInternal::GetTopMostPopupModal() != NULL || (g.NavWindowingTarget != NULL && g.NavWindowingHighlightAlpha > 0.0f))
		g.DimBgRatio = KGMin(g.DimBgRatio + g.IO.DeltaTime * 6.0f, 1.0f);
	else
		g.DimBgRatio = KGMax(g.DimBgRatio - g.IO.DeltaTime * 10.0f, 0.0f);

	g.MouseCursor = KGGuiMouseCursor_Arrow;
	g.WantCaptureMouseNextFrame = g.WantCaptureKeyboardNextFrame = g.WantTextInputNextFrame = -1;

	// Platform IME data: reset for the frame
	g.PlatformImeDataPrev = g.PlatformImeData;
	g.PlatformImeData.WantVisible = false;

	// Mouse wheel scrolling, scale
	KarmaGuiInternal::UpdateMouseWheel();

	// Mark all windows as not visible and compact unused memory.
	KR_CORE_ASSERT(g.WindowsFocusOrder.Size <= g.Windows.Size, "");
	const float memory_compact_start_time = (g.GcCompactAll || g.IO.ConfigMemoryCompactTimer < 0.0f) ? FLT_MAX : (float)g.Time - g.IO.ConfigMemoryCompactTimer;
	for (int i = 0; i != g.Windows.Size; i++)
	{
		KGGuiWindow* window = g.Windows[i];
		window->WasActive = window->Active;
		window->Active = false;
		window->WriteAccessed = false;
		window->BeginCountPreviousFrame = window->BeginCount;
		window->BeginCount = 0;

		// Garbage collect transient buffers of recently unused windows
		if (!window->WasActive && !window->MemoryCompacted && window->LastTimeActive < memory_compact_start_time)
			KarmaGuiInternal::GcCompactTransientWindowBuffers(window);
	}

	// Garbage collect transient buffers of recently unused tables
	for (int i = 0; i < g.TablesLastTimeActive.Size; i++)
		if (g.TablesLastTimeActive[i] >= 0.0f && g.TablesLastTimeActive[i] < memory_compact_start_time)
			KarmaGuiInternal::TableGcCompactTransientBuffers(g.Tables.GetByIndex(i));
	for (int i = 0; i < g.TablesTempData.Size; i++)
		if (g.TablesTempData[i].LastTimeActive >= 0.0f && g.TablesTempData[i].LastTimeActive < memory_compact_start_time)
			KarmaGuiInternal::TableGcCompactTransientBuffers(&g.TablesTempData[i]);
	if (g.GcCompactAll)
		KarmaGuiInternal::GcCompactTransientMiscBuffers();
	g.GcCompactAll = false;

	// Closing the focused window restore focus to the first active root window in descending z-order
	if (g.NavWindow && !g.NavWindow->WasActive)
		KarmaGuiInternal::FocusTopMostWindowUnderOne(NULL, NULL);

	// No window should be open at the beginning of the frame.
	// But in order to allow the user to call NewFrame() multiple times without calling Render(), we are doing an explicit clear.
	g.CurrentWindowStack.resize(0);
	g.BeginPopupStack.resize(0);
	g.ItemFlagsStack.resize(0);
	g.ItemFlagsStack.push_back(KGGuiItemFlags_None);
	g.GroupStack.resize(0);

	// Docking
	KarmaGuiInternal::DockContextNewFrameUpdateDocking(&g);

	// [DEBUG] Update debug features
	KarmaGuiInternal::UpdateDebugToolItemPicker();
	KarmaGuiInternal::UpdateDebugToolStackQueries();
	if (g.DebugLocateFrames > 0 && --g.DebugLocateFrames == 0)
		g.DebugLocateId = 0;

	// Create implicit/fallback window - which we will only render it if the user has added something to it.
	// We don't use "Debug" to avoid colliding with user trying to create a "Debug" window with custom flags.
	// This fallback is particularly important as it prevents KarmaGui:: calls from crashing.
	g.WithinFrameScopeWithImplicitWindow = true;
	SetNextWindowSize(KGVec2(400, 400), KGGuiCond_FirstUseEver);
	Begin("Debug##Default");
	KR_CORE_ASSERT(g.CurrentWindow->IsFallbackWindow == true, "");

	KarmaGuiInternal::CallContextHooks(&g, ImGuiContextHookType_NewFramePost);
}

// FIXME: Add a more explicit sort order in the window structure.
static int ChildWindowComparer(const void* lhs, const void* rhs)
{
	const KGGuiWindow* const a = *(const KGGuiWindow* const*)lhs;
	const KGGuiWindow* const b = *(const KGGuiWindow* const*)rhs;
	if (int d = (a->Flags & KGGuiWindowFlags_Popup) - (b->Flags & KGGuiWindowFlags_Popup))
		return d;
	if (int d = (a->Flags & KGGuiWindowFlags_Tooltip) - (b->Flags & KGGuiWindowFlags_Tooltip))
		return d;
	return (a->BeginOrderWithinParent - b->BeginOrderWithinParent);
}

static void AddWindowToSortBuffer(KGVector<KGGuiWindow*>* out_sorted_windows, KGGuiWindow* window)
{
	out_sorted_windows->push_back(window);
	if (window->Active)
	{
		int count = window->DC.ChildWindows.Size;
		KGQsort(window->DC.ChildWindows.Data, (size_t)count, sizeof(KGGuiWindow*), ChildWindowComparer);
		for (int i = 0; i < count; i++)
		{
			KGGuiWindow* child = window->DC.ChildWindows[i];
			if (child->Active)
				AddWindowToSortBuffer(out_sorted_windows, child);
		}
	}
}

static void AddDrawListToDrawData(KGVector<KGDrawList*>* out_list, KGDrawList* draw_list)
{
	if (draw_list->CmdBuffer.Size == 0)
		return;
	if (draw_list->CmdBuffer.Size == 1 && draw_list->CmdBuffer[0].ElemCount == 0 && draw_list->CmdBuffer[0].UserCallback == NULL)
		return;

	// Draw list sanity check. Detect mismatch between PrimReserve() calls and incrementing _VtxCurrentIdx, _VtxWritePtr etc.
	// May trigger for you if you are using PrimXXX functions incorrectly.
	KR_CORE_ASSERT(draw_list->VtxBuffer.Size == 0 || draw_list->_VtxWritePtr == draw_list->VtxBuffer.Data + draw_list->VtxBuffer.Size, "");
	KR_CORE_ASSERT(draw_list->IdxBuffer.Size == 0 || draw_list->_IdxWritePtr == draw_list->IdxBuffer.Data + draw_list->IdxBuffer.Size, "");
	if (!(draw_list->Flags & KGDrawListFlags_AllowVtxOffset))
	{
		KR_CORE_ASSERT((int)draw_list->_VtxCurrentIdx == draw_list->VtxBuffer.Size, "");
	}

	// Check that draw_list doesn't use more vertices than indexable (default KGDrawIdx = unsigned short = 2 bytes = 64K vertices per KGDrawList = per window)
	// If this assert triggers because you are drawing lots of stuff manually:
	// - First, make sure you are coarse clipping yourself and not trying to draw many things outside visible bounds.
	//   Be mindful that the KGDrawList API doesn't filter vertices. Use the Metrics/Debugger window to inspect draw list contents.
	// - If you want large meshes with more than 64K vertices, you can either:
	//   (A) Handle the KGDrawCmd::VtxOffset value in your renderer backend, and set 'io.BackendFlags |= KGGuiBackendFlags_RendererHasVtxOffset'.
	//       Most example backends already support this from 1.71. Pre-1.71 backends won't.
	//       Some graphics API such as GL ES 1/2 don't have a way to offset the starting vertex so it is not supported for them.
	//   (B) Or handle 32-bit indices in your renderer backend, and uncomment '#define KGDrawIdx unsigned int' line in imconfig.h.
	//       Most example backends already support this. For example, the OpenGL example code detect index size at compile-time:
	//         glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(KGDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
	//       Your own engine or render API may use different parameters or function calls to specify index sizes.
	//       2 and 4 bytes indices are generally supported by most graphics API.
	// - If for some reason neither of those solutions works for you, a workaround is to call BeginChild()/EndChild() before reaching
	//   the 64K limit to split your draw commands in multiple draw lists.
	if (sizeof(KGDrawIdx) == 2)
	{
		KR_CORE_ASSERT(draw_list->_VtxCurrentIdx < (1 << 16) && "Too many vertices in KGDrawList using 16-bit indices. Read comment above", "");
	}

	out_list->push_back(draw_list);
}

static void AddWindowToDrawData(KGGuiWindow* window, int layer)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGGuiViewportP* viewport = window->Viewport;
	g.IO.MetricsRenderWindows++;
	if (window->Flags & KGGuiWindowFlags_DockNodeHost)
		window->DrawList->ChannelsMerge();
	AddDrawListToDrawData(&viewport->DrawDataBuilder.Layers[layer], window->DrawList);
	for (int i = 0; i < window->DC.ChildWindows.Size; i++)
	{
		KGGuiWindow* child = window->DC.ChildWindows[i];
		if (IsWindowActiveAndVisible(child)) // Clipped children may have been marked not active
			AddWindowToDrawData(child, layer);
	}
}

static inline int GetWindowDisplayLayer(KGGuiWindow* window)
{
	return (window->Flags & KGGuiWindowFlags_Tooltip) ? 1 : 0;
}

// Layer is locked for the root window, however child windows may use a different viewport (e.g. extruding menu)
static inline void AddRootWindowToDrawData(KGGuiWindow* window)
{
	AddWindowToDrawData(window, GetWindowDisplayLayer(window));
}

void KGDrawDataBuilder::FlattenIntoSingleLayer()
{
	int n = Layers[0].Size;
	int size = n;
	for (int i = 1; i < KG_ARRAYSIZE(Layers); i++)
		size += Layers[i].Size;
	Layers[0].resize(size);
	for (int layer_n = 1; layer_n < KG_ARRAYSIZE(Layers); layer_n++)
	{
		KGVector<KGDrawList*>& layer = Layers[layer_n];
		if (layer.empty())
			continue;
		memcpy(&Layers[0][n], &layer[0], layer.Size * sizeof(KGDrawList*));
		n += layer.Size;
		layer.resize(0);
	}
}

static void SetupViewportDrawData(KGGuiViewportP* viewport, KGVector<KGDrawList*>* draw_lists)
{
	// When minimized, we report draw_data->DisplaySize as zero to be consistent with non-viewport mode,
	// and to allow applications/backends to easily skip rendering.
	// FIXME: Note that we however do NOT attempt to report "zero drawlist / vertices" into the KGDrawData structure.
	// This is because the work has been done already, and its wasted! We should fix that and add optimizations for
	// it earlier in the pipeline, rather than pretend to hide the data at the end of the pipeline.
	const bool is_minimized = (viewport->Flags & KGGuiViewportFlags_Minimized) != 0;

	KarmaGuiIO& io = Karma::KarmaGui::GetIO();
	KGDrawData* draw_data = &viewport->DrawDataP;
	viewport->DrawData = draw_data; // Make publicly accessible
	draw_data->Valid = true;
	draw_data->CmdLists = (draw_lists->Size > 0) ? draw_lists->Data : NULL;
	draw_data->CmdListsCount = draw_lists->Size;
	draw_data->TotalVtxCount = draw_data->TotalIdxCount = 0;
	draw_data->DisplayPos = viewport->Pos;
	draw_data->DisplaySize = is_minimized ? KGVec2(0.0f, 0.0f) : viewport->Size;
	draw_data->FramebufferScale = io.DisplayFramebufferScale; // FIXME-VIEWPORT: This may vary on a per-monitor/viewport basis?
	draw_data->OwnerViewport = viewport;
	for (int n = 0; n < draw_lists->Size; n++)
	{
		KGDrawList* draw_list = draw_lists->Data[n];
		draw_list->_PopUnusedDrawCmd();
		draw_data->TotalVtxCount += draw_list->VtxBuffer.Size;
		draw_data->TotalIdxCount += draw_list->IdxBuffer.Size;
	}
}

// Push a clipping rectangle for both ImGui logic (hit-testing etc.) and low-level KGDrawList rendering.
// - When using this function it is sane to ensure that float are perfectly rounded to integer values,
//   so that e.g. (int)(max.x-min.x) in user's render produce correct result.
// - If the code here changes, may need to update code of functions like NextColumn() and PushColumnClipRect():
//   some frequently called functions which to modify both channels and clipping simultaneously tend to use the
//   more specialized SetWindowClipRectBeforeSetChannel() to avoid extraneous updates of underlying ImDrawCmds.
void Karma::KarmaGui::PushClipRect(const KGVec2& clip_rect_min, const KGVec2& clip_rect_max, bool intersect_with_current_clip_rect)
{
	KGGuiWindow* window = Karma::KarmaGuiInternal::GetCurrentWindow();
	window->DrawList->PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
	window->ClipRect = window->DrawList->_ClipRectStack.back();
}

void Karma::KarmaGui::PopClipRect()
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	window->DrawList->PopClipRect();
	window->ClipRect = window->DrawList->_ClipRectStack.back();
}

static KGGuiWindow* FindFrontMostVisibleChildWindow(KGGuiWindow* window)
{
	for (int n = window->DC.ChildWindows.Size - 1; n >= 0; n--)
		if (IsWindowActiveAndVisible(window->DC.ChildWindows[n]))
			return FindFrontMostVisibleChildWindow(window->DC.ChildWindows[n]);
	return window;
}

void Karma::KarmaGuiInternal::RenderDimmedBackgroundBehindWindow(KGGuiWindow* window, KGU32 col)
{
	if ((col & KG_COL32_A_MASK) == 0)
		return;

	KGGuiViewportP* viewport = window->Viewport;
	KGRect viewport_rect = viewport->GetMainRect();

	// Draw behind window by moving the draw command at the FRONT of the draw list
	{
		// We've already called AddWindowToDrawData() which called DrawList->ChannelsMerge() on DockNodeHost windows,
		// and draw list have been trimmed already, hence the explicit recreation of a draw command if missing.
		// FIXME: This is creating complication, might be simpler if we could inject a drawlist in drawdata at a given position and not attempt to manipulate KGDrawCmd order.
		KGDrawList* draw_list = window->RootWindowDockTree->DrawList;
		if (draw_list->CmdBuffer.Size == 0)
			draw_list->AddDrawCmd();
		draw_list->PushClipRect(viewport_rect.Min - KGVec2(1, 1), viewport_rect.Max + KGVec2(1, 1), false); // Ensure KGDrawCmd are not merged
		draw_list->AddRectFilled(viewport_rect.Min, viewport_rect.Max, col);
		KGDrawCmd cmd = draw_list->CmdBuffer.back();
		KR_CORE_ASSERT(cmd.ElemCount == 6, "");
		draw_list->CmdBuffer.pop_back();
		draw_list->CmdBuffer.push_front(cmd);
		draw_list->PopClipRect();
		draw_list->AddDrawCmd(); // We need to create a command as CmdBuffer.back().IdxOffset won't be correct if we append to same command.
	}

	// Draw over sibling docking nodes in a same docking tree
	if (window->RootWindow->DockIsActive)
	{
		KGDrawList* draw_list = FindFrontMostVisibleChildWindow(window->RootWindowDockTree)->DrawList;
		if (draw_list->CmdBuffer.Size == 0)
			draw_list->AddDrawCmd();
		draw_list->PushClipRect(viewport_rect.Min, viewport_rect.Max, false);
		RenderRectFilledWithHole(draw_list, window->RootWindowDockTree->Rect(), window->RootWindow->Rect(), col, 0.0f);// window->RootWindowDockTree->WindowRounding);
		draw_list->PopClipRect();
	}
}

KGGuiWindow* Karma::KarmaGuiInternal::FindBottomMostVisibleWindowWithinBeginStack(KGGuiWindow* parent_window)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* bottom_most_visible_window = parent_window;
	for (int i = FindWindowDisplayIndex(parent_window); i >= 0; i--)
	{
		KGGuiWindow* window = g.Windows[i];
		if (window->Flags & KGGuiWindowFlags_ChildWindow)
			continue;
		if (!IsWindowWithinBeginStackOf(window, parent_window))
			break;
		if (IsWindowActiveAndVisible(window) && GetWindowDisplayLayer(window) <= GetWindowDisplayLayer(parent_window))
			bottom_most_visible_window = window;
	}
	return bottom_most_visible_window;
}

void Karma::KarmaGuiInternal::RenderDimmedBackgrounds()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* modal_window = GetTopMostAndVisiblePopupModal();
	if (g.DimBgRatio <= 0.0f && g.NavWindowingHighlightAlpha <= 0.0f)
		return;
	const bool dim_bg_for_modal = (modal_window != NULL);
	const bool dim_bg_for_window_list = (g.NavWindowingTargetAnim != NULL && g.NavWindowingTargetAnim->Active);
	if (!dim_bg_for_modal && !dim_bg_for_window_list)
		return;

	KarmaGuiViewport* viewports_already_dimmed[2] = { NULL, NULL };
	if (dim_bg_for_modal)
	{
		// Draw dimming behind modal or a begin stack child, whichever comes first in draw order.
		KGGuiWindow* dim_behind_window = FindBottomMostVisibleWindowWithinBeginStack(modal_window);
		RenderDimmedBackgroundBehindWindow(dim_behind_window, KarmaGui::GetColorU32(KGGuiCol_ModalWindowDimBg, g.DimBgRatio));
		viewports_already_dimmed[0] = modal_window->Viewport;
	}
	else if (dim_bg_for_window_list)
	{
		// Draw dimming behind CTRL+Tab target window and behind CTRL+Tab UI window
		RenderDimmedBackgroundBehindWindow(g.NavWindowingTargetAnim, KarmaGui::GetColorU32(KGGuiCol_NavWindowingDimBg, g.DimBgRatio));
		if (g.NavWindowingListWindow != NULL && g.NavWindowingListWindow->Viewport && g.NavWindowingListWindow->Viewport != g.NavWindowingTargetAnim->Viewport)
			RenderDimmedBackgroundBehindWindow(g.NavWindowingListWindow, KarmaGui::GetColorU32(KGGuiCol_NavWindowingDimBg, g.DimBgRatio));
		viewports_already_dimmed[0] = g.NavWindowingTargetAnim->Viewport;
		viewports_already_dimmed[1] = g.NavWindowingListWindow ? g.NavWindowingListWindow->Viewport : NULL;

		// Draw border around CTRL+Tab target window
		KGGuiWindow* window = g.NavWindowingTargetAnim;
		KarmaGuiViewport* viewport = window->Viewport;
		float distance = g.FontSize;
		KGRect bb = window->Rect();
		bb.Expand(distance);
		if (bb.GetWidth() >= viewport->Size.x && bb.GetHeight() >= viewport->Size.y)
			bb.Expand(-distance - 1.0f); // If a window fits the entire viewport, adjust its highlight inward
		if (window->DrawList->CmdBuffer.Size == 0)
			window->DrawList->AddDrawCmd();
		window->DrawList->PushClipRect(viewport->Pos, viewport->Pos + viewport->Size);
		window->DrawList->AddRect(bb.Min, bb.Max, KarmaGui::GetColorU32(KGGuiCol_NavWindowingHighlight, g.NavWindowingHighlightAlpha), window->WindowRounding, 0, 3.0f);
		window->DrawList->PopClipRect();
	}

	// Draw dimming background on _other_ viewports than the ones our windows are in
	for (int viewport_n = 0; viewport_n < g.Viewports.Size; viewport_n++)
	{
		KGGuiViewportP* viewport = g.Viewports[viewport_n];
		if (viewport == viewports_already_dimmed[0] || viewport == viewports_already_dimmed[1])
			continue;
		if (modal_window && viewport->Window && IsWindowAbove(viewport->Window, modal_window))
			continue;
		KGDrawList* draw_list = KarmaGui::GetForegroundDrawList(viewport);
		const KGU32 dim_bg_col = KarmaGui::GetColorU32(dim_bg_for_modal ? KGGuiCol_ModalWindowDimBg : KGGuiCol_NavWindowingDimBg, g.DimBgRatio);
		draw_list->AddRectFilled(viewport->Pos, viewport->Pos + viewport->Size, dim_bg_col);
	}
}

// This is normally called by Render(). You may want to call it directly if you want to avoid calling Render() but the gain will be very minimal.
void Karma::KarmaGui::EndFrame()
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(g.Initialized, "");

	// Don't process EndFrame() multiple times.
	if (g.FrameCountEnded == g.FrameCount)
		return;
	KR_CORE_ASSERT(g.WithinFrameScope, "Forgot to call KarmaGui::NewFrame()?");

	KarmaGuiInternal::CallContextHooks(&g, ImGuiContextHookType_EndFramePre);

	KarmaGuiInternal::ErrorCheckEndFrameSanityChecks();

	// Notify Platform/OS when our Input Method Editor cursor has moved (e.g. CJK inputs using Microsoft IME)
	if (g.IO.SetPlatformImeDataFn && memcmp(&g.PlatformImeData, &g.PlatformImeDataPrev, sizeof(KarmaGuiPlatformImeData)) != 0)
	{
		KarmaGuiViewport* viewport = FindViewportByID(g.PlatformImeViewport);
		g.IO.SetPlatformImeDataFn(viewport ? viewport : GetMainViewport(), &g.PlatformImeData);
	}

	// Hide implicit/fallback "Debug" window if it hasn't been used
	g.WithinFrameScopeWithImplicitWindow = false;
	if (g.CurrentWindow && !g.CurrentWindow->WriteAccessed)
		g.CurrentWindow->Active = false;
	End();

	// Update navigation: CTRL+Tab, wrap-around requests
	KarmaGuiInternal::NavEndFrame();

	// Update docking
	KarmaGuiInternal::DockContextEndFrame(&g);

	KarmaGuiInternal::SetCurrentViewport(NULL, NULL);

	// Drag and Drop: Elapse payload (if delivered, or if source stops being submitted)
	if (g.DragDropActive)
	{
		bool is_delivered = g.DragDropPayload.Delivery;
		bool is_elapsed = (g.DragDropPayload.DataFrameCount + 1 < g.FrameCount) && ((g.DragDropSourceFlags & KGGuiDragDropFlags_SourceAutoExpirePayload) || !IsMouseDown(g.DragDropMouseButton));
		if (is_delivered || is_elapsed)
			KarmaGuiInternal::ClearDragDrop();
	}

	// Drag and Drop: Fallback for source tooltip. This is not ideal but better than nothing.
	if (g.DragDropActive && g.DragDropSourceFrameCount < g.FrameCount && !(g.DragDropSourceFlags & KGGuiDragDropFlags_SourceNoPreviewTooltip))
	{
		g.DragDropWithinSource = true;
		SetTooltip("...");
		g.DragDropWithinSource = false;
	}

	// End frame
	g.WithinFrameScope = false;
	g.FrameCountEnded = g.FrameCount;

	// Initiate moving window + handle left-click and right-click focus
	KarmaGuiInternal::UpdateMouseMovingWindowEndFrame();

	// Update user-facing viewport list (g.Viewports -> g.PlatformIO.Viewports after filtering out some)
	KarmaGuiInternal::UpdateViewportsEndFrame();

	// Sort the window list so that all child windows are after their parent
	// We cannot do that on FocusWindow() because children may not exist yet
	g.WindowsTempSortBuffer.resize(0);
	g.WindowsTempSortBuffer.reserve(g.Windows.Size);
	for (int i = 0; i != g.Windows.Size; i++)
	{
		KGGuiWindow* window = g.Windows[i];
		if (window->Active && (window->Flags & KGGuiWindowFlags_ChildWindow))       // if a child is active its parent will add it
			continue;
		AddWindowToSortBuffer(&g.WindowsTempSortBuffer, window);
	}

	// This usually assert if there is a mismatch between the KGGuiWindowFlags_ChildWindow / ParentWindow values and DC.ChildWindows[] in parents, aka we've done something wrong.
	KR_CORE_ASSERT(g.Windows.Size == g.WindowsTempSortBuffer.Size, "");
	g.Windows.swap(g.WindowsTempSortBuffer);
	g.IO.MetricsActiveWindows = g.WindowsActiveCount;

	// Unlock font atlas
	g.IO.Fonts->Locked = false;

	// Clear Input data for next frame
	g.IO.AppFocusLost = false;
	g.IO.MouseWheel = g.IO.MouseWheelH = 0.0f;
	g.IO.InputQueueCharacters.resize(0);

	KarmaGuiInternal::CallContextHooks(&g, ImGuiContextHookType_EndFramePost);
}

// Prepare the data for rendering so you can call GetDrawData()
// (As with anything within the KarmaGui:: namspace this doesn't touch your GPU or graphics API at all:
// it is the role of the ImGui_ImplXXXX_RenderDrawData() function provided by the renderer backend)
void Karma::KarmaGui::Render()
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(g.Initialized, "");

	if (g.FrameCountEnded != g.FrameCount)
		EndFrame();
	const bool first_render_of_frame = (g.FrameCountRendered != g.FrameCount);
	g.FrameCountRendered = g.FrameCount;
	g.IO.MetricsRenderWindows = 0;

	Karma::KarmaGuiInternal::CallContextHooks(&g, ImGuiContextHookType_RenderPre);

	// Add background KGDrawList (for each active viewport)
	for (int n = 0; n != g.Viewports.Size; n++)
	{
		KGGuiViewportP* viewport = g.Viewports[n];
		viewport->DrawDataBuilder.Clear();
		if (viewport->DrawLists[0] != NULL)
			AddDrawListToDrawData(&viewport->DrawDataBuilder.Layers[0], GetBackgroundDrawList(viewport));
	}

	// Add KGDrawList to render
	KGGuiWindow* windows_to_render_top_most[2];
	windows_to_render_top_most[0] = (g.NavWindowingTarget && !(g.NavWindowingTarget->Flags & KGGuiWindowFlags_NoBringToFrontOnFocus)) ? g.NavWindowingTarget->RootWindowDockTree : NULL;
	windows_to_render_top_most[1] = (g.NavWindowingTarget ? g.NavWindowingListWindow : NULL);
	for (int n = 0; n != g.Windows.Size; n++)
	{
		KGGuiWindow* window = g.Windows[n];
		KG_MSVC_WARNING_SUPPRESS(6011); // Static Analysis false positive "warning C6011: Dereferencing NULL pointer 'window'"
		if (IsWindowActiveAndVisible(window) && (window->Flags & KGGuiWindowFlags_ChildWindow) == 0 && window != windows_to_render_top_most[0] && window != windows_to_render_top_most[1])
			AddRootWindowToDrawData(window);
	}
	for (int n = 0; n < KG_ARRAYSIZE(windows_to_render_top_most); n++)
		if (windows_to_render_top_most[n] && IsWindowActiveAndVisible(windows_to_render_top_most[n])) // NavWindowingTarget is always temporarily displayed as the top-most window
			AddRootWindowToDrawData(windows_to_render_top_most[n]);

	// Draw modal/window whitening backgrounds
	if (first_render_of_frame)
		KarmaGuiInternal::RenderDimmedBackgrounds();

	// Draw software mouse cursor if requested by io.MouseDrawCursor flag
	if (g.IO.MouseDrawCursor && first_render_of_frame && g.MouseCursor != KGGuiMouseCursor_None)
		KarmaGuiInternal::RenderMouseCursor(g.IO.MousePos, g.Style.MouseCursorScale, g.MouseCursor, KG_COL32_WHITE, KG_COL32_BLACK, KG_COL32(0, 0, 0, 48));

	// Setup KGDrawData structures for end-user
	g.IO.MetricsRenderVertices = g.IO.MetricsRenderIndices = 0;
	for (int n = 0; n < g.Viewports.Size; n++)
	{
		KGGuiViewportP* viewport = g.Viewports[n];
		viewport->DrawDataBuilder.FlattenIntoSingleLayer();

		// Add foreground KGDrawList (for each active viewport)
		if (viewport->DrawLists[1] != NULL)
			AddDrawListToDrawData(&viewport->DrawDataBuilder.Layers[0], GetForegroundDrawList(viewport));

		SetupViewportDrawData(viewport, &viewport->DrawDataBuilder.Layers[0]);
		KGDrawData* draw_data = viewport->DrawData;
		g.IO.MetricsRenderVertices += draw_data->TotalVtxCount;
		g.IO.MetricsRenderIndices += draw_data->TotalIdxCount;
	}

	KarmaGuiInternal::CallContextHooks(&g, ImGuiContextHookType_RenderPost);
}

// Calculate text size. Text can be multi-line. Optionally ignore text after a ## marker.
// CalcTextSize("") should return KGVec2(0.0f, g.FontSize)
KGVec2 Karma::KarmaGui::CalcTextSize(const char* text, const char* text_end, bool hide_text_after_double_hash, float wrap_width)
{
	KarmaGuiContext& g = *GKarmaGui;

	const char* text_display_end;
	if (hide_text_after_double_hash)
		text_display_end = KarmaGuiInternal::FindRenderedTextEnd(text, text_end);      // Hide anything after a '##' string
	else
		text_display_end = text_end;

	KGFont* font = g.Font;
	const float font_size = g.FontSize;
	if (text == text_display_end)
		return KGVec2(0.0f, font_size);
	KGVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, wrap_width, text, text_display_end, NULL);

	// Round
	// FIXME: This has been here since Dec 2015 (7b0bf230) but down the line we want this out.
	// FIXME: Investigate using ceilf or e.g.
	// - https://git.musl-libc.org/cgit/musl/tree/src/math/ceilf.c
	// - https://embarkstudios.github.io/rust-gpu/api/src/libm/math/ceilf.rs.html
	text_size.x = KG_FLOOR(text_size.x + 0.99999f);

	return text_size;
}

// Find window given position, search front-to-back
// FIXME: Note that we have an inconsequential lag here: OuterRectClipped is updated in Begin(), so windows moved programmatically
// with SetWindowPos() and not SetNextWindowPos() will have that rectangle lagging by a frame at the time FindHoveredWindow() is
// called, aka before the next Begin(). Moving window isn't affected.
void Karma::KarmaGuiInternal::FindHoveredWindow()
{
	KarmaGuiContext& g = *Karma::GKarmaGui;

	// Special handling for the window being moved: Ignore the mouse viewport check (because it may reset/lose its viewport during the undocking frame)
	KGGuiViewportP* moving_window_viewport = g.MovingWindow ? g.MovingWindow->Viewport : NULL;
	if (g.MovingWindow)
		g.MovingWindow->Viewport = g.MouseViewport;

	KGGuiWindow* hovered_window = NULL;
	KGGuiWindow* hovered_window_ignoring_moving_window = NULL;
	if (g.MovingWindow && !(g.MovingWindow->Flags & KGGuiWindowFlags_NoMouseInputs))
		hovered_window = g.MovingWindow;

	KGVec2 padding_regular = g.Style.TouchExtraPadding;
	KGVec2 padding_for_resize = g.IO.ConfigWindowsResizeFromEdges ? g.WindowsHoverPadding : padding_regular;
	for (int i = g.Windows.Size - 1; i >= 0; i--)
	{
		KGGuiWindow* window = g.Windows[i];
		KG_MSVC_WARNING_SUPPRESS(28182); // [Static Analyzer] Dereferencing NULL pointer.
		if (!window->Active || window->Hidden)
			continue;
		if (window->Flags & KGGuiWindowFlags_NoMouseInputs)
			continue;
		KR_CORE_ASSERT(window->Viewport, "");
		if (window->Viewport != g.MouseViewport)
			continue;

		// Using the clipped AABB, a child window will typically be clipped by its parent (not always)
		KGRect bb(window->OuterRectClipped);
		if (window->Flags & (KGGuiWindowFlags_ChildWindow | KGGuiWindowFlags_NoResize | KGGuiWindowFlags_AlwaysAutoResize))
			bb.Expand(padding_regular);
		else
			bb.Expand(padding_for_resize);
		if (!bb.Contains(g.IO.MousePos))
			continue;

		// Support for one rectangular hole in any given window
		// FIXME: Consider generalizing hit-testing override (with more generic data, callback, etc.) (#1512)
		if (window->HitTestHoleSize.x != 0)
		{
			KGVec2 hole_pos(window->Pos.x + (float)window->HitTestHoleOffset.x, window->Pos.y + (float)window->HitTestHoleOffset.y);
			KGVec2 hole_size((float)window->HitTestHoleSize.x, (float)window->HitTestHoleSize.y);
			if (KGRect(hole_pos, hole_pos + hole_size).Contains(g.IO.MousePos))
				continue;
		}

		if (hovered_window == NULL)
			hovered_window = window;
		KG_MSVC_WARNING_SUPPRESS(28182); // [Static Analyzer] Dereferencing NULL pointer.
		if (hovered_window_ignoring_moving_window == NULL && (!g.MovingWindow || window->RootWindowDockTree != g.MovingWindow->RootWindowDockTree))
			hovered_window_ignoring_moving_window = window;
		if (hovered_window && hovered_window_ignoring_moving_window)
			break;
	}

	g.HoveredWindow = hovered_window;
	g.HoveredWindowUnderMovingWindow = hovered_window_ignoring_moving_window;

	if (g.MovingWindow)
		g.MovingWindow->Viewport = moving_window_viewport;
}

bool Karma::KarmaGui::IsItemActive()
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.ActiveId)
		return g.ActiveId == g.LastItemData.ID;
	return false;
}

bool Karma::KarmaGui::IsItemActivated()
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.ActiveId)
		if (g.ActiveId == g.LastItemData.ID && g.ActiveIdPreviousFrame != g.LastItemData.ID)
			return true;
	return false;
}

bool Karma::KarmaGui::IsItemDeactivated()
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.LastItemData.StatusFlags & KGGuiItemStatusFlags_HasDeactivated)
		return (g.LastItemData.StatusFlags & KGGuiItemStatusFlags_Deactivated) != 0;
	return (g.ActiveIdPreviousFrame == g.LastItemData.ID && g.ActiveIdPreviousFrame != 0 && g.ActiveId != g.LastItemData.ID);
}

bool Karma::KarmaGui::IsItemDeactivatedAfterEdit()
{
	KarmaGuiContext& g = *GKarmaGui;
	return IsItemDeactivated() && (g.ActiveIdPreviousFrameHasBeenEditedBefore || (g.ActiveId == 0 && g.ActiveIdHasBeenEditedBefore));
}

// == GetItemID() == GetFocusID()
bool Karma::KarmaGui::IsItemFocused()
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.NavId != g.LastItemData.ID || g.NavId == 0)
		return false;

	// Special handling for the dummy item after Begin() which represent the title bar or tab.
	// When the window is collapsed (SkipItems==true) that last item will never be overwritten so we need to detect the case.
	KGGuiWindow* window = g.CurrentWindow;
	if (g.LastItemData.ID == window->ID && window->WriteAccessed)
		return false;

	return true;
}

// Important: this can be useful but it is NOT equivalent to the behavior of e.g.Button()!
// Most widgets have specific reactions based on mouse-up/down state, mouse position etc.
bool Karma::KarmaGui::IsItemClicked(KarmaGuiMouseButton mouse_button)
{
	return IsMouseClicked(mouse_button) && IsItemHovered(KGGuiHoveredFlags_None);
}

bool Karma::KarmaGui::IsItemToggledOpen()
{
	KarmaGuiContext& g = *GKarmaGui;
	return (g.LastItemData.StatusFlags & KGGuiItemStatusFlags_ToggledOpen) ? true : false;
}

bool Karma::KarmaGuiInternal::IsItemToggledSelection()
{
	KarmaGuiContext& g = *GKarmaGui;
	return (g.LastItemData.StatusFlags & KGGuiItemStatusFlags_ToggledSelection) ? true : false;
}

bool Karma::KarmaGui::IsAnyItemHovered()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.HoveredId != 0 || g.HoveredIdPreviousFrame != 0;
}

bool Karma::KarmaGui::IsAnyItemActive()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.ActiveId != 0;
}

bool Karma::KarmaGui::IsAnyItemFocused()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.NavId != 0 && !g.NavDisableHighlight;
}

bool Karma::KarmaGui::IsItemVisible()
{
	KarmaGuiContext& g = *GKarmaGui;
	return (g.LastItemData.StatusFlags & KGGuiItemStatusFlags_Visible) != 0;
}

bool Karma::KarmaGui::IsItemEdited()
{
	KarmaGuiContext& g = *GKarmaGui;
	return (g.LastItemData.StatusFlags & KGGuiItemStatusFlags_Edited) != 0;
}

// Allow last item to be overlapped by a subsequent item. Both may be activated during the same frame before the later one takes priority.
// FIXME: Although this is exposed, its interaction and ideal idiom with using KGGuiButtonFlags_AllowItemOverlap flag are extremely confusing, need rework.
void Karma::KarmaGui::SetItemAllowOverlap()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiID id = g.LastItemData.ID;
	if (g.HoveredId == id)
		g.HoveredIdAllowOverlap = true;
	if (g.ActiveId == id)
		g.ActiveIdAllowOverlap = true;
}

// FIXME: It might be undesirable that this will likely disable KeyOwner-aware shortcuts systems. Consider a more fine-tuned version for the two users of this function.
void Karma::KarmaGuiInternal::SetActiveIdUsingAllKeyboardKeys()
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(g.ActiveId != 0, "");
	g.ActiveIdUsingNavDirMask = (1 << KGGuiDir_COUNT) - 1;
	g.ActiveIdUsingAllKeyboardKeys = true;
	NavMoveRequestCancel();
}

KGGuiID Karma::KarmaGui::GetItemID()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.LastItemData.ID;
}

KGVec2 Karma::KarmaGui::GetItemRectMin()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.LastItemData.Rect.Min;
}

KGVec2 Karma::KarmaGui::GetItemRectMax()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.LastItemData.Rect.Max;
}

KGVec2 Karma::KarmaGui::GetItemRectSize()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.LastItemData.Rect.GetSize();
}

bool Karma::KarmaGuiInternal::BeginChildEx(const char* name, KGGuiID id, const KGVec2& size_arg, bool border, KarmaGuiWindowFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* parent_window = g.CurrentWindow;

	flags |= KGGuiWindowFlags_NoTitleBar | KGGuiWindowFlags_NoResize | KGGuiWindowFlags_NoSavedSettings | KGGuiWindowFlags_ChildWindow | KGGuiWindowFlags_NoDocking;
	flags |= (parent_window->Flags & KGGuiWindowFlags_NoMove);  // Inherit the NoMove flag

	// Size
	const KGVec2 content_avail = KarmaGui::GetContentRegionAvail();
	KGVec2 size = KGFloor(size_arg);
	const int auto_fit_axises = ((size.x == 0.0f) ? (1 << KGGuiAxis_X) : 0x00) | ((size.y == 0.0f) ? (1 << KGGuiAxis_Y) : 0x00);
	if (size.x <= 0.0f)
		size.x = KGMax(content_avail.x + size.x, 4.0f); // Arbitrary minimum child size (0.0f causing too many issues)
	if (size.y <= 0.0f)
		size.y = KGMax(content_avail.y + size.y, 4.0f);
	KarmaGui::SetNextWindowSize(size);

	// Build up name. If you need to append to a same child from multiple location in the ID stack, use BeginChild(KGGuiID id) with a stable value.
	const char* temp_window_name;
	if (name)
		KGFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%s_%08X", parent_window->Name, name, id);
	else
		KGFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%08X", parent_window->Name, id);

	const float backup_border_size = g.Style.ChildBorderSize;
	if (!border)
		g.Style.ChildBorderSize = 0.0f;
	bool ret = KarmaGui::Begin(temp_window_name, NULL, flags);
	g.Style.ChildBorderSize = backup_border_size;

	KGGuiWindow* child_window = g.CurrentWindow;
	child_window->ChildId = id;
	child_window->AutoFitChildAxises = (KGS8)auto_fit_axises;

	// Set the cursor to handle case where the user called SetNextWindowPos()+BeginChild() manually.
	// While this is not really documented/defined, it seems that the expected thing to do.
	if (child_window->BeginCount == 1)
		parent_window->DC.CursorPos = child_window->Pos;

	// Process navigation-in immediately so NavInit can run on first frame
	if (g.NavActivateId == id && !(flags & KGGuiWindowFlags_NavFlattened) && (child_window->DC.NavLayersActiveMask != 0 || child_window->DC.NavHasScroll))
	{
		FocusWindow(child_window);
		NavInitWindow(child_window, false);
		SetActiveID(id + 1, child_window); // Steal ActiveId with another arbitrary id so that key-press won't activate child item
		g.ActiveIdSource = KGGuiInputSource_Nav;
	}
	return ret;
}

bool Karma::KarmaGui::BeginChild(const char* str_id, const KGVec2& size_arg, bool border, KarmaGuiWindowFlags extra_flags)
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	return KarmaGuiInternal::BeginChildEx(str_id, window->GetID(str_id), size_arg, border, extra_flags);
}

bool Karma::KarmaGui::BeginChild(KGGuiID id, const KGVec2& size_arg, bool border, KarmaGuiWindowFlags extra_flags)
{
	KR_CORE_ASSERT(id != 0, "");
	return KarmaGuiInternal::BeginChildEx(NULL, id, size_arg, border, extra_flags);
}

void Karma::KarmaGui::EndChild()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;

	KR_CORE_ASSERT(g.WithinEndChild == false, "");
	KR_CORE_ASSERT(window->Flags & KGGuiWindowFlags_ChildWindow, "");   // Mismatched BeginChild()/EndChild() calls

	g.WithinEndChild = true;
	if (window->BeginCount > 1)
	{
		End();
	}
	else
	{
		KGVec2 sz = window->Size;
		if (window->AutoFitChildAxises & (1 << KGGuiAxis_X)) // Arbitrary minimum zero-ish child size of 4.0f causes less trouble than a 0.0f
			sz.x = KGMax(4.0f, sz.x);
		if (window->AutoFitChildAxises & (1 << KGGuiAxis_Y))
			sz.y = KGMax(4.0f, sz.y);
		End();

		KGGuiWindow* parent_window = g.CurrentWindow;
		KGRect bb(parent_window->DC.CursorPos, parent_window->DC.CursorPos + sz);
		KarmaGuiInternal::ItemSize(sz);
		if ((window->DC.NavLayersActiveMask != 0 || window->DC.NavHasScroll) && !(window->Flags & KGGuiWindowFlags_NavFlattened))
		{
			KarmaGuiInternal::ItemAdd(bb, window->ChildId);
			KarmaGuiInternal::RenderNavHighlight(bb, window->ChildId);

			// When browsing a window that has no activable items (scroll only) we keep a highlight on the child (pass g.NavId to trick into always displaying)
			if (window->DC.NavLayersActiveMask == 0 && window == g.NavWindow)
				KarmaGuiInternal::RenderNavHighlight(KGRect(bb.Min - KGVec2(2, 2), bb.Max + KGVec2(2, 2)), g.NavId, KGGuiNavHighlightFlags_TypeThin);
		}
		else
		{
			// Not navigable into
			KarmaGuiInternal::ItemAdd(bb, 0);
		}
		if (g.HoveredWindow == window)
			g.LastItemData.StatusFlags |= KGGuiItemStatusFlags_HoveredWindow;
	}
	g.WithinEndChild = false;
	g.LogLinePosY = -FLT_MAX; // To enforce a carriage return
}

// Helper to create a child window / scrolling region that looks like a normal widget frame.
bool Karma::KarmaGui::BeginChildFrame(KGGuiID id, const KGVec2& size, KarmaGuiWindowFlags extra_flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	const KarmaGuiStyle& style = g.Style;
	PushStyleColor(KGGuiCol_ChildBg, style.Colors[KGGuiCol_FrameBg]);
	PushStyleVar(KGGuiStyleVar_ChildRounding, style.FrameRounding);
	PushStyleVar(KGGuiStyleVar_ChildBorderSize, style.FrameBorderSize);
	PushStyleVar(KGGuiStyleVar_WindowPadding, style.FramePadding);
	bool ret = BeginChild(id, size, true, KGGuiWindowFlags_NoMove | KGGuiWindowFlags_AlwaysUseWindowPadding | extra_flags);
	PopStyleVar(3);
	PopStyleColor();
	return ret;
}

void Karma::KarmaGui::EndChildFrame()
{
	EndChild();
}

static void SetWindowConditionAllowFlags(KGGuiWindow* window, KarmaGuiCond flags, bool enabled)
{
	window->SetWindowPosAllowFlags = enabled ? (window->SetWindowPosAllowFlags | flags) : (window->SetWindowPosAllowFlags & ~flags);
	window->SetWindowSizeAllowFlags = enabled ? (window->SetWindowSizeAllowFlags | flags) : (window->SetWindowSizeAllowFlags & ~flags);
	window->SetWindowCollapsedAllowFlags = enabled ? (window->SetWindowCollapsedAllowFlags | flags) : (window->SetWindowCollapsedAllowFlags & ~flags);
	window->SetWindowDockAllowFlags = enabled ? (window->SetWindowDockAllowFlags | flags) : (window->SetWindowDockAllowFlags & ~flags);
}

KGGuiWindow* Karma::KarmaGuiInternal::FindWindowByID(KGGuiID id)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	return (KGGuiWindow*)g.WindowsById.GetVoidPtr(id);
}

KGGuiWindow* Karma::KarmaGuiInternal::FindWindowByName(const char* name)
{
	KGGuiID id = KGHashStr(name);
	return Karma::KarmaGuiInternal::FindWindowByID(id);
}

static void ApplyWindowSettings(KGGuiWindow* window, KGGuiWindowSettings* settings)
{
	const KarmaGuiViewport* main_viewport = Karma::KarmaGui::GetMainViewport();
	window->ViewportPos = main_viewport->Pos;
	if (settings->ViewportId)
	{
		window->ViewportId = settings->ViewportId;
		window->ViewportPos = KGVec2(settings->ViewportPos.x, settings->ViewportPos.y);
	}
	window->Pos = KGFloor(KGVec2(settings->Pos.x + window->ViewportPos.x, settings->Pos.y + window->ViewportPos.y));
	if (settings->Size.x > 0 && settings->Size.y > 0)
		window->Size = window->SizeFull = KGFloor(KGVec2(settings->Size.x, settings->Size.y));
	window->Collapsed = settings->Collapsed;
	window->DockId = settings->DockId;
	window->DockOrder = settings->DockOrder;
}

static void UpdateWindowInFocusOrderList(KGGuiWindow* window, bool just_created, KarmaGuiWindowFlags new_flags)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;

	const bool new_is_explicit_child = (new_flags & KGGuiWindowFlags_ChildWindow) != 0 && ((new_flags & KGGuiWindowFlags_Popup) == 0 || (new_flags & KGGuiWindowFlags_ChildMenu) != 0);
	const bool child_flag_changed = new_is_explicit_child != window->IsExplicitChild;
	if ((just_created || child_flag_changed) && !new_is_explicit_child)
	{
		KR_CORE_ASSERT(!g.WindowsFocusOrder.contains(window), "");
		g.WindowsFocusOrder.push_back(window);
		window->FocusOrder = (short)(g.WindowsFocusOrder.Size - 1);
	}
	else if (!just_created && child_flag_changed && new_is_explicit_child)
	{
		KR_CORE_ASSERT(g.WindowsFocusOrder[window->FocusOrder] == window, "");
		for (int n = window->FocusOrder + 1; n < g.WindowsFocusOrder.Size; n++)
			g.WindowsFocusOrder[n]->FocusOrder--;
		g.WindowsFocusOrder.erase(g.WindowsFocusOrder.Data + window->FocusOrder);
		window->FocusOrder = -1;
	}
	window->IsExplicitChild = new_is_explicit_child;
}

static KGGuiWindow* CreateNewWindow(const char* name, KarmaGuiWindowFlags flags)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	//KARMAGUI_DEBUG_LOG("CreateNewWindow '%s', flags = 0x%08X\n", name, flags);

	// Create window the first time
	KGGuiWindow* window = KG_NEW(KGGuiWindow)(&g, name);
	window->Flags = flags;
	g.WindowsById.SetVoidPtr(window->ID, window);

	// Default/arbitrary window position. Use SetNextWindowPos() with the appropriate condition flag to change the initial position of a window.
	const KarmaGuiViewport* main_viewport = Karma::KarmaGui::GetMainViewport();
	window->Pos = main_viewport->Pos + KGVec2(60, 60);
	window->ViewportPos = main_viewport->Pos;

	// User can disable loading and saving of settings. Tooltip and child windows also don't store settings.
	if (!(flags & KGGuiWindowFlags_NoSavedSettings))
		if (KGGuiWindowSettings* settings = Karma::KarmaGuiInternal::FindWindowSettings(window->ID))
		{
			// Retrieve settings from .ini file
			window->SettingsOffset = g.SettingsWindows.offset_from_ptr(settings);
			SetWindowConditionAllowFlags(window, KGGuiCond_FirstUseEver, false);
			ApplyWindowSettings(window, settings);
		}
	window->DC.CursorStartPos = window->DC.CursorMaxPos = window->DC.IdealMaxPos = window->Pos; // So first call to CalcWindowContentSizes() doesn't return crazy values

	if ((flags & KGGuiWindowFlags_AlwaysAutoResize) != 0)
	{
		window->AutoFitFramesX = window->AutoFitFramesY = 2;
		window->AutoFitOnlyGrows = false;
	}
	else
	{
		if (window->Size.x <= 0.0f)
			window->AutoFitFramesX = 2;
		if (window->Size.y <= 0.0f)
			window->AutoFitFramesY = 2;
		window->AutoFitOnlyGrows = (window->AutoFitFramesX > 0) || (window->AutoFitFramesY > 0);
	}

	if (flags & KGGuiWindowFlags_NoBringToFrontOnFocus)
		g.Windows.push_front(window); // Quite slow but rare and only once
	else
		g.Windows.push_back(window);

	return window;
}

static KGGuiWindow* GetWindowForTitleDisplay(KGGuiWindow* window)
{
	return window->DockNodeAsHost ? window->DockNodeAsHost->VisibleWindow : window;
}

static KGGuiWindow* GetWindowForTitleAndMenuHeight(KGGuiWindow* window)
{
	return (window->DockNodeAsHost && window->DockNodeAsHost->VisibleWindow) ? window->DockNodeAsHost->VisibleWindow : window;
}

static KGVec2 CalcWindowSizeAfterConstraint(KGGuiWindow* window, const KGVec2& size_desired)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGVec2 new_size = size_desired;
	if (g.NextWindowData.Flags & KGGuiNextWindowDataFlags_HasSizeConstraint)
	{
		// Using -1,-1 on either X/Y axis to preserve the current size.
		KGRect cr = g.NextWindowData.SizeConstraintRect;
		new_size.x = (cr.Min.x >= 0 && cr.Max.x >= 0) ? KGClamp(new_size.x, cr.Min.x, cr.Max.x) : window->SizeFull.x;
		new_size.y = (cr.Min.y >= 0 && cr.Max.y >= 0) ? KGClamp(new_size.y, cr.Min.y, cr.Max.y) : window->SizeFull.y;
		if (g.NextWindowData.SizeCallback)
		{
			KarmaGuiSizeCallbackData data;
			data.UserData = g.NextWindowData.SizeCallbackUserData;
			data.Pos = window->Pos;
			data.CurrentSize = window->SizeFull;
			data.DesiredSize = new_size;
			g.NextWindowData.SizeCallback(&data);
			new_size = data.DesiredSize;
		}
		new_size.x = KG_FLOOR(new_size.x);
		new_size.y = KG_FLOOR(new_size.y);
	}

	// Minimum size
	if (!(window->Flags & (KGGuiWindowFlags_ChildWindow | KGGuiWindowFlags_AlwaysAutoResize)))
	{
		KGGuiWindow* window_for_height = GetWindowForTitleAndMenuHeight(window);
		new_size = KGMax(new_size, g.Style.WindowMinSize);
		const float minimum_height = window_for_height->TitleBarHeight() + window_for_height->MenuBarHeight() + KGMax(0.0f, g.Style.WindowRounding - 1.0f);
		new_size.y = KGMax(new_size.y, minimum_height); // Reduce artifacts with very small windows
	}
	return new_size;
}

static void CalcWindowContentSizes(KGGuiWindow* window, KGVec2* content_size_current, KGVec2* content_size_ideal)
{
	bool preserve_old_content_sizes = false;
	if (window->Collapsed && window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0)
		preserve_old_content_sizes = true;
	else if (window->Hidden && window->HiddenFramesCannotSkipItems == 0 && window->HiddenFramesCanSkipItems > 0)
		preserve_old_content_sizes = true;
	if (preserve_old_content_sizes)
	{
		*content_size_current = window->ContentSize;
		*content_size_ideal = window->ContentSizeIdeal;
		return;
	}

	content_size_current->x = (window->ContentSizeExplicit.x != 0.0f) ? window->ContentSizeExplicit.x : KG_FLOOR(window->DC.CursorMaxPos.x - window->DC.CursorStartPos.x);
	content_size_current->y = (window->ContentSizeExplicit.y != 0.0f) ? window->ContentSizeExplicit.y : KG_FLOOR(window->DC.CursorMaxPos.y - window->DC.CursorStartPos.y);
	content_size_ideal->x = (window->ContentSizeExplicit.x != 0.0f) ? window->ContentSizeExplicit.x : KG_FLOOR(KGMax(window->DC.CursorMaxPos.x, window->DC.IdealMaxPos.x) - window->DC.CursorStartPos.x);
	content_size_ideal->y = (window->ContentSizeExplicit.y != 0.0f) ? window->ContentSizeExplicit.y : KG_FLOOR(KGMax(window->DC.CursorMaxPos.y, window->DC.IdealMaxPos.y) - window->DC.CursorStartPos.y);
}

static KGVec2 CalcWindowAutoFitSize(KGGuiWindow* window, const KGVec2& size_contents)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KarmaGuiStyle& style = g.Style;
	const float decoration_w_without_scrollbars = window->DecoOuterSizeX1 + window->DecoOuterSizeX2 - window->ScrollbarSizes.x;
	const float decoration_h_without_scrollbars = window->DecoOuterSizeY1 + window->DecoOuterSizeY2 - window->ScrollbarSizes.y;
	KGVec2 size_pad = window->WindowPadding * 2.0f;
	KGVec2 size_desired = size_contents + size_pad + KGVec2(decoration_w_without_scrollbars, decoration_h_without_scrollbars);
	if (window->Flags & KGGuiWindowFlags_Tooltip)
	{
		// Tooltip always resize
		return size_desired;
	}
	else
	{
		// Maximum window size is determined by the viewport size or monitor size
		const bool is_popup = (window->Flags & KGGuiWindowFlags_Popup) != 0;
		const bool is_menu = (window->Flags & KGGuiWindowFlags_ChildMenu) != 0;
		KGVec2 size_min = style.WindowMinSize;
		if (is_popup || is_menu) // Popups and menus bypass style.WindowMinSize by default, but we give then a non-zero minimum size to facilitate understanding problematic cases (e.g. empty popups)
			size_min = KGMin(size_min, KGVec2(4.0f, 4.0f));

		KGVec2 avail_size = window->Viewport->WorkSize;
		if (window->ViewportOwned)
			avail_size = KGVec2(FLT_MAX, FLT_MAX);
		const int monitor_idx = window->ViewportAllowPlatformMonitorExtend;
		if (monitor_idx >= 0 && monitor_idx < g.PlatformIO.Monitors.Size)
			avail_size = g.PlatformIO.Monitors[monitor_idx].WorkSize;
		KGVec2 size_auto_fit = KGClamp(size_desired, size_min, KGMax(size_min, avail_size - style.DisplaySafeAreaPadding * 2.0f));

		// When the window cannot fit all contents (either because of constraints, either because screen is too small),
		// we are growing the size on the other axis to compensate for expected scrollbar. FIXME: Might turn bigger than ViewportSize-WindowPadding.
		KGVec2 size_auto_fit_after_constraint = CalcWindowSizeAfterConstraint(window, size_auto_fit);
		bool will_have_scrollbar_x = (size_auto_fit_after_constraint.x - size_pad.x - decoration_w_without_scrollbars < size_contents.x && !(window->Flags& KGGuiWindowFlags_NoScrollbar) && (window->Flags& KGGuiWindowFlags_HorizontalScrollbar)) || (window->Flags & KGGuiWindowFlags_AlwaysHorizontalScrollbar);
		bool will_have_scrollbar_y = (size_auto_fit_after_constraint.y - size_pad.y - decoration_h_without_scrollbars < size_contents.y && !(window->Flags& KGGuiWindowFlags_NoScrollbar)) || (window->Flags & KGGuiWindowFlags_AlwaysVerticalScrollbar);
		if (will_have_scrollbar_x)
			size_auto_fit.y += style.ScrollbarSize;
		if (will_have_scrollbar_y)
			size_auto_fit.x += style.ScrollbarSize;
		return size_auto_fit;
	}
}

KGVec2 Karma::KarmaGuiInternal::CalcWindowNextAutoFitSize(KGGuiWindow* window)
{
	KGVec2 size_contents_current;
	KGVec2 size_contents_ideal;
	CalcWindowContentSizes(window, &size_contents_current, &size_contents_ideal);
	KGVec2 size_auto_fit = CalcWindowAutoFitSize(window, size_contents_ideal);
	KGVec2 size_final = CalcWindowSizeAfterConstraint(window, size_auto_fit);
	return size_final;
}

static KarmaGuiCol GetWindowBgColorIdx(KGGuiWindow* window)
{
	if (window->Flags & (KGGuiWindowFlags_Tooltip | KGGuiWindowFlags_Popup))
		return KGGuiCol_PopupBg;
	if ((window->Flags & KGGuiWindowFlags_ChildWindow) && !window->DockIsActive)
		return KGGuiCol_ChildBg;
	return KGGuiCol_WindowBg;
}

static void CalcResizePosSizeFromAnyCorner(KGGuiWindow* window, const KGVec2& corner_target, const KGVec2& corner_norm, KGVec2* out_pos, KGVec2* out_size)
{
	KGVec2 pos_min = KGLerp(corner_target, window->Pos, corner_norm);                // Expected window upper-left
	KGVec2 pos_max = KGLerp(window->Pos + window->Size, corner_target, corner_norm); // Expected window lower-right
	KGVec2 size_expected = pos_max - pos_min;
	KGVec2 size_constrained = CalcWindowSizeAfterConstraint(window, size_expected);
	*out_pos = pos_min;
	if (corner_norm.x == 0.0f)
		out_pos->x -= (size_constrained.x - size_expected.x);
	if (corner_norm.y == 0.0f)
		out_pos->y -= (size_constrained.y - size_expected.y);
	*out_size = size_constrained;
}

// Data for resizing from corner
struct ImGuiResizeGripDef
{
	KGVec2  CornerPosN;
	KGVec2  InnerDir;
	int     AngleMin12, AngleMax12;
};
static const ImGuiResizeGripDef resize_grip_def[4] =
{
	{ KGVec2(1, 1), KGVec2(-1, -1), 0, 3 },  // Lower-right
	{ KGVec2(0, 1), KGVec2(+1, -1), 3, 6 },  // Lower-left
	{ KGVec2(0, 0), KGVec2(+1, +1), 6, 9 },  // Upper-left (Unused)
	{ KGVec2(1, 0), KGVec2(-1, +1), 9, 12 }  // Upper-right (Unused)
};

// Data for resizing from borders
struct ImGuiResizeBorderDef
{
	KGVec2 InnerDir;
	KGVec2 SegmentN1, SegmentN2;
	float  OuterAngle;
};
static const ImGuiResizeBorderDef resize_border_def[4] =
{
	{ KGVec2(+1, 0), KGVec2(0, 1), KGVec2(0, 0), KG_PI * 1.00f }, // Left
	{ KGVec2(-1, 0), KGVec2(1, 0), KGVec2(1, 1), KG_PI * 0.00f }, // Right
	{ KGVec2(0, +1), KGVec2(0, 0), KGVec2(1, 0), KG_PI * 1.50f }, // Up
	{ KGVec2(0, -1), KGVec2(1, 1), KGVec2(0, 1), KG_PI * 0.50f }  // Down
};

static KGRect GetResizeBorderRect(KGGuiWindow* window, int border_n, float perp_padding, float thickness)
{
	KGRect rect = window->Rect();
	if (thickness == 0.0f)
		rect.Max -= KGVec2(1, 1);
	if (border_n == KGGuiDir_Left) { return KGRect(rect.Min.x - thickness, rect.Min.y + perp_padding, rect.Min.x + thickness, rect.Max.y - perp_padding); }
	if (border_n == KGGuiDir_Right) { return KGRect(rect.Max.x - thickness, rect.Min.y + perp_padding, rect.Max.x + thickness, rect.Max.y - perp_padding); }
	if (border_n == KGGuiDir_Up) { return KGRect(rect.Min.x + perp_padding, rect.Min.y - thickness, rect.Max.x - perp_padding, rect.Min.y + thickness); }
	if (border_n == KGGuiDir_Down) { return KGRect(rect.Min.x + perp_padding, rect.Max.y - thickness, rect.Max.x - perp_padding, rect.Max.y + thickness); }
	KR_CORE_ASSERT(0, "");
	return KGRect();
}

// 0..3: corners (Lower-right, Lower-left, Unused, Unused)
KGGuiID Karma::KarmaGuiInternal::GetWindowResizeCornerID(KGGuiWindow* window, int n)
{
	KR_CORE_ASSERT(n >= 0 && n < 4, "");
	KGGuiID id = window->DockIsActive ? window->DockNode->HostWindow->ID : window->ID;
	id = KGHashStr("#RESIZE", 0, id);
	id = KGHashData(&n, sizeof(int), id);
	return id;
}

// Borders (Left, Right, Up, Down)
KGGuiID Karma::KarmaGuiInternal::GetWindowResizeBorderID(KGGuiWindow* window, KarmaGuiDir dir)
{
	KR_CORE_ASSERT(dir >= 0 && dir < 4, "");
	int n = (int)dir + 4;
	KGGuiID id = window->DockIsActive ? window->DockNode->HostWindow->ID : window->ID;
	id = KGHashStr("#RESIZE", 0, id);
	id = KGHashData(&n, sizeof(int), id);
	return id;
}

// Handle resize for: Resize Grips, Borders, Gamepad
// Return true when using auto-fit (double-click on resize grip)
bool Karma::KarmaGuiInternal::UpdateWindowManualResize(KGGuiWindow* window, const KGVec2& size_auto_fit, int* border_held, int resize_grip_count, KGU32 resize_grip_col[4], const KGRect& visibility_rect)
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiWindowFlags flags = window->Flags;

	if ((flags & KGGuiWindowFlags_NoResize) || (flags & KGGuiWindowFlags_AlwaysAutoResize) || window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0)
		return false;
	if (window->WasActive == false) // Early out to avoid running this code for e.g. a hidden implicit/fallback Debug window.
		return false;

	bool ret_auto_fit = false;
	const int resize_border_count = g.IO.ConfigWindowsResizeFromEdges ? 4 : 0;
	const float grip_draw_size = KG_FLOOR(KGMax(g.FontSize * 1.35f, window->WindowRounding + 1.0f + g.FontSize * 0.2f));
	const float grip_hover_inner_size = KG_FLOOR(grip_draw_size * 0.75f);
	const float grip_hover_outer_size = g.IO.ConfigWindowsResizeFromEdges ? WINDOWS_HOVER_PADDING : 0.0f;

	KGVec2 pos_target(FLT_MAX, FLT_MAX);
	KGVec2 size_target(FLT_MAX, FLT_MAX);

	// Clip mouse interaction rectangles within the viewport rectangle (in practice the narrowing is going to happen most of the time).
	// - Not narrowing would mostly benefit the situation where OS windows _without_ decoration have a threshold for hovering when outside their limits.
	//   This is however not the case with current backends under Win32, but a custom borderless window implementation would benefit from it.
	// - When decoration are enabled we typically benefit from that distance, but then our resize elements would be conflicting with OS resize elements, so we also narrow.
	// - Note that we are unable to tell if the platform setup allows hovering with a distance threshold (on Win32, decorated window have such threshold).
	// We only clip interaction so we overwrite window->ClipRect, cannot call PushClipRect() yet as DrawList is not yet setup.
	const bool clip_with_viewport_rect = !(g.IO.BackendFlags & KGGuiBackendFlags_HasMouseHoveredViewport) || (g.IO.MouseHoveredViewport != window->ViewportId) || !(window->Viewport->Flags & KGGuiViewportFlags_NoDecoration);
	if (clip_with_viewport_rect)
		window->ClipRect = window->Viewport->GetMainRect();

	// Resize grips and borders are on layer 1
	window->DC.NavLayerCurrent = KGGuiNavLayer_Menu;

	// Manual resize grips
	KarmaGui::PushID("#RESIZE");
	for (int resize_grip_n = 0; resize_grip_n < resize_grip_count; resize_grip_n++)
	{
		const ImGuiResizeGripDef& def = resize_grip_def[resize_grip_n];
		const KGVec2 corner = KGLerp(window->Pos, window->Pos + window->Size, def.CornerPosN);

		// Using the FlattenChilds button flag we make the resize button accessible even if we are hovering over a child window
		bool hovered, held;
		KGRect resize_rect(corner - def.InnerDir * grip_hover_outer_size, corner + def.InnerDir * grip_hover_inner_size);
		if (resize_rect.Min.x > resize_rect.Max.x) KGSwap(resize_rect.Min.x, resize_rect.Max.x);
		if (resize_rect.Min.y > resize_rect.Max.y) KGSwap(resize_rect.Min.y, resize_rect.Max.y);
		KGGuiID resize_grip_id = window->GetID(resize_grip_n); // == GetWindowResizeCornerID()
		ItemAdd(resize_rect, resize_grip_id, NULL, KGGuiItemFlags_NoNav);
		ButtonBehavior(resize_rect, resize_grip_id, &hovered, &held, KGGuiButtonFlags_FlattenChildren | KGGuiButtonFlags_NoNavFocus);
		//GetForegroundDrawList(window)->AddRect(resize_rect.Min, resize_rect.Max, KG_COL32(255, 255, 0, 255));
		if (hovered || held)
			g.MouseCursor = (resize_grip_n & 1) ? KGGuiMouseCursor_ResizeNESW : KGGuiMouseCursor_ResizeNWSE;

		if (held && g.IO.MouseClickedCount[0] == 2 && resize_grip_n == 0)
		{
			// Manual auto-fit when double-clicking
			size_target = CalcWindowSizeAfterConstraint(window, size_auto_fit);
			ret_auto_fit = true;
			ClearActiveID();
		}
		else if (held)
		{
			// Resize from any of the four corners
			// We don't use an incremental MouseDelta but rather compute an absolute target size based on mouse position
			KGVec2 clamp_min = KGVec2(def.CornerPosN.x == 1.0f ? visibility_rect.Min.x : -FLT_MAX, def.CornerPosN.y == 1.0f ? visibility_rect.Min.y : -FLT_MAX);
			KGVec2 clamp_max = KGVec2(def.CornerPosN.x == 0.0f ? visibility_rect.Max.x : +FLT_MAX, def.CornerPosN.y == 0.0f ? visibility_rect.Max.y : +FLT_MAX);
			KGVec2 corner_target = g.IO.MousePos - g.ActiveIdClickOffset + KGLerp(def.InnerDir * grip_hover_outer_size, def.InnerDir * -grip_hover_inner_size, def.CornerPosN); // Corner of the window corresponding to our corner grip
			corner_target = KGClamp(corner_target, clamp_min, clamp_max);
			CalcResizePosSizeFromAnyCorner(window, corner_target, def.CornerPosN, &pos_target, &size_target);
		}

		// Only lower-left grip is visible before hovering/activating
		if (resize_grip_n == 0 || held || hovered)
			resize_grip_col[resize_grip_n] = KarmaGui::GetColorU32(held ? KGGuiCol_ResizeGripActive : hovered ? KGGuiCol_ResizeGripHovered : KGGuiCol_ResizeGrip);
	}
	for (int border_n = 0; border_n < resize_border_count; border_n++)
	{
		const ImGuiResizeBorderDef& def = resize_border_def[border_n];
		const KGGuiAxis axis = (border_n == KGGuiDir_Left || border_n == KGGuiDir_Right) ? KGGuiAxis_X : KGGuiAxis_Y;

		bool hovered, held;
		KGRect border_rect = GetResizeBorderRect(window, border_n, grip_hover_inner_size, WINDOWS_HOVER_PADDING);
		KGGuiID border_id = window->GetID(border_n + 4); // == GetWindowResizeBorderID()
		ItemAdd(border_rect, border_id, NULL, KGGuiItemFlags_NoNav);
		ButtonBehavior(border_rect, border_id, &hovered, &held, KGGuiButtonFlags_FlattenChildren | KGGuiButtonFlags_NoNavFocus);
		//GetForegroundDrawLists(window)->AddRect(border_rect.Min, border_rect.Max, KG_COL32(255, 255, 0, 255));
		if ((hovered && g.HoveredIdTimer > WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER) || held)
		{
			g.MouseCursor = (axis == KGGuiAxis_X) ? KGGuiMouseCursor_ResizeEW : KGGuiMouseCursor_ResizeNS;
			if (held)
				*border_held = border_n;
		}
		if (held)
		{
			KGVec2 clamp_min(border_n == KGGuiDir_Right ? visibility_rect.Min.x : -FLT_MAX, border_n == KGGuiDir_Down ? visibility_rect.Min.y : -FLT_MAX);
			KGVec2 clamp_max(border_n == KGGuiDir_Left ? visibility_rect.Max.x : +FLT_MAX, border_n == KGGuiDir_Up ? visibility_rect.Max.y : +FLT_MAX);
			KGVec2 border_target = window->Pos;
			border_target[axis] = g.IO.MousePos[axis] - g.ActiveIdClickOffset[axis] + WINDOWS_HOVER_PADDING;
			border_target = KGClamp(border_target, clamp_min, clamp_max);
			CalcResizePosSizeFromAnyCorner(window, border_target, KGMin(def.SegmentN1, def.SegmentN2), &pos_target, &size_target);
		}
	}
	KarmaGui::PopID();

	// Restore nav layer
	window->DC.NavLayerCurrent = KGGuiNavLayer_Main;

	// Navigation resize (keyboard/gamepad)
	// FIXME: This cannot be moved to NavUpdateWindowing() because CalcWindowSizeAfterConstraint() need to callback into user.
	// Not even sure the callback works here.
	if (g.NavWindowingTarget && g.NavWindowingTarget->RootWindowDockTree == window)
	{
		KGVec2 nav_resize_dir;
		if (g.NavInputSource == KGGuiInputSource_Keyboard && g.IO.KeyShift)
			nav_resize_dir = GetKeyMagnitude2d(KGGuiKey_LeftArrow, KGGuiKey_RightArrow, KGGuiKey_UpArrow, KGGuiKey_DownArrow);
		if (g.NavInputSource == KGGuiInputSource_Gamepad)
			nav_resize_dir = GetKeyMagnitude2d(KGGuiKey_GamepadDpadLeft, KGGuiKey_GamepadDpadRight, KGGuiKey_GamepadDpadUp, KGGuiKey_GamepadDpadDown);
		if (nav_resize_dir.x != 0.0f || nav_resize_dir.y != 0.0f)
		{
			const float NAV_RESIZE_SPEED = 600.0f;
			const float resize_step = NAV_RESIZE_SPEED * g.IO.DeltaTime * KGMin(g.IO.DisplayFramebufferScale.x, g.IO.DisplayFramebufferScale.y);
			g.NavWindowingAccumDeltaSize += nav_resize_dir * resize_step;
			g.NavWindowingAccumDeltaSize = KGMax(g.NavWindowingAccumDeltaSize, visibility_rect.Min - window->Pos - window->Size); // We need Pos+Size >= visibility_rect.Min, so Size >= visibility_rect.Min - Pos, so size_delta >= visibility_rect.Min - window->Pos - window->Size
			g.NavWindowingToggleLayer = false;
			g.NavDisableMouseHover = true;
			resize_grip_col[0] = KarmaGui::GetColorU32(KGGuiCol_ResizeGripActive);
			KGVec2 accum_floored = KGFloor(g.NavWindowingAccumDeltaSize);
			if (accum_floored.x != 0.0f || accum_floored.y != 0.0f)
			{
				// FIXME-NAV: Should store and accumulate into a separate size buffer to handle sizing constraints properly, right now a constraint will make us stuck.
				size_target = CalcWindowSizeAfterConstraint(window, window->SizeFull + accum_floored);
				g.NavWindowingAccumDeltaSize -= accum_floored;
			}
		}
	}

	// Apply back modified position/size to window
	if (size_target.x != FLT_MAX)
	{
		window->SizeFull = size_target;
		MarkIniSettingsDirty(window);
	}
	if (pos_target.x != FLT_MAX)
	{
		window->Pos = KGFloor(pos_target);
		MarkIniSettingsDirty(window);
	}

	window->Size = window->SizeFull;
	return ret_auto_fit;
}

static inline void ClampWindowPos(KGGuiWindow* window, const KGRect& visibility_rect)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGVec2 size_for_clamping = window->Size;
	if (g.IO.ConfigWindowsMoveFromTitleBarOnly && (!(window->Flags & KGGuiWindowFlags_NoTitleBar) || window->DockNodeAsHost))
		size_for_clamping.y = Karma::KarmaGui::GetFrameHeight(); // Not using window->TitleBarHeight() as DockNodeAsHost will report 0.0f here.
	window->Pos = KGClamp(window->Pos, visibility_rect.Min - size_for_clamping, visibility_rect.Max);
}

void Karma::KarmaGuiInternal::RenderWindowOuterBorders(KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;
	float rounding = window->WindowRounding;
	float border_size = window->WindowBorderSize;
	if (border_size > 0.0f && !(window->Flags & KGGuiWindowFlags_NoBackground))
		window->DrawList->AddRect(window->Pos, window->Pos + window->Size, KarmaGui::GetColorU32(KGGuiCol_Border), rounding, 0, border_size);

	int border_held = window->ResizeBorderHeld;
	if (border_held != -1)
	{
		const ImGuiResizeBorderDef& def = resize_border_def[border_held];
		KGRect border_r = GetResizeBorderRect(window, border_held, rounding, 0.0f);
		window->DrawList->PathArcTo(KGLerp(border_r.Min, border_r.Max, def.SegmentN1) + KGVec2(0.5f, 0.5f) + def.InnerDir * rounding, rounding, def.OuterAngle - KG_PI * 0.25f, def.OuterAngle);
		window->DrawList->PathArcTo(KGLerp(border_r.Min, border_r.Max, def.SegmentN2) + KGVec2(0.5f, 0.5f) + def.InnerDir * rounding, rounding, def.OuterAngle, def.OuterAngle + KG_PI * 0.25f);
		window->DrawList->PathStroke(KarmaGui::GetColorU32(KGGuiCol_SeparatorActive), 0, KGMax(2.0f, border_size)); // Thicker than usual
	}
	if (g.Style.FrameBorderSize > 0 && !(window->Flags & KGGuiWindowFlags_NoTitleBar) && !window->DockIsActive)
	{
		float y = window->Pos.y + window->TitleBarHeight() - 1;
		window->DrawList->AddLine(KGVec2(window->Pos.x + border_size, y), KGVec2(window->Pos.x + window->Size.x - border_size, y), KarmaGui::GetColorU32(KGGuiCol_Border), g.Style.FrameBorderSize);
	}
}

KGFont* Karma::KarmaGuiInternal::GetDefaultFont()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.IO.FontDefault ? g.IO.FontDefault : g.IO.Fonts->Fonts[0];
}

KGGuiWindow* Karma::KarmaGuiInternal::GetCurrentWindow()
{
	KarmaGuiContext& g = *GKarmaGui;
	g.CurrentWindow->WriteAccessed = true;
	return g.CurrentWindow;
}

// Draw background and borders
// Draw and handle scrollbars
void Karma::KarmaGuiInternal::RenderWindowDecorations(KGGuiWindow* window, const KGRect& title_bar_rect, bool title_bar_is_highlight, bool handle_borders_and_resize_grips, int resize_grip_count, const KGU32 resize_grip_col[4], float resize_grip_draw_size)
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiStyle& style = g.Style;
	KarmaGuiWindowFlags flags = window->Flags;

	// Ensure that ScrollBar doesn't read last frame's SkipItems
	KR_CORE_ASSERT(window->BeginCount == 0, "");
	window->SkipItems = false;

	// Draw window + handle manual resize
	// As we highlight the title bar when want_focus is set, multiple reappearing windows will have their title bar highlighted on their reappearing frame.
	const float window_rounding = window->WindowRounding;
	const float window_border_size = window->WindowBorderSize;
	if (window->Collapsed)
	{
		// Title bar only
		const float backup_border_size = style.FrameBorderSize;
		g.Style.FrameBorderSize = window->WindowBorderSize;
		KGU32 title_bar_col = KarmaGui::GetColorU32((title_bar_is_highlight && !g.NavDisableHighlight) ? KGGuiCol_TitleBgActive : KGGuiCol_TitleBgCollapsed);
		if (window->ViewportOwned)
			title_bar_col |= KG_COL32_A_MASK; // No alpha (we don't support is_docking_transparent_payload here because simpler and less meaningful, but could with a bit of code shuffle/reuse)
		RenderFrame(title_bar_rect.Min, title_bar_rect.Max, title_bar_col, true, window_rounding);
		g.Style.FrameBorderSize = backup_border_size;
	}
	else
	{
		// Window background
		if (!(flags & KGGuiWindowFlags_NoBackground))
		{
			bool is_docking_transparent_payload = false;
			if (g.DragDropActive && (g.FrameCount - g.DragDropAcceptFrameCount) <= 1 && g.IO.ConfigDockingTransparentPayload)
				if (g.DragDropPayload.IsDataType(KARMAGUI_PAYLOAD_TYPE_WINDOW) && *(KGGuiWindow**)g.DragDropPayload.Data == window)
					is_docking_transparent_payload = true;

			KGU32 bg_col = KarmaGui::GetColorU32(GetWindowBgColorIdx(window));

			if (window->bUseCustomBgColor)
			{
				bg_col = window->BgColor;
			}

			if (window->ViewportOwned)
			{
				bg_col |= KG_COL32_A_MASK; // No alpha
				if (is_docking_transparent_payload)
					window->Viewport->Alpha *= DOCKING_TRANSPARENT_PAYLOAD_ALPHA;
			}
			else
			{
				// Adjust alpha. For docking
				bool override_alpha = false;
				float alpha = 1.0f;
				if (g.NextWindowData.Flags & KGGuiNextWindowDataFlags_HasBgAlpha)
				{
					alpha = g.NextWindowData.BgAlphaVal;
					override_alpha = true;
				}
				if (is_docking_transparent_payload)
				{
					alpha *= DOCKING_TRANSPARENT_PAYLOAD_ALPHA; // FIXME-DOCK: Should that be an override?
					override_alpha = true;
				}
				if (override_alpha)
					bg_col = (bg_col & ~KG_COL32_A_MASK) | (KG_F32_TO_INT8_SAT(alpha) << KG_COL32_A_SHIFT);
			}

			// Render, for docked windows and host windows we ensure bg goes before decorations
			if (window->DockIsActive)
				window->DockNode->LastBgColor = bg_col;
			KGDrawList* bg_draw_list = window->DockIsActive ? window->DockNode->HostWindow->DrawList : window->DrawList;
			if (window->DockIsActive || (flags & KGGuiWindowFlags_DockNodeHost))
				bg_draw_list->ChannelsSetCurrent(DOCKING_HOST_DRAW_CHANNEL_BG);
			bg_draw_list->AddRectFilled(window->Pos + KGVec2(0, window->TitleBarHeight()), window->Pos + window->Size, bg_col, window_rounding, (flags & KGGuiWindowFlags_NoTitleBar) ? 0 : KGDrawFlags_RoundCornersBottom);
			if (window->DockIsActive || (flags & KGGuiWindowFlags_DockNodeHost))
				bg_draw_list->ChannelsSetCurrent(DOCKING_HOST_DRAW_CHANNEL_FG);
		}
		if (window->DockIsActive)
			window->DockNode->IsBgDrawnThisFrame = true;

		// Title bar
		// (when docked, DockNode are drawing their own title bar. Individual windows however do NOT set the _NoTitleBar flag,
		// in order for their pos/size to be matching their undocking state.)
		if (!(flags & KGGuiWindowFlags_NoTitleBar) && !window->DockIsActive)
		{
			KGU32 title_bar_col = KarmaGui::GetColorU32(title_bar_is_highlight ? KGGuiCol_TitleBgActive : KGGuiCol_TitleBg);
			window->DrawList->AddRectFilled(title_bar_rect.Min, title_bar_rect.Max, title_bar_col, window_rounding, KGDrawFlags_RoundCornersTop);
		}

		// Menu bar
		if (flags & KGGuiWindowFlags_MenuBar)
		{
			KGRect menu_bar_rect = window->MenuBarRect();
			menu_bar_rect.ClipWith(window->Rect());  // Soft clipping, in particular child window don't have minimum size covering the menu bar so this is useful for them.
			window->DrawList->AddRectFilled(menu_bar_rect.Min + KGVec2(window_border_size, 0), menu_bar_rect.Max - KGVec2(window_border_size, 0), KarmaGui::GetColorU32(KGGuiCol_MenuBarBg), (flags & KGGuiWindowFlags_NoTitleBar) ? window_rounding : 0.0f, KGDrawFlags_RoundCornersTop);
			if (style.FrameBorderSize > 0.0f && menu_bar_rect.Max.y < window->Pos.y + window->Size.y)
				window->DrawList->AddLine(menu_bar_rect.GetBL(), menu_bar_rect.GetBR(), KarmaGui::GetColorU32(KGGuiCol_Border), style.FrameBorderSize);
		}

		// Docking: Unhide tab bar (small triangle in the corner), drag from small triangle to quickly undock
		KGGuiDockNode* node = window->DockNode;
		if (window->DockIsActive && node->IsHiddenTabBar() && !node->IsNoTabBar())
		{
			float unhide_sz_draw = KGFloor(g.FontSize * 0.70f);
			float unhide_sz_hit = KGFloor(g.FontSize * 0.55f);
			KGVec2 p = node->Pos;
			KGRect r(p, p + KGVec2(unhide_sz_hit, unhide_sz_hit));
			KGGuiID unhide_id = window->GetID("#UNHIDE");
			KeepAliveID(unhide_id);
			bool hovered, held;
			if (ButtonBehavior(r, unhide_id, &hovered, &held, KGGuiButtonFlags_FlattenChildren))
				node->WantHiddenTabBarToggle = true;
			else if (held && KarmaGui::IsMouseDragging(0))
				StartMouseMovingWindowOrNode(window, node, true);

			// FIXME-DOCK: Ideally we'd use KGGuiCol_TitleBgActive/KGGuiCol_TitleBg here, but neither is guaranteed to be visible enough at this sort of size..
			KGU32 col = KarmaGui::GetColorU32(((held && hovered) || (node->IsFocused && !hovered)) ? KGGuiCol_ButtonActive : hovered ? KGGuiCol_ButtonHovered : KGGuiCol_Button);
			window->DrawList->AddTriangleFilled(p, p + KGVec2(unhide_sz_draw, 0.0f), p + KGVec2(0.0f, unhide_sz_draw), col);
		}

		// Scrollbars
		if (window->ScrollbarX)
			Scrollbar(KGGuiAxis_X);
		if (window->ScrollbarY)
			Scrollbar(KGGuiAxis_Y);

		// Render resize grips (after their input handling so we don't have a frame of latency)
		if (handle_borders_and_resize_grips && !(flags & KGGuiWindowFlags_NoResize))
		{
			for (int resize_grip_n = 0; resize_grip_n < resize_grip_count; resize_grip_n++)
			{
				const ImGuiResizeGripDef& grip = resize_grip_def[resize_grip_n];
				const KGVec2 corner = KGLerp(window->Pos, window->Pos + window->Size, grip.CornerPosN);
				window->DrawList->PathLineTo(corner + grip.InnerDir * ((resize_grip_n & 1) ? KGVec2(window_border_size, resize_grip_draw_size) : KGVec2(resize_grip_draw_size, window_border_size)));
				window->DrawList->PathLineTo(corner + grip.InnerDir * ((resize_grip_n & 1) ? KGVec2(resize_grip_draw_size, window_border_size) : KGVec2(window_border_size, resize_grip_draw_size)));
				window->DrawList->PathArcToFast(KGVec2(corner.x + grip.InnerDir.x * (window_rounding + window_border_size), corner.y + grip.InnerDir.y * (window_rounding + window_border_size)), window_rounding, grip.AngleMin12, grip.AngleMax12);
				window->DrawList->PathFillConvex(resize_grip_col[resize_grip_n]);
			}
		}

		// Borders (for dock node host they will be rendered over after the tab bar)
		if (handle_borders_and_resize_grips && !window->DockNodeAsHost)
			RenderWindowOuterBorders(window);
	}
}

// When inside a dock node, this is handled in DockNodeCalcTabBarLayout() instead.
// Render title text, collapse button, close button
void Karma::KarmaGuiInternal::RenderWindowTitleBarContents(KGGuiWindow* window, const KGRect& title_bar_rect, const char* name, bool* p_open)
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiStyle& style = g.Style;
	KarmaGuiWindowFlags flags = window->Flags;

	const bool has_close_button = (p_open != NULL);
	const bool has_collapse_button = !(flags & KGGuiWindowFlags_NoCollapse) && (style.WindowMenuButtonPosition != KGGuiDir_None);

	// Close & Collapse button are on the Menu NavLayer and don't default focus (unless there's nothing else on that layer)
	// FIXME-NAV: Might want (or not?) to set the equivalent of KGGuiButtonFlags_NoNavFocus so that mouse clicks on standard title bar items don't necessarily set nav/keyboard ref?
	const KGGuiItemFlags item_flags_backup = g.CurrentItemFlags;
	g.CurrentItemFlags |= KGGuiItemFlags_NoNavDefaultFocus;
	window->DC.NavLayerCurrent = KGGuiNavLayer_Menu;

	// Layout buttons
	// FIXME: Would be nice to generalize the subtleties expressed here into reusable code.
	float pad_l = style.FramePadding.x;
	float pad_r = style.FramePadding.x;
	float button_sz = g.FontSize;
	KGVec2 close_button_pos;
	KGVec2 collapse_button_pos;
	if (has_close_button)
	{
		pad_r += button_sz;
		close_button_pos = KGVec2(title_bar_rect.Max.x - pad_r - style.FramePadding.x, title_bar_rect.Min.y);
	}
	if (has_collapse_button && style.WindowMenuButtonPosition == KGGuiDir_Right)
	{
		pad_r += button_sz;
		collapse_button_pos = KGVec2(title_bar_rect.Max.x - pad_r - style.FramePadding.x, title_bar_rect.Min.y);
	}
	if (has_collapse_button && style.WindowMenuButtonPosition == KGGuiDir_Left)
	{
		collapse_button_pos = KGVec2(title_bar_rect.Min.x + pad_l - style.FramePadding.x, title_bar_rect.Min.y);
		pad_l += button_sz;
	}

	// Collapse button (submitting first so it gets priority when choosing a navigation init fallback)
	if (has_collapse_button)
		if (CollapseButton(window->GetID("#COLLAPSE"), collapse_button_pos, NULL))
			window->WantCollapseToggle = true; // Defer actual collapsing to next frame as we are too far in the Begin() function

	// Close button
	if (has_close_button)
		if (CloseButton(window->GetID("#CLOSE"), close_button_pos))
			*p_open = false;

	window->DC.NavLayerCurrent = KGGuiNavLayer_Main;
	g.CurrentItemFlags = item_flags_backup;

	// Title bar text (with: horizontal alignment, avoiding collapse/close button, optional "unsaved document" marker)
	// FIXME: Refactor text alignment facilities along with RenderText helpers, this is WAY too much messy code..
	const float marker_size_x = (flags & KGGuiWindowFlags_UnsavedDocument) ? button_sz * 0.80f : 0.0f;
	const KGVec2 text_size = KarmaGui::CalcTextSize(name, NULL, true) + KGVec2(marker_size_x, 0.0f);

	// As a nice touch we try to ensure that centered title text doesn't get affected by visibility of Close/Collapse button,
	// while uncentered title text will still reach edges correctly.
	if (pad_l > style.FramePadding.x)
		pad_l += g.Style.ItemInnerSpacing.x;
	if (pad_r > style.FramePadding.x)
		pad_r += g.Style.ItemInnerSpacing.x;
	if (style.WindowTitleAlign.x > 0.0f && style.WindowTitleAlign.x < 1.0f)
	{
		float centerness = KGSaturate(1.0f - KGFabs(style.WindowTitleAlign.x - 0.5f) * 2.0f); // 0.0f on either edges, 1.0f on center
		float pad_extend = KGMin(KGMax(pad_l, pad_r), title_bar_rect.GetWidth() - pad_l - pad_r - text_size.x);
		pad_l = KGMax(pad_l, pad_extend * centerness);
		pad_r = KGMax(pad_r, pad_extend * centerness);
	}

	KGRect layout_r(title_bar_rect.Min.x + pad_l, title_bar_rect.Min.y, title_bar_rect.Max.x - pad_r, title_bar_rect.Max.y);
	KGRect clip_r(layout_r.Min.x, layout_r.Min.y, KGMin(layout_r.Max.x + g.Style.ItemInnerSpacing.x, title_bar_rect.Max.x), layout_r.Max.y);
	if (flags & KGGuiWindowFlags_UnsavedDocument)
	{
		KGVec2 marker_pos;
		marker_pos.x = KGClamp(layout_r.Min.x + (layout_r.GetWidth() - text_size.x) * style.WindowTitleAlign.x + text_size.x, layout_r.Min.x, layout_r.Max.x);
		marker_pos.y = (layout_r.Min.y + layout_r.Max.y) * 0.5f;
		if (marker_pos.x > layout_r.Min.x)
		{
			RenderBullet(window->DrawList, marker_pos, KarmaGui::GetColorU32(KGGuiCol_Text));
			clip_r.Max.x = KGMin(clip_r.Max.x, marker_pos.x - (int)(marker_size_x * 0.5f));
		}
	}
	//if (g.IO.KeyShift) window->DrawList->AddRect(layout_r.Min, layout_r.Max, KG_COL32(255, 128, 0, 255)); // [DEBUG]
	//if (g.IO.KeyCtrl) window->DrawList->AddRect(clip_r.Min, clip_r.Max, KG_COL32(255, 128, 0, 255)); // [DEBUG]
	RenderTextClipped(layout_r.Min, layout_r.Max, name, NULL, &text_size, style.WindowTitleAlign, &clip_r);
}

void Karma::KarmaGuiInternal::UpdateWindowParentAndRootLinks(KGGuiWindow* window, KarmaGuiWindowFlags flags, KGGuiWindow* parent_window)
{
	window->ParentWindow = parent_window;
	window->RootWindow = window->RootWindowPopupTree = window->RootWindowDockTree = window->RootWindowForTitleBarHighlight = window->RootWindowForNav = window;
	if (parent_window && (flags & KGGuiWindowFlags_ChildWindow) && !(flags & KGGuiWindowFlags_Tooltip))
	{
		window->RootWindowDockTree = parent_window->RootWindowDockTree;
		if (!window->DockIsActive && !(parent_window->Flags & KGGuiWindowFlags_DockNodeHost))
			window->RootWindow = parent_window->RootWindow;
	}
	if (parent_window && (flags & KGGuiWindowFlags_Popup))
		window->RootWindowPopupTree = parent_window->RootWindowPopupTree;
	if (parent_window && !(flags & KGGuiWindowFlags_Modal) && (flags & (KGGuiWindowFlags_ChildWindow | KGGuiWindowFlags_Popup))) // FIXME: simply use _NoTitleBar ?
		window->RootWindowForTitleBarHighlight = parent_window->RootWindowForTitleBarHighlight;
	while (window->RootWindowForNav->Flags & KGGuiWindowFlags_NavFlattened)
	{
		KR_CORE_ASSERT(window->RootWindowForNav->ParentWindow != NULL, "");
		window->RootWindowForNav = window->RootWindowForNav->ParentWindow;
	}
}

// When a modal popup is open, newly created windows that want focus (i.e. are not popups and do not specify KGGuiWindowFlags_NoFocusOnAppearing)
// should be positioned behind that modal window, unless the window was created inside the modal begin-stack.
// In case of multiple stacked modals newly created window honors begin stack order and does not go below its own modal parent.
// - Window             // FindBlockingModal() returns Modal1
//   - Window           //                  .. returns Modal1
//   - Modal1           //                  .. returns Modal2
//      - Window        //                  .. returns Modal2
//          - Window    //                  .. returns Modal2
//          - Modal2    //                  .. returns Modal2
KGGuiWindow* Karma::KarmaGuiInternal::FindBlockingModal(KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.OpenPopupStack.Size <= 0)
		return NULL;

	// Find a modal that has common parent with specified window. Specified window should be positioned behind that modal.
	for (int i = g.OpenPopupStack.Size - 1; i >= 0; i--)
	{
		KGGuiWindow* popup_window = g.OpenPopupStack.Data[i].Window;
		if (popup_window == NULL || !(popup_window->Flags & KGGuiWindowFlags_Modal))
			continue;
		if (!popup_window->Active && !popup_window->WasActive)      // Check WasActive, because this code may run before popup renders on current frame, also check Active to handle newly created windows.
			continue;
		if (IsWindowWithinBeginStackOf(window, popup_window))       // Window is rendered over last modal, no render order change needed.
			break;
		for (KGGuiWindow* parent = popup_window->ParentWindowInBeginStack->RootWindow; parent != NULL; parent = parent->ParentWindowInBeginStack->RootWindow)
			if (IsWindowWithinBeginStackOf(window, parent))
				return popup_window;                                // Place window above its begin stack parent.
	}
	return NULL;
}

// Push a new Dear ImGui window to add widgets to.
// - A default window called "Debug" is automatically stacked at the beginning of every frame so you can use widgets without explicitly calling a Begin/End pair.
// - Begin/End can be called multiple times during the frame with the same window name to append content.
// - The window name is used as a unique identifier to preserve window information across frames (and save rudimentary information to the .ini file).
//   You can use the "##" or "###" markers to use the same label with different id, or same id with different label. See documentation at the top of this file.
// - Return false when window is collapsed, so you can early out in your code. You always need to call KarmaGui::End() even if false is returned.
// - Passing 'bool* p_open' displays a Close button on the upper-right corner of the window, the pointed value will be set to false when the button is pressed.
bool Karma::KarmaGui::Begin(const char* name, bool* p_open, KarmaGuiWindowFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	const KarmaGuiStyle& style = g.Style;
	KR_CORE_ASSERT(name != NULL && name[0] != '\0', "");     // Window name required
	KR_CORE_ASSERT(g.WithinFrameScope, "");                  // Forgot to call KarmaGui::NewFrame()
	KR_CORE_ASSERT(g.FrameCountEnded != g.FrameCount, "");   // Called KarmaGui::Render() or KarmaGui::EndFrame() and haven't called KarmaGui::NewFrame() again yet

	// Find or create
	KGGuiWindow* window = KarmaGuiInternal::FindWindowByName(name);
	const bool window_just_created = (window == NULL);
	if (window_just_created)
		window = CreateNewWindow(name, flags);

	// Automatically disable manual moving/resizing when NoInputs is set
	if ((flags & KGGuiWindowFlags_NoInputs) == KGGuiWindowFlags_NoInputs)
		flags |= KGGuiWindowFlags_NoMove | KGGuiWindowFlags_NoResize;

	if (flags & KGGuiWindowFlags_NavFlattened)
	{
		KR_CORE_ASSERT(flags & KGGuiWindowFlags_ChildWindow, "");
	}

	const int current_frame = g.FrameCount;
	const bool first_begin_of_the_frame = (window->LastFrameActive != current_frame);
	window->IsFallbackWindow = (g.CurrentWindowStack.Size == 0 && g.WithinFrameScopeWithImplicitWindow);

	// Update the Appearing flag (note: the BeginDocked() path may also set this to true later)
	bool window_just_activated_by_user = (window->LastFrameActive < current_frame - 1); // Not using !WasActive because the implicit "Debug" window would always toggle off->on
	if (flags & KGGuiWindowFlags_Popup)
	{
		KGGuiPopupData& popup_ref = g.OpenPopupStack[g.BeginPopupStack.Size];
		window_just_activated_by_user |= (window->PopupId != popup_ref.PopupId); // We recycle popups so treat window as activated if popup id changed
		window_just_activated_by_user |= (window != popup_ref.Window);
	}

	// Update Flags, LastFrameActive, BeginOrderXXX fields
	const bool window_was_appearing = window->Appearing;
	if (first_begin_of_the_frame)
	{
		UpdateWindowInFocusOrderList(window, window_just_created, flags);
		window->Appearing = window_just_activated_by_user;
		if (window->Appearing)
			SetWindowConditionAllowFlags(window, KGGuiCond_Appearing, true);
		window->FlagsPreviousFrame = window->Flags;
		window->Flags = (KarmaGuiWindowFlags)flags;
		window->LastFrameActive = current_frame;
		window->LastTimeActive = (float)g.Time;
		window->BeginOrderWithinParent = 0;
		window->BeginOrderWithinContext = (short)(g.WindowsActiveCount++);
	}
	else
	{
		flags = window->Flags;
	}

	// Docking
	// (NB: during the frame dock nodes are created, it is possible that (window->DockIsActive == false) even though (window->DockNode->Windows.Size > 1)
	KR_CORE_ASSERT(window->DockNode == NULL || window->DockNodeAsHost == NULL, ""); // Cannot be both
	if (g.NextWindowData.Flags & KGGuiNextWindowDataFlags_HasDock)
		KarmaGuiInternal::SetWindowDock(window, g.NextWindowData.DockId, g.NextWindowData.DockCond);
	if (first_begin_of_the_frame)
	{
		bool has_dock_node = (window->DockId != 0 || window->DockNode != NULL);
		bool new_auto_dock_node = !has_dock_node && KarmaGuiInternal::GetWindowAlwaysWantOwnTabBar(window);
		bool dock_node_was_visible = window->DockNodeIsVisible;
		bool dock_tab_was_visible = window->DockTabIsVisible;
		if (has_dock_node || new_auto_dock_node)
		{
			KarmaGuiInternal::BeginDocked(window, p_open);
			flags = window->Flags;
			if (window->DockIsActive)
			{
				KR_CORE_ASSERT(window->DockNode != NULL, "");
				g.NextWindowData.Flags &= ~KGGuiNextWindowDataFlags_HasSizeConstraint; // Docking currently override constraints
			}

			// Amend the Appearing flag
			if (window->DockTabIsVisible && !dock_tab_was_visible && dock_node_was_visible && !window->Appearing && !window_was_appearing)
			{
				window->Appearing = true;
				SetWindowConditionAllowFlags(window, KGGuiCond_Appearing, true);
			}
		}
		else
		{
			window->DockIsActive = window->DockNodeIsVisible = window->DockTabIsVisible = false;
		}
	}

	// Parent window is latched only on the first call to Begin() of the frame, so further append-calls can be done from a different window stack
	KGGuiWindow* parent_window_in_stack = (window->DockIsActive && window->DockNode->HostWindow) ? window->DockNode->HostWindow : g.CurrentWindowStack.empty() ? NULL : g.CurrentWindowStack.back().Window;
	KGGuiWindow* parent_window = first_begin_of_the_frame ? ((flags & (KGGuiWindowFlags_ChildWindow | KGGuiWindowFlags_Popup)) ? parent_window_in_stack : NULL) : window->ParentWindow;
	KR_CORE_ASSERT(parent_window != NULL || !(flags & KGGuiWindowFlags_ChildWindow), "");

	// We allow window memory to be compacted so recreate the base stack when needed.
	if (window->IDStack.Size == 0)
		window->IDStack.push_back(window->ID);

	// Add to stack
	// We intentionally set g.CurrentWindow to NULL to prevent usage until when the viewport is set, then will call SetCurrentWindow()
	g.CurrentWindow = window;
	KGGuiWindowStackData window_stack_data;
	window_stack_data.Window = window;
	window_stack_data.ParentLastItemDataBackup = g.LastItemData;
	window_stack_data.StackSizesOnBegin.SetToCurrentState();
	g.CurrentWindowStack.push_back(window_stack_data);
	if (flags & KGGuiWindowFlags_ChildMenu)
		g.BeginMenuCount++;

	// Update ->RootWindow and others pointers (before any possible call to FocusWindow)
	if (first_begin_of_the_frame)
	{
		KarmaGuiInternal::UpdateWindowParentAndRootLinks(window, flags, parent_window);
		window->ParentWindowInBeginStack = parent_window_in_stack;
	}

	// Add to focus scope stack
	KarmaGuiInternal::PushFocusScope(window->ID);
	window->NavRootFocusScopeId = g.CurrentFocusScopeId;
	g.CurrentWindow = NULL;

	// Add to popup stack
	if (flags & KGGuiWindowFlags_Popup)
	{
		KGGuiPopupData& popup_ref = g.OpenPopupStack[g.BeginPopupStack.Size];
		popup_ref.Window = window;
		popup_ref.ParentNavLayer = parent_window_in_stack->DC.NavLayerCurrent;
		g.BeginPopupStack.push_back(popup_ref);
		window->PopupId = popup_ref.PopupId;
	}

	// Process SetNextWindow***() calls
	// (FIXME: Consider splitting the HasXXX flags into X/Y components
	bool window_pos_set_by_api = false;
	bool window_size_x_set_by_api = false, window_size_y_set_by_api = false;
	if (g.NextWindowData.Flags & KGGuiNextWindowDataFlags_HasPos)
	{
		window_pos_set_by_api = (window->SetWindowPosAllowFlags & g.NextWindowData.PosCond) != 0;
		if (window_pos_set_by_api && KGLengthSqr(g.NextWindowData.PosPivotVal) > 0.00001f)
		{
			// May be processed on the next frame if this is our first frame and we are measuring size
			// FIXME: Look into removing the branch so everything can go through this same code path for consistency.
			window->SetWindowPosVal = g.NextWindowData.PosVal;
			window->SetWindowPosPivot = g.NextWindowData.PosPivotVal;
			window->SetWindowPosAllowFlags &= ~(KGGuiCond_Once | KGGuiCond_FirstUseEver | KGGuiCond_Appearing);
		}
		else
		{
			KarmaGuiInternal::SetWindowPos(window, g.NextWindowData.PosVal, g.NextWindowData.PosCond);
		}
	}
	if (g.NextWindowData.Flags & KGGuiNextWindowDataFlags_HasSize)
	{
		window_size_x_set_by_api = (window->SetWindowSizeAllowFlags & g.NextWindowData.SizeCond) != 0 && (g.NextWindowData.SizeVal.x > 0.0f);
		window_size_y_set_by_api = (window->SetWindowSizeAllowFlags & g.NextWindowData.SizeCond) != 0 && (g.NextWindowData.SizeVal.y > 0.0f);
		KarmaGuiInternal::SetWindowSize(window, g.NextWindowData.SizeVal, g.NextWindowData.SizeCond);
	}
	if (g.NextWindowData.Flags & KGGuiNextWindowDataFlags_HasScroll)
	{
		if (g.NextWindowData.ScrollVal.x >= 0.0f)
		{
			window->ScrollTarget.x = g.NextWindowData.ScrollVal.x;
			window->ScrollTargetCenterRatio.x = 0.0f;
		}
		if (g.NextWindowData.ScrollVal.y >= 0.0f)
		{
			window->ScrollTarget.y = g.NextWindowData.ScrollVal.y;
			window->ScrollTargetCenterRatio.y = 0.0f;
		}
	}
	if (g.NextWindowData.Flags & KGGuiNextWindowDataFlags_HasContentSize)
		window->ContentSizeExplicit = g.NextWindowData.ContentSizeVal;
	else if (first_begin_of_the_frame)
		window->ContentSizeExplicit = KGVec2(0.0f, 0.0f);
	if (g.NextWindowData.Flags & KGGuiNextWindowDataFlags_HasWindowClass)
		window->WindowClass = g.NextWindowData.WindowClass;
	if (g.NextWindowData.Flags & KGGuiNextWindowDataFlags_HasCollapsed)
		KarmaGuiInternal::SetWindowCollapsed(window, g.NextWindowData.CollapsedVal, g.NextWindowData.CollapsedCond);
	if (g.NextWindowData.Flags & KGGuiNextWindowDataFlags_HasFocus)
		KarmaGuiInternal::FocusWindow(window);
	if (window->Appearing)
		SetWindowConditionAllowFlags(window, KGGuiCond_Appearing, false);

	// When reusing window again multiple times a frame, just append content (don't need to setup again)
	if (first_begin_of_the_frame)
	{
		// Initialize
		const bool window_is_child_tooltip = (flags & KGGuiWindowFlags_ChildWindow) && (flags & KGGuiWindowFlags_Tooltip); // FIXME-WIP: Undocumented behavior of Child+Tooltip for pinned tooltip (#1345)
		const bool window_just_appearing_after_hidden_for_resize = (window->HiddenFramesCannotSkipItems > 0);
		window->Active = true;
		window->HasCloseButton = (p_open != NULL);
		window->ClipRect = KGVec4(-FLT_MAX, -FLT_MAX, +FLT_MAX, +FLT_MAX);
		window->IDStack.resize(1);
		window->DrawList->_ResetForNewFrame();
		window->DC.CurrentTableIdx = -1;
		if (flags & KGGuiWindowFlags_DockNodeHost)
		{
			window->DrawList->ChannelsSplit(2);
			window->DrawList->ChannelsSetCurrent(DOCKING_HOST_DRAW_CHANNEL_FG); // Render decorations on channel 1 as we will render the backgrounds manually later
		}

		// Restore buffer capacity when woken from a compacted state, to avoid
		if (window->MemoryCompacted)
			KarmaGuiInternal::GcAwakeTransientWindowBuffers(window);

		// Update stored window name when it changes (which can _only_ happen with the "###" operator, so the ID would stay unchanged).
		// The title bar always display the 'name' parameter, so we only update the string storage if it needs to be visible to the end-user elsewhere.
		bool window_title_visible_elsewhere = false;
		if ((window->Viewport && window->Viewport->Window == window) || (window->DockIsActive))
			window_title_visible_elsewhere = true;
		else if (g.NavWindowingListWindow != NULL && (window->Flags & KGGuiWindowFlags_NoNavFocus) == 0)   // Window titles visible when using CTRL+TAB
			window_title_visible_elsewhere = true;
		if (window_title_visible_elsewhere && !window_just_created && strcmp(name, window->Name) != 0)
		{
			size_t buf_len = (size_t)window->NameBufLen;
			window->Name = KGStrdupcpy(window->Name, &buf_len, name);
			window->NameBufLen = (int)buf_len;
		}

		// UPDATE CONTENTS SIZE, UPDATE HIDDEN STATUS

		// Update contents size from last frame for auto-fitting (or use explicit size)
		CalcWindowContentSizes(window, &window->ContentSize, &window->ContentSizeIdeal);

		// FIXME: These flags are decremented before they are used. This means that in order to have these fields produce their intended behaviors
		// for one frame we must set them to at least 2, which is counter-intuitive. HiddenFramesCannotSkipItems is a more complicated case because
		// it has a single usage before this code block and may be set below before it is finally checked.
		if (window->HiddenFramesCanSkipItems > 0)
			window->HiddenFramesCanSkipItems--;
		if (window->HiddenFramesCannotSkipItems > 0)
			window->HiddenFramesCannotSkipItems--;
		if (window->HiddenFramesForRenderOnly > 0)
			window->HiddenFramesForRenderOnly--;

		// Hide new windows for one frame until they calculate their size
		if (window_just_created && (!window_size_x_set_by_api || !window_size_y_set_by_api))
			window->HiddenFramesCannotSkipItems = 1;

		// Hide popup/tooltip window when re-opening while we measure size (because we recycle the windows)
		// We reset Size/ContentSize for reappearing popups/tooltips early in this function, so further code won't be tempted to use the old size.
		if (window_just_activated_by_user && (flags & (KGGuiWindowFlags_Popup | KGGuiWindowFlags_Tooltip)) != 0)
		{
			window->HiddenFramesCannotSkipItems = 1;
			if (flags & KGGuiWindowFlags_AlwaysAutoResize)
			{
				if (!window_size_x_set_by_api)
					window->Size.x = window->SizeFull.x = 0.f;
				if (!window_size_y_set_by_api)
					window->Size.y = window->SizeFull.y = 0.f;
				window->ContentSize = window->ContentSizeIdeal = KGVec2(0.f, 0.f);
			}
		}

		// SELECT VIEWPORT
		// We need to do this before using any style/font sizes, as viewport with a different DPI may affect font sizes.

		KarmaGuiInternal::WindowSelectViewport(window);
		KarmaGuiInternal::SetCurrentViewport(window, window->Viewport);
		window->FontDpiScale = (g.IO.ConfigFlags & KGGuiConfigFlags_DpiEnableScaleFonts) ? window->Viewport->DpiScale : 1.0f;
		SetCurrentWindow(window);
		flags = window->Flags;

		// LOCK BORDER SIZE AND PADDING FOR THE FRAME (so that altering them doesn't cause inconsistencies)
		// We read Style data after the call to UpdateSelectWindowViewport() which might be swapping the style.

		if (flags & KGGuiWindowFlags_ChildWindow)
			window->WindowBorderSize = style.ChildBorderSize;
		else
			window->WindowBorderSize = ((flags & (KGGuiWindowFlags_Popup | KGGuiWindowFlags_Tooltip)) && !(flags & KGGuiWindowFlags_Modal)) ? style.PopupBorderSize : style.WindowBorderSize;
		if (!window->DockIsActive && (flags & KGGuiWindowFlags_ChildWindow) && !(flags & (KGGuiWindowFlags_AlwaysUseWindowPadding | KGGuiWindowFlags_Popup)) && window->WindowBorderSize == 0.0f)
			window->WindowPadding = KGVec2(0.0f, (flags & KGGuiWindowFlags_MenuBar) ? style.WindowPadding.y : 0.0f);
		else
			window->WindowPadding = style.WindowPadding;

		// Lock menu offset so size calculation can use it as menu-bar windows need a minimum size.
		window->DC.MenuBarOffset.x = KGMax(KGMax(window->WindowPadding.x, style.ItemSpacing.x), g.NextWindowData.MenuBarOffsetMinVal.x);
		window->DC.MenuBarOffset.y = g.NextWindowData.MenuBarOffsetMinVal.y;

		bool use_current_size_for_scrollbar_x = window_just_created;
		bool use_current_size_for_scrollbar_y = window_just_created;

		// Collapse window by double-clicking on title bar
		// At this point we don't have a clipping rectangle setup yet, so we can use the title bar area for hit detection and drawing
		if (!(flags & KGGuiWindowFlags_NoTitleBar) && !(flags & KGGuiWindowFlags_NoCollapse) && !window->DockIsActive)
		{
			// We don't use a regular button+id to test for double-click on title bar (mostly due to legacy reason, could be fixed), so verify that we don't have items over the title bar.
			KGRect title_bar_rect = window->TitleBarRect();
			if (g.HoveredWindow == window && g.HoveredId == 0 && g.HoveredIdPreviousFrame == 0 && IsMouseHoveringRect(title_bar_rect.Min, title_bar_rect.Max) && g.IO.MouseClickedCount[0] == 2)
				window->WantCollapseToggle = true;
			if (window->WantCollapseToggle)
			{
				window->Collapsed = !window->Collapsed;
				if (!window->Collapsed)
					use_current_size_for_scrollbar_y = true;
				KarmaGuiInternal::MarkIniSettingsDirty(window);
			}
		}
		else
		{
			window->Collapsed = false;
		}
		window->WantCollapseToggle = false;

		// SIZE

		// Outer Decoration Sizes
		// (we need to clear ScrollbarSize immediatly as CalcWindowAutoFitSize() needs it and can be called from other locations).
		const KGVec2 scrollbar_sizes_from_last_frame = window->ScrollbarSizes;
		window->DecoOuterSizeX1 = 0.0f;
		window->DecoOuterSizeX2 = 0.0f;
		window->DecoOuterSizeY1 = window->TitleBarHeight() + window->MenuBarHeight();
		window->DecoOuterSizeY2 = 0.0f;
		window->ScrollbarSizes = KGVec2(0.0f, 0.0f);

		// Calculate auto-fit size, handle automatic resize
		const KGVec2 size_auto_fit = CalcWindowAutoFitSize(window, window->ContentSizeIdeal);
		if ((flags & KGGuiWindowFlags_AlwaysAutoResize) && !window->Collapsed)
		{
			// Using SetNextWindowSize() overrides KGGuiWindowFlags_AlwaysAutoResize, so it can be used on tooltips/popups, etc.
			if (!window_size_x_set_by_api)
			{
				window->SizeFull.x = size_auto_fit.x;
				use_current_size_for_scrollbar_x = true;
			}
			if (!window_size_y_set_by_api)
			{
				window->SizeFull.y = size_auto_fit.y;
				use_current_size_for_scrollbar_y = true;
			}
		}
		else if (window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0)
		{
			// Auto-fit may only grow window during the first few frames
			// We still process initial auto-fit on collapsed windows to get a window width, but otherwise don't honor KGGuiWindowFlags_AlwaysAutoResize when collapsed.
			if (!window_size_x_set_by_api && window->AutoFitFramesX > 0)
			{
				window->SizeFull.x = window->AutoFitOnlyGrows ? KGMax(window->SizeFull.x, size_auto_fit.x) : size_auto_fit.x;
				use_current_size_for_scrollbar_x = true;
			}
			if (!window_size_y_set_by_api && window->AutoFitFramesY > 0)
			{
				window->SizeFull.y = window->AutoFitOnlyGrows ? KGMax(window->SizeFull.y, size_auto_fit.y) : size_auto_fit.y;
				use_current_size_for_scrollbar_y = true;
			}
			if (!window->Collapsed)
				KarmaGuiInternal::MarkIniSettingsDirty(window);
		}

		// Apply minimum/maximum window size constraints and final size
		window->SizeFull = CalcWindowSizeAfterConstraint(window, window->SizeFull);
		window->Size = window->Collapsed && !(flags & KGGuiWindowFlags_ChildWindow) ? window->TitleBarRect().GetSize() : window->SizeFull;

		// POSITION

		// Popup latch its initial position, will position itself when it appears next frame
		if (window_just_activated_by_user)
		{
			window->AutoPosLastDirection = KGGuiDir_None;
			if ((flags & KGGuiWindowFlags_Popup) != 0 && !(flags & KGGuiWindowFlags_Modal) && !window_pos_set_by_api) // FIXME: BeginPopup() could use SetNextWindowPos()
				window->Pos = g.BeginPopupStack.back().OpenPopupPos;
		}

		// Position child window
		if (flags & KGGuiWindowFlags_ChildWindow)
		{
			KR_CORE_ASSERT(parent_window && parent_window->Active, "");
			window->BeginOrderWithinParent = (short)parent_window->DC.ChildWindows.Size;
			parent_window->DC.ChildWindows.push_back(window);
			if (!(flags & KGGuiWindowFlags_Popup) && !window_pos_set_by_api && !window_is_child_tooltip)
				window->Pos = parent_window->DC.CursorPos;
		}

		const bool window_pos_with_pivot = (window->SetWindowPosVal.x != FLT_MAX && window->HiddenFramesCannotSkipItems == 0);
		if (window_pos_with_pivot)
			KarmaGuiInternal::SetWindowPos(window, window->SetWindowPosVal - window->Size * window->SetWindowPosPivot, 0); // Position given a pivot (e.g. for centering)
		else if ((flags & KGGuiWindowFlags_ChildMenu) != 0)
			window->Pos = KarmaGuiInternal::FindBestWindowPosForPopup(window);
		else if ((flags & KGGuiWindowFlags_Popup) != 0 && !window_pos_set_by_api && window_just_appearing_after_hidden_for_resize)
			window->Pos = KarmaGuiInternal::FindBestWindowPosForPopup(window);
		else if ((flags & KGGuiWindowFlags_Tooltip) != 0 && !window_pos_set_by_api && !window_is_child_tooltip)
			window->Pos = KarmaGuiInternal::FindBestWindowPosForPopup(window);

		// Late create viewport if we don't fit within our current host viewport.
		if (window->ViewportAllowPlatformMonitorExtend >= 0 && !window->ViewportOwned && !(window->Viewport->Flags & KGGuiViewportFlags_Minimized))
			if (!window->Viewport->GetMainRect().Contains(window->Rect()))
			{
				// This is based on the assumption that the DPI will be known ahead (same as the DPI of the selection done in UpdateSelectWindowViewport)
				//KarmaGuiViewport* old_viewport = window->Viewport;
				window->Viewport = KarmaGuiInternal::AddUpdateViewport(window, window->ID, window->Pos, window->Size, KGGuiViewportFlags_NoFocusOnAppearing);

				// FIXME-DPI
				//KR_CORE_ASSERT(old_viewport->DpiScale == window->Viewport->DpiScale); // FIXME-DPI: Something went wrong
				KarmaGuiInternal::SetCurrentViewport(window, window->Viewport);
				window->FontDpiScale = (g.IO.ConfigFlags & KGGuiConfigFlags_DpiEnableScaleFonts) ? window->Viewport->DpiScale : 1.0f;
				SetCurrentWindow(window);
			}

		if (window->ViewportOwned)
			KarmaGuiInternal::WindowSyncOwnedViewport(window, parent_window_in_stack);

		// Calculate the range of allowed position for that window (to be movable and visible past safe area padding)
		// When clamping to stay visible, we will enforce that window->Pos stays inside of visibility_rect.
		KGRect viewport_rect(window->Viewport->GetMainRect());
		KGRect viewport_work_rect(window->Viewport->GetWorkRect());
		KGVec2 visibility_padding = KGMax(style.DisplayWindowPadding, style.DisplaySafeAreaPadding);
		KGRect visibility_rect(viewport_work_rect.Min + visibility_padding, viewport_work_rect.Max - visibility_padding);

		// Clamp position/size so window stays visible within its viewport or monitor
		// Ignore zero-sized display explicitly to avoid losing positions if a window manager reports zero-sized window when initializing or minimizing.
		// FIXME: Similar to code in GetWindowAllowedExtentRect()
		if (!window_pos_set_by_api && !(flags & KGGuiWindowFlags_ChildWindow))
		{
			if (!window->ViewportOwned && viewport_rect.GetWidth() > 0 && viewport_rect.GetHeight() > 0.0f)
			{
				ClampWindowPos(window, visibility_rect);
			}
			else if (window->ViewportOwned && g.PlatformIO.Monitors.Size > 0)
			{
				// Lost windows (e.g. a monitor disconnected) will naturally moved to the fallback/dummy monitor aka the main viewport.
				const KarmaGuiPlatformMonitor* monitor = KarmaGuiInternal::GetViewportPlatformMonitor(window->Viewport);
				visibility_rect.Min = monitor->WorkPos + visibility_padding;
				visibility_rect.Max = monitor->WorkPos + monitor->WorkSize - visibility_padding;
				ClampWindowPos(window, visibility_rect);
			}
		}
		window->Pos = KGFloor(window->Pos);

		// Lock window rounding for the frame (so that altering them doesn't cause inconsistencies)
		// Large values tend to lead to variety of artifacts and are not recommended.
		if (window->ViewportOwned || window->DockIsActive)
			window->WindowRounding = 0.0f;
		else
			window->WindowRounding = (flags & KGGuiWindowFlags_ChildWindow) ? style.ChildRounding : ((flags & KGGuiWindowFlags_Popup) && !(flags & KGGuiWindowFlags_Modal)) ? style.PopupRounding : style.WindowRounding;

		// For windows with title bar or menu bar, we clamp to FrameHeight(FontSize + FramePadding.y * 2.0f) to completely hide artifacts.
		//if ((window->Flags & KGGuiWindowFlags_MenuBar) || !(window->Flags & KGGuiWindowFlags_NoTitleBar))
		//    window->WindowRounding = KGMin(window->WindowRounding, g.FontSize + style.FramePadding.y * 2.0f);

		// Apply window focus (new and reactivated windows are moved to front)
		bool want_focus = false;
		if (window_just_activated_by_user && !(flags & KGGuiWindowFlags_NoFocusOnAppearing))
		{
			if (flags & KGGuiWindowFlags_Popup)
				want_focus = true;
			else if ((window->DockIsActive || (flags & KGGuiWindowFlags_ChildWindow) == 0) && !(flags & KGGuiWindowFlags_Tooltip))
				want_focus = true;

			KGGuiWindow* modal = KarmaGuiInternal::GetTopMostPopupModal();
			if (modal != NULL && !KarmaGuiInternal::IsWindowWithinBeginStackOf(window, modal))
			{
				// Avoid focusing a window that is created outside of active modal. This will prevent active modal from being closed.
				// Since window is not focused it would reappear at the same display position like the last time it was visible.
				// In case of completely new windows it would go to the top (over current modal), but input to such window would still be blocked by modal.
				// Position window behind a modal that is not a begin-parent of this window.
				want_focus = false;
				if (window == window->RootWindow)
				{
					KGGuiWindow* blocking_modal = KarmaGuiInternal::FindBlockingModal(window);
					KR_CORE_ASSERT(blocking_modal != NULL, "");
					KarmaGuiInternal::BringWindowToDisplayBehind(window, blocking_modal);
				}
			}
		}

		// [Test Engine] Register whole window in the item system
#ifdef IMGUI_ENABLE_TEST_ENGINE
		if (g.TestEngineHookItems)
		{
			KR_CORE_ASSERT(window->IDStack.Size == 1);
			window->IDStack.Size = 0; // As window->IDStack[0] == window->ID here, make sure TestEngine doesn't erroneously see window as parent of itself.
			KARMAGUI_TEST_ENGINE_ITEM_ADD(window->Rect(), window->ID);
			KARMAGUI_TEST_ENGINE_ITEM_INFO(window->ID, window->Name, (g.HoveredWindow == window) ? KGGuiItemStatusFlags_HoveredRect : 0);
			window->IDStack.Size = 1;
		}
#endif

		// Decide if we are going to handle borders and resize grips
		const bool handle_borders_and_resize_grips = (window->DockNodeAsHost || !window->DockIsActive);

		// Handle manual resize: Resize Grips, Borders, Gamepad
		int border_held = -1;
		KGU32 resize_grip_col[4] = {};
		const int resize_grip_count = g.IO.ConfigWindowsResizeFromEdges ? 2 : 1; // Allow resize from lower-left if we have the mouse cursor feedback for it.
		const float resize_grip_draw_size = KG_FLOOR(KGMax(g.FontSize * 1.10f, window->WindowRounding + 1.0f + g.FontSize * 0.2f));
		if (handle_borders_and_resize_grips && !window->Collapsed)
			if (KarmaGuiInternal::UpdateWindowManualResize(window, size_auto_fit, &border_held, resize_grip_count, &resize_grip_col[0], visibility_rect))
				use_current_size_for_scrollbar_x = use_current_size_for_scrollbar_y = true;
		window->ResizeBorderHeld = (signed char)border_held;

		// Synchronize window --> viewport again and one last time (clamping and manual resize may have affected either)
		if (window->ViewportOwned)
		{
			if (!window->Viewport->PlatformRequestMove)
				window->Viewport->Pos = window->Pos;
			if (!window->Viewport->PlatformRequestResize)
				window->Viewport->Size = window->Size;
			window->Viewport->UpdateWorkRect();
			viewport_rect = window->Viewport->GetMainRect();
		}

		// Save last known viewport position within the window itself (so it can be saved in .ini file and restored)
		window->ViewportPos = window->Viewport->Pos;

		// SCROLLBAR VISIBILITY

		// Update scrollbar visibility (based on the Size that was effective during last frame or the auto-resized Size).
		if (!window->Collapsed)
		{
			// When reading the current size we need to read it after size constraints have been applied.
			// Intentionally use previous frame values for InnerRect and ScrollbarSizes.
			// And when we use window->DecorationUp here it doesn't have ScrollbarSizes.y applied yet.
			KGVec2 avail_size_from_current_frame = KGVec2(window->SizeFull.x, window->SizeFull.y - (window->DecoOuterSizeY1 + window->DecoOuterSizeY2));
			KGVec2 avail_size_from_last_frame = window->InnerRect.GetSize() + scrollbar_sizes_from_last_frame;
			KGVec2 needed_size_from_last_frame = window_just_created ? KGVec2(0, 0) : window->ContentSize + window->WindowPadding * 2.0f;
			float size_x_for_scrollbars = use_current_size_for_scrollbar_x ? avail_size_from_current_frame.x : avail_size_from_last_frame.x;
			float size_y_for_scrollbars = use_current_size_for_scrollbar_y ? avail_size_from_current_frame.y : avail_size_from_last_frame.y;
			//bool scrollbar_y_from_last_frame = window->ScrollbarY; // FIXME: May want to use that in the ScrollbarX expression? How many pros vs cons?
			window->ScrollbarY = (flags & KGGuiWindowFlags_AlwaysVerticalScrollbar) || ((needed_size_from_last_frame.y > size_y_for_scrollbars) && !(flags & KGGuiWindowFlags_NoScrollbar));
			window->ScrollbarX = (flags & KGGuiWindowFlags_AlwaysHorizontalScrollbar) || ((needed_size_from_last_frame.x > size_x_for_scrollbars - (window->ScrollbarY ? style.ScrollbarSize : 0.0f)) && !(flags & KGGuiWindowFlags_NoScrollbar) && (flags & KGGuiWindowFlags_HorizontalScrollbar));
			if (window->ScrollbarX && !window->ScrollbarY)
				window->ScrollbarY = (needed_size_from_last_frame.y > size_y_for_scrollbars) && !(flags & KGGuiWindowFlags_NoScrollbar);
			window->ScrollbarSizes = KGVec2(window->ScrollbarY ? style.ScrollbarSize : 0.0f, window->ScrollbarX ? style.ScrollbarSize : 0.0f);

			// Amend the partially filled window->DecorationXXX values.
			window->DecoOuterSizeX2 += window->ScrollbarSizes.x;
			window->DecoOuterSizeY2 += window->ScrollbarSizes.y;
		}

		// UPDATE RECTANGLES (1- THOSE NOT AFFECTED BY SCROLLING)
		// Update various regions. Variables they depend on should be set above in this function.
		// We set this up after processing the resize grip so that our rectangles doesn't lag by a frame.

		// Outer rectangle
		// Not affected by window border size. Used by:
		// - FindHoveredWindow() (w/ extra padding when border resize is enabled)
		// - Begin() initial clipping rect for drawing window background and borders.
		// - Begin() clipping whole child
		const KGRect host_rect = ((flags & KGGuiWindowFlags_ChildWindow) && !(flags & KGGuiWindowFlags_Popup) && !window_is_child_tooltip) ? parent_window->ClipRect : viewport_rect;
		const KGRect outer_rect = window->Rect();
		const KGRect title_bar_rect = window->TitleBarRect();
		window->OuterRectClipped = outer_rect;
		if (window->DockIsActive)
			window->OuterRectClipped.Min.y += window->TitleBarHeight();
		window->OuterRectClipped.ClipWith(host_rect);

		// Inner rectangle
		// Not affected by window border size. Used by:
		// - InnerClipRect
		// - ScrollToRectEx()
		// - NavUpdatePageUpPageDown()
		// - Scrollbar()
		window->InnerRect.Min.x = window->Pos.x + window->DecoOuterSizeX1;
		window->InnerRect.Min.y = window->Pos.y + window->DecoOuterSizeY1;
		window->InnerRect.Max.x = window->Pos.x + window->Size.x - window->DecoOuterSizeX2;
		window->InnerRect.Max.y = window->Pos.y + window->Size.y - window->DecoOuterSizeY2;

		// Inner clipping rectangle.
		// Will extend a little bit outside the normal work region.
		// This is to allow e.g. Selectable or CollapsingHeader or some separators to cover that space.
		// Force round operator last to ensure that e.g. (int)(max.x-min.x) in user's render code produce correct result.
		// Note that if our window is collapsed we will end up with an inverted (~null) clipping rectangle which is the correct behavior.
		// Affected by window/frame border size. Used by:
		// - Begin() initial clip rect
		float top_border_size = (((flags & KGGuiWindowFlags_MenuBar) || !(flags & KGGuiWindowFlags_NoTitleBar)) ? style.FrameBorderSize : window->WindowBorderSize);
		window->InnerClipRect.Min.x = KGFloor(0.5f + window->InnerRect.Min.x + KGMax(KGFloor(window->WindowPadding.x * 0.5f), window->WindowBorderSize));
		window->InnerClipRect.Min.y = KGFloor(0.5f + window->InnerRect.Min.y + top_border_size);
		window->InnerClipRect.Max.x = KGFloor(0.5f + window->InnerRect.Max.x - KGMax(KGFloor(window->WindowPadding.x * 0.5f), window->WindowBorderSize));
		window->InnerClipRect.Max.y = KGFloor(0.5f + window->InnerRect.Max.y - window->WindowBorderSize);
		window->InnerClipRect.ClipWithFull(host_rect);

		// Default item width. Make it proportional to window size if window manually resizes
		if (window->Size.x > 0.0f && !(flags & KGGuiWindowFlags_Tooltip) && !(flags & KGGuiWindowFlags_AlwaysAutoResize))
			window->ItemWidthDefault = KGFloor(window->Size.x * 0.65f);
		else
			window->ItemWidthDefault = KGFloor(g.FontSize * 16.0f);

		// SCROLLING

		// Lock down maximum scrolling
		// The value of ScrollMax are ahead from ScrollbarX/ScrollbarY which is intentionally using InnerRect from previous rect in order to accommodate
		// for right/bottom aligned items without creating a scrollbar.
		window->ScrollMax.x = KGMax(0.0f, window->ContentSize.x + window->WindowPadding.x * 2.0f - window->InnerRect.GetWidth());
		window->ScrollMax.y = KGMax(0.0f, window->ContentSize.y + window->WindowPadding.y * 2.0f - window->InnerRect.GetHeight());

		// Apply scrolling
		window->Scroll = KarmaGuiInternal::CalcNextScrollFromScrollTargetAndClamp(window);
		window->ScrollTarget = KGVec2(FLT_MAX, FLT_MAX);
		window->DecoInnerSizeX1 = window->DecoInnerSizeY1 = 0.0f;

		// DRAWING

		// Setup draw list and outer clipping rectangle
		KR_CORE_ASSERT(window->DrawList->CmdBuffer.Size == 1 && window->DrawList->CmdBuffer[0].ElemCount == 0, "");
		window->DrawList->PushTextureID(g.Font->ContainerAtlas->TexID);
		PushClipRect(host_rect.Min, host_rect.Max, false);

		// Child windows can render their decoration (bg color, border, scrollbars, etc.) within their parent to save a draw call (since 1.71)
		// When using overlapping child windows, this will break the assumption that child z-order is mapped to submission order.
		// FIXME: User code may rely on explicit sorting of overlapping child window and would need to disable this somehow. Please get in contact if you are affected (github #4493)
		const bool is_undocked_or_docked_visible = !window->DockIsActive || window->DockTabIsVisible;
		if (is_undocked_or_docked_visible)
		{
			bool render_decorations_in_parent = false;
			if ((flags & KGGuiWindowFlags_ChildWindow) && !(flags & KGGuiWindowFlags_Popup) && !window_is_child_tooltip)
			{
				// - We test overlap with the previous child window only (testing all would end up being O(log N) not a good investment here)
				// - We disable this when the parent window has zero vertices, which is a common pattern leading to laying out multiple overlapping childs
				KGGuiWindow* previous_child = parent_window->DC.ChildWindows.Size >= 2 ? parent_window->DC.ChildWindows[parent_window->DC.ChildWindows.Size - 2] : NULL;
				bool previous_child_overlapping = previous_child ? previous_child->Rect().Overlaps(window->Rect()) : false;
				bool parent_is_empty = parent_window->DrawList->VtxBuffer.Size > 0;
				if (window->DrawList->CmdBuffer.back().ElemCount == 0 && parent_is_empty && !previous_child_overlapping)
					render_decorations_in_parent = true;
			}
			if (render_decorations_in_parent)
				window->DrawList = parent_window->DrawList;

			// Handle title bar, scrollbar, resize grips and resize borders
			const KGGuiWindow* window_to_highlight = g.NavWindowingTarget ? g.NavWindowingTarget : g.NavWindow;
			const bool title_bar_is_highlight = want_focus || (window_to_highlight && (window->RootWindowForTitleBarHighlight == window_to_highlight->RootWindowForTitleBarHighlight || (window->DockNode && window->DockNode == window_to_highlight->DockNode)));
			KarmaGuiInternal::RenderWindowDecorations(window, title_bar_rect, title_bar_is_highlight, handle_borders_and_resize_grips, resize_grip_count, resize_grip_col, resize_grip_draw_size);

			if (render_decorations_in_parent)
				window->DrawList = &window->DrawListInst;
		}

		// UPDATE RECTANGLES (2- THOSE AFFECTED BY SCROLLING)

		// Work rectangle.
		// Affected by window padding and border size. Used by:
		// - Columns() for right-most edge
		// - TreeNode(), CollapsingHeader() for right-most edge
		// - BeginTabBar() for right-most edge
		const bool allow_scrollbar_x = !(flags & KGGuiWindowFlags_NoScrollbar) && (flags & KGGuiWindowFlags_HorizontalScrollbar);
		const bool allow_scrollbar_y = !(flags & KGGuiWindowFlags_NoScrollbar);
		const float work_rect_size_x = (window->ContentSizeExplicit.x != 0.0f ? window->ContentSizeExplicit.x : KGMax(allow_scrollbar_x ? window->ContentSize.x : 0.0f, window->Size.x - window->WindowPadding.x * 2.0f - (window->DecoOuterSizeX1 + window->DecoOuterSizeX2)));
		const float work_rect_size_y = (window->ContentSizeExplicit.y != 0.0f ? window->ContentSizeExplicit.y : KGMax(allow_scrollbar_y ? window->ContentSize.y : 0.0f, window->Size.y - window->WindowPadding.y * 2.0f - (window->DecoOuterSizeY1 + window->DecoOuterSizeY2)));
		window->WorkRect.Min.x = KGFloor(window->InnerRect.Min.x - window->Scroll.x + KGMax(window->WindowPadding.x, window->WindowBorderSize));
		window->WorkRect.Min.y = KGFloor(window->InnerRect.Min.y - window->Scroll.y + KGMax(window->WindowPadding.y, window->WindowBorderSize));
		window->WorkRect.Max.x = window->WorkRect.Min.x + work_rect_size_x;
		window->WorkRect.Max.y = window->WorkRect.Min.y + work_rect_size_y;
		window->ParentWorkRect = window->WorkRect;

		// [LEGACY] Content Region
		// FIXME-OBSOLETE: window->ContentRegionRect.Max is currently very misleading / partly faulty, but some BeginChild() patterns relies on it.
		// Used by:
		// - Mouse wheel scrolling + many other things
		window->ContentRegionRect.Min.x = window->Pos.x - window->Scroll.x + window->WindowPadding.x + window->DecoOuterSizeX1;
		window->ContentRegionRect.Min.y = window->Pos.y - window->Scroll.y + window->WindowPadding.y + window->DecoOuterSizeY1;
		window->ContentRegionRect.Max.x = window->ContentRegionRect.Min.x + (window->ContentSizeExplicit.x != 0.0f ? window->ContentSizeExplicit.x : (window->Size.x - window->WindowPadding.x * 2.0f - (window->DecoOuterSizeX1 + window->DecoOuterSizeX2)));
		window->ContentRegionRect.Max.y = window->ContentRegionRect.Min.y + (window->ContentSizeExplicit.y != 0.0f ? window->ContentSizeExplicit.y : (window->Size.y - window->WindowPadding.y * 2.0f - (window->DecoOuterSizeY1 + window->DecoOuterSizeY2)));

		// Setup drawing context
		// (NB: That term "drawing context / DC" lost its meaning a long time ago. Initially was meant to hold transient data only. Nowadays difference between window-> and window->DC-> is dubious.)
		window->DC.Indent.x = window->DecoOuterSizeX1 + window->WindowPadding.x - window->Scroll.x;
		window->DC.GroupOffset.x = 0.0f;
		window->DC.ColumnsOffset.x = 0.0f;

		// Record the loss of precision of CursorStartPos which can happen due to really large scrolling amount.
		// This is used by clipper to compensate and fix the most common use case of large scroll area. Easy and cheap, next best thing compared to switching everything to double or KGU64.
		double start_pos_highp_x = (double)window->Pos.x + window->WindowPadding.x - (double)window->Scroll.x + window->DecoOuterSizeX1 + window->DC.ColumnsOffset.x;
		double start_pos_highp_y = (double)window->Pos.y + window->WindowPadding.y - (double)window->Scroll.y + window->DecoOuterSizeY1;
		window->DC.CursorStartPos = KGVec2((float)start_pos_highp_x, (float)start_pos_highp_y);
		window->DC.CursorStartPosLossyness = KGVec2((float)(start_pos_highp_x - window->DC.CursorStartPos.x), (float)(start_pos_highp_y - window->DC.CursorStartPos.y));
		window->DC.CursorPos = window->DC.CursorStartPos;
		window->DC.CursorPosPrevLine = window->DC.CursorPos;
		window->DC.CursorMaxPos = window->DC.CursorStartPos;
		window->DC.IdealMaxPos = window->DC.CursorStartPos;
		window->DC.CurrLineSize = window->DC.PrevLineSize = KGVec2(0.0f, 0.0f);
		window->DC.CurrLineTextBaseOffset = window->DC.PrevLineTextBaseOffset = 0.0f;
		window->DC.IsSameLine = window->DC.IsSetPos = false;

		window->DC.NavLayerCurrent = KGGuiNavLayer_Main;
		window->DC.NavLayersActiveMask = window->DC.NavLayersActiveMaskNext;
		window->DC.NavLayersActiveMaskNext = 0x00;
		window->DC.NavHideHighlightOneFrame = false;
		window->DC.NavHasScroll = (window->ScrollMax.y > 0.0f);

		window->DC.MenuBarAppending = false;
		window->DC.MenuColumns.Update(style.ItemSpacing.x, window_just_activated_by_user);
		window->DC.TreeDepth = 0;
		window->DC.TreeJumpToParentOnPopMask = 0x00;
		window->DC.ChildWindows.resize(0);
		window->DC.StateStorage = &window->StateStorage;
		window->DC.CurrentColumns = NULL;
		window->DC.LayoutType = KGGuiLayoutType_Vertical;
		window->DC.ParentLayoutType = parent_window ? parent_window->DC.LayoutType : KGGuiLayoutType_Vertical;

		window->DC.ItemWidth = window->ItemWidthDefault;
		window->DC.TextWrapPos = -1.0f; // disabled
		window->DC.ItemWidthStack.resize(0);
		window->DC.TextWrapPosStack.resize(0);

		if (window->AutoFitFramesX > 0)
			window->AutoFitFramesX--;
		if (window->AutoFitFramesY > 0)
			window->AutoFitFramesY--;

		// Apply focus (we need to call FocusWindow() AFTER setting DC.CursorStartPos so our initial navigation reference rectangle can start around there)
		if (want_focus)
		{
			KarmaGuiInternal::FocusWindow(window);
			KarmaGuiInternal::NavInitWindow(window, false); // <-- this is in the way for us to be able to defer and sort reappearing FocusWindow() calls
		}

		// Close requested by platform window
		if (p_open != NULL && window->Viewport->PlatformRequestClose && window->Viewport != GetMainViewport())
		{
			if (!window->DockIsActive || window->DockTabIsVisible)
			{
				window->Viewport->PlatformRequestClose = false;
				g.NavWindowingToggleLayer = false; // Assume user mapped PlatformRequestClose on ALT-F4 so we disable ALT for menu toggle. False positive not an issue.
				KR_CORE_INFO("[viewport] Window '{0}' PlatformRequestClose", window->Name);
				*p_open = false;
			}
		}

		// Title bar
		if (!(flags & KGGuiWindowFlags_NoTitleBar) && !window->DockIsActive)
			KarmaGuiInternal::RenderWindowTitleBarContents(window, KGRect(title_bar_rect.Min.x + window->WindowBorderSize, title_bar_rect.Min.y, title_bar_rect.Max.x - window->WindowBorderSize, title_bar_rect.Max.y), name, p_open);

		// Clear hit test shape every frame
		window->HitTestHoleSize.x = window->HitTestHoleSize.y = 0;

		// Pressing CTRL+C while holding on a window copy its content to the clipboard
		// This works but 1. doesn't handle multiple Begin/End pairs, 2. recursing into another Begin/End pair - so we need to work that out and add better logging scope.
		// Maybe we can support CTRL+C on every element?
		/*
		//if (g.NavWindow == window && g.ActiveId == 0)
		if (g.ActiveId == window->MoveId)
			if (g.IO.KeyCtrl && IsKeyPressed(KGGuiKey_C))
				LogToClipboard();
		*/

		if (g.IO.ConfigFlags & KGGuiConfigFlags_DockingEnable)
		{
			// Docking: Dragging a dockable window (or any of its child) turns it into a drag and drop source.
			// We need to do this _before_ we overwrite window->DC.LastItemId below because BeginDockableDragDropSource() also overwrites it.
			if ((g.MovingWindow == window) && (g.IO.ConfigDockingWithShift == g.IO.KeyShift))
				if ((window->RootWindowDockTree->Flags & KGGuiWindowFlags_NoDocking) == 0)
					KarmaGuiInternal::BeginDockableDragDropSource(window);

			// Docking: Any dockable window can act as a target. For dock node hosts we call BeginDockableDragDropTarget() in DockNodeUpdate() instead.
			if (g.DragDropActive && !(flags & KGGuiWindowFlags_NoDocking))
				if (g.MovingWindow == NULL || g.MovingWindow->RootWindowDockTree != window)
					if ((window == window->RootWindowDockTree) && !(window->Flags & KGGuiWindowFlags_DockNodeHost))
						KarmaGuiInternal::BeginDockableDragDropTarget(window);
		}

		// We fill last item data based on Title Bar/Tab, in order for IsItemHovered() and IsItemActive() to be usable after Begin().
		// This is useful to allow creating context menus on title bar only, etc.
		if (window->DockIsActive)
			KarmaGuiInternal::SetLastItemData(window->MoveId, g.CurrentItemFlags, window->DockTabItemStatusFlags, window->DockTabItemRect);
		else
			KarmaGuiInternal::SetLastItemData(window->MoveId, g.CurrentItemFlags, IsMouseHoveringRect(title_bar_rect.Min, title_bar_rect.Max, false) ? KGGuiItemStatusFlags_HoveredRect : 0, title_bar_rect);

		// [DEBUG]
#ifndef KARMAGUI_DISABLE_DEBUG_TOOLS
		if (g.DebugLocateId != 0 && (window->ID == g.DebugLocateId || window->MoveId == g.DebugLocateId))
			KarmaGuiInternal::DebugLocateItemResolveWithLastItem();
#endif

		// [Test Engine] Register title bar / tab
#ifdef IMGUI_ENABLE_TEST_ENGINE
		if (!(window->Flags & KGGuiWindowFlags_NoTitleBar))
			KARMAGUI_TEST_ENGINE_ITEM_ADD(g.LastItemData.Rect, g.LastItemData.ID);
#endif
	}
	else
	{
		// Append
		KarmaGuiInternal::SetCurrentViewport(window, window->Viewport);
		SetCurrentWindow(window);
	}

	if (!(flags & KGGuiWindowFlags_DockNodeHost))
		PushClipRect(window->InnerClipRect.Min, window->InnerClipRect.Max, true);

	// Clear 'accessed' flag last thing (After PushClipRect which will set the flag. We want the flag to stay false when the default "Debug" window is unused)
	window->WriteAccessed = false;
	window->BeginCount++;
	g.NextWindowData.ClearFlags();

	// Update visibility
	if (first_begin_of_the_frame)
	{
		// When we are about to select this tab (which will only be visible on the _next frame_), flag it with a non-zero HiddenFramesCannotSkipItems.
		// This will have the important effect of actually returning true in Begin() and not setting SkipItems, allowing an earlier submission of the window contents.
		// This is analogous to regular windows being hidden from one frame.
		// It is especially important as e.g. nested TabBars would otherwise generate flicker in the form of one empty frame, or focus requests won't be processed.
		if (window->DockIsActive && !window->DockTabIsVisible)
		{
			if (window->LastFrameJustFocused == g.FrameCount)
				window->HiddenFramesCannotSkipItems = 1;
			else
				window->HiddenFramesCanSkipItems = 1;
		}

		if (flags & KGGuiWindowFlags_ChildWindow)
		{
			// Child window can be out of sight and have "negative" clip windows.
			// Mark them as collapsed so commands are skipped earlier (we can't manually collapse them because they have no title bar).
			KR_CORE_ASSERT((flags & KGGuiWindowFlags_NoTitleBar) != 0 || (window->DockIsActive), "");
			if (!(flags & KGGuiWindowFlags_AlwaysAutoResize) && window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0) // FIXME: Doesn't make sense for ChildWindow??
			{
				const bool nav_request = (flags & KGGuiWindowFlags_NavFlattened) && (g.NavAnyRequest && g.NavWindow && g.NavWindow->RootWindowForNav == window->RootWindowForNav);
				if (!g.LogEnabled && !nav_request)
					if (window->OuterRectClipped.Min.x >= window->OuterRectClipped.Max.x || window->OuterRectClipped.Min.y >= window->OuterRectClipped.Max.y)
						window->HiddenFramesCanSkipItems = 1;
			}

			// Hide along with parent or if parent is collapsed
			if (parent_window && (parent_window->Collapsed || parent_window->HiddenFramesCanSkipItems > 0))
				window->HiddenFramesCanSkipItems = 1;
			if (parent_window && (parent_window->Collapsed || parent_window->HiddenFramesCannotSkipItems > 0))
				window->HiddenFramesCannotSkipItems = 1;
		}

		// Don't render if style alpha is 0.0 at the time of Begin(). This is arbitrary and inconsistent but has been there for a long while (may remove at some point)
		if (style.Alpha <= 0.0f)
			window->HiddenFramesCanSkipItems = 1;

		// Update the Hidden flag
		bool hidden_regular = (window->HiddenFramesCanSkipItems > 0) || (window->HiddenFramesCannotSkipItems > 0);
		window->Hidden = hidden_regular || (window->HiddenFramesForRenderOnly > 0);

		// Disable inputs for requested number of frames
		if (window->DisableInputsFrames > 0)
		{
			window->DisableInputsFrames--;
			window->Flags |= KGGuiWindowFlags_NoInputs;
		}

		// Update the SkipItems flag, used to early out of all items functions (no layout required)
		bool skip_items = false;
		if (window->Collapsed || !window->Active || hidden_regular)
			if (window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0 && window->HiddenFramesCannotSkipItems <= 0)
				skip_items = true;
		window->SkipItems = skip_items;

		// Restore NavLayersActiveMaskNext to previous value when not visible, so a CTRL+Tab back can use a safe value.
		if (window->SkipItems)
			window->DC.NavLayersActiveMaskNext = window->DC.NavLayersActiveMask;

		// Sanity check: there are two spots which can set Appearing = true
		// - when 'window_just_activated_by_user' is set -> HiddenFramesCannotSkipItems is set -> SkipItems always false
		// - in BeginDocked() path when DockNodeIsVisible == DockTabIsVisible == true -> hidden _should_ be all zero // FIXME: Not formally proven, hence the assert.
		if (window->SkipItems && !window->Appearing)
		{
			KR_CORE_ASSERT(window->Appearing == false, ""); // Please report on GitHub if this triggers: https://github.com/ocornut/imgui/issues/4177
		}
	}

	return !window->SkipItems;
}

void Karma::KarmaGui::End()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;

	// Error checking: verify that user hasn't called End() too many times!
	if (g.CurrentWindowStack.Size <= 1 && g.WithinFrameScopeWithImplicitWindow)
	{
		KR_CORE_ASSERT(g.CurrentWindowStack.Size > 1, "Calling End() too many times!");
		return;
	}
	KR_CORE_ASSERT(g.CurrentWindowStack.Size > 0, "");

	// Error checking: verify that user doesn't directly call End() on a child window.
	if ((window->Flags & KGGuiWindowFlags_ChildWindow) && !(window->Flags & KGGuiWindowFlags_DockNodeHost) && !window->DockIsActive)
	{
		KR_CORE_ASSERT(g.WithinEndChild, "Must call EndChild() and not End()!");
	}

	// Close anything that is open
	if (window->DC.CurrentColumns)
		KarmaGuiInternal::EndColumns();
	if (!(window->Flags & KGGuiWindowFlags_DockNodeHost))   // Pop inner window clip rectangle
		PopClipRect();
	KarmaGuiInternal::PopFocusScope();

	// Stop logging
	if (!(window->Flags & KGGuiWindowFlags_ChildWindow))    // FIXME: add more options for scope of logging
		LogFinish();

	if (window->DC.IsSetPos)
		KarmaGuiInternal::ErrorCheckUsingSetCursorPosToExtendParentBoundaries();

	// Docking: report contents sizes to parent to allow for auto-resize
	if (window->DockNode && window->DockTabIsVisible)
		if (KGGuiWindow* host_window = window->DockNode->HostWindow)         // FIXME-DOCK
			host_window->DC.CursorMaxPos = window->DC.CursorMaxPos + window->WindowPadding - host_window->WindowPadding;

	// Pop from window stack
	g.LastItemData = g.CurrentWindowStack.back().ParentLastItemDataBackup;
	if (window->Flags & KGGuiWindowFlags_ChildMenu)
		g.BeginMenuCount--;
	if (window->Flags & KGGuiWindowFlags_Popup)
		g.BeginPopupStack.pop_back();
	g.CurrentWindowStack.back().StackSizesOnBegin.CompareWithCurrentState();
	g.CurrentWindowStack.pop_back();
	SetCurrentWindow(g.CurrentWindowStack.Size == 0 ? NULL : g.CurrentWindowStack.back().Window);
	if (g.CurrentWindow)
		KarmaGuiInternal::SetCurrentViewport(g.CurrentWindow, g.CurrentWindow->Viewport);
}

void Karma::KarmaGuiInternal::BringWindowToFocusFront(KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(window == window->RootWindow, "");

	const int cur_order = window->FocusOrder;
	KR_CORE_ASSERT(g.WindowsFocusOrder[cur_order] == window, "");
	if (g.WindowsFocusOrder.back() == window)
		return;

	const int new_order = g.WindowsFocusOrder.Size - 1;
	for (int n = cur_order; n < new_order; n++)
	{
		g.WindowsFocusOrder[n] = g.WindowsFocusOrder[n + 1];
		g.WindowsFocusOrder[n]->FocusOrder--;
		KR_CORE_ASSERT(g.WindowsFocusOrder[n]->FocusOrder == n, "");
	}
	g.WindowsFocusOrder[new_order] = window;
	window->FocusOrder = (short)new_order;
}

void Karma::KarmaGuiInternal::BringWindowToDisplayFront(KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* current_front_window = g.Windows.back();
	if (current_front_window == window || current_front_window->RootWindowDockTree == window) // Cheap early out (could be better)
		return;
	for (int i = g.Windows.Size - 2; i >= 0; i--) // We can ignore the top-most window
		if (g.Windows[i] == window)
		{
			memmove(&g.Windows[i], &g.Windows[i + 1], (size_t)(g.Windows.Size - i - 1) * sizeof(KGGuiWindow*));
			g.Windows[g.Windows.Size - 1] = window;
			break;
		}
}

void Karma::KarmaGuiInternal::BringWindowToDisplayBack(KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.Windows[0] == window)
		return;
	for (int i = 0; i < g.Windows.Size; i++)
		if (g.Windows[i] == window)
		{
			memmove(&g.Windows[1], &g.Windows[0], (size_t)i * sizeof(KGGuiWindow*));
			g.Windows[0] = window;
			break;
		}
}

void Karma::KarmaGuiInternal::BringWindowToDisplayBehind(KGGuiWindow* window, KGGuiWindow* behind_window)
{
	KR_CORE_ASSERT(window != NULL && behind_window != NULL, "");
	KarmaGuiContext& g = *GKarmaGui;
	window = window->RootWindow;
	behind_window = behind_window->RootWindow;
	int pos_wnd = FindWindowDisplayIndex(window);
	int pos_beh = FindWindowDisplayIndex(behind_window);
	if (pos_wnd < pos_beh)
	{
		size_t copy_bytes = (pos_beh - pos_wnd - 1) * sizeof(KGGuiWindow*);
		memmove(&g.Windows.Data[pos_wnd], &g.Windows.Data[pos_wnd + 1], copy_bytes);
		g.Windows[pos_beh - 1] = window;
	}
	else
	{
		size_t copy_bytes = (pos_wnd - pos_beh) * sizeof(KGGuiWindow*);
		memmove(&g.Windows.Data[pos_beh + 1], &g.Windows.Data[pos_beh], copy_bytes);
		g.Windows[pos_beh] = window;
	}
}

int Karma::KarmaGuiInternal::FindWindowDisplayIndex(KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.Windows.index_from_ptr(g.Windows.find(window));
}

// Moving window to front of display and set focus (which happens to be back of our sorted list)
void Karma::KarmaGuiInternal::FocusWindow(KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;

	if (g.NavWindow != window)
	{
		SetNavWindow(window);
		if (window && g.NavDisableMouseHover)
			g.NavMousePosDirty = true;
		g.NavId = window ? window->NavLastIds[0] : 0; // Restore NavId
		g.NavLayer = KGGuiNavLayer_Main;
		g.NavFocusScopeId = window ? window->NavRootFocusScopeId : 0;
		g.NavIdIsAlive = false;

		// Close popups if any
		ClosePopupsOverWindow(window, false);
	}

	// Move the root window to the top of the pile
	KR_CORE_ASSERT(window == NULL || window->RootWindowDockTree != NULL, "");
	KGGuiWindow* focus_front_window = window ? window->RootWindow : NULL;
	KGGuiWindow* display_front_window = window ? window->RootWindowDockTree : NULL;
	KGGuiDockNode* dock_node = window ? window->DockNode : NULL;
	bool active_id_window_is_dock_node_host = (g.ActiveIdWindow && dock_node && dock_node->HostWindow == g.ActiveIdWindow);

	// Steal active widgets. Some of the cases it triggers includes:
	// - Focus a window while an InputText in another window is active, if focus happens before the old InputText can run.
	// - When using Nav to activate menu items (due to timing of activating on press->new window appears->losing ActiveId)
	// - Using dock host items (tab, collapse button) can trigger this before we redirect the ActiveIdWindow toward the child window.
	if (g.ActiveId != 0 && g.ActiveIdWindow && g.ActiveIdWindow->RootWindow != focus_front_window)
		if (!g.ActiveIdNoClearOnFocusLoss && !active_id_window_is_dock_node_host)
			ClearActiveID();

	// Passing NULL allow to disable keyboard focus
	if (!window)
		return;
	window->LastFrameJustFocused = g.FrameCount;

	// Select in dock node
	if (dock_node && dock_node->TabBar)
		dock_node->TabBar->SelectedTabId = dock_node->TabBar->NextSelectedTabId = window->TabId;

	// Bring to front
	BringWindowToFocusFront(focus_front_window);
	if (((window->Flags | focus_front_window->Flags | display_front_window->Flags) & KGGuiWindowFlags_NoBringToFrontOnFocus) == 0)
		BringWindowToDisplayFront(display_front_window);
}

void Karma::KarmaGuiInternal::FocusTopMostWindowUnderOne(KGGuiWindow* under_this_window, KGGuiWindow* ignore_window)
{
	KarmaGuiContext& g = *GKarmaGui;
	int start_idx = g.WindowsFocusOrder.Size - 1;
	if (under_this_window != NULL)
	{
		// Aim at root window behind us, if we are in a child window that's our own root (see #4640)
		int offset = -1;
		while (under_this_window->Flags & KGGuiWindowFlags_ChildWindow)
		{
			under_this_window = under_this_window->ParentWindow;
			offset = 0;
		}
		start_idx = FindWindowFocusIndex(under_this_window) + offset;
	}
	for (int i = start_idx; i >= 0; i--)
	{
		// We may later decide to test for different NoXXXInputs based on the active navigation input (mouse vs nav) but that may feel more confusing to the user.
		KGGuiWindow* window = g.WindowsFocusOrder[i];
		KR_CORE_ASSERT(window == window->RootWindow, "");
		if (window != ignore_window && window->WasActive)
			if ((window->Flags & (KGGuiWindowFlags_NoMouseInputs | KGGuiWindowFlags_NoNavInputs)) != (KGGuiWindowFlags_NoMouseInputs | KGGuiWindowFlags_NoNavInputs))
			{
				// FIXME-DOCK: This is failing (lagging by one frame) for docked windows.
				// If A and B are docked into window and B disappear, at the NewFrame() call site window->NavLastChildNavWindow will still point to B.
				// We might leverage the tab order implicitly stored in window->DockNodeAsHost->TabBar (essentially the 'most_recently_selected_tab' code in tab bar will do that but on next update)
				// to tell which is the "previous" window. Or we may leverage 'LastFrameFocused/LastFrameJustFocused' and have this function handle child window itself?
				KGGuiWindow* focus_window = NavRestoreLastChildNavWindow(window);
				FocusWindow(focus_window);
				return;
			}
	}
	FocusWindow(NULL);
}

// Important: this alone doesn't alter current KGDrawList state. This is called by PushFont/PopFont only.
void Karma::KarmaGuiInternal::SetCurrentFont(KGFont* font)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(font && font->IsLoaded(), "");    // Font Atlas not created. Did you call io.Fonts->GetTexDataAsRGBA32 / GetTexDataAsAlpha8 ?
	KR_CORE_ASSERT(font->Scale > 0.0f, "");
	g.Font = font;
	g.FontBaseSize = KGMax(1.0f, g.IO.FontGlobalScale * g.Font->FontSize * g.Font->Scale);
	g.FontSize = g.CurrentWindow ? g.CurrentWindow->CalcFontSize() : 0.0f;

	KGFontAtlas* atlas = g.Font->ContainerAtlas;
	g.DrawListSharedData.TexUvWhitePixel = atlas->TexUvWhitePixel;
	g.DrawListSharedData.TexUvLines = atlas->TexUvLines;
	g.DrawListSharedData.Font = g.Font;
	g.DrawListSharedData.FontSize = g.FontSize;
}

void Karma::KarmaGui::PushFont(KGFont* font)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (!font)
		font = KarmaGuiInternal::GetDefaultFont();
	KarmaGuiInternal::SetCurrentFont(font);
	g.FontStack.push_back(font);
	g.CurrentWindow->DrawList->PushTextureID(font->ContainerAtlas->TexID);
}

void Karma::KarmaGui::PopFont()
{
	KarmaGuiContext& g = *GKarmaGui;
	g.CurrentWindow->DrawList->PopTextureID();
	g.FontStack.pop_back();
	KarmaGuiInternal::SetCurrentFont(g.FontStack.empty() ? KarmaGuiInternal::GetDefaultFont() : g.FontStack.back());
}

void Karma::KarmaGuiInternal::PushItemFlag(KGGuiItemFlags option, bool enabled)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiItemFlags item_flags = g.CurrentItemFlags;
	KR_CORE_ASSERT(item_flags == g.ItemFlagsStack.back(), "");
	if (enabled)
		item_flags |= option;
	else
		item_flags &= ~option;
	g.CurrentItemFlags = item_flags;
	g.ItemFlagsStack.push_back(item_flags);
}

void Karma::KarmaGuiInternal::PopItemFlag()
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(g.ItemFlagsStack.Size > 1, ""); // Too many calls to PopItemFlag() - we always leave a 0 at the bottom of the stack.
	g.ItemFlagsStack.pop_back();
	g.CurrentItemFlags = g.ItemFlagsStack.back();
}

// BeginDisabled()/EndDisabled()
// - Those can be nested but it cannot be used to enable an already disabled section (a single BeginDisabled(true) in the stack is enough to keep everything disabled)
// - Visually this is currently altering alpha, but it is expected that in a future styling system this would work differently.
// - Feedback welcome at https://github.com/ocornut/imgui/issues/211
// - BeginDisabled(false) essentially does nothing useful but is provided to facilitate use of boolean expressions. If you can avoid calling BeginDisabled(False)/EndDisabled() best to avoid it.
// - Optimized shortcuts instead of PushStyleVar() + PushItemFlag()
void Karma::KarmaGui::BeginDisabled(bool disabled)
{
	KarmaGuiContext& g = *GKarmaGui;
	bool was_disabled = (g.CurrentItemFlags & KGGuiItemFlags_Disabled) != 0;
	if (!was_disabled && disabled)
	{
		g.DisabledAlphaBackup = g.Style.Alpha;
		g.Style.Alpha *= g.Style.DisabledAlpha; // PushStyleVar(KGGuiStyleVar_Alpha, g.Style.Alpha * g.Style.DisabledAlpha);
	}
	if (was_disabled || disabled)
		g.CurrentItemFlags |= KGGuiItemFlags_Disabled;
	g.ItemFlagsStack.push_back(g.CurrentItemFlags);
	g.DisabledStackSize++;
}

void Karma::KarmaGui::EndDisabled()
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(g.DisabledStackSize > 0, "");
	g.DisabledStackSize--;
	bool was_disabled = (g.CurrentItemFlags & KGGuiItemFlags_Disabled) != 0;
	//PopItemFlag();
	g.ItemFlagsStack.pop_back();
	g.CurrentItemFlags = g.ItemFlagsStack.back();
	if (was_disabled && (g.CurrentItemFlags & KGGuiItemFlags_Disabled) == 0)
		g.Style.Alpha = g.DisabledAlphaBackup; //PopStyleVar();
}

// FIXME: Look into renaming this once we have settled the new Focus/Activation/TabStop system.
void Karma::KarmaGui::PushAllowKeyboardFocus(bool allow_keyboard_focus)
{
	KarmaGuiInternal::PushItemFlag(KGGuiItemFlags_NoTabStop, !allow_keyboard_focus);
}

void Karma::KarmaGui::PopAllowKeyboardFocus()
{
	KarmaGuiInternal::PopItemFlag();
}

void Karma::KarmaGui::PushButtonRepeat(bool repeat)
{
	KarmaGuiInternal::PushItemFlag(KGGuiItemFlags_ButtonRepeat, repeat);
}

void Karma::KarmaGui::PopButtonRepeat()
{
	KarmaGuiInternal::PopItemFlag();
}

void Karma::KarmaGui::PushTextWrapPos(float wrap_pos_x)
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	window->DC.TextWrapPosStack.push_back(window->DC.TextWrapPos);
	window->DC.TextWrapPos = wrap_pos_x;
}

void Karma::KarmaGui::PopTextWrapPos()
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	window->DC.TextWrapPos = window->DC.TextWrapPosStack.back();
	window->DC.TextWrapPosStack.pop_back();
}

static KGGuiWindow* GetCombinedRootWindow(KGGuiWindow* window, bool popup_hierarchy, bool dock_hierarchy)
{
	KGGuiWindow* last_window = NULL;
	while (last_window != window)
	{
		last_window = window;
		window = window->RootWindow;
		if (popup_hierarchy)
			window = window->RootWindowPopupTree;
		if (dock_hierarchy)
			window = window->RootWindowDockTree;
	}
	return window;
}

bool Karma::KarmaGuiInternal::IsWindowChildOf(KGGuiWindow* window, KGGuiWindow* potential_parent, bool popup_hierarchy, bool dock_hierarchy)
{
	KGGuiWindow* window_root = GetCombinedRootWindow(window, popup_hierarchy, dock_hierarchy);
	if (window_root == potential_parent)
		return true;
	while (window != NULL)
	{
		if (window == potential_parent)
			return true;
		if (window == window_root) // end of chain
			return false;
		window = window->ParentWindow;
	}
	return false;
}

bool Karma::KarmaGuiInternal::IsWindowWithinBeginStackOf(KGGuiWindow* window, KGGuiWindow* potential_parent)
{
	if (window->RootWindow == potential_parent)
		return true;
	while (window != NULL)
	{
		if (window == potential_parent)
			return true;
		window = window->ParentWindowInBeginStack;
	}
	return false;
}

bool Karma::KarmaGuiInternal::IsWindowAbove(KGGuiWindow* potential_above, KGGuiWindow* potential_below)
{
	KarmaGuiContext& g = *GKarmaGui;

	// It would be saner to ensure that display layer is always reflected in the g.Windows[] order, which would likely requires altering all manipulations of that array
	const int display_layer_delta = GetWindowDisplayLayer(potential_above) - GetWindowDisplayLayer(potential_below);
	if (display_layer_delta != 0)
		return display_layer_delta > 0;

	for (int i = g.Windows.Size - 1; i >= 0; i--)
	{
		KGGuiWindow* candidate_window = g.Windows[i];
		if (candidate_window == potential_above)
			return true;
		if (candidate_window == potential_below)
			return false;
	}
	return false;
}

bool Karma::KarmaGui::IsWindowHovered(KarmaGuiHoveredFlags flags)
{
	KR_CORE_ASSERT((flags & (KGGuiHoveredFlags_AllowWhenOverlapped | KGGuiHoveredFlags_AllowWhenDisabled)) == 0, "");   // Flags not supported by this function
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* ref_window = g.HoveredWindow;
	KGGuiWindow* cur_window = g.CurrentWindow;
	if (ref_window == NULL)
		return false;

	if ((flags & KGGuiHoveredFlags_AnyWindow) == 0)
	{
		KR_CORE_ASSERT(cur_window, ""); // Not inside a Begin()/End()
		const bool popup_hierarchy = (flags & KGGuiHoveredFlags_NoPopupHierarchy) == 0;
		const bool dock_hierarchy = (flags & KGGuiHoveredFlags_DockHierarchy) != 0;
		if (flags & KGGuiHoveredFlags_RootWindow)
			cur_window = GetCombinedRootWindow(cur_window, popup_hierarchy, dock_hierarchy);

		bool result;
		if (flags & KGGuiHoveredFlags_ChildWindows)
			result = KarmaGuiInternal::IsWindowChildOf(ref_window, cur_window, popup_hierarchy, dock_hierarchy);
		else
			result = (ref_window == cur_window);
		if (!result)
			return false;
	}

	if (!IsWindowContentHoverable(ref_window, flags))
		return false;
	if (!(flags & KGGuiHoveredFlags_AllowWhenBlockedByActiveItem))
		if (g.ActiveId != 0 && !g.ActiveIdAllowOverlap && g.ActiveId != ref_window->MoveId)
			return false;
	return true;
}

bool Karma::KarmaGui::IsWindowFocused(KarmaGuiFocusedFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* ref_window = g.NavWindow;
	KGGuiWindow* cur_window = g.CurrentWindow;

	if (ref_window == NULL)
		return false;
	if (flags & KGGuiFocusedFlags_AnyWindow)
		return true;

	KR_CORE_ASSERT(cur_window, ""); // Not inside a Begin()/End()
	const bool popup_hierarchy = (flags & KGGuiFocusedFlags_NoPopupHierarchy) == 0;
	const bool dock_hierarchy = (flags & KGGuiFocusedFlags_DockHierarchy) != 0;
	if (flags & KGGuiHoveredFlags_RootWindow)
		cur_window = GetCombinedRootWindow(cur_window, popup_hierarchy, dock_hierarchy);

	if (flags & KGGuiHoveredFlags_ChildWindows)
		return KarmaGuiInternal::IsWindowChildOf(ref_window, cur_window, popup_hierarchy, dock_hierarchy);
	else
		return (ref_window == cur_window);
}

KGGuiID Karma::KarmaGui::GetWindowDockID()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.CurrentWindow->DockId;
}

bool Karma::KarmaGui::IsWindowDocked()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.CurrentWindow->DockIsActive;
}

// Can we focus this window with CTRL+TAB (or PadMenu + PadFocusPrev/PadFocusNext)
// Note that NoNavFocus makes the window not reachable with CTRL+TAB but it can still be focused with mouse or programmatically.
// If you want a window to never be focused, you may use the e.g. NoInputs flag.
bool Karma::KarmaGuiInternal::IsWindowNavFocusable(KGGuiWindow* window)
{
	return window->WasActive && window == window->RootWindow && !(window->Flags & KGGuiWindowFlags_NoNavFocus);
}

float Karma::KarmaGui::GetWindowWidth()
{
	KGGuiWindow* window = GKarmaGui->CurrentWindow;
	return window->Size.x;
}

float Karma::KarmaGui::GetWindowHeight()
{
	KGGuiWindow* window = GKarmaGui->CurrentWindow;
	return window->Size.y;
}

KGVec2 Karma::KarmaGui::GetWindowPos()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	return window->Pos;
}

void Karma::KarmaGuiInternal::SetWindowPos(KGGuiWindow* window, const KGVec2& pos, KarmaGuiCond cond)
{
	// Test condition (NB: bit 0 is always true) and clear flags for next time
	if (cond && (window->SetWindowPosAllowFlags & cond) == 0)
		return;

	KR_CORE_ASSERT(cond == 0 || KGIsPowerOfTwo(cond), ""); // Make sure the user doesn't attempt to combine multiple condition flags.
	window->SetWindowPosAllowFlags &= ~(KGGuiCond_Once | KGGuiCond_FirstUseEver | KGGuiCond_Appearing);
	window->SetWindowPosVal = KGVec2(FLT_MAX, FLT_MAX);

	// Set
	const KGVec2 old_pos = window->Pos;
	window->Pos = KGFloor(pos);
	KGVec2 offset = window->Pos - old_pos;
	if (offset.x == 0.0f && offset.y == 0.0f)
		return;
	MarkIniSettingsDirty(window);
	// FIXME: share code with TranslateWindow(), need to confirm whether the 3 rect modified by TranslateWindow() are desirable here.
	window->DC.CursorPos += offset;         // As we happen to move the window while it is being appended to (which is a bad idea - will smear) let's at least offset the cursor
	window->DC.CursorMaxPos += offset;      // And more importantly we need to offset CursorMaxPos/CursorStartPos this so ContentSize calculation doesn't get affected.
	window->DC.IdealMaxPos += offset;
	window->DC.CursorStartPos += offset;
}

void Karma::KarmaGui::SetWindowPos(const KGVec2& pos, KarmaGuiCond cond)
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindowRead();
	KarmaGuiInternal::SetWindowPos(window, pos, cond);
}

void Karma::KarmaGui::SetWindowPos(const char* name, const KGVec2& pos, KarmaGuiCond cond)
{
	if (KGGuiWindow* window = KarmaGuiInternal::FindWindowByName(name))
		KarmaGuiInternal::SetWindowPos(window, pos, cond);
}

KGVec2 Karma::KarmaGui::GetWindowSize()
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindowRead();
	return window->Size;
}

void Karma::KarmaGuiInternal::SetWindowSize(KGGuiWindow* window, const KGVec2& size, KarmaGuiCond cond)
{
	// Test condition (NB: bit 0 is always true) and clear flags for next time
	if (cond && (window->SetWindowSizeAllowFlags & cond) == 0)
		return;

	KR_CORE_ASSERT(cond == 0 || KGIsPowerOfTwo(cond), ""); // Make sure the user doesn't attempt to combine multiple condition flags.
	window->SetWindowSizeAllowFlags &= ~(KGGuiCond_Once | KGGuiCond_FirstUseEver | KGGuiCond_Appearing);

	// Set
	KGVec2 old_size = window->SizeFull;
	window->AutoFitFramesX = (size.x <= 0.0f) ? 2 : 0;
	window->AutoFitFramesY = (size.y <= 0.0f) ? 2 : 0;
	if (size.x <= 0.0f)
		window->AutoFitOnlyGrows = false;
	else
		window->SizeFull.x = KG_FLOOR(size.x);
	if (size.y <= 0.0f)
		window->AutoFitOnlyGrows = false;
	else
		window->SizeFull.y = KG_FLOOR(size.y);
	if (old_size.x != window->SizeFull.x || old_size.y != window->SizeFull.y)
		MarkIniSettingsDirty(window);
}

void Karma::KarmaGui::SetWindowSize(const KGVec2& size, KarmaGuiCond cond)
{
	KarmaGuiInternal::SetWindowSize(GKarmaGui->CurrentWindow, size, cond);
}

void Karma::KarmaGui::SetWindowSize(const char* name, const KGVec2& size, KarmaGuiCond cond)
{
	if (KGGuiWindow* window = KarmaGuiInternal::FindWindowByName(name))
		KarmaGuiInternal::SetWindowSize(window, size, cond);
}

void Karma::KarmaGuiInternal::SetWindowCollapsed(KGGuiWindow* window, bool collapsed, KarmaGuiCond cond)
{
	// Test condition (NB: bit 0 is always true) and clear flags for next time
	if (cond && (window->SetWindowCollapsedAllowFlags & cond) == 0)
		return;
	window->SetWindowCollapsedAllowFlags &= ~(KGGuiCond_Once | KGGuiCond_FirstUseEver | KGGuiCond_Appearing);

	// Set
	window->Collapsed = collapsed;
}

void Karma::KarmaGuiInternal::SetWindowHitTestHole(KGGuiWindow* window, const KGVec2& pos, const KGVec2& size)
{
	KR_CORE_ASSERT(window->HitTestHoleSize.x == 0, "");     // We don't support multiple holes/hit test filters
	window->HitTestHoleSize = KGVec2ih(size);
	window->HitTestHoleOffset = KGVec2ih(pos - window->Pos);
}

void Karma::KarmaGui::SetWindowCollapsed(bool collapsed, KarmaGuiCond cond)
{
	KarmaGuiInternal::SetWindowCollapsed(GKarmaGui->CurrentWindow, collapsed, cond);
}

bool Karma::KarmaGui::IsWindowCollapsed()
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindowRead();
	return window->Collapsed;
}

bool Karma::KarmaGui::IsWindowAppearing()
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindowRead();
	return window->Appearing;
}

void Karma::KarmaGui::SetWindowCollapsed(const char* name, bool collapsed, KarmaGuiCond cond)
{
	if (KGGuiWindow* window = KarmaGuiInternal::FindWindowByName(name))
		KarmaGuiInternal::SetWindowCollapsed(window, collapsed, cond);
}

void Karma::KarmaGui::SetWindowFocus()
{
	KarmaGuiInternal::FocusWindow(GKarmaGui->CurrentWindow);
}

void Karma::KarmaGui::SetWindowFocus(const char* name)
{
	if (name)
	{
		if (KGGuiWindow* window = KarmaGuiInternal::FindWindowByName(name))
			KarmaGuiInternal::FocusWindow(window);
	}
	else
	{
		KarmaGuiInternal::FocusWindow(NULL);
	}
}

void Karma::KarmaGui::SetNextWindowPos(const KGVec2& pos, KarmaGuiCond cond, const KGVec2& pivot)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(cond == 0 || KGIsPowerOfTwo(cond), ""); // Make sure the user (client) doesn't attempt to combine multiple condition flags.
	g.NextWindowData.Flags |= KGGuiNextWindowDataFlags_HasPos;
	g.NextWindowData.PosVal = pos;
	g.NextWindowData.PosPivotVal = pivot;
	g.NextWindowData.PosCond = cond ? cond : KGGuiCond_Always;
	g.NextWindowData.PosUndock = true;
}

void Karma::KarmaGui::SetNextWindowSize(const KGVec2& size, KarmaGuiCond cond)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(cond == 0 || KGIsPowerOfTwo(cond), ""); // Make sure the user doesn't attempt to combine multiple condition flags.
	g.NextWindowData.Flags |= KGGuiNextWindowDataFlags_HasSize;
	g.NextWindowData.SizeVal = size;
	g.NextWindowData.SizeCond = cond ? cond : KGGuiCond_Always;
}

void Karma::KarmaGui::SetNextWindowSizeConstraints(const KGVec2& size_min, const KGVec2& size_max, KarmaGuiSizeCallback custom_callback, void* custom_callback_user_data)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.NextWindowData.Flags |= KGGuiNextWindowDataFlags_HasSizeConstraint;
	g.NextWindowData.SizeConstraintRect = KGRect(size_min, size_max);
	g.NextWindowData.SizeCallback = custom_callback;
	g.NextWindowData.SizeCallbackUserData = custom_callback_user_data;
}

// Content size = inner scrollable rectangle, padded with WindowPadding.
// SetNextWindowContentSize(KGVec2(100,100) + KGGuiWindowFlags_AlwaysAutoResize will always allow submitting a 100x100 item.
void Karma::KarmaGui::SetNextWindowContentSize(const KGVec2& size)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.NextWindowData.Flags |= KGGuiNextWindowDataFlags_HasContentSize;
	g.NextWindowData.ContentSizeVal = KGFloor(size);
}

void Karma::KarmaGui::SetNextWindowScroll(const KGVec2& scroll)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.NextWindowData.Flags |= KGGuiNextWindowDataFlags_HasScroll;
	g.NextWindowData.ScrollVal = scroll;
}

void Karma::KarmaGui::SetNextWindowCollapsed(bool collapsed, KarmaGuiCond cond)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(cond == 0 || KGIsPowerOfTwo(cond), ""); // Make sure the user doesn't attempt to combine multiple condition flags.
	g.NextWindowData.Flags |= KGGuiNextWindowDataFlags_HasCollapsed;
	g.NextWindowData.CollapsedVal = collapsed;
	g.NextWindowData.CollapsedCond = cond ? cond : KGGuiCond_Always;
}

void Karma::KarmaGui::SetNextWindowFocus()
{
	KarmaGuiContext& g = *GKarmaGui;
	g.NextWindowData.Flags |= KGGuiNextWindowDataFlags_HasFocus;
}

void Karma::KarmaGui::SetNextWindowBgAlpha(float alpha)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.NextWindowData.Flags |= KGGuiNextWindowDataFlags_HasBgAlpha;
	g.NextWindowData.BgAlphaVal = alpha;
}

void Karma::KarmaGui::SetNextWindowViewport(KGGuiID id)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.NextWindowData.Flags |= KGGuiNextWindowDataFlags_HasViewport;
	g.NextWindowData.ViewportId = id;
}

void Karma::KarmaGui::SetNextWindowDockID(KGGuiID id, KarmaGuiCond cond)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.NextWindowData.Flags |= KGGuiNextWindowDataFlags_HasDock;
	g.NextWindowData.DockCond = cond ? cond : KGGuiCond_Always;
	g.NextWindowData.DockId = id;
}

void Karma::KarmaGui::SetNextWindowClass(const KarmaGuiWindowClass* window_class)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT((window_class->ViewportFlagsOverrideSet & window_class->ViewportFlagsOverrideClear) == 0, ""); // Cannot set both set and clear for the same bit
	g.NextWindowData.Flags |= KGGuiNextWindowDataFlags_HasWindowClass;
	g.NextWindowData.WindowClass = *window_class;
}

KGDrawList* Karma::KarmaGui::GetWindowDrawList()
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	return window->DrawList;
}

float Karma::KarmaGui::GetWindowDpiScale()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.CurrentDpiScale;
}

KarmaGuiViewport* Karma::KarmaGui::GetWindowViewport()
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(g.CurrentViewport != NULL && g.CurrentViewport == g.CurrentWindow->Viewport, "");
	return g.CurrentViewport;
}

KGFont* Karma::KarmaGui::GetFont()
{
	return GKarmaGui->Font;
}

float Karma::KarmaGui::GetFontSize()
{
	return GKarmaGui->FontSize;
}

KGVec2 Karma::KarmaGui::GetFontTexUvWhitePixel()
{
	return GKarmaGui->DrawListSharedData.TexUvWhitePixel;
}

void Karma::KarmaGui::SetWindowFontScale(float scale)
{
	KR_CORE_ASSERT(scale > 0.0f, "");
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	window->FontWindowScale = scale;
	g.FontSize = g.DrawListSharedData.FontSize = window->CalcFontSize();
}

void Karma::KarmaGuiInternal::ActivateItem(KGGuiID id)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.NavNextActivateId = id;
	g.NavNextActivateFlags = KGGuiActivateFlags_None;
}

void Karma::KarmaGuiInternal::PushFocusScope(KGGuiID id)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.FocusScopeStack.push_back(id);
	g.CurrentFocusScopeId = id;
}

void Karma::KarmaGuiInternal::PopFocusScope()
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(g.FocusScopeStack.Size > 0, ""); // Too many PopFocusScope() ?
	g.FocusScopeStack.pop_back();
	g.CurrentFocusScopeId = g.FocusScopeStack.Size ? g.FocusScopeStack.back() : 0;
}

// Note: this will likely be called ActivateItem() once we rework our Focus/Activation system!
void Karma::KarmaGui::SetKeyboardFocusHere(int offset)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	KR_CORE_ASSERT(offset >= -1, "");    // -1 is allowed but not below
	KR_CORE_INFO("SetKeyboardFocusHere(%d) in window \"%s\"\n", offset, window->Name);

	// It makes sense in the vast majority of cases to never interrupt a drag and drop.
	// When we refactor this function into ActivateItem() we may want to make this an option.
	// MovingWindow is protected from most user inputs using SetActiveIdUsingNavAndKeys(), but
	// is also automatically dropped in the event g.ActiveId is stolen.
	if (g.DragDropActive || g.MovingWindow != NULL)
	{
		KR_CORE_INFO("SetKeyboardFocusHere() ignored while DragDropActive!\n");
		return;
	}

	KarmaGuiInternal::SetNavWindow(window);

	KGGuiScrollFlags scroll_flags = window->Appearing ? KGGuiScrollFlags_KeepVisibleEdgeX | KGGuiScrollFlags_AlwaysCenterY : KGGuiScrollFlags_KeepVisibleEdgeX | KGGuiScrollFlags_KeepVisibleEdgeY;
	KarmaGuiInternal::NavMoveRequestSubmit(KGGuiDir_None, offset < 0 ? KGGuiDir_Up : KGGuiDir_Down, KGGuiNavMoveFlags_Tabbing | KGGuiNavMoveFlags_FocusApi, scroll_flags); // FIXME-NAV: Once we refactor tabbing, add LegacyApi flag to not activate non-inputable.
	if (offset == -1)
	{
		KarmaGuiInternal::NavMoveRequestResolveWithLastItem(&g.NavMoveResultLocal);
	}
	else
	{
		g.NavTabbingDir = 1;
		g.NavTabbingCounter = offset + 1;
	}
}

void Karma::KarmaGui::SetItemDefaultFocus()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	if (!window->Appearing)
		return;
	if (g.NavWindow != window->RootWindowForNav || (!g.NavInitRequest && g.NavInitResultId == 0) || g.NavLayer != window->DC.NavLayerCurrent)
		return;

	g.NavInitRequest = false;
	g.NavInitResultId = g.LastItemData.ID;
	g.NavInitResultRectRel = KarmaGuiInternal::WindowRectAbsToRel(window, g.LastItemData.Rect);
	KarmaGuiInternal::NavUpdateAnyRequestFlag();

	// Scroll could be done in NavInitRequestApplyResult() via an opt-in flag (we however don't want regular init requests to scroll)
	if (!IsItemVisible())
		KarmaGuiInternal::ScrollToRectEx(window, g.LastItemData.Rect, KGGuiScrollFlags_None);
}

void Karma::KarmaGui::SetStateStorage(KarmaGuiStorage* tree)
{
	KGGuiWindow* window = GKarmaGui->CurrentWindow;
	window->DC.StateStorage = tree ? tree : &window->StateStorage;
}

KarmaGuiStorage* Karma::KarmaGui::GetStateStorage()
{
	KGGuiWindow* window = GKarmaGui->CurrentWindow;
	return window->DC.StateStorage;
}

void Karma::KarmaGui::PushID(const char* str_id)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	KGGuiID id = window->GetID(str_id);
	window->IDStack.push_back(id);
}

void Karma::KarmaGui::PushID(const char* str_id_begin, const char* str_id_end)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	KGGuiID id = window->GetID(str_id_begin, str_id_end);
	window->IDStack.push_back(id);
}

void Karma::KarmaGui::PushID(const void* ptr_id)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	KGGuiID id = window->GetID(ptr_id);
	window->IDStack.push_back(id);
}

void Karma::KarmaGui::PushID(int int_id)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	KGGuiID id = window->GetID(int_id);
	window->IDStack.push_back(id);
}

// Push a given id value ignoring the ID stack as a seed.
void Karma::KarmaGuiInternal::PushOverrideID(KGGuiID id)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	if (g.DebugHookIdInfo == id)
		DebugHookIdInfo(id, KGGuiDataType_ID, NULL, NULL);
	window->IDStack.push_back(id);
}

// Helper to avoid a common series of PushOverrideID -> GetID() -> PopID() call
// (note that when using this pattern, TestEngine's "Stack Tool" will tend to not display the intermediate stack level.
//  for that to work we would need to do PushOverrideID() -> ItemAdd() -> PopID() which would alter widget code a little more)
KGGuiID Karma::KarmaGuiInternal::GetIDWithSeed(const char* str, const char* str_end, KGGuiID seed)
{
	KGGuiID id = KGHashStr(str, str_end ? (str_end - str) : 0, seed);
	KarmaGuiContext& g = *GKarmaGui;
	if (g.DebugHookIdInfo == id)
		DebugHookIdInfo(id, KGGuiDataType_String, str, str_end);
	return id;
}

void Karma::KarmaGui::PopID()
{
	KGGuiWindow* window = GKarmaGui->CurrentWindow;
	KR_CORE_ASSERT(window->IDStack.Size > 1, ""); // Too many PopID(), or could be popping in a wrong/different window?
	window->IDStack.pop_back();
}

KGGuiID Karma::KarmaGui::GetID(const char* str_id)
{
	KGGuiWindow* window = GKarmaGui->CurrentWindow;
	return window->GetID(str_id);
}

KGGuiID Karma::KarmaGui::GetID(const char* str_id_begin, const char* str_id_end)
{
	KGGuiWindow* window = GKarmaGui->CurrentWindow;
	return window->GetID(str_id_begin, str_id_end);
}

KGGuiID Karma::KarmaGui::GetID(const void* ptr_id)
{
	KGGuiWindow* window = GKarmaGui->CurrentWindow;
	return window->GetID(ptr_id);
}

bool Karma::KarmaGui::IsRectVisible(const KGVec2& size)
{
	KGGuiWindow* window = GKarmaGui->CurrentWindow;
	return window->ClipRect.Overlaps(KGRect(window->DC.CursorPos, window->DC.CursorPos + size));
}

bool Karma::KarmaGui::IsRectVisible(const KGVec2& rect_min, const KGVec2& rect_max)
{
	KGGuiWindow* window = GKarmaGui->CurrentWindow;
	return window->ClipRect.Overlaps(KGRect(rect_min, rect_max));
}

//-----------------------------------------------------------------------------
// [SECTION] INPUTS
//-----------------------------------------------------------------------------
// - GetKeyData() [Internal]
// - GetKeyIndex() [Internal]
// - GetKeyName()
// - GetKeyChordName() [Internal]
// - CalcTypematicRepeatAmount() [Internal]
// - GetTypematicRepeatRate() [Internal]
// - GetKeyPressedAmount() [Internal]
// - GetKeyMagnitude2d() [Internal]
//-----------------------------------------------------------------------------
// - UpdateKeyRoutingTable() [Internal]
// - GetRoutingIdFromOwnerId() [Internal]
// - GetShortcutRoutingData() [Internal]
// - CalcRoutingScore() [Internal]
// - SetShortcutRouting() [Internal]
// - TestShortcutRouting() [Internal]
//-----------------------------------------------------------------------------
// - IsKeyDown()
// - IsKeyPressed()
// - IsKeyReleased()
//-----------------------------------------------------------------------------
// - IsMouseDown()
// - IsMouseClicked()
// - IsMouseReleased()
// - IsMouseDoubleClicked()
// - GetMouseClickedCount()
// - IsMouseHoveringRect() [Internal]
// - IsMouseDragPastThreshold() [Internal]
// - IsMouseDragging()
// - GetMousePos()
// - GetMousePosOnOpeningCurrentPopup()
// - IsMousePosValid()
// - IsAnyMouseDown()
// - GetMouseDragDelta()
// - ResetMouseDragDelta()
// - GetMouseCursor()
// - SetMouseCursor()
//-----------------------------------------------------------------------------
// - UpdateAliasKey()
// - GetMergedModsFromKeys()
// - UpdateKeyboardInputs()
// - UpdateMouseInputs()
//-----------------------------------------------------------------------------
// - LockWheelingWindow [Internal]
// - FindBestWheelingWindow [Internal]
// - UpdateMouseWheel() [Internal]
//-----------------------------------------------------------------------------
// - SetNextFrameWantCaptureKeyboard()
// - SetNextFrameWantCaptureMouse()
//-----------------------------------------------------------------------------
// - GetInputSourceName() [Internal]
// - DebugPrintInputEvent() [Internal]
// - UpdateInputEvents() [Internal]
//-----------------------------------------------------------------------------
// - GetKeyOwner() [Internal]
// - TestKeyOwner() [Internal]
// - SetKeyOwner() [Internal]
// - SetItemKeyOwner() [Internal]
// - Shortcut() [Internal]
//-----------------------------------------------------------------------------

KarmaGuiKeyData* Karma::KarmaGuiInternal::GetKeyData(KarmaGuiKey key)
{
	KarmaGuiContext& g = *GKarmaGui;

	// Special storage location for mods
	if (key & KGGuiMod_Mask_)
		key = ConvertSingleModFlagToKey(key);

	int index;
	/*#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
		KR_CORE_ASSERT(key >= KGGuiKey_LegacyNativeKey_BEGIN && key < KGGuiKey_NamedKey_END);
		if (IsLegacyKey(key))
			index = (g.IO.KeyMap[key] != -1) ? g.IO.KeyMap[key] : key; // Remap native->imgui or imgui->native
		else
			index = key;
	#else*/
	KR_CORE_ASSERT(IsNamedKey(key), "Support for user key indices was dropped in favor of KarmaGuiKey. Please update backend & user code.");
	index = key - KGGuiKey_NamedKey_BEGIN;
	//#endif
	return &g.IO.KeysData[index];
}

/*
#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
KarmaGuiKey KarmaGui::GetKeyIndex(KarmaGuiKey key)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(IsNamedKey(key));
	const KarmaGuiKeyData* key_data = GetKeyData(key);
	return (KarmaGuiKey)(key_data - g.IO.KeysData);
}
#endif*/

// Those names a provided for debugging purpose and are not meant to be saved persistently not compared.
const char* const Karma::KarmaGui::GKeyNames[] =
{
	"Tab", "LeftArrow", "RightArrow", "UpArrow", "DownArrow", "PageUp", "PageDown",
	"Home", "End", "Insert", "Delete", "Backspace", "Space", "Enter", "Escape",
	"LeftCtrl", "LeftShift", "LeftAlt", "LeftSuper", "RightCtrl", "RightShift", "RightAlt", "RightSuper", "Menu",
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "G", "H",
	"I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
	"Apostrophe", "Comma", "Minus", "Period", "Slash", "Semicolon", "Equal", "LeftBracket",
	"Backslash", "RightBracket", "GraveAccent", "CapsLock", "ScrollLock", "NumLock", "PrintScreen",
	"Pause", "Keypad0", "Keypad1", "Keypad2", "Keypad3", "Keypad4", "Keypad5", "Keypad6",
	"Keypad7", "Keypad8", "Keypad9", "KeypadDecimal", "KeypadDivide", "KeypadMultiply",
	"KeypadSubtract", "KeypadAdd", "KeypadEnter", "KeypadEqual",
	"GamepadStart", "GamepadBack",
	"GamepadFaceLeft", "GamepadFaceRight", "GamepadFaceUp", "GamepadFaceDown",
	"GamepadDpadLeft", "GamepadDpadRight", "GamepadDpadUp", "GamepadDpadDown",
	"GamepadL1", "GamepadR1", "GamepadL2", "GamepadR2", "GamepadL3", "GamepadR3",
	"GamepadLStickLeft", "GamepadLStickRight", "GamepadLStickUp", "GamepadLStickDown",
	"GamepadRStickLeft", "GamepadRStickRight", "GamepadRStickUp", "GamepadRStickDown",
	"MouseLeft", "MouseRight", "MouseMiddle", "MouseX1", "MouseX2", "MouseWheelX", "MouseWheelY",
	"ModCtrl", "ModShift", "ModAlt", "ModSuper", // ReservedForModXXX are showing the ModXXX names.
};
//KR_CORE_ASSERT(KarmaGuiKey::KGGuiKey_NamedKey_COUNT == KG_ARRAYSIZE(Karma::KarmaGui::GKeyNames), "size mismatch");

const char* Karma::KarmaGui::GetKeyName(KarmaGuiKey key)
{
	KR_CORE_ASSERT(KarmaGuiKey::KGGuiKey_NamedKey_COUNT == KG_ARRAYSIZE(Karma::KarmaGui::GKeyNames), "size mismatch");
	//#ifdef IMGUI_DISABLE_OBSOLETE_KEYIO
	KR_CORE_ASSERT((KarmaGuiInternal::IsNamedKey(key) || key == KGGuiKey_None), "Support for user key indices was dropped in favor of KarmaGuiKey. Please update backend and user code.");
	/*#else
		if (IsLegacyKey(key))
		{
			KarmaGuiIO& io = GetIO();
			if (io.KeyMap[key] == -1)
				return "N/A";
			KR_CORE_ASSERT(IsNamedKey((KarmaGuiKey)io.KeyMap[key]));
			key = (KarmaGuiKey)io.KeyMap[key];
		}
	#endif*/
	if (key == KGGuiKey_None)
		return "None";
	if (key & KGGuiMod_Mask_)
		key = Karma::KarmaGuiInternal::ConvertSingleModFlagToKey(key);
	if (!KarmaGuiInternal::IsNamedKey(key))
		return "Unknown";

	return GKeyNames[key - KGGuiKey_NamedKey_BEGIN];
}

// KGGuiMod_Shortcut is translated to either Ctrl or Super.
void Karma::KarmaGuiInternal::GetKeyChordName(KarmaGuiKeyChord key_chord, char* out_buf, int out_buf_size)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (key_chord & KGGuiMod_Shortcut)
		key_chord = KarmaGuiInternal::ConvertShortcutMod(key_chord);
	KGFormatString(out_buf, (size_t)out_buf_size, "%s%s%s%s%s",
		(key_chord & KGGuiMod_Ctrl) ? "Ctrl+" : "",
		(key_chord & KGGuiMod_Shift) ? "Shift+" : "",
		(key_chord & KGGuiMod_Alt) ? "Alt+" : "",
		(key_chord & KGGuiMod_Super) ? (g.IO.ConfigMacOSXBehaviors ? "Cmd+" : "Super+") : "",
		KarmaGui::GetKeyName((KarmaGuiKey)(key_chord & ~KGGuiMod_Mask_)));
}

// t0 = previous time (e.g.: g.Time - g.IO.DeltaTime)
// t1 = current time (e.g.: g.Time)
// An event is triggered at:
//  t = 0.0f     t = repeat_delay,    t = repeat_delay + repeat_rate*N
int Karma::KarmaGuiInternal::CalcTypematicRepeatAmount(float t0, float t1, float repeat_delay, float repeat_rate)
{
	if (t1 == 0.0f)
		return 1;
	if (t0 >= t1)
		return 0;
	if (repeat_rate <= 0.0f)
		return (t0 < repeat_delay) && (t1 >= repeat_delay);
	const int count_t0 = (t0 < repeat_delay) ? -1 : (int)((t0 - repeat_delay) / repeat_rate);
	const int count_t1 = (t1 < repeat_delay) ? -1 : (int)((t1 - repeat_delay) / repeat_rate);
	const int count = count_t1 - count_t0;
	return count;
}

void Karma::KarmaGuiInternal::GetTypematicRepeatRate(KarmaGuiInputFlags flags, float* repeat_delay, float* repeat_rate)
{
	KarmaGuiContext& g = *GKarmaGui;
	switch (flags & KGGuiInputFlags_RepeatRateMask_)
	{
	case KGGuiInputFlags_RepeatRateNavMove:             *repeat_delay = g.IO.KeyRepeatDelay * 0.72f; *repeat_rate = g.IO.KeyRepeatRate * 0.80f; return;
	case KGGuiInputFlags_RepeatRateNavTweak:            *repeat_delay = g.IO.KeyRepeatDelay * 0.72f; *repeat_rate = g.IO.KeyRepeatRate * 0.30f; return;
	case KGGuiInputFlags_RepeatRateDefault: default:    *repeat_delay = g.IO.KeyRepeatDelay * 1.00f; *repeat_rate = g.IO.KeyRepeatRate * 1.00f; return;
	}
}

// Return value representing the number of presses in the last time period, for the given repeat rate
// (most often returns 0 or 1. The result is generally only >1 when RepeatRate is smaller than DeltaTime, aka large DeltaTime or fast RepeatRate)
int Karma::KarmaGui::GetKeyPressedAmount(KarmaGuiKey key, float repeat_delay, float repeat_rate)
{
	KarmaGuiContext& g = *GKarmaGui;
	const KarmaGuiKeyData* key_data = KarmaGuiInternal::GetKeyData(key);
	if (!key_data->Down) // In theory this should already be encoded as (DownDuration < 0.0f), but testing this facilitates eating mechanism (until we finish work on key ownership)
		return 0;
	const float t = key_data->DownDuration;
	return KarmaGuiInternal::CalcTypematicRepeatAmount(t - g.IO.DeltaTime, t, repeat_delay, repeat_rate);
}

// Return 2D vector representing the combination of four cardinal direction, with analog value support (for e.g. KGGuiKey_GamepadLStick* values).
KGVec2 Karma::KarmaGuiInternal::GetKeyMagnitude2d(KarmaGuiKey key_left, KarmaGuiKey key_right, KarmaGuiKey key_up, KarmaGuiKey key_down)
{
	return KGVec2(
		KarmaGuiInternal::GetKeyData(key_right)->AnalogValue - KarmaGuiInternal::GetKeyData(key_left)->AnalogValue,
		KarmaGuiInternal::GetKeyData(key_down)->AnalogValue - KarmaGuiInternal::GetKeyData(key_up)->AnalogValue);
}

// Rewrite routing data buffers to strip old entries + sort by key to make queries not touch scattered data.
//   Entries   D,A,B,B,A,C,B     --> A,A,B,B,B,C,D
//   Index     A:1 B:2 C:5 D:0   --> A:0 B:2 C:5 D:6
// See 'Metrics->Key Owners & Shortcut Routing' to visualize the result of that operation.
void Karma::KarmaGuiInternal::UpdateKeyRoutingTable(KGGuiKeyRoutingTable* rt)
{
	KarmaGuiContext& g = *GKarmaGui;
	rt->EntriesNext.resize(0);
	for (KarmaGuiKey key = KGGuiKey_NamedKey_BEGIN; key < KGGuiKey_NamedKey_END; key = (KarmaGuiKey)(key + 1))
	{
		const int new_routing_start_idx = rt->EntriesNext.Size;
		KGGuiKeyRoutingData* routing_entry;
		for (int old_routing_idx = rt->Index[key - KGGuiKey_NamedKey_BEGIN]; old_routing_idx != -1; old_routing_idx = routing_entry->NextEntryIndex)
		{
			routing_entry = &rt->Entries[old_routing_idx];
			routing_entry->RoutingCurr = routing_entry->RoutingNext; // Update entry
			routing_entry->RoutingNext = KGGuiKeyOwner_None;
			routing_entry->RoutingNextScore = 255;
			if (routing_entry->RoutingCurr == KGGuiKeyOwner_None)
				continue;
			rt->EntriesNext.push_back(*routing_entry); // Write alive ones into new buffer

			// Apply routing to owner if there's no owner already (RoutingCurr == None at this point)
			if (routing_entry->Mods == g.IO.KeyMods)
			{
				KGGuiKeyOwnerData* owner_data = KarmaGuiInternal::GetKeyOwnerData(key);
				if (owner_data->OwnerCurr == KGGuiKeyOwner_None)
					owner_data->OwnerCurr = routing_entry->RoutingCurr;
			}
		}

		// Rewrite linked-list
		rt->Index[key - KGGuiKey_NamedKey_BEGIN] = (KGGuiKeyRoutingIndex)(new_routing_start_idx < rt->EntriesNext.Size ? new_routing_start_idx : -1);
		for (int n = new_routing_start_idx; n < rt->EntriesNext.Size; n++)
			rt->EntriesNext[n].NextEntryIndex = (KGGuiKeyRoutingIndex)((n + 1 < rt->EntriesNext.Size) ? n + 1 : -1);
	}
	rt->Entries.swap(rt->EntriesNext); // Swap new and old indexes
}

// owner_id may be None/Any, but routing_id needs to be always be set, so we default to GetCurrentFocusScope().
static inline KGGuiID GetRoutingIdFromOwnerId(KGGuiID owner_id)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	return (owner_id != KGGuiKeyOwner_None && owner_id != KGGuiKeyOwner_Any) ? owner_id : g.CurrentFocusScopeId;
}

KGGuiKeyRoutingData* Karma::KarmaGuiInternal::GetShortcutRoutingData(KarmaGuiKeyChord key_chord)
{
	// Majority of shortcuts will be Key + any number of Mods
	// We accept _Single_ mod with KGGuiKey_None.
	//  - Shortcut(KGGuiKey_S | KGGuiMod_Ctrl);                    // Legal
	//  - Shortcut(KGGuiKey_S | KGGuiMod_Ctrl | KGGuiMod_Shift);   // Legal
	//  - Shortcut(KGGuiMod_Ctrl);                                 // Legal
	//  - Shortcut(KGGuiMod_Ctrl | KGGuiMod_Shift);                // Not legal
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiKeyRoutingTable* rt = &g.KeysRoutingTable;
	KGGuiKeyRoutingData* routing_data;
	if (key_chord & KGGuiMod_Shortcut)
		key_chord = ConvertShortcutMod(key_chord);
	KarmaGuiKey key = (KarmaGuiKey)(key_chord & ~KGGuiMod_Mask_);
	KarmaGuiKey mods = (KarmaGuiKey)(key_chord & KGGuiMod_Mask_);
	if (key == KGGuiKey_None)
		key = ConvertSingleModFlagToKey(mods);
	KR_CORE_ASSERT(IsNamedKey(key), "");

	// Get (in the majority of case, the linked list will have one element so this should be 2 reads.
	// Subsequent elements will be contiguous in memory as list is sorted/rebuilt in NewFrame).
	for (KGGuiKeyRoutingIndex idx = rt->Index[key - KGGuiKey_NamedKey_BEGIN]; idx != -1; idx = routing_data->NextEntryIndex)
	{
		routing_data = &rt->Entries[idx];
		if (routing_data->Mods == mods)
			return routing_data;
	}

	// Add to linked-list
	KGGuiKeyRoutingIndex routing_data_idx = (KGGuiKeyRoutingIndex)rt->Entries.Size;
	rt->Entries.push_back(KGGuiKeyRoutingData());
	routing_data = &rt->Entries[routing_data_idx];
	routing_data->Mods = (KGU16)mods;
	routing_data->NextEntryIndex = rt->Index[key - KGGuiKey_NamedKey_BEGIN]; // Setup linked list
	rt->Index[key - KGGuiKey_NamedKey_BEGIN] = routing_data_idx;
	return routing_data;
}

// Current score encoding (lower is highest priority):
//  -   0: KGGuiInputFlags_RouteGlobalHigh
//  -   1: KGGuiInputFlags_RouteFocused (if item active)
//  -   2: KGGuiInputFlags_RouteGlobal
//  -  3+: KGGuiInputFlags_RouteFocused (if window in focus-stack)
//  - 254: KGGuiInputFlags_RouteGlobalLow
//  - 255: never route
// 'flags' should include an explicit routing policy
static int CalcRoutingScore(KGGuiWindow* location, KGGuiID owner_id, KarmaGuiInputFlags flags)
{
	if (flags & KGGuiInputFlags_RouteFocused)
	{
		KarmaGuiContext& g = *Karma::GKarmaGui;
		KGGuiWindow* focused = g.NavWindow;

		// ActiveID gets top priority
		// (we don't check g.ActiveIdUsingAllKeys here. Routing is applied but if input ownership is tested later it may discard it)
		if (owner_id != 0 && g.ActiveId == owner_id)
			return 1;

		// Score based on distance to focused window (lower is better)
		// Assuming both windows are submitting a routing request,
		// - When Window....... is focused -> Window scores 3 (best), Window/ChildB scores 255 (no match)
		// - When Window/ChildB is focused -> Window scores 4,        Window/ChildB scores 3 (best)
		// Assuming only WindowA is submitting a routing request,
		// - When Window/ChildB is focused -> Window scores 4 (best), Window/ChildB doesn't have a score.
		if (focused != NULL && focused->RootWindow == location->RootWindow)
			for (int next_score = 3; focused != NULL; next_score++)
			{
				if (focused == location)
				{
					KR_CORE_ASSERT(next_score < 255, "");
					return next_score;
				}
				focused = (focused->RootWindow != focused) ? focused->ParentWindow : NULL; // FIXME: This could be later abstracted as a focus path
			}
		return 255;
	}

	// KGGuiInputFlags_RouteGlobalHigh is default, so calls without flags are not conditional
	if (flags & KGGuiInputFlags_RouteGlobal)
		return 2;
	if (flags & KGGuiInputFlags_RouteGlobalLow)
		return 254;
	return 0;
}

// Request a desired route for an input chord (key + mods).
// Return true if the route is available this frame.
// - Routes and key ownership are attributed at the beginning of next frame based on best score and mod state.
//   (Conceptually this does a "Submit for next frame" + "Test for current frame".
//   As such, it could be called TrySetXXX or SubmitXXX, or the Submit and Test operations should be separate.)
// - Using 'owner_id == KGGuiKeyOwner_Any/0': auto-assign an owner based on current focus scope (each window has its focus scope by default)
// - Using 'owner_id == KGGuiKeyOwner_None': allows disabling/locking a shortcut.
bool Karma::KarmaGuiInternal::SetShortcutRouting(KarmaGuiKeyChord key_chord, KGGuiID owner_id, KarmaGuiInputFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	if ((flags & KGGuiInputFlags_RouteMask_) == 0)
		flags |= KGGuiInputFlags_RouteGlobalHigh; // IMPORTANT: This is the default for SetShortcutRouting() but NOT Shortcut()
	else
		KR_CORE_ASSERT(KGIsPowerOfTwo(flags & KGGuiInputFlags_RouteMask_), ""); // Check that only 1 routing flag is used

	if (flags & KGGuiInputFlags_RouteUnlessBgFocused)
		if (g.NavWindow == NULL)
			return false;
	if (flags & KGGuiInputFlags_RouteAlways)
		return true;

	const int score = CalcRoutingScore(g.CurrentWindow, owner_id, flags);
	if (score == 255)
		return false;

	// Submit routing for NEXT frame (assuming score is sufficient)
	// FIXME: Could expose a way to use a "serve last" policy for same score resolution (using <= instead of <).
	KGGuiKeyRoutingData* routing_data = GetShortcutRoutingData(key_chord);
	const KGGuiID routing_id = GetRoutingIdFromOwnerId(owner_id);
	//const bool set_route = (flags & KGGuiInputFlags_ServeLast) ? (score <= routing_data->RoutingNextScore) : (score < routing_data->RoutingNextScore);
	if (score < routing_data->RoutingNextScore)
	{
		routing_data->RoutingNext = routing_id;
		routing_data->RoutingNextScore = (KGU8)score;
	}

	// Return routing state for CURRENT frame
	return routing_data->RoutingCurr == routing_id;
}

// Currently unused by core (but used by tests)
// Note: this cannot be turned into GetShortcutRouting() because we do the owner_id->routing_id translation, name would be more misleading.
bool Karma::KarmaGuiInternal::TestShortcutRouting(KarmaGuiKeyChord key_chord, KGGuiID owner_id)
{
	const KGGuiID routing_id = GetRoutingIdFromOwnerId(owner_id);
	KGGuiKeyRoutingData* routing_data = GetShortcutRoutingData(key_chord); // FIXME: Could avoid creating entry.
	return routing_data->RoutingCurr == routing_id;
}

// Note that Dear ImGui doesn't know the meaning/semantic of KarmaGuiKey from 0..511: they are legacy native keycodes.
// Consider transitioning from 'IsKeyDown(MY_ENGINE_KEY_A)' (<1.87) to IsKeyDown(KGGuiKey_A) (>= 1.87)
bool Karma::KarmaGui::IsKeyDown(KarmaGuiKey key)
{
	return KarmaGuiInternal::IsKeyDown(key, KGGuiKeyOwner_Any);
}

bool Karma::KarmaGuiInternal::IsKeyDown(KarmaGuiKey key, KGGuiID owner_id)
{
	const KarmaGuiKeyData* key_data = GetKeyData(key);
	if (!key_data->Down)
		return false;
	if (!TestKeyOwner(key, owner_id))
		return false;
	return true;
}

bool Karma::KarmaGui::IsKeyPressed(KarmaGuiKey key, bool repeat)
{
	return KarmaGuiInternal::IsKeyPressed(key, KGGuiKeyOwner_Any, repeat ? KGGuiInputFlags_Repeat : KGGuiInputFlags_None);
}

// Important: unless legacy IsKeyPressed(KarmaGuiKey, bool repeat=true) which DEFAULT to repeat, this requires EXPLICIT repeat.
bool Karma::KarmaGuiInternal::IsKeyPressed(KarmaGuiKey key, KGGuiID owner_id, KarmaGuiInputFlags flags)
{
	const KarmaGuiKeyData* key_data = GetKeyData(key);
	if (!key_data->Down) // In theory this should already be encoded as (DownDuration < 0.0f), but testing this facilitates eating mechanism (until we finish work on key ownership)
		return false;
	const float t = key_data->DownDuration;
	if (t < 0.0f)
		return false;
	KR_CORE_ASSERT((flags & ~KGGuiInputFlags_SupportedByIsKeyPressed) == 0, ""); // Passing flags not supported by this function!

	bool pressed = (t == 0.0f);
	if (!pressed && ((flags & KGGuiInputFlags_Repeat) != 0))
	{
		float repeat_delay, repeat_rate;
		GetTypematicRepeatRate(flags, &repeat_delay, &repeat_rate);
		pressed = (t > repeat_delay) && KarmaGui::GetKeyPressedAmount(key, repeat_delay, repeat_rate) > 0;
	}
	if (!pressed)
		return false;
	if (!TestKeyOwner(key, owner_id))
		return false;
	return true;
}

bool Karma::KarmaGui::IsKeyReleased(KarmaGuiKey key)
{
	return KarmaGuiInternal::IsKeyReleased(key, KGGuiKeyOwner_Any);
}

bool Karma::KarmaGuiInternal::IsKeyReleased(KarmaGuiKey key, KGGuiID owner_id)
{
	const KarmaGuiKeyData* key_data = GetKeyData(key);
	if (key_data->DownDurationPrev < 0.0f || key_data->Down)
		return false;
	if (!TestKeyOwner(key, owner_id))
		return false;
	return true;
}

bool Karma::KarmaGui::IsMouseDown(KarmaGuiMouseButton button)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(button >= 0 && button < KG_ARRAYSIZE(g.IO.MouseDown), "");
	return g.IO.MouseDown[button] && KarmaGuiInternal::TestKeyOwner(KarmaGuiInternal::MouseButtonToKey(button), KGGuiKeyOwner_Any); // should be same as IsKeyDown(MouseButtonToKey(button), KGGuiKeyOwner_Any), but this allows legacy code hijacking the io.Mousedown[] array.
}

bool Karma::KarmaGuiInternal::IsMouseDown(KarmaGuiMouseButton button, KGGuiID owner_id)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(button >= 0 && button < KG_ARRAYSIZE(g.IO.MouseDown), "");
	return g.IO.MouseDown[button] && TestKeyOwner(MouseButtonToKey(button), owner_id); // Should be same as IsKeyDown(MouseButtonToKey(button), owner_id), but this allows legacy code hijacking the io.Mousedown[] array.
}

bool Karma::KarmaGui::IsMouseClicked(KarmaGuiMouseButton button, bool repeat)
{
	return KarmaGuiInternal::IsMouseClicked(button, KGGuiKeyOwner_Any, repeat ? KGGuiInputFlags_Repeat : KGGuiInputFlags_None);
}

bool Karma::KarmaGuiInternal::IsMouseClicked(KarmaGuiMouseButton button, KGGuiID owner_id, KarmaGuiInputFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(button >= 0 && button < KG_ARRAYSIZE(g.IO.MouseDown), "");
	if (!g.IO.MouseDown[button]) // In theory this should already be encoded as (DownDuration < 0.0f), but testing this facilitates eating mechanism (until we finish work on key ownership)
		return false;
	const float t = g.IO.MouseDownDuration[button];
	if (t < 0.0f)
		return false;
	KR_CORE_ASSERT((flags & ~KGGuiInputFlags_SupportedByIsKeyPressed) == 0, ""); // Passing flags not supported by this function!

	const bool repeat = (flags & KGGuiInputFlags_Repeat) != 0;
	const bool pressed = (t == 0.0f) || (repeat && t > g.IO.KeyRepeatDelay && CalcTypematicRepeatAmount(t - g.IO.DeltaTime, t, g.IO.KeyRepeatDelay, g.IO.KeyRepeatRate) > 0);
	if (!pressed)
		return false;

	if (!TestKeyOwner(MouseButtonToKey(button), owner_id))
		return false;

	return true;
}

bool Karma::KarmaGui::IsMouseReleased(KarmaGuiMouseButton button)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(button >= 0 && button < KG_ARRAYSIZE(g.IO.MouseDown), "");
	return g.IO.MouseReleased[button] && KarmaGuiInternal::TestKeyOwner(KarmaGuiInternal::MouseButtonToKey(button), KGGuiKeyOwner_Any); // Should be same as IsKeyReleased(MouseButtonToKey(button), KGGuiKeyOwner_Any)
}

bool Karma::KarmaGuiInternal::IsMouseReleased(KarmaGuiMouseButton button, KGGuiID owner_id)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(button >= 0 && button < KG_ARRAYSIZE(g.IO.MouseDown), "");
	return g.IO.MouseReleased[button] && TestKeyOwner(MouseButtonToKey(button), owner_id); // Should be same as IsKeyReleased(MouseButtonToKey(button), owner_id)
}

bool Karma::KarmaGui::IsMouseDoubleClicked(KarmaGuiMouseButton button)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(button >= 0 && button < KG_ARRAYSIZE(g.IO.MouseDown), "");
	return g.IO.MouseClickedCount[button] == 2 && KarmaGuiInternal::TestKeyOwner(KarmaGuiInternal::MouseButtonToKey(button), KGGuiKeyOwner_Any);
}

int Karma::KarmaGui::GetMouseClickedCount(KarmaGuiMouseButton button)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(button >= 0 && button < KG_ARRAYSIZE(g.IO.MouseDown), "");
	return g.IO.MouseClickedCount[button];
}

// Test if mouse cursor is hovering given rectangle
// NB- Rectangle is clipped by our current clip setting
// NB- Expand the rectangle to be generous on imprecise inputs systems (g.Style.TouchExtraPadding)
bool Karma::KarmaGui::IsMouseHoveringRect(const KGVec2& r_min, const KGVec2& r_max, bool clip)
{
	KarmaGuiContext& g = *GKarmaGui;

	// Clip
	KGRect rect_clipped(r_min, r_max);
	if (clip)
		rect_clipped.ClipWith(g.CurrentWindow->ClipRect);

	// Expand for touch input
	const KGRect rect_for_touch(rect_clipped.Min - g.Style.TouchExtraPadding, rect_clipped.Max + g.Style.TouchExtraPadding);
	if (!rect_for_touch.Contains(g.IO.MousePos))
		return false;
	if (!g.MouseViewport->GetMainRect().Overlaps(rect_clipped))
		return false;
	return true;
}

// Return if a mouse click/drag went past the given threshold. Valid to call during the MouseReleased frame.
// [Internal] This doesn't test if the button is pressed
bool Karma::KarmaGuiInternal::IsMouseDragPastThreshold(KarmaGuiMouseButton button, float lock_threshold)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(button >= 0 && button < KG_ARRAYSIZE(g.IO.MouseDown), "");
	if (lock_threshold < 0.0f)
		lock_threshold = g.IO.MouseDragThreshold;
	return g.IO.MouseDragMaxDistanceSqr[button] >= lock_threshold * lock_threshold;
}

bool Karma::KarmaGui::IsMouseDragging(KarmaGuiMouseButton button, float lock_threshold)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(button >= 0 && button < KG_ARRAYSIZE(g.IO.MouseDown), "");
	if (!g.IO.MouseDown[button])
		return false;
	return KarmaGuiInternal::IsMouseDragPastThreshold(button, lock_threshold);
}

KGVec2 Karma::KarmaGui::GetMousePos()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.IO.MousePos;
}

// NB: prefer to call right after BeginPopup(). At the time Selectable/MenuItem is activated, the popup is already closed!
KGVec2 Karma::KarmaGui::GetMousePosOnOpeningCurrentPopup()
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.BeginPopupStack.Size > 0)
		return g.OpenPopupStack[g.BeginPopupStack.Size - 1].OpenMousePos;
	return g.IO.MousePos;
}

// We typically use KGVec2(-FLT_MAX,-FLT_MAX) to denote an invalid mouse position.
bool Karma::KarmaGui::IsMousePosValid(const KGVec2* mouse_pos)
{
	// The assert is only to silence a false-positive in XCode Static Analysis.
	// Because GKarmaGui is not dereferenced in every code path, the static analyzer assume that it may be NULL (which it doesn't for other functions).
	KR_CORE_ASSERT(GKarmaGui != NULL, "");
	const float MOUSE_INVALID = -256000.0f;
	KGVec2 p = mouse_pos ? *mouse_pos : GKarmaGui->IO.MousePos;
	return p.x >= MOUSE_INVALID && p.y >= MOUSE_INVALID;
}

// [WILL OBSOLETE] This was designed for backends, but prefer having backend maintain a mask of held mouse buttons, because upcoming input queue system will make this invalid.
bool Karma::KarmaGui::IsAnyMouseDown()
{
	KarmaGuiContext& g = *GKarmaGui;
	for (int n = 0; n < KG_ARRAYSIZE(g.IO.MouseDown); n++)
		if (g.IO.MouseDown[n])
			return true;
	return false;
}

// Return the delta from the initial clicking position while the mouse button is clicked or was just released.
// This is locked and return 0.0f until the mouse moves past a distance threshold at least once.
// NB: This is only valid if IsMousePosValid(). backends in theory should always keep mouse position valid when dragging even outside the client window.
KGVec2 Karma::KarmaGui::GetMouseDragDelta(KarmaGuiMouseButton button, float lock_threshold)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(button >= 0 && button < KG_ARRAYSIZE(g.IO.MouseDown), "");
	if (lock_threshold < 0.0f)
		lock_threshold = g.IO.MouseDragThreshold;
	if (g.IO.MouseDown[button] || g.IO.MouseReleased[button])
		if (g.IO.MouseDragMaxDistanceSqr[button] >= lock_threshold * lock_threshold)
			if (IsMousePosValid(&g.IO.MousePos) && IsMousePosValid(&g.IO.MouseClickedPos[button]))
				return g.IO.MousePos - g.IO.MouseClickedPos[button];
	return KGVec2(0.0f, 0.0f);
}

void Karma::KarmaGui::ResetMouseDragDelta(KarmaGuiMouseButton button)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(button >= 0 && button < KG_ARRAYSIZE(g.IO.MouseDown), "");
	// NB: We don't need to reset g.IO.MouseDragMaxDistanceSqr
	g.IO.MouseClickedPos[button] = g.IO.MousePos;
}

// Get desired mouse cursor shape.
// Important: this is meant to be used by a platform backend, it is reset in KarmaGui::NewFrame(),
// updated during the frame, and locked in EndFrame()/Render().
// If you use software rendering by setting io.MouseDrawCursor then Dear ImGui will render those for you
KarmaGuiMouseCursor Karma::KarmaGui::GetMouseCursor()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.MouseCursor;
}

void Karma::KarmaGui::SetMouseCursor(KarmaGuiMouseCursor cursor_type)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.MouseCursor = cursor_type;
}

static void UpdateAliasKey(KarmaGuiKey key, bool v, float analog_value)
{
	KR_CORE_ASSERT(Karma::KarmaGuiInternal::IsAliasKey(key), "");
	KarmaGuiKeyData* key_data = Karma::KarmaGuiInternal::GetKeyData(key);
	key_data->Down = v;
	key_data->AnalogValue = analog_value;
}

// [Internal] Do not use directly
static KarmaGuiKeyChord GetMergedModsFromKeys()
{
	KarmaGuiKeyChord mods = 0;
	if (Karma::KarmaGui::IsKeyDown(KGGuiMod_Ctrl)) { mods |= KGGuiMod_Ctrl; }
	if (Karma::KarmaGui::IsKeyDown(KGGuiMod_Shift)) { mods |= KGGuiMod_Shift; }
	if (Karma::KarmaGui::IsKeyDown(KGGuiMod_Alt)) { mods |= KGGuiMod_Alt; }
	if (Karma::KarmaGui::IsKeyDown(KGGuiMod_Super)) { mods |= KGGuiMod_Super; }
	return mods;
}

void Karma::KarmaGuiInternal::UpdateKeyboardInputs()
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiIO& io = g.IO;

	// Import legacy keys or verify they are not used
	/*
#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
	if (io.BackendUsingLegacyKeyArrays == 0)
	{
		// Backend used new io.AddKeyEvent() API: Good! Verify that old arrays are never written to externally.
		for (int n = 0; n < KGGuiKey_LegacyNativeKey_END; n++)
			KR_CORE_ASSERT((io.KeysDown[n] == false || KarmaGui::IsKeyDown((KarmaGuiKey)n)), "Backend needs to either only use io.AddKeyEvent(), either only fill legacy io.KeysDown[] + io.KeyMap[]. Not both!");
	}
	else
	{
		if (g.FrameCount == 0)
			for (int n = KGGuiKey_LegacyNativeKey_BEGIN; n < KGGuiKey_LegacyNativeKey_END; n++)
		{
				KR_CORE_ASSERT(g.IO.KeyMap[n] == -1, "Backend is not allowed to write to io.KeyMap[0..511]!");
		}

		// Build reverse KeyMap (Named -> Legacy)
		for (int n = KGGuiKey_NamedKey_BEGIN; n < KGGuiKey_NamedKey_END; n++)
			if (io.KeyMap[n] != -1)
			{
				KR_CORE_ASSERT(IsLegacyKey((KarmaGuiKey)io.KeyMap[n]), "");
				io.KeyMap[io.KeyMap[n]] = n;
			}

		// Import legacy keys into new ones
		for (int n = KGGuiKey_LegacyNativeKey_BEGIN; n < KGGuiKey_LegacyNativeKey_END; n++)
			if (io.KeysDown[n] || io.BackendUsingLegacyKeyArrays == 1)
			{
				const KarmaGuiKey key = (KarmaGuiKey)(io.KeyMap[n] != -1 ? io.KeyMap[n] : n);
				KR_CORE_ASSERT(io.KeyMap[n] == -1 || IsNamedKey(key), "");
				io.KeysData[key].Down = io.KeysDown[n];
				if (key != n)
					io.KeysDown[key] = io.KeysDown[n]; // Allow legacy code using io.KeysDown[GetKeyIndex()] with old backends
				io.BackendUsingLegacyKeyArrays = 1;
			}
		if (io.BackendUsingLegacyKeyArrays == 1)
		{
			GetKeyData(KGGuiMod_Ctrl)->Down = io.KeyCtrl;
			GetKeyData(KGGuiMod_Shift)->Down = io.KeyShift;
			GetKeyData(KGGuiMod_Alt)->Down = io.KeyAlt;
			GetKeyData(KGGuiMod_Super)->Down = io.KeySuper;
		}
	}

#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
	const bool nav_gamepad_active = (io.ConfigFlags & KGGuiConfigFlags_NavEnableGamepad) != 0 && (io.BackendFlags & KGGuiBackendFlags_HasGamepad) != 0;
	if (io.BackendUsingLegacyNavInputArray && nav_gamepad_active)
	{
		#define MAP_LEGACY_NAV_INPUT_TO_KEY1(_KEY, _NAV1)           do { io.KeysData[_KEY].Down = (io.NavInputs[_NAV1] > 0.0f); io.KeysData[_KEY].AnalogValue = io.NavInputs[_NAV1]; } while (0)
		#define MAP_LEGACY_NAV_INPUT_TO_KEY2(_KEY, _NAV1, _NAV2)    do { io.KeysData[_KEY].Down = (io.NavInputs[_NAV1] > 0.0f) || (io.NavInputs[_NAV2] > 0.0f); io.KeysData[_KEY].AnalogValue = KGMax(io.NavInputs[_NAV1], io.NavInputs[_NAV2]); } while (0)
		MAP_LEGACY_NAV_INPUT_TO_KEY1(KGGuiKey_GamepadFaceDown, KGGuiNavInput_Activate);
		MAP_LEGACY_NAV_INPUT_TO_KEY1(KGGuiKey_GamepadFaceRight, KGGuiNavInput_Cancel);
		MAP_LEGACY_NAV_INPUT_TO_KEY1(KGGuiKey_GamepadFaceLeft, KGGuiNavInput_Menu);
		MAP_LEGACY_NAV_INPUT_TO_KEY1(KGGuiKey_GamepadFaceUp, KGGuiNavInput_Input);
		MAP_LEGACY_NAV_INPUT_TO_KEY1(KGGuiKey_GamepadDpadLeft, KGGuiNavInput_DpadLeft);
		MAP_LEGACY_NAV_INPUT_TO_KEY1(KGGuiKey_GamepadDpadRight, KGGuiNavInput_DpadRight);
		MAP_LEGACY_NAV_INPUT_TO_KEY1(KGGuiKey_GamepadDpadUp, KGGuiNavInput_DpadUp);
		MAP_LEGACY_NAV_INPUT_TO_KEY1(KGGuiKey_GamepadDpadDown, KGGuiNavInput_DpadDown);
		MAP_LEGACY_NAV_INPUT_TO_KEY2(KGGuiKey_GamepadL1, KGGuiNavInput_FocusPrev, KGGuiNavInput_TweakSlow);
		MAP_LEGACY_NAV_INPUT_TO_KEY2(KGGuiKey_GamepadR1, KGGuiNavInput_FocusNext, KGGuiNavInput_TweakFast);
		MAP_LEGACY_NAV_INPUT_TO_KEY1(KGGuiKey_GamepadLStickLeft, KGGuiNavInput_LStickLeft);
		MAP_LEGACY_NAV_INPUT_TO_KEY1(KGGuiKey_GamepadLStickRight, KGGuiNavInput_LStickRight);
		MAP_LEGACY_NAV_INPUT_TO_KEY1(KGGuiKey_GamepadLStickUp, KGGuiNavInput_LStickUp);
		MAP_LEGACY_NAV_INPUT_TO_KEY1(KGGuiKey_GamepadLStickDown, KGGuiNavInput_LStickDown);
		#undef NAV_MAP_KEY
	}
#endif
#endif*/

// Update aliases
	for (int n = 0; n < KGGuiMouseButton_COUNT; n++)
		UpdateAliasKey(MouseButtonToKey(n), io.MouseDown[n], io.MouseDown[n] ? 1.0f : 0.0f);
	UpdateAliasKey(KGGuiKey_MouseWheelX, io.MouseWheelH != 0.0f, io.MouseWheelH);
	UpdateAliasKey(KGGuiKey_MouseWheelY, io.MouseWheel != 0.0f, io.MouseWheel);

	// Synchronize io.KeyMods and io.KeyXXX values.
	// - New backends (1.87+): send io.AddKeyEvent(KGGuiMod_XXX) ->                                      -> (here) deriving io.KeyMods + io.KeyXXX from key array.
	// - Legacy backends:      set io.KeyXXX bools               -> (above) set key array from io.KeyXXX -> (here) deriving io.KeyMods + io.KeyXXX from key array.
	// So with legacy backends the 4 values will do a unnecessary back-and-forth but it makes the code simpler and future facing.
	io.KeyMods = GetMergedModsFromKeys();
	io.KeyCtrl = (io.KeyMods & KGGuiMod_Ctrl) != 0;
	io.KeyShift = (io.KeyMods & KGGuiMod_Shift) != 0;
	io.KeyAlt = (io.KeyMods & KGGuiMod_Alt) != 0;
	io.KeySuper = (io.KeyMods & KGGuiMod_Super) != 0;

	// Clear gamepad data if disabled
	if ((io.BackendFlags & KGGuiBackendFlags_HasGamepad) == 0)
		for (int i = KGGuiKey_Gamepad_BEGIN; i < KGGuiKey_Gamepad_END; i++)
		{
			io.KeysData[i - KGGuiKey_KeysData_OFFSET].Down = false;
			io.KeysData[i - KGGuiKey_KeysData_OFFSET].AnalogValue = 0.0f;
		}

	// Update keys
	for (int i = 0; i < KGGuiKey_KeysData_SIZE; i++)
	{
		KarmaGuiKeyData* key_data = &io.KeysData[i];
		key_data->DownDurationPrev = key_data->DownDuration;
		key_data->DownDuration = key_data->Down ? (key_data->DownDuration < 0.0f ? 0.0f : key_data->DownDuration + io.DeltaTime) : -1.0f;
	}

	// Update keys/input owner (named keys only): one entry per key
	for (KarmaGuiKey key = KGGuiKey_NamedKey_BEGIN; key < KGGuiKey_NamedKey_END; key = (KarmaGuiKey)(key + 1))
	{
		KarmaGuiKeyData* key_data = &io.KeysData[key - KGGuiKey_KeysData_OFFSET];
		KGGuiKeyOwnerData* owner_data = &g.KeysOwnerData[key - KGGuiKey_NamedKey_BEGIN];
		owner_data->OwnerCurr = owner_data->OwnerNext;
		if (!key_data->Down) // Important: ownership is released on the frame after a release. Ensure a 'MouseDown -> CloseWindow -> MouseUp' chain doesn't lead to someone else seeing the MouseUp.
			owner_data->OwnerNext = KGGuiKeyOwner_None;
		owner_data->LockThisFrame = owner_data->LockUntilRelease = owner_data->LockUntilRelease && key_data->Down;  // Clear LockUntilRelease when key is not Down anymore
	}

	UpdateKeyRoutingTable(&g.KeysRoutingTable);
}

void Karma::KarmaGuiInternal::UpdateMouseInputs()
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiIO& io = g.IO;

	// Round mouse position to avoid spreading non-rounded position (e.g. UpdateManualResize doesn't support them well)
	if (KarmaGui::IsMousePosValid(&io.MousePos))
		io.MousePos = g.MouseLastValidPos = KGFloorSigned(io.MousePos);

	// If mouse just appeared or disappeared (usually denoted by -FLT_MAX components) we cancel out movement in MouseDelta
	if (KarmaGui::IsMousePosValid(&io.MousePos) && KarmaGui::IsMousePosValid(&io.MousePosPrev))
		io.MouseDelta = io.MousePos - io.MousePosPrev;
	else
		io.MouseDelta = KGVec2(0.0f, 0.0f);

	// If mouse moved we re-enable mouse hovering in case it was disabled by gamepad/keyboard. In theory should use a >0.0f threshold but would need to reset in everywhere we set this to true.
	if (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f)
		g.NavDisableMouseHover = false;

	io.MousePosPrev = io.MousePos;
	for (int i = 0; i < KG_ARRAYSIZE(io.MouseDown); i++)
	{
		io.MouseClicked[i] = io.MouseDown[i] && io.MouseDownDuration[i] < 0.0f;
		io.MouseClickedCount[i] = 0; // Will be filled below
		io.MouseReleased[i] = !io.MouseDown[i] && io.MouseDownDuration[i] >= 0.0f;
		io.MouseDownDurationPrev[i] = io.MouseDownDuration[i];
		io.MouseDownDuration[i] = io.MouseDown[i] ? (io.MouseDownDuration[i] < 0.0f ? 0.0f : io.MouseDownDuration[i] + io.DeltaTime) : -1.0f;
		if (io.MouseClicked[i])
		{
			bool is_repeated_click = false;
			if ((float)(g.Time - io.MouseClickedTime[i]) < io.MouseDoubleClickTime)
			{
				KGVec2 delta_from_click_pos = KarmaGui::IsMousePosValid(&io.MousePos) ? (io.MousePos - io.MouseClickedPos[i]) : KGVec2(0.0f, 0.0f);
				if (KGLengthSqr(delta_from_click_pos) < io.MouseDoubleClickMaxDist * io.MouseDoubleClickMaxDist)
					is_repeated_click = true;
			}
			if (is_repeated_click)
				io.MouseClickedLastCount[i]++;
			else
				io.MouseClickedLastCount[i] = 1;
			io.MouseClickedTime[i] = g.Time;
			io.MouseClickedPos[i] = io.MousePos;
			io.MouseClickedCount[i] = io.MouseClickedLastCount[i];
			io.MouseDragMaxDistanceAbs[i] = KGVec2(0.0f, 0.0f);
			io.MouseDragMaxDistanceSqr[i] = 0.0f;
		}
		else if (io.MouseDown[i])
		{
			// Maintain the maximum distance we reaching from the initial click position, which is used with dragging threshold
			KGVec2 delta_from_click_pos = KarmaGui::IsMousePosValid(&io.MousePos) ? (io.MousePos - io.MouseClickedPos[i]) : KGVec2(0.0f, 0.0f);
			io.MouseDragMaxDistanceSqr[i] = KGMax(io.MouseDragMaxDistanceSqr[i], KGLengthSqr(delta_from_click_pos));
			io.MouseDragMaxDistanceAbs[i].x = KGMax(io.MouseDragMaxDistanceAbs[i].x, delta_from_click_pos.x < 0.0f ? -delta_from_click_pos.x : delta_from_click_pos.x);
			io.MouseDragMaxDistanceAbs[i].y = KGMax(io.MouseDragMaxDistanceAbs[i].y, delta_from_click_pos.y < 0.0f ? -delta_from_click_pos.y : delta_from_click_pos.y);
		}

		// We provide io.MouseDoubleClicked[] as a legacy service
		io.MouseDoubleClicked[i] = (io.MouseClickedCount[i] == 2);

		// Clicking any mouse button reactivate mouse hovering which may have been deactivated by gamepad/keyboard navigation
		if (io.MouseClicked[i])
			g.NavDisableMouseHover = false;
	}
}

static void LockWheelingWindow(KGGuiWindow* window, float wheel_amount)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	if (window)
		g.WheelingWindowReleaseTimer = KGMin(g.WheelingWindowReleaseTimer + KGAbs(wheel_amount) * WINDOWS_MOUSE_WHEEL_SCROLL_LOCK_TIMER, WINDOWS_MOUSE_WHEEL_SCROLL_LOCK_TIMER);
	else
		g.WheelingWindowReleaseTimer = 0.0f;
	if (g.WheelingWindow == window)
		return;
	KR_CORE_INFO("LockWheelingWindow() \"{0}\"", window ? window->Name : "NULL");
	g.WheelingWindow = window;
	g.WheelingWindowRefMousePos = g.IO.MousePos;
	if (window == NULL)
	{
		g.WheelingWindowStartFrame = -1;
		g.WheelingAxisAvg = KGVec2(0.0f, 0.0f);
	}
}

static KGGuiWindow* FindBestWheelingWindow(const KGVec2& wheel)
{
	// For each axis, find window in the hierarchy that may want to use scrolling
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGGuiWindow* windows[2] = { NULL, NULL };
	for (int axis = 0; axis < 2; axis++)
		if (wheel[axis] != 0.0f)
			for (KGGuiWindow* window = windows[axis] = g.HoveredWindow; window->Flags & KGGuiWindowFlags_ChildWindow; window = windows[axis] = window->ParentWindow)
			{
				// Bubble up into parent window if:
				// - a child window doesn't allow any scrolling.
				// - a child window has the KGGuiWindowFlags_NoScrollWithMouse flag.
				//// - a child window doesn't need scrolling because it is already at the edge for the direction we are going in (FIXME-WIP)
				const bool has_scrolling = (window->ScrollMax[axis] != 0.0f);
				const bool inputs_disabled = (window->Flags & KGGuiWindowFlags_NoScrollWithMouse) && !(window->Flags & KGGuiWindowFlags_NoMouseInputs);
				//const bool scrolling_past_limits = (wheel_v < 0.0f) ? (window->Scroll[axis] <= 0.0f) : (window->Scroll[axis] >= window->ScrollMax[axis]);
				if (has_scrolling && !inputs_disabled) // && !scrolling_past_limits)
					break; // select this window
			}
	if (windows[0] == NULL && windows[1] == NULL)
		return NULL;

	// If there's only one window or only one axis then there's no ambiguity
	if (windows[0] == windows[1] || windows[0] == NULL || windows[1] == NULL)
		return windows[1] ? windows[1] : windows[0];

	// If candidate are different windows we need to decide which one to prioritize
	// - First frame: only find a winner if one axis is zero.
	// - Subsequent frames: only find a winner when one is more than the other.
	if (g.WheelingWindowStartFrame == -1)
		g.WheelingWindowStartFrame = g.FrameCount;
	if ((g.WheelingWindowStartFrame == g.FrameCount && wheel.x != 0.0f && wheel.y != 0.0f) || (g.WheelingAxisAvg.x == g.WheelingAxisAvg.y))
	{
		g.WheelingWindowWheelRemainder = wheel;
		return NULL;
	}
	return (g.WheelingAxisAvg.x > g.WheelingAxisAvg.y) ? windows[0] : windows[1];
}

// Called by NewFrame()
void Karma::KarmaGuiInternal::UpdateMouseWheel()
{
	// Reset the locked window if we move the mouse or after the timer elapses.
	// FIXME: Ideally we could refactor to have one timer for "changing window w/ same axis" and a shorter timer for "changing window or axis w/ other axis" (#3795)
	KarmaGuiContext& g = *GKarmaGui;
	if (g.WheelingWindow != NULL)
	{
		g.WheelingWindowReleaseTimer -= g.IO.DeltaTime;
		if (KarmaGui::IsMousePosValid() && KGLengthSqr(g.IO.MousePos - g.WheelingWindowRefMousePos) > g.IO.MouseDragThreshold * g.IO.MouseDragThreshold)
			g.WheelingWindowReleaseTimer = 0.0f;
		if (g.WheelingWindowReleaseTimer <= 0.0f)
			LockWheelingWindow(NULL, 0.0f);
	}

	KGVec2 wheel;
	wheel.x = TestKeyOwner(KGGuiKey_MouseWheelX, KGGuiKeyOwner_None) ? g.IO.MouseWheelH : 0.0f;
	wheel.y = TestKeyOwner(KGGuiKey_MouseWheelY, KGGuiKeyOwner_None) ? g.IO.MouseWheel : 0.0f;

	//KARMAGUI_DEBUG_LOG("MouseWheel X:%.3f Y:%.3f\n", wheel_x, wheel_y);
	KGGuiWindow* mouse_window = g.WheelingWindow ? g.WheelingWindow : g.HoveredWindow;
	if (!mouse_window || mouse_window->Collapsed)
		return;

	// Zoom / Scale window
	// FIXME-OBSOLETE: This is an old feature, it still works but pretty much nobody is using it and may be best redesigned.
	if (wheel.y != 0.0f && g.IO.KeyCtrl && g.IO.FontAllowUserScaling)
	{
		LockWheelingWindow(mouse_window, wheel.y);
		KGGuiWindow* window = mouse_window;
		const float new_font_scale = KGClamp(window->FontWindowScale + g.IO.MouseWheel * 0.10f, 0.50f, 2.50f);
		const float scale = new_font_scale / window->FontWindowScale;
		window->FontWindowScale = new_font_scale;
		if (window == window->RootWindow)
		{
			const KGVec2 offset = window->Size * (1.0f - scale) * (g.IO.MousePos - window->Pos) / window->Size;
			SetWindowPos(window, window->Pos + offset, 0);
			window->Size = KGFloor(window->Size * scale);
			window->SizeFull = KGFloor(window->SizeFull * scale);
		}
		return;
	}
	if (g.IO.KeyCtrl)
		return;

	// Mouse wheel scrolling
	// As a standard behavior holding SHIFT while using Vertical Mouse Wheel triggers Horizontal scroll instead
	// (we avoid doing it on OSX as it the OS input layer handles this already)
	const bool swap_axis = g.IO.KeyShift && !g.IO.ConfigMacOSXBehaviors;
	if (swap_axis)
	{
		wheel.x = wheel.y;
		wheel.y = 0.0f;
	}

	// Maintain a rough average of moving magnitude on both axises
	// FIXME: should by based on wall clock time rather than frame-counter
	g.WheelingAxisAvg.x = KGExponentialMovingAverage(g.WheelingAxisAvg.x, KGAbs(wheel.x), 30);
	g.WheelingAxisAvg.y = KGExponentialMovingAverage(g.WheelingAxisAvg.y, KGAbs(wheel.y), 30);

	// In the rare situation where FindBestWheelingWindow() had to defer first frame of wheeling due to ambiguous main axis, reinject it now.
	wheel += g.WheelingWindowWheelRemainder;
	g.WheelingWindowWheelRemainder = KGVec2(0.0f, 0.0f);
	if (wheel.x == 0.0f && wheel.y == 0.0f)
		return;

	// Mouse wheel scrolling: find target and apply
	// - don't renew lock if axis doesn't apply on the window.
	// - select a main axis when both axises are being moved.
	if (KGGuiWindow* window = (g.WheelingWindow ? g.WheelingWindow : FindBestWheelingWindow(wheel)))
		if (!(window->Flags & KGGuiWindowFlags_NoScrollWithMouse) && !(window->Flags & KGGuiWindowFlags_NoMouseInputs))
		{
			bool do_scroll[2] = { wheel.x != 0.0f && window->ScrollMax.x != 0.0f, wheel.y != 0.0f && window->ScrollMax.y != 0.0f };
			if (do_scroll[KGGuiAxis_X] && do_scroll[KGGuiAxis_Y])
				do_scroll[(g.WheelingAxisAvg.x > g.WheelingAxisAvg.y) ? KGGuiAxis_Y : KGGuiAxis_X] = false;
			if (do_scroll[KGGuiAxis_X])
			{
				LockWheelingWindow(window, wheel.x);
				float max_step = window->InnerRect.GetWidth() * 0.67f;
				float scroll_step = KGFloor(KGMin(2 * window->CalcFontSize(), max_step));
				SetScrollX(window, window->Scroll.x - wheel.x * scroll_step);
			}
			if (do_scroll[KGGuiAxis_Y])
			{
				LockWheelingWindow(window, wheel.y);
				float max_step = window->InnerRect.GetHeight() * 0.67f;
				float scroll_step = KGFloor(KGMin(5 * window->CalcFontSize(), max_step));
				SetScrollY(window, window->Scroll.y - wheel.y * scroll_step);
			}
		}
}

void Karma::KarmaGui::SetNextFrameWantCaptureKeyboard(bool want_capture_keyboard)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.WantCaptureKeyboardNextFrame = want_capture_keyboard ? 1 : 0;
}

void Karma::KarmaGui::SetNextFrameWantCaptureMouse(bool want_capture_mouse)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.WantCaptureMouseNextFrame = want_capture_mouse ? 1 : 0;
}

#ifndef KARMAGUI_DISABLE_DEBUG_TOOLS
static const char* GetInputSourceName(KGGuiInputSource source)
{
	const char* input_source_names[] = { "None", "Mouse", "Keyboard", "Gamepad", "Nav", "Clipboard" };
	KR_CORE_ASSERT(KG_ARRAYSIZE(input_source_names) == KGGuiInputSource_COUNT && source >= 0 && source < KGGuiInputSource_COUNT, "");
	return input_source_names[source];
}
static void DebugPrintInputEvent(const char* prefix, const KGGuiInputEvent* e)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	if (e->Type == KGGuiInputEventType_MousePos)
	{
		if (e->MousePos.PosX == -FLT_MAX && e->MousePos.PosY == -FLT_MAX)
		{
			KR_CORE_INFO("{0}: MousePos (-FLT_MAX, -FLT_MAX)\n", prefix);
		}
		else
		{
			KR_CORE_INFO("{0}: MousePos ({1}, {2})", prefix, e->MousePos.PosX, e->MousePos.PosY);
		}
		return;
	}
	if (e->Type == KGGuiInputEventType_MouseButton) { KR_CORE_INFO("{0}: MouseButton {1} {2}", prefix, e->MouseButton.Button, e->MouseButton.Down ? "Down" : "Up"); return; }
	if (e->Type == KGGuiInputEventType_MouseWheel) { KR_CORE_INFO("{0}: MouseWheel (%.3f, %.3f)", prefix, e->MouseWheel.WheelX, e->MouseWheel.WheelY); return; }
	if (e->Type == KGGuiInputEventType_MouseViewport) { KR_CORE_INFO("{0}: MouseViewport (0x%08X)", prefix, e->MouseViewport.HoveredViewportID); return; }
	if (e->Type == KGGuiInputEventType_Key) { KR_CORE_INFO("{0}: Key \"{1}\" {2}", prefix, Karma::KarmaGui::GetKeyName(e->Key.Key), e->Key.Down ? "Down" : "Up"); return; }
	if (e->Type == KGGuiInputEventType_Text) { KR_CORE_INFO("{0}: Text: %c (U+%08X)", prefix, e->Text.Char, e->Text.Char); return; }
	if (e->Type == KGGuiInputEventType_Focus) { KR_CORE_INFO("{0}: AppFocused %d", prefix, e->AppFocused.Focused); return; }
}
#endif

bool Karma::KarmaGuiInternal::IsActiveIdUsingNavDir(KarmaGuiDir dir)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	return (g.ActiveIdUsingNavDirMask & (1 << dir)) != 0;
}

// Process input queue
// We always call this with the value of 'bool g.IO.ConfigInputTrickleEventQueue'.
// - trickle_fast_inputs = false : process all events, turn into flattened input state (e.g. successive down/up/down/up will be lost)
// - trickle_fast_inputs = true  : process as many events as possible (successive down/up/down/up will be trickled over several frames so nothing is lost) (new feature in 1.87)
void Karma::KarmaGuiInternal::UpdateInputEvents(bool trickle_fast_inputs)
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiIO& io = g.IO;

	// Only trickle chars<>key when working with InputText()
	// FIXME: InputText() could parse event trail?
	// FIXME: Could specialize chars<>keys trickling rules for control keys (those not typically associated to characters)
	const bool trickle_interleaved_keys_and_text = (trickle_fast_inputs && g.WantTextInputNextFrame == 1);

	bool mouse_moved = false, mouse_wheeled = false, key_changed = false, text_inputted = false;
	int  mouse_button_changed = 0x00;
	KGBitArray<KGGuiKey_KeysData_SIZE> key_changed_mask;

	int event_n = 0;
	for (; event_n < g.InputEventsQueue.Size; event_n++)
	{
		KGGuiInputEvent* e = &g.InputEventsQueue[event_n];
		if (e->Type == KGGuiInputEventType_MousePos)
		{
			// Trickling Rule: Stop processing queued events if we already handled a mouse button change
			KGVec2 event_pos(e->MousePos.PosX, e->MousePos.PosY);
			if (trickle_fast_inputs && (mouse_button_changed != 0 || mouse_wheeled || key_changed || text_inputted))
				break;
			io.MousePos = event_pos;
			mouse_moved = true;
		}
		else if (e->Type == KGGuiInputEventType_MouseButton)
		{
			// Trickling Rule: Stop processing queued events if we got multiple action on the same button
			const KarmaGuiMouseButton button = e->MouseButton.Button;
			KR_CORE_ASSERT(button >= 0 && button < KGGuiMouseButton_COUNT, "");
			if (trickle_fast_inputs && ((mouse_button_changed & (1 << button)) || mouse_wheeled))
				break;
			io.MouseDown[button] = e->MouseButton.Down;
			mouse_button_changed |= (1 << button);
		}
		else if (e->Type == KGGuiInputEventType_MouseWheel)
		{
			// Trickling Rule: Stop processing queued events if we got multiple action on the event
			if (trickle_fast_inputs && (mouse_moved || mouse_button_changed != 0))
				break;
			io.MouseWheelH += e->MouseWheel.WheelX;
			io.MouseWheel += e->MouseWheel.WheelY;
			mouse_wheeled = true;
		}
		else if (e->Type == KGGuiInputEventType_MouseViewport)
		{
			io.MouseHoveredViewport = e->MouseViewport.HoveredViewportID;
		}
		else if (e->Type == KGGuiInputEventType_Key)
		{
			// Trickling Rule: Stop processing queued events if we got multiple action on the same button
			KarmaGuiKey key = e->Key.Key;
			KR_CORE_ASSERT(key != KGGuiKey_None, "");
			KarmaGuiKeyData* key_data = GetKeyData(key);
			const int key_data_index = (int)(key_data - g.IO.KeysData);
			if (trickle_fast_inputs && key_data->Down != e->Key.Down && (key_changed_mask.TestBit(key_data_index) || text_inputted || mouse_button_changed != 0))
				break;
			key_data->Down = e->Key.Down;
			key_data->AnalogValue = e->Key.AnalogValue;
			key_changed = true;
			key_changed_mask.SetBit(key_data_index);

			// Allow legacy code using io.KeysDown[GetKeyIndex()] with new backends
#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
			io.KeysDown[key_data_index] = key_data->Down;
			if (io.KeyMap[key_data_index] != -1)
				io.KeysDown[io.KeyMap[key_data_index]] = key_data->Down;
#endif
		}
		else if (e->Type == KGGuiInputEventType_Text)
		{
			// Trickling Rule: Stop processing queued events if keys/mouse have been interacted with
			if (trickle_fast_inputs && ((key_changed && trickle_interleaved_keys_and_text) || mouse_button_changed != 0 || mouse_moved || mouse_wheeled))
				break;
			unsigned int c = e->Text.Char;
			io.InputQueueCharacters.push_back(c <= KG_UNICODE_CODEPOINT_MAX ? (KGWchar)c : KG_UNICODE_CODEPOINT_INVALID);
			if (trickle_interleaved_keys_and_text)
				text_inputted = true;
		}
		else if (e->Type == KGGuiInputEventType_Focus)
		{
			// We intentionally overwrite this and process in NewFrame(), in order to give a chance
			// to multi-viewports backends to queue AddFocusEvent(false) + AddFocusEvent(true) in same frame.
			const bool focus_lost = !e->AppFocused.Focused;
			io.AppFocusLost = focus_lost;
		}
		else
		{
			KR_CORE_ASSERT(0, "Unknown event!");
		}
	}

	// Record trail (for domain-specific applications wanting to access a precise trail)
	//if (event_n != 0) KARMAGUI_DEBUG_LOG_IO("Processed: %d / Remaining: %d\n", event_n, g.InputEventsQueue.Size - event_n);
	for (int n = 0; n < event_n; n++)
		g.InputEventsTrail.push_back(g.InputEventsQueue[n]);

	// [DEBUG]
#ifndef KARMAGUI_DISABLE_DEBUG_TOOLS
	if (event_n != 0 && (g.DebugLogFlags & KGGuiDebugLogFlags_EventIO))
		for (int n = 0; n < g.InputEventsQueue.Size; n++)
			DebugPrintInputEvent(n < event_n ? "Processed" : "Remaining", & g.InputEventsQueue[n]);
#endif

	// Remaining events will be processed on the next frame
	if (event_n == g.InputEventsQueue.Size)
		g.InputEventsQueue.resize(0);
	else
		g.InputEventsQueue.erase(g.InputEventsQueue.Data, g.InputEventsQueue.Data + event_n);

	// Clear buttons state when focus is lost
	// - this is useful so e.g. releasing Alt after focus loss on Alt-Tab doesn't trigger the Alt menu toggle.
	// - we clear in EndFrame() and not now in order allow application/user code polling this flag
	//   (e.g. custom backend may want to clear additional data, custom widgets may want to react with a "canceling" event).
	if (g.IO.AppFocusLost)
		g.IO.ClearInputKeys();
}

KGGuiID Karma::KarmaGuiInternal::GetKeyOwner(KarmaGuiKey key)
{
	if (!KarmaGuiInternal::IsNamedKeyOrModKey(key))
		return KGGuiKeyOwner_None;

	KarmaGuiContext& g = *GKarmaGui;
	KGGuiKeyOwnerData* owner_data = GetKeyOwnerData(key);
	KGGuiID owner_id = owner_data->OwnerCurr;

	if (g.ActiveIdUsingAllKeyboardKeys && owner_id != g.ActiveId && owner_id != KGGuiKeyOwner_Any)
		if (key >= KGGuiKey_Keyboard_BEGIN && key < KGGuiKey_Keyboard_END)
			return KGGuiKeyOwner_None;

	return owner_id;
}

// TestKeyOwner(..., ID)   : (owner == None || owner == ID)
// TestKeyOwner(..., None) : (owner == None)
// TestKeyOwner(..., Any)  : no owner test
// All paths are also testing for key not being locked, for the rare cases that key have been locked with using KGGuiInputFlags_LockXXX flags.
bool Karma::KarmaGuiInternal::TestKeyOwner(KarmaGuiKey key, KGGuiID owner_id)
{
	if (!KarmaGuiInternal::IsNamedKeyOrModKey(key))
		return true;

	KarmaGuiContext& g = *GKarmaGui;
	if (g.ActiveIdUsingAllKeyboardKeys && owner_id != g.ActiveId && owner_id != KGGuiKeyOwner_Any)
		if (key >= KGGuiKey_Keyboard_BEGIN && key < KGGuiKey_Keyboard_END)
			return false;

	KGGuiKeyOwnerData* owner_data = GetKeyOwnerData(key);
	if (owner_id == KGGuiKeyOwner_Any)
		return (owner_data->LockThisFrame == false);

	// Note: SetKeyOwner() sets OwnerCurr. It is not strictly required for most mouse routing overlap (because of ActiveId/HoveredId
	// are acting as filter before this has a chance to filter), but sane as soon as user tries to look into things.
	// Setting OwnerCurr in SetKeyOwner() is more consistent than testing OwnerNext here: would be inconsistent with getter and other functions.
	if (owner_data->OwnerCurr != owner_id)
	{
		if (owner_data->LockThisFrame)
			return false;
		if (owner_data->OwnerCurr != KGGuiKeyOwner_None)
			return false;
	}

	return true;
}

// _LockXXX flags are useful to lock keys away from code which is not input-owner aware.
// When using _LockXXX flags, you can use KGGuiKeyOwner_Any to lock keys from everyone.
// - SetKeyOwner(..., None)              : clears owner
// - SetKeyOwner(..., Any, !Lock)        : illegal (assert)
// - SetKeyOwner(..., Any or None, Lock) : set lock
void Karma::KarmaGuiInternal::SetKeyOwner(KarmaGuiKey key, KGGuiID owner_id, KarmaGuiInputFlags flags)
{
	KR_CORE_ASSERT(IsNamedKeyOrModKey(key) && (owner_id != KGGuiKeyOwner_Any || (flags & (KGGuiInputFlags_LockThisFrame | KGGuiInputFlags_LockUntilRelease))), ""); // Can only use _Any with _LockXXX flags (to eat a key away without an ID to retrieve it)
	KR_CORE_ASSERT((flags & ~KGGuiInputFlags_SupportedBySetKeyOwner) == 0, ""); // Passing flags not supported by this function!

	KGGuiKeyOwnerData* owner_data = GetKeyOwnerData(key);
	owner_data->OwnerCurr = owner_data->OwnerNext = owner_id;

	// We cannot lock by default as it would likely break lots of legacy code.
	// In the case of using LockUntilRelease while key is not down we still lock during the frame (no key_data->Down test)
	owner_data->LockUntilRelease = (flags & KGGuiInputFlags_LockUntilRelease) != 0;
	owner_data->LockThisFrame = (flags & KGGuiInputFlags_LockThisFrame) != 0 || (owner_data->LockUntilRelease);
}

// This is more or less equivalent to:
//   if (IsItemHovered() || IsItemActive())
//       SetKeyOwner(key, GetItemID());
// Extensive uses of that (e.g. many calls for a single item) may want to manually perform the tests once and then call SetKeyOwner() multiple times.
// More advanced usage scenarios may want to call SetKeyOwner() manually based on different condition.
// Worth noting is that only one item can be hovered and only one item can be active, therefore this usage pattern doesn't need to bother with routing and priority.
void Karma::KarmaGuiInternal::SetItemKeyOwner(KarmaGuiKey key, KarmaGuiInputFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiID id = g.LastItemData.ID;
	if (id == 0 || (g.HoveredId != id && g.ActiveId != id))
		return;
	if ((flags & KGGuiInputFlags_CondMask_) == 0)
		flags |= KGGuiInputFlags_CondDefault_;
	if ((g.HoveredId == id && (flags & KGGuiInputFlags_CondHovered)) || (g.ActiveId == id && (flags & KGGuiInputFlags_CondActive)))
	{
		KR_CORE_ASSERT((flags & ~KGGuiInputFlags_SupportedBySetItemKeyOwner) == 0, ""); // Passing flags not supported by this function!
		SetKeyOwner(key, id, flags & ~KGGuiInputFlags_CondMask_);
	}
}

bool Karma::KarmaGui::Shortcut(KarmaGuiKeyChord key_chord, KGGuiID owner_id, KarmaGuiInputFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;

	// When using (owner_id == 0/Any): SetShortcutRouting() will use CurrentFocusScopeId and filter with this, so IsKeyPressed() is fine with he 0/Any.
	if ((flags & KGGuiInputFlags_RouteMask_) == 0)
		flags |= KGGuiInputFlags_RouteFocused;
	if (!KarmaGuiInternal::SetShortcutRouting(key_chord, owner_id, flags))
		return false;

	if (key_chord & KGGuiMod_Shortcut)
		key_chord = KarmaGuiInternal::ConvertShortcutMod(key_chord);
	KarmaGuiKey mods = (KarmaGuiKey)(key_chord & KGGuiMod_Mask_);
	if (g.IO.KeyMods != mods)
		return false;

	// Special storage location for mods
	KarmaGuiKey key = (KarmaGuiKey)(key_chord & ~KGGuiMod_Mask_);
	if (key == KGGuiKey_None)
		key = KarmaGuiInternal::ConvertSingleModFlagToKey(mods);

	if (!KarmaGuiInternal::IsKeyPressed(key, owner_id, (flags & (KGGuiInputFlags_Repeat | (KarmaGuiInputFlags)KGGuiInputFlags_RepeatRateMask_))))
		return false;
	KR_CORE_ASSERT((flags & ~KGGuiInputFlags_SupportedByShortcut) == 0, ""); // Passing flags not supported by this function!

	return true;
}

//-----------------------------------------------------------------------------
// [SECTION] ERROR CHECKING
//-----------------------------------------------------------------------------

// Helper function to verify ABI compatibility between caller code and compiled version of Dear ImGui.
// Verify that the type sizes are matching between the calling file's compilation unit and imgui.cpp's compilation unit
// If this triggers you have an issue:
// - Most commonly: mismatched headers and compiled code version.
// - Or: mismatched configuration #define, compilation settings, packing pragma etc.
//   The configuration settings mentioned in imconfig.h must be set for all compilation units involved with Dear ImGui,
//   which is way it is required you put them in your imconfig file (and not just before including imgui.h).
//   Otherwise it is possible that different compilation units would see different structure layout
bool Karma::KarmaGui::DebugCheckVersionAndDataLayout(const char* version, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_vert, size_t sz_idx)
{
	bool error = false;
	if (sz_io != sizeof(KarmaGuiIO)) { error = true; KR_CORE_ASSERT(sz_io == sizeof(KarmaGuiIO), "Mismatched struct layout!"); }
	if (sz_style != sizeof(KarmaGuiStyle)) { error = true; KR_CORE_ASSERT(sz_style == sizeof(KarmaGuiStyle), "Mismatched struct layout!"); }
	if (sz_vec2 != sizeof(KGVec2)) { error = true; KR_CORE_ASSERT(sz_vec2 == sizeof(KGVec2), "Mismatched struct layout!"); }
	if (sz_vec4 != sizeof(KGVec4)) { error = true; KR_CORE_ASSERT(sz_vec4 == sizeof(KGVec4), "Mismatched struct layout!"); }
	if (sz_vert != sizeof(KGDrawVert)) { error = true; KR_CORE_ASSERT(sz_vert == sizeof(KGDrawVert), "Mismatched struct layout!"); }
	if (sz_idx != sizeof(KGDrawIdx)) { error = true; KR_CORE_ASSERT(sz_idx == sizeof(KGDrawIdx), "Mismatched struct layout!"); }
	return !error;
}

// Until 1.89 (IMGUI_VERSION_NUM < 18814) it was legal to use SetCursorPos() to extend the boundary of a parent (e.g. window or table cell)
// This is causing issues and ambiguity and we need to retire that.
// See https://github.com/ocornut/imgui/issues/5548 for more details.
// [Scenario 1]
//  Previously this would make the window content size ~200x200:
//    Begin(...) + SetCursorScreenPos(GetCursorScreenPos() + KGVec2(200,200)) + End();  // NOT OK
//  Instead, please submit an item:
//    Begin(...) + SetCursorScreenPos(GetCursorScreenPos() + KGVec2(200,200)) + Dummy(KGVec2(0,0)) + End(); // OK
//  Alternative:
//    Begin(...) + Dummy(KGVec2(200,200)) + End(); // OK
// [Scenario 2]
//  For reference this is one of the issue what we aim to fix with this change:
//    BeginGroup() + SomeItem("foobar") + SetCursorScreenPos(GetCursorScreenPos()) + EndGroup()
//  The previous logic made SetCursorScreenPos(GetCursorScreenPos()) have a side-effect! It would erroneously incorporate ItemSpacing.y after the item into content size, making the group taller!
//  While this code is a little twisted, no-one would expect SetXXX(GetXXX()) to have a side-effect. Using vertical alignment patterns could trigger this issue.
void Karma::KarmaGuiInternal::ErrorCheckUsingSetCursorPosToExtendParentBoundaries()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	KR_CORE_ASSERT(window->DC.IsSetPos, "");
	window->DC.IsSetPos = false;
#ifdef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
	if (window->DC.CursorPos.x <= window->DC.CursorMaxPos.x && window->DC.CursorPos.y <= window->DC.CursorMaxPos.y)
		return;
	if (window->SkipItems)
		return;
	KR_CORE_ASSERT(0, "Code uses SetCursorPos()/SetCursorScreenPos() to extend window/parent boundaries. Please submit an item e.g. Dummy() to validate extent.");
#else
	window->DC.CursorMaxPos = KGMax(window->DC.CursorMaxPos, window->DC.CursorPos);
#endif
}

void Karma::KarmaGuiInternal::ErrorCheckNewFrameSanityChecks()
{
	KarmaGuiContext& g = *GKarmaGui;

	// Check user KR_CORE_ASSERT macro
	// (IF YOU GET A WARNING OR COMPILE ERROR HERE: it means your assert macro is incorrectly defined!
	//  If your macro uses multiple statements, it NEEDS to be surrounded by a 'do { ... } while (0)' block.
	//  This is a common C/C++ idiom to allow multiple statements macros to be used in control flow blocks.)
	// #define KR_CORE_ASSERT(EXPR)   if (SomeCode(EXPR)) SomeMoreCode();                    // Wrong!
	// #define KR_CORE_ASSERT(EXPR)   do { if (SomeCode(EXPR)) SomeMoreCode(); } while (0)   // Correct!
	if (true)
	{
		KR_CORE_ASSERT(1, "");
	}
	else
	{
		KR_CORE_ASSERT(0, "");
	}

	// Check user data
	// (We pass an error message in the assert expression to make it visible to programmers who are not using a debugger, as most assert handlers display their argument)
	KR_CORE_ASSERT(g.Initialized, "");
	KR_CORE_ASSERT((g.IO.DeltaTime > 0.0f || g.FrameCount == 0), "Need a positive DeltaTime!");
	KR_CORE_ASSERT((g.FrameCount == 0 || g.FrameCountEnded == g.FrameCount), "Forgot to call Render() or EndFrame() at the end of the previous frame?");
	KR_CORE_ASSERT(g.IO.DisplaySize.x >= 0.0f && g.IO.DisplaySize.y >= 0.0f, "Invalid DisplaySize value!");
	KR_CORE_ASSERT(g.IO.Fonts->IsBuilt(), "Font Atlas not built! Make sure you called ImGui_ImplXXXX_NewFrame() function for renderer backend, which should call io.Fonts->GetTexDataAsRGBA32() / GetTexDataAsAlpha8()");
	KR_CORE_ASSERT(g.Style.CurveTessellationTol > 0.0f, "Invalid style setting!");
	KR_CORE_ASSERT(g.Style.CircleTessellationMaxError > 0.0f, "Invalid style setting!");
	KR_CORE_ASSERT(g.Style.Alpha >= 0.0f && g.Style.Alpha <= 1.0f, "Invalid style setting!"); // Allows us to avoid a few clamps in color computations
	KR_CORE_ASSERT(g.Style.WindowMinSize.x >= 1.0f && g.Style.WindowMinSize.y >= 1.0f, "Invalid style setting.");
	KR_CORE_ASSERT(g.Style.WindowMenuButtonPosition == KGGuiDir_None || g.Style.WindowMenuButtonPosition == KGGuiDir_Left || g.Style.WindowMenuButtonPosition == KGGuiDir_Right, "");
	KR_CORE_ASSERT(g.Style.ColorButtonPosition == KGGuiDir_Left || g.Style.ColorButtonPosition == KGGuiDir_Right, "");
#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
	for (int n = KGGuiKey_NamedKey_BEGIN; n < KGGuiKey_COUNT; n++)
		KR_CORE_ASSERT(g.IO.KeyMap[n] >= -1 && g.IO.KeyMap[n] < KGGuiKey_LegacyNativeKey_END, "io.KeyMap[] contains an out of bound value (need to be 0..511, or -1 for unmapped key)");

	// Check: required key mapping (we intentionally do NOT check all keys to not pressure user into setting up everything, but Space is required and was only added in 1.60 WIP)
	if ((g.IO.ConfigFlags & KGGuiConfigFlags_NavEnableKeyboard) && g.IO.BackendUsingLegacyKeyArrays == 1)
	{
		KR_CORE_ASSERT(g.IO.KeyMap[KGGuiKey_Space] != -1, "KGGuiKey_Space is not mapped, required for keyboard navigation.");
	}
#endif

	// Check: the io.ConfigWindowsResizeFromEdges option requires backend to honor mouse cursor changes and set the KGGuiBackendFlags_HasMouseCursors flag accordingly.
	if (g.IO.ConfigWindowsResizeFromEdges && !(g.IO.BackendFlags & KGGuiBackendFlags_HasMouseCursors))
		g.IO.ConfigWindowsResizeFromEdges = false;

	// Perform simple check: error if Docking or Viewport are enabled _exactly_ on frame 1 (instead of frame 0 or later), which is a common error leading to loss of .ini data.
	if (g.FrameCount == 1 && (g.IO.ConfigFlags & KGGuiConfigFlags_DockingEnable) && (g.ConfigFlagsLastFrame & KGGuiConfigFlags_DockingEnable) == 0)
	{
		KR_CORE_ASSERT(0, "Please set DockingEnable before the first call to NewFrame()! Otherwise you will lose your .ini settings!");
	}
	if (g.FrameCount == 1 && (g.IO.ConfigFlags & KGGuiConfigFlags_ViewportsEnable) && (g.ConfigFlagsLastFrame & KGGuiConfigFlags_ViewportsEnable) == 0)
	{
		KR_CORE_ASSERT(0, "Please set ViewportsEnable before the first call to NewFrame()! Otherwise you will lose your .ini settings!");
	}

	// Perform simple checks: multi-viewport and platform windows support
	if (g.IO.ConfigFlags & KGGuiConfigFlags_ViewportsEnable)
	{
		if ((g.IO.BackendFlags & KGGuiBackendFlags_PlatformHasViewports) && (g.IO.BackendFlags & KGGuiBackendFlags_RendererHasViewports))
		{
			KR_CORE_ASSERT((g.FrameCount == 0 || g.FrameCount == g.FrameCountPlatformEnded), "Forgot to call UpdatePlatformWindows() in main loop after EndFrame()? Check examples/ applications for reference.");
			KR_CORE_ASSERT(g.PlatformIO.Platform_CreateWindow != NULL, "Platform init didn't install handlers?");
			KR_CORE_ASSERT(g.PlatformIO.Platform_DestroyWindow != NULL, "Platform init didn't install handlers?");
			KR_CORE_ASSERT(g.PlatformIO.Platform_GetWindowPos != NULL, "Platform init didn't install handlers?");
			KR_CORE_ASSERT(g.PlatformIO.Platform_SetWindowPos != NULL, "Platform init didn't install handlers?");
			KR_CORE_ASSERT(g.PlatformIO.Platform_GetWindowSize != NULL, "Platform init didn't install handlers?");
			KR_CORE_ASSERT(g.PlatformIO.Platform_SetWindowSize != NULL, "Platform init didn't install handlers?");
			KR_CORE_ASSERT(g.PlatformIO.Monitors.Size > 0, "Platform init didn't setup Monitors list?");
			KR_CORE_ASSERT((g.Viewports[0]->PlatformUserData != NULL || g.Viewports[0]->PlatformHandle != NULL), "Platform init didn't setup main viewport.");
			if (g.IO.ConfigDockingTransparentPayload && (g.IO.ConfigFlags & KGGuiConfigFlags_DockingEnable))
			{
				KR_CORE_ASSERT(g.PlatformIO.Platform_SetWindowAlpha != NULL, "Platform_SetWindowAlpha handler is required to use io.ConfigDockingTransparent!");
			}
		}
		else
		{
			// Disable feature, our backends do not support it
			g.IO.ConfigFlags &= ~KGGuiConfigFlags_ViewportsEnable;
		}

		// Perform simple checks on platform monitor data + compute a total bounding box for quick early outs
		for (int monitor_n = 0; monitor_n < g.PlatformIO.Monitors.Size; monitor_n++)
		{
			KarmaGuiPlatformMonitor& mon = g.PlatformIO.Monitors[monitor_n];
			KG_UNUSED(mon);
			KR_CORE_ASSERT(mon.MainSize.x > 0.0f && mon.MainSize.y > 0.0f, "Monitor main bounds not setup properly.");
			KR_CORE_ASSERT(KGRect(mon.MainPos, mon.MainPos + mon.MainSize).Contains(KGRect(mon.WorkPos, mon.WorkPos + mon.WorkSize)), "Monitor work bounds not setup properly. If you don't have work area information, just copy MainPos/MainSize into them.");
			KR_CORE_ASSERT(mon.DpiScale != 0.0f, "");
		}
	}
}

void Karma::KarmaGuiInternal::ErrorCheckEndFrameSanityChecks()
{
	KarmaGuiContext& g = *GKarmaGui;

	// Verify that io.KeyXXX fields haven't been tampered with. Key mods should not be modified between NewFrame() and EndFrame()
	// One possible reason leading to this assert is that your backends update inputs _AFTER_ NewFrame().
	// It is known that when some modal native windows called mid-frame takes focus away, some backends such as GLFW will
	// send key release events mid-frame. This would normally trigger this assertion and lead to sheared inputs.
	// We silently accommodate for this case by ignoring the case where all io.KeyXXX modifiers were released (aka key_mod_flags == 0),
	// while still correctly asserting on mid-frame key press events.
	const KarmaGuiKeyChord key_mods = GetMergedModsFromKeys();
	KR_CORE_ASSERT((key_mods == 0 || g.IO.KeyMods == key_mods), "Mismatching io.KeyCtrl/io.KeyShift/io.KeyAlt/io.KeySuper vs io.KeyMods");
	KG_UNUSED(key_mods);

	// [EXPERIMENTAL] Recover from errors: You may call this yourself before EndFrame().
	//ErrorCheckEndFrameRecover();

	// Report when there is a mismatch of Begin/BeginChild vs End/EndChild calls. Important: Remember that the Begin/BeginChild API requires you
	// to always call End/EndChild even if Begin/BeginChild returns false! (this is unfortunately inconsistent with most other Begin* API).
	if (g.CurrentWindowStack.Size != 1)
	{
		if (g.CurrentWindowStack.Size > 1)
		{
			KR_CORE_ASSERT(g.CurrentWindowStack.Size == 1, "Mismatched Begin/BeginChild vs End/EndChild calls: did you forget to call End/EndChild?");
			while (g.CurrentWindowStack.Size > 1)
				KarmaGui::End();
		}
		else
		{
			KR_CORE_ASSERT(g.CurrentWindowStack.Size == 1, "Mismatched Begin/BeginChild vs End/EndChild calls: did you call End/EndChild too much?");
		}
	}

	KR_CORE_ASSERT(g.GroupStack.Size == 0, "Missing EndGroup call!");
}

// Experimental recovery from incorrect usage of BeginXXX/EndXXX/PushXXX/PopXXX calls.
// Must be called during or before EndFrame().
// This is generally flawed as we are not necessarily End/Popping things in the right order.
// FIXME: Can't recover from inside BeginTabItem/EndTabItem yet.
// FIXME: Can't recover from interleaved BeginTabBar/Begin
void Karma::KarmaGuiInternal::ErrorCheckEndFrameRecover(KGGuiErrorLogCallback log_callback, void* user_data)
{
	// PVS-Studio V1044 is "Loop break conditions do not depend on the number of iterations"
	KarmaGuiContext& g = *GKarmaGui;
	while (g.CurrentWindowStack.Size > 0) //-V1044
	{
		ErrorCheckEndWindowRecover(log_callback, user_data);
		KGGuiWindow* window = g.CurrentWindow;
		if (g.CurrentWindowStack.Size == 1)
		{
			KR_CORE_ASSERT(window->IsFallbackWindow, "");
			break;
		}
		if (window->Flags & KGGuiWindowFlags_ChildWindow)
		{
			if (log_callback) log_callback(user_data, "Recovered from missing EndChild() for '%s'", window->Name);
			KarmaGui::EndChild();
		}
		else
		{
			if (log_callback) log_callback(user_data, "Recovered from missing End() for '%s'", window->Name);
			KarmaGui::End();
		}
	}
}

// Must be called before End()/EndChild()
void Karma::KarmaGuiInternal::ErrorCheckEndWindowRecover(KGGuiErrorLogCallback log_callback, void* user_data)
{
	KarmaGuiContext& g = *GKarmaGui;
	while (g.CurrentTable && (g.CurrentTable->OuterWindow == g.CurrentWindow || g.CurrentTable->InnerWindow == g.CurrentWindow))
	{
		if (log_callback) log_callback(user_data, "Recovered from missing EndTable() in '%s'", g.CurrentTable->OuterWindow->Name);
		KarmaGui::EndTable();
	}

	KGGuiWindow* window = g.CurrentWindow;
	KGGuiStackSizes* stack_sizes = &g.CurrentWindowStack.back().StackSizesOnBegin;
	KR_CORE_ASSERT(window != NULL, "");
	while (g.CurrentTabBar != NULL) //-V1044
	{
		if (log_callback) log_callback(user_data, "Recovered from missing EndTabBar() in '%s'", window->Name);
		KarmaGui::EndTabBar();
	}
	while (window->DC.TreeDepth > 0)
	{
		if (log_callback) log_callback(user_data, "Recovered from missing TreePop() in '%s'", window->Name);
		KarmaGui::TreePop();
	}
	while (g.GroupStack.Size > stack_sizes->SizeOfGroupStack) //-V1044
	{
		if (log_callback) log_callback(user_data, "Recovered from missing EndGroup() in '%s'", window->Name);
		KarmaGui::EndGroup();
	}
	while (window->IDStack.Size > 1)
	{
		if (log_callback) log_callback(user_data, "Recovered from missing PopID() in '%s'", window->Name);
		KarmaGui::PopID();
	}
	while (g.DisabledStackSize > stack_sizes->SizeOfDisabledStack) //-V1044
	{
		if (log_callback) log_callback(user_data, "Recovered from missing EndDisabled() in '%s'", window->Name);
		KarmaGui::EndDisabled();
	}
	while (g.ColorStack.Size > stack_sizes->SizeOfColorStack)
	{
		if (log_callback) log_callback(user_data, "Recovered from missing PopStyleColor() in '%s' for KGGuiCol_%s", window->Name, KarmaGui::GetStyleColorName(g.ColorStack.back().Col));
		KarmaGui::PopStyleColor();
	}
	while (g.ItemFlagsStack.Size > stack_sizes->SizeOfItemFlagsStack) //-V1044
	{
		if (log_callback) log_callback(user_data, "Recovered from missing PopItemFlag() in '%s'", window->Name);
		PopItemFlag();
	}
	while (g.StyleVarStack.Size > stack_sizes->SizeOfStyleVarStack) //-V1044
	{
		if (log_callback) log_callback(user_data, "Recovered from missing PopStyleVar() in '%s'", window->Name);
		KarmaGui::PopStyleVar();
	}
	while (g.FocusScopeStack.Size > stack_sizes->SizeOfFocusScopeStack + 1) //-V1044
	{
		if (log_callback) log_callback(user_data, "Recovered from missing PopFocusScope() in '%s'", window->Name);
		PopFocusScope();
	}
}

// Save current stack sizes for later compare
void KGGuiStackSizes::SetToCurrentState()
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	SizeOfIDStack = (short)window->IDStack.Size;
	SizeOfColorStack = (short)g.ColorStack.Size;
	SizeOfStyleVarStack = (short)g.StyleVarStack.Size;
	SizeOfFontStack = (short)g.FontStack.Size;
	SizeOfFocusScopeStack = (short)g.FocusScopeStack.Size;
	SizeOfGroupStack = (short)g.GroupStack.Size;
	SizeOfItemFlagsStack = (short)g.ItemFlagsStack.Size;
	SizeOfBeginPopupStack = (short)g.BeginPopupStack.Size;
	SizeOfDisabledStack = (short)g.DisabledStackSize;
}

// Compare to detect usage errors
void KGGuiStackSizes::CompareWithCurrentState()
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	KG_UNUSED(window);

	// Window stacks
	// NOT checking: DC.ItemWidth, DC.TextWrapPos (per window) to allow user to conveniently push once and not pop (they are cleared on Begin)
	KR_CORE_ASSERT(SizeOfIDStack == window->IDStack.Size, "PushID/PopID or TreeNode/TreePop Mismatch!");

	// Global stacks
	// For color, style and font stacks there is an incentive to use Push/Begin/Pop/.../End patterns, so we relax our checks a little to allow them.
	KR_CORE_ASSERT(SizeOfGroupStack == g.GroupStack.Size, "BeginGroup/EndGroup Mismatch!");
	KR_CORE_ASSERT(SizeOfBeginPopupStack == g.BeginPopupStack.Size, "BeginPopup/EndPopup or BeginMenu/EndMenu Mismatch!");
	KR_CORE_ASSERT(SizeOfDisabledStack == g.DisabledStackSize, "BeginDisabled/EndDisabled Mismatch!");
	KR_CORE_ASSERT(SizeOfItemFlagsStack >= g.ItemFlagsStack.Size, "PushItemFlag/PopItemFlag Mismatch!");
	KR_CORE_ASSERT(SizeOfColorStack >= g.ColorStack.Size, "PushStyleColor/PopStyleColor Mismatch!");
	KR_CORE_ASSERT(SizeOfStyleVarStack >= g.StyleVarStack.Size, "PushStyleVar/PopStyleVar Mismatch!");
	KR_CORE_ASSERT(SizeOfFontStack >= g.FontStack.Size, "PushFont/PopFont Mismatch!");
	KR_CORE_ASSERT(SizeOfFocusScopeStack == g.FocusScopeStack.Size, "PushFocusScope/PopFocusScope Mismatch!");
}

//-----------------------------------------------------------------------------
// [SECTION] LAYOUT
//-----------------------------------------------------------------------------
// - ItemSize()
// - ItemAdd()
// - SameLine()
// - GetCursorScreenPos()
// - SetCursorScreenPos()
// - GetCursorPos(), GetCursorPosX(), GetCursorPosY()
// - SetCursorPos(), SetCursorPosX(), SetCursorPosY()
// - GetCursorStartPos()
// - Indent()
// - Unindent()
// - SetNextItemWidth()
// - PushItemWidth()
// - PushMultiItemsWidths()
// - PopItemWidth()
// - CalcItemWidth()
// - CalcItemSize()
// - GetTextLineHeight()
// - GetTextLineHeightWithSpacing()
// - GetFrameHeight()
// - GetFrameHeightWithSpacing()
// - GetContentRegionMax()
// - GetContentRegionMaxAbs() [Internal]
// - GetContentRegionAvail(),
// - GetWindowContentRegionMin(), GetWindowContentRegionMax()
// - BeginGroup()
// - EndGroup()
// Also see in imgui_widgets: tab bars, and in imgui_tables: tables, columns.
//-----------------------------------------------------------------------------

// Advance cursor given item size for layout.
// Register minimum needed size so it can extend the bounding box used for auto-fit calculation.
// See comments in ItemAdd() about how/why the size provided to ItemSize() vs ItemAdd() may often different.
void Karma::KarmaGuiInternal::ItemSize(const KGVec2& size, float text_baseline_y)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	if (window->SkipItems)
		return;

	// We increase the height in this function to accommodate for baseline offset.
	// In theory we should be offsetting the starting position (window->DC.CursorPos), that will be the topic of a larger refactor,
	// but since ItemSize() is not yet an API that moves the cursor (to handle e.g. wrapping) enlarging the height has the same effect.
	const float offset_to_match_baseline_y = (text_baseline_y >= 0) ? KGMax(0.0f, window->DC.CurrLineTextBaseOffset - text_baseline_y) : 0.0f;

	const float line_y1 = window->DC.IsSameLine ? window->DC.CursorPosPrevLine.y : window->DC.CursorPos.y;
	const float line_height = KGMax(window->DC.CurrLineSize.y, /*KGMax(*/window->DC.CursorPos.y - line_y1/*, 0.0f)*/ + size.y + offset_to_match_baseline_y);

	// Always align ourselves on pixel boundaries
	//if (g.IO.KeyAlt) window->DrawList->AddRect(window->DC.CursorPos, window->DC.CursorPos + KGVec2(size.x, line_height), KG_COL32(255,0,0,200)); // [DEBUG]
	window->DC.CursorPosPrevLine.x = window->DC.CursorPos.x + size.x;
	window->DC.CursorPosPrevLine.y = line_y1;
	window->DC.CursorPos.x = KG_FLOOR(window->Pos.x + window->DC.Indent.x + window->DC.ColumnsOffset.x);    // Next line
	window->DC.CursorPos.y = KG_FLOOR(line_y1 + line_height + g.Style.ItemSpacing.y);                       // Next line
	window->DC.CursorMaxPos.x = KGMax(window->DC.CursorMaxPos.x, window->DC.CursorPosPrevLine.x);
	window->DC.CursorMaxPos.y = KGMax(window->DC.CursorMaxPos.y, window->DC.CursorPos.y - g.Style.ItemSpacing.y);
	//if (g.IO.KeyAlt) window->DrawList->AddCircle(window->DC.CursorMaxPos, 3.0f, KG_COL32(255,0,0,255), 4); // [DEBUG]

	window->DC.PrevLineSize.y = line_height;
	window->DC.CurrLineSize.y = 0.0f;
	window->DC.PrevLineTextBaseOffset = KGMax(window->DC.CurrLineTextBaseOffset, text_baseline_y);
	window->DC.CurrLineTextBaseOffset = 0.0f;
	window->DC.IsSameLine = window->DC.IsSetPos = false;

	// Horizontal layout mode
	if (window->DC.LayoutType == KGGuiLayoutType_Horizontal)
		KarmaGui::SameLine();
}

// Declare item bounding box for clipping and interaction.
// Note that the size can be different than the one provided to ItemSize(). Typically, widgets that spread over available surface
// declare their minimum size requirement to ItemSize() and provide a larger region to ItemAdd() which is used drawing/interaction.
bool Karma::KarmaGuiInternal::ItemAdd(const KGRect& bb, KGGuiID id, const KGRect* nav_bb_arg, KGGuiItemFlags extra_flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;

	// Set item data
	// (DisplayRect is left untouched, made valid when KGGuiItemStatusFlags_HasDisplayRect is set)
	g.LastItemData.ID = id;
	g.LastItemData.Rect = bb;
	g.LastItemData.NavRect = nav_bb_arg ? *nav_bb_arg : bb;
	g.LastItemData.InFlags = g.CurrentItemFlags | extra_flags;
	g.LastItemData.StatusFlags = KGGuiItemStatusFlags_None;

	// Directional navigation processing
	if (id != 0)
	{
		KeepAliveID(id);

		// Runs prior to clipping early-out
		//  (a) So that NavInitRequest can be honored, for newly opened windows to select a default widget
		//  (b) So that we can scroll up/down past clipped items. This adds a small O(N) cost to regular navigation requests
		//      unfortunately, but it is still limited to one window. It may not scale very well for windows with ten of
		//      thousands of item, but at least NavMoveRequest is only set on user interaction, aka maximum once a frame.
		//      We could early out with "if (is_clipped && !g.NavInitRequest) return false;" but when we wouldn't be able
		//      to reach unclipped widgets. This would work if user had explicit scrolling control (e.g. mapped on a stick).
		// We intentionally don't check if g.NavWindow != NULL because g.NavAnyRequest should only be set when it is non null.
		// If we crash on a NULL g.NavWindow we need to fix the bug elsewhere.
		if (!(g.LastItemData.InFlags & KGGuiItemFlags_NoNav))
		{
			window->DC.NavLayersActiveMaskNext |= (1 << window->DC.NavLayerCurrent);
			if (g.NavId == id || g.NavAnyRequest)
				if (g.NavWindow->RootWindowForNav == window->RootWindowForNav)
					if (window == g.NavWindow || ((window->Flags | g.NavWindow->Flags) & KGGuiWindowFlags_NavFlattened))
						NavProcessItem();
		}

		// [DEBUG] People keep stumbling on this problem and using "" as identifier in the root of a window instead of "##something".
		// Empty identifier are valid and useful in a small amount of cases, but 99.9% of the time you want to use "##something".
		// READ THE FAQ: https://dearimgui.org/faq
		KR_CORE_ASSERT(id != window->ID, "Cannot have an empty ID at the root of a window. If you need an empty label, use ## and read the FAQ about how the ID Stack works!");
	}
	g.NextItemData.Flags = KGGuiNextItemDataFlags_None;

#ifdef IMGUI_ENABLE_TEST_ENGINE
	if (id != 0)
		KARMAGUI_TEST_ENGINE_ITEM_ADD(nav_bb_arg ? *nav_bb_arg : bb, id);
#endif

	// Clipping test
	// (FIXME: This is a modified copy of IsClippedEx() so we can reuse the is_rect_visible value)
	//const bool is_clipped = IsClippedEx(bb, id);
	//if (is_clipped)
	//    return false;
	const bool is_rect_visible = bb.Overlaps(window->ClipRect);
	if (!is_rect_visible)
		if (id == 0 || (id != g.ActiveId && id != g.NavId))
			if (!g.LogEnabled)
				return false;

	// [DEBUG]
#ifndef KARMAGUI_DISABLE_DEBUG_TOOLS
	if (id != 0 && id == g.DebugLocateId)
		DebugLocateItemResolveWithLastItem();
#endif
	//if (g.IO.KeyAlt) window->DrawList->AddRect(bb.Min, bb.Max, KG_COL32(255,255,0,120)); // [DEBUG]

	// We need to calculate this now to take account of the current clipping rectangle (as items like Selectable may change them)
	if (is_rect_visible)
		g.LastItemData.StatusFlags |= KGGuiItemStatusFlags_Visible;
	if (KarmaGui::IsMouseHoveringRect(bb.Min, bb.Max))
		g.LastItemData.StatusFlags |= KGGuiItemStatusFlags_HoveredRect;
	return true;
}

// Gets back to previous line and continue with horizontal layout
//      offset_from_start_x == 0 : follow right after previous item
//      offset_from_start_x != 0 : align to specified x position (relative to window/group left)
//      spacing_w < 0            : use default spacing if pos_x == 0, no spacing if pos_x != 0
//      spacing_w >= 0           : enforce spacing amount
void Karma::KarmaGui::SameLine(float offset_from_start_x, float spacing_w)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	if (window->SkipItems)
		return;

	if (offset_from_start_x != 0.0f)
	{
		if (spacing_w < 0.0f)
			spacing_w = 0.0f;
		window->DC.CursorPos.x = window->Pos.x - window->Scroll.x + offset_from_start_x + spacing_w + window->DC.GroupOffset.x + window->DC.ColumnsOffset.x;
		window->DC.CursorPos.y = window->DC.CursorPosPrevLine.y;
	}
	else
	{
		if (spacing_w < 0.0f)
			spacing_w = g.Style.ItemSpacing.x;
		window->DC.CursorPos.x = window->DC.CursorPosPrevLine.x + spacing_w;
		window->DC.CursorPos.y = window->DC.CursorPosPrevLine.y;
	}
	window->DC.CurrLineSize = window->DC.PrevLineSize;
	window->DC.CurrLineTextBaseOffset = window->DC.PrevLineTextBaseOffset;
	window->DC.IsSameLine = true;
}

KGVec2 Karma::KarmaGui::GetCursorScreenPos()
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindowRead();
	return window->DC.CursorPos;
}

// 2022/08/05: Setting cursor position also extend boundaries (via modifying CursorMaxPos) used to compute window size, group size etc.
// I believe this was is a judicious choice but it's probably being relied upon (it has been the case since 1.31 and 1.50)
// It would be sane if we requested user to use SetCursorPos() + Dummy(KGVec2(0,0)) to extend CursorMaxPos...
void Karma::KarmaGui::SetCursorScreenPos(const KGVec2& pos)
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	window->DC.CursorPos = pos;
	//window->DC.CursorMaxPos = KGMax(window->DC.CursorMaxPos, window->DC.CursorPos);
	window->DC.IsSetPos = true;
}

// User generally sees positions in window coordinates. Internally we store CursorPos in absolute screen coordinates because it is more convenient.
// Conversion happens as we pass the value to user, but it makes our naming convention confusing because GetCursorPos() == (DC.CursorPos - window.Pos). May want to rename 'DC.CursorPos'.
KGVec2 Karma::KarmaGui::GetCursorPos()
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindowRead();
	return window->DC.CursorPos - window->Pos + window->Scroll;
}

float Karma::KarmaGui::GetCursorPosX()
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindowRead();
	return window->DC.CursorPos.x - window->Pos.x + window->Scroll.x;
}

float Karma::KarmaGui::GetCursorPosY()
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindowRead();
	return window->DC.CursorPos.y - window->Pos.y + window->Scroll.y;
}

void Karma::KarmaGui::SetCursorPos(const KGVec2& local_pos)
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	window->DC.CursorPos = window->Pos - window->Scroll + local_pos;
	//window->DC.CursorMaxPos = KGMax(window->DC.CursorMaxPos, window->DC.CursorPos);
	window->DC.IsSetPos = true;
}

void Karma::KarmaGui::SetCursorPosX(float x)
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	window->DC.CursorPos.x = window->Pos.x - window->Scroll.x + x;
	//window->DC.CursorMaxPos.x = KGMax(window->DC.CursorMaxPos.x, window->DC.CursorPos.x);
	window->DC.IsSetPos = true;
}

void Karma::KarmaGui::SetCursorPosY(float y)
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	window->DC.CursorPos.y = window->Pos.y - window->Scroll.y + y;
	//window->DC.CursorMaxPos.y = KGMax(window->DC.CursorMaxPos.y, window->DC.CursorPos.y);
	window->DC.IsSetPos = true;
}

KGVec2 Karma::KarmaGui::GetCursorStartPos()
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindowRead();
	return window->DC.CursorStartPos - window->Pos;
}

void Karma::KarmaGui::Indent(float indent_w)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	window->DC.Indent.x += (indent_w != 0.0f) ? indent_w : g.Style.IndentSpacing;
	window->DC.CursorPos.x = window->Pos.x + window->DC.Indent.x + window->DC.ColumnsOffset.x;
}

void Karma::KarmaGui::Unindent(float indent_w)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	window->DC.Indent.x -= (indent_w != 0.0f) ? indent_w : g.Style.IndentSpacing;
	window->DC.CursorPos.x = window->Pos.x + window->DC.Indent.x + window->DC.ColumnsOffset.x;
}

// Affect large frame+labels widgets only.
void Karma::KarmaGui::SetNextItemWidth(float item_width)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.NextItemData.Flags |= KGGuiNextItemDataFlags_HasWidth;
	g.NextItemData.Width = item_width;
}

// FIXME: Remove the == 0.0f behavior?
void Karma::KarmaGui::PushItemWidth(float item_width)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	window->DC.ItemWidthStack.push_back(window->DC.ItemWidth); // Backup current width
	window->DC.ItemWidth = (item_width == 0.0f ? window->ItemWidthDefault : item_width);
	g.NextItemData.Flags &= ~KGGuiNextItemDataFlags_HasWidth;
}

void Karma::KarmaGuiInternal::PushMultiItemsWidths(int components, float w_full)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	const KarmaGuiStyle& style = g.Style;
	const float w_item_one = KGMax(1.0f, KG_FLOOR((w_full - (style.ItemInnerSpacing.x) * (components - 1)) / (float)components));
	const float w_item_last = KGMax(1.0f, KG_FLOOR(w_full - (w_item_one + style.ItemInnerSpacing.x) * (components - 1)));
	window->DC.ItemWidthStack.push_back(window->DC.ItemWidth); // Backup current width
	window->DC.ItemWidthStack.push_back(w_item_last);
	for (int i = 0; i < components - 2; i++)
		window->DC.ItemWidthStack.push_back(w_item_one);
	window->DC.ItemWidth = (components == 1) ? w_item_last : w_item_one;
	g.NextItemData.Flags &= ~KGGuiNextItemDataFlags_HasWidth;
}

void Karma::KarmaGui::PopItemWidth()
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	window->DC.ItemWidth = window->DC.ItemWidthStack.back();
	window->DC.ItemWidthStack.pop_back();
}

// Calculate default item width given value passed to PushItemWidth() or SetNextItemWidth().
// The SetNextItemWidth() data is generally cleared/consumed by ItemAdd() or NextItemData.ClearFlags()
float Karma::KarmaGui::CalcItemWidth()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	float w;
	if (g.NextItemData.Flags & KGGuiNextItemDataFlags_HasWidth)
		w = g.NextItemData.Width;
	else
		w = window->DC.ItemWidth;
	if (w < 0.0f)
	{
		float region_max_x = KarmaGuiInternal::GetContentRegionMaxAbs().x;
		w = KGMax(1.0f, region_max_x - window->DC.CursorPos.x + w);
	}
	w = KG_FLOOR(w);
	return w;
}

// [Internal] Calculate full item size given user provided 'size' parameter and default width/height. Default width is often == CalcItemWidth().
// Those two functions CalcItemWidth vs CalcItemSize are awkwardly named because they are not fully symmetrical.
// Note that only CalcItemWidth() is publicly exposed.
// The 4.0f here may be changed to match CalcItemWidth() and/or BeginChild() (right now we have a mismatch which is harmless but undesirable)
KGVec2 Karma::KarmaGuiInternal::CalcItemSize(KGVec2 size, float default_w, float default_h)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;

	KGVec2 region_max;
	if (size.x < 0.0f || size.y < 0.0f)
		region_max = GetContentRegionMaxAbs();

	if (size.x == 0.0f)
		size.x = default_w;
	else if (size.x < 0.0f)
		size.x = KGMax(4.0f, region_max.x - window->DC.CursorPos.x + size.x);

	if (size.y == 0.0f)
		size.y = default_h;
	else if (size.y < 0.0f)
		size.y = KGMax(4.0f, region_max.y - window->DC.CursorPos.y + size.y);

	return size;
}

float Karma::KarmaGui::GetTextLineHeight()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.FontSize;
}

float Karma::KarmaGui::GetTextLineHeightWithSpacing()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.FontSize + g.Style.ItemSpacing.y;
}

float Karma::KarmaGui::GetFrameHeight()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.FontSize + g.Style.FramePadding.y * 2.0f;
}

float Karma::KarmaGui::GetFrameHeightWithSpacing()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.FontSize + g.Style.FramePadding.y * 2.0f + g.Style.ItemSpacing.y;
}

// FIXME: All the Contents Region function are messy or misleading. WE WILL AIM TO OBSOLETE ALL OF THEM WITH A NEW "WORK RECT" API. Thanks for your patience!

// FIXME: This is in window space (not screen space!).
KGVec2 Karma::KarmaGui::GetContentRegionMax()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	KGVec2 mx = window->ContentRegionRect.Max - window->Pos;
	if (window->DC.CurrentColumns || g.CurrentTable)
		mx.x = window->WorkRect.Max.x - window->Pos.x;
	return mx;
}

// [Internal] Absolute coordinate. Saner. This is not exposed until we finishing refactoring work rect features.
KGVec2 Karma::KarmaGuiInternal::GetContentRegionMaxAbs()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	KGVec2 mx = window->ContentRegionRect.Max;
	if (window->DC.CurrentColumns || g.CurrentTable)
		mx.x = window->WorkRect.Max.x;
	return mx;
}

KGVec2 Karma::KarmaGui::GetContentRegionAvail()
{
	KGGuiWindow* window = GKarmaGui->CurrentWindow;
	return KarmaGuiInternal::GetContentRegionMaxAbs() - window->DC.CursorPos;
}

// In window space (not screen space!)
KGVec2 Karma::KarmaGui::GetWindowContentRegionMin()
{
	KGGuiWindow* window = GKarmaGui->CurrentWindow;
	return window->ContentRegionRect.Min - window->Pos;
}

KGVec2 Karma::KarmaGui::GetWindowContentRegionMax()
{
	KGGuiWindow* window = GKarmaGui->CurrentWindow;
	return window->ContentRegionRect.Max - window->Pos;
}

// Lock horizontal starting position + capture group bounding box into one "item" (so you can use IsItemHovered() or layout primitives such as SameLine() on whole group, etc.)
// Groups are currently a mishmash of functionalities which should perhaps be clarified and separated.
// FIXME-OPT: Could we safely early out on ->SkipItems?
void Karma::KarmaGui::BeginGroup()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;

	g.GroupStack.resize(g.GroupStack.Size + 1);
	KGGuiGroupData& group_data = g.GroupStack.back();
	group_data.WindowID = window->ID;
	group_data.BackupCursorPos = window->DC.CursorPos;
	group_data.BackupCursorMaxPos = window->DC.CursorMaxPos;
	group_data.BackupIndent = window->DC.Indent;
	group_data.BackupGroupOffset = window->DC.GroupOffset;
	group_data.BackupCurrLineSize = window->DC.CurrLineSize;
	group_data.BackupCurrLineTextBaseOffset = window->DC.CurrLineTextBaseOffset;
	group_data.BackupActiveIdIsAlive = g.ActiveIdIsAlive;
	group_data.BackupHoveredIdIsAlive = g.HoveredId != 0;
	group_data.BackupActiveIdPreviousFrameIsAlive = g.ActiveIdPreviousFrameIsAlive;
	group_data.EmitItem = true;

	window->DC.GroupOffset.x = window->DC.CursorPos.x - window->Pos.x - window->DC.ColumnsOffset.x;
	window->DC.Indent = window->DC.GroupOffset;
	window->DC.CursorMaxPos = window->DC.CursorPos;
	window->DC.CurrLineSize = KGVec2(0.0f, 0.0f);
	if (g.LogEnabled)
		g.LogLinePosY = -FLT_MAX; // To enforce a carriage return
}

void Karma::KarmaGui::EndGroup()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	KR_CORE_ASSERT(g.GroupStack.Size > 0, ""); // Mismatched BeginGroup()/EndGroup() calls

	KGGuiGroupData& group_data = g.GroupStack.back();
	KR_CORE_ASSERT(group_data.WindowID == window->ID, ""); // EndGroup() in wrong window?

	if (window->DC.IsSetPos)
		KarmaGuiInternal::ErrorCheckUsingSetCursorPosToExtendParentBoundaries();

	KGRect group_bb(group_data.BackupCursorPos, KGMax(window->DC.CursorMaxPos, group_data.BackupCursorPos));

	window->DC.CursorPos = group_data.BackupCursorPos;
	window->DC.CursorMaxPos = KGMax(group_data.BackupCursorMaxPos, window->DC.CursorMaxPos);
	window->DC.Indent = group_data.BackupIndent;
	window->DC.GroupOffset = group_data.BackupGroupOffset;
	window->DC.CurrLineSize = group_data.BackupCurrLineSize;
	window->DC.CurrLineTextBaseOffset = group_data.BackupCurrLineTextBaseOffset;
	if (g.LogEnabled)
		g.LogLinePosY = -FLT_MAX; // To enforce a carriage return

	if (!group_data.EmitItem)
	{
		g.GroupStack.pop_back();
		return;
	}

	window->DC.CurrLineTextBaseOffset = KGMax(window->DC.PrevLineTextBaseOffset, group_data.BackupCurrLineTextBaseOffset);      // FIXME: Incorrect, we should grab the base offset from the *first line* of the group but it is hard to obtain now.
	KarmaGuiInternal::ItemSize(group_bb.GetSize());
	KarmaGuiInternal::ItemAdd(group_bb, 0, NULL, KGGuiItemFlags_NoTabStop);

	// If the current ActiveId was declared within the boundary of our group, we copy it to LastItemId so IsItemActive(), IsItemDeactivated() etc. will be functional on the entire group.
	// It would be neater if we replaced window.DC.LastItemId by e.g. 'bool LastItemIsActive', but would put a little more burden on individual widgets.
	// Also if you grep for LastItemId you'll notice it is only used in that context.
	// (The two tests not the same because ActiveIdIsAlive is an ID itself, in order to be able to handle ActiveId being overwritten during the frame.)
	const bool group_contains_curr_active_id = (group_data.BackupActiveIdIsAlive != g.ActiveId) && (g.ActiveIdIsAlive == g.ActiveId) && g.ActiveId;
	const bool group_contains_prev_active_id = (group_data.BackupActiveIdPreviousFrameIsAlive == false) && (g.ActiveIdPreviousFrameIsAlive == true);
	if (group_contains_curr_active_id)
		g.LastItemData.ID = g.ActiveId;
	else if (group_contains_prev_active_id)
		g.LastItemData.ID = g.ActiveIdPreviousFrame;
	g.LastItemData.Rect = group_bb;

	// Forward Hovered flag
	const bool group_contains_curr_hovered_id = (group_data.BackupHoveredIdIsAlive == false) && g.HoveredId != 0;
	if (group_contains_curr_hovered_id)
		g.LastItemData.StatusFlags |= KGGuiItemStatusFlags_HoveredWindow;

	// Forward Edited flag
	if (group_contains_curr_active_id && g.ActiveIdHasBeenEditedThisFrame)
		g.LastItemData.StatusFlags |= KGGuiItemStatusFlags_Edited;

	// Forward Deactivated flag
	g.LastItemData.StatusFlags |= KGGuiItemStatusFlags_HasDeactivated;
	if (group_contains_prev_active_id && g.ActiveId != g.ActiveIdPreviousFrame)
		g.LastItemData.StatusFlags |= KGGuiItemStatusFlags_Deactivated;

	g.GroupStack.pop_back();
	//window->DrawList->AddRect(group_bb.Min, group_bb.Max, KG_COL32(255,0,255,255));   // [Debug]
}

//-----------------------------------------------------------------------------
// [SECTION] SCROLLING
//-----------------------------------------------------------------------------

// Helper to snap on edges when aiming at an item very close to the edge,
// So the difference between WindowPadding and ItemSpacing will be in the visible area after scrolling.
// When we refactor the scrolling API this may be configurable with a flag?
// Note that the effect for this won't be visible on X axis with default Style settings as WindowPadding.x == ItemSpacing.x by default.
static float CalcScrollEdgeSnap(float target, float snap_min, float snap_max, float snap_threshold, float center_ratio)
{
	if (target <= snap_min + snap_threshold)
		return KGLerp(snap_min, target, center_ratio);
	if (target >= snap_max - snap_threshold)
		return KGLerp(target, snap_max, center_ratio);
	return target;
}

KGVec2 Karma::KarmaGuiInternal::CalcNextScrollFromScrollTargetAndClamp(KGGuiWindow* window)
{
	KGVec2 scroll = window->Scroll;
	KGVec2 decoration_size(window->DecoOuterSizeX1 + window->DecoInnerSizeX1 + window->DecoOuterSizeX2, window->DecoOuterSizeY1 + window->DecoInnerSizeY1 + window->DecoOuterSizeY2);
	for (int axis = 0; axis < 2; axis++)
	{
		if (window->ScrollTarget[axis] < FLT_MAX)
		{
			float center_ratio = window->ScrollTargetCenterRatio[axis];
			float scroll_target = window->ScrollTarget[axis];
			if (window->ScrollTargetEdgeSnapDist[axis] > 0.0f)
			{
				float snap_min = 0.0f;
				float snap_max = window->ScrollMax[axis] + window->SizeFull[axis] - decoration_size[axis];
				scroll_target = CalcScrollEdgeSnap(scroll_target, snap_min, snap_max, window->ScrollTargetEdgeSnapDist[axis], center_ratio);
			}
			scroll[axis] = scroll_target - center_ratio * (window->SizeFull[axis] - decoration_size[axis]);
		}
		scroll[axis] = KG_FLOOR(KGMax(scroll[axis], 0.0f));
		if (!window->Collapsed && !window->SkipItems)
			scroll[axis] = KGMin(scroll[axis], window->ScrollMax[axis]);
	}
	return scroll;
}

void Karma::KarmaGuiInternal::ScrollToItem(KGGuiScrollFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	ScrollToRectEx(window, g.LastItemData.NavRect, flags);
}

void Karma::KarmaGuiInternal::ScrollToRect(KGGuiWindow* window, const KGRect& item_rect, KGGuiScrollFlags flags)
{
	ScrollToRectEx(window, item_rect, flags);
}

// Scroll to keep newly navigated item fully into view
KGVec2 Karma::KarmaGuiInternal::ScrollToRectEx(KGGuiWindow* window, const KGRect& item_rect, KGGuiScrollFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGRect scroll_rect(window->InnerRect.Min - KGVec2(1, 1), window->InnerRect.Max + KGVec2(1, 1));
	scroll_rect.Min.x = KGMin(scroll_rect.Min.x + window->DecoInnerSizeX1, scroll_rect.Max.x);
	scroll_rect.Min.y = KGMin(scroll_rect.Min.y + window->DecoInnerSizeY1, scroll_rect.Max.y);
	//GetForegroundDrawList(window)->AddRect(item_rect.Min, item_rect.Max, KG_COL32(255,0,0,255), 0.0f, 0, 5.0f); // [DEBUG]
	//GetForegroundDrawList(window)->AddRect(scroll_rect.Min, scroll_rect.Max, KG_COL32_WHITE); // [DEBUG]

	// Check that only one behavior is selected per axis
	KR_CORE_ASSERT((flags & KGGuiScrollFlags_MaskX_) == 0 || KGIsPowerOfTwo(flags & KGGuiScrollFlags_MaskX_), "");
	KR_CORE_ASSERT((flags & KGGuiScrollFlags_MaskY_) == 0 || KGIsPowerOfTwo(flags & KGGuiScrollFlags_MaskY_), "");

	// Defaults
	KGGuiScrollFlags in_flags = flags;
	if ((flags & KGGuiScrollFlags_MaskX_) == 0 && window->ScrollbarX)
		flags |= KGGuiScrollFlags_KeepVisibleEdgeX;
	if ((flags & KGGuiScrollFlags_MaskY_) == 0)
		flags |= window->Appearing ? KGGuiScrollFlags_AlwaysCenterY : KGGuiScrollFlags_KeepVisibleEdgeY;

	const bool fully_visible_x = item_rect.Min.x >= scroll_rect.Min.x && item_rect.Max.x <= scroll_rect.Max.x;
	const bool fully_visible_y = item_rect.Min.y >= scroll_rect.Min.y && item_rect.Max.y <= scroll_rect.Max.y;
	const bool can_be_fully_visible_x = (item_rect.GetWidth() + g.Style.ItemSpacing.x * 2.0f) <= scroll_rect.GetWidth() || (window->AutoFitFramesX > 0) || (window->Flags & KGGuiWindowFlags_AlwaysAutoResize) != 0;
	const bool can_be_fully_visible_y = (item_rect.GetHeight() + g.Style.ItemSpacing.y * 2.0f) <= scroll_rect.GetHeight() || (window->AutoFitFramesY > 0) || (window->Flags & KGGuiWindowFlags_AlwaysAutoResize) != 0;

	if ((flags & KGGuiScrollFlags_KeepVisibleEdgeX) && !fully_visible_x)
	{
		if (item_rect.Min.x < scroll_rect.Min.x || !can_be_fully_visible_x)
			SetScrollFromPosX(window, item_rect.Min.x - g.Style.ItemSpacing.x - window->Pos.x, 0.0f);
		else if (item_rect.Max.x >= scroll_rect.Max.x)
			SetScrollFromPosX(window, item_rect.Max.x + g.Style.ItemSpacing.x - window->Pos.x, 1.0f);
	}
	else if (((flags & KGGuiScrollFlags_KeepVisibleCenterX) && !fully_visible_x) || (flags & KGGuiScrollFlags_AlwaysCenterX))
	{
		if (can_be_fully_visible_x)
			SetScrollFromPosX(window, KGFloor((item_rect.Min.x + item_rect.Max.y) * 0.5f) - window->Pos.x, 0.5f);
		else
			SetScrollFromPosX(window, item_rect.Min.x - window->Pos.x, 0.0f);
	}

	if ((flags & KGGuiScrollFlags_KeepVisibleEdgeY) && !fully_visible_y)
	{
		if (item_rect.Min.y < scroll_rect.Min.y || !can_be_fully_visible_y)
			SetScrollFromPosY(window, item_rect.Min.y - g.Style.ItemSpacing.y - window->Pos.y, 0.0f);
		else if (item_rect.Max.y >= scroll_rect.Max.y)
			SetScrollFromPosY(window, item_rect.Max.y + g.Style.ItemSpacing.y - window->Pos.y, 1.0f);
	}
	else if (((flags & KGGuiScrollFlags_KeepVisibleCenterY) && !fully_visible_y) || (flags & KGGuiScrollFlags_AlwaysCenterY))
	{
		if (can_be_fully_visible_y)
			SetScrollFromPosY(window, KGFloor((item_rect.Min.y + item_rect.Max.y) * 0.5f) - window->Pos.y, 0.5f);
		else
			SetScrollFromPosY(window, item_rect.Min.y - window->Pos.y, 0.0f);
	}

	KGVec2 next_scroll = CalcNextScrollFromScrollTargetAndClamp(window);
	KGVec2 delta_scroll = next_scroll - window->Scroll;

	// Also scroll parent window to keep us into view if necessary
	if (!(flags & KGGuiScrollFlags_NoScrollParent) && (window->Flags & KGGuiWindowFlags_ChildWindow))
	{
		// FIXME-SCROLL: May be an option?
		if ((in_flags & (KGGuiScrollFlags_AlwaysCenterX | KGGuiScrollFlags_KeepVisibleCenterX)) != 0)
			in_flags = (in_flags & ~KGGuiScrollFlags_MaskX_) | KGGuiScrollFlags_KeepVisibleEdgeX;
		if ((in_flags & (KGGuiScrollFlags_AlwaysCenterY | KGGuiScrollFlags_KeepVisibleCenterY)) != 0)
			in_flags = (in_flags & ~KGGuiScrollFlags_MaskY_) | KGGuiScrollFlags_KeepVisibleEdgeY;
		delta_scroll += ScrollToRectEx(window->ParentWindow, KGRect(item_rect.Min - delta_scroll, item_rect.Max - delta_scroll), in_flags);
	}

	return delta_scroll;
}

float Karma::KarmaGui::GetScrollX()
{
	KGGuiWindow* window = GKarmaGui->CurrentWindow;
	return window->Scroll.x;
}

float Karma::KarmaGui::GetScrollY()
{
	KGGuiWindow* window = GKarmaGui->CurrentWindow;
	return window->Scroll.y;
}

float Karma::KarmaGui::GetScrollMaxX()
{
	KGGuiWindow* window = GKarmaGui->CurrentWindow;
	return window->ScrollMax.x;
}

float Karma::KarmaGui::GetScrollMaxY()
{
	KGGuiWindow* window = GKarmaGui->CurrentWindow;
	return window->ScrollMax.y;
}

void Karma::KarmaGuiInternal::SetScrollX(KGGuiWindow* window, float scroll_x)
{
	window->ScrollTarget.x = scroll_x;
	window->ScrollTargetCenterRatio.x = 0.0f;
	window->ScrollTargetEdgeSnapDist.x = 0.0f;
}

void Karma::KarmaGuiInternal::SetScrollY(KGGuiWindow* window, float scroll_y)
{
	window->ScrollTarget.y = scroll_y;
	window->ScrollTargetCenterRatio.y = 0.0f;
	window->ScrollTargetEdgeSnapDist.y = 0.0f;
}

void Karma::KarmaGui::SetScrollX(float scroll_x)
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiInternal::SetScrollX(g.CurrentWindow, scroll_x);
}

void Karma::KarmaGui::SetScrollY(float scroll_y)
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiInternal::SetScrollY(g.CurrentWindow, scroll_y);
}

// Note that a local position will vary depending on initial scroll value,
// This is a little bit confusing so bear with us:
//  - local_pos = (absolution_pos - window->Pos)
//  - So local_x/local_y are 0.0f for a position at the upper-left corner of a window,
//    and generally local_x/local_y are >(padding+decoration) && <(size-padding-decoration) when in the visible area.
//  - They mostly exist because of legacy API.
// Following the rules above, when trying to work with scrolling code, consider that:
//  - SetScrollFromPosY(0.0f) == SetScrollY(0.0f + scroll.y) == has no effect!
//  - SetScrollFromPosY(-scroll.y) == SetScrollY(-scroll.y + scroll.y) == SetScrollY(0.0f) == reset scroll. Of course writing SetScrollY(0.0f) directly then makes more sense
// We store a target position so centering and clamping can occur on the next frame when we are guaranteed to have a known window size
void Karma::KarmaGuiInternal::SetScrollFromPosX(KGGuiWindow* window, float local_x, float center_x_ratio)
{
	KR_CORE_ASSERT(center_x_ratio >= 0.0f && center_x_ratio <= 1.0f, "");
	window->ScrollTarget.x = KG_FLOOR(local_x - window->DecoOuterSizeX1 - window->DecoInnerSizeX1 + window->Scroll.x); // Convert local position to scroll offset
	window->ScrollTargetCenterRatio.x = center_x_ratio;
	window->ScrollTargetEdgeSnapDist.x = 0.0f;
}

void Karma::KarmaGuiInternal::SetScrollFromPosY(KGGuiWindow* window, float local_y, float center_y_ratio)
{
	KR_CORE_ASSERT(center_y_ratio >= 0.0f && center_y_ratio <= 1.0f, "");
	window->ScrollTarget.y = KG_FLOOR(local_y - window->DecoOuterSizeY1 - window->DecoInnerSizeY1 + window->Scroll.y); // Convert local position to scroll offset
	window->ScrollTargetCenterRatio.y = center_y_ratio;
	window->ScrollTargetEdgeSnapDist.y = 0.0f;
}

void Karma::KarmaGui::SetScrollFromPosX(float local_x, float center_x_ratio)
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiInternal::SetScrollFromPosX(g.CurrentWindow, local_x, center_x_ratio);
}

void Karma::KarmaGui::SetScrollFromPosY(float local_y, float center_y_ratio)
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiInternal::SetScrollFromPosY(g.CurrentWindow, local_y, center_y_ratio);
}

// center_x_ratio: 0.0f left of last item, 0.5f horizontal center of last item, 1.0f right of last item.
void Karma::KarmaGui::SetScrollHereX(float center_x_ratio)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	float spacing_x = KGMax(window->WindowPadding.x, g.Style.ItemSpacing.x);
	float target_pos_x = KGLerp(g.LastItemData.Rect.Min.x - spacing_x, g.LastItemData.Rect.Max.x + spacing_x, center_x_ratio);
	KarmaGuiInternal::SetScrollFromPosX(window, target_pos_x - window->Pos.x, center_x_ratio); // Convert from absolute to local pos

	// Tweak: snap on edges when aiming at an item very close to the edge
	window->ScrollTargetEdgeSnapDist.x = KGMax(0.0f, window->WindowPadding.x - spacing_x);
}

// center_y_ratio: 0.0f top of last item, 0.5f vertical center of last item, 1.0f bottom of last item.
void Karma::KarmaGui::SetScrollHereY(float center_y_ratio)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	float spacing_y = KGMax(window->WindowPadding.y, g.Style.ItemSpacing.y);
	float target_pos_y = KGLerp(window->DC.CursorPosPrevLine.y - spacing_y, window->DC.CursorPosPrevLine.y + window->DC.PrevLineSize.y + spacing_y, center_y_ratio);
	KarmaGuiInternal::SetScrollFromPosY(window, target_pos_y - window->Pos.y, center_y_ratio); // Convert from absolute to local pos

	// Tweak: snap on edges when aiming at an item very close to the edge
	window->ScrollTargetEdgeSnapDist.y = KGMax(0.0f, window->WindowPadding.y - spacing_y);
}

//-----------------------------------------------------------------------------
// [SECTION] TOOLTIPS
//-----------------------------------------------------------------------------

void Karma::KarmaGui::BeginTooltip()
{
	KarmaGuiInternal::BeginTooltipEx(KGGuiTooltipFlags_None, KGGuiWindowFlags_None);
}

void Karma::KarmaGuiInternal::BeginTooltipEx(KGGuiTooltipFlags tooltip_flags, KarmaGuiWindowFlags extra_window_flags)
{
	KarmaGuiContext& g = *GKarmaGui;

	if (g.DragDropWithinSource || g.DragDropWithinTarget)
	{
		// The default tooltip position is a little offset to give space to see the context menu (it's also clamped within the current viewport/monitor)
		// In the context of a dragging tooltip we try to reduce that offset and we enforce following the cursor.
		// Whatever we do we want to call SetNextWindowPos() to enforce a tooltip position and disable clipping the tooltip without our display area, like regular tooltip do.
		//KGVec2 tooltip_pos = g.IO.MousePos - g.ActiveIdClickOffset - g.Style.WindowPadding;
		KGVec2 tooltip_pos = g.IO.MousePos + KGVec2(16 * g.Style.MouseCursorScale, 8 * g.Style.MouseCursorScale);
		KarmaGui::SetNextWindowPos(tooltip_pos);
		KarmaGui::SetNextWindowBgAlpha(g.Style.Colors[KGGuiCol_PopupBg].w * 0.60f);
		//PushStyleVar(KGGuiStyleVar_Alpha, g.Style.Alpha * 0.60f); // This would be nice but e.g ColorButton with checkboard has issue with transparent colors :(
		tooltip_flags |= KGGuiTooltipFlags_OverridePreviousTooltip;
	}

	char window_name[16];
	KGFormatString(window_name, KG_ARRAYSIZE(window_name), "##Tooltip_%02d", g.TooltipOverrideCount);
	if (tooltip_flags & KGGuiTooltipFlags_OverridePreviousTooltip)
		if (KGGuiWindow* window = FindWindowByName(window_name))
			if (window->Active)
			{
				// Hide previous tooltip from being displayed. We can't easily "reset" the content of a window so we create a new one.
				window->Hidden = true;
				window->HiddenFramesCanSkipItems = 1; // FIXME: This may not be necessary?
				KGFormatString(window_name, KG_ARRAYSIZE(window_name), "##Tooltip_%02d", ++g.TooltipOverrideCount);
			}
	KarmaGuiWindowFlags flags = KGGuiWindowFlags_Tooltip | KGGuiWindowFlags_NoInputs | KGGuiWindowFlags_NoTitleBar | KGGuiWindowFlags_NoMove | KGGuiWindowFlags_NoResize | KGGuiWindowFlags_NoSavedSettings | KGGuiWindowFlags_AlwaysAutoResize | KGGuiWindowFlags_NoDocking;
	KarmaGui::Begin(window_name, NULL, flags | extra_window_flags);
}

void Karma::KarmaGui::EndTooltip()
{
	KR_CORE_ASSERT(KarmaGuiInternal::GetCurrentWindowRead()->Flags & KGGuiWindowFlags_Tooltip, "");   // Mismatched BeginTooltip()/EndTooltip() calls
	End();
}

void Karma::KarmaGui::SetTooltipV(const char* fmt, va_list args)
{
	KarmaGuiInternal::BeginTooltipEx(KGGuiTooltipFlags_OverridePreviousTooltip, KGGuiWindowFlags_None);
	TextV(fmt, args);
	EndTooltip();
}

void Karma::KarmaGui::SetTooltip(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	SetTooltipV(fmt, args);
	va_end(args);
}

//-----------------------------------------------------------------------------
// [SECTION] POPUPS
//-----------------------------------------------------------------------------

// Supported flags: KGGuiPopupFlags_AnyPopupId, KGGuiPopupFlags_AnyPopupLevel
bool Karma::KarmaGuiInternal::IsPopupOpen(KGGuiID id, KarmaGuiPopupFlags popup_flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (popup_flags & KGGuiPopupFlags_AnyPopupId)
	{
		// Return true if any popup is open at the current BeginPopup() level of the popup stack
		// This may be used to e.g. test for another popups already opened to handle popups priorities at the same level.
		KR_CORE_ASSERT(id == 0, "");
		if (popup_flags & KGGuiPopupFlags_AnyPopupLevel)
			return g.OpenPopupStack.Size > 0;
		else
			return g.OpenPopupStack.Size > g.BeginPopupStack.Size;
	}
	else
	{
		if (popup_flags & KGGuiPopupFlags_AnyPopupLevel)
		{
			// Return true if the popup is open anywhere in the popup stack
			for (int n = 0; n < g.OpenPopupStack.Size; n++)
				if (g.OpenPopupStack[n].PopupId == id)
					return true;
			return false;
		}
		else
		{
			// Return true if the popup is open at the current BeginPopup() level of the popup stack (this is the most-common query)
			return g.OpenPopupStack.Size > g.BeginPopupStack.Size && g.OpenPopupStack[g.BeginPopupStack.Size].PopupId == id;
		}
	}
}

bool Karma::KarmaGui::IsPopupOpen(const char* str_id, KarmaGuiPopupFlags popup_flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiID id = (popup_flags & KGGuiPopupFlags_AnyPopupId) ? 0 : g.CurrentWindow->GetID(str_id);
	if ((popup_flags & KGGuiPopupFlags_AnyPopupLevel) && id != 0)
	{
		KR_CORE_ASSERT(0, "Cannot use IsPopupOpen() with a string id and KGGuiPopupFlags_AnyPopupLevel."); // But non-string version is legal and used internally
	}
	return KarmaGuiInternal::IsPopupOpen(id, popup_flags);
}

KGGuiWindow* Karma::KarmaGuiInternal::GetTopMostPopupModal()
{
	KarmaGuiContext& g = *GKarmaGui;
	for (int n = g.OpenPopupStack.Size - 1; n >= 0; n--)
		if (KGGuiWindow* popup = g.OpenPopupStack.Data[n].Window)
			if (popup->Flags & KGGuiWindowFlags_Modal)
				return popup;
	return NULL;
}

KGGuiWindow* Karma::KarmaGuiInternal::GetTopMostAndVisiblePopupModal()
{
	KarmaGuiContext& g = *GKarmaGui;
	for (int n = g.OpenPopupStack.Size - 1; n >= 0; n--)
		if (KGGuiWindow* popup = g.OpenPopupStack.Data[n].Window)
			if ((popup->Flags & KGGuiWindowFlags_Modal) && IsWindowActiveAndVisible(popup))
				return popup;
	return NULL;
}

void Karma::KarmaGui::OpenPopup(const char* str_id, KarmaGuiPopupFlags popup_flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiID id = g.CurrentWindow->GetID(str_id);
	KR_CORE_INFO("[popup] OpenPopup(\"{0}\" -> {1}", str_id, id);
	KarmaGuiInternal::OpenPopupEx(id, popup_flags);
}

void Karma::KarmaGui::OpenPopup(KGGuiID id, KarmaGuiPopupFlags popup_flags)
{
	KarmaGuiInternal::OpenPopupEx(id, popup_flags);
}

// Mark popup as open (toggle toward open state).
// Popups are closed when user click outside, or activate a pressable item, or CloseCurrentPopup() is called within a BeginPopup()/EndPopup() block.
// Popup identifiers are relative to the current ID-stack (so OpenPopup and BeginPopup needs to be at the same level).
// One open popup per level of the popup hierarchy (NB: when assigning we reset the Window member of ImGuiPopupRef to NULL)
void Karma::KarmaGuiInternal::OpenPopupEx(KGGuiID id, KarmaGuiPopupFlags popup_flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* parent_window = g.CurrentWindow;
	const int current_stack_size = g.BeginPopupStack.Size;

	if (popup_flags & KGGuiPopupFlags_NoOpenOverExistingPopup)
		if (IsPopupOpen(0u, KGGuiPopupFlags_AnyPopupId))
			return;

	KGGuiPopupData popup_ref; // Tagged as new ref as Window will be set back to NULL if we write this into OpenPopupStack.
	popup_ref.PopupId = id;
	popup_ref.Window = NULL;
	popup_ref.BackupNavWindow = g.NavWindow;            // When popup closes focus may be restored to NavWindow (depend on window type).
	popup_ref.OpenFrameCount = g.FrameCount;
	popup_ref.OpenParentId = parent_window->IDStack.back();
	popup_ref.OpenPopupPos = NavCalcPreferredRefPos();
	popup_ref.OpenMousePos = KarmaGui::IsMousePosValid(&g.IO.MousePos) ? g.IO.MousePos : popup_ref.OpenPopupPos;

	KR_CORE_INFO("[popup] OpenPopupEx({0})", id);
	if (g.OpenPopupStack.Size < current_stack_size + 1)
	{
		g.OpenPopupStack.push_back(popup_ref);
	}
	else
	{
		// Gently handle the user mistakenly calling OpenPopup() every frame. It is a programming mistake! However, if we were to run the regular code path, the ui
		// would become completely unusable because the popup will always be in hidden-while-calculating-size state _while_ claiming focus. Which would be a very confusing
		// situation for the programmer. Instead, we silently allow the popup to proceed, it will keep reappearing and the programming error will be more obvious to understand.
		if (g.OpenPopupStack[current_stack_size].PopupId == id && g.OpenPopupStack[current_stack_size].OpenFrameCount == g.FrameCount - 1)
		{
			g.OpenPopupStack[current_stack_size].OpenFrameCount = popup_ref.OpenFrameCount;
		}
		else
		{
			// Close child popups if any, then flag popup for open/reopen
			ClosePopupToLevel(current_stack_size, false);
			g.OpenPopupStack.push_back(popup_ref);
		}

		// When reopening a popup we first refocus its parent, otherwise if its parent is itself a popup it would get closed by ClosePopupsOverWindow().
		// This is equivalent to what ClosePopupToLevel() does.
		//if (g.OpenPopupStack[current_stack_size].PopupId == id)
		//    FocusWindow(parent_window);
	}
}

// When popups are stacked, clicking on a lower level popups puts focus back to it and close popups above it.
// This function closes any popups that are over 'ref_window'.
void Karma::KarmaGuiInternal::ClosePopupsOverWindow(KGGuiWindow* ref_window, bool restore_focus_to_window_under_popup)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.OpenPopupStack.Size == 0)
		return;

	// Don't close our own child popup windows.
	int popup_count_to_keep = 0;
	if (ref_window)
	{
		// Find the highest popup which is a descendant of the reference window (generally reference window = NavWindow)
		for (; popup_count_to_keep < g.OpenPopupStack.Size; popup_count_to_keep++)
		{
			KGGuiPopupData& popup = g.OpenPopupStack[popup_count_to_keep];
			if (!popup.Window)
				continue;
			KR_CORE_ASSERT((popup.Window->Flags & KGGuiWindowFlags_Popup) != 0, "");
			if (popup.Window->Flags & KGGuiWindowFlags_ChildWindow)
				continue;

			// Trim the stack unless the popup is a direct parent of the reference window (the reference window is often the NavWindow)
			// - With this stack of window, clicking/focusing Popup1 will close Popup2 and Popup3:
			//     Window -> Popup1 -> Popup2 -> Popup3
			// - Each popups may contain child windows, which is why we compare ->RootWindowDockTree!
			//     Window -> Popup1 -> Popup1_Child -> Popup2 -> Popup2_Child
			bool ref_window_is_descendent_of_popup = false;
			for (int n = popup_count_to_keep; n < g.OpenPopupStack.Size; n++)
				if (KGGuiWindow* popup_window = g.OpenPopupStack[n].Window)
					//if (popup_window->RootWindowDockTree == ref_window->RootWindowDockTree) // FIXME-MERGE
					if (IsWindowWithinBeginStackOf(ref_window, popup_window))
					{
						ref_window_is_descendent_of_popup = true;
						break;
					}
			if (!ref_window_is_descendent_of_popup)
				break;
		}
	}
	if (popup_count_to_keep < g.OpenPopupStack.Size) // This test is not required but it allows to set a convenient breakpoint on the statement below
	{
		KR_CORE_INFO("[popup] ClosePopupsOverWindow(\"{0}\")", ref_window ? ref_window->Name : "<NULL>");
		ClosePopupToLevel(popup_count_to_keep, restore_focus_to_window_under_popup);
	}
}

void Karma::KarmaGuiInternal::ClosePopupsExceptModals()
{
	KarmaGuiContext& g = *GKarmaGui;

	int popup_count_to_keep;
	for (popup_count_to_keep = g.OpenPopupStack.Size; popup_count_to_keep > 0; popup_count_to_keep--)
	{
		KGGuiWindow* window = g.OpenPopupStack[popup_count_to_keep - 1].Window;
		if (!window || window->Flags & KGGuiWindowFlags_Modal)
			break;
	}
	if (popup_count_to_keep < g.OpenPopupStack.Size) // This test is not required but it allows to set a convenient breakpoint on the statement below
		ClosePopupToLevel(popup_count_to_keep, true);
}

void Karma::KarmaGuiInternal::ClosePopupToLevel(int remaining, bool restore_focus_to_window_under_popup)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_INFO("[popup] ClosePopupToLevel({0}), restore_focus_to_window_under_popup={1}", remaining, restore_focus_to_window_under_popup);
	KR_CORE_ASSERT(remaining >= 0 && remaining < g.OpenPopupStack.Size, "");

	// Trim open popup stack
	KGGuiWindow* popup_window = g.OpenPopupStack[remaining].Window;
	KGGuiWindow* popup_backup_nav_window = g.OpenPopupStack[remaining].BackupNavWindow;
	g.OpenPopupStack.resize(remaining);

	if (restore_focus_to_window_under_popup)
	{
		KGGuiWindow* focus_window = (popup_window && popup_window->Flags & KGGuiWindowFlags_ChildMenu) ? popup_window->ParentWindow : popup_backup_nav_window;
		if (focus_window && !focus_window->WasActive && popup_window)
		{
			// Fallback
			FocusTopMostWindowUnderOne(popup_window, NULL);
		}
		else
		{
			if (g.NavLayer == KGGuiNavLayer_Main && focus_window)
				focus_window = NavRestoreLastChildNavWindow(focus_window);
			FocusWindow(focus_window);
		}
	}
}

// Close the popup we have begin-ed into.
void Karma::KarmaGui::CloseCurrentPopup()
{
	KarmaGuiContext& g = *GKarmaGui;
	int popup_idx = g.BeginPopupStack.Size - 1;
	if (popup_idx < 0 || popup_idx >= g.OpenPopupStack.Size || g.BeginPopupStack[popup_idx].PopupId != g.OpenPopupStack[popup_idx].PopupId)
		return;

	// Closing a menu closes its top-most parent popup (unless a modal)
	while (popup_idx > 0)
	{
		KGGuiWindow* popup_window = g.OpenPopupStack[popup_idx].Window;
		KGGuiWindow* parent_popup_window = g.OpenPopupStack[popup_idx - 1].Window;
		bool close_parent = false;
		if (popup_window && (popup_window->Flags & KGGuiWindowFlags_ChildMenu))
			if (parent_popup_window && !(parent_popup_window->Flags & KGGuiWindowFlags_MenuBar))
				close_parent = true;
		if (!close_parent)
			break;
		popup_idx--;
	}
	KR_CORE_INFO("[popup] CloseCurrentPopup {0} -> {1}", g.BeginPopupStack.Size - 1, popup_idx);
	KarmaGuiInternal::ClosePopupToLevel(popup_idx, true);

	// A common pattern is to close a popup when selecting a menu item/selectable that will open another window.
	// To improve this usage pattern, we avoid nav highlight for a single frame in the parent window.
	// Similarly, we could avoid mouse hover highlight in this window but it is less visually problematic.
	if (KGGuiWindow* window = g.NavWindow)
		window->DC.NavHideHighlightOneFrame = true;
}

// Attention! BeginPopup() adds default flags which BeginPopupEx()!
bool Karma::KarmaGuiInternal::BeginPopupEx(KGGuiID id, KarmaGuiWindowFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (!IsPopupOpen(id, KGGuiPopupFlags_None))
	{
		g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
		return false;
	}

	char name[20];
	if (flags & KGGuiWindowFlags_ChildMenu)
		KGFormatString(name, KG_ARRAYSIZE(name), "##Menu_%02d", g.BeginMenuCount); // Recycle windows based on depth
	else
		KGFormatString(name, KG_ARRAYSIZE(name), "##Popup_%08x", id); // Not recycling, so we can close/open during the same frame

	flags |= KGGuiWindowFlags_Popup | KGGuiWindowFlags_NoDocking;
	bool is_open = KarmaGui::Begin(name, NULL, flags);
	if (!is_open) // NB: Begin can return false when the popup is completely clipped (e.g. zero size display)
		KarmaGui::EndPopup();

	return is_open;
}

bool Karma::KarmaGui::BeginPopup(const char* str_id, KarmaGuiWindowFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.OpenPopupStack.Size <= g.BeginPopupStack.Size) // Early out for performance
	{
		g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
		return false;
	}
	flags |= KGGuiWindowFlags_AlwaysAutoResize | KGGuiWindowFlags_NoTitleBar | KGGuiWindowFlags_NoSavedSettings;
	KGGuiID id = g.CurrentWindow->GetID(str_id);
	return KarmaGuiInternal::BeginPopupEx(id, flags);
}

// If 'p_open' is specified for a modal popup window, the popup will have a regular close button which will close the popup.
// Note that popup visibility status is owned by Dear ImGui (and manipulated with e.g. OpenPopup) so the actual value of *p_open is meaningless here.
bool Karma::KarmaGui::BeginPopupModal(const char* name, bool* p_open, KarmaGuiWindowFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	const KGGuiID id = window->GetID(name);
	if (!KarmaGuiInternal::IsPopupOpen(id, KGGuiPopupFlags_None))
	{
		g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
		return false;
	}

	// Center modal windows by default for increased visibility
	// (this won't really last as settings will kick in, and is mostly for backward compatibility. user may do the same themselves)
	// FIXME: Should test for (PosCond & window->SetWindowPosAllowFlags) with the upcoming window.
	if ((g.NextWindowData.Flags & KGGuiNextWindowDataFlags_HasPos) == 0)
	{
		const KarmaGuiViewport* viewport = window->WasActive ? window->Viewport : GetMainViewport(); // FIXME-VIEWPORT: What may be our reference viewport?
		SetNextWindowPos(viewport->GetCenter(), KGGuiCond_FirstUseEver, KGVec2(0.5f, 0.5f));
	}

	flags |= KGGuiWindowFlags_Popup | KGGuiWindowFlags_Modal | KGGuiWindowFlags_NoCollapse | KGGuiWindowFlags_NoDocking;
	const bool is_open = Begin(name, p_open, flags);
	if (!is_open || (p_open && !*p_open)) // NB: is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
	{
		EndPopup();
		if (is_open)
			KarmaGuiInternal::ClosePopupToLevel(g.BeginPopupStack.Size, true);
		return false;
	}
	return is_open;
}

void Karma::KarmaGui::EndPopup()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	KR_CORE_ASSERT(window->Flags & KGGuiWindowFlags_Popup, "");  // Mismatched BeginPopup()/EndPopup() calls
	KR_CORE_ASSERT(g.BeginPopupStack.Size > 0, "");

	// Make all menus and popups wrap around for now, may need to expose that policy (e.g. focus scope could include wrap/loop policy flags used by new move requests)
	if (g.NavWindow == window)
		KarmaGuiInternal::NavMoveRequestTryWrapping(window, KGGuiNavMoveFlags_LoopY);

	// Child-popups don't need to be laid out
	KR_CORE_ASSERT(g.WithinEndChild == false, "");
	if (window->Flags & KGGuiWindowFlags_ChildWindow)
		g.WithinEndChild = true;
	End();
	g.WithinEndChild = false;
}

// Helper to open a popup if mouse button is released over the item
// - This is essentially the same as BeginPopupContextItem() but without the trailing BeginPopup()
void Karma::KarmaGui::OpenPopupOnItemClick(const char* str_id, KarmaGuiPopupFlags popup_flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	int mouse_button = (popup_flags & KGGuiPopupFlags_MouseButtonMask_);
	if (IsMouseReleased(mouse_button) && IsItemHovered(KGGuiHoveredFlags_AllowWhenBlockedByPopup))
	{
		KGGuiID id = str_id ? window->GetID(str_id) : g.LastItemData.ID;    // If user hasn't passed an ID, we can use the LastItemID. Using LastItemID as a Popup ID won't conflict!
		KR_CORE_ASSERT(id != 0, "");                                             // You cannot pass a NULL str_id if the last item has no identifier (e.g. a Text() item)
		KarmaGuiInternal::OpenPopupEx(id, popup_flags);
	}
}

// This is a helper to handle the simplest case of associating one named popup to one given widget.
// - To create a popup associated to the last item, you generally want to pass a NULL value to str_id.
// - To create a popup with a specific identifier, pass it in str_id.
//    - This is useful when using using BeginPopupContextItem() on an item which doesn't have an identifier, e.g. a Text() call.
//    - This is useful when multiple code locations may want to manipulate/open the same popup, given an explicit id.
// - You may want to handle the whole on user side if you have specific needs (e.g. tweaking IsItemHovered() parameters).
//   This is essentially the same as:
//       id = str_id ? GetID(str_id) : GetItemID();
//       OpenPopupOnItemClick(str_id, KGGuiPopupFlags_MouseButtonRight);
//       return BeginPopup(id);
//   Which is essentially the same as:
//       id = str_id ? GetID(str_id) : GetItemID();
//       if (IsItemHovered() && IsMouseReleased(KGGuiMouseButton_Right))
//           OpenPopup(id);
//       return BeginPopup(id);
//   The main difference being that this is tweaked to avoid computing the ID twice.
bool Karma::KarmaGui::BeginPopupContextItem(const char* str_id, KarmaGuiPopupFlags popup_flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	if (window->SkipItems)
		return false;
	KGGuiID id = str_id ? window->GetID(str_id) : g.LastItemData.ID;    // If user hasn't passed an ID, we can use the LastItemID. Using LastItemID as a Popup ID won't conflict!
	KR_CORE_ASSERT(id != 0, "");                                             // You cannot pass a NULL str_id if the last item has no identifier (e.g. a Text() item)
	int mouse_button = (popup_flags & KGGuiPopupFlags_MouseButtonMask_);
	if (IsMouseReleased(mouse_button) && IsItemHovered(KGGuiHoveredFlags_AllowWhenBlockedByPopup))
		KarmaGuiInternal::OpenPopupEx(id, popup_flags);
	return KarmaGuiInternal::BeginPopupEx(id, KGGuiWindowFlags_AlwaysAutoResize | KGGuiWindowFlags_NoTitleBar | KGGuiWindowFlags_NoSavedSettings);
}

bool Karma::KarmaGui::BeginPopupContextWindow(const char* str_id, KarmaGuiPopupFlags popup_flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	if (!str_id)
		str_id = "window_context";
	KGGuiID id = window->GetID(str_id);
	int mouse_button = (popup_flags & KGGuiPopupFlags_MouseButtonMask_);
	if (IsMouseReleased(mouse_button) && IsWindowHovered(KGGuiHoveredFlags_AllowWhenBlockedByPopup))
		if (!(popup_flags & KGGuiPopupFlags_NoOpenOverItems) || !IsAnyItemHovered())
			KarmaGuiInternal::OpenPopupEx(id, popup_flags);
	return KarmaGuiInternal::BeginPopupEx(id, KGGuiWindowFlags_AlwaysAutoResize | KGGuiWindowFlags_NoTitleBar | KGGuiWindowFlags_NoSavedSettings);
}

bool Karma::KarmaGui::BeginPopupContextVoid(const char* str_id, KarmaGuiPopupFlags popup_flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	if (!str_id)
		str_id = "void_context";
	KGGuiID id = window->GetID(str_id);
	int mouse_button = (popup_flags & KGGuiPopupFlags_MouseButtonMask_);
	if (IsMouseReleased(mouse_button) && !IsWindowHovered(KGGuiHoveredFlags_AnyWindow))
		if (KarmaGuiInternal::GetTopMostPopupModal() == NULL)
			KarmaGuiInternal::OpenPopupEx(id, popup_flags);
	return KarmaGuiInternal::BeginPopupEx(id, KGGuiWindowFlags_AlwaysAutoResize | KGGuiWindowFlags_NoTitleBar | KGGuiWindowFlags_NoSavedSettings);
}

// r_avoid = the rectangle to avoid (e.g. for tooltip it is a rectangle around the mouse cursor which we want to avoid. for popups it's a small point around the cursor.)
// r_outer = the visible area rectangle, minus safe area padding. If our popup size won't fit because of safe area padding we ignore it.
// (r_outer is usually equivalent to the viewport rectangle minus padding, but when multi-viewports are enabled and monitor
//  information are available, it may represent the entire platform monitor from the frame of reference of the current viewport.
//  this allows us to have tooltips/popups displayed out of the parent viewport.)
KGVec2 Karma::KarmaGuiInternal::FindBestWindowPosForPopupEx(const KGVec2& ref_pos, const KGVec2& size, KarmaGuiDir* last_dir, const KGRect& r_outer, const KGRect& r_avoid, KGGuiPopupPositionPolicy policy)
{
	KGVec2 base_pos_clamped = KGClamp(ref_pos, r_outer.Min, r_outer.Max - size);
	//GetForegroundDrawList()->AddRect(r_avoid.Min, r_avoid.Max, KG_COL32(255,0,0,255));
	//GetForegroundDrawList()->AddRect(r_outer.Min, r_outer.Max, KG_COL32(0,255,0,255));

	// Combo Box policy (we want a connecting edge)
	if (policy == KGGuiPopupPositionPolicy_ComboBox)
	{
		const KarmaGuiDir dir_prefered_order[KGGuiDir_COUNT] = { KGGuiDir_Down, KGGuiDir_Right, KGGuiDir_Left, KGGuiDir_Up };
		for (int n = (*last_dir != KGGuiDir_None) ? -1 : 0; n < KGGuiDir_COUNT; n++)
		{
			const KarmaGuiDir dir = (n == -1) ? *last_dir : dir_prefered_order[n];
			if (n != -1 && dir == *last_dir) // Already tried this direction?
				continue;
			KGVec2 pos;
			if (dir == KGGuiDir_Down)  pos = KGVec2(r_avoid.Min.x, r_avoid.Max.y);          // Below, Toward Right (default)
			if (dir == KGGuiDir_Right) pos = KGVec2(r_avoid.Min.x, r_avoid.Min.y - size.y); // Above, Toward Right
			if (dir == KGGuiDir_Left)  pos = KGVec2(r_avoid.Max.x - size.x, r_avoid.Max.y); // Below, Toward Left
			if (dir == KGGuiDir_Up)    pos = KGVec2(r_avoid.Max.x - size.x, r_avoid.Min.y - size.y); // Above, Toward Left
			if (!r_outer.Contains(KGRect(pos, pos + size)))
				continue;
			*last_dir = dir;
			return pos;
		}
	}

	// Tooltip and Default popup policy
	// (Always first try the direction we used on the last frame, if any)
	if (policy == KGGuiPopupPositionPolicy_Tooltip || policy == KGGuiPopupPositionPolicy_Default)
	{
		const KarmaGuiDir dir_prefered_order[KGGuiDir_COUNT] = { KGGuiDir_Right, KGGuiDir_Down, KGGuiDir_Up, KGGuiDir_Left };
		for (int n = (*last_dir != KGGuiDir_None) ? -1 : 0; n < KGGuiDir_COUNT; n++)
		{
			const KarmaGuiDir dir = (n == -1) ? *last_dir : dir_prefered_order[n];
			if (n != -1 && dir == *last_dir) // Already tried this direction?
				continue;

			const float avail_w = (dir == KGGuiDir_Left ? r_avoid.Min.x : r_outer.Max.x) - (dir == KGGuiDir_Right ? r_avoid.Max.x : r_outer.Min.x);
			const float avail_h = (dir == KGGuiDir_Up ? r_avoid.Min.y : r_outer.Max.y) - (dir == KGGuiDir_Down ? r_avoid.Max.y : r_outer.Min.y);

			// If there's not enough room on one axis, there's no point in positioning on a side on this axis (e.g. when not enough width, use a top/bottom position to maximize available width)
			if (avail_w < size.x && (dir == KGGuiDir_Left || dir == KGGuiDir_Right))
				continue;
			if (avail_h < size.y && (dir == KGGuiDir_Up || dir == KGGuiDir_Down))
				continue;

			KGVec2 pos;
			pos.x = (dir == KGGuiDir_Left) ? r_avoid.Min.x - size.x : (dir == KGGuiDir_Right) ? r_avoid.Max.x : base_pos_clamped.x;
			pos.y = (dir == KGGuiDir_Up) ? r_avoid.Min.y - size.y : (dir == KGGuiDir_Down) ? r_avoid.Max.y : base_pos_clamped.y;

			// Clamp top-left corner of popup
			pos.x = KGMax(pos.x, r_outer.Min.x);
			pos.y = KGMax(pos.y, r_outer.Min.y);

			*last_dir = dir;
			return pos;
		}
	}

	// Fallback when not enough room:
	*last_dir = KGGuiDir_None;

	// For tooltip we prefer avoiding the cursor at all cost even if it means that part of the tooltip won't be visible.
	if (policy == KGGuiPopupPositionPolicy_Tooltip)
		return ref_pos + KGVec2(2, 2);

	// Otherwise try to keep within display
	KGVec2 pos = ref_pos;
	pos.x = KGMax(KGMin(pos.x + size.x, r_outer.Max.x) - size.x, r_outer.Min.x);
	pos.y = KGMax(KGMin(pos.y + size.y, r_outer.Max.y) - size.y, r_outer.Min.y);
	return pos;
}

// Note that this is used for popups, which can overlap the non work-area of individual viewports.
KGRect Karma::KarmaGuiInternal::GetPopupAllowedExtentRect(KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGRect r_screen;
	if (window->ViewportAllowPlatformMonitorExtend >= 0)
	{
		// Extent with be in the frame of reference of the given viewport (so Min is likely to be negative here)
		const KarmaGuiPlatformMonitor& monitor = g.PlatformIO.Monitors[window->ViewportAllowPlatformMonitorExtend];
		r_screen.Min = monitor.WorkPos;
		r_screen.Max = monitor.WorkPos + monitor.WorkSize;
	}
	else
	{
		// Use the full viewport area (not work area) for popups
		r_screen = window->Viewport->GetMainRect();
	}
	KGVec2 padding = g.Style.DisplaySafeAreaPadding;
	r_screen.Expand(KGVec2((r_screen.GetWidth() > padding.x * 2) ? -padding.x : 0.0f, (r_screen.GetHeight() > padding.y * 2) ? -padding.y : 0.0f));
	return r_screen;
}

KGVec2 Karma::KarmaGuiInternal::FindBestWindowPosForPopup(KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;

	KGRect r_outer = GetPopupAllowedExtentRect(window);
	if (window->Flags & KGGuiWindowFlags_ChildMenu)
	{
		// Child menus typically request _any_ position within the parent menu item, and then we move the new menu outside the parent bounds.
		// This is how we end up with child menus appearing (most-commonly) on the right of the parent menu.
		KGGuiWindow* parent_window = window->ParentWindow;
		float horizontal_overlap = g.Style.ItemInnerSpacing.x; // We want some overlap to convey the relative depth of each menu (currently the amount of overlap is hard-coded to style.ItemSpacing.x).
		KGRect r_avoid;
		if (parent_window->DC.MenuBarAppending)
			r_avoid = KGRect(-FLT_MAX, parent_window->ClipRect.Min.y, FLT_MAX, parent_window->ClipRect.Max.y); // Avoid parent menu-bar. If we wanted multi-line menu-bar, we may instead want to have the calling window setup e.g. a NextWindowData.PosConstraintAvoidRect field
		else
			r_avoid = KGRect(parent_window->Pos.x + horizontal_overlap, -FLT_MAX, parent_window->Pos.x + parent_window->Size.x - horizontal_overlap - parent_window->ScrollbarSizes.x, FLT_MAX);
		return FindBestWindowPosForPopupEx(window->Pos, window->Size, &window->AutoPosLastDirection, r_outer, r_avoid, KGGuiPopupPositionPolicy_Default);
	}
	if (window->Flags & KGGuiWindowFlags_Popup)
	{
		return FindBestWindowPosForPopupEx(window->Pos, window->Size, &window->AutoPosLastDirection, r_outer, KGRect(window->Pos, window->Pos), KGGuiPopupPositionPolicy_Default); // Ideally we'd disable r_avoid here
	}
	if (window->Flags & KGGuiWindowFlags_Tooltip)
	{
		// Position tooltip (always follows mouse)
		float sc = g.Style.MouseCursorScale;
		KGVec2 ref_pos = NavCalcPreferredRefPos();
		KGRect r_avoid;
		if (!g.NavDisableHighlight && g.NavDisableMouseHover && !(g.IO.ConfigFlags & KGGuiConfigFlags_NavEnableSetMousePos))
			r_avoid = KGRect(ref_pos.x - 16, ref_pos.y - 8, ref_pos.x + 16, ref_pos.y + 8);
		else
			r_avoid = KGRect(ref_pos.x - 16, ref_pos.y - 8, ref_pos.x + 24 * sc, ref_pos.y + 24 * sc); // FIXME: Hard-coded based on mouse cursor shape expectation. Exact dimension not very important.
		return FindBestWindowPosForPopupEx(ref_pos, window->Size, &window->AutoPosLastDirection, r_outer, r_avoid, KGGuiPopupPositionPolicy_Tooltip);
	}
	KR_CORE_ASSERT(0, "");
	return window->Pos;
}

//-----------------------------------------------------------------------------
// [SECTION] KEYBOARD/GAMEPAD NAVIGATION
//-----------------------------------------------------------------------------

// FIXME-NAV: The existence of SetNavID vs SetFocusID vs FocusWindow() needs to be clarified/reworked.
// In our terminology those should be interchangeable, yet right now this is super confusing.
// Those two functions are merely a legacy artifact, so at minimum naming should be clarified.

void Karma::KarmaGuiInternal::SetNavWindow(KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.NavWindow != window)
	{
		KR_CORE_INFO("[focus] SetNavWindow(\"{0}\")\n", window ? window->Name : "<NULL>");
		g.NavWindow = window;
	}
	g.NavInitRequest = g.NavMoveSubmitted = g.NavMoveScoringItems = false;
	NavUpdateAnyRequestFlag();
}

void Karma::KarmaGuiInternal::SetNavID(KGGuiID id, KGGuiNavLayer nav_layer, KGGuiID focus_scope_id, const KGRect& rect_rel)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(g.NavWindow != NULL, "");
	KR_CORE_ASSERT(nav_layer == KGGuiNavLayer_Main || nav_layer == KGGuiNavLayer_Menu, "");
	g.NavId = id;
	g.NavLayer = nav_layer;
	g.NavFocusScopeId = focus_scope_id;
	g.NavWindow->NavLastIds[nav_layer] = id;
	g.NavWindow->NavRectRel[nav_layer] = rect_rel;
}

void Karma::KarmaGuiInternal::SetFocusID(KGGuiID id, KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(id != 0, "");

	if (g.NavWindow != window)
		SetNavWindow(window);

	// Assume that SetFocusID() is called in the context where its window->DC.NavLayerCurrent and g.CurrentFocusScopeId are valid.
	// Note that window may be != g.CurrentWindow (e.g. SetFocusID call in InputTextEx for multi-line text)
	const KGGuiNavLayer nav_layer = window->DC.NavLayerCurrent;
	g.NavId = id;
	g.NavLayer = nav_layer;
	g.NavFocusScopeId = g.CurrentFocusScopeId;
	window->NavLastIds[nav_layer] = id;
	if (g.LastItemData.ID == id)
		window->NavRectRel[nav_layer] = WindowRectAbsToRel(window, g.LastItemData.NavRect);

	if (g.ActiveIdSource == KGGuiInputSource_Nav)
		g.NavDisableMouseHover = true;
	else
		g.NavDisableHighlight = true;
}

KarmaGuiDir KGGetDirQuadrantFromDelta(float dx, float dy)
{
	if (KGFabs(dx) > KGFabs(dy))
		return (dx > 0.0f) ? KGGuiDir_Right : KGGuiDir_Left;
	return (dy > 0.0f) ? KGGuiDir_Down : KGGuiDir_Up;
}

static float inline NavScoreItemDistInterval(float a0, float a1, float b0, float b1)
{
	if (a1 < b0)
		return a1 - b0;
	if (b1 < a0)
		return a0 - b1;
	return 0.0f;
}

static void inline NavClampRectToVisibleAreaForMoveDir(KarmaGuiDir move_dir, KGRect& r, const KGRect& clip_rect)
{
	if (move_dir == KGGuiDir_Left || move_dir == KGGuiDir_Right)
	{
		r.Min.y = KGClamp(r.Min.y, clip_rect.Min.y, clip_rect.Max.y);
		r.Max.y = KGClamp(r.Max.y, clip_rect.Min.y, clip_rect.Max.y);
	}
	else // FIXME: PageUp/PageDown are leaving move_dir == None
	{
		r.Min.x = KGClamp(r.Min.x, clip_rect.Min.x, clip_rect.Max.x);
		r.Max.x = KGClamp(r.Max.x, clip_rect.Min.x, clip_rect.Max.x);
	}
}

// Scoring function for gamepad/keyboard directional navigation. Based on https://gist.github.com/rygorous/6981057
bool Karma::KarmaGuiInternal::NavScoreItem(KGGuiNavItemData* result)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	if (g.NavLayer != window->DC.NavLayerCurrent)
		return false;

	// FIXME: Those are not good variables names
	KGRect cand = g.LastItemData.NavRect;   // Current item nav rectangle
	const KGRect curr = g.NavScoringRect;   // Current modified source rect (NB: we've applied Max.x = Min.x in NavUpdate() to inhibit the effect of having varied item width)
	g.NavScoringDebugCount++;

	// When entering through a NavFlattened border, we consider child window items as fully clipped for scoring
	if (window->ParentWindow == g.NavWindow)
	{
		KR_CORE_ASSERT((window->Flags | g.NavWindow->Flags) & KGGuiWindowFlags_NavFlattened, "");
		if (!window->ClipRect.Overlaps(cand))
			return false;
		cand.ClipWithFull(window->ClipRect); // This allows the scored item to not overlap other candidates in the parent window
	}

	// We perform scoring on items bounding box clipped by the current clipping rectangle on the other axis (clipping on our movement axis would give us equal scores for all clipped items)
	// For example, this ensures that items in one column are not reached when moving vertically from items in another column.
	NavClampRectToVisibleAreaForMoveDir(g.NavMoveClipDir, cand, window->ClipRect);

	// Compute distance between boxes
	// FIXME-NAV: Introducing biases for vertical navigation, needs to be removed.
	float dbx = NavScoreItemDistInterval(cand.Min.x, cand.Max.x, curr.Min.x, curr.Max.x);
	float dby = NavScoreItemDistInterval(KGLerp(cand.Min.y, cand.Max.y, 0.2f), KGLerp(cand.Min.y, cand.Max.y, 0.8f), KGLerp(curr.Min.y, curr.Max.y, 0.2f), KGLerp(curr.Min.y, curr.Max.y, 0.8f)); // Scale down on Y to keep using box-distance for vertically touching items
	if (dby != 0.0f && dbx != 0.0f)
		dbx = (dbx / 1000.0f) + ((dbx > 0.0f) ? +1.0f : -1.0f);
	float dist_box = KGFabs(dbx) + KGFabs(dby);

	// Compute distance between centers (this is off by a factor of 2, but we only compare center distances with each other so it doesn't matter)
	float dcx = (cand.Min.x + cand.Max.x) - (curr.Min.x + curr.Max.x);
	float dcy = (cand.Min.y + cand.Max.y) - (curr.Min.y + curr.Max.y);
	float dist_center = KGFabs(dcx) + KGFabs(dcy); // L1 metric (need this for our connectedness guarantee)

	// Determine which quadrant of 'curr' our candidate item 'cand' lies in based on distance
	KarmaGuiDir quadrant;
	float dax = 0.0f, day = 0.0f, dist_axial = 0.0f;
	if (dbx != 0.0f || dby != 0.0f)
	{
		// For non-overlapping boxes, use distance between boxes
		dax = dbx;
		day = dby;
		dist_axial = dist_box;
		quadrant = KGGetDirQuadrantFromDelta(dbx, dby);
	}
	else if (dcx != 0.0f || dcy != 0.0f)
	{
		// For overlapping boxes with different centers, use distance between centers
		dax = dcx;
		day = dcy;
		dist_axial = dist_center;
		quadrant = KGGetDirQuadrantFromDelta(dcx, dcy);
	}
	else
	{
		// Degenerate case: two overlapping buttons with same center, break ties arbitrarily (note that LastItemId here is really the _previous_ item order, but it doesn't matter)
		quadrant = (g.LastItemData.ID < g.NavId) ? KGGuiDir_Left : KGGuiDir_Right;
	}

#if KARMAGUI_DEBUG_NAV_SCORING
	char buf[128];
	if (KarmaGui::IsMouseHoveringRect(cand.Min, cand.Max))
	{
		KGFormatString(buf, KG_ARRAYSIZE(buf), "dbox (%.2f,%.2f->%.4f)\ndcen (%.2f,%.2f->%.4f)\nd (%.2f,%.2f->%.4f)\nnav %c, quadrant %c", dbx, dby, dist_box, dcx, dcy, dist_center, dax, day, dist_axial, "WENS"[g.NavMoveDir], "WENS"[quadrant]);
		KGDrawList* draw_list = GetForegroundDrawList(window);
		draw_list->AddRect(curr.Min, curr.Max, KG_COL32(255, 200, 0, 100));
		draw_list->AddRect(cand.Min, cand.Max, KG_COL32(255, 255, 0, 200));
		draw_list->AddRectFilled(cand.Max - KGVec2(4, 4), cand.Max + KarmaGui::CalcTextSize(buf) + KGVec2(4, 4), KG_COL32(40, 0, 0, 150));
		draw_list->AddText(cand.Max, ~0U, buf);
	}
	else if (g.IO.KeyCtrl) // Hold to preview score in matching quadrant. Press C to rotate.
	{
		if (quadrant == g.NavMoveDir)
		{
			KGFormatString(buf, KG_ARRAYSIZE(buf), "%.0f/%.0f", dist_box, dist_center);
			KGDrawList* draw_list = GetForegroundDrawList(window);
			draw_list->AddRectFilled(cand.Min, cand.Max, KG_COL32(255, 0, 0, 200));
			draw_list->AddText(cand.Min, KG_COL32(255, 255, 255, 255), buf);
		}
	}
#endif

	// Is it in the quadrant we're interested in moving to?
	bool new_best = false;
	const KarmaGuiDir move_dir = g.NavMoveDir;
	if (quadrant == move_dir)
	{
		// Does it beat the current best candidate?
		if (dist_box < result->DistBox)
		{
			result->DistBox = dist_box;
			result->DistCenter = dist_center;
			return true;
		}
		if (dist_box == result->DistBox)
		{
			// Try using distance between center points to break ties
			if (dist_center < result->DistCenter)
			{
				result->DistCenter = dist_center;
				new_best = true;
			}
			else if (dist_center == result->DistCenter)
			{
				// Still tied! we need to be extra-careful to make sure everything gets linked properly. We consistently break ties by symbolically moving "later" items
				// (with higher index) to the right/downwards by an infinitesimal amount since we the current "best" button already (so it must have a lower index),
				// this is fairly easy. This rule ensures that all buttons with dx==dy==0 will end up being linked in order of appearance along the x axis.
				if (((move_dir == KGGuiDir_Up || move_dir == KGGuiDir_Down) ? dby : dbx) < 0.0f) // moving bj to the right/down decreases distance
					new_best = true;
			}
		}
	}

	// Axial check: if 'curr' has no link at all in some direction and 'cand' lies roughly in that direction, add a tentative link. This will only be kept if no "real" matches
	// are found, so it only augments the graph produced by the above method using extra links. (important, since it doesn't guarantee strong connectedness)
	// This is just to avoid buttons having no links in a particular direction when there's a suitable neighbor. you get good graphs without this too.
	// 2017/09/29: FIXME: This now currently only enabled inside menu bars, ideally we'd disable it everywhere. Menus in particular need to catch failure. For general navigation it feels awkward.
	// Disabling it may lead to disconnected graphs when nodes are very spaced out on different axis. Perhaps consider offering this as an option?
	if (result->DistBox == FLT_MAX && dist_axial < result->DistAxial)  // Check axial match
		if (g.NavLayer == KGGuiNavLayer_Menu && !(g.NavWindow->Flags & KGGuiWindowFlags_ChildMenu))
			if ((move_dir == KGGuiDir_Left && dax < 0.0f) || (move_dir == KGGuiDir_Right && dax > 0.0f) || (move_dir == KGGuiDir_Up && day < 0.0f) || (move_dir == KGGuiDir_Down && day > 0.0f))
			{
				result->DistAxial = dist_axial;
				new_best = true;
			}

	return new_best;
}

void Karma::KarmaGuiInternal::NavApplyItemToResult(KGGuiNavItemData* result)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	result->Window = window;
	result->ID = g.LastItemData.ID;
	result->FocusScopeId = g.CurrentFocusScopeId;
	result->InFlags = g.LastItemData.InFlags;
	result->RectRel = WindowRectAbsToRel(window, g.LastItemData.NavRect);
}

// We get there when either NavId == id, or when g.NavAnyRequest is set (which is updated by NavUpdateAnyRequestFlag above)
// This is called after LastItemData is set.
void Karma::KarmaGuiInternal::NavProcessItem()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	const KGGuiID id = g.LastItemData.ID;
	const KGRect nav_bb = g.LastItemData.NavRect;
	const KGGuiItemFlags item_flags = g.LastItemData.InFlags;

	// Process Init Request
	if (g.NavInitRequest && g.NavLayer == window->DC.NavLayerCurrent && (item_flags & KGGuiItemFlags_Disabled) == 0)
	{
		// Even if 'KGGuiItemFlags_NoNavDefaultFocus' is on (typically collapse/close button) we record the first ResultId so they can be used as a fallback
		const bool candidate_for_nav_default_focus = (item_flags & KGGuiItemFlags_NoNavDefaultFocus) == 0;
		if (candidate_for_nav_default_focus || g.NavInitResultId == 0)
		{
			g.NavInitResultId = id;
			g.NavInitResultRectRel = WindowRectAbsToRel(window, nav_bb);
		}
		if (candidate_for_nav_default_focus)
		{
			g.NavInitRequest = false; // Found a match, clear request
			NavUpdateAnyRequestFlag();
		}
	}

	// Process Move Request (scoring for navigation)
	// FIXME-NAV: Consider policy for double scoring (scoring from NavScoringRect + scoring from a rect wrapped according to current wrapping policy)
	if (g.NavMoveScoringItems)
	{
		const bool is_tab_stop = (item_flags & KGGuiItemFlags_Inputable) && (item_flags & (KGGuiItemFlags_NoTabStop | KGGuiItemFlags_Disabled)) == 0;
		const bool is_tabbing = (g.NavMoveFlags & KGGuiNavMoveFlags_Tabbing) != 0;
		if (is_tabbing)
		{
			if (is_tab_stop || (g.NavMoveFlags & KGGuiNavMoveFlags_FocusApi))
				NavProcessItemForTabbingRequest(id);
		}
		else if ((g.NavId != id || (g.NavMoveFlags & KGGuiNavMoveFlags_AllowCurrentNavId)) && !(item_flags & KGGuiItemFlags_Disabled))
		{
			KGGuiNavItemData* result = (window == g.NavWindow) ? &g.NavMoveResultLocal : &g.NavMoveResultOther;
			if (!is_tabbing)
			{
				if (NavScoreItem(result))
					NavApplyItemToResult(result);

				// Features like PageUp/PageDown need to maintain a separate score for the visible set of items.
				const float VISIBLE_RATIO = 0.70f;
				if ((g.NavMoveFlags & KGGuiNavMoveFlags_AlsoScoreVisibleSet) && window->ClipRect.Overlaps(nav_bb))
					if (KGClamp(nav_bb.Max.y, window->ClipRect.Min.y, window->ClipRect.Max.y) - KGClamp(nav_bb.Min.y, window->ClipRect.Min.y, window->ClipRect.Max.y) >= (nav_bb.Max.y - nav_bb.Min.y) * VISIBLE_RATIO)
						if (NavScoreItem(&g.NavMoveResultLocalVisible))
							NavApplyItemToResult(&g.NavMoveResultLocalVisible);
			}
		}
	}

	// Update window-relative bounding box of navigated item
	if (g.NavId == id)
	{
		if (g.NavWindow != window)
			SetNavWindow(window); // Always refresh g.NavWindow, because some operations such as FocusItem() may not have a window.
		g.NavLayer = window->DC.NavLayerCurrent;
		g.NavFocusScopeId = g.CurrentFocusScopeId;
		g.NavIdIsAlive = true;
		window->NavRectRel[window->DC.NavLayerCurrent] = WindowRectAbsToRel(window, nav_bb);    // Store item bounding box (relative to window position)
	}
}

// Handle "scoring" of an item for a tabbing/focusing request initiated by NavUpdateCreateTabbingRequest().
// Note that SetKeyboardFocusHere() API calls are considered tabbing requests!
// - Case 1: no nav/active id:    set result to first eligible item, stop storing.
// - Case 2: tab forward:         on ref id set counter, on counter elapse store result
// - Case 3: tab forward wrap:    set result to first eligible item (preemptively), on ref id set counter, on next frame if counter hasn't elapsed store result. // FIXME-TABBING: Could be done as a next-frame forwarded request
// - Case 4: tab backward:        store all results, on ref id pick prev, stop storing
// - Case 5: tab backward wrap:   store all results, on ref id if no result keep storing until last // FIXME-TABBING: Could be done as next-frame forwarded requested
void Karma::KarmaGuiInternal::NavProcessItemForTabbingRequest(KGGuiID id)
{
	KarmaGuiContext& g = *GKarmaGui;

	// Always store in NavMoveResultLocal (unlike directional request which uses NavMoveResultOther on sibling/flattened windows)
	KGGuiNavItemData* result = &g.NavMoveResultLocal;
	if (g.NavTabbingDir == +1)
	{
		// Tab Forward or SetKeyboardFocusHere() with >= 0
		if (g.NavTabbingResultFirst.ID == 0)
			NavApplyItemToResult(&g.NavTabbingResultFirst);
		if (--g.NavTabbingCounter == 0)
			NavMoveRequestResolveWithLastItem(result);
		else if (g.NavId == id)
			g.NavTabbingCounter = 1;
	}
	else if (g.NavTabbingDir == -1)
	{
		// Tab Backward
		if (g.NavId == id)
		{
			if (result->ID)
			{
				g.NavMoveScoringItems = false;
				NavUpdateAnyRequestFlag();
			}
		}
		else
		{
			NavApplyItemToResult(result);
		}
	}
	else if (g.NavTabbingDir == 0)
	{
		// Tab Init
		if (g.NavTabbingResultFirst.ID == 0)
			NavMoveRequestResolveWithLastItem(&g.NavTabbingResultFirst);
	}
}

bool Karma::KarmaGuiInternal::NavMoveRequestButNoResultYet()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.NavMoveScoringItems && g.NavMoveResultLocal.ID == 0 && g.NavMoveResultOther.ID == 0;
}

// FIXME: ScoringRect is not set
void Karma::KarmaGuiInternal::NavMoveRequestSubmit(KarmaGuiDir move_dir, KarmaGuiDir clip_dir, KGGuiNavMoveFlags move_flags, KGGuiScrollFlags scroll_flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(g.NavWindow != NULL, "");

	if (move_flags & KGGuiNavMoveFlags_Tabbing)
		move_flags |= KGGuiNavMoveFlags_AllowCurrentNavId;

	g.NavMoveSubmitted = g.NavMoveScoringItems = true;
	g.NavMoveDir = move_dir;
	g.NavMoveDirForDebug = move_dir;
	g.NavMoveClipDir = clip_dir;
	g.NavMoveFlags = move_flags;
	g.NavMoveScrollFlags = scroll_flags;
	g.NavMoveForwardToNextFrame = false;
	g.NavMoveKeyMods = g.IO.KeyMods;
	g.NavMoveResultLocal.Clear();
	g.NavMoveResultLocalVisible.Clear();
	g.NavMoveResultOther.Clear();
	g.NavTabbingCounter = 0;
	g.NavTabbingResultFirst.Clear();
	NavUpdateAnyRequestFlag();
}

void Karma::KarmaGuiInternal::NavMoveRequestResolveWithLastItem(KGGuiNavItemData* result)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.NavMoveScoringItems = false; // Ensure request doesn't need more processing
	NavApplyItemToResult(result);
	NavUpdateAnyRequestFlag();
}

void Karma::KarmaGuiInternal::NavMoveRequestCancel()
{
	KarmaGuiContext& g = *GKarmaGui;
	g.NavMoveSubmitted = g.NavMoveScoringItems = false;
	NavUpdateAnyRequestFlag();
}

// Forward will reuse the move request again on the next frame (generally with modifications done to it)
void Karma::KarmaGuiInternal::NavMoveRequestForward(KarmaGuiDir move_dir, KarmaGuiDir clip_dir, KGGuiNavMoveFlags move_flags, KGGuiScrollFlags scroll_flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(g.NavMoveForwardToNextFrame == false, "");
	NavMoveRequestCancel();
	g.NavMoveForwardToNextFrame = true;
	g.NavMoveDir = move_dir;
	g.NavMoveClipDir = clip_dir;
	g.NavMoveFlags = move_flags | KGGuiNavMoveFlags_Forwarded;
	g.NavMoveScrollFlags = scroll_flags;
}

// Navigation wrap-around logic is delayed to the end of the frame because this operation is only valid after entire
// popup is assembled and in case of appended popups it is not clear which EndPopup() call is final.
void Karma::KarmaGuiInternal::NavMoveRequestTryWrapping(KGGuiWindow* window, KGGuiNavMoveFlags wrap_flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(wrap_flags != 0, ""); // Call with _WrapX, _WrapY, _LoopX, _LoopY
	// In theory we should test for NavMoveRequestButNoResultYet() but there's no point doing it, NavEndFrame() will do the same test
	if (g.NavWindow == window && g.NavMoveScoringItems && g.NavLayer == KGGuiNavLayer_Main)
		g.NavMoveFlags |= wrap_flags;
}

// FIXME: This could be replaced by updating a frame number in each window when (window == NavWindow) and (NavLayer == 0).
// This way we could find the last focused window among our children. It would be much less confusing this way?
void Karma::KarmaGuiInternal::NavSaveLastChildNavWindowIntoParent(KGGuiWindow* nav_window)
{
	KGGuiWindow* parent = nav_window;
	while (parent && parent->RootWindow != parent && (parent->Flags & (KGGuiWindowFlags_Popup | KGGuiWindowFlags_ChildMenu)) == 0)
		parent = parent->ParentWindow;
	if (parent && parent != nav_window)
		parent->NavLastChildNavWindow = nav_window;
}

// Restore the last focused child.
// Call when we are expected to land on the Main Layer (0) after FocusWindow()
KGGuiWindow* Karma::KarmaGuiInternal::NavRestoreLastChildNavWindow(KGGuiWindow* window)
{
	if (window->NavLastChildNavWindow && window->NavLastChildNavWindow->WasActive)
		return window->NavLastChildNavWindow;
	if (window->DockNodeAsHost && window->DockNodeAsHost->TabBar)
		if (KGGuiTabItem* tab = TabBarFindMostRecentlySelectedTabForActiveWindow(window->DockNodeAsHost->TabBar))
			return tab->Window;
	return window;
}

void Karma::KarmaGuiInternal::NavRestoreLayer(KGGuiNavLayer layer)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (layer == KGGuiNavLayer_Main)
	{
		KGGuiWindow* prev_nav_window = g.NavWindow;
		g.NavWindow = NavRestoreLastChildNavWindow(g.NavWindow);    // FIXME-NAV: Should clear ongoing nav requests?
		if (prev_nav_window)
			KR_CORE_INFO("[focus] NavRestoreLayer: from \"{0}\" to SetNavWindow(\"{1}\")", prev_nav_window->Name, g.NavWindow->Name);
	}
	KGGuiWindow* window = g.NavWindow;
	if (window->NavLastIds[layer] != 0)
	{
		SetNavID(window->NavLastIds[layer], layer, 0, window->NavRectRel[layer]);
	}
	else
	{
		g.NavLayer = layer;
		NavInitWindow(window, true);
	}
}

void Karma::KarmaGuiInternal::NavRestoreHighlightAfterMove()
{
	KarmaGuiContext& g = *GKarmaGui;
	g.NavDisableHighlight = false;
	g.NavDisableMouseHover = g.NavMousePosDirty = true;
}

inline void Karma::KarmaGuiInternal::NavUpdateAnyRequestFlag()
{
	KarmaGuiContext& g = *GKarmaGui;
	g.NavAnyRequest = g.NavMoveScoringItems || g.NavInitRequest || (KARMAGUI_DEBUG_NAV_SCORING && g.NavWindow != NULL);
	if (g.NavAnyRequest)
	{
		KR_CORE_ASSERT(g.NavWindow != NULL, "");
	}
}

// This needs to be called before we submit any widget (aka in or before Begin)
void Karma::KarmaGuiInternal::NavInitWindow(KGGuiWindow* window, bool force_reinit)
{
	// FIXME: ChildWindow test here is wrong for docking
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(window == g.NavWindow, "");

	if (window->Flags & KGGuiWindowFlags_NoNavInputs)
	{
		g.NavId = 0;
		g.NavFocusScopeId = window->NavRootFocusScopeId;
		return;
	}

	bool init_for_nav = false;
	if (window == window->RootWindow || (window->Flags & KGGuiWindowFlags_Popup) || (window->NavLastIds[0] == 0) || force_reinit)
		init_for_nav = true;
	KR_CORE_INFO("[nav] NavInitRequest: from NavInitWindow(), init_for_nav={0}, window=\"{1}\", layer={2}", init_for_nav, window->Name, g.NavLayer);
	if (init_for_nav)
	{
		SetNavID(0, g.NavLayer, window->NavRootFocusScopeId, KGRect());
		g.NavInitRequest = true;
		g.NavInitRequestFromMove = false;
		g.NavInitResultId = 0;
		g.NavInitResultRectRel = KGRect();
		NavUpdateAnyRequestFlag();
	}
	else
	{
		g.NavId = window->NavLastIds[0];
		g.NavFocusScopeId = window->NavRootFocusScopeId;
	}
}

KGVec2 Karma::KarmaGuiInternal::NavCalcPreferredRefPos()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.NavWindow;
	if (g.NavDisableHighlight || !g.NavDisableMouseHover || !window)
	{
		// Mouse (we need a fallback in case the mouse becomes invalid after being used)
		// The +1.0f offset when stored by OpenPopupEx() allows reopening this or another popup (same or another mouse button) while not moving the mouse, it is pretty standard.
		// In theory we could move that +1.0f offset in OpenPopupEx()
		KGVec2 p = KarmaGui::IsMousePosValid(&g.IO.MousePos) ? g.IO.MousePos : g.MouseLastValidPos;
		return KGVec2(p.x + 1.0f, p.y);
	}
	else
	{
		// When navigation is active and mouse is disabled, pick a position around the bottom left of the currently navigated item
		// Take account of upcoming scrolling (maybe set mouse pos should be done in EndFrame?)
		KGRect rect_rel = WindowRectRelToAbs(window, window->NavRectRel[g.NavLayer]);
		if (window->LastFrameActive != g.FrameCount && (window->ScrollTarget.x != FLT_MAX || window->ScrollTarget.y != FLT_MAX))
		{
			KGVec2 next_scroll = CalcNextScrollFromScrollTargetAndClamp(window);
			rect_rel.Translate(window->Scroll - next_scroll);
		}
		KGVec2 pos = KGVec2(rect_rel.Min.x + KGMin(g.Style.FramePadding.x * 4, rect_rel.GetWidth()), rect_rel.Max.y - KGMin(g.Style.FramePadding.y, rect_rel.GetHeight()));
		KarmaGuiViewport* viewport = window->Viewport;
		return KGFloor(KGClamp(pos, viewport->Pos, viewport->Pos + viewport->Size)); // KGFloor() is important because non-integer mouse position application in backend might be lossy and result in undesirable non-zero delta.
	}
}

float Karma::KarmaGuiInternal::GetNavTweakPressedAmount(KGGuiAxis axis)
{
	KarmaGuiContext& g = *GKarmaGui;
	float repeat_delay, repeat_rate;
	GetTypematicRepeatRate(KGGuiInputFlags_RepeatRateNavTweak, &repeat_delay, &repeat_rate);

	KarmaGuiKey key_less, key_more;
	if (g.NavInputSource == KGGuiInputSource_Gamepad)
	{
		key_less = (axis == KGGuiAxis_X) ? KGGuiKey_GamepadDpadLeft : KGGuiKey_GamepadDpadUp;
		key_more = (axis == KGGuiAxis_X) ? KGGuiKey_GamepadDpadRight : KGGuiKey_GamepadDpadDown;
	}
	else
	{
		key_less = (axis == KGGuiAxis_X) ? KGGuiKey_LeftArrow : KGGuiKey_UpArrow;
		key_more = (axis == KGGuiAxis_X) ? KGGuiKey_RightArrow : KGGuiKey_DownArrow;
	}
	float amount = (float)KarmaGui::GetKeyPressedAmount(key_more, repeat_delay, repeat_rate) - (float)KarmaGui::GetKeyPressedAmount(key_less, repeat_delay, repeat_rate);
	if (amount != 0.0f && KarmaGui::IsKeyDown(key_less) && KarmaGui::IsKeyDown(key_more)) // Cancel when opposite directions are held, regardless of repeat phase
		amount = 0.0f;
	return amount;
}

void Karma::KarmaGuiInternal::NavUpdate()
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiIO& io = g.IO;

	io.WantSetMousePos = false;
	//if (g.NavScoringDebugCount > 0) KARMAGUI_DEBUG_LOG_NAV("[nav] NavScoringDebugCount %d for '%s' layer %d (Init:%d, Move:%d)\n", g.NavScoringDebugCount, g.NavWindow ? g.NavWindow->Name : "NULL", g.NavLayer, g.NavInitRequest || g.NavInitResultId != 0, g.NavMoveRequest);

	// Set input source based on which keys are last pressed (as some features differs when used with Gamepad vs Keyboard)
	// FIXME-NAV: Now that keys are separated maybe we can get rid of NavInputSource?
	const bool nav_gamepad_active = (io.ConfigFlags & KGGuiConfigFlags_NavEnableGamepad) != 0 && (io.BackendFlags & KGGuiBackendFlags_HasGamepad) != 0;
	const KarmaGuiKey nav_gamepad_keys_to_change_source[] = { KGGuiKey_GamepadFaceRight, KGGuiKey_GamepadFaceLeft, KGGuiKey_GamepadFaceUp, KGGuiKey_GamepadFaceDown, KGGuiKey_GamepadDpadRight, KGGuiKey_GamepadDpadLeft, KGGuiKey_GamepadDpadUp, KGGuiKey_GamepadDpadDown };
	if (nav_gamepad_active)
		for (KarmaGuiKey key : nav_gamepad_keys_to_change_source)
			if (KarmaGui::IsKeyDown(key))
				g.NavInputSource = KGGuiInputSource_Gamepad;
	const bool nav_keyboard_active = (io.ConfigFlags & KGGuiConfigFlags_NavEnableKeyboard) != 0;
	const KarmaGuiKey nav_keyboard_keys_to_change_source[] = { KGGuiKey_Space, KGGuiKey_Enter, KGGuiKey_Escape, KGGuiKey_RightArrow, KGGuiKey_LeftArrow, KGGuiKey_UpArrow, KGGuiKey_DownArrow };
	if (nav_keyboard_active)
		for (KarmaGuiKey key : nav_keyboard_keys_to_change_source)
			if (KarmaGui::IsKeyDown(key))
				g.NavInputSource = KGGuiInputSource_Keyboard;

	// Process navigation init request (select first/default focus)
	if (g.NavInitResultId != 0)
		NavInitRequestApplyResult();
	g.NavInitRequest = false;
	g.NavInitRequestFromMove = false;
	g.NavInitResultId = 0;
	g.NavJustMovedToId = 0;

	// Process navigation move request
	if (g.NavMoveSubmitted)
		NavMoveRequestApplyResult();
	g.NavTabbingCounter = 0;
	g.NavMoveSubmitted = g.NavMoveScoringItems = false;

	// Schedule mouse position update (will be done at the bottom of this function, after 1) processing all move requests and 2) updating scrolling)
	bool set_mouse_pos = false;
	if (g.NavMousePosDirty && g.NavIdIsAlive)
		if (!g.NavDisableHighlight && g.NavDisableMouseHover && g.NavWindow)
			set_mouse_pos = true;
	g.NavMousePosDirty = false;
	KR_CORE_ASSERT(g.NavLayer == KGGuiNavLayer_Main || g.NavLayer == KGGuiNavLayer_Menu, "");

	// Store our return window (for returning from Menu Layer to Main Layer) and clear it as soon as we step back in our own Layer 0
	if (g.NavWindow)
		NavSaveLastChildNavWindowIntoParent(g.NavWindow);
	if (g.NavWindow && g.NavWindow->NavLastChildNavWindow != NULL && g.NavLayer == KGGuiNavLayer_Main)
		g.NavWindow->NavLastChildNavWindow = NULL;

	// Update CTRL+TAB and Windowing features (hold Square to move/resize/etc.)
	NavUpdateWindowing();

	// Set output flags for user application
	io.NavActive = (nav_keyboard_active || nav_gamepad_active) && g.NavWindow && !(g.NavWindow->Flags & KGGuiWindowFlags_NoNavInputs);
	io.NavVisible = (io.NavActive && g.NavId != 0 && !g.NavDisableHighlight) || (g.NavWindowingTarget != NULL);

	// Process NavCancel input (to close a popup, get back to parent, clear focus)
	NavUpdateCancelRequest();

	// Process manual activation request
	g.NavActivateId = g.NavActivateDownId = g.NavActivatePressedId = g.NavActivateInputId = 0;
	g.NavActivateFlags = KGGuiActivateFlags_None;
	if (g.NavId != 0 && !g.NavDisableHighlight && !g.NavWindowingTarget && g.NavWindow && !(g.NavWindow->Flags & KGGuiWindowFlags_NoNavInputs))
	{
		const bool activate_down = (nav_keyboard_active && KarmaGui::IsKeyDown(KGGuiKey_Space)) || (nav_gamepad_active && KarmaGui::IsKeyDown(KGGuiKey_NavGamepadActivate));
		const bool activate_pressed = activate_down && ((nav_keyboard_active && IsKeyPressed(KGGuiKey_Space, false)) || (nav_gamepad_active && IsKeyPressed(KGGuiKey_NavGamepadActivate, false)));
		const bool input_down = (nav_keyboard_active && KarmaGui::IsKeyDown(KGGuiKey_Enter)) || (nav_gamepad_active && KarmaGui::IsKeyDown(KGGuiKey_NavGamepadInput));
		const bool input_pressed = input_down && ((nav_keyboard_active && IsKeyPressed(KGGuiKey_Enter, false)) || (nav_gamepad_active && IsKeyPressed(KGGuiKey_NavGamepadInput, false)));
		if (g.ActiveId == 0 && activate_pressed)
		{
			g.NavActivateId = g.NavId;
			g.NavActivateFlags = KGGuiActivateFlags_PreferTweak;
		}
		if ((g.ActiveId == 0 || g.ActiveId == g.NavId) && input_pressed)
		{
			g.NavActivateInputId = g.NavId;
			g.NavActivateFlags = KGGuiActivateFlags_PreferInput;
		}
		if ((g.ActiveId == 0 || g.ActiveId == g.NavId) && activate_down)
			g.NavActivateDownId = g.NavId;
		if ((g.ActiveId == 0 || g.ActiveId == g.NavId) && activate_pressed)
			g.NavActivatePressedId = g.NavId;
	}
	if (g.NavWindow && (g.NavWindow->Flags & KGGuiWindowFlags_NoNavInputs))
		g.NavDisableHighlight = true;
	if (g.NavActivateId != 0)
	{
		KR_CORE_ASSERT(g.NavActivateDownId == g.NavActivateId, "");
	}

	// Process programmatic activation request
	// FIXME-NAV: Those should eventually be queued (unlike focus they don't cancel each others)
	if (g.NavNextActivateId != 0)
	{
		if (g.NavNextActivateFlags & KGGuiActivateFlags_PreferInput)
			g.NavActivateInputId = g.NavNextActivateId;
		else
			g.NavActivateId = g.NavActivateDownId = g.NavActivatePressedId = g.NavNextActivateId;
		g.NavActivateFlags = g.NavNextActivateFlags;
	}
	g.NavNextActivateId = 0;

	// Process move requests
	NavUpdateCreateMoveRequest();
	if (g.NavMoveDir == KGGuiDir_None)
		NavUpdateCreateTabbingRequest();
	NavUpdateAnyRequestFlag();
	g.NavIdIsAlive = false;

	// Scrolling
	if (g.NavWindow && !(g.NavWindow->Flags & KGGuiWindowFlags_NoNavInputs) && !g.NavWindowingTarget)
	{
		// *Fallback* manual-scroll with Nav directional keys when window has no navigable item
		KGGuiWindow* window = g.NavWindow;
		const float scroll_speed = KG_ROUND(window->CalcFontSize() * 100 * io.DeltaTime); // We need round the scrolling speed because sub-pixel scroll isn't reliably supported.
		const KarmaGuiDir move_dir = g.NavMoveDir;
		if (window->DC.NavLayersActiveMask == 0x00 && window->DC.NavHasScroll && move_dir != KGGuiDir_None)
		{
			if (move_dir == KGGuiDir_Left || move_dir == KGGuiDir_Right)
				SetScrollX(window, KGFloor(window->Scroll.x + ((move_dir == KGGuiDir_Left) ? -1.0f : +1.0f) * scroll_speed));
			if (move_dir == KGGuiDir_Up || move_dir == KGGuiDir_Down)
				SetScrollY(window, KGFloor(window->Scroll.y + ((move_dir == KGGuiDir_Up) ? -1.0f : +1.0f) * scroll_speed));
		}

		// *Normal* Manual scroll with LStick
		// Next movement request will clamp the NavId reference rectangle to the visible area, so navigation will resume within those bounds.
		if (nav_gamepad_active)
		{
			const KGVec2 scroll_dir = GetKeyMagnitude2d(KGGuiKey_GamepadLStickLeft, KGGuiKey_GamepadLStickRight, KGGuiKey_GamepadLStickUp, KGGuiKey_GamepadLStickDown);
			const float tweak_factor = KarmaGui::IsKeyDown(KGGuiKey_NavGamepadTweakSlow) ? 1.0f / 10.0f : KarmaGui::IsKeyDown(KGGuiKey_NavGamepadTweakFast) ? 10.0f : 1.0f;
			if (scroll_dir.x != 0.0f && window->ScrollbarX)
				SetScrollX(window, KGFloor(window->Scroll.x + scroll_dir.x * scroll_speed * tweak_factor));
			if (scroll_dir.y != 0.0f)
				SetScrollY(window, KGFloor(window->Scroll.y + scroll_dir.y * scroll_speed * tweak_factor));
		}
	}

	// Always prioritize mouse highlight if navigation is disabled
	if (!nav_keyboard_active && !nav_gamepad_active)
	{
		g.NavDisableHighlight = true;
		g.NavDisableMouseHover = set_mouse_pos = false;
	}

	// Update mouse position if requested
	// (This will take into account the possibility that a Scroll was queued in the window to offset our absolute mouse position before scroll has been applied)
	if (set_mouse_pos && (io.ConfigFlags & KGGuiConfigFlags_NavEnableSetMousePos) && (io.BackendFlags & KGGuiBackendFlags_HasSetMousePos))
	{
		io.MousePos = io.MousePosPrev = NavCalcPreferredRefPos();
		io.WantSetMousePos = true;
		//KARMAGUI_DEBUG_LOG_IO("SetMousePos: (%.1f,%.1f)\n", io.MousePos.x, io.MousePos.y);
	}

	// [DEBUG]
	g.NavScoringDebugCount = 0;
#if KARMAGUI_DEBUG_NAV_RECTS
	if (g.NavWindow)
	{
		KGDrawList* draw_list = GetForegroundDrawList(g.NavWindow);
		if (1) { for (int layer = 0; layer < 2; layer++) { KGRect r = WindowRectRelToAbs(g.NavWindow, g.NavWindow->NavRectRel[layer]); draw_list->AddRect(r.Min, r.Max, KG_COL32(255, 200, 0, 255)); } } // [DEBUG]
		if (1) { KGU32 col = (!g.NavWindow->Hidden) ? KG_COL32(255, 0, 255, 255) : KG_COL32(255, 0, 0, 255); KGVec2 p = NavCalcPreferredRefPos(); char buf[32]; KGFormatString(buf, 32, "%d", g.NavLayer); draw_list->AddCircleFilled(p, 3.0f, col); draw_list->AddText(NULL, 13.0f, p + KGVec2(8, -4), col, buf); }
	}
#endif
}

void Karma::KarmaGuiInternal::NavInitRequestApplyResult()
{
	// In very rare cases g.NavWindow may be null (e.g. clearing focus after requesting an init request, which does happen when releasing Alt while clicking on void)
	KarmaGuiContext& g = *GKarmaGui;
	if (!g.NavWindow)
		return;

	// Apply result from previous navigation init request (will typically select the first item, unless SetItemDefaultFocus() has been called)
	// FIXME-NAV: On _NavFlattened windows, g.NavWindow will only be updated during subsequent frame. Not a problem currently.
	KR_CORE_INFO("[nav] NavInitRequest: ApplyResult: NavID {0} in Layer {1} Window \"{2}\"", g.NavInitResultId, g.NavLayer, g.NavWindow->Name);
	SetNavID(g.NavInitResultId, g.NavLayer, 0, g.NavInitResultRectRel);
	g.NavIdIsAlive = true; // Mark as alive from previous frame as we got a result
	if (g.NavInitRequestFromMove)
		NavRestoreHighlightAfterMove();
}

void Karma::KarmaGuiInternal::NavUpdateCreateMoveRequest()
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiIO& io = g.IO;
	KGGuiWindow* window = g.NavWindow;
	const bool nav_gamepad_active = (io.ConfigFlags & KGGuiConfigFlags_NavEnableGamepad) != 0 && (io.BackendFlags & KGGuiBackendFlags_HasGamepad) != 0;
	const bool nav_keyboard_active = (io.ConfigFlags & KGGuiConfigFlags_NavEnableKeyboard) != 0;

	if (g.NavMoveForwardToNextFrame && window != NULL)
	{
		// Forwarding previous request (which has been modified, e.g. wrap around menus rewrite the requests with a starting rectangle at the other side of the window)
		// (preserve most state, which were already set by the NavMoveRequestForward() function)
		KR_CORE_ASSERT(g.NavMoveDir != KGGuiDir_None && g.NavMoveClipDir != KGGuiDir_None, "");
		KR_CORE_ASSERT(g.NavMoveFlags & KGGuiNavMoveFlags_Forwarded, "");
		KR_CORE_INFO("[nav] NavMoveRequestForward {0}", g.NavMoveDir);
	}
	else
	{
		// Initiate directional inputs request
		g.NavMoveDir = KGGuiDir_None;
		g.NavMoveFlags = KGGuiNavMoveFlags_None;
		g.NavMoveScrollFlags = KGGuiScrollFlags_None;
		if (window && !g.NavWindowingTarget && !(window->Flags & KGGuiWindowFlags_NoNavInputs))
		{
			const KarmaGuiInputFlags repeat_mode = KGGuiInputFlags_Repeat | (KarmaGuiInputFlags)KGGuiInputFlags_RepeatRateNavMove;
			if (!IsActiveIdUsingNavDir(KGGuiDir_Left) && ((nav_gamepad_active && IsKeyPressed(KGGuiKey_GamepadDpadLeft, KGGuiKeyOwner_None, repeat_mode)) || (nav_keyboard_active && IsKeyPressed(KGGuiKey_LeftArrow, KGGuiKeyOwner_None, repeat_mode)))) { g.NavMoveDir = KGGuiDir_Left; }
			if (!IsActiveIdUsingNavDir(KGGuiDir_Right) && ((nav_gamepad_active && IsKeyPressed(KGGuiKey_GamepadDpadRight, KGGuiKeyOwner_None, repeat_mode)) || (nav_keyboard_active && IsKeyPressed(KGGuiKey_RightArrow, KGGuiKeyOwner_None, repeat_mode)))) { g.NavMoveDir = KGGuiDir_Right; }
			if (!IsActiveIdUsingNavDir(KGGuiDir_Up) && ((nav_gamepad_active && IsKeyPressed(KGGuiKey_GamepadDpadUp, KGGuiKeyOwner_None, repeat_mode)) || (nav_keyboard_active && IsKeyPressed(KGGuiKey_UpArrow, KGGuiKeyOwner_None, repeat_mode)))) { g.NavMoveDir = KGGuiDir_Up; }
			if (!IsActiveIdUsingNavDir(KGGuiDir_Down) && ((nav_gamepad_active && IsKeyPressed(KGGuiKey_GamepadDpadDown, KGGuiKeyOwner_None, repeat_mode)) || (nav_keyboard_active && IsKeyPressed(KGGuiKey_DownArrow, KGGuiKeyOwner_None, repeat_mode)))) { g.NavMoveDir = KGGuiDir_Down; }
		}
		g.NavMoveClipDir = g.NavMoveDir;
		g.NavScoringNoClipRect = KGRect(+FLT_MAX, +FLT_MAX, -FLT_MAX, -FLT_MAX);
	}

	// Update PageUp/PageDown/Home/End scroll
	// FIXME-NAV: Consider enabling those keys even without the master KGGuiConfigFlags_NavEnableKeyboard flag?
	float scoring_rect_offset_y = 0.0f;
	if (window && g.NavMoveDir == KGGuiDir_None && nav_keyboard_active)
		scoring_rect_offset_y = NavUpdatePageUpPageDown();
	if (scoring_rect_offset_y != 0.0f)
	{
		g.NavScoringNoClipRect = window->InnerRect;
		g.NavScoringNoClipRect.TranslateY(scoring_rect_offset_y);
	}

	// [DEBUG] Always send a request
#if KARMAGUI_DEBUG_NAV_SCORING
	if (io.KeyCtrl && IsKeyPressed(KGGuiKey_C))
		g.NavMoveDirForDebug = (KarmaGuiDir)((g.NavMoveDirForDebug + 1) & 3);
	if (io.KeyCtrl && g.NavMoveDir == KGGuiDir_None)
	{
		g.NavMoveDir = g.NavMoveDirForDebug;
		g.NavMoveFlags |= KGGuiNavMoveFlags_DebugNoResult;
	}
#endif

	// Submit
	g.NavMoveForwardToNextFrame = false;
	if (g.NavMoveDir != KGGuiDir_None)
		NavMoveRequestSubmit(g.NavMoveDir, g.NavMoveClipDir, g.NavMoveFlags, g.NavMoveScrollFlags);

	// Moving with no reference triggers an init request (will be used as a fallback if the direction fails to find a match)
	if (g.NavMoveSubmitted && g.NavId == 0)
	{
		KR_CORE_INFO("[nav] NavInitRequest: from move, window \"{0}\", layer={1}", window ? window->Name : "<NULL>", g.NavLayer);
		g.NavInitRequest = g.NavInitRequestFromMove = true;
		g.NavInitResultId = 0;
		g.NavDisableHighlight = false;
	}

	// When using gamepad, we project the reference nav bounding box into window visible area.
	// This is to allow resuming navigation inside the visible area after doing a large amount of scrolling, since with gamepad all movements are relative
	// (can't focus a visible object like we can with the mouse).
	if (g.NavMoveSubmitted && g.NavInputSource == KGGuiInputSource_Gamepad && g.NavLayer == KGGuiNavLayer_Main && window != NULL)// && (g.NavMoveFlags & KGGuiNavMoveFlags_Forwarded))
	{
		bool clamp_x = (g.NavMoveFlags & (KGGuiNavMoveFlags_LoopX | KGGuiNavMoveFlags_WrapX)) == 0;
		bool clamp_y = (g.NavMoveFlags & (KGGuiNavMoveFlags_LoopY | KGGuiNavMoveFlags_WrapY)) == 0;
		KGRect inner_rect_rel = WindowRectAbsToRel(window, KGRect(window->InnerRect.Min - KGVec2(1, 1), window->InnerRect.Max + KGVec2(1, 1)));
		if ((clamp_x || clamp_y) && !inner_rect_rel.Contains(window->NavRectRel[g.NavLayer]))
		{
			//KARMAGUI_DEBUG_LOG_NAV("[nav] NavMoveRequest: clamp NavRectRel for gamepad move\n");
			float pad_x = KGMin(inner_rect_rel.GetWidth(), window->CalcFontSize() * 0.5f);
			float pad_y = KGMin(inner_rect_rel.GetHeight(), window->CalcFontSize() * 0.5f); // Terrible approximation for the intent of starting navigation from first fully visible item
			inner_rect_rel.Min.x = clamp_x ? (inner_rect_rel.Min.x + pad_x) : -FLT_MAX;
			inner_rect_rel.Max.x = clamp_x ? (inner_rect_rel.Max.x - pad_x) : +FLT_MAX;
			inner_rect_rel.Min.y = clamp_y ? (inner_rect_rel.Min.y + pad_y) : -FLT_MAX;
			inner_rect_rel.Max.y = clamp_y ? (inner_rect_rel.Max.y - pad_y) : +FLT_MAX;
			window->NavRectRel[g.NavLayer].ClipWithFull(inner_rect_rel);
			g.NavId = 0;
		}
	}

	// For scoring we use a single segment on the left side our current item bounding box (not touching the edge to avoid box overlap with zero-spaced items)
	KGRect scoring_rect;
	if (window != NULL)
	{
		KGRect nav_rect_rel = !window->NavRectRel[g.NavLayer].IsInverted() ? window->NavRectRel[g.NavLayer] : KGRect(0, 0, 0, 0);
		scoring_rect = WindowRectRelToAbs(window, nav_rect_rel);
		scoring_rect.TranslateY(scoring_rect_offset_y);
		scoring_rect.Min.x = KGMin(scoring_rect.Min.x + 1.0f, scoring_rect.Max.x);
		scoring_rect.Max.x = scoring_rect.Min.x;
		KR_CORE_ASSERT(!scoring_rect.IsInverted(), ""); // Ensure if we have a finite, non-inverted bounding box here will allow us to remove extraneous KGFabs() calls in NavScoreItem().
		//GetForegroundDrawList()->AddRect(scoring_rect.Min, scoring_rect.Max, KG_COL32(255,200,0,255)); // [DEBUG]
		//if (!g.NavScoringNoClipRect.IsInverted()) { GetForegroundDrawList()->AddRect(g.NavScoringNoClipRect.Min, g.NavScoringNoClipRect.Max, KG_COL32(255, 200, 0, 255)); } // [DEBUG]
	}
	g.NavScoringRect = scoring_rect;
	g.NavScoringNoClipRect.Add(scoring_rect);
}

void Karma::KarmaGuiInternal::NavUpdateCreateTabbingRequest()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.NavWindow;
	KR_CORE_ASSERT(g.NavMoveDir == KGGuiDir_None, "");
	if (window == NULL || g.NavWindowingTarget != NULL || (window->Flags & KGGuiWindowFlags_NoNavInputs))
		return;

	const bool tab_pressed = IsKeyPressed(KGGuiKey_Tab, KGGuiKeyOwner_None, KGGuiInputFlags_Repeat) && !g.IO.KeyCtrl && !g.IO.KeyAlt;
	if (!tab_pressed)
		return;

	// Initiate tabbing request
	// (this is ALWAYS ENABLED, regardless of KGGuiConfigFlags_NavEnableKeyboard flag!)
	// Initially this was designed to use counters and modulo arithmetic, but that could not work with unsubmitted items (list clipper). Instead we use a strategy close to other move requests.
	// See NavProcessItemForTabbingRequest() for a description of the various forward/backward tabbing cases with and without wrapping.
	//// FIXME: We use (g.ActiveId == 0) but (g.NavDisableHighlight == false) might be righter once we can tab through anything
	g.NavTabbingDir = g.IO.KeyShift ? -1 : (g.ActiveId == 0) ? 0 : +1;
	KGGuiScrollFlags scroll_flags = window->Appearing ? KGGuiScrollFlags_KeepVisibleEdgeX | KGGuiScrollFlags_AlwaysCenterY : KGGuiScrollFlags_KeepVisibleEdgeX | KGGuiScrollFlags_KeepVisibleEdgeY;
	KarmaGuiDir clip_dir = (g.NavTabbingDir < 0) ? KGGuiDir_Up : KGGuiDir_Down;
	NavMoveRequestSubmit(KGGuiDir_None, clip_dir, KGGuiNavMoveFlags_Tabbing, scroll_flags); // FIXME-NAV: Once we refactor tabbing, add LegacyApi flag to not activate non-inputable.
	g.NavTabbingCounter = -1;
}

// Apply result from previous frame navigation directional move request. Always called from NavUpdate()
void Karma::KarmaGuiInternal::NavMoveRequestApplyResult()
{
	KarmaGuiContext& g = *GKarmaGui;
#if KARMAGUI_DEBUG_NAV_SCORING
	if (g.NavMoveFlags & KGGuiNavMoveFlags_DebugNoResult) // [DEBUG] Scoring all items in NavWindow at all times
		return;
#endif

	// Select which result to use
	KGGuiNavItemData* result = (g.NavMoveResultLocal.ID != 0) ? &g.NavMoveResultLocal : (g.NavMoveResultOther.ID != 0) ? &g.NavMoveResultOther : NULL;

	// Tabbing forward wrap
	if (g.NavMoveFlags & KGGuiNavMoveFlags_Tabbing)
		if ((g.NavTabbingCounter == 1 || g.NavTabbingDir == 0) && g.NavTabbingResultFirst.ID)
			result = &g.NavTabbingResultFirst;

	// In a situation when there are no results but NavId != 0, re-enable the Navigation highlight (because g.NavId is not considered as a possible result)
	if (result == NULL)
	{
		if (g.NavMoveFlags & KGGuiNavMoveFlags_Tabbing)
			g.NavMoveFlags |= KGGuiNavMoveFlags_DontSetNavHighlight;
		if (g.NavId != 0 && (g.NavMoveFlags & KGGuiNavMoveFlags_DontSetNavHighlight) == 0)
			NavRestoreHighlightAfterMove();
		return;
	}

	// PageUp/PageDown behavior first jumps to the bottom/top mostly visible item, _otherwise_ use the result from the previous/next page.
	if (g.NavMoveFlags & KGGuiNavMoveFlags_AlsoScoreVisibleSet)
		if (g.NavMoveResultLocalVisible.ID != 0 && g.NavMoveResultLocalVisible.ID != g.NavId)
			result = &g.NavMoveResultLocalVisible;

	// Maybe entering a flattened child from the outside? In this case solve the tie using the regular scoring rules.
	if (result != &g.NavMoveResultOther && g.NavMoveResultOther.ID != 0 && g.NavMoveResultOther.Window->ParentWindow == g.NavWindow)
		if ((g.NavMoveResultOther.DistBox < result->DistBox) || (g.NavMoveResultOther.DistBox == result->DistBox && g.NavMoveResultOther.DistCenter < result->DistCenter))
			result = &g.NavMoveResultOther;
	KR_CORE_ASSERT(g.NavWindow && result->Window, "");

	// Scroll to keep newly navigated item fully into view.
	if (g.NavLayer == KGGuiNavLayer_Main)
	{
		if (g.NavMoveFlags & KGGuiNavMoveFlags_ScrollToEdgeY)
		{
			// FIXME: Should remove this
			float scroll_target = (g.NavMoveDir == KGGuiDir_Up) ? result->Window->ScrollMax.y : 0.0f;
			SetScrollY(result->Window, scroll_target);
		}
		else
		{
			KGRect rect_abs = WindowRectRelToAbs(result->Window, result->RectRel);
			ScrollToRectEx(result->Window, rect_abs, g.NavMoveScrollFlags);
		}
	}

	if (g.NavWindow != result->Window)
	{
		KR_CORE_INFO("[focus] NavMoveRequest: SetNavWindow(\"{0}\")", result->Window->Name);
		g.NavWindow = result->Window;
	}
	if (g.ActiveId != result->ID)
		ClearActiveID();
	if (g.NavId != result->ID)
	{
		// Don't set NavJustMovedToId if just landed on the same spot (which may happen with KGGuiNavMoveFlags_AllowCurrentNavId)
		g.NavJustMovedToId = result->ID;
		g.NavJustMovedToFocusScopeId = result->FocusScopeId;
		g.NavJustMovedToKeyMods = g.NavMoveKeyMods;
	}

	// Focus
	KR_CORE_INFO("[nav] NavMoveRequest: result NavID {0} in Layer {1} Window \"{1}\"", result->ID, g.NavLayer, g.NavWindow->Name);
	SetNavID(result->ID, g.NavLayer, result->FocusScopeId, result->RectRel);

	// Tabbing: Activates Inputable or Focus non-Inputable
	if ((g.NavMoveFlags & KGGuiNavMoveFlags_Tabbing) && (result->InFlags & KGGuiItemFlags_Inputable))
	{
		g.NavNextActivateId = result->ID;
		g.NavNextActivateFlags = KGGuiActivateFlags_PreferInput | KGGuiActivateFlags_TryToPreserveState;
		g.NavMoveFlags |= KGGuiNavMoveFlags_DontSetNavHighlight;
	}

	// Activate
	if (g.NavMoveFlags & KGGuiNavMoveFlags_Activate)
	{
		g.NavNextActivateId = result->ID;
		g.NavNextActivateFlags = KGGuiActivateFlags_None;
	}

	// Enable nav highlight
	if ((g.NavMoveFlags & KGGuiNavMoveFlags_DontSetNavHighlight) == 0)
		NavRestoreHighlightAfterMove();
}

// Process NavCancel input (to close a popup, get back to parent, clear focus)
// FIXME: In order to support e.g. Escape to clear a selection we'll need:
// - either to store the equivalent of ActiveIdUsingKeyInputMask for a FocusScope and test for it.
// - either to move most/all of those tests to the epilogue/end functions of the scope they are dealing with (e.g. exit child window in EndChild()) or in EndFrame(), to allow an earlier intercept
void Karma::KarmaGuiInternal::NavUpdateCancelRequest()
{
	KarmaGuiContext& g = *GKarmaGui;
	const bool nav_gamepad_active = (g.IO.ConfigFlags & KGGuiConfigFlags_NavEnableGamepad) != 0 && (g.IO.BackendFlags & KGGuiBackendFlags_HasGamepad) != 0;
	const bool nav_keyboard_active = (g.IO.ConfigFlags & KGGuiConfigFlags_NavEnableKeyboard) != 0;
	if (!(nav_keyboard_active && IsKeyPressed(KGGuiKey_Escape, KGGuiKeyOwner_None)) && !(nav_gamepad_active && IsKeyPressed(KGGuiKey_NavGamepadCancel, KGGuiKeyOwner_None)))
		return;

	KR_CORE_INFO("[nav] NavUpdateCancelRequest()");
	if (g.ActiveId != 0)
	{
		ClearActiveID();
	}
	else if (g.NavLayer != KGGuiNavLayer_Main)
	{
		// Leave the "menu" layer
		NavRestoreLayer(KGGuiNavLayer_Main);
		NavRestoreHighlightAfterMove();
	}
	else if (g.NavWindow && g.NavWindow != g.NavWindow->RootWindow && !(g.NavWindow->Flags & KGGuiWindowFlags_Popup) && g.NavWindow->ParentWindow)
	{
		// Exit child window
		KGGuiWindow* child_window = g.NavWindow;
		KGGuiWindow* parent_window = g.NavWindow->ParentWindow;
		KR_CORE_ASSERT(child_window->ChildId != 0, "");
		KGRect child_rect = child_window->Rect();
		FocusWindow(parent_window);
		SetNavID(child_window->ChildId, KGGuiNavLayer_Main, 0, WindowRectAbsToRel(parent_window, child_rect));
		NavRestoreHighlightAfterMove();
	}
	else if (g.OpenPopupStack.Size > 0 && g.OpenPopupStack.back().Window != NULL && !(g.OpenPopupStack.back().Window->Flags & KGGuiWindowFlags_Modal))
	{
		// Close open popup/menu
		ClosePopupToLevel(g.OpenPopupStack.Size - 1, true);
	}
	else
	{
		// Clear NavLastId for popups but keep it for regular child window so we can leave one and come back where we were
		if (g.NavWindow && ((g.NavWindow->Flags & KGGuiWindowFlags_Popup) || !(g.NavWindow->Flags & KGGuiWindowFlags_ChildWindow)))
			g.NavWindow->NavLastIds[0] = 0;
		g.NavId = 0;
	}
}

// Handle PageUp/PageDown/Home/End keys
// Called from NavUpdateCreateMoveRequest() which will use our output to create a move request
// FIXME-NAV: This doesn't work properly with NavFlattened siblings as we use NavWindow rectangle for reference
// FIXME-NAV: how to get Home/End to aim at the beginning/end of a 2D grid?
float Karma::KarmaGuiInternal::NavUpdatePageUpPageDown()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.NavWindow;
	if ((window->Flags & KGGuiWindowFlags_NoNavInputs) || g.NavWindowingTarget != NULL)
		return 0.0f;

	const bool page_up_held = IsKeyDown(KGGuiKey_PageUp, KGGuiKeyOwner_None);
	const bool page_down_held = IsKeyDown(KGGuiKey_PageDown, KGGuiKeyOwner_None);
	const bool home_pressed = IsKeyPressed(KGGuiKey_Home, KGGuiKeyOwner_None, KGGuiInputFlags_Repeat);
	const bool end_pressed = IsKeyPressed(KGGuiKey_End, KGGuiKeyOwner_None, KGGuiInputFlags_Repeat);
	if (page_up_held == page_down_held && home_pressed == end_pressed) // Proceed if either (not both) are pressed, otherwise early out
		return 0.0f;

	if (g.NavLayer != KGGuiNavLayer_Main)
		NavRestoreLayer(KGGuiNavLayer_Main);

	if (window->DC.NavLayersActiveMask == 0x00 && window->DC.NavHasScroll)
	{
		// Fallback manual-scroll when window has no navigable item
		if (IsKeyPressed(KGGuiKey_PageUp, KGGuiKeyOwner_None, KGGuiInputFlags_Repeat))
			SetScrollY(window, window->Scroll.y - window->InnerRect.GetHeight());
		else if (IsKeyPressed(KGGuiKey_PageDown, KGGuiKeyOwner_None, KGGuiInputFlags_Repeat))
			SetScrollY(window, window->Scroll.y + window->InnerRect.GetHeight());
		else if (home_pressed)
			SetScrollY(window, 0.0f);
		else if (end_pressed)
			SetScrollY(window, window->ScrollMax.y);
	}
	else
	{
		KGRect& nav_rect_rel = window->NavRectRel[g.NavLayer];
		const float page_offset_y = KGMax(0.0f, window->InnerRect.GetHeight() - window->CalcFontSize() * 1.0f + nav_rect_rel.GetHeight());
		float nav_scoring_rect_offset_y = 0.0f;
		if (IsKeyPressed(KGGuiKey_PageUp, true))
		{
			nav_scoring_rect_offset_y = -page_offset_y;
			g.NavMoveDir = KGGuiDir_Down; // Because our scoring rect is offset up, we request the down direction (so we can always land on the last item)
			g.NavMoveClipDir = KGGuiDir_Up;
			g.NavMoveFlags = KGGuiNavMoveFlags_AllowCurrentNavId | KGGuiNavMoveFlags_AlsoScoreVisibleSet;
		}
		else if (IsKeyPressed(KGGuiKey_PageDown, true))
		{
			nav_scoring_rect_offset_y = +page_offset_y;
			g.NavMoveDir = KGGuiDir_Up; // Because our scoring rect is offset down, we request the up direction (so we can always land on the last item)
			g.NavMoveClipDir = KGGuiDir_Down;
			g.NavMoveFlags = KGGuiNavMoveFlags_AllowCurrentNavId | KGGuiNavMoveFlags_AlsoScoreVisibleSet;
		}
		else if (home_pressed)
		{
			// FIXME-NAV: handling of Home/End is assuming that the top/bottom most item will be visible with Scroll.y == 0/ScrollMax.y
			// Scrolling will be handled via the KGGuiNavMoveFlags_ScrollToEdgeY flag, we don't scroll immediately to avoid scrolling happening before nav result.
			// Preserve current horizontal position if we have any.
			nav_rect_rel.Min.y = nav_rect_rel.Max.y = 0.0f;
			if (nav_rect_rel.IsInverted())
				nav_rect_rel.Min.x = nav_rect_rel.Max.x = 0.0f;
			g.NavMoveDir = KGGuiDir_Down;
			g.NavMoveFlags = KGGuiNavMoveFlags_AllowCurrentNavId | KGGuiNavMoveFlags_ScrollToEdgeY;
			// FIXME-NAV: MoveClipDir left to _None, intentional?
		}
		else if (end_pressed)
		{
			nav_rect_rel.Min.y = nav_rect_rel.Max.y = window->ContentSize.y;
			if (nav_rect_rel.IsInverted())
				nav_rect_rel.Min.x = nav_rect_rel.Max.x = 0.0f;
			g.NavMoveDir = KGGuiDir_Up;
			g.NavMoveFlags = KGGuiNavMoveFlags_AllowCurrentNavId | KGGuiNavMoveFlags_ScrollToEdgeY;
			// FIXME-NAV: MoveClipDir left to _None, intentional?
		}
		return nav_scoring_rect_offset_y;
	}
	return 0.0f;
}

void Karma::KarmaGuiInternal::NavEndFrame()
{
	KarmaGuiContext& g = *GKarmaGui;

	// Show CTRL+TAB list window
	if (g.NavWindowingTarget != NULL)
		NavUpdateWindowingOverlay();

	// Perform wrap-around in menus
	// FIXME-NAV: Wrap may need to apply a weight bias on the other axis. e.g. 4x4 grid with 2 last items missing on last item won't handle LoopY/WrapY correctly.
	// FIXME-NAV: Wrap (not Loop) support could be handled by the scoring function and then WrapX would function without an extra frame.
	const KGGuiNavMoveFlags wanted_flags = KGGuiNavMoveFlags_WrapX | KGGuiNavMoveFlags_LoopX | KGGuiNavMoveFlags_WrapY | KGGuiNavMoveFlags_LoopY;
	if (g.NavWindow && NavMoveRequestButNoResultYet() && (g.NavMoveFlags & wanted_flags) && (g.NavMoveFlags & KGGuiNavMoveFlags_Forwarded) == 0)
		NavUpdateCreateWrappingRequest();
}

void Karma::KarmaGuiInternal::NavUpdateCreateWrappingRequest()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.NavWindow;

	bool do_forward = false;
	KGRect bb_rel = window->NavRectRel[g.NavLayer];
	KarmaGuiDir clip_dir = g.NavMoveDir;
	const KGGuiNavMoveFlags move_flags = g.NavMoveFlags;
	if (g.NavMoveDir == KGGuiDir_Left && (move_flags & (KGGuiNavMoveFlags_WrapX | KGGuiNavMoveFlags_LoopX)))
	{
		bb_rel.Min.x = bb_rel.Max.x = window->ContentSize.x + window->WindowPadding.x;
		if (move_flags & KGGuiNavMoveFlags_WrapX)
		{
			bb_rel.TranslateY(-bb_rel.GetHeight()); // Previous row
			clip_dir = KGGuiDir_Up;
		}
		do_forward = true;
	}
	if (g.NavMoveDir == KGGuiDir_Right && (move_flags & (KGGuiNavMoveFlags_WrapX | KGGuiNavMoveFlags_LoopX)))
	{
		bb_rel.Min.x = bb_rel.Max.x = -window->WindowPadding.x;
		if (move_flags & KGGuiNavMoveFlags_WrapX)
		{
			bb_rel.TranslateY(+bb_rel.GetHeight()); // Next row
			clip_dir = KGGuiDir_Down;
		}
		do_forward = true;
	}
	if (g.NavMoveDir == KGGuiDir_Up && (move_flags & (KGGuiNavMoveFlags_WrapY | KGGuiNavMoveFlags_LoopY)))
	{
		bb_rel.Min.y = bb_rel.Max.y = window->ContentSize.y + window->WindowPadding.y;
		if (move_flags & KGGuiNavMoveFlags_WrapY)
		{
			bb_rel.TranslateX(-bb_rel.GetWidth()); // Previous column
			clip_dir = KGGuiDir_Left;
		}
		do_forward = true;
	}
	if (g.NavMoveDir == KGGuiDir_Down && (move_flags & (KGGuiNavMoveFlags_WrapY | KGGuiNavMoveFlags_LoopY)))
	{
		bb_rel.Min.y = bb_rel.Max.y = -window->WindowPadding.y;
		if (move_flags & KGGuiNavMoveFlags_WrapY)
		{
			bb_rel.TranslateX(+bb_rel.GetWidth()); // Next column
			clip_dir = KGGuiDir_Right;
		}
		do_forward = true;
	}
	if (!do_forward)
		return;
	window->NavRectRel[g.NavLayer] = bb_rel;
	NavMoveRequestForward(g.NavMoveDir, clip_dir, move_flags, g.NavMoveScrollFlags);
}

int Karma::KarmaGuiInternal::FindWindowFocusIndex(KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;
	KG_UNUSED(g);
	int order = window->FocusOrder;
	KR_CORE_ASSERT(window->RootWindow == window, ""); // No child window (not testing _ChildWindow because of docking)
	KR_CORE_ASSERT(g.WindowsFocusOrder[order] == window, "");
	return order;
}

static KGGuiWindow* FindWindowNavFocusable(int i_start, int i_stop, int dir) // FIXME-OPT O(N)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	for (int i = i_start; i >= 0 && i < g.WindowsFocusOrder.Size && i != i_stop; i += dir)
		if (Karma::KarmaGuiInternal::IsWindowNavFocusable(g.WindowsFocusOrder[i]))
			return g.WindowsFocusOrder[i];
	return NULL;
}

static void NavUpdateWindowingHighlightWindow(int focus_change_dir)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KR_CORE_ASSERT(g.NavWindowingTarget, "");
	if (g.NavWindowingTarget->Flags & KGGuiWindowFlags_Modal)
		return;

	const int i_current = Karma::KarmaGuiInternal::FindWindowFocusIndex(g.NavWindowingTarget);
	KGGuiWindow* window_target = FindWindowNavFocusable(i_current + focus_change_dir, -INT_MAX, focus_change_dir);
	if (!window_target)
		window_target = FindWindowNavFocusable((focus_change_dir < 0) ? (g.WindowsFocusOrder.Size - 1) : 0, i_current, focus_change_dir);
	if (window_target) // Don't reset windowing target if there's a single window in the list
	{
		g.NavWindowingTarget = g.NavWindowingTargetAnim = window_target;
		g.NavWindowingAccumDeltaPos = g.NavWindowingAccumDeltaSize = KGVec2(0.0f, 0.0f);
	}
	g.NavWindowingToggleLayer = false;
}

// Windowing management mode
// Keyboard: CTRL+Tab (change focus/move/resize), Alt (toggle menu layer)
// Gamepad:  Hold Menu/Square (change focus/move/resize), Tap Menu/Square (toggle menu layer)
void Karma::KarmaGuiInternal::NavUpdateWindowing()
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiIO& io = g.IO;

	KGGuiWindow* apply_focus_window = NULL;
	bool apply_toggle_layer = false;

	KGGuiWindow* modal_window = GetTopMostPopupModal();
	bool allow_windowing = (modal_window == NULL);
	if (!allow_windowing)
		g.NavWindowingTarget = NULL;

	// Fade out
	if (g.NavWindowingTargetAnim && g.NavWindowingTarget == NULL)
	{
		g.NavWindowingHighlightAlpha = KGMax(g.NavWindowingHighlightAlpha - io.DeltaTime * 10.0f, 0.0f);
		if (g.DimBgRatio <= 0.0f && g.NavWindowingHighlightAlpha <= 0.0f)
			g.NavWindowingTargetAnim = NULL;
	}

	// Start CTRL+Tab or Square+L/R window selection
	const bool nav_gamepad_active = (io.ConfigFlags & KGGuiConfigFlags_NavEnableGamepad) != 0 && (io.BackendFlags & KGGuiBackendFlags_HasGamepad) != 0;
	const bool nav_keyboard_active = (io.ConfigFlags & KGGuiConfigFlags_NavEnableKeyboard) != 0;
	const bool keyboard_next_window = allow_windowing && g.ConfigNavWindowingKeyNext && KarmaGui::Shortcut(g.ConfigNavWindowingKeyNext, KGGuiKeyOwner_None, KGGuiInputFlags_Repeat | KGGuiInputFlags_RouteAlways);
	const bool keyboard_prev_window = allow_windowing && g.ConfigNavWindowingKeyPrev && KarmaGui::Shortcut(g.ConfigNavWindowingKeyPrev, KGGuiKeyOwner_None, KGGuiInputFlags_Repeat | KGGuiInputFlags_RouteAlways);
	const bool start_windowing_with_gamepad = allow_windowing && nav_gamepad_active && !g.NavWindowingTarget && IsKeyPressed(KGGuiKey_NavGamepadMenu, 0, KGGuiInputFlags_None);
	const bool start_windowing_with_keyboard = allow_windowing && !g.NavWindowingTarget && (keyboard_next_window || keyboard_prev_window); // Note: enabled even without NavEnableKeyboard!
	if (start_windowing_with_gamepad || start_windowing_with_keyboard)
		if (KGGuiWindow* window = g.NavWindow ? g.NavWindow : FindWindowNavFocusable(g.WindowsFocusOrder.Size - 1, -INT_MAX, -1))
		{
			g.NavWindowingTarget = g.NavWindowingTargetAnim = window->RootWindow;
			g.NavWindowingTimer = g.NavWindowingHighlightAlpha = 0.0f;
			g.NavWindowingAccumDeltaPos = g.NavWindowingAccumDeltaSize = KGVec2(0.0f, 0.0f);
			g.NavWindowingToggleLayer = start_windowing_with_gamepad ? true : false; // Gamepad starts toggling layer
			g.NavInputSource = start_windowing_with_keyboard ? KGGuiInputSource_Keyboard : KGGuiInputSource_Gamepad;
		}

	// Gamepad update
	g.NavWindowingTimer += io.DeltaTime;
	if (g.NavWindowingTarget && g.NavInputSource == KGGuiInputSource_Gamepad)
	{
		// Highlight only appears after a brief time holding the button, so that a fast tap on PadMenu (to toggle NavLayer) doesn't add visual noise
		g.NavWindowingHighlightAlpha = KGMax(g.NavWindowingHighlightAlpha, KGSaturate((g.NavWindowingTimer - NAV_WINDOWING_HIGHLIGHT_DELAY) / 0.05f));

		// Select window to focus
		const int focus_change_dir = (int)KarmaGui::IsKeyPressed(KGGuiKey_GamepadL1) - (int)KarmaGui::IsKeyPressed(KGGuiKey_GamepadR1);
		if (focus_change_dir != 0)
		{
			NavUpdateWindowingHighlightWindow(focus_change_dir);
			g.NavWindowingHighlightAlpha = 1.0f;
		}

		// Single press toggles NavLayer, long press with L/R apply actual focus on release (until then the window was merely rendered top-most)
		if (!KarmaGui::IsKeyDown(KGGuiKey_NavGamepadMenu))
		{
			g.NavWindowingToggleLayer &= (g.NavWindowingHighlightAlpha < 1.0f); // Once button was held long enough we don't consider it a tap-to-toggle-layer press anymore.
			if (g.NavWindowingToggleLayer && g.NavWindow)
				apply_toggle_layer = true;
			else if (!g.NavWindowingToggleLayer)
				apply_focus_window = g.NavWindowingTarget;
			g.NavWindowingTarget = NULL;
		}
	}

	// Keyboard: Focus
	if (g.NavWindowingTarget && g.NavInputSource == KGGuiInputSource_Keyboard)
	{
		// Visuals only appears after a brief time after pressing TAB the first time, so that a fast CTRL+TAB doesn't add visual noise
		KarmaGuiKeyChord shared_mods = ((g.ConfigNavWindowingKeyNext ? g.ConfigNavWindowingKeyNext : KGGuiMod_Mask_) & (g.ConfigNavWindowingKeyPrev ? g.ConfigNavWindowingKeyPrev : KGGuiMod_Mask_)) & KGGuiMod_Mask_;
		KR_CORE_ASSERT(shared_mods != 0, ""); // Next/Prev shortcut currently needs a shared modifier to "hold", otherwise Prev actions would keep cycling between two windows.
		g.NavWindowingHighlightAlpha = KGMax(g.NavWindowingHighlightAlpha, KGSaturate((g.NavWindowingTimer - NAV_WINDOWING_HIGHLIGHT_DELAY) / 0.05f)); // 1.0f
		if (keyboard_next_window || keyboard_prev_window)
			NavUpdateWindowingHighlightWindow(keyboard_next_window ? -1 : +1);
		else if ((io.KeyMods & shared_mods) != shared_mods)
			apply_focus_window = g.NavWindowingTarget;
	}

	// Keyboard: Press and Release ALT to toggle menu layer
	// - Testing that only Alt is tested prevents Alt+Shift or AltGR from toggling menu layer.
	// - AltGR is normally Alt+Ctrl but we can't reliably detect it (not all backends/systems/layout emit it as Alt+Ctrl). But even on keyboards without AltGR we don't want Alt+Ctrl to open menu anyway.
	if (nav_keyboard_active && IsKeyPressed(KGGuiMod_Alt, KGGuiKeyOwner_None))
	{
		g.NavWindowingToggleLayer = true;
		g.NavInputSource = KGGuiInputSource_Keyboard;
	}
	if (g.NavWindowingToggleLayer && g.NavInputSource == KGGuiInputSource_Keyboard)
	{
		// We cancel toggling nav layer when any text has been typed (generally while holding Alt). (See #370)
		// We cancel toggling nav layer when other modifiers are pressed. (See #4439)
		// We cancel toggling nav layer if an owner has claimed the key.
		if (io.InputQueueCharacters.Size > 0 || io.KeyCtrl || io.KeyShift || io.KeySuper || TestKeyOwner(KGGuiMod_Alt, KGGuiKeyOwner_None) == false)
			g.NavWindowingToggleLayer = false;

		// Apply layer toggle on release
		// Important: as before version <18314 we lacked an explicit IO event for focus gain/loss, we also compare mouse validity to detect old backends clearing mouse pos on focus loss.
		if (KarmaGui::IsKeyReleased(KGGuiMod_Alt) && g.NavWindowingToggleLayer)
			if (g.ActiveId == 0 || g.ActiveIdAllowOverlap)
				if (KarmaGui::IsMousePosValid(&io.MousePos) == KarmaGui::IsMousePosValid(&io.MousePosPrev))
					apply_toggle_layer = true;
		if (!KarmaGui::IsKeyDown(KGGuiMod_Alt))
			g.NavWindowingToggleLayer = false;
	}

	// Move window
	if (g.NavWindowingTarget && !(g.NavWindowingTarget->Flags & KGGuiWindowFlags_NoMove))
	{
		KGVec2 nav_move_dir;
		if (g.NavInputSource == KGGuiInputSource_Keyboard && !io.KeyShift)
			nav_move_dir = GetKeyMagnitude2d(KGGuiKey_LeftArrow, KGGuiKey_RightArrow, KGGuiKey_UpArrow, KGGuiKey_DownArrow);
		if (g.NavInputSource == KGGuiInputSource_Gamepad)
			nav_move_dir = GetKeyMagnitude2d(KGGuiKey_GamepadLStickLeft, KGGuiKey_GamepadLStickRight, KGGuiKey_GamepadLStickUp, KGGuiKey_GamepadLStickDown);
		if (nav_move_dir.x != 0.0f || nav_move_dir.y != 0.0f)
		{
			const float NAV_MOVE_SPEED = 800.0f;
			const float move_step = NAV_MOVE_SPEED * io.DeltaTime * KGMin(io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
			g.NavWindowingAccumDeltaPos += nav_move_dir * move_step;
			g.NavDisableMouseHover = true;
			KGVec2 accum_floored = KGFloor(g.NavWindowingAccumDeltaPos);
			if (accum_floored.x != 0.0f || accum_floored.y != 0.0f)
			{
				KGGuiWindow* moving_window = g.NavWindowingTarget->RootWindowDockTree;
				SetWindowPos(moving_window, moving_window->Pos + accum_floored, KGGuiCond_Always);
				g.NavWindowingAccumDeltaPos -= accum_floored;
			}
		}
	}

	// Apply final focus
	if (apply_focus_window && (g.NavWindow == NULL || apply_focus_window != g.NavWindow->RootWindow))
	{
		KarmaGuiViewport* previous_viewport = g.NavWindow ? g.NavWindow->Viewport : NULL;
		ClearActiveID();
		NavRestoreHighlightAfterMove();
		apply_focus_window = NavRestoreLastChildNavWindow(apply_focus_window);
		ClosePopupsOverWindow(apply_focus_window, false);
		FocusWindow(apply_focus_window);
		if (apply_focus_window->NavLastIds[0] == 0)
			NavInitWindow(apply_focus_window, false);

		// If the window has ONLY a menu layer (no main layer), select it directly
		// Use NavLayersActiveMaskNext since windows didn't have a chance to be Begin()-ed on this frame,
		// so CTRL+Tab where the keys are only held for 1 frame will be able to use correct layers mask since
		// the target window as already been previewed once.
		// FIXME-NAV: This should be done in NavInit.. or in FocusWindow... However in both of those cases,
		// we won't have a guarantee that windows has been visible before and therefore NavLayersActiveMask*
		// won't be valid.
		if (apply_focus_window->DC.NavLayersActiveMaskNext == (1 << KGGuiNavLayer_Menu))
			g.NavLayer = KGGuiNavLayer_Menu;

		// Request OS level focus
		if (apply_focus_window->Viewport != previous_viewport && g.PlatformIO.Platform_SetWindowFocus)
			g.PlatformIO.Platform_SetWindowFocus(apply_focus_window->Viewport);
	}
	if (apply_focus_window)
		g.NavWindowingTarget = NULL;

	// Apply menu/layer toggle
	if (apply_toggle_layer && g.NavWindow)
	{
		ClearActiveID();

		// Move to parent menu if necessary
		KGGuiWindow* new_nav_window = g.NavWindow;
		while (new_nav_window->ParentWindow
			&& (new_nav_window->DC.NavLayersActiveMask & (1 << KGGuiNavLayer_Menu)) == 0
			&& (new_nav_window->Flags & KGGuiWindowFlags_ChildWindow) != 0
			&& (new_nav_window->Flags & (KGGuiWindowFlags_Popup | KGGuiWindowFlags_ChildMenu)) == 0)
			new_nav_window = new_nav_window->ParentWindow;
		if (new_nav_window != g.NavWindow)
		{
			KGGuiWindow* old_nav_window = g.NavWindow;
			FocusWindow(new_nav_window);
			new_nav_window->NavLastChildNavWindow = old_nav_window;
		}

		// Toggle layer
		const KGGuiNavLayer new_nav_layer = (g.NavWindow->DC.NavLayersActiveMask & (1 << KGGuiNavLayer_Menu)) ? (KGGuiNavLayer)((int)g.NavLayer ^ 1) : KGGuiNavLayer_Main;
		if (new_nav_layer != g.NavLayer)
		{
			// Reinitialize navigation when entering menu bar with the Alt key (FIXME: could be a properly of the layer?)
			const bool preserve_layer_1_nav_id = (new_nav_window->DockNodeAsHost != NULL);
			if (new_nav_layer == KGGuiNavLayer_Menu && !preserve_layer_1_nav_id)
				g.NavWindow->NavLastIds[new_nav_layer] = 0;
			NavRestoreLayer(new_nav_layer);
			NavRestoreHighlightAfterMove();
		}
	}
}

// Window has already passed the IsWindowNavFocusable()
static const char* GetFallbackWindowNameForWindowingList(KGGuiWindow* window)
{
	if (window->Flags & KGGuiWindowFlags_Popup)
		return Karma::KarmaGuiInternal::LocalizeGetMsg(ImGuiLocKey_WindowingPopup);
	if ((window->Flags & KGGuiWindowFlags_MenuBar) && strcmp(window->Name, "##MainMenuBar") == 0)
		return Karma::KarmaGuiInternal::LocalizeGetMsg(ImGuiLocKey_WindowingMainMenuBar);
	if (window->DockNodeAsHost)
		return "(Dock node)"; // Not normally shown to user.
	return Karma::KarmaGuiInternal::LocalizeGetMsg(ImGuiLocKey_WindowingUntitled);
}

// Overlay displayed when using CTRL+TAB. Called by EndFrame().
void Karma::KarmaGuiInternal::NavUpdateWindowingOverlay()
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(g.NavWindowingTarget != NULL, "");

	if (g.NavWindowingTimer < NAV_WINDOWING_LIST_APPEAR_DELAY)
		return;

	if (g.NavWindowingListWindow == NULL)
		g.NavWindowingListWindow = FindWindowByName("###NavWindowingList");
	const KarmaGuiViewport* viewport = /*g.NavWindow ? g.NavWindow->Viewport :*/ KarmaGui::GetMainViewport();
	KarmaGui::SetNextWindowSizeConstraints(KGVec2(viewport->Size.x * 0.20f, viewport->Size.y * 0.20f), KGVec2(FLT_MAX, FLT_MAX));
	KarmaGui::SetNextWindowPos(viewport->GetCenter(), KGGuiCond_Always, KGVec2(0.5f, 0.5f));
	KarmaGui::PushStyleVar(KGGuiStyleVar_WindowPadding, g.Style.WindowPadding * 2.0f);
	KarmaGui::Begin("###NavWindowingList", NULL, KGGuiWindowFlags_NoTitleBar | KGGuiWindowFlags_NoFocusOnAppearing | KGGuiWindowFlags_NoResize | KGGuiWindowFlags_NoMove | KGGuiWindowFlags_NoInputs | KGGuiWindowFlags_AlwaysAutoResize | KGGuiWindowFlags_NoSavedSettings);
	for (int n = g.WindowsFocusOrder.Size - 1; n >= 0; n--)
	{
		KGGuiWindow* window = g.WindowsFocusOrder[n];
		KR_CORE_ASSERT(window != NULL, ""); // Fix static analyzers
		if (!IsWindowNavFocusable(window))
			continue;
		const char* label = window->Name;
		if (label == FindRenderedTextEnd(label))
			label = GetFallbackWindowNameForWindowingList(window);
		KarmaGui::Selectable(label, g.NavWindowingTarget == window);
	}
	KarmaGui::End();
	KarmaGui::PopStyleVar();
}

//-----------------------------------------------------------------------------
// [SECTION] DRAG AND DROP
//-----------------------------------------------------------------------------

bool Karma::KarmaGuiInternal::IsDragDropActive()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.DragDropActive;
}

void Karma::KarmaGuiInternal::ClearDragDrop()
{
	KarmaGuiContext& g = *GKarmaGui;
	g.DragDropActive = false;
	g.DragDropPayload.Clear();
	g.DragDropAcceptFlags = KGGuiDragDropFlags_None;
	g.DragDropAcceptIdCurr = g.DragDropAcceptIdPrev = 0;
	g.DragDropAcceptIdCurrRectSurface = FLT_MAX;
	g.DragDropAcceptFrameCount = -1;

	g.DragDropPayloadBufHeap.clear();
	memset(&g.DragDropPayloadBufLocal, 0, sizeof(g.DragDropPayloadBufLocal));
}

// When this returns true you need to: a) call SetDragDropPayload() exactly once, b) you may render the payload visual/description, c) call EndDragDropSource()
// If the item has an identifier:
// - This assume/require the item to be activated (typically via ButtonBehavior).
// - Therefore if you want to use this with a mouse button other than left mouse button, it is up to the item itself to activate with another button.
// - We then pull and use the mouse button that was used to activate the item and use it to carry on the drag.
// If the item has no identifier:
// - Currently always assume left mouse button.
bool Karma::KarmaGui::BeginDragDropSource(KarmaGuiDragDropFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;

	// FIXME-DRAGDROP: While in the common-most "drag from non-zero active id" case we can tell the mouse button,
	// in both SourceExtern and id==0 cases we may requires something else (explicit flags or some heuristic).
	KarmaGuiMouseButton mouse_button = KGGuiMouseButton_Left;

	bool source_drag_active = false;
	KGGuiID source_id = 0;
	KGGuiID source_parent_id = 0;
	if (!(flags & KGGuiDragDropFlags_SourceExtern))
	{
		source_id = g.LastItemData.ID;
		if (source_id != 0)
		{
			// Common path: items with ID
			if (g.ActiveId != source_id)
				return false;
			if (g.ActiveIdMouseButton != -1)
				mouse_button = g.ActiveIdMouseButton;
			if (g.IO.MouseDown[mouse_button] == false || window->SkipItems)
				return false;
			g.ActiveIdAllowOverlap = false;
		}
		else
		{
			// Uncommon path: items without ID
			if (g.IO.MouseDown[mouse_button] == false || window->SkipItems)
				return false;
			if ((g.LastItemData.StatusFlags & KGGuiItemStatusFlags_HoveredRect) == 0 && (g.ActiveId == 0 || g.ActiveIdWindow != window))
				return false;

			// If you want to use BeginDragDropSource() on an item with no unique identifier for interaction, such as Text() or Image(), you need to:
			// A) Read the explanation below, B) Use the KGGuiDragDropFlags_SourceAllowNullID flag.
			if (!(flags & KGGuiDragDropFlags_SourceAllowNullID))
			{
				KR_CORE_ASSERT(0, "");
				return false;
			}

			// Magic fallback to handle items with no assigned ID, e.g. Text(), Image()
			// We build a throwaway ID based on current ID stack + relative AABB of items in window.
			// THE IDENTIFIER WON'T SURVIVE ANY REPOSITIONING/RESIZINGG OF THE WIDGET, so if your widget moves your dragging operation will be canceled.
			// We don't need to maintain/call ClearActiveID() as releasing the button will early out this function and trigger !ActiveIdIsAlive.
			// Rely on keeping other window->LastItemXXX fields intact.
			source_id = g.LastItemData.ID = window->GetIDFromRectangle(g.LastItemData.Rect);
			KarmaGuiInternal::KeepAliveID(source_id);
			bool is_hovered = KarmaGuiInternal::ItemHoverable(g.LastItemData.Rect, source_id);
			if (is_hovered && g.IO.MouseClicked[mouse_button])
			{
				KarmaGuiInternal::SetActiveID(source_id, window);
				KarmaGuiInternal::FocusWindow(window);
			}
			if (g.ActiveId == source_id) // Allow the underlying widget to display/return hovered during the mouse release frame, else we would get a flicker.
				g.ActiveIdAllowOverlap = is_hovered;
		}
		if (g.ActiveId != source_id)
			return false;
		source_parent_id = window->IDStack.back();
		source_drag_active = IsMouseDragging(mouse_button);

		// Disable navigation and key inputs while dragging + cancel existing request if any
		KarmaGuiInternal::SetActiveIdUsingAllKeyboardKeys();
	}
	else
	{
		window = NULL;
		source_id = KGHashStr("#SourceExtern");
		source_drag_active = true;
	}

	if (source_drag_active)
	{
		if (!g.DragDropActive)
		{
			KR_CORE_ASSERT(source_id != 0, "");
			KarmaGuiInternal::ClearDragDrop();
			KarmaGuiPayload& payload = g.DragDropPayload;
			payload.SourceId = source_id;
			payload.SourceParentId = source_parent_id;
			g.DragDropActive = true;
			g.DragDropSourceFlags = flags;
			g.DragDropMouseButton = mouse_button;
			if (payload.SourceId == g.ActiveId)
				g.ActiveIdNoClearOnFocusLoss = true;
		}
		g.DragDropSourceFrameCount = g.FrameCount;
		g.DragDropWithinSource = true;

		if (!(flags & KGGuiDragDropFlags_SourceNoPreviewTooltip))
		{
			// Target can request the Source to not display its tooltip (we use a dedicated flag to make this request explicit)
			// We unfortunately can't just modify the source flags and skip the call to BeginTooltip, as caller may be emitting contents.
			BeginTooltip();
			if (g.DragDropAcceptIdPrev && (g.DragDropAcceptFlags & KGGuiDragDropFlags_AcceptNoPreviewTooltip))
			{
				KGGuiWindow* tooltip_window = g.CurrentWindow;
				tooltip_window->Hidden = tooltip_window->SkipItems = true;
				tooltip_window->HiddenFramesCanSkipItems = 1;
			}
		}

		if (!(flags & KGGuiDragDropFlags_SourceNoDisableHover) && !(flags & KGGuiDragDropFlags_SourceExtern))
			g.LastItemData.StatusFlags &= ~KGGuiItemStatusFlags_HoveredRect;

		return true;
	}
	return false;
}

void Karma::KarmaGui::EndDragDropSource()
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(g.DragDropActive, "");
	KR_CORE_ASSERT(g.DragDropWithinSource, "Not after a BeginDragDropSource()?");

	if (!(g.DragDropSourceFlags & KGGuiDragDropFlags_SourceNoPreviewTooltip))
		EndTooltip();

	// Discard the drag if have not called SetDragDropPayload()
	if (g.DragDropPayload.DataFrameCount == -1)
		KarmaGuiInternal::ClearDragDrop();
	g.DragDropWithinSource = false;
}

// Use 'cond' to choose to submit payload on drag start or every frame
bool Karma::KarmaGui::SetDragDropPayload(const char* type, const void* data, size_t data_size, KarmaGuiCond cond)
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiPayload& payload = g.DragDropPayload;
	if (cond == 0)
		cond = KGGuiCond_Always;

	KR_CORE_ASSERT(type != NULL, "");
	KR_CORE_ASSERT(strlen(type) < KG_ARRAYSIZE(payload.DataType), "Payload type can be at most 32 characters long");
	KR_CORE_ASSERT((data != NULL && data_size > 0) || (data == NULL && data_size == 0), "");
	KR_CORE_ASSERT(cond == KGGuiCond_Always || cond == KGGuiCond_Once, "");
	KR_CORE_ASSERT(payload.SourceId != 0, "");                               // Not called between BeginDragDropSource() and EndDragDropSource()

	if (cond == KGGuiCond_Always || payload.DataFrameCount == -1)
	{
		// Copy payload
		KGStrncpy(payload.DataType, type, KG_ARRAYSIZE(payload.DataType));
		g.DragDropPayloadBufHeap.resize(0);
		if (data_size > sizeof(g.DragDropPayloadBufLocal))
		{
			// Store in heap
			g.DragDropPayloadBufHeap.resize((int)data_size);
			payload.Data = g.DragDropPayloadBufHeap.Data;
			memcpy(payload.Data, data, data_size);
		}
		else if (data_size > 0)
		{
			// Store locally
			memset(&g.DragDropPayloadBufLocal, 0, sizeof(g.DragDropPayloadBufLocal));
			payload.Data = g.DragDropPayloadBufLocal;
			memcpy(payload.Data, data, data_size);
		}
		else
		{
			payload.Data = NULL;
		}
		payload.DataSize = (int)data_size;
	}
	payload.DataFrameCount = g.FrameCount;

	// Return whether the payload has been accepted
	return (g.DragDropAcceptFrameCount == g.FrameCount) || (g.DragDropAcceptFrameCount == g.FrameCount - 1);
}

bool Karma::KarmaGuiInternal::BeginDragDropTargetCustom(const KGRect& bb, KGGuiID id)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (!g.DragDropActive)
		return false;

	KGGuiWindow* window = g.CurrentWindow;
	KGGuiWindow* hovered_window = g.HoveredWindowUnderMovingWindow;
	if (hovered_window == NULL || window->RootWindowDockTree != hovered_window->RootWindowDockTree)
		return false;
	KR_CORE_ASSERT(id != 0, "");
	if (!KarmaGui::IsMouseHoveringRect(bb.Min, bb.Max) || (id == g.DragDropPayload.SourceId))
		return false;
	if (window->SkipItems)
		return false;

	KR_CORE_ASSERT(g.DragDropWithinTarget == false, "");
	g.DragDropTargetRect = bb;
	g.DragDropTargetId = id;
	g.DragDropWithinTarget = true;
	return true;
}

// We don't use BeginDragDropTargetCustom() and duplicate its code because:
// 1) we use LastItemRectHoveredRect which handles items that push a temporarily clip rectangle in their code. Calling BeginDragDropTargetCustom(LastItemRect) would not handle them.
// 2) and it's faster. as this code may be very frequently called, we want to early out as fast as we can.
// Also note how the HoveredWindow test is positioned differently in both functions (in both functions we optimize for the cheapest early out case)
bool Karma::KarmaGui::BeginDragDropTarget()
{
	KarmaGuiContext& g = *GKarmaGui;
	if (!g.DragDropActive)
		return false;

	KGGuiWindow* window = g.CurrentWindow;
	if (!(g.LastItemData.StatusFlags & KGGuiItemStatusFlags_HoveredRect))
		return false;
	KGGuiWindow* hovered_window = g.HoveredWindowUnderMovingWindow;
	if (hovered_window == NULL || window->RootWindowDockTree != hovered_window->RootWindowDockTree || window->SkipItems)
		return false;

	const KGRect& display_rect = (g.LastItemData.StatusFlags & KGGuiItemStatusFlags_HasDisplayRect) ? g.LastItemData.DisplayRect : g.LastItemData.Rect;
	KGGuiID id = g.LastItemData.ID;
	if (id == 0)
	{
		id = window->GetIDFromRectangle(display_rect);
		KarmaGuiInternal::KeepAliveID(id);
	}
	if (g.DragDropPayload.SourceId == id)
		return false;

	KR_CORE_ASSERT(g.DragDropWithinTarget == false, "");
	g.DragDropTargetRect = display_rect;
	g.DragDropTargetId = id;
	g.DragDropWithinTarget = true;
	return true;
}

bool Karma::KarmaGuiInternal::IsDragDropPayloadBeingAccepted()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.DragDropActive && g.DragDropAcceptIdPrev != 0;
}

const KarmaGuiPayload* Karma::KarmaGui::AcceptDragDropPayload(const char* type, KarmaGuiDragDropFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	KarmaGuiPayload& payload = g.DragDropPayload;
	KR_CORE_ASSERT(g.DragDropActive, "");                        // Not called between BeginDragDropTarget() and EndDragDropTarget() ?
	KR_CORE_ASSERT(payload.DataFrameCount != -1, "");            // Forgot to call EndDragDropTarget() ?
	if (type != NULL && !payload.IsDataType(type))
		return NULL;

	// Accept smallest drag target bounding box, this allows us to nest drag targets conveniently without ordering constraints.
	// NB: We currently accept NULL id as target. However, overlapping targets requires a unique ID to function!
	const bool was_accepted_previously = (g.DragDropAcceptIdPrev == g.DragDropTargetId);
	KGRect r = g.DragDropTargetRect;
	float r_surface = r.GetWidth() * r.GetHeight();
	if (r_surface <= g.DragDropAcceptIdCurrRectSurface)
	{
		g.DragDropAcceptFlags = flags;
		g.DragDropAcceptIdCurr = g.DragDropTargetId;
		g.DragDropAcceptIdCurrRectSurface = r_surface;
	}

	// Render default drop visuals
	payload.Preview = was_accepted_previously;
	flags |= (g.DragDropSourceFlags & KGGuiDragDropFlags_AcceptNoDrawDefaultRect); // Source can also inhibit the preview (useful for external sources that live for 1 frame)
	if (!(flags & KGGuiDragDropFlags_AcceptNoDrawDefaultRect) && payload.Preview)
		window->DrawList->AddRect(r.Min - KGVec2(3.5f, 3.5f), r.Max + KGVec2(3.5f, 3.5f), GetColorU32(KGGuiCol_DragDropTarget), 0.0f, 0, 2.0f);

	g.DragDropAcceptFrameCount = g.FrameCount;
	payload.Delivery = was_accepted_previously && !IsMouseDown(g.DragDropMouseButton); // For extern drag sources affecting os window focus, it's easier to just test !IsMouseDown() instead of IsMouseReleased()
	if (!payload.Delivery && !(flags & KGGuiDragDropFlags_AcceptBeforeDelivery))
		return NULL;

	return &payload;
}

// FIXME-DRAGDROP: Settle on a proper default visuals for drop target.
void Karma::KarmaGuiInternal::RenderDragDropTargetRect(const KGRect& bb)
{
	KarmaGui::GetWindowDrawList()->AddRect(bb.Min - KGVec2(3.5f, 3.5f), bb.Max + KGVec2(3.5f, 3.5f), KarmaGui::GetColorU32(KGGuiCol_DragDropTarget), 0.0f, 0, 2.0f);
}

const KarmaGuiPayload* Karma::KarmaGui::GetDragDropPayload()
{
	KarmaGuiContext& g = *GKarmaGui;
	return (g.DragDropActive && g.DragDropPayload.DataFrameCount != -1) ? &g.DragDropPayload : NULL;
}

// We don't really use/need this now, but added it for the sake of consistency and because we might need it later.
void Karma::KarmaGui::EndDragDropTarget()
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(g.DragDropActive, "");
	KR_CORE_ASSERT(g.DragDropWithinTarget, "");
	g.DragDropWithinTarget = false;
}

//-----------------------------------------------------------------------------
// [SECTION] LOGGING/CAPTURING
//-----------------------------------------------------------------------------
// All text output from the interface can be captured into tty/file/clipboard.
// By default, tree nodes are automatically opened during logging.
//-----------------------------------------------------------------------------

// Pass text data straight to log (without being displayed)
void Karma::KarmaGui::LogTextV(KarmaGuiContext& g, const char* fmt, va_list args)
{
	if (g.LogFile)
	{
		g.LogBuffer.Buf.resize(0);
		g.LogBuffer.appendfv(fmt, args);
		KGFileWrite(g.LogBuffer.c_str(), sizeof(char), (KGU64)g.LogBuffer.size(), g.LogFile);
	}
	else
	{
		g.LogBuffer.appendfv(fmt, args);
	}
}

void Karma::KarmaGui::LogText(const char* fmt, ...)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (!g.LogEnabled)
		return;

	va_list args;
	va_start(args, fmt);
	LogTextV(g, fmt, args);
	va_end(args);
}

void Karma::KarmaGui::LogTextV(const char* fmt, va_list args)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (!g.LogEnabled)
		return;

	LogTextV(g, fmt, args);
}

// Internal version that takes a position to decide on newline placement and pad items according to their depth.
// We split text into individual lines to add current tree level padding
// FIXME: This code is a little complicated perhaps, considering simplifying the whole system.
void Karma::KarmaGuiInternal::LogRenderedText(const KGVec2* ref_pos, const char* text, const char* text_end)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;

	const char* prefix = g.LogNextPrefix;
	const char* suffix = g.LogNextSuffix;
	g.LogNextPrefix = g.LogNextSuffix = NULL;

	if (!text_end)
		text_end = FindRenderedTextEnd(text, text_end);

	const bool log_new_line = ref_pos && (ref_pos->y > g.LogLinePosY + g.Style.FramePadding.y + 1);
	if (ref_pos)
		g.LogLinePosY = ref_pos->y;
	if (log_new_line)
	{
		KarmaGui::LogText(KG_NEWLINE);
		g.LogLineFirstItem = true;
	}

	if (prefix)
		LogRenderedText(ref_pos, prefix, prefix + strlen(prefix)); // Calculate end ourself to ensure "##" are included here.

	// Re-adjust padding if we have popped out of our starting depth
	if (g.LogDepthRef > window->DC.TreeDepth)
		g.LogDepthRef = window->DC.TreeDepth;
	const int tree_depth = (window->DC.TreeDepth - g.LogDepthRef);

	const char* text_remaining = text;
	for (;;)
	{
		// Split the string. Each new line (after a '\n') is followed by indentation corresponding to the current depth of our log entry.
		// We don't add a trailing \n yet to allow a subsequent item on the same line to be captured.
		const char* line_start = text_remaining;
		const char* line_end = KGStreolRange(line_start, text_end);
		const bool is_last_line = (line_end == text_end);
		if (line_start != line_end || !is_last_line)
		{
			const int line_length = (int)(line_end - line_start);
			const int indentation = g.LogLineFirstItem ? tree_depth * 4 : 1;
			KarmaGui::LogText("%*s%.*s", indentation, "", line_length, line_start);
			g.LogLineFirstItem = false;
			if (*line_end == '\n')
			{
				KarmaGui::LogText(KG_NEWLINE);
				g.LogLineFirstItem = true;
			}
		}
		if (is_last_line)
			break;
		text_remaining = line_end + 1;
	}

	if (suffix)
		LogRenderedText(ref_pos, suffix, suffix + strlen(suffix));
}

// Start logging/capturing text output
void Karma::KarmaGuiInternal::LogBegin(KGGuiLogType type, int auto_open_depth)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	KR_CORE_ASSERT(g.LogEnabled == false, "");
	KR_CORE_ASSERT(g.LogFile == NULL, "");
	KR_CORE_ASSERT(g.LogBuffer.empty(), "");
	g.LogEnabled = true;
	g.LogType = type;
	g.LogNextPrefix = g.LogNextSuffix = NULL;
	g.LogDepthRef = window->DC.TreeDepth;
	g.LogDepthToExpand = ((auto_open_depth >= 0) ? auto_open_depth : g.LogDepthToExpandDefault);
	g.LogLinePosY = FLT_MAX;
	g.LogLineFirstItem = true;
}

// Important: doesn't copy underlying data, use carefully (prefix/suffix must be in scope at the time of the next LogRenderedText)
void Karma::KarmaGuiInternal::LogSetNextTextDecoration(const char* prefix, const char* suffix)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.LogNextPrefix = prefix;
	g.LogNextSuffix = suffix;
}

void Karma::KarmaGui::LogToTTY(int auto_open_depth)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.LogEnabled)
		return;
	KG_UNUSED(auto_open_depth);
#ifndef IMGUI_DISABLE_TTY_FUNCTIONS
	KarmaGuiInternal::LogBegin(KGGuiLogType_TTY, auto_open_depth);
	g.LogFile = stdout;
#endif
}

// Start logging/capturing text output to given file
void Karma::KarmaGui::LogToFile(int auto_open_depth, const char* filename)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.LogEnabled)
		return;

	// FIXME: We could probably open the file in text mode "at", however note that clipboard/buffer logging will still
	// be subject to outputting OS-incompatible carriage return if within strings the user doesn't use KG_NEWLINE.
	// By opening the file in binary mode "ab" we have consistent output everywhere.
	if (!filename)
		filename = g.IO.LogFilename;
	if (!filename || !filename[0])
		return;
	ImFileHandle f = KGFileOpen(filename, "ab");
	if (!f)
	{
		KR_CORE_ASSERT(0, "");
		return;
	}

	KarmaGuiInternal::LogBegin(KGGuiLogType_File, auto_open_depth);
	g.LogFile = f;
}

// Start logging/capturing text output to clipboard
void Karma::KarmaGui::LogToClipboard(int auto_open_depth)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.LogEnabled)
		return;
	KarmaGuiInternal::LogBegin(KGGuiLogType_Clipboard, auto_open_depth);
}

void Karma::KarmaGuiInternal::LogToBuffer(int auto_open_depth)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.LogEnabled)
		return;
	LogBegin(KGGuiLogType_Buffer, auto_open_depth);
}

void Karma::KarmaGui::LogFinish()
{
	KarmaGuiContext& g = *GKarmaGui;
	if (!g.LogEnabled)
		return;

	LogText(KG_NEWLINE);
	switch (g.LogType)
	{
	case KGGuiLogType_TTY:
#ifndef IMGUI_DISABLE_TTY_FUNCTIONS
		fflush(g.LogFile);
#endif
		break;
	case KGGuiLogType_File:
		KGFileClose(g.LogFile);
		break;
	case KGGuiLogType_Buffer:
		break;
	case KGGuiLogType_Clipboard:
		if (!g.LogBuffer.empty())
			SetClipboardText(g.LogBuffer.begin());
		break;
	case KGGuiLogType_None:
		KR_CORE_ASSERT(0, "");
		break;
	}

	g.LogEnabled = false;
	g.LogType = KGGuiLogType_None;
	g.LogFile = NULL;
	g.LogBuffer.clear();
}

// Helper to display logging buttons
// FIXME-OBSOLETE: We should probably obsolete this and let the user have their own helper (this is one of the oldest function alive!)
void Karma::KarmaGui::LogButtons()
{
	KarmaGuiContext& g = *GKarmaGui;

	PushID("LogButtons");
#ifndef IMGUI_DISABLE_TTY_FUNCTIONS
	const bool log_to_tty = Button("Log To TTY"); SameLine();
#else
	const bool log_to_tty = false;
#endif
	const bool log_to_file = Button("Log To File"); SameLine();
	const bool log_to_clipboard = Button("Log To Clipboard"); SameLine();
	PushAllowKeyboardFocus(false);
	SetNextItemWidth(80.0f);
	SliderInt("Default Depth", &g.LogDepthToExpandDefault, 0, 9, NULL);
	PopAllowKeyboardFocus();
	PopID();

	// Start logging at the end of the function so that the buttons don't appear in the log
	if (log_to_tty)
		LogToTTY();
	if (log_to_file)
		LogToFile();
	if (log_to_clipboard)
		LogToClipboard();
}

//-----------------------------------------------------------------------------
// [SECTION] SETTINGS
//-----------------------------------------------------------------------------
// - UpdateSettings() [Internal]
// - MarkIniSettingsDirty() [Internal]
// - CreateNewWindowSettings() [Internal]
// - FindWindowSettings() [Internal]
// - FindOrCreateWindowSettings() [Internal]
// - FindSettingsHandler() [Internal]
// - ClearIniSettings() [Internal]
// - LoadIniSettingsFromDisk()
// - LoadIniSettingsFromMemory()
// - SaveIniSettingsToDisk()
// - SaveIniSettingsToMemory()
// - WindowSettingsHandler_***() [Internal]
//-----------------------------------------------------------------------------

// Called by NewFrame()
void Karma::KarmaGuiInternal::UpdateSettings()
{
	// Load settings on first frame (if not explicitly loaded manually before)
	KarmaGuiContext& g = *GKarmaGui;
	if (!g.SettingsLoaded)
	{
		KR_CORE_ASSERT(g.SettingsWindows.empty(), "");
		if (g.IO.IniFilename)
			KarmaGui::LoadIniSettingsFromDisk(g.IO.IniFilename);
		g.SettingsLoaded = true;
	}

	// Save settings (with a delay after the last modification, so we don't spam disk too much)
	if (g.SettingsDirtyTimer > 0.0f)
	{
		g.SettingsDirtyTimer -= g.IO.DeltaTime;
		if (g.SettingsDirtyTimer <= 0.0f)
		{
			if (g.IO.IniFilename != NULL)
				KarmaGui::SaveIniSettingsToDisk(g.IO.IniFilename);
			else
				g.IO.WantSaveIniSettings = true;  // Let user know they can call SaveIniSettingsToMemory(). user will need to clear io.WantSaveIniSettings themselves.
			g.SettingsDirtyTimer = 0.0f;
		}
	}
}

void Karma::KarmaGuiInternal::MarkIniSettingsDirty()
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.SettingsDirtyTimer <= 0.0f)
		g.SettingsDirtyTimer = g.IO.IniSavingRate;
}

void Karma::KarmaGuiInternal::MarkIniSettingsDirty(KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (!(window->Flags & KGGuiWindowFlags_NoSavedSettings))
		if (g.SettingsDirtyTimer <= 0.0f)
			g.SettingsDirtyTimer = g.IO.IniSavingRate;
}

KGGuiWindowSettings* Karma::KarmaGuiInternal::CreateNewWindowSettings(const char* name)
{
	KarmaGuiContext& g = *GKarmaGui;

#if !KARMAGUI_DEBUG_INI_SETTINGS
	// Skip to the "###" marker if any. We don't skip past to match the behavior of GetID()
	// Preserve the full string when KARMAGUI_DEBUG_INI_SETTINGS is set to make .ini inspection easier.
	if (const char* p = strstr(name, "###"))
		name = p;
#endif
	const size_t name_len = strlen(name);

	// Allocate chunk
	const size_t chunk_size = sizeof(KGGuiWindowSettings) + name_len + 1;
	KGGuiWindowSettings* settings = g.SettingsWindows.alloc_chunk(chunk_size);
	KG_PLACEMENT_NEW(settings) KGGuiWindowSettings();
	settings->ID = KGHashStr(name, name_len);
	memcpy(settings->GetName(), name, name_len + 1);   // Store with zero terminator

	return settings;
}

KGGuiWindowSettings* Karma::KarmaGuiInternal::FindWindowSettings(KGGuiID id)
{
	KarmaGuiContext& g = *GKarmaGui;
	for (KGGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
		if (settings->ID == id)
			return settings;
	return NULL;
}

KGGuiWindowSettings* Karma::KarmaGuiInternal::FindOrCreateWindowSettings(const char* name)
{
	if (KGGuiWindowSettings* settings = FindWindowSettings(KGHashStr(name)))
		return settings;
	return CreateNewWindowSettings(name);
}

void Karma::KarmaGuiInternal::AddSettingsHandler(const KGGuiSettingsHandler* handler)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(FindSettingsHandler(handler->TypeName) == NULL, "");
	g.SettingsHandlers.push_back(*handler);
}

void Karma::KarmaGuiInternal::RemoveSettingsHandler(const char* type_name)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (KGGuiSettingsHandler* handler = FindSettingsHandler(type_name))
		g.SettingsHandlers.erase(handler);
}

KGGuiSettingsHandler* Karma::KarmaGuiInternal::FindSettingsHandler(const char* type_name)
{
	KarmaGuiContext& g = *GKarmaGui;
	const KGGuiID type_hash = KGHashStr(type_name);
	for (int handler_n = 0; handler_n < g.SettingsHandlers.Size; handler_n++)
		if (g.SettingsHandlers[handler_n].TypeHash == type_hash)
			return &g.SettingsHandlers[handler_n];
	return NULL;
}

void Karma::KarmaGuiInternal::ClearIniSettings()
{
	KarmaGuiContext& g = *GKarmaGui;
	g.SettingsIniData.clear();
	for (int handler_n = 0; handler_n < g.SettingsHandlers.Size; handler_n++)
		if (g.SettingsHandlers[handler_n].ClearAllFn)
			g.SettingsHandlers[handler_n].ClearAllFn(&g, &g.SettingsHandlers[handler_n]);
}

void Karma::KarmaGui::LoadIniSettingsFromDisk(const char* ini_filename)
{
	size_t file_data_size = 0;
	char* file_data = (char*)KGFileLoadToMemory(ini_filename, "rb", &file_data_size);
	if (!file_data)
		return;
	if (file_data_size > 0)
		LoadIniSettingsFromMemory(file_data, (size_t)file_data_size);
	KG_FREE(file_data);
}

// Zero-tolerance, no error reporting, cheap .ini parsing
void Karma::KarmaGui::LoadIniSettingsFromMemory(const char* ini_data, size_t ini_size)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(g.Initialized, "");
	//KR_CORE_ASSERT(!g.WithinFrameScope && "Cannot be called between NewFrame() and EndFrame()");
	//KR_CORE_ASSERT(g.SettingsLoaded == false && g.FrameCount == 0);

	// For user convenience, we allow passing a non zero-terminated string (hence the ini_size parameter).
	// For our convenience and to make the code simpler, we'll also write zero-terminators within the buffer. So let's create a writable copy..
	if (ini_size == 0)
		ini_size = strlen(ini_data);
	g.SettingsIniData.Buf.resize((int)ini_size + 1);
	char* const buf = g.SettingsIniData.Buf.Data;
	char* const buf_end = buf + ini_size;
	memcpy(buf, ini_data, ini_size);
	buf_end[0] = 0;

	// Call pre-read handlers
	// Some types will clear their data (e.g. dock information) some types will allow merge/override (window)
	for (int handler_n = 0; handler_n < g.SettingsHandlers.Size; handler_n++)
		if (g.SettingsHandlers[handler_n].ReadInitFn)
			g.SettingsHandlers[handler_n].ReadInitFn(&g, &g.SettingsHandlers[handler_n]);

	void* entry_data = NULL;
	KGGuiSettingsHandler* entry_handler = NULL;

	char* line_end = NULL;
	for (char* line = buf; line < buf_end; line = line_end + 1)
	{
		// Skip new lines markers, then find end of the line
		while (*line == '\n' || *line == '\r')
			line++;
		line_end = line;
		while (line_end < buf_end && *line_end != '\n' && *line_end != '\r')
			line_end++;
		line_end[0] = 0;
		if (line[0] == ';')
			continue;
		if (line[0] == '[' && line_end > line && line_end[-1] == ']')
		{
			// Parse "[Type][Name]". Note that 'Name' can itself contains [] characters, which is acceptable with the current format and parsing code.
			line_end[-1] = 0;
			const char* name_end = line_end - 1;
			const char* type_start = line + 1;
			char* type_end = (char*)(void*)KGStrchrRange(type_start, name_end, ']');
			const char* name_start = type_end ? KGStrchrRange(type_end + 1, name_end, '[') : NULL;
			if (!type_end || !name_start)
				continue;
			*type_end = 0; // Overwrite first ']'
			name_start++;  // Skip second '['
			entry_handler = KarmaGuiInternal::FindSettingsHandler(type_start);
			entry_data = entry_handler ? entry_handler->ReadOpenFn(&g, entry_handler, name_start) : NULL;
		}
		else if (entry_handler != NULL && entry_data != NULL)
		{
			// Let type handler parse the line
			entry_handler->ReadLineFn(&g, entry_handler, entry_data, line);
		}
	}
	g.SettingsLoaded = true;

	// [DEBUG] Restore untouched copy so it can be browsed in Metrics (not strictly necessary)
	memcpy(buf, ini_data, ini_size);

	// Call post-read handlers
	for (int handler_n = 0; handler_n < g.SettingsHandlers.Size; handler_n++)
		if (g.SettingsHandlers[handler_n].ApplyAllFn)
			g.SettingsHandlers[handler_n].ApplyAllFn(&g, &g.SettingsHandlers[handler_n]);
}

void Karma::KarmaGui::SaveIniSettingsToDisk(const char* ini_filename)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.SettingsDirtyTimer = 0.0f;
	if (!ini_filename)
		return;

	size_t ini_data_size = 0;
	const char* ini_data = SaveIniSettingsToMemory(&ini_data_size);
	ImFileHandle f = KGFileOpen(ini_filename, "wt");
	if (!f)
		return;
	KGFileWrite(ini_data, sizeof(char), ini_data_size, f);
	KGFileClose(f);
}

// Call registered handlers (e.g. SettingsHandlerWindow_WriteAll() + custom handlers) to write their stuff into a text buffer
const char* Karma::KarmaGui::SaveIniSettingsToMemory(size_t* out_size)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.SettingsDirtyTimer = 0.0f;
	g.SettingsIniData.Buf.resize(0);
	g.SettingsIniData.Buf.push_back(0);
	for (int handler_n = 0; handler_n < g.SettingsHandlers.Size; handler_n++)
	{
		KGGuiSettingsHandler* handler = &g.SettingsHandlers[handler_n];
		handler->WriteAllFn(&g, handler, &g.SettingsIniData);
	}
	if (out_size)
		*out_size = (size_t)g.SettingsIniData.size();
	return g.SettingsIniData.c_str();
}

void Karma::KarmaGuiInternal::WindowSettingsHandler_ClearAll(KarmaGuiContext* ctx, KGGuiSettingsHandler*)
{
	KarmaGuiContext& g = *ctx;
	for (int i = 0; i != g.Windows.Size; i++)
		g.Windows[i]->SettingsOffset = -1;
	g.SettingsWindows.clear();
}

void* Karma::KarmaGuiInternal::WindowSettingsHandler_ReadOpen(KarmaGuiContext*, KGGuiSettingsHandler*, const char* name)
{
	KGGuiWindowSettings* settings = Karma::KarmaGuiInternal::FindOrCreateWindowSettings(name);
	KGGuiID id = settings->ID;
	*settings = KGGuiWindowSettings(); // Clear existing if recycling previous entry
	settings->ID = id;
	settings->WantApply = true;
	return (void*)settings;
}

void Karma::KarmaGuiInternal::WindowSettingsHandler_ReadLine(KarmaGuiContext*, KGGuiSettingsHandler*, void* entry, const char* line)
{
	KGGuiWindowSettings* settings = (KGGuiWindowSettings*)entry;
	int x, y;
	int i;
	KGU32 u1;
	if (sscanf(line, "Pos=%i,%i", &x, &y) == 2) { settings->Pos = KGVec2ih((short)x, (short)y); }
	else if (sscanf(line, "Size=%i,%i", &x, &y) == 2) { settings->Size = KGVec2ih((short)x, (short)y); }
	else if (sscanf(line, "ViewportId=0x%08X", &u1) == 1) { settings->ViewportId = u1; }
	else if (sscanf(line, "ViewportPos=%i,%i", &x, &y) == 2) { settings->ViewportPos = KGVec2ih((short)x, (short)y); }
	else if (sscanf(line, "Collapsed=%d", &i) == 1) { settings->Collapsed = (i != 0); }
	else if (sscanf(line, "DockId=0x%X,%d", &u1, &i) == 2) { settings->DockId = u1; settings->DockOrder = (short)i; }
	else if (sscanf(line, "DockId=0x%X", &u1) == 1) { settings->DockId = u1; settings->DockOrder = -1; }
	else if (sscanf(line, "ClassId=0x%X", &u1) == 1) { settings->ClassId = u1; }
}

// Apply to existing windows (if any)
void Karma::KarmaGuiInternal::WindowSettingsHandler_ApplyAll(KarmaGuiContext* ctx, KGGuiSettingsHandler*)
{
	KarmaGuiContext& g = *ctx;
	for (KGGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
		if (settings->WantApply)
		{
			if (KGGuiWindow* window = Karma::KarmaGuiInternal::FindWindowByID(settings->ID))
				ApplyWindowSettings(window, settings);
			settings->WantApply = false;
		}
}

void Karma::KarmaGuiInternal::WindowSettingsHandler_WriteAll(KarmaGuiContext* ctx, KGGuiSettingsHandler* handler, KarmaGuiTextBuffer* buf)
{
	// Gather data from windows that were active during this session
	// (if a window wasn't opened in this session we preserve its settings)
	KarmaGuiContext& g = *ctx;
	for (int i = 0; i != g.Windows.Size; i++)
	{
		KGGuiWindow* window = g.Windows[i];
		if (window->Flags & KGGuiWindowFlags_NoSavedSettings)
			continue;

		KGGuiWindowSettings* settings = (window->SettingsOffset != -1) ? g.SettingsWindows.ptr_from_offset(window->SettingsOffset) : Karma::KarmaGuiInternal::FindWindowSettings(window->ID);
		if (!settings)
		{
			settings = Karma::KarmaGuiInternal::CreateNewWindowSettings(window->Name);
			window->SettingsOffset = g.SettingsWindows.offset_from_ptr(settings);
		}
		KR_CORE_ASSERT(settings->ID == window->ID, "");
		settings->Pos = KGVec2ih(window->Pos - window->ViewportPos);
		settings->Size = KGVec2ih(window->SizeFull);
		settings->ViewportId = window->ViewportId;
		settings->ViewportPos = KGVec2ih(window->ViewportPos);
		KR_CORE_ASSERT(window->DockNode == NULL || window->DockNode->ID == window->DockId, "");
		settings->DockId = window->DockId;
		settings->ClassId = window->WindowClass.ClassId;
		settings->DockOrder = window->DockOrder;
		settings->Collapsed = window->Collapsed;
	}

	// Write to text buffer
	buf->reserve(buf->size() + g.SettingsWindows.size() * 6); // ballpark reserve
	for (KGGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
	{
		const char* settings_name = settings->GetName();
		buf->appendf("[%s][%s]\n", handler->TypeName, settings_name);
		if (settings->ViewportId != 0 && settings->ViewportId != Karma::KarmaGuiInternal::IMGUI_VIEWPORT_DEFAULT_ID)
		{
			buf->appendf("ViewportPos=%d,%d\n", settings->ViewportPos.x, settings->ViewportPos.y);
			buf->appendf("ViewportId=0x%08X\n", settings->ViewportId);
		}
		if (settings->Pos.x != 0 || settings->Pos.y != 0 || settings->ViewportId == Karma::KarmaGuiInternal::IMGUI_VIEWPORT_DEFAULT_ID)
			buf->appendf("Pos=%d,%d\n", settings->Pos.x, settings->Pos.y);
		if (settings->Size.x != 0 || settings->Size.y != 0)
			buf->appendf("Size=%d,%d\n", settings->Size.x, settings->Size.y);
		buf->appendf("Collapsed=%d\n", settings->Collapsed);
		if (settings->DockId != 0)
		{
			//buf->appendf("TabId=0x%08X\n", KGHashStr("#TAB", 4, settings->ID)); // window->TabId: this is not read back but writing it makes "debugging" the .ini data easier.
			if (settings->DockOrder == -1)
				buf->appendf("DockId=0x%08X\n", settings->DockId);
			else
				buf->appendf("DockId=0x%08X,%d\n", settings->DockId, settings->DockOrder);
			if (settings->ClassId != 0)
				buf->appendf("ClassId=0x%08X\n", settings->ClassId);
		}
		buf->append("\n");
	}
}

//-----------------------------------------------------------------------------
// [SECTION] LOCALIZATION
//-----------------------------------------------------------------------------

void Karma::KarmaGuiInternal::LocalizeRegisterEntries(const KGGuiLocEntry* entries, int count)
{
	KarmaGuiContext& g = *GKarmaGui;
	for (int n = 0; n < count; n++)
		g.LocalizationTable[entries[n].Key] = entries[n].Text;
}

//-----------------------------------------------------------------------------
// [SECTION] VIEWPORTS, PLATFORM WINDOWS
//-----------------------------------------------------------------------------
// - GetMainViewport()
// - FindViewportByID()
// - FindViewportByPlatformHandle()
// - SetCurrentViewport() [Internal]
// - SetWindowViewport() [Internal]
// - GetWindowAlwaysWantOwnViewport() [Internal]
// - UpdateTryMergeWindowIntoHostViewport() [Internal]
// - UpdateTryMergeWindowIntoHostViewports() [Internal]
// - TranslateWindowsInViewport() [Internal]
// - ScaleWindowsInViewport() [Internal]
// - FindHoveredViewportFromPlatformWindowStack() [Internal]
// - UpdateViewportsNewFrame() [Internal]
// - UpdateViewportsEndFrame() [Internal]
// - AddUpdateViewport() [Internal]
// - WindowSelectViewport() [Internal]
// - WindowSyncOwnedViewport() [Internal]
// - UpdatePlatformWindows()
// - RenderPlatformWindowsDefault()
// - FindPlatformMonitorForPos() [Internal]
// - FindPlatformMonitorForRect() [Internal]
// - UpdateViewportPlatformMonitor() [Internal]
// - DestroyPlatformWindow() [Internal]
// - DestroyPlatformWindows()
//-----------------------------------------------------------------------------

KarmaGuiViewport* Karma::KarmaGui::GetMainViewport()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.Viewports[0];
}

// FIXME: This leaks access to viewports not listed in PlatformIO.Viewports[]. Problematic? (#4236)
KarmaGuiViewport* Karma::KarmaGui::FindViewportByID(KGGuiID id)
{
	KarmaGuiContext& g = *GKarmaGui;
	for (int n = 0; n < g.Viewports.Size; n++)
		if (g.Viewports[n]->ID == id)
			return g.Viewports[n];
	return NULL;
}

KarmaGuiViewport* Karma::KarmaGui::FindViewportByPlatformHandle(void* platform_handle)
{
	KarmaGuiContext& g = *GKarmaGui;
	for (int i = 0; i != g.Viewports.Size; i++)
		if (g.Viewports[i]->PlatformHandle == platform_handle)
			return g.Viewports[i];
	return NULL;
}

void Karma::KarmaGuiInternal::SetCurrentViewport(KGGuiWindow* current_window, KGGuiViewportP* viewport)
{
	KarmaGuiContext& g = *GKarmaGui;
	(void)current_window;

	if (viewport)
		viewport->LastFrameActive = g.FrameCount;
	if (g.CurrentViewport == viewport)
		return;
	g.CurrentDpiScale = viewport ? viewport->DpiScale : 1.0f;
	g.CurrentViewport = viewport;
	//KARMAGUI_DEBUG_LOG_VIEWPORT("[viewport] SetCurrentViewport changed '%s' 0x%08X\n", current_window ? current_window->Name : NULL, viewport ? viewport->ID : 0);

	// Notify platform layer of viewport changes
	// FIXME-DPI: This is only currently used for experimenting with handling of multiple DPI
	if (g.CurrentViewport && g.PlatformIO.Platform_OnChangedViewport)
		g.PlatformIO.Platform_OnChangedViewport(g.CurrentViewport);
}

void Karma::KarmaGuiInternal::SetWindowViewport(KGGuiWindow* window, KGGuiViewportP* viewport)
{
	// Abandon viewport
	if (window->ViewportOwned && window->Viewport->Window == window)
		window->Viewport->Size = KGVec2(0.0f, 0.0f);

	window->Viewport = viewport;
	window->ViewportId = viewport->ID;
	window->ViewportOwned = (viewport->Window == window);
}

bool Karma::KarmaGuiInternal::GetWindowAlwaysWantOwnViewport(KGGuiWindow* window)
{
	// Tooltips and menus are not automatically forced into their own viewport when the NoMerge flag is set, however the multiplication of viewports makes them more likely to protrude and create their own.
	KarmaGuiContext& g = *GKarmaGui;
	if (g.IO.ConfigViewportsNoAutoMerge || (window->WindowClass.ViewportFlagsOverrideSet & KGGuiViewportFlags_NoAutoMerge))
		if (g.ConfigFlagsCurrFrame & KGGuiConfigFlags_ViewportsEnable)
			if (!window->DockIsActive)
				if ((window->Flags & (KGGuiWindowFlags_ChildWindow | KGGuiWindowFlags_ChildMenu | KGGuiWindowFlags_Tooltip)) == 0)
					if ((window->Flags & KGGuiWindowFlags_Popup) == 0 || (window->Flags & KGGuiWindowFlags_Modal) != 0)
						return true;
	return false;
}

bool Karma::KarmaGuiInternal::UpdateTryMergeWindowIntoHostViewport(KGGuiWindow* window, KGGuiViewportP* viewport)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (window->Viewport == viewport)
		return false;
	if ((viewport->Flags & KGGuiViewportFlags_CanHostOtherWindows) == 0)
		return false;
	if ((viewport->Flags & KGGuiViewportFlags_Minimized) != 0)
		return false;
	if (!viewport->GetMainRect().Contains(window->Rect()))
		return false;
	if (GetWindowAlwaysWantOwnViewport(window))
		return false;

	// FIXME: Can't use g.WindowsFocusOrder[] for root windows only as we care about Z order. If we maintained a DisplayOrder along with FocusOrder we could..
	for (int n = 0; n < g.Windows.Size; n++)
	{
		KGGuiWindow* window_behind = g.Windows[n];
		if (window_behind == window)
			break;
		if (window_behind->WasActive && window_behind->ViewportOwned && !(window_behind->Flags & KGGuiWindowFlags_ChildWindow))
			if (window_behind->Viewport->GetMainRect().Overlaps(window->Rect()))
				return false;
	}

	// Move to the existing viewport, Move child/hosted windows as well (FIXME-OPT: iterate child)
	KGGuiViewportP* old_viewport = window->Viewport;
	if (window->ViewportOwned)
		for (int n = 0; n < g.Windows.Size; n++)
			if (g.Windows[n]->Viewport == old_viewport)
				SetWindowViewport(g.Windows[n], viewport);
	SetWindowViewport(window, viewport);
	BringWindowToDisplayFront(window);

	return true;
}

// FIXME: handle 0 to N host viewports
bool Karma::KarmaGuiInternal::UpdateTryMergeWindowIntoHostViewports(KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;
	return UpdateTryMergeWindowIntoHostViewport(window, g.Viewports[0]);
}

// Translate Dear ImGui windows when a Host Viewport has been moved
// (This additionally keeps windows at the same place when KGGuiConfigFlags_ViewportsEnable is toggled!)
void Karma::KarmaGuiInternal::TranslateWindowsInViewport(KGGuiViewportP* viewport, const KGVec2& old_pos, const KGVec2& new_pos)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(viewport->Window == NULL && (viewport->Flags & KGGuiViewportFlags_CanHostOtherWindows), "");

	// 1) We test if KGGuiConfigFlags_ViewportsEnable was just toggled, which allows us to conveniently
	// translate imgui windows from OS-window-local to absolute coordinates or vice-versa.
	// 2) If it's not going to fit into the new size, keep it at same absolute position.
	// One problem with this is that most Win32 applications doesn't update their render while dragging,
	// and so the window will appear to teleport when releasing the mouse.
	const bool translate_all_windows = (g.ConfigFlagsCurrFrame & KGGuiConfigFlags_ViewportsEnable) != (g.ConfigFlagsLastFrame & KGGuiConfigFlags_ViewportsEnable);
	KGRect test_still_fit_rect(old_pos, old_pos + viewport->Size);
	KGVec2 delta_pos = new_pos - old_pos;
	for (int window_n = 0; window_n < g.Windows.Size; window_n++) // FIXME-OPT
		if (translate_all_windows || (g.Windows[window_n]->Viewport == viewport && test_still_fit_rect.Contains(g.Windows[window_n]->Rect())))
			TranslateWindow(g.Windows[window_n], delta_pos);
}

// Scale all windows (position, size). Use when e.g. changing DPI. (This is a lossy operation!)
void Karma::KarmaGuiInternal::ScaleWindowsInViewport(KGGuiViewportP* viewport, float scale)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (viewport->Window)
	{
		ScaleWindow(viewport->Window, scale);
	}
	else
	{
		for (int i = 0; i != g.Windows.Size; i++)
			if (g.Windows[i]->Viewport == viewport)
				ScaleWindow(g.Windows[i], scale);
	}
}

// If the backend doesn't set MouseLastHoveredViewport or doesn't honor KGGuiViewportFlags_NoInputs, we do a search ourselves.
// A) It won't take account of the possibility that non-imgui windows may be in-between our dragged window and our target window.
// B) It requires Platform_GetWindowFocus to be implemented by backend.
KGGuiViewportP* Karma::KarmaGuiInternal::FindHoveredViewportFromPlatformWindowStack(const KGVec2& mouse_platform_pos)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiViewportP* best_candidate = NULL;
	for (int n = 0; n < g.Viewports.Size; n++)
	{
		KGGuiViewportP* viewport = g.Viewports[n];
		if (!(viewport->Flags & (KGGuiViewportFlags_NoInputs | KGGuiViewportFlags_Minimized)) && viewport->GetMainRect().Contains(mouse_platform_pos))
			if (best_candidate == NULL || best_candidate->LastFrontMostStampCount < viewport->LastFrontMostStampCount)
				best_candidate = viewport;
	}
	return best_candidate;
}

// Update viewports and monitor infos
// Note that this is running even if 'KGGuiConfigFlags_ViewportsEnable' is not set, in order to clear unused viewports (if any) and update monitor info.
void Karma::KarmaGuiInternal::UpdateViewportsNewFrame()
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(g.PlatformIO.Viewports.Size <= g.Viewports.Size, "");

	// Update Minimized status (we need it first in order to decide if we'll apply Pos/Size of the main viewport)
	const bool viewports_enabled = (g.ConfigFlagsCurrFrame & KGGuiConfigFlags_ViewportsEnable) != 0;
	if (viewports_enabled)
	{
		for (int n = 0; n < g.Viewports.Size; n++)
		{
			KGGuiViewportP* viewport = g.Viewports[n];
			const bool platform_funcs_available = viewport->PlatformWindowCreated;
			if (g.PlatformIO.Platform_GetWindowMinimized && platform_funcs_available)
			{
				bool minimized = g.PlatformIO.Platform_GetWindowMinimized(viewport);
				if (minimized)
					viewport->Flags |= KGGuiViewportFlags_Minimized;
				else
					viewport->Flags &= ~KGGuiViewportFlags_Minimized;
			}
		}
	}

	// Create/update main viewport with current platform position.
	// FIXME-VIEWPORT: Size is driven by backend/user code for backward-compatibility but we should aim to make this more consistent.
	KGGuiViewportP* main_viewport = g.Viewports[0];
	KR_CORE_ASSERT(main_viewport->ID == IMGUI_VIEWPORT_DEFAULT_ID, "");
	KR_CORE_ASSERT(main_viewport->Window == NULL, "");
	KGVec2 main_viewport_pos = viewports_enabled ? g.PlatformIO.Platform_GetWindowPos(main_viewport) : KGVec2(0.0f, 0.0f);
	KGVec2 main_viewport_size = g.IO.DisplaySize;
	if (viewports_enabled && (main_viewport->Flags & KGGuiViewportFlags_Minimized))
	{
		main_viewport_pos = main_viewport->Pos;    // Preserve last pos/size when minimized (FIXME: We don't do the same for Size outside of the viewport path)
		main_viewport_size = main_viewport->Size;
	}
	AddUpdateViewport(NULL, IMGUI_VIEWPORT_DEFAULT_ID, main_viewport_pos, main_viewport_size, KGGuiViewportFlags_OwnedByApp | KGGuiViewportFlags_CanHostOtherWindows);

	g.CurrentDpiScale = 0.0f;
	g.CurrentViewport = NULL;
	g.MouseViewport = NULL;
	for (int n = 0; n < g.Viewports.Size; n++)
	{
		KGGuiViewportP* viewport = g.Viewports[n];
		viewport->Idx = n;

		// Erase unused viewports
		if (n > 0 && viewport->LastFrameActive < g.FrameCount - 2)
		{
			DestroyViewport(viewport);
			n--;
			continue;
		}

		const bool platform_funcs_available = viewport->PlatformWindowCreated;
		if (viewports_enabled)
		{
			// Update Position and Size (from Platform Window to ImGui) if requested.
			// We do it early in the frame instead of waiting for UpdatePlatformWindows() to avoid a frame of lag when moving/resizing using OS facilities.
			if (!(viewport->Flags & KGGuiViewportFlags_Minimized) && platform_funcs_available)
			{
				// Viewport->WorkPos and WorkSize will be updated below
				if (viewport->PlatformRequestMove)
					viewport->Pos = viewport->LastPlatformPos = g.PlatformIO.Platform_GetWindowPos(viewport);
				if (viewport->PlatformRequestResize)
					viewport->Size = viewport->LastPlatformSize = g.PlatformIO.Platform_GetWindowSize(viewport);
			}
		}

		// Update/copy monitor info
		UpdateViewportPlatformMonitor(viewport);

		// Lock down space taken by menu bars and status bars, reset the offset for functions like BeginMainMenuBar() to alter them again.
		viewport->WorkOffsetMin = viewport->BuildWorkOffsetMin;
		viewport->WorkOffsetMax = viewport->BuildWorkOffsetMax;
		viewport->BuildWorkOffsetMin = viewport->BuildWorkOffsetMax = KGVec2(0.0f, 0.0f);
		viewport->UpdateWorkRect();

		// Reset alpha every frame. Users of transparency (docking) needs to request a lower alpha back.
		viewport->Alpha = 1.0f;

		// Translate Dear ImGui windows when a Host Viewport has been moved
		// (This additionally keeps windows at the same place when KGGuiConfigFlags_ViewportsEnable is toggled!)
		const KGVec2 viewport_delta_pos = viewport->Pos - viewport->LastPos;
		if ((viewport->Flags & KGGuiViewportFlags_CanHostOtherWindows) && (viewport_delta_pos.x != 0.0f || viewport_delta_pos.y != 0.0f))
			TranslateWindowsInViewport(viewport, viewport->LastPos, viewport->Pos);

		// Update DPI scale
		float new_dpi_scale;
		if (g.PlatformIO.Platform_GetWindowDpiScale && platform_funcs_available)
			new_dpi_scale = g.PlatformIO.Platform_GetWindowDpiScale(viewport);
		else if (viewport->PlatformMonitor != -1)
			new_dpi_scale = g.PlatformIO.Monitors[viewport->PlatformMonitor].DpiScale;
		else
			new_dpi_scale = (viewport->DpiScale != 0.0f) ? viewport->DpiScale : 1.0f;
		if (viewport->DpiScale != 0.0f && new_dpi_scale != viewport->DpiScale)
		{
			float scale_factor = new_dpi_scale / viewport->DpiScale;
			if (g.IO.ConfigFlags & KGGuiConfigFlags_DpiEnableScaleViewports)
				ScaleWindowsInViewport(viewport, scale_factor);
			//if (viewport == GetMainViewport())
			//    g.PlatformInterface.SetWindowSize(viewport, viewport->Size * scale_factor);

			// Scale our window moving pivot so that the window will rescale roughly around the mouse position.
			// FIXME-VIEWPORT: This currently creates a resizing feedback loop when a window is straddling a DPI transition border.
			// (Minor: since our sizes do not perfectly linearly scale, deferring the click offset scale until we know the actual window scale ratio may get us slightly more precise mouse positioning.)
			//if (g.MovingWindow != NULL && g.MovingWindow->Viewport == viewport)
			//    g.ActiveIdClickOffset = KGFloor(g.ActiveIdClickOffset * scale_factor);
		}
		viewport->DpiScale = new_dpi_scale;
	}

	// Update fallback monitor
	if (g.PlatformIO.Monitors.Size == 0)
	{
		KarmaGuiPlatformMonitor* monitor = &g.FallbackMonitor;
		monitor->MainPos = main_viewport->Pos;
		monitor->MainSize = main_viewport->Size;
		monitor->WorkPos = main_viewport->WorkPos;
		monitor->WorkSize = main_viewport->WorkSize;
		monitor->DpiScale = main_viewport->DpiScale;
	}

	if (!viewports_enabled)
	{
		g.MouseViewport = main_viewport;
		return;
	}

	// Mouse handling: decide on the actual mouse viewport for this frame between the active/focused viewport and the hovered viewport.
	// Note that 'viewport_hovered' should skip over any viewport that has the KGGuiViewportFlags_NoInputs flags set.
	KGGuiViewportP* viewport_hovered = NULL;
	if (g.IO.BackendFlags & KGGuiBackendFlags_HasMouseHoveredViewport)
	{
		viewport_hovered = g.IO.MouseHoveredViewport ? (KGGuiViewportP*)KarmaGui::FindViewportByID(g.IO.MouseHoveredViewport) : NULL;
		if (viewport_hovered && (viewport_hovered->Flags & KGGuiViewportFlags_NoInputs))
			viewport_hovered = FindHoveredViewportFromPlatformWindowStack(g.IO.MousePos); // Backend failed to handle _NoInputs viewport: revert to our fallback.
	}
	else
	{
		// If the backend doesn't know how to honor KGGuiViewportFlags_NoInputs, we do a search ourselves. Note that this search:
		// A) won't take account of the possibility that non-imgui windows may be in-between our dragged window and our target window.
		// B) won't take account of how the backend apply parent<>child relationship to secondary viewports, which affects their Z order.
		// C) uses LastFrameAsRefViewport as a flawed replacement for the last time a window was focused (we could/should fix that by introducing Focus functions in PlatformIO)
		viewport_hovered = FindHoveredViewportFromPlatformWindowStack(g.IO.MousePos);
	}
	if (viewport_hovered != NULL)
		g.MouseLastHoveredViewport = viewport_hovered;
	else if (g.MouseLastHoveredViewport == NULL)
		g.MouseLastHoveredViewport = g.Viewports[0];

	// Update mouse reference viewport
	// (when moving a window we aim at its viewport, but this will be overwritten below if we go in drag and drop mode)
	// (MovingViewport->Viewport will be NULL in the rare situation where the window disappared while moving, set UpdateMouseMovingWindowNewFrame() for details)
	if (g.MovingWindow && g.MovingWindow->Viewport)
		g.MouseViewport = g.MovingWindow->Viewport;
	else
		g.MouseViewport = g.MouseLastHoveredViewport;

	// When dragging something, always refer to the last hovered viewport.
	// - when releasing a moving window we will revert to aiming behind (at viewport_hovered)
	// - when we are between viewports, our dragged preview will tend to show in the last viewport _even_ if we don't have tooltips in their viewports (when lacking monitor info)
	// - consider the case of holding on a menu item to browse child menus: even thou a mouse button is held, there's no active id because menu items only react on mouse release.
	// FIXME-VIEWPORT: This is essentially broken, when KGGuiBackendFlags_HasMouseHoveredViewport is set we want to trust when viewport_hovered==NULL and use that.
	const bool is_mouse_dragging_with_an_expected_destination = g.DragDropActive;
	if (is_mouse_dragging_with_an_expected_destination && viewport_hovered == NULL)
		viewport_hovered = g.MouseLastHoveredViewport;
	if (is_mouse_dragging_with_an_expected_destination || g.ActiveId == 0 || !KarmaGui::IsAnyMouseDown())
		if (viewport_hovered != NULL && viewport_hovered != g.MouseViewport && !(viewport_hovered->Flags & KGGuiViewportFlags_NoInputs))
			g.MouseViewport = viewport_hovered;

	KR_CORE_ASSERT(g.MouseViewport != NULL, "");
}

// Update user-facing viewport list (g.Viewports -> g.PlatformIO.Viewports after filtering out some)
void Karma::KarmaGuiInternal::UpdateViewportsEndFrame()
{
	KarmaGuiContext& g = *GKarmaGui;
	g.PlatformIO.Viewports.resize(0);
	for (int i = 0; i < g.Viewports.Size; i++)
	{
		KGGuiViewportP* viewport = g.Viewports[i];
		viewport->LastPos = viewport->Pos;
		if (viewport->LastFrameActive < g.FrameCount || viewport->Size.x <= 0.0f || viewport->Size.y <= 0.0f)
			if (i > 0) // Always include main viewport in the list
				continue;
		if (viewport->Window && !IsWindowActiveAndVisible(viewport->Window))
			continue;
		if (i > 0)
		{
			KR_CORE_ASSERT(viewport->Window != NULL, "");
		}
		g.PlatformIO.Viewports.push_back(viewport);
	}
	g.Viewports[0]->ClearRequestFlags(); // Clear main viewport flags because UpdatePlatformWindows() won't do it and may not even be called
}

// FIXME: We should ideally refactor the system to call this every frame (we currently don't)
KGGuiViewportP* Karma::KarmaGuiInternal::AddUpdateViewport(KGGuiWindow* window, KGGuiID id, const KGVec2& pos, const KGVec2& size, KarmaGuiViewportFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(id != 0, "");

	flags |= KGGuiViewportFlags_IsPlatformWindow;
	if (window != NULL)
	{
		if (g.MovingWindow && g.MovingWindow->RootWindowDockTree == window)
			flags |= KGGuiViewportFlags_NoInputs | KGGuiViewportFlags_NoFocusOnAppearing;
		if ((window->Flags & KGGuiWindowFlags_NoMouseInputs) && (window->Flags & KGGuiWindowFlags_NoNavInputs))
			flags |= KGGuiViewportFlags_NoInputs;
		if (window->Flags & KGGuiWindowFlags_NoFocusOnAppearing)
			flags |= KGGuiViewportFlags_NoFocusOnAppearing;
	}

	KGGuiViewportP* viewport = (KGGuiViewportP*)KarmaGui::FindViewportByID(id);
	if (viewport)
	{
		// Always update for main viewport as we are already pulling correct platform pos/size (see #4900)
		if (!viewport->PlatformRequestMove || viewport->ID == IMGUI_VIEWPORT_DEFAULT_ID)
			viewport->Pos = pos;
		if (!viewport->PlatformRequestResize || viewport->ID == IMGUI_VIEWPORT_DEFAULT_ID)
			viewport->Size = size;
		viewport->Flags = flags | (viewport->Flags & KGGuiViewportFlags_Minimized); // Preserve existing flags
	}
	else
	{
		// New viewport
		viewport = KG_NEW(KGGuiViewportP)();
		viewport->ID = id;
		viewport->Idx = g.Viewports.Size;
		viewport->Pos = viewport->LastPos = pos;
		viewport->Size = size;
		viewport->Flags = flags;
		UpdateViewportPlatformMonitor(viewport);
		g.Viewports.push_back(viewport);
		KR_CORE_INFO("[viewport] Add Viewport {0} '{1}'", id, window ? window->Name : "<NULL>");

		// We normally setup for all viewports in NewFrame() but here need to handle the mid-frame creation of a new viewport.
		// We need to extend the fullscreen clip rect so the OverlayDrawList clip is correct for that the first frame
		g.DrawListSharedData.ClipRectFullscreen.x = KGMin(g.DrawListSharedData.ClipRectFullscreen.x, viewport->Pos.x);
		g.DrawListSharedData.ClipRectFullscreen.y = KGMin(g.DrawListSharedData.ClipRectFullscreen.y, viewport->Pos.y);
		g.DrawListSharedData.ClipRectFullscreen.z = KGMax(g.DrawListSharedData.ClipRectFullscreen.z, viewport->Pos.x + viewport->Size.x);
		g.DrawListSharedData.ClipRectFullscreen.w = KGMax(g.DrawListSharedData.ClipRectFullscreen.w, viewport->Pos.y + viewport->Size.y);

		// Store initial DpiScale before the OS platform window creation, based on expected monitor data.
		// This is so we can select an appropriate font size on the first frame of our window lifetime
		if (viewport->PlatformMonitor != -1)
			viewport->DpiScale = g.PlatformIO.Monitors[viewport->PlatformMonitor].DpiScale;
	}

	viewport->Window = window;
	viewport->LastFrameActive = g.FrameCount;
	viewport->UpdateWorkRect();
	KR_CORE_ASSERT(window == NULL || viewport->ID == window->ID, "");

	if (window != NULL)
		window->ViewportOwned = true;

	return viewport;
}

void Karma::KarmaGuiInternal::DestroyViewport(KGGuiViewportP* viewport)
{
	// Clear references to this viewport in windows (window->ViewportId becomes the master data)
	KarmaGuiContext& g = *GKarmaGui;
	for (int window_n = 0; window_n < g.Windows.Size; window_n++)
	{
		KGGuiWindow* window = g.Windows[window_n];
		if (window->Viewport != viewport)
			continue;
		window->Viewport = NULL;
		window->ViewportOwned = false;
	}
	if (viewport == g.MouseLastHoveredViewport)
		g.MouseLastHoveredViewport = NULL;

	// Destroy
	KR_CORE_INFO("[viewport] Delete Viewport {0} '{1}'", viewport->ID, viewport->Window ? viewport->Window->Name : "n/a");
	DestroyPlatformWindow(viewport); // In most circumstances the platform window will already be destroyed here.
	KR_CORE_ASSERT(g.PlatformIO.Viewports.contains(viewport) == false, "");
	KR_CORE_ASSERT(g.Viewports[viewport->Idx] == viewport, "");
	g.Viewports.erase(g.Viewports.Data + viewport->Idx);
	KG_DELETE(viewport);
}

// FIXME-VIEWPORT: This is all super messy and ought to be clarified or rewritten.
void Karma::KarmaGuiInternal::WindowSelectViewport(KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiWindowFlags flags = window->Flags;
	window->ViewportAllowPlatformMonitorExtend = -1;

	// Restore main viewport if multi-viewport is not supported by the backend
	KGGuiViewportP* main_viewport = (KGGuiViewportP*)(void*)KarmaGui::GetMainViewport();
	if (!(g.ConfigFlagsCurrFrame & KGGuiConfigFlags_ViewportsEnable))
	{
		SetWindowViewport(window, main_viewport);
		return;
	}
	window->ViewportOwned = false;

	// Appearing popups reset their viewport so they can inherit again
	if ((flags & (KGGuiWindowFlags_Popup | KGGuiWindowFlags_Tooltip)) && window->Appearing)
	{
		window->Viewport = NULL;
		window->ViewportId = 0;
	}

	if ((g.NextWindowData.Flags & KGGuiNextWindowDataFlags_HasViewport) == 0)
	{
		// By default inherit from parent window
		if (window->Viewport == NULL && window->ParentWindow && (!window->ParentWindow->IsFallbackWindow || window->ParentWindow->WasActive))
			window->Viewport = window->ParentWindow->Viewport;

		// Attempt to restore saved viewport id (= window that hasn't been activated yet), try to restore the viewport based on saved 'window->ViewportPos' restored from .ini file
		if (window->Viewport == NULL && window->ViewportId != 0)
		{
			window->Viewport = (KGGuiViewportP*)KarmaGui::FindViewportByID(window->ViewportId);
			if (window->Viewport == NULL && window->ViewportPos.x != FLT_MAX && window->ViewportPos.y != FLT_MAX)
				window->Viewport = AddUpdateViewport(window, window->ID, window->ViewportPos, window->Size, KGGuiViewportFlags_None);
		}
	}

	bool lock_viewport = false;
	if (g.NextWindowData.Flags & KGGuiNextWindowDataFlags_HasViewport)
	{
		// Code explicitly request a viewport
		window->Viewport = (KGGuiViewportP*)KarmaGui::FindViewportByID(g.NextWindowData.ViewportId);
		window->ViewportId = g.NextWindowData.ViewportId; // Store ID even if Viewport isn't resolved yet.
		lock_viewport = true;
	}
	else if ((flags & KGGuiWindowFlags_ChildWindow) || (flags & KGGuiWindowFlags_ChildMenu))
	{
		// Always inherit viewport from parent window
		if (window->DockNode && window->DockNode->HostWindow)
		{
			KR_CORE_ASSERT(window->DockNode->HostWindow->Viewport == window->ParentWindow->Viewport, "");
		}
		window->Viewport = window->ParentWindow->Viewport;
	}
	else if (window->DockNode && window->DockNode->HostWindow)
	{
		// This covers the "always inherit viewport from parent window" case for when a window reattach to a node that was just created mid-frame
		window->Viewport = window->DockNode->HostWindow->Viewport;
	}
	else if (flags & KGGuiWindowFlags_Tooltip)
	{
		window->Viewport = g.MouseViewport;
	}
	else if (GetWindowAlwaysWantOwnViewport(window))
	{
		window->Viewport = AddUpdateViewport(window, window->ID, window->Pos, window->Size, KGGuiViewportFlags_None);
	}
	else if (g.MovingWindow && g.MovingWindow->RootWindowDockTree == window && KarmaGui::IsMousePosValid())
	{
		if (window->Viewport != NULL && window->Viewport->Window == window)
			window->Viewport = AddUpdateViewport(window, window->ID, window->Pos, window->Size, KGGuiViewportFlags_None);
	}
	else
	{
		// Merge into host viewport?
		// We cannot test window->ViewportOwned as it set lower in the function.
		// Testing (g.ActiveId == 0 || g.ActiveIdAllowOverlap) to avoid merging during a short-term widget interaction. Main intent was to avoid during resize (see #4212)
		bool try_to_merge_into_host_viewport = (window->Viewport && window == window->Viewport->Window && (g.ActiveId == 0 || g.ActiveIdAllowOverlap));
		if (try_to_merge_into_host_viewport)
			UpdateTryMergeWindowIntoHostViewports(window);
	}

	// Fallback: merge in default viewport if z-order matches, otherwise create a new viewport
	if (window->Viewport == NULL)
		if (!UpdateTryMergeWindowIntoHostViewport(window, main_viewport))
			window->Viewport = AddUpdateViewport(window, window->ID, window->Pos, window->Size, KGGuiViewportFlags_None);

	// Mark window as allowed to protrude outside of its viewport and into the current monitor
	if (!lock_viewport)
	{
		if (flags & (KGGuiWindowFlags_Tooltip | KGGuiWindowFlags_Popup))
		{
			// We need to take account of the possibility that mouse may become invalid.
			// Popups/Tooltip always set ViewportAllowPlatformMonitorExtend so GetWindowAllowedExtentRect() will return full monitor bounds.
			KGVec2 mouse_ref = (flags & KGGuiWindowFlags_Tooltip) ? g.IO.MousePos : g.BeginPopupStack.back().OpenMousePos;
			bool use_mouse_ref = (g.NavDisableHighlight || !g.NavDisableMouseHover || !g.NavWindow);
			bool mouse_valid = KarmaGui::IsMousePosValid(&mouse_ref);
			if ((window->Appearing || (flags & (KGGuiWindowFlags_Tooltip | KGGuiWindowFlags_ChildMenu))) && (!use_mouse_ref || mouse_valid))
				window->ViewportAllowPlatformMonitorExtend = FindPlatformMonitorForPos((use_mouse_ref && mouse_valid) ? mouse_ref : NavCalcPreferredRefPos());
			else
				window->ViewportAllowPlatformMonitorExtend = window->Viewport->PlatformMonitor;
		}
		else if (window->Viewport && window != window->Viewport->Window && window->Viewport->Window && !(flags & KGGuiWindowFlags_ChildWindow) && window->DockNode == NULL)
		{
			// When called from Begin() we don't have access to a proper version of the Hidden flag yet, so we replicate this code.
			const bool will_be_visible = (window->DockIsActive && !window->DockTabIsVisible) ? false : true;
			if ((window->Flags & KGGuiWindowFlags_DockNodeHost) && window->Viewport->LastFrameActive < g.FrameCount && will_be_visible)
			{
				// Steal/transfer ownership
				KR_CORE_INFO("[viewport] Window '{0}' steal Viewport {1} from Window '{2}'", window->Name, window->Viewport->ID, window->Viewport->Window->Name);
				window->Viewport->Window = window;
				window->Viewport->ID = window->ID;
				window->Viewport->LastNameHash = 0;
			}
			else if (!UpdateTryMergeWindowIntoHostViewports(window)) // Merge?
			{
				// New viewport
				window->Viewport = AddUpdateViewport(window, window->ID, window->Pos, window->Size, KGGuiViewportFlags_NoFocusOnAppearing);
			}
		}
		else if (window->ViewportAllowPlatformMonitorExtend < 0 && (flags & KGGuiWindowFlags_ChildWindow) == 0)
		{
			// Regular (non-child, non-popup) windows by default are also allowed to protrude
			// Child windows are kept contained within their parent.
			window->ViewportAllowPlatformMonitorExtend = window->Viewport->PlatformMonitor;
		}
	}

	// Update flags
	window->ViewportOwned = (window == window->Viewport->Window);
	window->ViewportId = window->Viewport->ID;

	// If the OS window has a title bar, hide our imgui title bar
	//if (window->ViewportOwned && !(window->Viewport->Flags & KGGuiViewportFlags_NoDecoration))
	//    window->Flags |= KGGuiWindowFlags_NoTitleBar;
}

void Karma::KarmaGuiInternal::WindowSyncOwnedViewport(KGGuiWindow* window, KGGuiWindow* parent_window_in_stack)
{
	KarmaGuiContext& g = *GKarmaGui;

	bool viewport_rect_changed = false;

	// Synchronize window --> viewport in most situations
	// Synchronize viewport -> window in case the platform window has been moved or resized from the OS/WM
	if (window->Viewport->PlatformRequestMove)
	{
		window->Pos = window->Viewport->Pos;
		MarkIniSettingsDirty(window);
	}
	else if (memcmp(&window->Viewport->Pos, &window->Pos, sizeof(window->Pos)) != 0)
	{
		viewport_rect_changed = true;
		window->Viewport->Pos = window->Pos;
	}

	if (window->Viewport->PlatformRequestResize)
	{
		window->Size = window->SizeFull = window->Viewport->Size;
		MarkIniSettingsDirty(window);
	}
	else if (memcmp(&window->Viewport->Size, &window->Size, sizeof(window->Size)) != 0)
	{
		viewport_rect_changed = true;
		window->Viewport->Size = window->Size;
	}
	window->Viewport->UpdateWorkRect();

	// The viewport may have changed monitor since the global update in UpdateViewportsNewFrame()
	// Either a SetNextWindowPos() call in the current frame or a SetWindowPos() call in the previous frame may have this effect.
	if (viewport_rect_changed)
		UpdateViewportPlatformMonitor(window->Viewport);

	// Update common viewport flags
	const KarmaGuiViewportFlags viewport_flags_to_clear = KGGuiViewportFlags_TopMost | KGGuiViewportFlags_NoTaskBarIcon | KGGuiViewportFlags_NoDecoration | KGGuiViewportFlags_NoRendererClear;
	KarmaGuiViewportFlags viewport_flags = window->Viewport->Flags & ~viewport_flags_to_clear;
	KarmaGuiWindowFlags window_flags = window->Flags;
	const bool is_modal = (window_flags & KGGuiWindowFlags_Modal) != 0;
	const bool is_short_lived_floating_window = (window_flags & (KGGuiWindowFlags_ChildMenu | KGGuiWindowFlags_Tooltip | KGGuiWindowFlags_Popup)) != 0;
	if (window_flags & KGGuiWindowFlags_Tooltip)
		viewport_flags |= KGGuiViewportFlags_TopMost;
	if ((g.IO.ConfigViewportsNoTaskBarIcon || is_short_lived_floating_window) && !is_modal)
		viewport_flags |= KGGuiViewportFlags_NoTaskBarIcon;
	if (g.IO.ConfigViewportsNoDecoration || is_short_lived_floating_window)
		viewport_flags |= KGGuiViewportFlags_NoDecoration;

	// Not correct to set modal as topmost because:
	// - Because other popups can be stacked above a modal (e.g. combo box in a modal)
	// - KGGuiViewportFlags_TopMost is currently handled different in backends: in Win32 it is "appear top most" whereas in GLFW and SDL it is "stay topmost"
	//if (flags & KGGuiWindowFlags_Modal)
	//    viewport_flags |= KGGuiViewportFlags_TopMost;

	// For popups and menus that may be protruding out of their parent viewport, we enable _NoFocusOnClick so that clicking on them
	// won't steal the OS focus away from their parent window (which may be reflected in OS the title bar decoration).
	// Setting _NoFocusOnClick would technically prevent us from bringing back to front in case they are being covered by an OS window from a different app,
	// but it shouldn't be much of a problem considering those are already popups that are closed when clicking elsewhere.
	if (is_short_lived_floating_window && !is_modal)
		viewport_flags |= KGGuiViewportFlags_NoFocusOnAppearing | KGGuiViewportFlags_NoFocusOnClick;

	// We can overwrite viewport flags using KarmaGuiWindowClass (advanced users)
	if (window->WindowClass.ViewportFlagsOverrideSet)
		viewport_flags |= window->WindowClass.ViewportFlagsOverrideSet;
	if (window->WindowClass.ViewportFlagsOverrideClear)
		viewport_flags &= ~window->WindowClass.ViewportFlagsOverrideClear;

	// We can also tell the backend that clearing the platform window won't be necessary,
	// as our window background is filling the viewport and we have disabled BgAlpha.
	// FIXME: Work on support for per-viewport transparency (#2766)
	if (!(window_flags & KGGuiWindowFlags_NoBackground))
		viewport_flags |= KGGuiViewportFlags_NoRendererClear;

	window->Viewport->Flags = viewport_flags;

	// Update parent viewport ID
	// (the !IsFallbackWindow test mimic the one done in WindowSelectViewport())
	if (window->WindowClass.ParentViewportId != (KGGuiID)-1)
		window->Viewport->ParentViewportId = window->WindowClass.ParentViewportId;
	else if ((window_flags & (KGGuiWindowFlags_Popup | KGGuiWindowFlags_Tooltip)) && parent_window_in_stack && (!parent_window_in_stack->IsFallbackWindow || parent_window_in_stack->WasActive))
		window->Viewport->ParentViewportId = parent_window_in_stack->Viewport->ID;
	else
		window->Viewport->ParentViewportId = g.IO.ConfigViewportsNoDefaultParent ? 0 : IMGUI_VIEWPORT_DEFAULT_ID;
}

// Called by user at the end of the main loop, after EndFrame()
// This will handle the creation/update of all OS windows via function defined in the KarmaGuiPlatformIO api.
void Karma::KarmaGui::UpdatePlatformWindows()
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(g.FrameCountEnded == g.FrameCount, "Forgot to call Render() or EndFrame() before UpdatePlatformWindows()?");
	KR_CORE_ASSERT(g.FrameCountPlatformEnded < g.FrameCount, "");
	g.FrameCountPlatformEnded = g.FrameCount;
	if (!(g.ConfigFlagsCurrFrame & KGGuiConfigFlags_ViewportsEnable))
		return;

	// Create/resize/destroy platform windows to match each active viewport.
	// Skip the main viewport (index 0), which is always fully handled by the application!
	for (int i = 1; i < g.Viewports.Size; i++)
	{
		KGGuiViewportP* viewport = g.Viewports[i];

		// Destroy platform window if the viewport hasn't been submitted or if it is hosting a hidden window
		// (the implicit/fallback Debug##Default window will be registering its viewport then be disabled, causing a dummy DestroyPlatformWindow to be made each frame)
		bool destroy_platform_window = false;
		destroy_platform_window |= (viewport->LastFrameActive < g.FrameCount - 1);
		destroy_platform_window |= (viewport->Window && !IsWindowActiveAndVisible(viewport->Window));
		if (destroy_platform_window)
		{
			KarmaGuiInternal::DestroyPlatformWindow(viewport);
			continue;
		}

		// New windows that appears directly in a new viewport won't always have a size on their first frame
		if (viewport->LastFrameActive < g.FrameCount || viewport->Size.x <= 0 || viewport->Size.y <= 0)
			continue;

		// Create window
		const bool is_new_platform_window = (viewport->PlatformWindowCreated == false);
		if (is_new_platform_window)
		{
			KR_CORE_INFO("[viewport] Create Platform Window {0} '{1}'", viewport->ID, viewport->Window ? viewport->Window->Name : "n/a");
			g.PlatformIO.Platform_CreateWindow(viewport);
			if (g.PlatformIO.Renderer_CreateWindow != NULL)
				g.PlatformIO.Renderer_CreateWindow(viewport);
			viewport->LastNameHash = 0;
			viewport->LastPlatformPos = viewport->LastPlatformSize = KGVec2(FLT_MAX, FLT_MAX); // By clearing those we'll enforce a call to Platform_SetWindowPos/Size below, before Platform_ShowWindow (FIXME: Is that necessary?)
			viewport->LastRendererSize = viewport->Size;                                       // We don't need to call Renderer_SetWindowSize() as it is expected Renderer_CreateWindow() already did it.
			viewport->PlatformWindowCreated = true;
		}

		// Apply Position and Size (from ImGui to Platform/Renderer backends)
		if ((viewport->LastPlatformPos.x != viewport->Pos.x || viewport->LastPlatformPos.y != viewport->Pos.y) && !viewport->PlatformRequestMove)
			g.PlatformIO.Platform_SetWindowPos(viewport, viewport->Pos);
		if ((viewport->LastPlatformSize.x != viewport->Size.x || viewport->LastPlatformSize.y != viewport->Size.y) && !viewport->PlatformRequestResize)
			g.PlatformIO.Platform_SetWindowSize(viewport, viewport->Size);
		if ((viewport->LastRendererSize.x != viewport->Size.x || viewport->LastRendererSize.y != viewport->Size.y) && g.PlatformIO.Renderer_SetWindowSize)
			g.PlatformIO.Renderer_SetWindowSize(viewport, viewport->Size);
		viewport->LastPlatformPos = viewport->Pos;
		viewport->LastPlatformSize = viewport->LastRendererSize = viewport->Size;

		// Update title bar (if it changed)
		if (KGGuiWindow* window_for_title = GetWindowForTitleDisplay(viewport->Window))
		{
			const char* title_begin = window_for_title->Name;
			char* title_end = (char*)(intptr_t)KarmaGuiInternal::FindRenderedTextEnd(title_begin);
			const KGGuiID title_hash = KGHashStr(title_begin, title_end - title_begin);
			if (viewport->LastNameHash != title_hash)
			{
				char title_end_backup_c = *title_end;
				*title_end = 0; // Cut existing buffer short instead of doing an alloc/free, no small gain.
				g.PlatformIO.Platform_SetWindowTitle(viewport, title_begin);
				*title_end = title_end_backup_c;
				viewport->LastNameHash = title_hash;
			}
		}

		// Update alpha (if it changed)
		if (viewport->LastAlpha != viewport->Alpha && g.PlatformIO.Platform_SetWindowAlpha)
			g.PlatformIO.Platform_SetWindowAlpha(viewport, viewport->Alpha);
		viewport->LastAlpha = viewport->Alpha;

		// Optional, general purpose call to allow the backend to perform general book-keeping even if things haven't changed.
		if (g.PlatformIO.Platform_UpdateWindow)
			g.PlatformIO.Platform_UpdateWindow(viewport);

		if (is_new_platform_window)
		{
			// On startup ensure new platform window don't steal focus (give it a few frames, as nested contents may lead to viewport being created a few frames late)
			if (g.FrameCount < 3)
				viewport->Flags |= KGGuiViewportFlags_NoFocusOnAppearing;

			// Show window
			g.PlatformIO.Platform_ShowWindow(viewport);

			// Even without focus, we assume the window becomes front-most.
			// This is useful for our platform z-order heuristic when io.MouseHoveredViewport is not available.
			if (viewport->LastFrontMostStampCount != g.ViewportFrontMostStampCount)
				viewport->LastFrontMostStampCount = ++g.ViewportFrontMostStampCount;
		}

		// Clear request flags
		viewport->ClearRequestFlags();
	}

	// Update our implicit z-order knowledge of platform windows, which is used when the backend cannot provide io.MouseHoveredViewport.
	// When setting Platform_GetWindowFocus, it is expected that the platform backend can handle calls without crashing if it doesn't have data stored.
	// FIXME-VIEWPORT: We should use this information to also set dear imgui-side focus, allowing us to handle os-level alt+tab.
	if (g.PlatformIO.Platform_GetWindowFocus != NULL)
	{
		KGGuiViewportP* focused_viewport = NULL;
		for (int n = 0; n < g.Viewports.Size && focused_viewport == NULL; n++)
		{
			KGGuiViewportP* viewport = g.Viewports[n];
			if (viewport->PlatformWindowCreated)
				if (g.PlatformIO.Platform_GetWindowFocus(viewport))
					focused_viewport = viewport;
		}

		// Store a tag so we can infer z-order easily from all our windows
		// We compare PlatformLastFocusedViewportId so newly created viewports with _NoFocusOnAppearing flag
		// will keep the front most stamp instead of losing it back to their parent viewport.
		if (focused_viewport && g.PlatformLastFocusedViewportId != focused_viewport->ID)
		{
			if (focused_viewport->LastFrontMostStampCount != g.ViewportFrontMostStampCount)
				focused_viewport->LastFrontMostStampCount = ++g.ViewportFrontMostStampCount;
			g.PlatformLastFocusedViewportId = focused_viewport->ID;
		}
	}
}

// This is a default/basic function for performing the rendering/swap of multiple Platform Windows.
// Custom renderers may prefer to not call this function at all, and instead iterate the publicly exposed platform data and handle rendering/sync themselves.
// The Render/Swap functions stored in KarmaGuiPlatformIO are merely here to allow for this helper to exist, but you can do it yourself:
//
//    KarmaGuiPlatformIO& platform_io = KarmaGui::GetPlatformIO();
//    for (int i = 1; i < platform_io.Viewports.Size; i++)
//        if ((platform_io.Viewports[i]->Flags & KGGuiViewportFlags_Minimized) == 0)
//            MyRenderFunction(platform_io.Viewports[i], my_args);
//    for (int i = 1; i < platform_io.Viewports.Size; i++)
//        if ((platform_io.Viewports[i]->Flags & KGGuiViewportFlags_Minimized) == 0)
//            MySwapBufferFunction(platform_io.Viewports[i], my_args);
//
void Karma::KarmaGui::RenderPlatformWindowsDefault(void* platform_render_arg, void* renderer_render_arg)
{
	// Skip the main viewport (index 0), which is always fully handled by the application!
	KarmaGuiPlatformIO& platform_io = KarmaGui::GetPlatformIO();
	for (int i = 1; i < platform_io.Viewports.Size; i++)
	{
		KarmaGuiViewport* viewport = platform_io.Viewports[i];
		if (viewport->Flags & KGGuiViewportFlags_Minimized)
			continue;
		if (platform_io.Platform_RenderWindow) platform_io.Platform_RenderWindow(viewport, platform_render_arg);
		if (platform_io.Renderer_RenderWindow) platform_io.Renderer_RenderWindow(viewport, renderer_render_arg);
	}
	for (int i = 1; i < platform_io.Viewports.Size; i++)
	{
		KarmaGuiViewport* viewport = platform_io.Viewports[i];
		if (viewport->Flags & KGGuiViewportFlags_Minimized)
			continue;
		if (platform_io.Platform_SwapBuffers) platform_io.Platform_SwapBuffers(viewport, platform_render_arg);
		if (platform_io.Renderer_SwapBuffers) platform_io.Renderer_SwapBuffers(viewport, renderer_render_arg);
	}
}

int Karma::KarmaGuiInternal::FindPlatformMonitorForPos(const KGVec2& pos)
{
	KarmaGuiContext& g = *GKarmaGui;
	for (int monitor_n = 0; monitor_n < g.PlatformIO.Monitors.Size; monitor_n++)
	{
		const KarmaGuiPlatformMonitor& monitor = g.PlatformIO.Monitors[monitor_n];
		if (KGRect(monitor.MainPos, monitor.MainPos + monitor.MainSize).Contains(pos))
			return monitor_n;
	}
	return -1;
}

// Search for the monitor with the largest intersection area with the given rectangle
// We generally try to avoid searching loops but the monitor count should be very small here
// FIXME-OPT: We could test the last monitor used for that viewport first, and early
int Karma::KarmaGuiInternal::FindPlatformMonitorForRect(const KGRect& rect)
{
	KarmaGuiContext& g = *GKarmaGui;

	const int monitor_count = g.PlatformIO.Monitors.Size;
	if (monitor_count <= 1)
		return monitor_count - 1;

	// Use a minimum threshold of 1.0f so a zero-sized rect won't false positive, and will still find the correct monitor given its position.
	// This is necessary for tooltips which always resize down to zero at first.
	const float surface_threshold = KGMax(rect.GetWidth() * rect.GetHeight() * 0.5f, 1.0f);
	int best_monitor_n = -1;
	float best_monitor_surface = 0.001f;

	for (int monitor_n = 0; monitor_n < g.PlatformIO.Monitors.Size && best_monitor_surface < surface_threshold; monitor_n++)
	{
		const KarmaGuiPlatformMonitor& monitor = g.PlatformIO.Monitors[monitor_n];
		const KGRect monitor_rect = KGRect(monitor.MainPos, monitor.MainPos + monitor.MainSize);
		if (monitor_rect.Contains(rect))
			return monitor_n;
		KGRect overlapping_rect = rect;
		overlapping_rect.ClipWithFull(monitor_rect);
		float overlapping_surface = overlapping_rect.GetWidth() * overlapping_rect.GetHeight();
		if (overlapping_surface < best_monitor_surface)
			continue;
		best_monitor_surface = overlapping_surface;
		best_monitor_n = monitor_n;
	}
	return best_monitor_n;
}

// Update monitor from viewport rectangle (we'll use this info to clamp windows and save windows lost in a removed monitor)
void Karma::KarmaGuiInternal::UpdateViewportPlatformMonitor(KGGuiViewportP* viewport)
{
	viewport->PlatformMonitor = (short)FindPlatformMonitorForRect(viewport->GetMainRect());
}

// Return value is always != NULL, but don't hold on it across frames.
const KarmaGuiPlatformMonitor* Karma::KarmaGuiInternal::GetViewportPlatformMonitor(KarmaGuiViewport* viewport_p)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiViewportP* viewport = (KGGuiViewportP*)(void*)viewport_p;
	int monitor_idx = viewport->PlatformMonitor;
	if (monitor_idx >= 0 && monitor_idx < g.PlatformIO.Monitors.Size)
		return &g.PlatformIO.Monitors[monitor_idx];
	return &g.FallbackMonitor;
}

void Karma::KarmaGuiInternal::DestroyPlatformWindow(KGGuiViewportP* viewport)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (viewport->PlatformWindowCreated)
	{
		if (g.PlatformIO.Renderer_DestroyWindow)
			g.PlatformIO.Renderer_DestroyWindow(viewport);
		if (g.PlatformIO.Platform_DestroyWindow)
			g.PlatformIO.Platform_DestroyWindow(viewport);
		KR_CORE_ASSERT(viewport->RendererUserData == NULL && viewport->PlatformUserData == NULL, "");

		// Don't clear PlatformWindowCreated for the main viewport, as we initially set that up to true in Initialize()
		// The righter way may be to leave it to the backend to set this flag all-together, and made the flag public.
		if (viewport->ID != IMGUI_VIEWPORT_DEFAULT_ID)
			viewport->PlatformWindowCreated = false;
	}
	else
	{
		KR_CORE_ASSERT(viewport->RendererUserData == NULL && viewport->PlatformUserData == NULL && viewport->PlatformHandle == NULL, "");
	}
	viewport->RendererUserData = viewport->PlatformUserData = viewport->PlatformHandle = NULL;
	viewport->ClearRequestFlags();
}

void Karma::KarmaGui::DestroyPlatformWindows()
{
	// We call the destroy window on every viewport (including the main viewport, index 0) to give a chance to the backend
	// to clear any data they may have stored in e.g. PlatformUserData, RendererUserData.
	// It is convenient for the platform backend code to store something in the main viewport, in order for e.g. the mouse handling
	// code to operator a consistent manner.
	// It is expected that the backend can handle calls to Renderer_DestroyWindow/Platform_DestroyWindow without
	// crashing if it doesn't have data stored.
	KarmaGuiContext& g = *GKarmaGui;
	for (int i = 0; i < g.Viewports.Size; i++)
		KarmaGuiInternal::DestroyPlatformWindow(g.Viewports[i]);
}

//-----------------------------------------------------------------------------
// [SECTION] DOCKING
//-----------------------------------------------------------------------------
// Docking: Internal Types
// Docking: Forward Declarations
// Docking: KGGuiDockContext
// Docking: KGGuiDockContext Docking/Undocking functions
// Docking: KGGuiDockNode
// Docking: KGGuiDockNode Tree manipulation functions
// Docking: Public Functions (SetWindowDock, DockSpace, DockSpaceOverViewport)
// Docking: Builder Functions
// Docking: Begin/End Support Functions (called from Begin/End)
// Docking: Settings
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Typical Docking call flow: (root level is generally public API):
//-----------------------------------------------------------------------------
// - NewFrame()                               new dear imgui frame
//    | DockContextNewFrameUpdateUndocking()  - process queued undocking requests
//    | - DockContextProcessUndockWindow()    - process one window undocking request
//    | - DockContextProcessUndockNode()      - process one whole node undocking request
//    | DockContextNewFrameUpdateUndocking()  - process queue docking requests, create floating dock nodes
//    | - update g.HoveredDockNode            - [debug] update node hovered by mouse
//    | - DockContextProcessDock()            - process one docking request
//    | - DockNodeUpdate()
//    |   - DockNodeUpdateForRootNode()
//    |     - DockNodeUpdateFlagsAndCollapse()
//    |     - DockNodeFindInfo()
//    |   - destroy unused node or tab bar
//    |   - create dock node host window
//    |      - Begin() etc.
//    |   - DockNodeStartMouseMovingWindow()
//    |   - DockNodeTreeUpdatePosSize()
//    |   - DockNodeTreeUpdateSplitter()
//    |   - draw node background
//    |   - DockNodeUpdateTabBar()            - create/update tab bar for a docking node
//    |     - DockNodeAddTabBar()
//    |     - DockNodeUpdateWindowMenu()
//    |     - DockNodeCalcTabBarLayout()
//    |     - BeginTabBarEx()
//    |     - TabItemEx() calls
//    |     - EndTabBar()
//    |   - BeginDockableDragDropTarget()
//    |      - DockNodeUpdate()               - recurse into child nodes...
//-----------------------------------------------------------------------------
// - DockSpace()                              user submit a dockspace into a window
//    | Begin(Child)                          - create a child window
//    | DockNodeUpdate()                      - call main dock node update function
//    | End(Child)
//    | ItemSize()
//-----------------------------------------------------------------------------
// - Begin()
//    | BeginDocked()
//    | BeginDockableDragDropSource()
//    | BeginDockableDragDropTarget()
//    | - DockNodePreviewDockRender()
//-----------------------------------------------------------------------------
// - EndFrame()
//    | DockContextEndFrame()
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Docking: Internal Types
//-----------------------------------------------------------------------------
// - ImGuiDockRequestType
// - KGGuiDockRequest
// - ImGuiDockPreviewData
// - KGGuiDockNodeSettings
// - KGGuiDockContext
//-----------------------------------------------------------------------------

enum ImGuiDockRequestType
{
	ImGuiDockRequestType_None = 0,
	ImGuiDockRequestType_Dock,
	ImGuiDockRequestType_Undock,
	ImGuiDockRequestType_Split                  // Split is the same as Dock but without a DockPayload
};

struct KGGuiDockRequest
{
	ImGuiDockRequestType    Type;
	KGGuiWindow* DockTargetWindow;   // Destination/Target Window to dock into (may be a loose window or a DockNode, might be NULL in which case DockTargetNode cannot be NULL)
	KGGuiDockNode* DockTargetNode;     // Destination/Target Node to dock into
	KGGuiWindow* DockPayload;        // Source/Payload window to dock (may be a loose window or a DockNode), [Optional]
	KarmaGuiDir                DockSplitDir;
	float                   DockSplitRatio;
	bool                    DockSplitOuter;
	KGGuiWindow* UndockTargetWindow;
	KGGuiDockNode* UndockTargetNode;

	KGGuiDockRequest()
	{
		Type = ImGuiDockRequestType_None;
		DockTargetWindow = DockPayload = UndockTargetWindow = NULL;
		DockTargetNode = UndockTargetNode = NULL;
		DockSplitDir = KGGuiDir_None;
		DockSplitRatio = 0.5f;
		DockSplitOuter = false;
	}
};

struct ImGuiDockPreviewData
{
	KGGuiDockNode   FutureNode;
	bool            IsDropAllowed;
	bool            IsCenterAvailable;
	bool            IsSidesAvailable;           // Hold your breath, grammar freaks..
	bool            IsSplitDirExplicit;         // Set when hovered the drop rect (vs. implicit SplitDir==None when hovered the window)
	KGGuiDockNode* SplitNode;
	KarmaGuiDir        SplitDir;
	float           SplitRatio;
	KGRect          DropRectsDraw[KGGuiDir_COUNT + 1];  // May be slightly different from hit-testing drop rects used in DockNodeCalcDropRects()

	ImGuiDockPreviewData() : FutureNode(0) { IsDropAllowed = IsCenterAvailable = IsSidesAvailable = IsSplitDirExplicit = false; SplitNode = NULL; SplitDir = KGGuiDir_None; SplitRatio = 0.f; for (int n = 0; n < KG_ARRAYSIZE(DropRectsDraw); n++) DropRectsDraw[n] = KGRect(+FLT_MAX, +FLT_MAX, -FLT_MAX, -FLT_MAX); }
};

// Persistent Settings data, stored contiguously in SettingsNodes (sizeof() ~32 bytes)
struct KGGuiDockNodeSettings
{
	KGGuiID             ID;
	KGGuiID             ParentNodeId;
	KGGuiID             ParentWindowId;
	KGGuiID             SelectedTabId;
	signed char         SplitAxis;
	char                Depth;
	KarmaGuiDockNodeFlags  Flags;                  // NB: We save individual flags one by one in ascii format (KGGuiDockNodeFlags_SavedFlagsMask_)
	KGVec2ih            Pos;
	KGVec2ih            Size;
	KGVec2ih            SizeRef;
	KGGuiDockNodeSettings() { memset(this, 0, sizeof(*this)); SplitAxis = KGGuiAxis_None; }
};

//-----------------------------------------------------------------------------
// Docking: Forward Declarations
//-----------------------------------------------------------------------------

namespace Karma
{
	// KGGuiDockContext
	static KGGuiDockNode* DockContextAddNode(KarmaGuiContext* ctx, KGGuiID id);
	static void             DockContextRemoveNode(KarmaGuiContext* ctx, KGGuiDockNode* node, bool merge_sibling_into_parent_node);
	static void             DockContextQueueNotifyRemovedNode(KarmaGuiContext* ctx, KGGuiDockNode* node);
	static void             DockContextProcessDock(KarmaGuiContext* ctx, KGGuiDockRequest* req);
	static void             DockContextProcessUndockWindow(KarmaGuiContext* ctx, KGGuiWindow* window, bool clear_persistent_docking_ref = true);
	static void             DockContextProcessUndockNode(KarmaGuiContext* ctx, KGGuiDockNode* node);
	static void             DockContextPruneUnusedSettingsNodes(KarmaGuiContext* ctx);
	static KGGuiDockNode* DockContextBindNodeToWindow(KarmaGuiContext* ctx, KGGuiWindow* window);
	static void             DockContextBuildNodesFromSettings(KarmaGuiContext* ctx, KGGuiDockNodeSettings* node_settings_array, int node_settings_count);
	static void             DockContextBuildAddWindowsToNodes(KarmaGuiContext* ctx, KGGuiID root_id);                            // Use root_id==0 to add all

	// KGGuiDockNode
	static void             DockNodeAddWindow(KGGuiDockNode* node, KGGuiWindow* window, bool add_to_tab_bar);
	static void             DockNodeMoveWindows(KGGuiDockNode* dst_node, KGGuiDockNode* src_node);
	static void             DockNodeMoveChildNodes(KGGuiDockNode* dst_node, KGGuiDockNode* src_node);
	static KGGuiWindow* DockNodeFindWindowByID(KGGuiDockNode* node, KGGuiID id);
	static void             DockNodeApplyPosSizeToWindows(KGGuiDockNode* node);
	static void             DockNodeRemoveWindow(KGGuiDockNode* node, KGGuiWindow* window, KGGuiID save_dock_id);
	static void             DockNodeHideHostWindow(KGGuiDockNode* node);
	static void             DockNodeUpdate(KGGuiDockNode* node);
	static void             DockNodeUpdateForRootNode(KGGuiDockNode* node);
	static void             DockNodeUpdateFlagsAndCollapse(KGGuiDockNode* node);
	static void             DockNodeUpdateHasCentralNodeChild(KGGuiDockNode* node);
	static void             DockNodeUpdateTabBar(KGGuiDockNode* node, KGGuiWindow* host_window);
	static void             DockNodeAddTabBar(KGGuiDockNode* node);
	static void             DockNodeRemoveTabBar(KGGuiDockNode* node);
	static KGGuiID          DockNodeUpdateWindowMenu(KGGuiDockNode* node, KGGuiTabBar* tab_bar);
	static void             DockNodeUpdateVisibleFlag(KGGuiDockNode* node);
	static void             DockNodeStartMouseMovingWindow(KGGuiDockNode* node, KGGuiWindow* window);
	static bool             DockNodeIsDropAllowed(KGGuiWindow* host_window, KGGuiWindow* payload_window);
	static void             DockNodePreviewDockSetup(KGGuiWindow* host_window, KGGuiDockNode* host_node, KGGuiWindow* payload_window, KGGuiDockNode* payload_node, ImGuiDockPreviewData* preview_data, bool is_explicit_target, bool is_outer_docking);
	static void             DockNodePreviewDockRender(KGGuiWindow* host_window, KGGuiDockNode* host_node, KGGuiWindow* payload_window, const ImGuiDockPreviewData* preview_data);
	static void             DockNodeCalcTabBarLayout(const KGGuiDockNode* node, KGRect* out_title_rect, KGRect* out_tab_bar_rect, KGVec2* out_window_menu_button_pos, KGVec2* out_close_button_pos);
	static void             DockNodeCalcSplitRects(KGVec2& pos_old, KGVec2& size_old, KGVec2& pos_new, KGVec2& size_new, KarmaGuiDir dir, KGVec2 size_new_desired);
	static bool             DockNodeCalcDropRectsAndTestMousePos(const KGRect& parent, KarmaGuiDir dir, KGRect& out_draw, bool outer_docking, KGVec2* test_mouse_pos);
	static const char* DockNodeGetHostWindowTitle(KGGuiDockNode* node, char* buf, int buf_size) { KGFormatString(buf, buf_size, "##DockNode_%02X", node->ID); return buf; }
	static int              DockNodeGetTabOrder(KGGuiWindow* window);

	// KGGuiDockNode tree manipulations
	static void             DockNodeTreeSplit(KarmaGuiContext* ctx, KGGuiDockNode* parent_node, KGGuiAxis split_axis, int split_first_child, float split_ratio, KGGuiDockNode* new_node);
	static void             DockNodeTreeMerge(KarmaGuiContext* ctx, KGGuiDockNode* parent_node, KGGuiDockNode* merge_lead_child);
	static void             DockNodeTreeUpdatePosSize(KGGuiDockNode* node, KGVec2 pos, KGVec2 size, KGGuiDockNode* only_write_to_single_node = NULL);
	static void             DockNodeTreeUpdateSplitter(KGGuiDockNode* node);
	static KGGuiDockNode* DockNodeTreeFindVisibleNodeByPos(KGGuiDockNode* node, KGVec2 pos);
	static KGGuiDockNode* DockNodeTreeFindFallbackLeafNode(KGGuiDockNode* node);

	// Settings
	static void             DockSettingsRenameNodeReferences(KGGuiID old_node_id, KGGuiID new_node_id);
	static void             DockSettingsRemoveNodeReferences(KGGuiID* node_ids, int node_ids_count);
	static KGGuiDockNodeSettings* DockSettingsFindNodeSettings(KarmaGuiContext* ctx, KGGuiID node_id);
	static void             DockSettingsHandler_ClearAll(KarmaGuiContext*, KGGuiSettingsHandler*);
	static void             DockSettingsHandler_ApplyAll(KarmaGuiContext*, KGGuiSettingsHandler*);
	static void* DockSettingsHandler_ReadOpen(KarmaGuiContext*, KGGuiSettingsHandler*, const char* name);
	static void             DockSettingsHandler_ReadLine(KarmaGuiContext*, KGGuiSettingsHandler*, void* entry, const char* line);
	static void             DockSettingsHandler_WriteAll(KarmaGuiContext* imgui_ctx, KGGuiSettingsHandler* handler, KarmaGuiTextBuffer* buf);
}

//-----------------------------------------------------------------------------
// Docking: KGGuiDockContext
//-----------------------------------------------------------------------------
// The lifetime model is different from the one of regular windows: we always create a KGGuiDockNode for each KGGuiDockNodeSettings,
// or we always hold the entire docking node tree. Nodes are frequently hidden, e.g. if the window(s) or child nodes they host are not active.
// At boot time only, we run a simple GC to remove nodes that have no references.
// Because dock node settings (which are small, contiguous structures) are always mirrored by their corresponding dock nodes (more complete structures),
// we can also very easily recreate the nodes from scratch given the settings data (this is what DockContextRebuild() does).
// This is convenient as docking reconfiguration can be implemented by mostly poking at the simpler settings data.
//-----------------------------------------------------------------------------
// - DockContextInitialize()
// - DockContextShutdown()
// - DockContextClearNodes()
// - DockContextRebuildNodes()
// - DockContextNewFrameUpdateUndocking()
// - DockContextNewFrameUpdateDocking()
// - DockContextEndFrame()
// - DockContextFindNodeByID()
// - DockContextBindNodeToWindow()
// - DockContextGenNodeID()
// - DockContextAddNode()
// - DockContextRemoveNode()
// - ImGuiDockContextPruneNodeData
// - DockContextPruneUnusedSettingsNodes()
// - DockContextBuildNodesFromSettings()
// - DockContextBuildAddWindowsToNodes()
//-----------------------------------------------------------------------------

void Karma::KarmaGuiInternal::DockContextInitialize(KarmaGuiContext* ctx)
{
	KarmaGuiContext& g = *ctx;

	// Add .ini handle for persistent docking data
	KGGuiSettingsHandler ini_handler;
	ini_handler.TypeName = "Docking";
	ini_handler.TypeHash = KGHashStr("Docking");
	ini_handler.ClearAllFn = Karma::DockSettingsHandler_ClearAll;
	ini_handler.ReadInitFn = Karma::DockSettingsHandler_ClearAll; // Also clear on read
	ini_handler.ReadOpenFn = Karma::DockSettingsHandler_ReadOpen;
	ini_handler.ReadLineFn = Karma::DockSettingsHandler_ReadLine;
	ini_handler.ApplyAllFn = Karma::DockSettingsHandler_ApplyAll;
	ini_handler.WriteAllFn = Karma::DockSettingsHandler_WriteAll;
	g.SettingsHandlers.push_back(ini_handler);
}

void Karma::KarmaGuiInternal::DockContextShutdown(KarmaGuiContext* ctx)
{
	KGGuiDockContext* dc = &ctx->DockContext;
	for (int n = 0; n < dc->Nodes.Data.Size; n++)
		if (KGGuiDockNode* node = (KGGuiDockNode*)dc->Nodes.Data[n].val_p)
			KG_DELETE(node);
}

void Karma::KarmaGuiInternal::DockContextClearNodes(KarmaGuiContext* ctx, KGGuiID root_id, bool clear_settings_refs)
{
	KG_UNUSED(ctx);
	KR_CORE_ASSERT(ctx == GKarmaGui, "");
	DockBuilderRemoveNodeDockedWindows(root_id, clear_settings_refs);
	DockBuilderRemoveNodeChildNodes(root_id);
}

// [DEBUG] This function also acts as a defacto test to make sure we can rebuild from scratch without a glitch
// (Different from DockSettingsHandler_ClearAll() + DockSettingsHandler_ApplyAll() because this reuses current settings!)
void Karma::KarmaGuiInternal::DockContextRebuildNodes(KarmaGuiContext* ctx)
{
	KarmaGuiContext& g = *ctx;
	KGGuiDockContext* dc = &ctx->DockContext;
	KR_CORE_INFO("[docking] DockContextRebuildNodes");
	KarmaGui::SaveIniSettingsToMemory();
	KGGuiID root_id = 0; // Rebuild all
	DockContextClearNodes(ctx, root_id, false);
	Karma::DockContextBuildNodesFromSettings(ctx, dc->NodesSettings.Data, dc->NodesSettings.Size);
	Karma::DockContextBuildAddWindowsToNodes(ctx, root_id);
}

// Docking context update function, called by NewFrame()
void Karma::KarmaGuiInternal::DockContextNewFrameUpdateUndocking(KarmaGuiContext* ctx)
{
	KarmaGuiContext& g = *ctx;
	KGGuiDockContext* dc = &ctx->DockContext;
	if (!(g.IO.ConfigFlags & KGGuiConfigFlags_DockingEnable))
	{
		if (dc->Nodes.Data.Size > 0 || dc->Requests.Size > 0)
			DockContextClearNodes(ctx, 0, true);
		return;
	}

	// Setting NoSplit at runtime merges all nodes
	if (g.IO.ConfigDockingNoSplit)
		for (int n = 0; n < dc->Nodes.Data.Size; n++)
			if (KGGuiDockNode* node = (KGGuiDockNode*)dc->Nodes.Data[n].val_p)
				if (node->IsRootNode() && node->IsSplitNode())
				{
					DockBuilderRemoveNodeChildNodes(node->ID);
					//dc->WantFullRebuild = true;
				}

	// Process full rebuild
#if 0
	if (KarmaGui::IsKeyPressed(KarmaGui::GetKeyIndex(KGGuiKey_C)))
		dc->WantFullRebuild = true;
#endif
	if (dc->WantFullRebuild)
	{
		DockContextRebuildNodes(ctx);
		dc->WantFullRebuild = false;
	}

	// Process Undocking requests (we need to process them _before_ the UpdateMouseMovingWindowNewFrame call in NewFrame)
	for (int n = 0; n < dc->Requests.Size; n++)
	{
		KGGuiDockRequest* req = &dc->Requests[n];
		if (req->Type == ImGuiDockRequestType_Undock && req->UndockTargetWindow)
			Karma::DockContextProcessUndockWindow(ctx, req->UndockTargetWindow);
		else if (req->Type == ImGuiDockRequestType_Undock && req->UndockTargetNode)
			Karma::DockContextProcessUndockNode(ctx, req->UndockTargetNode);
	}
}

// Docking context update function, called by NewFrame()
void Karma::KarmaGuiInternal::DockContextNewFrameUpdateDocking(KarmaGuiContext* ctx)
{
	KarmaGuiContext& g = *ctx;
	KGGuiDockContext* dc = &ctx->DockContext;
	if (!(g.IO.ConfigFlags & KGGuiConfigFlags_DockingEnable))
		return;

	// [DEBUG] Store hovered dock node.
	// We could in theory use DockNodeTreeFindVisibleNodeByPos() on the root host dock node, but using ->DockNode is a good shortcut.
	// Note this is mostly a debug thing and isn't actually used for docking target, because docking involve more detailed filtering.
	g.DebugHoveredDockNode = NULL;
	if (KGGuiWindow* hovered_window = g.HoveredWindowUnderMovingWindow)
	{
		if (hovered_window->DockNodeAsHost)
			g.DebugHoveredDockNode = Karma::DockNodeTreeFindVisibleNodeByPos(hovered_window->DockNodeAsHost, g.IO.MousePos);
		else if (hovered_window->RootWindow->DockNode)
			g.DebugHoveredDockNode = hovered_window->RootWindow->DockNode;
	}

	// Process Docking requests
	for (int n = 0; n < dc->Requests.Size; n++)
		if (dc->Requests[n].Type == ImGuiDockRequestType_Dock)
			Karma::DockContextProcessDock(ctx, &dc->Requests[n]);
	dc->Requests.resize(0);

	// Create windows for each automatic docking nodes
	// We can have NULL pointers when we delete nodes, but because ID are recycled this should amortize nicely (and our node count will never be very high)
	for (int n = 0; n < dc->Nodes.Data.Size; n++)
		if (KGGuiDockNode* node = (KGGuiDockNode*)dc->Nodes.Data[n].val_p)
			if (node->IsFloatingNode())
				Karma::DockNodeUpdate(node);
}

void Karma::KarmaGuiInternal::DockContextEndFrame(KarmaGuiContext* ctx)
{
	// Draw backgrounds of node missing their window
	KarmaGuiContext& g = *ctx;
	KGGuiDockContext* dc = &g.DockContext;
	for (int n = 0; n < dc->Nodes.Data.Size; n++)
		if (KGGuiDockNode* node = (KGGuiDockNode*)dc->Nodes.Data[n].val_p)
			if (node->LastFrameActive == g.FrameCount && node->IsVisible && node->HostWindow && node->IsLeafNode() && !node->IsBgDrawnThisFrame)
			{
				KGRect bg_rect(node->Pos + KGVec2(0.0f, KarmaGui::GetFrameHeight()), node->Pos + node->Size);
				KGDrawFlags bg_rounding_flags = CalcRoundingFlagsForRectInRect(bg_rect, node->HostWindow->Rect(), DOCKING_SPLITTER_SIZE);
				node->HostWindow->DrawList->ChannelsSetCurrent(DOCKING_HOST_DRAW_CHANNEL_BG);
				node->HostWindow->DrawList->AddRectFilled(bg_rect.Min, bg_rect.Max, node->LastBgColor, node->HostWindow->WindowRounding, bg_rounding_flags);
			}
}

KGGuiDockNode* Karma::KarmaGuiInternal::DockContextFindNodeByID(KarmaGuiContext* ctx, KGGuiID id)
{
	return (KGGuiDockNode*)ctx->DockContext.Nodes.GetVoidPtr(id);
}

KGGuiID Karma::KarmaGuiInternal::DockContextGenNodeID(KarmaGuiContext* ctx)
{
	// Generate an ID for new node (the exact ID value doesn't matter as long as it is not already used)
	// FIXME-OPT FIXME-DOCK: This is suboptimal, even if the node count is small enough not to be a worry.0
	// We should poke in ctx->Nodes to find a suitable ID faster. Even more so trivial that ctx->Nodes lookup is already sorted.
	KGGuiID id = 0x0001;
	while (DockContextFindNodeByID(ctx, id) != NULL)
		id++;
	return id;
}

KGGuiDockNode* Karma::DockContextAddNode(KarmaGuiContext* ctx, KGGuiID id)
{
	// Generate an ID for the new node (the exact ID value doesn't matter as long as it is not already used) and add the first window.
	KarmaGuiContext& g = *ctx;
	if (id == 0)
		id = Karma::KarmaGuiInternal::DockContextGenNodeID(ctx);
	else
		KR_CORE_ASSERT(Karma::KarmaGuiInternal::DockContextFindNodeByID(ctx, id) == NULL, "");

	// We don't set node->LastFrameAlive on construction. Nodes are always created at all time to reflect .ini settings!
	KR_CORE_INFO("[docking] DockContextAddNode {0}", id);
	KGGuiDockNode* node = KG_NEW(KGGuiDockNode)(id);
	ctx->DockContext.Nodes.SetVoidPtr(node->ID, node);
	return node;
}

void Karma::DockContextRemoveNode(KarmaGuiContext* ctx, KGGuiDockNode* node, bool merge_sibling_into_parent_node)
{
	KarmaGuiContext& g = *ctx;
	KGGuiDockContext* dc = &ctx->DockContext;

	KR_CORE_INFO("[docking] DockContextRemoveNode {0}", node->ID);
	KR_CORE_ASSERT(KarmaGuiInternal::DockContextFindNodeByID(ctx, node->ID) == node, "");
	KR_CORE_ASSERT(node->ChildNodes[0] == NULL && node->ChildNodes[1] == NULL, "");
	KR_CORE_ASSERT(node->Windows.Size == 0, "");

	if (node->HostWindow)
		node->HostWindow->DockNodeAsHost = NULL;

	KGGuiDockNode* parent_node = node->ParentNode;
	const bool merge = (merge_sibling_into_parent_node && parent_node != NULL);
	if (merge)
	{
		KR_CORE_ASSERT(parent_node->ChildNodes[0] == node || parent_node->ChildNodes[1] == node, "");
		KGGuiDockNode* sibling_node = (parent_node->ChildNodes[0] == node ? parent_node->ChildNodes[1] : parent_node->ChildNodes[0]);
		Karma::DockNodeTreeMerge(&g, parent_node, sibling_node);
	}
	else
	{
		for (int n = 0; parent_node && n < KG_ARRAYSIZE(parent_node->ChildNodes); n++)
			if (parent_node->ChildNodes[n] == node)
				node->ParentNode->ChildNodes[n] = NULL;
		dc->Nodes.SetVoidPtr(node->ID, NULL);
		KG_DELETE(node);
	}
}

static int DockNodeComparerDepthMostFirst(const void* lhs, const void* rhs)
{
	const KGGuiDockNode* a = *(const KGGuiDockNode* const*)lhs;
	const KGGuiDockNode* b = *(const KGGuiDockNode* const*)rhs;
	return Karma::KarmaGuiInternal::DockNodeGetDepth(b) - Karma::KarmaGuiInternal::DockNodeGetDepth(a);
}

// Pre C++0x doesn't allow us to use a function-local type (without linkage) as template parameter, so we moved this here.
struct ImGuiDockContextPruneNodeData
{
	int         CountWindows, CountChildWindows, CountChildNodes;
	KGGuiID     RootId;
	ImGuiDockContextPruneNodeData() { CountWindows = CountChildWindows = CountChildNodes = 0; RootId = 0; }
};

// Garbage collect unused nodes (run once at init time)
void Karma::DockContextPruneUnusedSettingsNodes(KarmaGuiContext* ctx)
{
	KarmaGuiContext& g = *ctx;
	KGGuiDockContext* dc = &ctx->DockContext;
	KR_CORE_ASSERT(g.Windows.Size == 0, "");

	KGPool<ImGuiDockContextPruneNodeData> pool;
	pool.Reserve(dc->NodesSettings.Size);

	// Count child nodes and compute RootID
	for (int settings_n = 0; settings_n < dc->NodesSettings.Size; settings_n++)
	{
		KGGuiDockNodeSettings* settings = &dc->NodesSettings[settings_n];
		ImGuiDockContextPruneNodeData* parent_data = settings->ParentNodeId ? pool.GetByKey(settings->ParentNodeId) : 0;
		pool.GetOrAddByKey(settings->ID)->RootId = parent_data ? parent_data->RootId : settings->ID;
		if (settings->ParentNodeId)
			pool.GetOrAddByKey(settings->ParentNodeId)->CountChildNodes++;
	}

	// Count reference to dock ids from dockspaces
	// We track the 'auto-DockNode <- manual-Window <- manual-DockSpace' in order to avoid 'auto-DockNode' being ditched by DockContextPruneUnusedSettingsNodes()
	for (int settings_n = 0; settings_n < dc->NodesSettings.Size; settings_n++)
	{
		KGGuiDockNodeSettings* settings = &dc->NodesSettings[settings_n];
		if (settings->ParentWindowId != 0)
			if (KGGuiWindowSettings* window_settings = Karma::KarmaGuiInternal::FindWindowSettings(settings->ParentWindowId))
				if (window_settings->DockId)
					if (ImGuiDockContextPruneNodeData* data = pool.GetByKey(window_settings->DockId))
						data->CountChildNodes++;
	}

	// Count reference to dock ids from window settings
	// We guard against the possibility of an invalid .ini file (RootID may point to a missing node)
	for (KGGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
		if (KGGuiID dock_id = settings->DockId)
			if (ImGuiDockContextPruneNodeData* data = pool.GetByKey(dock_id))
			{
				data->CountWindows++;
				if (ImGuiDockContextPruneNodeData* data_root = (data->RootId == dock_id) ? data : pool.GetByKey(data->RootId))
					data_root->CountChildWindows++;
			}

	// Prune
	for (int settings_n = 0; settings_n < dc->NodesSettings.Size; settings_n++)
	{
		KGGuiDockNodeSettings* settings = &dc->NodesSettings[settings_n];
		ImGuiDockContextPruneNodeData* data = pool.GetByKey(settings->ID);
		if (data->CountWindows > 1)
			continue;
		ImGuiDockContextPruneNodeData* data_root = (data->RootId == settings->ID) ? data : pool.GetByKey(data->RootId);

		bool remove = false;
		remove |= (data->CountWindows == 1 && settings->ParentNodeId == 0 && data->CountChildNodes == 0 && !(settings->Flags & KGGuiDockNodeFlags_CentralNode));  // Floating root node with only 1 window
		remove |= (data->CountWindows == 0 && settings->ParentNodeId == 0 && data->CountChildNodes == 0); // Leaf nodes with 0 window
		remove |= (data_root->CountChildWindows == 0);
		if (remove)
		{
			KR_CORE_INFO("[docking] DockContextPruneUnusedSettingsNodes: Prune {0}", settings->ID);
			Karma::DockSettingsRemoveNodeReferences(&settings->ID, 1);
			settings->ID = 0;
		}
	}
}

void Karma::DockContextBuildNodesFromSettings(KarmaGuiContext* ctx, KGGuiDockNodeSettings* node_settings_array, int node_settings_count)
{
	// Build nodes
	for (int node_n = 0; node_n < node_settings_count; node_n++)
	{
		KGGuiDockNodeSettings* settings = &node_settings_array[node_n];
		if (settings->ID == 0)
			continue;
		KGGuiDockNode* node = Karma::DockContextAddNode(ctx, settings->ID);
		node->ParentNode = settings->ParentNodeId ? Karma::KarmaGuiInternal::DockContextFindNodeByID(ctx, settings->ParentNodeId) : NULL;
		node->Pos = KGVec2(settings->Pos.x, settings->Pos.y);
		node->Size = KGVec2(settings->Size.x, settings->Size.y);
		node->SizeRef = KGVec2(settings->SizeRef.x, settings->SizeRef.y);
		node->AuthorityForPos = node->AuthorityForSize = node->AuthorityForViewport = KGGuiDataAuthority_DockNode;
		if (node->ParentNode && node->ParentNode->ChildNodes[0] == NULL)
			node->ParentNode->ChildNodes[0] = node;
		else if (node->ParentNode && node->ParentNode->ChildNodes[1] == NULL)
			node->ParentNode->ChildNodes[1] = node;
		node->SelectedTabId = settings->SelectedTabId;
		node->SplitAxis = (KGGuiAxis)settings->SplitAxis;
		node->SetLocalFlags(settings->Flags & KGGuiDockNodeFlags_SavedFlagsMask_);

		// Bind host window immediately if it already exist (in case of a rebuild)
		// This is useful as the RootWindowForTitleBarHighlight links necessary to highlight the currently focused node requires node->HostWindow to be set.
		char host_window_title[20];
		KGGuiDockNode* root_node = Karma::KarmaGuiInternal::DockNodeGetRootNode(node);
		node->HostWindow = Karma::KarmaGuiInternal::FindWindowByName(Karma::DockNodeGetHostWindowTitle(root_node, host_window_title, KG_ARRAYSIZE(host_window_title)));
	}
}

void Karma::DockContextBuildAddWindowsToNodes(KarmaGuiContext* ctx, KGGuiID root_id)
{
	// Rebind all windows to nodes (they can also lazily rebind but we'll have a visible glitch during the first frame)
	KarmaGuiContext& g = *ctx;
	for (int n = 0; n < g.Windows.Size; n++)
	{
		KGGuiWindow* window = g.Windows[n];
		if (window->DockId == 0 || window->LastFrameActive < g.FrameCount - 1)
			continue;
		if (window->DockNode != NULL)
			continue;

		KGGuiDockNode* node = Karma::KarmaGuiInternal::DockContextFindNodeByID(ctx, window->DockId);
		KR_CORE_ASSERT(node != NULL, "");   // This should have been called after DockContextBuildNodesFromSettings()
		if (root_id == 0 || Karma::KarmaGuiInternal::DockNodeGetRootNode(node)->ID == root_id)
			Karma::DockNodeAddWindow(node, window, true);
	}
}

//-----------------------------------------------------------------------------
// Docking: KGGuiDockContext Docking/Undocking functions
//-----------------------------------------------------------------------------
// - DockContextQueueDock()
// - DockContextQueueUndockWindow()
// - DockContextQueueUndockNode()
// - DockContextQueueNotifyRemovedNode()
// - DockContextProcessDock()
// - DockContextProcessUndockWindow()
// - DockContextProcessUndockNode()
// - DockContextCalcDropPosForDocking()
//-----------------------------------------------------------------------------

void Karma::KarmaGuiInternal::DockContextQueueDock(KarmaGuiContext* ctx, KGGuiWindow* target, KGGuiDockNode* target_node, KGGuiWindow* payload, KarmaGuiDir split_dir, float split_ratio, bool split_outer)
{
	KR_CORE_ASSERT(target != payload, "");
	KGGuiDockRequest req;
	req.Type = ImGuiDockRequestType_Dock;
	req.DockTargetWindow = target;
	req.DockTargetNode = target_node;
	req.DockPayload = payload;
	req.DockSplitDir = split_dir;
	req.DockSplitRatio = split_ratio;
	req.DockSplitOuter = split_outer;
	ctx->DockContext.Requests.push_back(req);
}

void Karma::KarmaGuiInternal::DockContextQueueUndockWindow(KarmaGuiContext* ctx, KGGuiWindow* window)
{
	KGGuiDockRequest req;
	req.Type = ImGuiDockRequestType_Undock;
	req.UndockTargetWindow = window;
	ctx->DockContext.Requests.push_back(req);
}

void Karma::KarmaGuiInternal::DockContextQueueUndockNode(KarmaGuiContext* ctx, KGGuiDockNode* node)
{
	KGGuiDockRequest req;
	req.Type = ImGuiDockRequestType_Undock;
	req.UndockTargetNode = node;
	ctx->DockContext.Requests.push_back(req);
}

void Karma::DockContextQueueNotifyRemovedNode(KarmaGuiContext* ctx, KGGuiDockNode* node)
{
	KGGuiDockContext* dc = &ctx->DockContext;
	for (int n = 0; n < dc->Requests.Size; n++)
		if (dc->Requests[n].DockTargetNode == node)
			dc->Requests[n].Type = ImGuiDockRequestType_None;
}

void Karma::DockContextProcessDock(KarmaGuiContext* ctx, KGGuiDockRequest* req)
{
	KR_CORE_ASSERT((req->Type == ImGuiDockRequestType_Dock && req->DockPayload != NULL) || (req->Type == ImGuiDockRequestType_Split && req->DockPayload == NULL), "");
	KR_CORE_ASSERT(req->DockTargetWindow != NULL || req->DockTargetNode != NULL, "");

	KarmaGuiContext& g = *ctx;
	KG_UNUSED(g);

	KGGuiWindow* payload_window = req->DockPayload;     // Optional
	KGGuiWindow* target_window = req->DockTargetWindow;
	KGGuiDockNode* node = req->DockTargetNode;
	if (payload_window)
	{
		KR_CORE_INFO("[docking] DockContextProcessDock node {0} target '{1}' dock window '{2}', split_dir {3}", node ? node->ID : 0, target_window ? target_window->Name : "NULL", payload_window->Name, req->DockSplitDir);
	}
	else
	{
		KR_CORE_INFO("[docking] DockContextProcessDock node {0}, split_dir {1}", node ? node->ID : 0, req->DockSplitDir);
	}

	// Decide which Tab will be selected at the end of the operation
	KGGuiID next_selected_id = 0;
	KGGuiDockNode* payload_node = NULL;
	if (payload_window)
	{
		payload_node = payload_window->DockNodeAsHost;
		payload_window->DockNodeAsHost = NULL; // Important to clear this as the node will have its life as a child which might be merged/deleted later.
		if (payload_node && payload_node->IsLeafNode())
			next_selected_id = payload_node->TabBar->NextSelectedTabId ? payload_node->TabBar->NextSelectedTabId : payload_node->TabBar->SelectedTabId;
		if (payload_node == NULL)
			next_selected_id = payload_window->TabId;
	}

	// FIXME-DOCK: When we are trying to dock an existing single-window node into a loose window, transfer Node ID as well
	// When processing an interactive split, usually LastFrameAlive will be < g.FrameCount. But DockBuilder operations can make it ==.
	if (node)
	{
		KR_CORE_ASSERT(node->LastFrameAlive <= g.FrameCount, "");
	}
	if (node && target_window && node == target_window->DockNodeAsHost)
	{
		KR_CORE_ASSERT(node->Windows.Size > 0 || node->IsSplitNode() || node->IsCentralNode(), "");
	}

	// Create new node and add existing window to it
	if (node == NULL)
	{
		node = Karma::DockContextAddNode(ctx, 0);
		node->Pos = target_window->Pos;
		node->Size = target_window->Size;
		if (target_window->DockNodeAsHost == NULL)
		{
			Karma::DockNodeAddWindow(node, target_window, true);
			node->TabBar->Tabs[0].Flags &= ~KGGuiTabItemFlags_Unsorted;
			target_window->DockIsActive = true;
		}
	}

	KarmaGuiDir split_dir = req->DockSplitDir;
	if (split_dir != KGGuiDir_None)
	{
		// Split into two, one side will be our payload node unless we are dropping a loose window
		const KGGuiAxis split_axis = (split_dir == KGGuiDir_Left || split_dir == KGGuiDir_Right) ? KGGuiAxis_X : KGGuiAxis_Y;
		const int split_inheritor_child_idx = (split_dir == KGGuiDir_Left || split_dir == KGGuiDir_Up) ? 1 : 0; // Current contents will be moved to the opposite side
		const float split_ratio = req->DockSplitRatio;
		Karma::DockNodeTreeSplit(ctx, node, split_axis, split_inheritor_child_idx, split_ratio, payload_node);  // payload_node may be NULL here!
		KGGuiDockNode* new_node = node->ChildNodes[split_inheritor_child_idx ^ 1];
		new_node->HostWindow = node->HostWindow;
		node = new_node;
	}
	node->SetLocalFlags(node->LocalFlags & ~KGGuiDockNodeFlags_HiddenTabBar);

	if (node != payload_node)
	{
		// Create tab bar before we call DockNodeMoveWindows (which would attempt to move the old tab-bar, which would lead us to payload tabs wrongly appearing before target tabs!)
		if (node->Windows.Size > 0 && node->TabBar == NULL)
		{
			Karma::DockNodeAddTabBar(node);
			for (int n = 0; n < node->Windows.Size; n++)
				Karma::KarmaGuiInternal::TabBarAddTab(node->TabBar, KGGuiTabItemFlags_None, node->Windows[n]);
		}

		if (payload_node != NULL)
		{
			// Transfer full payload node (with 1+ child windows or child nodes)
			if (payload_node->IsSplitNode())
			{
				if (node->Windows.Size > 0)
				{
					// We can dock a split payload into a node that already has windows _only_ if our payload is a node tree with a single visible node.
					// In this situation, we move the windows of the target node into the currently visible node of the payload.
					// This allows us to preserve some of the underlying dock tree settings nicely.
					KR_CORE_ASSERT(payload_node->OnlyNodeWithWindows != NULL, ""); // The docking should have been blocked by DockNodePreviewDockSetup() early on and never submitted.
					KGGuiDockNode* visible_node = payload_node->OnlyNodeWithWindows;
					if (visible_node->TabBar)
					{
						KR_CORE_ASSERT(visible_node->TabBar->Tabs.Size > 0, "");
					}
					Karma::DockNodeMoveWindows(node, visible_node);
					Karma::DockNodeMoveWindows(visible_node, node);
					Karma::DockSettingsRenameNodeReferences(node->ID, visible_node->ID);
				}
				if (node->IsCentralNode())
				{
					// Central node property needs to be moved to a leaf node, pick the last focused one.
					// FIXME-DOCK: If we had to transfer other flags here, what would the policy be?
					KGGuiDockNode* last_focused_node = Karma::KarmaGuiInternal::DockContextFindNodeByID(ctx, payload_node->LastFocusedNodeId);
					KR_CORE_ASSERT(last_focused_node != NULL, "");
					KGGuiDockNode* last_focused_root_node = Karma::KarmaGuiInternal::DockNodeGetRootNode(last_focused_node);
					KR_CORE_ASSERT(last_focused_root_node == Karma::KarmaGuiInternal::DockNodeGetRootNode(payload_node), "");
					last_focused_node->SetLocalFlags(last_focused_node->LocalFlags | KGGuiDockNodeFlags_CentralNode);
					node->SetLocalFlags(node->LocalFlags & ~KGGuiDockNodeFlags_CentralNode);
					last_focused_root_node->CentralNode = last_focused_node;
				}

				KR_CORE_ASSERT(node->Windows.Size == 0, "");
				Karma::DockNodeMoveChildNodes(node, payload_node);
			}
			else
			{
				const KGGuiID payload_dock_id = payload_node->ID;
				Karma::DockNodeMoveWindows(node, payload_node);
				Karma::DockSettingsRenameNodeReferences(payload_dock_id, node->ID);
			}
			Karma::DockContextRemoveNode(ctx, payload_node, true);
		}
		else if (payload_window)
		{
			// Transfer single window
			const KGGuiID payload_dock_id = payload_window->DockId;
			node->VisibleWindow = payload_window;
			Karma::DockNodeAddWindow(node, payload_window, true);
			if (payload_dock_id != 0)
				Karma::DockSettingsRenameNodeReferences(payload_dock_id, node->ID);
		}
	}
	else
	{
		// When docking a floating single window node we want to reevaluate auto-hiding of the tab bar
		node->WantHiddenTabBarUpdate = true;
	}

	// Update selection immediately
	if (KGGuiTabBar* tab_bar = node->TabBar)
		tab_bar->NextSelectedTabId = next_selected_id;
	Karma::KarmaGuiInternal::MarkIniSettingsDirty();
}

// Problem:
//   Undocking a large (~full screen) window would leave it so large that the bottom right sizing corner would more
//   than likely be off the screen and the window would be hard to resize to fit on screen. This can be particularly problematic
//   with 'ConfigWindowsMoveFromTitleBarOnly=true' and/or with 'ConfigWindowsResizeFromEdges=false' as well (the later can be
//   due to missing KGGuiBackendFlags_HasMouseCursors backend flag).
// Solution:
//   When undocking a window we currently force its maximum size to 90% of the host viewport or monitor.
// Reevaluate this when we implement preserving docked/undocked size ("docking_wip/undocked_size" branch).
static KGVec2 FixLargeWindowsWhenUndocking(const KGVec2& size, KarmaGuiViewport* ref_viewport)
{
	if (ref_viewport == NULL)
		return size;

	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGVec2 max_size = KGFloor(ref_viewport->WorkSize * 0.90f);
	if (g.ConfigFlagsCurrFrame & KGGuiConfigFlags_ViewportsEnable)
	{
		const KarmaGuiPlatformMonitor* monitor = Karma::KarmaGuiInternal::GetViewportPlatformMonitor(ref_viewport);
		max_size = KGFloor(monitor->WorkSize * 0.90f);
	}
	return KGMin(size, max_size);
}

void Karma::DockContextProcessUndockWindow(KarmaGuiContext* ctx, KGGuiWindow* window, bool clear_persistent_docking_ref)
{
	KarmaGuiContext& g = *ctx;
	KR_CORE_INFO("[docking] DockContextProcessUndockWindow window '{0}', clear_persistent_docking_ref = {1}", window->Name, clear_persistent_docking_ref);
	if (window->DockNode)
		Karma::DockNodeRemoveWindow(window->DockNode, window, clear_persistent_docking_ref ? 0 : window->DockId);
	else
		window->DockId = 0;
	window->Collapsed = false;
	window->DockIsActive = false;
	window->DockNodeIsVisible = window->DockTabIsVisible = false;
	window->Size = window->SizeFull = FixLargeWindowsWhenUndocking(window->SizeFull, window->Viewport);

	Karma::KarmaGuiInternal::MarkIniSettingsDirty();
}

void Karma::DockContextProcessUndockNode(KarmaGuiContext* ctx, KGGuiDockNode* node)
{
	KarmaGuiContext& g = *ctx;
	KR_CORE_INFO("[docking] DockContextProcessUndockNode node {0}", node->ID);
	KR_CORE_ASSERT(node->IsLeafNode(), "");
	KR_CORE_ASSERT(node->Windows.Size >= 1, "");

	if (node->IsRootNode() || node->IsCentralNode())
	{
		// In the case of a root node or central node, the node will have to stay in place. Create a new node to receive the payload.
		KGGuiDockNode* new_node = Karma::DockContextAddNode(ctx, 0);
		new_node->Pos = node->Pos;
		new_node->Size = node->Size;
		new_node->SizeRef = node->SizeRef;
		Karma::DockNodeMoveWindows(new_node, node);
		Karma::DockSettingsRenameNodeReferences(node->ID, new_node->ID);
		node = new_node;
	}
	else
	{
		// Otherwise extract our node and merge our sibling back into the parent node.
		KR_CORE_ASSERT(node->ParentNode->ChildNodes[0] == node || node->ParentNode->ChildNodes[1] == node, "");
		int index_in_parent = (node->ParentNode->ChildNodes[0] == node) ? 0 : 1;
		node->ParentNode->ChildNodes[index_in_parent] = NULL;
		Karma::DockNodeTreeMerge(ctx, node->ParentNode, node->ParentNode->ChildNodes[index_in_parent ^ 1]);
		node->ParentNode->AuthorityForViewport = KGGuiDataAuthority_Window; // The node that stays in place keeps the viewport, so our newly dragged out node will create a new viewport
		node->ParentNode = NULL;
	}
	for (int n = 0; n < node->Windows.Size; n++)
	{
		KGGuiWindow* window = node->Windows[n];
		window->Flags &= ~KGGuiWindowFlags_ChildWindow;
		if (window->ParentWindow)
			window->ParentWindow->DC.ChildWindows.find_erase(window);
		Karma::KarmaGuiInternal::UpdateWindowParentAndRootLinks(window, window->Flags, NULL);
	}
	node->AuthorityForPos = node->AuthorityForSize = KGGuiDataAuthority_DockNode;
	node->Size = FixLargeWindowsWhenUndocking(node->Size, node->Windows[0]->Viewport);
	node->WantMouseMove = true;
	Karma::KarmaGuiInternal::MarkIniSettingsDirty();
}

// no context
// This is mostly used for automation.
bool DockContextCalcDropPosForDocking(KGGuiWindow* target, KGGuiDockNode* target_node, KGGuiWindow* payload_window, KGGuiDockNode* payload_node, KarmaGuiDir split_dir, bool split_outer, KGVec2* out_pos)
{
	// In DockNodePreviewDockSetup() for a root central node instead of showing both "inner" and "outer" drop rects
	// (which would be functionally identical) we only show the outer one. Reflect this here.
	if (target_node && target_node->ParentNode == NULL && target_node->IsCentralNode() && split_dir != KGGuiDir_None)
		split_outer = true;
	ImGuiDockPreviewData split_data;
	Karma::DockNodePreviewDockSetup(target, target_node, payload_window, payload_node, &split_data, false, split_outer);
	if (split_data.DropRectsDraw[split_dir + 1].IsInverted())
		return false;
	*out_pos = split_data.DropRectsDraw[split_dir + 1].GetCenter();
	return true;
}

//-----------------------------------------------------------------------------
// Docking: KGGuiDockNode
//-----------------------------------------------------------------------------
// - DockNodeGetTabOrder()
// - DockNodeAddWindow()
// - DockNodeRemoveWindow()
// - DockNodeMoveChildNodes()
// - DockNodeMoveWindows()
// - DockNodeApplyPosSizeToWindows()
// - DockNodeHideHostWindow()
// - ImGuiDockNodeFindInfoResults
// - DockNodeFindInfo()
// - DockNodeFindWindowByID()
// - DockNodeUpdateFlagsAndCollapse()
// - DockNodeUpdateHasCentralNodeFlag()
// - DockNodeUpdateVisibleFlag()
// - DockNodeStartMouseMovingWindow()
// - DockNodeUpdate()
// - DockNodeUpdateWindowMenu()
// - DockNodeBeginAmendTabBar()
// - DockNodeEndAmendTabBar()
// - DockNodeUpdateTabBar()
// - DockNodeAddTabBar()
// - DockNodeRemoveTabBar()
// - DockNodeIsDropAllowedOne()
// - DockNodeIsDropAllowed()
// - DockNodeCalcTabBarLayout()
// - DockNodeCalcSplitRects()
// - DockNodeCalcDropRectsAndTestMousePos()
// - DockNodePreviewDockSetup()
// - DockNodePreviewDockRender()
//-----------------------------------------------------------------------------

KGGuiDockNode::KGGuiDockNode(KGGuiID id)
{
	ID = id;
	SharedFlags = LocalFlags = LocalFlagsInWindows = MergedFlags = KGGuiDockNodeFlags_None;
	ParentNode = ChildNodes[0] = ChildNodes[1] = NULL;
	TabBar = NULL;
	SplitAxis = KGGuiAxis_None;

	State = KGGuiDockNodeState_Unknown;
	LastBgColor = KG_COL32_WHITE;
	HostWindow = VisibleWindow = NULL;
	CentralNode = OnlyNodeWithWindows = NULL;
	CountNodeWithWindows = 0;
	LastFrameAlive = LastFrameActive = LastFrameFocused = -1;
	LastFocusedNodeId = 0;
	SelectedTabId = 0;
	WantCloseTabId = 0;
	AuthorityForPos = AuthorityForSize = KGGuiDataAuthority_DockNode;
	AuthorityForViewport = KGGuiDataAuthority_Auto;
	IsVisible = true;
	IsFocused = HasCloseButton = HasWindowMenuButton = HasCentralNodeChild = false;
	IsBgDrawnThisFrame = false;
	WantCloseAll = WantLockSizeOnce = WantMouseMove = WantHiddenTabBarUpdate = WantHiddenTabBarToggle = false;
}

KGGuiDockNode::~KGGuiDockNode()
{
	KG_DELETE(TabBar);
	TabBar = NULL;
	ChildNodes[0] = ChildNodes[1] = NULL;
}

int Karma::DockNodeGetTabOrder(KGGuiWindow* window)
{
	KGGuiTabBar* tab_bar = window->DockNode->TabBar;
	if (tab_bar == NULL)
		return -1;
	KGGuiTabItem* tab = Karma::KarmaGuiInternal::TabBarFindTabByID(tab_bar, window->TabId);
	return tab ? tab_bar->GetTabOrder(tab) : -1;
}

static void DockNodeHideWindowDuringHostWindowCreation(KGGuiWindow* window)
{
	window->Hidden = true;
	window->HiddenFramesCanSkipItems = window->Active ? 1 : 2;
}

static void Karma::DockNodeAddWindow(KGGuiDockNode* node, KGGuiWindow* window, bool add_to_tab_bar)
{
	KarmaGuiContext& g = *GKarmaGui; (void)g;
	if (window->DockNode)
	{
		// Can overwrite an existing window->DockNode (e.g. pointing to a disabled DockSpace node)
		KR_CORE_ASSERT(window->DockNode->ID != node->ID, "");
		Karma::DockNodeRemoveWindow(window->DockNode, window, 0);
	}
	KR_CORE_ASSERT(window->DockNode == NULL || window->DockNodeAsHost == NULL, "");
	KR_CORE_INFO("[docking] DockNodeAddWindow node 0x%08X window '{0}'\n", node->ID, window->Name);

	// If more than 2 windows appeared on the same frame leading to the creation of a new hosting window,
	// we'll hide windows until the host window is ready. Hide the 1st window after its been output (so it is not visible for one frame).
	// We will call DockNodeHideWindowDuringHostWindowCreation() on ourselves in Begin()
	if (node->HostWindow == NULL && node->Windows.Size == 1 && node->Windows[0]->WasActive == false)
		DockNodeHideWindowDuringHostWindowCreation(node->Windows[0]);

	node->Windows.push_back(window);
	node->WantHiddenTabBarUpdate = true;
	window->DockNode = node;
	window->DockId = node->ID;
	window->DockIsActive = (node->Windows.Size > 1);
	window->DockTabWantClose = false;

	// When reactivating a node with one or two loose window, the window pos/size/viewport are authoritative over the node storage.
	// In particular it is important we init the viewport from the first window so we don't create two viewports and drop one.
	if (node->HostWindow == NULL && node->IsFloatingNode())
	{
		if (node->AuthorityForPos == KGGuiDataAuthority_Auto)
			node->AuthorityForPos = KGGuiDataAuthority_Window;
		if (node->AuthorityForSize == KGGuiDataAuthority_Auto)
			node->AuthorityForSize = KGGuiDataAuthority_Window;
		if (node->AuthorityForViewport == KGGuiDataAuthority_Auto)
			node->AuthorityForViewport = KGGuiDataAuthority_Window;
	}

	// Add to tab bar if requested
	if (add_to_tab_bar)
	{
		if (node->TabBar == NULL)
		{
			Karma::DockNodeAddTabBar(node);
			node->TabBar->SelectedTabId = node->TabBar->NextSelectedTabId = node->SelectedTabId;

			// Add existing windows
			for (int n = 0; n < node->Windows.Size - 1; n++)
				Karma::KarmaGuiInternal::TabBarAddTab(node->TabBar, KGGuiTabItemFlags_None, node->Windows[n]);
		}
		Karma::KarmaGuiInternal::TabBarAddTab(node->TabBar, KGGuiTabItemFlags_Unsorted, window);
	}

	Karma::DockNodeUpdateVisibleFlag(node);

	// Update this without waiting for the next time we Begin() in the window, so our host window will have the proper title bar color on its first frame.
	if (node->HostWindow)
		Karma::KarmaGuiInternal::UpdateWindowParentAndRootLinks(window, window->Flags | KGGuiWindowFlags_ChildWindow, node->HostWindow);
}

void Karma::DockNodeRemoveWindow(KGGuiDockNode* node, KGGuiWindow* window, KGGuiID save_dock_id)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(window->DockNode == node, "");
	//KR_CORE_ASSERT(window->RootWindowDockTree == node->HostWindow);
	//KR_CORE_ASSERT(window->LastFrameActive < g.FrameCount);    // We may call this from Begin()
	KR_CORE_ASSERT(save_dock_id == 0 || save_dock_id == node->ID, "");
	KR_CORE_INFO("[docking] DockNodeRemoveWindow node 0x%08X window '{0}'\n", node->ID, window->Name);

	window->DockNode = NULL;
	window->DockIsActive = window->DockTabWantClose = false;
	window->DockId = save_dock_id;
	window->Flags &= ~KGGuiWindowFlags_ChildWindow;
	if (window->ParentWindow)
		window->ParentWindow->DC.ChildWindows.find_erase(window);
	Karma::KarmaGuiInternal::UpdateWindowParentAndRootLinks(window, window->Flags, NULL); // Update immediately

	// Remove window
	bool erased = false;
	for (int n = 0; n < node->Windows.Size; n++)
		if (node->Windows[n] == window)
		{
			node->Windows.erase(node->Windows.Data + n);
			erased = true;
			break;
		}
	if (!erased)
	{
		KR_CORE_ASSERT(erased, "");
	}
	if (node->VisibleWindow == window)
		node->VisibleWindow = NULL;

	// Remove tab and possibly tab bar
	node->WantHiddenTabBarUpdate = true;
	if (node->TabBar)
	{
		Karma::KarmaGuiInternal::TabBarRemoveTab(node->TabBar, window->TabId);
		const int tab_count_threshold_for_tab_bar = node->IsCentralNode() ? 1 : 2;
		if (node->Windows.Size < tab_count_threshold_for_tab_bar)
			Karma::DockNodeRemoveTabBar(node);
	}

	if (node->Windows.Size == 0 && !node->IsCentralNode() && !node->IsDockSpace() && window->DockId != node->ID)
	{
		// Automatic dock node delete themselves if they are not holding at least one tab
		Karma::DockContextRemoveNode(&g, node, true);
		return;
	}

	if (node->Windows.Size == 1 && !node->IsCentralNode() && node->HostWindow)
	{
		KGGuiWindow* remaining_window = node->Windows[0];
		if (node->HostWindow->ViewportOwned && node->IsRootNode())
		{
			// Transfer viewport back to the remaining loose window
			KR_CORE_INFO("[viewport] Node {0} transfer Viewport {1}=>{2} for Window '{3}'", node->ID, node->HostWindow->Viewport->ID, remaining_window->ID, remaining_window->Name);
			KR_CORE_ASSERT(node->HostWindow->Viewport->Window == node->HostWindow, "");
			node->HostWindow->Viewport->Window = remaining_window;
			node->HostWindow->Viewport->ID = remaining_window->ID;
		}
		remaining_window->Collapsed = node->HostWindow->Collapsed;
	}

	// Update visibility immediately is required so the DockNodeUpdateRemoveInactiveChilds() processing can reflect changes up the tree
	Karma::DockNodeUpdateVisibleFlag(node);
}

void Karma::DockNodeMoveChildNodes(KGGuiDockNode* dst_node, KGGuiDockNode* src_node)
{
	KR_CORE_ASSERT(dst_node->Windows.Size == 0, "");
	dst_node->ChildNodes[0] = src_node->ChildNodes[0];
	dst_node->ChildNodes[1] = src_node->ChildNodes[1];
	if (dst_node->ChildNodes[0])
		dst_node->ChildNodes[0]->ParentNode = dst_node;
	if (dst_node->ChildNodes[1])
		dst_node->ChildNodes[1]->ParentNode = dst_node;
	dst_node->SplitAxis = src_node->SplitAxis;
	dst_node->SizeRef = src_node->SizeRef;
	src_node->ChildNodes[0] = src_node->ChildNodes[1] = NULL;
}

void Karma::DockNodeMoveWindows(KGGuiDockNode* dst_node, KGGuiDockNode* src_node)
{
	// Insert tabs in the same orders as currently ordered (node->Windows isn't ordered)
	KR_CORE_ASSERT(src_node && dst_node && dst_node != src_node, "");
	KGGuiTabBar* src_tab_bar = src_node->TabBar;
	if (src_tab_bar != NULL)
	{
		KR_CORE_ASSERT(src_node->Windows.Size <= src_node->TabBar->Tabs.Size, "");
	}

	// If the dst_node is empty we can just move the entire tab bar (to preserve selection, scrolling, etc.)
	bool move_tab_bar = (src_tab_bar != NULL) && (dst_node->TabBar == NULL);
	if (move_tab_bar)
	{
		dst_node->TabBar = src_node->TabBar;
		src_node->TabBar = NULL;
	}

	// Tab order is not important here, it is preserved by sorting in DockNodeUpdateTabBar().
	for (KGGuiWindow* window : src_node->Windows)
	{
		window->DockNode = NULL;
		window->DockIsActive = false;
		Karma::DockNodeAddWindow(dst_node, window, !move_tab_bar);
	}
	src_node->Windows.clear();

	if (!move_tab_bar && src_node->TabBar)
	{
		if (dst_node->TabBar)
			dst_node->TabBar->SelectedTabId = src_node->TabBar->SelectedTabId;
		Karma::DockNodeRemoveTabBar(src_node);
	}
}

void Karma::DockNodeApplyPosSizeToWindows(KGGuiDockNode* node)
{
	for (int n = 0; n < node->Windows.Size; n++)
	{
		Karma::KarmaGuiInternal::SetWindowPos(node->Windows[n], node->Pos, KGGuiCond_Always); // We don't assign directly to Pos because it can break the calculation of SizeContents on next frame
		Karma::KarmaGuiInternal::SetWindowSize(node->Windows[n], node->Size, KGGuiCond_Always);
	}
}

void Karma::DockNodeHideHostWindow(KGGuiDockNode* node)
{
	if (node->HostWindow)
	{
		if (node->HostWindow->DockNodeAsHost == node)
			node->HostWindow->DockNodeAsHost = NULL;
		node->HostWindow = NULL;
	}

	if (node->Windows.Size == 1)
	{
		node->VisibleWindow = node->Windows[0];
		node->Windows[0]->DockIsActive = false;
	}

	if (node->TabBar)
		Karma::DockNodeRemoveTabBar(node);
}

// Search function called once by root node in DockNodeUpdate()
struct ImGuiDockNodeTreeInfo
{
	KGGuiDockNode* CentralNode;
	KGGuiDockNode* FirstNodeWithWindows;
	int                 CountNodesWithWindows;
	//KarmaGuiWindowClass  WindowClassForMerges;

	ImGuiDockNodeTreeInfo() { memset(this, 0, sizeof(*this)); }
};

static void DockNodeFindInfo(KGGuiDockNode* node, ImGuiDockNodeTreeInfo* info)
{
	if (node->Windows.Size > 0)
	{
		if (info->FirstNodeWithWindows == NULL)
			info->FirstNodeWithWindows = node;
		info->CountNodesWithWindows++;
	}
	if (node->IsCentralNode())
	{
		KR_CORE_ASSERT(info->CentralNode == NULL, ""); // Should be only one
		KR_CORE_ASSERT(node->IsLeafNode(), "If you get this assert: please submit .ini file + repro of actions leading to this.");
		info->CentralNode = node;
	}
	if (info->CountNodesWithWindows > 1 && info->CentralNode != NULL)
		return;
	if (node->ChildNodes[0])
		DockNodeFindInfo(node->ChildNodes[0], info);
	if (node->ChildNodes[1])
		DockNodeFindInfo(node->ChildNodes[1], info);
}

KGGuiWindow* Karma::DockNodeFindWindowByID(KGGuiDockNode* node, KGGuiID id)
{
	KR_CORE_ASSERT(id != 0, "");
	for (int n = 0; n < node->Windows.Size; n++)
		if (node->Windows[n]->ID == id)
			return node->Windows[n];
	return NULL;
}

// - Remove inactive windows/nodes.
// - Update visibility flag.
void Karma::DockNodeUpdateFlagsAndCollapse(KGGuiDockNode* node)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(node->ParentNode == NULL || node->ParentNode->ChildNodes[0] == node || node->ParentNode->ChildNodes[1] == node, "");

	// Inherit most flags
	if (node->ParentNode)
		node->SharedFlags = node->ParentNode->SharedFlags & KGGuiDockNodeFlags_SharedFlagsInheritMask_;

	// Recurse into children
	// There is the possibility that one of our child becoming empty will delete itself and moving its sibling contents into 'node'.
	// If 'node->ChildNode[0]' delete itself, then 'node->ChildNode[1]->Windows' will be moved into 'node'
	// If 'node->ChildNode[1]' delete itself, then 'node->ChildNode[0]->Windows' will be moved into 'node' and the "remove inactive windows" loop will have run twice on those windows (harmless)
	node->HasCentralNodeChild = false;
	if (node->ChildNodes[0])
		Karma::DockNodeUpdateFlagsAndCollapse(node->ChildNodes[0]);
	if (node->ChildNodes[1])
		Karma::DockNodeUpdateFlagsAndCollapse(node->ChildNodes[1]);

	// Remove inactive windows, collapse nodes
	// Merge node flags overrides stored in windows
	node->LocalFlagsInWindows = KGGuiDockNodeFlags_None;
	for (int window_n = 0; window_n < node->Windows.Size; window_n++)
	{
		KGGuiWindow* window = node->Windows[window_n];
		KR_CORE_ASSERT(window->DockNode == node, "");

		bool node_was_active = (node->LastFrameActive + 1 == g.FrameCount);
		bool remove = false;
		remove |= node_was_active && (window->LastFrameActive + 1 < g.FrameCount);
		remove |= node_was_active && (node->WantCloseAll || node->WantCloseTabId == window->TabId) && window->HasCloseButton && !(window->Flags & KGGuiWindowFlags_UnsavedDocument);  // Submit all _expected_ closure from last frame
		remove |= (window->DockTabWantClose);
		if (remove)
		{
			window->DockTabWantClose = false;
			if (node->Windows.Size == 1 && !node->IsCentralNode())
			{
				Karma::DockNodeHideHostWindow(node);
				node->State = KGGuiDockNodeState_HostWindowHiddenBecauseSingleWindow;
				Karma::DockNodeRemoveWindow(node, window, node->ID); // Will delete the node so it'll be invalid on return
				return;
			}
			Karma::DockNodeRemoveWindow(node, window, node->ID);
			window_n--;
			continue;
		}

		// FIXME-DOCKING: Missing policies for conflict resolution, hence the "Experimental" tag on this.
		//node->LocalFlagsInWindow &= ~window->WindowClass.DockNodeFlagsOverrideClear;
		node->LocalFlagsInWindows |= window->WindowClass.DockNodeFlagsOverrideSet;
	}
	node->UpdateMergedFlags();

	// Auto-hide tab bar option
	KarmaGuiDockNodeFlags node_flags = node->MergedFlags;
	if (node->WantHiddenTabBarUpdate && node->Windows.Size == 1 && (node_flags & KGGuiDockNodeFlags_AutoHideTabBar) && !node->IsHiddenTabBar())
		node->WantHiddenTabBarToggle = true;
	node->WantHiddenTabBarUpdate = false;

	// Cancel toggling if we know our tab bar is enforced to be hidden at all times
	if (node->WantHiddenTabBarToggle && node->VisibleWindow && (node->VisibleWindow->WindowClass.DockNodeFlagsOverrideSet & KGGuiDockNodeFlags_HiddenTabBar))
		node->WantHiddenTabBarToggle = false;

	// Apply toggles at a single point of the frame (here!)
	if (node->Windows.Size > 1)
		node->SetLocalFlags(node->LocalFlags & ~KGGuiDockNodeFlags_HiddenTabBar);
	else if (node->WantHiddenTabBarToggle)
		node->SetLocalFlags(node->LocalFlags ^ KGGuiDockNodeFlags_HiddenTabBar);
	node->WantHiddenTabBarToggle = false;

	Karma::DockNodeUpdateVisibleFlag(node);
}

// This is rarely called as DockNodeUpdateForRootNode() generally does it most frames.
void Karma::DockNodeUpdateHasCentralNodeChild(KGGuiDockNode* node)
{
	node->HasCentralNodeChild = false;
	if (node->ChildNodes[0])
		Karma::DockNodeUpdateHasCentralNodeChild(node->ChildNodes[0]);
	if (node->ChildNodes[1])
		Karma::DockNodeUpdateHasCentralNodeChild(node->ChildNodes[1]);
	if (node->IsRootNode())
	{
		KGGuiDockNode* mark_node = node->CentralNode;
		while (mark_node)
		{
			mark_node->HasCentralNodeChild = true;
			mark_node = mark_node->ParentNode;
		}
	}
}

void Karma::DockNodeUpdateVisibleFlag(KGGuiDockNode* node)
{
	// Update visibility flag
	bool is_visible = (node->ParentNode == NULL) ? node->IsDockSpace() : node->IsCentralNode();
	is_visible |= (node->Windows.Size > 0);
	is_visible |= (node->ChildNodes[0] && node->ChildNodes[0]->IsVisible);
	is_visible |= (node->ChildNodes[1] && node->ChildNodes[1]->IsVisible);
	node->IsVisible = is_visible;
}

void Karma::DockNodeStartMouseMovingWindow(KGGuiDockNode* node, KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(node->WantMouseMove == true, "");
	Karma::KarmaGuiInternal::StartMouseMovingWindow(window);
	g.ActiveIdClickOffset = g.IO.MouseClickedPos[0] - node->Pos;
	g.MovingWindow = window; // If we are docked into a non moveable root window, StartMouseMovingWindow() won't set g.MovingWindow. Override that decision.
	node->WantMouseMove = false;
}

// Update CentralNode, OnlyNodeWithWindows, LastFocusedNodeID. Copy window class.
void Karma::DockNodeUpdateForRootNode(KGGuiDockNode* node)
{
	Karma::DockNodeUpdateFlagsAndCollapse(node);

	// - Setup central node pointers
	// - Find if there's only a single visible window in the hierarchy (in which case we need to display a regular title bar -> FIXME-DOCK: that last part is not done yet!)
	// Cannot merge this with DockNodeUpdateFlagsAndCollapse() because FirstNodeWithWindows is found after window removal and child collapsing
	ImGuiDockNodeTreeInfo info;
	DockNodeFindInfo(node, &info);
	node->CentralNode = info.CentralNode;
	node->OnlyNodeWithWindows = (info.CountNodesWithWindows == 1) ? info.FirstNodeWithWindows : NULL;
	node->CountNodeWithWindows = info.CountNodesWithWindows;
	if (node->LastFocusedNodeId == 0 && info.FirstNodeWithWindows != NULL)
		node->LastFocusedNodeId = info.FirstNodeWithWindows->ID;

	// Copy the window class from of our first window so it can be used for proper dock filtering.
	// When node has mixed windows, prioritize the class with the most constraint (DockingAllowUnclassed = false) as the reference to copy.
	// FIXME-DOCK: We don't recurse properly, this code could be reworked to work from DockNodeUpdateScanRec.
	if (KGGuiDockNode* first_node_with_windows = info.FirstNodeWithWindows)
	{
		node->WindowClass = first_node_with_windows->Windows[0]->WindowClass;
		for (int n = 1; n < first_node_with_windows->Windows.Size; n++)
			if (first_node_with_windows->Windows[n]->WindowClass.DockingAllowUnclassed == false)
			{
				node->WindowClass = first_node_with_windows->Windows[n]->WindowClass;
				break;
			}
	}

	KGGuiDockNode* mark_node = node->CentralNode;
	while (mark_node)
	{
		mark_node->HasCentralNodeChild = true;
		mark_node = mark_node->ParentNode;
	}
}

static void DockNodeSetupHostWindow(KGGuiDockNode* node, KGGuiWindow* host_window)
{
	// Remove ourselves from any previous different host window
	// This can happen if a user mistakenly does (see #4295 for details):
	//  - N+0: DockBuilderAddNode(id, 0)    // missing KGGuiDockNodeFlags_DockSpace
	//  - N+1: NewFrame()                   // will create floating host window for that node
	//  - N+1: DockSpace(id)                // requalify node as dockspace, moving host window
	if (node->HostWindow && node->HostWindow != host_window && node->HostWindow->DockNodeAsHost == node)
		node->HostWindow->DockNodeAsHost = NULL;

	host_window->DockNodeAsHost = node;
	node->HostWindow = host_window;
}

void Karma::DockNodeUpdate(KGGuiDockNode* node)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(node->LastFrameActive != g.FrameCount, "");
	node->LastFrameAlive = g.FrameCount;
	node->IsBgDrawnThisFrame = false;

	node->CentralNode = node->OnlyNodeWithWindows = NULL;
	if (node->IsRootNode())
		Karma::DockNodeUpdateForRootNode(node);

	// Remove tab bar if not needed
	if (node->TabBar && node->IsNoTabBar())
		Karma::DockNodeRemoveTabBar(node);

	// Early out for hidden root dock nodes (when all DockId references are in inactive windows, or there is only 1 floating window holding on the DockId)
	bool want_to_hide_host_window = false;
	if (node->IsFloatingNode())
	{
		if (node->Windows.Size <= 1 && node->IsLeafNode())
			if (!g.IO.ConfigDockingAlwaysTabBar && (node->Windows.Size == 0 || !node->Windows[0]->WindowClass.DockingAlwaysTabBar))
				want_to_hide_host_window = true;
		if (node->CountNodeWithWindows == 0)
			want_to_hide_host_window = true;
	}
	if (want_to_hide_host_window)
	{
		if (node->Windows.Size == 1)
		{
			// Floating window pos/size is authoritative
			KGGuiWindow* single_window = node->Windows[0];
			node->Pos = single_window->Pos;
			node->Size = single_window->SizeFull;
			node->AuthorityForPos = node->AuthorityForSize = node->AuthorityForViewport = KGGuiDataAuthority_Window;

			// Transfer focus immediately so when we revert to a regular window it is immediately selected
			if (node->HostWindow && g.NavWindow == node->HostWindow)
				Karma::KarmaGuiInternal::FocusWindow(single_window);
			if (node->HostWindow)
			{
				single_window->Viewport = node->HostWindow->Viewport;
				single_window->ViewportId = node->HostWindow->ViewportId;
				if (node->HostWindow->ViewportOwned)
				{
					single_window->Viewport->Window = single_window;
					single_window->ViewportOwned = true;
				}
			}
		}

		Karma::DockNodeHideHostWindow(node);
		node->State = KGGuiDockNodeState_HostWindowHiddenBecauseSingleWindow;
		node->WantCloseAll = false;
		node->WantCloseTabId = 0;
		node->HasCloseButton = node->HasWindowMenuButton = false;
		node->LastFrameActive = g.FrameCount;

		if (node->WantMouseMove && node->Windows.Size == 1)
			Karma::DockNodeStartMouseMovingWindow(node, node->Windows[0]);
		return;
	}

	// In some circumstance we will defer creating the host window (so everything will be kept hidden),
	// while the expected visible window is resizing itself.
	// This is important for first-time (no ini settings restored) single window when io.ConfigDockingAlwaysTabBar is enabled,
	// otherwise the node ends up using the minimum window size. Effectively those windows will take an extra frame to show up:
	//   N+0: Begin(): window created (with no known size), node is created
	//   N+1: DockNodeUpdate(): node skip creating host window / Begin(): window size applied, not visible
	//   N+2: DockNodeUpdate(): node can create host window / Begin(): window becomes visible
	// We could remove this frame if we could reliably calculate the expected window size during node update, before the Begin() code.
	// It would require a generalization of CalcWindowExpectedSize(), probably extracting code away from Begin().
	// In reality it isn't very important as user quickly ends up with size data in .ini file.
	if (node->IsVisible && node->HostWindow == NULL && node->IsFloatingNode() && node->IsLeafNode())
	{
		KR_CORE_ASSERT(node->Windows.Size > 0, "");
		KGGuiWindow* ref_window = NULL;
		if (node->SelectedTabId != 0) // Note that we prune single-window-node settings on .ini loading, so this is generally 0 for them!
			ref_window = Karma::DockNodeFindWindowByID(node, node->SelectedTabId);
		if (ref_window == NULL)
			ref_window = node->Windows[0];
		if (ref_window->AutoFitFramesX > 0 || ref_window->AutoFitFramesY > 0)
		{
			node->State = KGGuiDockNodeState_HostWindowHiddenBecauseWindowsAreResizing;
			return;
		}
	}

	const KarmaGuiDockNodeFlags node_flags = node->MergedFlags;

	// Decide if the node will have a close button and a window menu button
	node->HasWindowMenuButton = (node->Windows.Size > 0) && (node_flags & KGGuiDockNodeFlags_NoWindowMenuButton) == 0;
	node->HasCloseButton = false;
	for (int window_n = 0; window_n < node->Windows.Size; window_n++)
	{
		// FIXME-DOCK: Setting DockIsActive here means that for single active window in a leaf node, DockIsActive will be cleared until the next Begin() call.
		KGGuiWindow* window = node->Windows[window_n];
		node->HasCloseButton |= window->HasCloseButton;
		window->DockIsActive = (node->Windows.Size > 1);
	}
	if (node_flags & KGGuiDockNodeFlags_NoCloseButton)
		node->HasCloseButton = false;

	// Bind or create host window
	KGGuiWindow* host_window = NULL;
	bool beginned_into_host_window = false;
	if (node->IsDockSpace())
	{
		// [Explicit root dockspace node]
		KR_CORE_ASSERT(node->HostWindow, "");
		host_window = node->HostWindow;
	}
	else
	{
		// [Automatic root or child nodes]
		if (node->IsRootNode() && node->IsVisible)
		{
			KGGuiWindow* ref_window = (node->Windows.Size > 0) ? node->Windows[0] : NULL;

			// Sync Pos
			if (node->AuthorityForPos == KGGuiDataAuthority_Window && ref_window)
				Karma::KarmaGui::SetNextWindowPos(ref_window->Pos);
			else if (node->AuthorityForPos == KGGuiDataAuthority_DockNode)
				Karma::KarmaGui::SetNextWindowPos(node->Pos);

			// Sync Size
			if (node->AuthorityForSize == KGGuiDataAuthority_Window && ref_window)
				Karma::KarmaGui::SetNextWindowSize(ref_window->SizeFull);
			else if (node->AuthorityForSize == KGGuiDataAuthority_DockNode)
				Karma::KarmaGui::SetNextWindowSize(node->Size);

			// Sync Collapsed
			if (node->AuthorityForSize == KGGuiDataAuthority_Window && ref_window)
				Karma::KarmaGui::SetNextWindowCollapsed(ref_window->Collapsed);

			// Sync Viewport
			if (node->AuthorityForViewport == KGGuiDataAuthority_Window && ref_window)
				Karma::KarmaGui::SetNextWindowViewport(ref_window->ViewportId);

			Karma::KarmaGui::SetNextWindowClass(&node->WindowClass);

			// Begin into the host window
			char window_label[20];
			Karma::DockNodeGetHostWindowTitle(node, window_label, KG_ARRAYSIZE(window_label));
			KarmaGuiWindowFlags window_flags = KGGuiWindowFlags_NoScrollbar | KGGuiWindowFlags_NoScrollWithMouse | KGGuiWindowFlags_DockNodeHost;
			window_flags |= KGGuiWindowFlags_NoFocusOnAppearing;
			window_flags |= KGGuiWindowFlags_NoSavedSettings | KGGuiWindowFlags_NoNavFocus | KGGuiWindowFlags_NoCollapse;
			window_flags |= KGGuiWindowFlags_NoTitleBar;

			Karma::KarmaGui::SetNextWindowBgAlpha(0.0f); // Don't set KGGuiWindowFlags_NoBackground because it disables borders
			Karma::KarmaGui::PushStyleVar(KGGuiStyleVar_WindowPadding, KGVec2(0, 0));
			Karma::KarmaGui::Begin(window_label, NULL, window_flags);
			Karma::KarmaGui::PopStyleVar();
			beginned_into_host_window = true;

			host_window = g.CurrentWindow;
			DockNodeSetupHostWindow(node, host_window);
			host_window->DC.CursorPos = host_window->Pos;
			node->Pos = host_window->Pos;
			node->Size = host_window->Size;

			// We set KGGuiWindowFlags_NoFocusOnAppearing because we don't want the host window to take full focus (e.g. steal NavWindow)
			// But we still it bring it to the front of display. There's no way to choose this precise behavior via window flags.
			// One simple case to ponder if: window A has a toggle to create windows B/C/D. Dock B/C/D together, clear the toggle and enable it again.
			// When reappearing B/C/D will request focus and be moved to the top of the display pile, but they are not linked to the dock host window
			// during the frame they appear. The dock host window would keep its old display order, and the sorting in EndFrame would move B/C/D back
			// after the dock host window, losing their top-most status.
			if (node->HostWindow->Appearing)
				Karma::KarmaGuiInternal::BringWindowToDisplayFront(node->HostWindow);

			node->AuthorityForPos = node->AuthorityForSize = node->AuthorityForViewport = KGGuiDataAuthority_Auto;
		}
		else if (node->ParentNode)
		{
			node->HostWindow = host_window = node->ParentNode->HostWindow;
			node->AuthorityForPos = node->AuthorityForSize = node->AuthorityForViewport = KGGuiDataAuthority_Auto;
		}
		if (node->WantMouseMove && node->HostWindow)
			Karma::DockNodeStartMouseMovingWindow(node, node->HostWindow);
	}

	// Update focused node (the one whose title bar is highlight) within a node tree
	if (node->IsSplitNode())
	{
		KR_CORE_ASSERT(node->TabBar == NULL, "");
	}
	if (node->IsRootNode())
		if (KGGuiWindow* p_window = g.NavWindow ? g.NavWindow->RootWindow : NULL)
			while (p_window != NULL && p_window->DockNode != NULL)
			{
				KGGuiDockNode* p_node = Karma::KarmaGuiInternal::DockNodeGetRootNode(p_window->DockNode);
				if (p_node == node)
				{
					node->LastFocusedNodeId = p_window->DockNode->ID; // Note: not using root node ID!
					break;
				}
				p_window = p_node->HostWindow ? p_node->HostWindow->RootWindow : NULL;
			}

	// Register a hit-test hole in the window unless we are currently dragging a window that is compatible with our dockspace
	KGGuiDockNode* central_node = node->CentralNode;
	const bool central_node_hole = node->IsRootNode() && host_window && (node_flags & KGGuiDockNodeFlags_PassthruCentralNode) != 0 && central_node != NULL && central_node->IsEmpty();
	bool central_node_hole_register_hit_test_hole = central_node_hole;
	if (central_node_hole)
		if (const KarmaGuiPayload* payload = Karma::KarmaGui::GetDragDropPayload())
			if (payload->IsDataType(KARMAGUI_PAYLOAD_TYPE_WINDOW) && Karma::DockNodeIsDropAllowed(host_window, *(KGGuiWindow**)payload->Data))
				central_node_hole_register_hit_test_hole = false;
	if (central_node_hole_register_hit_test_hole)
	{
		// We add a little padding to match the "resize from edges" behavior and allow grabbing the splitter easily.
		// (But we only add it if there's something else on the other side of the hole, otherwise for e.g. fullscreen
		// covering passthru node we'd have a gap on the edge not covered by the hole)
		KR_CORE_ASSERT(node->IsDockSpace(), ""); // We cannot pass this flag without the DockSpace() api. Testing this because we also setup the hole in host_window->ParentNode
		KGGuiDockNode* root_node = Karma::KarmaGuiInternal::DockNodeGetRootNode(central_node);
		KGRect root_rect(root_node->Pos, root_node->Pos + root_node->Size);
		KGRect hole_rect(central_node->Pos, central_node->Pos + central_node->Size);
		if (hole_rect.Min.x > root_rect.Min.x) { hole_rect.Min.x += WINDOWS_HOVER_PADDING; }
		if (hole_rect.Max.x < root_rect.Max.x) { hole_rect.Max.x -= WINDOWS_HOVER_PADDING; }
		if (hole_rect.Min.y > root_rect.Min.y) { hole_rect.Min.y += WINDOWS_HOVER_PADDING; }
		if (hole_rect.Max.y < root_rect.Max.y) { hole_rect.Max.y -= WINDOWS_HOVER_PADDING; }
		//GetForegroundDrawList()->AddRect(hole_rect.Min, hole_rect.Max, KG_COL32(255, 0, 0, 255));
		if (central_node_hole && !hole_rect.IsInverted())
		{
			Karma::KarmaGuiInternal::SetWindowHitTestHole(host_window, hole_rect.Min, hole_rect.Max - hole_rect.Min);
			if (host_window->ParentWindow)
				Karma::KarmaGuiInternal::SetWindowHitTestHole(host_window->ParentWindow, hole_rect.Min, hole_rect.Max - hole_rect.Min);
		}
	}

	// Update position/size, process and draw resizing splitters
	if (node->IsRootNode() && host_window)
	{
		Karma::DockNodeTreeUpdatePosSize(node, host_window->Pos, host_window->Size);
		Karma::DockNodeTreeUpdateSplitter(node);
	}

	// Draw empty node background (currently can only be the Central Node)
	if (host_window && node->IsEmpty() && node->IsVisible)
	{
		host_window->DrawList->ChannelsSetCurrent(DOCKING_HOST_DRAW_CHANNEL_BG);
		node->LastBgColor = (node_flags & KGGuiDockNodeFlags_PassthruCentralNode) ? 0 : Karma::KarmaGui::GetColorU32(KGGuiCol_DockingEmptyBg);
		if (node->LastBgColor != 0)
			host_window->DrawList->AddRectFilled(node->Pos, node->Pos + node->Size, node->LastBgColor);
		node->IsBgDrawnThisFrame = true;
	}

	// Draw whole dockspace background if KGGuiDockNodeFlags_PassthruCentralNode if set.
	// We need to draw a background at the root level if requested by KGGuiDockNodeFlags_PassthruCentralNode, but we will only know the correct pos/size
	// _after_ processing the resizing splitters. So we are using the DrawList channel splitting facility to submit drawing primitives out of order!
	const bool render_dockspace_bg = node->IsRootNode() && host_window && (node_flags & KGGuiDockNodeFlags_PassthruCentralNode) != 0;
	if (render_dockspace_bg && node->IsVisible)
	{
		host_window->DrawList->ChannelsSetCurrent(DOCKING_HOST_DRAW_CHANNEL_BG);
		if (central_node_hole)
			Karma::KarmaGuiInternal::RenderRectFilledWithHole(host_window->DrawList, node->Rect(), central_node->Rect(), Karma::KarmaGui::GetColorU32(KGGuiCol_WindowBg), 0.0f);
		else
			host_window->DrawList->AddRectFilled(node->Pos, node->Pos + node->Size, Karma::KarmaGui::GetColorU32(KGGuiCol_WindowBg), 0.0f);
	}

	// Draw and populate Tab Bar
	if (host_window)
		host_window->DrawList->ChannelsSetCurrent(DOCKING_HOST_DRAW_CHANNEL_FG);
	if (host_window && node->Windows.Size > 0)
	{
		Karma::DockNodeUpdateTabBar(node, host_window);
	}
	else
	{
		node->WantCloseAll = false;
		node->WantCloseTabId = 0;
		node->IsFocused = false;
	}
	if (node->TabBar && node->TabBar->SelectedTabId)
		node->SelectedTabId = node->TabBar->SelectedTabId;
	else if (node->Windows.Size > 0)
		node->SelectedTabId = node->Windows[0]->TabId;

	// Draw payload drop target
	if (host_window && node->IsVisible)
		if (node->IsRootNode() && (g.MovingWindow == NULL || g.MovingWindow->RootWindowDockTree != host_window))
			Karma::KarmaGuiInternal::BeginDockableDragDropTarget(host_window);

	// We update this after DockNodeUpdateTabBar()
	node->LastFrameActive = g.FrameCount;

	// Recurse into children
	// FIXME-DOCK FIXME-OPT: Should not need to recurse into children
	if (host_window)
	{
		if (node->ChildNodes[0])
			DockNodeUpdate(node->ChildNodes[0]);
		if (node->ChildNodes[1])
			DockNodeUpdate(node->ChildNodes[1]);

		// Render outer borders last (after the tab bar)
		if (node->IsRootNode())
			Karma::KarmaGuiInternal::RenderWindowOuterBorders(host_window);
	}

	// End host window
	if (beginned_into_host_window) //-V1020
		Karma::KarmaGui::End();
}

// Compare TabItem nodes given the last known DockOrder (will persist in .ini file as hint), used to sort tabs when multiple tabs are added on the same frame.
int TabItemComparerByDockOrder(const void* lhs, const void* rhs)
{
	KGGuiWindow* a = ((const KGGuiTabItem*)lhs)->Window;
	KGGuiWindow* b = ((const KGGuiTabItem*)rhs)->Window;
	if (int d = ((a->DockOrder == -1) ? INT_MAX : a->DockOrder) - ((b->DockOrder == -1) ? INT_MAX : b->DockOrder))
		return d;
	return (a->BeginOrderWithinContext - b->BeginOrderWithinContext);
}

KGGuiID Karma::DockNodeUpdateWindowMenu(KGGuiDockNode* node, KGGuiTabBar* tab_bar)
{
	// Try to position the menu so it is more likely to stays within the same viewport
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiID ret_tab_id = 0;
	if (g.Style.WindowMenuButtonPosition == KGGuiDir_Left)
		Karma::KarmaGui::SetNextWindowPos(KGVec2(node->Pos.x, node->Pos.y + Karma::KarmaGui::GetFrameHeight()), KGGuiCond_Always, KGVec2(0.0f, 0.0f));
	else
		Karma::KarmaGui::SetNextWindowPos(KGVec2(node->Pos.x + node->Size.x, node->Pos.y + Karma::KarmaGui::GetFrameHeight()), KGGuiCond_Always, KGVec2(1.0f, 0.0f));
	if (Karma::KarmaGui::BeginPopup("#WindowMenu"))
	{
		node->IsFocused = true;
		if (tab_bar->Tabs.Size == 1)
		{
			if (Karma::KarmaGui::MenuItem(Karma::KarmaGuiInternal::LocalizeGetMsg(ImGuiLocKey_DockingHideTabBar), NULL, node->IsHiddenTabBar()))
				node->WantHiddenTabBarToggle = true;
		}
		else
		{
			for (int tab_n = 0; tab_n < tab_bar->Tabs.Size; tab_n++)
			{
				KGGuiTabItem* tab = &tab_bar->Tabs[tab_n];
				if (tab->Flags & KGGuiTabItemFlags_Button)
					continue;
				if (Karma::KarmaGui::Selectable(tab_bar->GetTabName(tab), tab->ID == tab_bar->SelectedTabId))
					ret_tab_id = tab->ID;
				Karma::KarmaGui::SameLine();
				Karma::KarmaGui::Text("   ");
			}
		}
		Karma::KarmaGui::EndPopup();
	}
	return ret_tab_id;
}

const char* Karma::KarmaGuiInternal::LocalizeGetMsg(KGGuiLocKey key)
{
	KarmaGuiContext& g = *GKarmaGui;
	const char* msg = g.LocalizationTable[key]; return msg ? msg : "*Missing Text*";
}

// User helper to append/amend into a dock node tab bar. Most commonly used to add e.g. a "+" button.
bool Karma::KarmaGuiInternal::DockNodeBeginAmendTabBar(KGGuiDockNode* node)
{
	if (node->TabBar == NULL || node->HostWindow == NULL)
		return false;
	if (node->MergedFlags & KGGuiDockNodeFlags_KeepAliveOnly)
		return false;
	KarmaGui::Begin(node->HostWindow->Name);
	PushOverrideID(node->ID);
	bool ret = BeginTabBarEx(node->TabBar, node->TabBar->BarRect, node->TabBar->Flags, node);
	KG_UNUSED(ret);
	KR_CORE_ASSERT(ret, "");
	return true;
}

void Karma::KarmaGuiInternal::DockNodeEndAmendTabBar()
{
	KarmaGui::EndTabBar();
	KarmaGui::PopID();
	KarmaGui::End();
}

static bool IsDockNodeTitleBarHighlighted(KGGuiDockNode* node, KGGuiDockNode* root_node)
{
	// CTRL+Tab highlight (only highlighting leaf node, not whole hierarchy)
	KarmaGuiContext& g = *Karma::GKarmaGui;
	if (g.NavWindowingTarget)
		return (g.NavWindowingTarget->DockNode == node);

	// FIXME-DOCKING: May want alternative to treat central node void differently? e.g. if (g.NavWindow == host_window)
	if (g.NavWindow && root_node->LastFocusedNodeId == node->ID)
	{
		// FIXME: This could all be backed in RootWindowForTitleBarHighlight? Probably need to reorganize for both dock nodes + other RootWindowForTitleBarHighlight users (not-node)
		KGGuiWindow* parent_window = g.NavWindow->RootWindow;
		while (parent_window->Flags & KGGuiWindowFlags_ChildMenu)
			parent_window = parent_window->ParentWindow->RootWindow;
		KGGuiDockNode* start_parent_node = parent_window->DockNodeAsHost ? parent_window->DockNodeAsHost : parent_window->DockNode;
		for (KGGuiDockNode* parent_node = start_parent_node; parent_node != NULL; parent_node = parent_node->HostWindow ? parent_node->HostWindow->RootWindow->DockNode : NULL)
			if ((parent_node = Karma::KarmaGuiInternal::DockNodeGetRootNode(parent_node)) == root_node)
				return true;
	}
	return false;
}

// Submit the tab bar corresponding to a dock node and various housekeeping details.
void Karma::DockNodeUpdateTabBar(KGGuiDockNode* node, KGGuiWindow* host_window)
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiStyle& style = g.Style;

	const bool node_was_active = (node->LastFrameActive + 1 == g.FrameCount);
	const bool closed_all = node->WantCloseAll && node_was_active;
	const KGGuiID closed_one = node->WantCloseTabId && node_was_active;
	node->WantCloseAll = false;
	node->WantCloseTabId = 0;

	// Decide if we should use a focused title bar color
	bool is_focused = false;
	KGGuiDockNode* root_node = Karma::KarmaGuiInternal::DockNodeGetRootNode(node);
	if (IsDockNodeTitleBarHighlighted(node, root_node))
		is_focused = true;

	// Hidden tab bar will show a triangle on the upper-left (in Begin)
	if (node->IsHiddenTabBar() || node->IsNoTabBar())
	{
		node->VisibleWindow = (node->Windows.Size > 0) ? node->Windows[0] : NULL;
		node->IsFocused = is_focused;
		if (is_focused)
			node->LastFrameFocused = g.FrameCount;
		if (node->VisibleWindow)
		{
			// Notify root of visible window (used to display title in OS task bar)
			if (is_focused || root_node->VisibleWindow == NULL)
				root_node->VisibleWindow = node->VisibleWindow;
			if (node->TabBar)
				node->TabBar->VisibleTabId = node->VisibleWindow->TabId;
		}
		return;
	}

	// Move ourselves to the Menu layer (so we can be accessed by tapping Alt) + undo SkipItems flag in order to draw over the title bar even if the window is collapsed
	bool backup_skip_item = host_window->SkipItems;
	if (!node->IsDockSpace())
	{
		host_window->SkipItems = false;
		host_window->DC.NavLayerCurrent = KGGuiNavLayer_Menu;
	}

	// Use PushOverrideID() instead of PushID() to use the node id _without_ the host window ID.
	// This is to facilitate computing those ID from the outside, and will affect more or less only the ID of the collapse button, popup and tabs,
	// as docked windows themselves will override the stack with their own root ID.
	Karma::KarmaGuiInternal::PushOverrideID(node->ID);
	KGGuiTabBar* tab_bar = node->TabBar;
	bool tab_bar_is_recreated = (tab_bar == NULL); // Tab bar are automatically destroyed when a node gets hidden
	if (tab_bar == NULL)
	{
		Karma::DockNodeAddTabBar(node);
		tab_bar = node->TabBar;
	}

	KGGuiID focus_tab_id = 0;
	node->IsFocused = is_focused;

	const KarmaGuiDockNodeFlags node_flags = node->MergedFlags;
	const bool has_window_menu_button = (node_flags & KGGuiDockNodeFlags_NoWindowMenuButton) == 0 && (style.WindowMenuButtonPosition != KGGuiDir_None);

	// In a dock node, the Collapse Button turns into the Window Menu button.
	// FIXME-DOCK FIXME-OPT: Could we recycle popups id across multiple dock nodes?
	if (has_window_menu_button && Karma::KarmaGui::IsPopupOpen("#WindowMenu"))
	{
		if (KGGuiID tab_id = Karma::DockNodeUpdateWindowMenu(node, tab_bar))
			focus_tab_id = tab_bar->NextSelectedTabId = tab_id;
		is_focused |= node->IsFocused;
	}

	// Layout
	KGRect title_bar_rect, tab_bar_rect;
	KGVec2 window_menu_button_pos;
	KGVec2 close_button_pos;
	Karma::DockNodeCalcTabBarLayout(node, &title_bar_rect, &tab_bar_rect, &window_menu_button_pos, &close_button_pos);

	// Submit new tabs, they will be added as Unsorted and sorted below based on relative DockOrder value.
	const int tabs_count_old = tab_bar->Tabs.Size;
	for (int window_n = 0; window_n < node->Windows.Size; window_n++)
	{
		KGGuiWindow* window = node->Windows[window_n];
		if (Karma::KarmaGuiInternal::TabBarFindTabByID(tab_bar, window->TabId) == NULL)
			Karma::KarmaGuiInternal::TabBarAddTab(tab_bar, KGGuiTabItemFlags_Unsorted, window);
	}

	// Title bar
	if (is_focused)
		node->LastFrameFocused = g.FrameCount;
	KGU32 title_bar_col = Karma::KarmaGui::GetColorU32(host_window->Collapsed ? KGGuiCol_TitleBgCollapsed : is_focused ? KGGuiCol_TitleBgActive : KGGuiCol_TitleBg);
	KGDrawFlags rounding_flags = Karma::KarmaGuiInternal::CalcRoundingFlagsForRectInRect(title_bar_rect, host_window->Rect(), DOCKING_SPLITTER_SIZE);
	host_window->DrawList->AddRectFilled(title_bar_rect.Min, title_bar_rect.Max, title_bar_col, host_window->WindowRounding, rounding_flags);

	// Docking/Collapse button
	if (has_window_menu_button)
	{
		if (Karma::KarmaGuiInternal::CollapseButton(host_window->GetID("#COLLAPSE"), window_menu_button_pos, node)) // == DockNodeGetWindowMenuButtonId(node)
			Karma::KarmaGui::OpenPopup("#WindowMenu");
		if (Karma::KarmaGui::IsItemActive())
			focus_tab_id = tab_bar->SelectedTabId;
	}

	// If multiple tabs are appearing on the same frame, sort them based on their persistent DockOrder value
	int tabs_unsorted_start = tab_bar->Tabs.Size;
	for (int tab_n = tab_bar->Tabs.Size - 1; tab_n >= 0 && (tab_bar->Tabs[tab_n].Flags & KGGuiTabItemFlags_Unsorted); tab_n--)
	{
		// FIXME-DOCK: Consider only clearing the flag after the tab has been alive for a few consecutive frames, allowing late comers to not break sorting?
		tab_bar->Tabs[tab_n].Flags &= ~KGGuiTabItemFlags_Unsorted;
		tabs_unsorted_start = tab_n;
	}
	if (tab_bar->Tabs.Size > tabs_unsorted_start)
	{
		KR_CORE_INFO("[docking] In node {0}: %d new appearing tabs:{1}", node->ID, tab_bar->Tabs.Size - tabs_unsorted_start, (tab_bar->Tabs.Size > tabs_unsorted_start + 1) ? " (will sort)" : "");
		for (int tab_n = tabs_unsorted_start; tab_n < tab_bar->Tabs.Size; tab_n++)
		{
			KR_CORE_INFO("[docking] - Tab '{0}' Order {1}", tab_bar->Tabs[tab_n].Window->Name, tab_bar->Tabs[tab_n].Window->DockOrder);
		}
		if (tab_bar->Tabs.Size > tabs_unsorted_start + 1)
			KGQsort(tab_bar->Tabs.Data + tabs_unsorted_start, tab_bar->Tabs.Size - tabs_unsorted_start, sizeof(KGGuiTabItem), TabItemComparerByDockOrder);
	}

	// Apply NavWindow focus back to the tab bar
	if (g.NavWindow && g.NavWindow->RootWindow->DockNode == node)
		tab_bar->SelectedTabId = g.NavWindow->RootWindow->TabId;

	// Selected newly added tabs, or persistent tab ID if the tab bar was just recreated
	if (tab_bar_is_recreated && Karma::KarmaGuiInternal::TabBarFindTabByID(tab_bar, node->SelectedTabId) != NULL)
		tab_bar->SelectedTabId = tab_bar->NextSelectedTabId = node->SelectedTabId;
	else if (tab_bar->Tabs.Size > tabs_count_old)
		tab_bar->SelectedTabId = tab_bar->NextSelectedTabId = tab_bar->Tabs.back().Window->TabId;

	// Begin tab bar
	KarmaGuiTabBarFlags tab_bar_flags = KGGuiTabBarFlags_Reorderable | KGGuiTabBarFlags_AutoSelectNewTabs; // | KGGuiTabBarFlags_NoTabListScrollingButtons);
	tab_bar_flags |= KGGuiTabBarFlags_SaveSettings | KGGuiTabBarFlags_DockNode;
	if (!host_window->Collapsed && is_focused)
		tab_bar_flags |= KGGuiTabBarFlags_IsFocused;
	Karma::KarmaGuiInternal::BeginTabBarEx(tab_bar, tab_bar_rect, tab_bar_flags, node);
	//host_window->DrawList->AddRect(tab_bar_rect.Min, tab_bar_rect.Max, KG_COL32(255,0,255,255));

	// Backup style colors
	KGVec4 backup_style_cols[KGGuiWindowDockStyleCol_COUNT];
	for (int color_n = 0; color_n < KGGuiWindowDockStyleCol_COUNT; color_n++)
		backup_style_cols[color_n] = g.Style.Colors[GWindowDockStyleColors[color_n]];

	// Submit actual tabs
	node->VisibleWindow = NULL;
	for (int window_n = 0; window_n < node->Windows.Size; window_n++)
	{
		KGGuiWindow* window = node->Windows[window_n];
		if ((closed_all || closed_one == window->TabId) && window->HasCloseButton && !(window->Flags & KGGuiWindowFlags_UnsavedDocument))
			continue;
		if (window->LastFrameActive + 1 >= g.FrameCount || !node_was_active)
		{
			KarmaGuiTabItemFlags tab_item_flags = 0;
			tab_item_flags |= window->WindowClass.TabItemFlagsOverrideSet;
			if (window->Flags & KGGuiWindowFlags_UnsavedDocument)
				tab_item_flags |= KGGuiTabItemFlags_UnsavedDocument;
			if (tab_bar->Flags & KGGuiTabBarFlags_NoCloseWithMiddleMouseButton)
				tab_item_flags |= KGGuiTabItemFlags_NoCloseWithMiddleMouseButton;

			// Apply stored style overrides for the window
			for (int color_n = 0; color_n < KGGuiWindowDockStyleCol_COUNT; color_n++)
				g.Style.Colors[GWindowDockStyleColors[color_n]] = Karma::KarmaGui::ColorConvertU32ToFloat4(window->DockStyle.Colors[color_n]);

			// Note that TabItemEx() calls TabBarCalcTabID() so our tab item ID will ignore the current ID stack (rightly so)
			bool tab_open = true;
			Karma::KarmaGuiInternal::TabItemEx(tab_bar, window->Name, window->HasCloseButton ? &tab_open : NULL, tab_item_flags, window);
			if (!tab_open)
				node->WantCloseTabId = window->TabId;
			if (tab_bar->VisibleTabId == window->TabId)
				node->VisibleWindow = window;

			// Store last item data so it can be queried with IsItemXXX functions after the user Begin() call
			window->DockTabItemStatusFlags = g.LastItemData.StatusFlags;
			window->DockTabItemRect = g.LastItemData.Rect;

			// Update navigation ID on menu layer
			if (g.NavWindow && g.NavWindow->RootWindow == window && (window->DC.NavLayersActiveMask & (1 << KGGuiNavLayer_Menu)) == 0)
				host_window->NavLastIds[1] = window->TabId;
		}
	}

	// Restore style colors
	for (int color_n = 0; color_n < KGGuiWindowDockStyleCol_COUNT; color_n++)
		g.Style.Colors[GWindowDockStyleColors[color_n]] = backup_style_cols[color_n];

	// Notify root of visible window (used to display title in OS task bar)
	if (node->VisibleWindow)
		if (is_focused || root_node->VisibleWindow == NULL)
			root_node->VisibleWindow = node->VisibleWindow;

	// Close button (after VisibleWindow was updated)
	// Note that VisibleWindow may have been overrided by CTRL+Tabbing, so VisibleWindow->TabId may be != from tab_bar->SelectedTabId
	const bool close_button_is_enabled = node->HasCloseButton && node->VisibleWindow && node->VisibleWindow->HasCloseButton;
	const bool close_button_is_visible = node->HasCloseButton;
	//const bool close_button_is_visible = close_button_is_enabled; // Most people would expect this behavior of not even showing the button (leaving a hole since we can't claim that space as other windows in the tba bar have one)
	if (close_button_is_visible)
	{
		if (!close_button_is_enabled)
		{
			Karma::KarmaGuiInternal::PushItemFlag(KGGuiItemFlags_Disabled, true);
			Karma::KarmaGui::PushStyleColor(KGGuiCol_Text, style.Colors[KGGuiCol_Text] * KGVec4(1.0f, 1.0f, 1.0f, 0.4f));
		}
		if (Karma::KarmaGuiInternal::CloseButton(host_window->GetID("#CLOSE"), close_button_pos))
		{
			node->WantCloseAll = true;
			for (int n = 0; n < tab_bar->Tabs.Size; n++)
				Karma::KarmaGuiInternal::TabBarCloseTab(tab_bar, &tab_bar->Tabs[n]);
		}
		//if (IsItemActive())
		//    focus_tab_id = tab_bar->SelectedTabId;
		if (!close_button_is_enabled)
		{
			Karma::KarmaGui::PopStyleColor();
			Karma::KarmaGuiInternal::PopItemFlag();
		}
	}

	// When clicking on the title bar outside of tabs, we still focus the selected tab for that node
	// FIXME: TabItem use AllowItemOverlap so we manually perform a more specific test for now (hovered || held)
	KGGuiID title_bar_id = host_window->GetID("#TITLEBAR");
	if (g.HoveredId == 0 || g.HoveredId == title_bar_id || g.ActiveId == title_bar_id)
	{
		bool held;
		Karma::KarmaGuiInternal::ButtonBehavior(title_bar_rect, title_bar_id, NULL, &held, KGGuiButtonFlags_AllowItemOverlap);
		if (g.HoveredId == title_bar_id)
		{
			// KGGuiButtonFlags_AllowItemOverlap + SetItemAllowOverlap() required for appending into dock node tab bar,
			// otherwise dragging window will steal HoveredId and amended tabs cannot get them.
			g.LastItemData.ID = title_bar_id;
			Karma::KarmaGui::SetItemAllowOverlap();
		}
		if (held)
		{
			if (Karma::KarmaGui::IsMouseClicked(0))
				focus_tab_id = tab_bar->SelectedTabId;

			// Forward moving request to selected window
			if (KGGuiTabItem* tab = Karma::KarmaGuiInternal::TabBarFindTabByID(tab_bar, tab_bar->SelectedTabId))
				Karma::KarmaGuiInternal::StartMouseMovingWindowOrNode(tab->Window ? tab->Window : node->HostWindow, node, false);
		}
	}

	// Forward focus from host node to selected window
	//if (is_focused && g.NavWindow == host_window && !g.NavWindowingTarget)
	//    focus_tab_id = tab_bar->SelectedTabId;

	// When clicked on a tab we requested focus to the docked child
	// This overrides the value set by "forward focus from host node to selected window".
	if (tab_bar->NextSelectedTabId)
		focus_tab_id = tab_bar->NextSelectedTabId;

	// Apply navigation focus
	if (focus_tab_id != 0)
		if (KGGuiTabItem* tab = Karma::KarmaGuiInternal::TabBarFindTabByID(tab_bar, focus_tab_id))
			if (tab->Window)
			{
				Karma::KarmaGuiInternal::FocusWindow(tab->Window);
				Karma::KarmaGuiInternal::NavInitWindow(tab->Window, false);
			}

	Karma::KarmaGui::EndTabBar();
	Karma::KarmaGui::PopID();

	// Restore SkipItems flag
	if (!node->IsDockSpace())
	{
		host_window->DC.NavLayerCurrent = KGGuiNavLayer_Main;
		host_window->SkipItems = backup_skip_item;
	}
}

void Karma::DockNodeAddTabBar(KGGuiDockNode* node)
{
	KR_CORE_ASSERT(node->TabBar == NULL, "");
	node->TabBar = KG_NEW(KGGuiTabBar);
}

void Karma::DockNodeRemoveTabBar(KGGuiDockNode* node)
{
	if (node->TabBar == NULL)
		return;
	KG_DELETE(node->TabBar);
	node->TabBar = NULL;
}

static bool DockNodeIsDropAllowedOne(KGGuiWindow* payload, KGGuiWindow* host_window)
{
	if (host_window->DockNodeAsHost && host_window->DockNodeAsHost->IsDockSpace() && payload->BeginOrderWithinContext < host_window->BeginOrderWithinContext)
		return false;

	KarmaGuiWindowClass* host_class = host_window->DockNodeAsHost ? &host_window->DockNodeAsHost->WindowClass : &host_window->WindowClass;
	KarmaGuiWindowClass* payload_class = &payload->WindowClass;
	if (host_class->ClassId != payload_class->ClassId)
	{
		if (host_class->ClassId != 0 && host_class->DockingAllowUnclassed && payload_class->ClassId == 0)
			return true;
		if (payload_class->ClassId != 0 && payload_class->DockingAllowUnclassed && host_class->ClassId == 0)
			return true;
		return false;
	}

	// Prevent docking any window created above a popup
	// Technically we should support it (e.g. in the case of a long-lived modal window that had fancy docking features),
	// by e.g. adding a 'if (!KarmaGui::IsWindowWithinBeginStackOf(host_window, popup_window))' test.
	// But it would requires more work on our end because the dock host windows is technically created in NewFrame()
	// and our ->ParentXXX and ->RootXXX pointers inside windows are currently mislading or lacking.
	KarmaGuiContext& g = *Karma::GKarmaGui;
	for (int i = g.OpenPopupStack.Size - 1; i >= 0; i--)
		if (KGGuiWindow* popup_window = g.OpenPopupStack[i].Window)
			if (Karma::KarmaGuiInternal::IsWindowWithinBeginStackOf(payload, popup_window))   // Payload is created from within a popup begin stack.
				return false;

	return true;
}

bool Karma::DockNodeIsDropAllowed(KGGuiWindow* host_window, KGGuiWindow* root_payload)
{
	if (root_payload->DockNodeAsHost && root_payload->DockNodeAsHost->IsSplitNode()) // FIXME-DOCK: Missing filtering
		return true;

	const int payload_count = root_payload->DockNodeAsHost ? root_payload->DockNodeAsHost->Windows.Size : 1;
	for (int payload_n = 0; payload_n < payload_count; payload_n++)
	{
		KGGuiWindow* payload = root_payload->DockNodeAsHost ? root_payload->DockNodeAsHost->Windows[payload_n] : root_payload;
		if (DockNodeIsDropAllowedOne(payload, host_window))
			return true;
	}
	return false;
}

// window menu button == collapse button when not in a dock node.
// FIXME: This is similar to RenderWindowTitleBarContents(), may want to share code.
void Karma::DockNodeCalcTabBarLayout(const KGGuiDockNode* node, KGRect* out_title_rect, KGRect* out_tab_bar_rect, KGVec2* out_window_menu_button_pos, KGVec2* out_close_button_pos)
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiStyle& style = g.Style;

	KGRect r = KGRect(node->Pos.x, node->Pos.y, node->Pos.x + node->Size.x, node->Pos.y + g.FontSize + g.Style.FramePadding.y * 2.0f);
	if (out_title_rect) { *out_title_rect = r; }

	r.Min.x += style.WindowBorderSize;
	r.Max.x -= style.WindowBorderSize;

	float button_sz = g.FontSize;

	KGVec2 window_menu_button_pos = r.Min;
	r.Min.x += style.FramePadding.x;
	r.Max.x -= style.FramePadding.x;
	if (node->HasCloseButton)
	{
		r.Max.x -= button_sz;
		if (out_close_button_pos) *out_close_button_pos = KGVec2(r.Max.x - style.FramePadding.x, r.Min.y);
	}
	if (node->HasWindowMenuButton && style.WindowMenuButtonPosition == KGGuiDir_Left)
	{
		r.Min.x += button_sz + style.ItemInnerSpacing.x;
	}
	else if (node->HasWindowMenuButton && style.WindowMenuButtonPosition == KGGuiDir_Right)
	{
		r.Max.x -= button_sz + style.FramePadding.x;
		window_menu_button_pos = KGVec2(r.Max.x, r.Min.y);
	}
	if (out_tab_bar_rect) { *out_tab_bar_rect = r; }
	if (out_window_menu_button_pos) { *out_window_menu_button_pos = window_menu_button_pos; }
}

void Karma::DockNodeCalcSplitRects(KGVec2& pos_old, KGVec2& size_old, KGVec2& pos_new, KGVec2& size_new, KarmaGuiDir dir, KGVec2 size_new_desired)
{
	KarmaGuiContext& g = *GKarmaGui;
	const float dock_spacing = g.Style.ItemInnerSpacing.x;
	const KGGuiAxis axis = (dir == KGGuiDir_Left || dir == KGGuiDir_Right) ? KGGuiAxis_X : KGGuiAxis_Y;
	pos_new[axis ^ 1] = pos_old[axis ^ 1];
	size_new[axis ^ 1] = size_old[axis ^ 1];

	// Distribute size on given axis (with a desired size or equally)
	const float w_avail = size_old[axis] - dock_spacing;
	if (size_new_desired[axis] > 0.0f && size_new_desired[axis] <= w_avail * 0.5f)
	{
		size_new[axis] = size_new_desired[axis];
		size_old[axis] = KG_FLOOR(w_avail - size_new[axis]);
	}
	else
	{
		size_new[axis] = KG_FLOOR(w_avail * 0.5f);
		size_old[axis] = KG_FLOOR(w_avail - size_new[axis]);
	}

	// Position each node
	if (dir == KGGuiDir_Right || dir == KGGuiDir_Down)
	{
		pos_new[axis] = pos_old[axis] + size_old[axis] + dock_spacing;
	}
	else if (dir == KGGuiDir_Left || dir == KGGuiDir_Up)
	{
		pos_new[axis] = pos_old[axis];
		pos_old[axis] = pos_new[axis] + size_new[axis] + dock_spacing;
	}
}

// Retrieve the drop rectangles for a given direction or for the center + perform hit testing.
bool Karma::DockNodeCalcDropRectsAndTestMousePos(const KGRect& parent, KarmaGuiDir dir, KGRect& out_r, bool outer_docking, KGVec2* test_mouse_pos)
{
	KarmaGuiContext& g = *GKarmaGui;

	const float parent_smaller_axis = KGMin(parent.GetWidth(), parent.GetHeight());
	const float hs_for_central_nodes = KGMin(g.FontSize * 1.5f, KGMax(g.FontSize * 0.5f, parent_smaller_axis / 8.0f));
	float hs_w; // Half-size, longer axis
	float hs_h; // Half-size, smaller axis
	KGVec2 off; // Distance from edge or center
	if (outer_docking)
	{
		//hs_w = KGFloor(KGClamp(parent_smaller_axis - hs_for_central_nodes * 4.0f, g.FontSize * 0.5f, g.FontSize * 8.0f));
		//hs_h = KGFloor(hs_w * 0.15f);
		//off = KGVec2(KGFloor(parent.GetWidth() * 0.5f - GetFrameHeightWithSpacing() * 1.4f - hs_h), KGFloor(parent.GetHeight() * 0.5f - GetFrameHeightWithSpacing() * 1.4f - hs_h));
		hs_w = KGFloor(hs_for_central_nodes * 1.50f);
		hs_h = KGFloor(hs_for_central_nodes * 0.80f);
		off = KGVec2(KGFloor(parent.GetWidth() * 0.5f - hs_h), KGFloor(parent.GetHeight() * 0.5f - hs_h));
	}
	else
	{
		hs_w = KGFloor(hs_for_central_nodes);
		hs_h = KGFloor(hs_for_central_nodes * 0.90f);
		off = KGVec2(KGFloor(hs_w * 2.40f), KGFloor(hs_w * 2.40f));
	}

	KGVec2 c = KGFloor(parent.GetCenter());
	if (dir == KGGuiDir_None) { out_r = KGRect(c.x - hs_w, c.y - hs_w, c.x + hs_w, c.y + hs_w); }
	else if (dir == KGGuiDir_Up) { out_r = KGRect(c.x - hs_w, c.y - off.y - hs_h, c.x + hs_w, c.y - off.y + hs_h); }
	else if (dir == KGGuiDir_Down) { out_r = KGRect(c.x - hs_w, c.y + off.y - hs_h, c.x + hs_w, c.y + off.y + hs_h); }
	else if (dir == KGGuiDir_Left) { out_r = KGRect(c.x - off.x - hs_h, c.y - hs_w, c.x - off.x + hs_h, c.y + hs_w); }
	else if (dir == KGGuiDir_Right) { out_r = KGRect(c.x + off.x - hs_h, c.y - hs_w, c.x + off.x + hs_h, c.y + hs_w); }

	if (test_mouse_pos == NULL)
		return false;

	KGRect hit_r = out_r;
	if (!outer_docking)
	{
		// Custom hit testing for the 5-way selection, designed to reduce flickering when moving diagonally between sides
		hit_r.Expand(KGFloor(hs_w * 0.30f));
		KGVec2 mouse_delta = (*test_mouse_pos - c);
		float mouse_delta_len2 = KGLengthSqr(mouse_delta);
		float r_threshold_center = hs_w * 1.4f;
		float r_threshold_sides = hs_w * (1.4f + 1.2f);
		if (mouse_delta_len2 < r_threshold_center * r_threshold_center)
			return (dir == KGGuiDir_None);
		if (mouse_delta_len2 < r_threshold_sides * r_threshold_sides)
			return (dir == KGGetDirQuadrantFromDelta(mouse_delta.x, mouse_delta.y));
	}
	return hit_r.Contains(*test_mouse_pos);
}

// host_node may be NULL if the window doesn't have a DockNode already.
// FIXME-DOCK: This is misnamed since it's also doing the filtering.
void Karma::DockNodePreviewDockSetup(KGGuiWindow* host_window, KGGuiDockNode* host_node, KGGuiWindow* payload_window, KGGuiDockNode* payload_node, ImGuiDockPreviewData* data, bool is_explicit_target, bool is_outer_docking)
{
	KarmaGuiContext& g = *GKarmaGui;

	// There is an edge case when docking into a dockspace which only has inactive nodes.
	// In this case DockNodeTreeFindNodeByPos() will have selected a leaf node which is inactive.
	// Because the inactive leaf node doesn't have proper pos/size yet, we'll use the root node as reference.
	if (payload_node == NULL)
		payload_node = payload_window->DockNodeAsHost;
	KGGuiDockNode* ref_node_for_rect = (host_node && !host_node->IsVisible) ? Karma::KarmaGuiInternal::DockNodeGetRootNode(host_node) : host_node;
	if (ref_node_for_rect)
	{
		KR_CORE_ASSERT(ref_node_for_rect->IsVisible == true, "");
	}

	// Filter, figure out where we are allowed to dock
	KarmaGuiDockNodeFlags src_node_flags = payload_node ? payload_node->MergedFlags : payload_window->WindowClass.DockNodeFlagsOverrideSet;
	KarmaGuiDockNodeFlags dst_node_flags = host_node ? host_node->MergedFlags : host_window->WindowClass.DockNodeFlagsOverrideSet;
	data->IsCenterAvailable = true;
	if (is_outer_docking)
		data->IsCenterAvailable = false;
	else if (dst_node_flags & KGGuiDockNodeFlags_NoDocking)
		data->IsCenterAvailable = false;
	else if (host_node && (dst_node_flags & KGGuiDockNodeFlags_NoDockingInCentralNode) && host_node->IsCentralNode())
		data->IsCenterAvailable = false;
	else if ((!host_node || !host_node->IsEmpty()) && payload_node && payload_node->IsSplitNode() && (payload_node->OnlyNodeWithWindows == NULL)) // Is _visibly_ split?
		data->IsCenterAvailable = false;
	else if (dst_node_flags & KGGuiDockNodeFlags_NoDockingOverMe)
		data->IsCenterAvailable = false;
	else if ((src_node_flags & KGGuiDockNodeFlags_NoDockingOverOther) && (!host_node || !host_node->IsEmpty()))
		data->IsCenterAvailable = false;
	else if ((src_node_flags & KGGuiDockNodeFlags_NoDockingOverEmpty) && host_node && host_node->IsEmpty())
		data->IsCenterAvailable = false;

	data->IsSidesAvailable = true;
	if ((dst_node_flags & KGGuiDockNodeFlags_NoSplit) || g.IO.ConfigDockingNoSplit)
		data->IsSidesAvailable = false;
	else if (!is_outer_docking && host_node && host_node->ParentNode == NULL && host_node->IsCentralNode())
		data->IsSidesAvailable = false;
	else if ((dst_node_flags & KGGuiDockNodeFlags_NoDockingSplitMe) || (src_node_flags & KGGuiDockNodeFlags_NoDockingSplitOther))
		data->IsSidesAvailable = false;

	// Build a tentative future node (reuse same structure because it is practical. Shape will be readjusted when previewing a split)
	data->FutureNode.HasCloseButton = (host_node ? host_node->HasCloseButton : host_window->HasCloseButton) || (payload_window->HasCloseButton);
	data->FutureNode.HasWindowMenuButton = host_node ? true : ((host_window->Flags & KGGuiWindowFlags_NoCollapse) == 0);
	data->FutureNode.Pos = ref_node_for_rect ? ref_node_for_rect->Pos : host_window->Pos;
	data->FutureNode.Size = ref_node_for_rect ? ref_node_for_rect->Size : host_window->Size;

	// Calculate drop shapes geometry for allowed splitting directions
	KR_CORE_ASSERT(KGGuiDir_None == -1, "");
	data->SplitNode = host_node;
	data->SplitDir = KGGuiDir_None;
	data->IsSplitDirExplicit = false;
	if (!host_window->Collapsed)
		for (int dir = KGGuiDir_None; dir < KGGuiDir_COUNT; dir++)
		{
			if (dir == KGGuiDir_None && !data->IsCenterAvailable)
				continue;
			if (dir != KGGuiDir_None && !data->IsSidesAvailable)
				continue;
			if (Karma::DockNodeCalcDropRectsAndTestMousePos(data->FutureNode.Rect(), (KarmaGuiDir)dir, data->DropRectsDraw[dir + 1], is_outer_docking, &g.IO.MousePos))
			{
				data->SplitDir = (KarmaGuiDir)dir;
				data->IsSplitDirExplicit = true;
			}
		}

	// When docking without holding Shift, we only allow and preview docking when hovering over a drop rect or over the title bar
	data->IsDropAllowed = (data->SplitDir != KGGuiDir_None) || (data->IsCenterAvailable);
	if (!is_explicit_target && !data->IsSplitDirExplicit && !g.IO.ConfigDockingWithShift)
		data->IsDropAllowed = false;

	// Calculate split area
	data->SplitRatio = 0.0f;
	if (data->SplitDir != KGGuiDir_None)
	{
		KarmaGuiDir split_dir = data->SplitDir;
		KGGuiAxis split_axis = (split_dir == KGGuiDir_Left || split_dir == KGGuiDir_Right) ? KGGuiAxis_X : KGGuiAxis_Y;
		KGVec2 pos_new, pos_old = data->FutureNode.Pos;
		KGVec2 size_new, size_old = data->FutureNode.Size;
		Karma::DockNodeCalcSplitRects(pos_old, size_old, pos_new, size_new, split_dir, payload_window->Size);

		// Calculate split ratio so we can pass it down the docking request
		float split_ratio = KGSaturate(size_new[split_axis] / data->FutureNode.Size[split_axis]);
		data->FutureNode.Pos = pos_new;
		data->FutureNode.Size = size_new;
		data->SplitRatio = (split_dir == KGGuiDir_Right || split_dir == KGGuiDir_Down) ? (1.0f - split_ratio) : (split_ratio);
	}
}

void Karma::DockNodePreviewDockRender(KGGuiWindow* host_window, KGGuiDockNode* host_node, KGGuiWindow* root_payload, const ImGuiDockPreviewData* data)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(g.CurrentWindow == host_window, "");   // Because we rely on font size to calculate tab sizes

	// With this option, we only display the preview on the target viewport, and the payload viewport is made transparent.
	// To compensate for the single layer obstructed by the payload, we'll increase the alpha of the preview nodes.
	const bool is_transparent_payload = g.IO.ConfigDockingTransparentPayload;

	// In case the two windows involved are on different viewports, we will draw the overlay on each of them.
	int overlay_draw_lists_count = 0;
	KGDrawList* overlay_draw_lists[2];
	overlay_draw_lists[overlay_draw_lists_count++] = Karma::KarmaGui::GetForegroundDrawList(host_window->Viewport);
	if (host_window->Viewport != root_payload->Viewport && !is_transparent_payload)
		overlay_draw_lists[overlay_draw_lists_count++] = Karma::KarmaGui::GetForegroundDrawList(root_payload->Viewport);

	// Draw main preview rectangle
	const KGU32 overlay_col_main = Karma::KarmaGui::GetColorU32(KGGuiCol_DockingPreview, is_transparent_payload ? 0.60f : 0.40f);
	const KGU32 overlay_col_drop = Karma::KarmaGui::GetColorU32(KGGuiCol_DockingPreview, is_transparent_payload ? 0.90f : 0.70f);
	const KGU32 overlay_col_drop_hovered = Karma::KarmaGui::GetColorU32(KGGuiCol_DockingPreview, is_transparent_payload ? 1.20f : 1.00f);
	const KGU32 overlay_col_lines = Karma::KarmaGui::GetColorU32(KGGuiCol_NavWindowingHighlight, is_transparent_payload ? 0.80f : 0.60f);

	// Display area preview
	const bool can_preview_tabs = (root_payload->DockNodeAsHost == NULL || root_payload->DockNodeAsHost->Windows.Size > 0);
	if (data->IsDropAllowed)
	{
		KGRect overlay_rect = data->FutureNode.Rect();
		if (data->SplitDir == KGGuiDir_None && can_preview_tabs)
			overlay_rect.Min.y += Karma::KarmaGui::GetFrameHeight();
		if (data->SplitDir != KGGuiDir_None || data->IsCenterAvailable)
			for (int overlay_n = 0; overlay_n < overlay_draw_lists_count; overlay_n++)
				overlay_draw_lists[overlay_n]->AddRectFilled(overlay_rect.Min, overlay_rect.Max, overlay_col_main, host_window->WindowRounding, Karma::KarmaGuiInternal::CalcRoundingFlagsForRectInRect(overlay_rect, host_window->Rect(), DOCKING_SPLITTER_SIZE));
	}

	// Display tab shape/label preview unless we are splitting node (it generally makes the situation harder to read)
	if (data->IsDropAllowed && can_preview_tabs && data->SplitDir == KGGuiDir_None && data->IsCenterAvailable)
	{
		// Compute target tab bar geometry so we can locate our preview tabs
		KGRect tab_bar_rect;
		Karma::DockNodeCalcTabBarLayout(&data->FutureNode, NULL, &tab_bar_rect, NULL, NULL);
		KGVec2 tab_pos = tab_bar_rect.Min;
		if (host_node && host_node->TabBar)
		{
			if (!host_node->IsHiddenTabBar() && !host_node->IsNoTabBar())
				tab_pos.x += host_node->TabBar->WidthAllTabs + g.Style.ItemInnerSpacing.x; // We don't use OffsetNewTab because when using non-persistent-order tab bar it is incremented with each Tab submission.
			else
				tab_pos.x += g.Style.ItemInnerSpacing.x + Karma::KarmaGuiInternal::TabItemCalcSize(host_node->Windows[0]).x;
		}
		else if (!(host_window->Flags & KGGuiWindowFlags_DockNodeHost))
		{
			tab_pos.x += g.Style.ItemInnerSpacing.x + Karma::KarmaGuiInternal::TabItemCalcSize(host_window).x; // Account for slight offset which will be added when changing from title bar to tab bar
		}

		// Draw tab shape/label preview (payload may be a loose window or a host window carrying multiple tabbed windows)
		if (root_payload->DockNodeAsHost)
		{
			KR_CORE_ASSERT(root_payload->DockNodeAsHost->Windows.Size <= root_payload->DockNodeAsHost->TabBar->Tabs.Size, "");
		}
		KGGuiTabBar* tab_bar_with_payload = root_payload->DockNodeAsHost ? root_payload->DockNodeAsHost->TabBar : NULL;
		const int payload_count = tab_bar_with_payload ? tab_bar_with_payload->Tabs.Size : 1;
		for (int payload_n = 0; payload_n < payload_count; payload_n++)
		{
			// DockNode's TabBar may have non-window Tabs manually appended by user
			KGGuiWindow* payload_window = tab_bar_with_payload ? tab_bar_with_payload->Tabs[payload_n].Window : root_payload;
			if (tab_bar_with_payload && payload_window == NULL)
				continue;
			if (!DockNodeIsDropAllowedOne(payload_window, host_window))
				continue;

			// Calculate the tab bounding box for each payload window
			KGVec2 tab_size = Karma::KarmaGuiInternal::TabItemCalcSize(payload_window);
			KGRect tab_bb(tab_pos.x, tab_pos.y, tab_pos.x + tab_size.x, tab_pos.y + tab_size.y);
			tab_pos.x += tab_size.x + g.Style.ItemInnerSpacing.x;
			const KGU32 overlay_col_text = Karma::KarmaGui::GetColorU32(payload_window->DockStyle.Colors[KGGuiWindowDockStyleCol_Text]);
			const KGU32 overlay_col_tabs = Karma::KarmaGui::GetColorU32(payload_window->DockStyle.Colors[KGGuiWindowDockStyleCol_TabActive]);
			Karma::KarmaGui::PushStyleColor(KGGuiCol_Text, overlay_col_text);
			for (int overlay_n = 0; overlay_n < overlay_draw_lists_count; overlay_n++)
			{
				KarmaGuiTabItemFlags tab_flags = KGGuiTabItemFlags_Preview | ((payload_window->Flags & KGGuiWindowFlags_UnsavedDocument) ? KGGuiTabItemFlags_UnsavedDocument : 0);
				if (!tab_bar_rect.Contains(tab_bb))
					overlay_draw_lists[overlay_n]->PushClipRect(tab_bar_rect.Min, tab_bar_rect.Max);
				Karma::KarmaGuiInternal::TabItemBackground(overlay_draw_lists[overlay_n], tab_bb, tab_flags, overlay_col_tabs);
				Karma::KarmaGuiInternal::TabItemLabelAndCloseButton(overlay_draw_lists[overlay_n], tab_bb, tab_flags, g.Style.FramePadding, payload_window->Name, 0, 0, false, NULL, NULL);
				if (!tab_bar_rect.Contains(tab_bb))
					overlay_draw_lists[overlay_n]->PopClipRect();
			}
			Karma::KarmaGui::PopStyleColor();
		}
	}

	// Display drop boxes
	const float overlay_rounding = KGMax(3.0f, g.Style.FrameRounding);
	for (int dir = KGGuiDir_None; dir < KGGuiDir_COUNT; dir++)
	{
		if (!data->DropRectsDraw[dir + 1].IsInverted())
		{
			KGRect draw_r = data->DropRectsDraw[dir + 1];
			KGRect draw_r_in = draw_r;
			draw_r_in.Expand(-2.0f);
			KGU32 overlay_col = (data->SplitDir == (KarmaGuiDir)dir && data->IsSplitDirExplicit) ? overlay_col_drop_hovered : overlay_col_drop;
			for (int overlay_n = 0; overlay_n < overlay_draw_lists_count; overlay_n++)
			{
				KGVec2 center = KGFloor(draw_r_in.GetCenter());
				overlay_draw_lists[overlay_n]->AddRectFilled(draw_r.Min, draw_r.Max, overlay_col, overlay_rounding);
				overlay_draw_lists[overlay_n]->AddRect(draw_r_in.Min, draw_r_in.Max, overlay_col_lines, overlay_rounding);
				if (dir == KGGuiDir_Left || dir == KGGuiDir_Right)
					overlay_draw_lists[overlay_n]->AddLine(KGVec2(center.x, draw_r_in.Min.y), KGVec2(center.x, draw_r_in.Max.y), overlay_col_lines);
				if (dir == KGGuiDir_Up || dir == KGGuiDir_Down)
					overlay_draw_lists[overlay_n]->AddLine(KGVec2(draw_r_in.Min.x, center.y), KGVec2(draw_r_in.Max.x, center.y), overlay_col_lines);
			}
		}

		// Stop after KGGuiDir_None
		if ((host_node && (host_node->MergedFlags & KGGuiDockNodeFlags_NoSplit)) || g.IO.ConfigDockingNoSplit)
			return;
	}
}

//-----------------------------------------------------------------------------
// Docking: KGGuiDockNode Tree manipulation functions
//-----------------------------------------------------------------------------
// - DockNodeTreeSplit()
// - DockNodeTreeMerge()
// - DockNodeTreeUpdatePosSize()
// - DockNodeTreeUpdateSplitterFindTouchingNode()
// - DockNodeTreeUpdateSplitter()
// - DockNodeTreeFindFallbackLeafNode()
// - DockNodeTreeFindNodeByPos()
//-----------------------------------------------------------------------------

void Karma::DockNodeTreeSplit(KarmaGuiContext* ctx, KGGuiDockNode* parent_node, KGGuiAxis split_axis, int split_inheritor_child_idx, float split_ratio, KGGuiDockNode* new_node)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(split_axis != KGGuiAxis_None, "");

	KGGuiDockNode* child_0 = (new_node && split_inheritor_child_idx != 0) ? new_node : Karma::DockContextAddNode(ctx, 0);
	child_0->ParentNode = parent_node;

	KGGuiDockNode* child_1 = (new_node && split_inheritor_child_idx != 1) ? new_node : Karma::DockContextAddNode(ctx, 0);
	child_1->ParentNode = parent_node;

	KGGuiDockNode* child_inheritor = (split_inheritor_child_idx == 0) ? child_0 : child_1;
	Karma::DockNodeMoveChildNodes(child_inheritor, parent_node);
	parent_node->ChildNodes[0] = child_0;
	parent_node->ChildNodes[1] = child_1;
	parent_node->ChildNodes[split_inheritor_child_idx]->VisibleWindow = parent_node->VisibleWindow;
	parent_node->SplitAxis = split_axis;
	parent_node->VisibleWindow = NULL;
	parent_node->AuthorityForPos = parent_node->AuthorityForSize = KGGuiDataAuthority_DockNode;

	float size_avail = (parent_node->Size[split_axis] - DOCKING_SPLITTER_SIZE);
	size_avail = KGMax(size_avail, g.Style.WindowMinSize[split_axis] * 2.0f);
	KR_CORE_ASSERT(size_avail > 0.0f, ""); // If you created a node manually with DockBuilderAddNode(), you need to also call DockBuilderSetNodeSize() before splitting.
	child_0->SizeRef = child_1->SizeRef = parent_node->Size;
	child_0->SizeRef[split_axis] = KGFloor(size_avail * split_ratio);
	child_1->SizeRef[split_axis] = KGFloor(size_avail - child_0->SizeRef[split_axis]);

	Karma::DockNodeMoveWindows(parent_node->ChildNodes[split_inheritor_child_idx], parent_node);
	Karma::DockSettingsRenameNodeReferences(parent_node->ID, parent_node->ChildNodes[split_inheritor_child_idx]->ID);
	Karma::DockNodeUpdateHasCentralNodeChild(Karma::KarmaGuiInternal::DockNodeGetRootNode(parent_node));
	Karma::DockNodeTreeUpdatePosSize(parent_node, parent_node->Pos, parent_node->Size);

	// Flags transfer (e.g. this is where we transfer the KGGuiDockNodeFlags_CentralNode property)
	child_0->SharedFlags = parent_node->SharedFlags & KGGuiDockNodeFlags_SharedFlagsInheritMask_;
	child_1->SharedFlags = parent_node->SharedFlags & KGGuiDockNodeFlags_SharedFlagsInheritMask_;
	child_inheritor->LocalFlags = parent_node->LocalFlags & KGGuiDockNodeFlags_LocalFlagsTransferMask_;
	parent_node->LocalFlags &= ~KGGuiDockNodeFlags_LocalFlagsTransferMask_;
	child_0->UpdateMergedFlags();
	child_1->UpdateMergedFlags();
	parent_node->UpdateMergedFlags();
	if (child_inheritor->IsCentralNode())
		Karma::KarmaGuiInternal::DockNodeGetRootNode(parent_node)->CentralNode = child_inheritor;
}

void Karma::DockNodeTreeMerge(KarmaGuiContext* ctx, KGGuiDockNode* parent_node, KGGuiDockNode* merge_lead_child)
{
	// When called from DockContextProcessUndockNode() it is possible that one of the child is NULL.
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiDockNode* child_0 = parent_node->ChildNodes[0];
	KGGuiDockNode* child_1 = parent_node->ChildNodes[1];
	KR_CORE_ASSERT(child_0 || child_1, "");
	KR_CORE_ASSERT(merge_lead_child == child_0 || merge_lead_child == child_1, "");
	if ((child_0 && child_0->Windows.Size > 0) || (child_1 && child_1->Windows.Size > 0))
	{
		KR_CORE_ASSERT(parent_node->TabBar == NULL, "");
		KR_CORE_ASSERT(parent_node->Windows.Size == 0, "");
	}
	KR_CORE_INFO("[docking] DockNodeTreeMerge: {0} + {1} back into parent {2}", child_0 ? child_0->ID : 0, child_1 ? child_1->ID : 0, parent_node->ID);

	KGVec2 backup_last_explicit_size = parent_node->SizeRef;
	Karma::DockNodeMoveChildNodes(parent_node, merge_lead_child);
	if (child_0)
	{
		Karma::DockNodeMoveWindows(parent_node, child_0); // Generally only 1 of the 2 child node will have windows
		Karma::DockSettingsRenameNodeReferences(child_0->ID, parent_node->ID);
	}
	if (child_1)
	{
		Karma::DockNodeMoveWindows(parent_node, child_1);
		Karma::DockSettingsRenameNodeReferences(child_1->ID, parent_node->ID);
	}
	Karma::DockNodeApplyPosSizeToWindows(parent_node);
	parent_node->AuthorityForPos = parent_node->AuthorityForSize = parent_node->AuthorityForViewport = KGGuiDataAuthority_Auto;
	parent_node->VisibleWindow = merge_lead_child->VisibleWindow;
	parent_node->SizeRef = backup_last_explicit_size;

	// Flags transfer
	parent_node->LocalFlags &= ~KGGuiDockNodeFlags_LocalFlagsTransferMask_; // Preserve Dockspace flag
	parent_node->LocalFlags |= (child_0 ? child_0->LocalFlags : 0) & KGGuiDockNodeFlags_LocalFlagsTransferMask_;
	parent_node->LocalFlags |= (child_1 ? child_1->LocalFlags : 0) & KGGuiDockNodeFlags_LocalFlagsTransferMask_;
	parent_node->LocalFlagsInWindows = (child_0 ? child_0->LocalFlagsInWindows : 0) | (child_1 ? child_1->LocalFlagsInWindows : 0); // FIXME: Would be more consistent to update from actual windows
	parent_node->UpdateMergedFlags();

	if (child_0)
	{
		ctx->DockContext.Nodes.SetVoidPtr(child_0->ID, NULL);
		KG_DELETE(child_0);
	}
	if (child_1)
	{
		ctx->DockContext.Nodes.SetVoidPtr(child_1->ID, NULL);
		KG_DELETE(child_1);
	}
}

// Update Pos/Size for a node hierarchy (don't affect child Windows yet)
// (Depth-first, Pre-Order)
void Karma::DockNodeTreeUpdatePosSize(KGGuiDockNode* node, KGVec2 pos, KGVec2 size, KGGuiDockNode* only_write_to_single_node)
{
	// During the regular dock node update we write to all nodes.
	// 'only_write_to_single_node' is only set when turning a node visible mid-frame and we need its size right-away.
	const bool write_to_node = only_write_to_single_node == NULL || only_write_to_single_node == node;
	if (write_to_node)
	{
		node->Pos = pos;
		node->Size = size;
	}

	if (node->IsLeafNode())
		return;

	KGGuiDockNode* child_0 = node->ChildNodes[0];
	KGGuiDockNode* child_1 = node->ChildNodes[1];
	KGVec2 child_0_pos = pos, child_1_pos = pos;
	KGVec2 child_0_size = size, child_1_size = size;

	const bool child_0_is_toward_single_node = (only_write_to_single_node != NULL && Karma::KarmaGuiInternal::DockNodeIsInHierarchyOf(only_write_to_single_node, child_0));
	const bool child_1_is_toward_single_node = (only_write_to_single_node != NULL && Karma::KarmaGuiInternal::DockNodeIsInHierarchyOf(only_write_to_single_node, child_1));
	const bool child_0_is_or_will_be_visible = child_0->IsVisible || child_0_is_toward_single_node;
	const bool child_1_is_or_will_be_visible = child_1->IsVisible || child_1_is_toward_single_node;

	if (child_0_is_or_will_be_visible && child_1_is_or_will_be_visible)
	{
		KarmaGuiContext& g = *GKarmaGui;
		const float spacing = DOCKING_SPLITTER_SIZE;
		const KGGuiAxis axis = (KGGuiAxis)node->SplitAxis;
		const float size_avail = KGMax(size[axis] - spacing, 0.0f);

		// Size allocation policy
		// 1) The first 0..WindowMinSize[axis]*2 are allocated evenly to both windows.
		const float size_min_each = KGFloor(KGMin(size_avail, g.Style.WindowMinSize[axis] * 2.0f) * 0.5f);

		// FIXME: Blocks 2) and 3) are essentially doing nearly the same thing.
		// Difference are: write-back to SizeRef; application of a minimum size; rounding before KGFloor()
		// Clarify and rework differences between Size & SizeRef and purpose of WantLockSizeOnce

		// 2) Process locked absolute size (during a splitter resize we preserve the child of nodes not touching the splitter edge)
		if (child_0->WantLockSizeOnce && !child_1->WantLockSizeOnce)
		{
			child_0_size[axis] = child_0->SizeRef[axis] = KGMin(size_avail - 1.0f, child_0->Size[axis]);
			child_1_size[axis] = child_1->SizeRef[axis] = (size_avail - child_0_size[axis]);
			KR_CORE_ASSERT(child_0->SizeRef[axis] > 0.0f && child_1->SizeRef[axis] > 0.0f, "");
		}
		else if (child_1->WantLockSizeOnce && !child_0->WantLockSizeOnce)
		{
			child_1_size[axis] = child_1->SizeRef[axis] = KGMin(size_avail - 1.0f, child_1->Size[axis]);
			child_0_size[axis] = child_0->SizeRef[axis] = (size_avail - child_1_size[axis]);
			KR_CORE_ASSERT(child_0->SizeRef[axis] > 0.0f && child_1->SizeRef[axis] > 0.0f, "");
		}
		else if (child_0->WantLockSizeOnce && child_1->WantLockSizeOnce)
		{
			// FIXME-DOCK: We cannot honor the requested size, so apply ratio.
			// Currently this path will only be taken if code programmatically sets WantLockSizeOnce
			float split_ratio = child_0_size[axis] / (child_0_size[axis] + child_1_size[axis]);
			child_0_size[axis] = child_0->SizeRef[axis] = KGFloor(size_avail * split_ratio);
			child_1_size[axis] = child_1->SizeRef[axis] = (size_avail - child_0_size[axis]);
			KR_CORE_ASSERT(child_0->SizeRef[axis] > 0.0f && child_1->SizeRef[axis] > 0.0f, "");
		}

		// 3) If one window is the central node (~ use remaining space, should be made explicit!), use explicit size from the other, and remainder for the central node
		else if (child_0->SizeRef[axis] != 0.0f && child_1->HasCentralNodeChild)
		{
			child_0_size[axis] = KGMin(size_avail - size_min_each, child_0->SizeRef[axis]);
			child_1_size[axis] = (size_avail - child_0_size[axis]);
		}
		else if (child_1->SizeRef[axis] != 0.0f && child_0->HasCentralNodeChild)
		{
			child_1_size[axis] = KGMin(size_avail - size_min_each, child_1->SizeRef[axis]);
			child_0_size[axis] = (size_avail - child_1_size[axis]);
		}
		else
		{
			// 4) Otherwise distribute according to the relative ratio of each SizeRef value
			float split_ratio = child_0->SizeRef[axis] / (child_0->SizeRef[axis] + child_1->SizeRef[axis]);
			child_0_size[axis] = KGMax(size_min_each, KGFloor(size_avail * split_ratio + 0.5f));
			child_1_size[axis] = (size_avail - child_0_size[axis]);
		}

		child_1_pos[axis] += spacing + child_0_size[axis];
	}

	if (only_write_to_single_node == NULL)
		child_0->WantLockSizeOnce = child_1->WantLockSizeOnce = false;

	const bool child_0_recurse = only_write_to_single_node ? child_0_is_toward_single_node : child_0->IsVisible;
	const bool child_1_recurse = only_write_to_single_node ? child_1_is_toward_single_node : child_1->IsVisible;
	if (child_0_recurse)
		Karma::DockNodeTreeUpdatePosSize(child_0, child_0_pos, child_0_size);
	if (child_1_recurse)
		Karma::DockNodeTreeUpdatePosSize(child_1, child_1_pos, child_1_size);
}

static void DockNodeTreeUpdateSplitterFindTouchingNode(KGGuiDockNode* node, KGGuiAxis axis, int side, KGVector<KGGuiDockNode*>* touching_nodes)
{
	if (node->IsLeafNode())
	{
		touching_nodes->push_back(node);
		return;
	}
	if (node->ChildNodes[0]->IsVisible)
		if (node->SplitAxis != axis || side == 0 || !node->ChildNodes[1]->IsVisible)
			DockNodeTreeUpdateSplitterFindTouchingNode(node->ChildNodes[0], axis, side, touching_nodes);
	if (node->ChildNodes[1]->IsVisible)
		if (node->SplitAxis != axis || side == 1 || !node->ChildNodes[0]->IsVisible)
			DockNodeTreeUpdateSplitterFindTouchingNode(node->ChildNodes[1], axis, side, touching_nodes);
}

// (Depth-First, Pre-Order)
void Karma::DockNodeTreeUpdateSplitter(KGGuiDockNode* node)
{
	if (node->IsLeafNode())
		return;

	KarmaGuiContext& g = *GKarmaGui;

	KGGuiDockNode* child_0 = node->ChildNodes[0];
	KGGuiDockNode* child_1 = node->ChildNodes[1];
	if (child_0->IsVisible && child_1->IsVisible)
	{
		// Bounding box of the splitter cover the space between both nodes (w = Spacing, h = Size[xy^1] for when splitting horizontally)
		const KGGuiAxis axis = (KGGuiAxis)node->SplitAxis;
		KR_CORE_ASSERT(axis != KGGuiAxis_None, "");
		KGRect bb;
		bb.Min = child_0->Pos;
		bb.Max = child_1->Pos;
		bb.Min[axis] += child_0->Size[axis];
		bb.Max[axis ^ 1] += child_1->Size[axis ^ 1];
		//if (g.IO.KeyCtrl) GetForegroundDrawList(g.CurrentWindow->Viewport)->AddRect(bb.Min, bb.Max, KG_COL32(255,0,255,255));

		const KarmaGuiDockNodeFlags merged_flags = child_0->MergedFlags | child_1->MergedFlags; // Merged flags for BOTH childs
		const KarmaGuiDockNodeFlags no_resize_axis_flag = (axis == KGGuiAxis_X) ? KGGuiDockNodeFlags_NoResizeX : KGGuiDockNodeFlags_NoResizeY;
		if ((merged_flags & KGGuiDockNodeFlags_NoResize) || (merged_flags & no_resize_axis_flag))
		{
			KGGuiWindow* window = g.CurrentWindow;
			window->DrawList->AddRectFilled(bb.Min, bb.Max, Karma::KarmaGui::GetColorU32(KGGuiCol_Separator), g.Style.FrameRounding);
		}
		else
		{
			//bb.Min[axis] += 1; // Display a little inward so highlight doesn't connect with nearby tabs on the neighbor node.
			//bb.Max[axis] -= 1;
			Karma::KarmaGui::PushID(node->ID);

			// Find resizing limits by gathering list of nodes that are touching the splitter line.
			KGVector<KGGuiDockNode*> touching_nodes[2];
			float min_size = g.Style.WindowMinSize[axis];
			float resize_limits[2];
			resize_limits[0] = node->ChildNodes[0]->Pos[axis] + min_size;
			resize_limits[1] = node->ChildNodes[1]->Pos[axis] + node->ChildNodes[1]->Size[axis] - min_size;

			KGGuiID splitter_id = Karma::KarmaGui::GetID("##Splitter");
			if (g.ActiveId == splitter_id) // Only process when splitter is active
			{
				DockNodeTreeUpdateSplitterFindTouchingNode(child_0, axis, 1, &touching_nodes[0]);
				DockNodeTreeUpdateSplitterFindTouchingNode(child_1, axis, 0, &touching_nodes[1]);
				for (int touching_node_n = 0; touching_node_n < touching_nodes[0].Size; touching_node_n++)
					resize_limits[0] = KGMax(resize_limits[0], touching_nodes[0][touching_node_n]->Rect().Min[axis] + min_size);
				for (int touching_node_n = 0; touching_node_n < touching_nodes[1].Size; touching_node_n++)
					resize_limits[1] = KGMin(resize_limits[1], touching_nodes[1][touching_node_n]->Rect().Max[axis] - min_size);

				// [DEBUG] Render touching nodes & limits
				/*
				KGDrawList* draw_list = node->HostWindow ? GetForegroundDrawList(node->HostWindow) : GetForegroundDrawList(GetMainViewport());
				for (int n = 0; n < 2; n++)
				{
					for (int touching_node_n = 0; touching_node_n < touching_nodes[n].Size; touching_node_n++)
						draw_list->AddRect(touching_nodes[n][touching_node_n]->Pos, touching_nodes[n][touching_node_n]->Pos + touching_nodes[n][touching_node_n]->Size, KG_COL32(0, 255, 0, 255));
					if (axis == KGGuiAxis_X)
						draw_list->AddLine(KGVec2(resize_limits[n], node->ChildNodes[n]->Pos.y), KGVec2(resize_limits[n], node->ChildNodes[n]->Pos.y + node->ChildNodes[n]->Size.y), KG_COL32(255, 0, 255, 255), 3.0f);
					else
						draw_list->AddLine(KGVec2(node->ChildNodes[n]->Pos.x, resize_limits[n]), KGVec2(node->ChildNodes[n]->Pos.x + node->ChildNodes[n]->Size.x, resize_limits[n]), KG_COL32(255, 0, 255, 255), 3.0f);
				}
				*/
			}

			// Use a short delay before highlighting the splitter (and changing the mouse cursor) in order for regular mouse movement to not highlight many splitters
			float cur_size_0 = child_0->Size[axis];
			float cur_size_1 = child_1->Size[axis];
			float min_size_0 = resize_limits[0] - child_0->Pos[axis];
			float min_size_1 = child_1->Pos[axis] + child_1->Size[axis] - resize_limits[1];
			KGU32 bg_col = Karma::KarmaGui::GetColorU32(KGGuiCol_WindowBg);
			if (Karma::KarmaGuiInternal::SplitterBehavior(bb, Karma::KarmaGui::GetID("##Splitter"), axis, &cur_size_0, &cur_size_1, min_size_0, min_size_1, WINDOWS_HOVER_PADDING, WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER, bg_col))
			{
				if (touching_nodes[0].Size > 0 && touching_nodes[1].Size > 0)
				{
					child_0->Size[axis] = child_0->SizeRef[axis] = cur_size_0;
					child_1->Pos[axis] -= cur_size_1 - child_1->Size[axis];
					child_1->Size[axis] = child_1->SizeRef[axis] = cur_size_1;

					// Lock the size of every node that is a sibling of the node we are touching
					// This might be less desirable if we can merge sibling of a same axis into the same parental level.
					for (int side_n = 0; side_n < 2; side_n++)
						for (int touching_node_n = 0; touching_node_n < touching_nodes[side_n].Size; touching_node_n++)
						{
							KGGuiDockNode* touching_node = touching_nodes[side_n][touching_node_n];
							//KGDrawList* draw_list = node->HostWindow ? GetForegroundDrawList(node->HostWindow) : GetForegroundDrawList(GetMainViewport());
							//draw_list->AddRect(touching_node->Pos, touching_node->Pos + touching_node->Size, KG_COL32(255, 128, 0, 255));
							while (touching_node->ParentNode != node)
							{
								if (touching_node->ParentNode->SplitAxis == axis)
								{
									// Mark other node so its size will be preserved during the upcoming call to DockNodeTreeUpdatePosSize().
									KGGuiDockNode* node_to_preserve = touching_node->ParentNode->ChildNodes[side_n];
									node_to_preserve->WantLockSizeOnce = true;
									//draw_list->AddRect(touching_node->Pos, touching_node->Rect().Max, KG_COL32(255, 0, 0, 255));
									//draw_list->AddRectFilled(node_to_preserve->Pos, node_to_preserve->Rect().Max, KG_COL32(0, 255, 0, 100));
								}
								touching_node = touching_node->ParentNode;
							}
						}

					Karma::DockNodeTreeUpdatePosSize(child_0, child_0->Pos, child_0->Size);
					Karma::DockNodeTreeUpdatePosSize(child_1, child_1->Pos, child_1->Size);
					Karma::KarmaGuiInternal::MarkIniSettingsDirty();
				}
			}
			Karma::KarmaGui::PopID();
		}
	}

	if (child_0->IsVisible)
		Karma::DockNodeTreeUpdateSplitter(child_0);
	if (child_1->IsVisible)
		Karma::DockNodeTreeUpdateSplitter(child_1);
}

KGGuiDockNode* Karma::DockNodeTreeFindFallbackLeafNode(KGGuiDockNode* node)
{
	if (node->IsLeafNode())
		return node;
	if (KGGuiDockNode* leaf_node = DockNodeTreeFindFallbackLeafNode(node->ChildNodes[0]))
		return leaf_node;
	if (KGGuiDockNode* leaf_node = DockNodeTreeFindFallbackLeafNode(node->ChildNodes[1]))
		return leaf_node;
	return NULL;
}

KGGuiDockNode* Karma::DockNodeTreeFindVisibleNodeByPos(KGGuiDockNode* node, KGVec2 pos)
{
	if (!node->IsVisible)
		return NULL;

	const float dock_spacing = 0.0f;// g.Style.ItemInnerSpacing.x; // FIXME: Relation to DOCKING_SPLITTER_SIZE?
	KGRect r(node->Pos, node->Pos + node->Size);
	r.Expand(dock_spacing * 0.5f);
	bool inside = r.Contains(pos);
	if (!inside)
		return NULL;

	if (node->IsLeafNode())
		return node;
	if (KGGuiDockNode* hovered_node = DockNodeTreeFindVisibleNodeByPos(node->ChildNodes[0], pos))
		return hovered_node;
	if (KGGuiDockNode* hovered_node = DockNodeTreeFindVisibleNodeByPos(node->ChildNodes[1], pos))
		return hovered_node;

	// This means we are hovering over the splitter/spacing of a parent node
	return node;
}

//-----------------------------------------------------------------------------
// Docking: Public Functions (SetWindowDock, DockSpace, DockSpaceOverViewport)
//-----------------------------------------------------------------------------
// - SetWindowDock() [Internal]
// - DockSpace()
// - DockSpaceOverViewport()
//-----------------------------------------------------------------------------

// [Internal] Called via SetNextWindowDockID()
void Karma::KarmaGuiInternal::SetWindowDock(KGGuiWindow* window, KGGuiID dock_id, KarmaGuiCond cond)
{
	// Test condition (NB: bit 0 is always true) and clear flags for next time
	if (cond && (window->SetWindowDockAllowFlags & cond) == 0)
		return;
	window->SetWindowDockAllowFlags &= ~(KGGuiCond_Once | KGGuiCond_FirstUseEver | KGGuiCond_Appearing);

	if (window->DockId == dock_id)
		return;

	// If the user attempt to set a dock id that is a split node, we'll dig within to find a suitable docking spot
	KarmaGuiContext* ctx = GKarmaGui;
	if (KGGuiDockNode* new_node = Karma::KarmaGuiInternal::DockContextFindNodeByID(ctx, dock_id))
		if (new_node->IsSplitNode())
		{
			// Policy: Find central node or latest focused node. We first move back to our root node.
			new_node = Karma::KarmaGuiInternal::DockNodeGetRootNode(new_node);
			if (new_node->CentralNode)
			{
				KR_CORE_ASSERT(new_node->CentralNode->IsCentralNode(), "");
				dock_id = new_node->CentralNode->ID;
			}
			else
			{
				dock_id = new_node->LastFocusedNodeId;
			}
		}

	if (window->DockId == dock_id)
		return;

	if (window->DockNode)
		Karma::DockNodeRemoveWindow(window->DockNode, window, 0);
	window->DockId = dock_id;
}

// Create an explicit dockspace node within an existing window. Also expose dock node flags and creates a CentralNode by default.
// The Central Node is always displayed even when empty and shrink/extend according to the requested size of its neighbors.
// DockSpace() needs to be submitted _before_ any window they can host. If you use a dockspace, submit it early in your app.
KGGuiID Karma::KarmaGui::DockSpace(KGGuiID id, const KGVec2& size_arg, KarmaGuiDockNodeFlags flags, const KarmaGuiWindowClass* window_class)
{
	KarmaGuiContext* ctx = GKarmaGui;
	KarmaGuiContext& g = *ctx;
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	if (!(g.IO.ConfigFlags & KGGuiConfigFlags_DockingEnable))
		return 0;

	// Early out if parent window is hidden/collapsed
	// This is faster but also DockNodeUpdateTabBar() relies on TabBarLayout() running (which won't if SkipItems=true) to set NextSelectedTabId = 0). See #2960.
	// If for whichever reason this is causing problem we would need to ensure that DockNodeUpdateTabBar() ends up clearing NextSelectedTabId even if SkipItems=true.
	if (window->SkipItems)
		flags |= KGGuiDockNodeFlags_KeepAliveOnly;

	KR_CORE_ASSERT((flags & KGGuiDockNodeFlags_DockSpace) == 0, "");
	KR_CORE_ASSERT(id != 0, "");
	KGGuiDockNode* node = KarmaGuiInternal::DockContextFindNodeByID(ctx, id);
	if (!node)
	{
		KR_CORE_INFO("[docking] DockSpace: dockspace node {0} created", id);
		node = DockContextAddNode(ctx, id);
		node->SetLocalFlags(KGGuiDockNodeFlags_CentralNode);
	}
	if (window_class && window_class->ClassId != node->WindowClass.ClassId)
	{
		KR_CORE_INFO("[docking] DockSpace: dockspace node {0}: setup WindowClass {1} -> {2}", id, node->WindowClass.ClassId, window_class->ClassId);
	}
	node->SharedFlags = flags;
	node->WindowClass = window_class ? *window_class : KarmaGuiWindowClass();

	// When a DockSpace transitioned form implicit to explicit this may be called a second time
	// It is possible that the node has already been claimed by a docked window which appeared before the DockSpace() node, so we overwrite IsDockSpace again.
	if (node->LastFrameActive == g.FrameCount && !(flags & KGGuiDockNodeFlags_KeepAliveOnly))
	{
		KR_CORE_ASSERT(node->IsDockSpace() == false, "Cannot call DockSpace() twice a frame with the same ID");
		node->SetLocalFlags(node->LocalFlags | KGGuiDockNodeFlags_DockSpace);
		return id;
	}
	node->SetLocalFlags(node->LocalFlags | KGGuiDockNodeFlags_DockSpace);

	// Keep alive mode, this is allow windows docked into this node so stay docked even if they are not visible
	if (flags & KGGuiDockNodeFlags_KeepAliveOnly)
	{
		node->LastFrameAlive = g.FrameCount;
		return id;
	}

	const KGVec2 content_avail = GetContentRegionAvail();
	KGVec2 size = KGFloor(size_arg);
	if (size.x <= 0.0f)
		size.x = KGMax(content_avail.x + size.x, 4.0f); // Arbitrary minimum child size (0.0f causing too much issues)
	if (size.y <= 0.0f)
		size.y = KGMax(content_avail.y + size.y, 4.0f);
	KR_CORE_ASSERT(size.x > 0.0f && size.y > 0.0f, "");

	node->Pos = window->DC.CursorPos;
	node->Size = node->SizeRef = size;
	SetNextWindowPos(node->Pos);
	SetNextWindowSize(node->Size);
	g.NextWindowData.PosUndock = false;

	// FIXME-DOCK: Why do we need a child window to host a dockspace, could we host it in the existing window?
	// FIXME-DOCK: What is the reason for not simply calling BeginChild()? (OK to have a reason but should be commented)
	KarmaGuiWindowFlags window_flags = KGGuiWindowFlags_ChildWindow | KGGuiWindowFlags_DockNodeHost;
	window_flags |= KGGuiWindowFlags_NoSavedSettings | KGGuiWindowFlags_NoResize | KGGuiWindowFlags_NoCollapse | KGGuiWindowFlags_NoTitleBar;
	window_flags |= KGGuiWindowFlags_NoScrollbar | KGGuiWindowFlags_NoScrollWithMouse;
	window_flags |= KGGuiWindowFlags_NoBackground;

	char title[256];
	KGFormatString(title, KG_ARRAYSIZE(title), "%s/DockSpace_%08X", window->Name, id);

	PushStyleVar(KGGuiStyleVar_ChildBorderSize, 0.0f);
	Begin(title, NULL, window_flags);
	PopStyleVar();

	KGGuiWindow* host_window = g.CurrentWindow;
	DockNodeSetupHostWindow(node, host_window);
	host_window->ChildId = window->GetID(title);
	node->OnlyNodeWithWindows = NULL;

	KR_CORE_ASSERT(node->IsRootNode(), "");

	// We need to handle the rare case were a central node is missing.
	// This can happen if the node was first created manually with DockBuilderAddNode() but _without_ the KGGuiDockNodeFlags_Dockspace.
	// Doing it correctly would set the _CentralNode flags, which would then propagate according to subsequent split.
	// It would also be ambiguous to attempt to assign a central node while there are split nodes, so we wait until there's a single node remaining.
	// The specific sub-property of _CentralNode we are interested in recovering here is the "Don't delete when empty" property,
	// as it doesn't make sense for an empty dockspace to not have this property.
	if (node->IsLeafNode() && !node->IsCentralNode())
		node->SetLocalFlags(node->LocalFlags | KGGuiDockNodeFlags_CentralNode);

	// Update the node
	Karma::DockNodeUpdate(node);

	End();
	KarmaGuiInternal::ItemSize(size);
	return id;
}

// Tips: Use with KGGuiDockNodeFlags_PassthruCentralNode!
// The limitation with this call is that your window won't have a menu bar.
// Even though we could pass window flags, it would also require the user to be able to call BeginMenuBar() somehow meaning we can't Begin/End in a single function.
// But you can also use BeginMainMenuBar(). If you really want a menu bar inside the same window as the one hosting the dockspace, you will need to copy this code somewhere and tweak it.
KGGuiID Karma::KarmaGui::DockSpaceOverViewport(const KarmaGuiViewport* viewport, KarmaGuiDockNodeFlags dockspace_flags, const KarmaGuiWindowClass* window_class)
{
	if (viewport == NULL)
		viewport = GetMainViewport();

	SetNextWindowPos(viewport->WorkPos);
	SetNextWindowSize(viewport->WorkSize);
	SetNextWindowViewport(viewport->ID);

	KarmaGuiWindowFlags host_window_flags = 0;
	host_window_flags |= KGGuiWindowFlags_NoTitleBar | KGGuiWindowFlags_NoCollapse | KGGuiWindowFlags_NoResize | KGGuiWindowFlags_NoMove | KGGuiWindowFlags_NoDocking;
	host_window_flags |= KGGuiWindowFlags_NoBringToFrontOnFocus | KGGuiWindowFlags_NoNavFocus;
	if (dockspace_flags & KGGuiDockNodeFlags_PassthruCentralNode)
		host_window_flags |= KGGuiWindowFlags_NoBackground;

	char label[32];
	KGFormatString(label, KG_ARRAYSIZE(label), "DockSpaceViewport_%08X", viewport->ID);

	PushStyleVar(KGGuiStyleVar_WindowRounding, 0.0f);
	PushStyleVar(KGGuiStyleVar_WindowBorderSize, 0.0f);
	PushStyleVar(KGGuiStyleVar_WindowPadding, KGVec2(0.0f, 0.0f));
	Begin(label, NULL, host_window_flags);
	PopStyleVar(3);

	KGGuiID dockspace_id = GetID("DockSpace");
	DockSpace(dockspace_id, KGVec2(0.0f, 0.0f), dockspace_flags, window_class);
	End();

	return dockspace_id;
}

//-----------------------------------------------------------------------------
// Docking: Builder Functions
//-----------------------------------------------------------------------------
// Very early end-user API to manipulate dock nodes.
// Only available in imgui_internal.h. Expect this API to change/break!
// It is expected that those functions are all called _before_ the dockspace node submission.
//-----------------------------------------------------------------------------
// - DockBuilderDockWindow()
// - DockBuilderGetNode()
// - DockBuilderSetNodePos()
// - DockBuilderSetNodeSize()
// - DockBuilderAddNode()
// - DockBuilderRemoveNode()
// - DockBuilderRemoveNodeChildNodes()
// - DockBuilderRemoveNodeDockedWindows()
// - DockBuilderSplitNode()
// - DockBuilderCopyNodeRec()
// - DockBuilderCopyNode()
// - DockBuilderCopyWindowSettings()
// - DockBuilderCopyDockSpace()
// - DockBuilderFinish()
//-----------------------------------------------------------------------------

void DockBuilderDockWindow(const char* window_name, KGGuiID node_id)
{
	// We don't preserve relative order of multiple docked windows (by clearing DockOrder back to -1)
	KGGuiID window_id = KGHashStr(window_name);
	if (KGGuiWindow* window = Karma::KarmaGuiInternal::FindWindowByID(window_id))
	{
		// Apply to created window
		Karma::KarmaGuiInternal::SetWindowDock(window, node_id, KGGuiCond_Always);
		window->DockOrder = -1;
	}
	else
	{
		// Apply to settings
		KGGuiWindowSettings* settings = Karma::KarmaGuiInternal::FindWindowSettings(window_id);
		if (settings == NULL)
			settings = Karma::KarmaGuiInternal::CreateNewWindowSettings(window_name);
		settings->DockId = node_id;
		settings->DockOrder = -1;
	}
}

KGGuiDockNode* DockBuilderGetNode(KGGuiID node_id)
{
	KarmaGuiContext* ctx = Karma::GKarmaGui;
	return Karma::KarmaGuiInternal::DockContextFindNodeByID(ctx, node_id);
}

void DockBuilderSetNodePos(KGGuiID node_id, KGVec2 pos)
{
	KarmaGuiContext* ctx = Karma::GKarmaGui;
	KGGuiDockNode* node = Karma::KarmaGuiInternal::DockContextFindNodeByID(ctx, node_id);
	if (node == NULL)
		return;
	node->Pos = pos;
	node->AuthorityForPos = KGGuiDataAuthority_DockNode;
}

void DockBuilderSetNodeSize(KGGuiID node_id, KGVec2 size)
{
	KarmaGuiContext* ctx = Karma::GKarmaGui;
	KGGuiDockNode* node = Karma::KarmaGuiInternal::DockContextFindNodeByID(ctx, node_id);
	if (node == NULL)
		return;
	KR_CORE_ASSERT(size.x > 0.0f && size.y > 0.0f, "");
	node->Size = node->SizeRef = size;
	node->AuthorityForSize = KGGuiDataAuthority_DockNode;
}

// Make sure to use the KGGuiDockNodeFlags_DockSpace flag to create a dockspace node! Otherwise this will create a floating node!
// - Floating node: you can then call DockBuilderSetNodePos()/DockBuilderSetNodeSize() to position and size the floating node.
// - Dockspace node: calling DockBuilderSetNodePos() is unnecessary.
// - If you intend to split a node immediately after creation using DockBuilderSplitNode(), make sure to call DockBuilderSetNodeSize() beforehand!
//   For various reason, the splitting code currently needs a base size otherwise space may not be allocated as precisely as you would expect.
// - Use (id == 0) to let the system allocate a node identifier.
// - Existing node with a same id will be removed.
KGGuiID DockBuilderAddNode(KGGuiID id, KarmaGuiDockNodeFlags flags)
{
	KarmaGuiContext* ctx = Karma::GKarmaGui;

	if (id != 0)
		Karma::KarmaGuiInternal::DockBuilderRemoveNode(id);

	KGGuiDockNode* node = NULL;
	if (flags & KGGuiDockNodeFlags_DockSpace)
	{
		Karma::KarmaGui::DockSpace(id, KGVec2(0, 0), (flags & ~KGGuiDockNodeFlags_DockSpace) | KGGuiDockNodeFlags_KeepAliveOnly);
		node = Karma::KarmaGuiInternal::DockContextFindNodeByID(ctx, id);
	}
	else
	{
		node = Karma::DockContextAddNode(ctx, id);
		node->SetLocalFlags(flags);
	}
	node->LastFrameAlive = ctx->FrameCount;   // Set this otherwise BeginDocked will undock during the same frame.
	return node->ID;
}

void Karma::KarmaGuiInternal::DockBuilderRemoveNode(KGGuiID node_id)
{
	KarmaGuiContext* ctx = GKarmaGui;
	KGGuiDockNode* node = Karma::KarmaGuiInternal::DockContextFindNodeByID(ctx, node_id);
	if (node == NULL)
		return;
	Karma::KarmaGuiInternal::DockBuilderRemoveNodeDockedWindows(node_id, true);
	Karma::KarmaGuiInternal::DockBuilderRemoveNodeChildNodes(node_id);
	// Node may have moved or deleted if e.g. any merge happened
	node = Karma::KarmaGuiInternal::DockContextFindNodeByID(ctx, node_id);
	if (node == NULL)
		return;
	if (node->IsCentralNode() && node->ParentNode)
		node->ParentNode->SetLocalFlags(node->ParentNode->LocalFlags | KGGuiDockNodeFlags_CentralNode);
	Karma::DockContextRemoveNode(ctx, node, true);
}

// root_id = 0 to remove all, root_id != 0 to remove child of given node.
void Karma::KarmaGuiInternal::DockBuilderRemoveNodeChildNodes(KGGuiID root_id)
{
	KarmaGuiContext* ctx = GKarmaGui;
	KGGuiDockContext* dc = &ctx->DockContext;

	KGGuiDockNode* root_node = root_id ? Karma::KarmaGuiInternal::DockContextFindNodeByID(ctx, root_id) : NULL;
	if (root_id && root_node == NULL)
		return;
	bool has_central_node = false;

	KGGuiDataAuthority backup_root_node_authority_for_pos = root_node ? root_node->AuthorityForPos : KGGuiDataAuthority_Auto;
	KGGuiDataAuthority backup_root_node_authority_for_size = root_node ? root_node->AuthorityForSize : KGGuiDataAuthority_Auto;

	// Process active windows
	KGVector<KGGuiDockNode*> nodes_to_remove;
	for (int n = 0; n < dc->Nodes.Data.Size; n++)
		if (KGGuiDockNode* node = (KGGuiDockNode*)dc->Nodes.Data[n].val_p)
		{
			bool want_removal = (root_id == 0) || (node->ID != root_id && Karma::KarmaGuiInternal::DockNodeGetRootNode(node)->ID == root_id);
			if (want_removal)
			{
				if (node->IsCentralNode())
					has_central_node = true;
				if (root_id != 0)
					Karma::DockContextQueueNotifyRemovedNode(ctx, node);
				if (root_node)
				{
					Karma::DockNodeMoveWindows(root_node, node);
					Karma::DockSettingsRenameNodeReferences(node->ID, root_node->ID);
				}
				nodes_to_remove.push_back(node);
			}
		}

	// DockNodeMoveWindows->DockNodeAddWindow will normally set those when reaching two windows (which is only adequate during interactive merge)
	// Make sure we don't lose our current pos/size. (FIXME-DOCK: Consider tidying up that code in DockNodeAddWindow instead)
	if (root_node)
	{
		root_node->AuthorityForPos = backup_root_node_authority_for_pos;
		root_node->AuthorityForSize = backup_root_node_authority_for_size;
	}

	// Apply to settings
	for (KGGuiWindowSettings* settings = ctx->SettingsWindows.begin(); settings != NULL; settings = ctx->SettingsWindows.next_chunk(settings))
		if (KGGuiID window_settings_dock_id = settings->DockId)
			for (int n = 0; n < nodes_to_remove.Size; n++)
				if (nodes_to_remove[n]->ID == window_settings_dock_id)
				{
					settings->DockId = root_id;
					break;
				}

	// Not really efficient, but easier to destroy a whole hierarchy considering DockContextRemoveNode is attempting to merge nodes
	if (nodes_to_remove.Size > 1)
		KGQsort(nodes_to_remove.Data, nodes_to_remove.Size, sizeof(KGGuiDockNode*), DockNodeComparerDepthMostFirst);
	for (int n = 0; n < nodes_to_remove.Size; n++)
		Karma::DockContextRemoveNode(ctx, nodes_to_remove[n], false);

	if (root_id == 0)
	{
		dc->Nodes.Clear();
		dc->Requests.clear();
	}
	else if (has_central_node)
	{
		root_node->CentralNode = root_node;
		root_node->SetLocalFlags(root_node->LocalFlags | KGGuiDockNodeFlags_CentralNode);
	}
}

void Karma::KarmaGuiInternal::DockBuilderRemoveNodeDockedWindows(KGGuiID root_id, bool clear_settings_refs)
{
	// Clear references in settings
	KarmaGuiContext* ctx = GKarmaGui;
	KarmaGuiContext& g = *ctx;
	if (clear_settings_refs)
	{
		for (KGGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
		{
			bool want_removal = (root_id == 0) || (settings->DockId == root_id);
			if (!want_removal && settings->DockId != 0)
				if (KGGuiDockNode* node = Karma::KarmaGuiInternal::DockContextFindNodeByID(ctx, settings->DockId))
					if (Karma::KarmaGuiInternal::DockNodeGetRootNode(node)->ID == root_id)
						want_removal = true;
			if (want_removal)
				settings->DockId = 0;
		}
	}

	// Clear references in windows
	for (int n = 0; n < g.Windows.Size; n++)
	{
		KGGuiWindow* window = g.Windows[n];
		bool want_removal = (root_id == 0) || (window->DockNode && Karma::KarmaGuiInternal::DockNodeGetRootNode(window->DockNode)->ID == root_id) || (window->DockNodeAsHost && window->DockNodeAsHost->ID == root_id);
		if (want_removal)
		{
			const KGGuiID backup_dock_id = window->DockId;
			KG_UNUSED(backup_dock_id);
			Karma::DockContextProcessUndockWindow(ctx, window, clear_settings_refs);
			if (!clear_settings_refs)
			{
				KR_CORE_ASSERT(window->DockId == backup_dock_id, "");
			}
		}
	}
}

// If 'out_id_at_dir' or 'out_id_at_opposite_dir' are non NULL, the function will write out the ID of the two new nodes created.
// Return value is ID of the node at the specified direction, so same as (*out_id_at_dir) if that pointer is set.
// FIXME-DOCK: We are not exposing nor using split_outer.
KGGuiID DockBuilderSplitNode(KGGuiID id, KarmaGuiDir split_dir, float size_ratio_for_node_at_dir, KGGuiID* out_id_at_dir, KGGuiID* out_id_at_opposite_dir)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KR_CORE_ASSERT(split_dir != KGGuiDir_None, "");
	KR_CORE_INFO("[docking] DockBuilderSplitNode: node {0}, split_dir {1}", id, split_dir);

	KGGuiDockNode* node = Karma::KarmaGuiInternal::DockContextFindNodeByID(&g, id);
	if (node == NULL)
	{
		KR_CORE_ASSERT(node != NULL, "");
		return 0;
	}

	KR_CORE_ASSERT(!node->IsSplitNode(), ""); // Assert if already Split

	KGGuiDockRequest req;
	req.Type = ImGuiDockRequestType_Split;
	req.DockTargetWindow = NULL;
	req.DockTargetNode = node;
	req.DockPayload = NULL;
	req.DockSplitDir = split_dir;
	req.DockSplitRatio = KGSaturate((split_dir == KGGuiDir_Left || split_dir == KGGuiDir_Up) ? size_ratio_for_node_at_dir : 1.0f - size_ratio_for_node_at_dir);
	req.DockSplitOuter = false;
	Karma::DockContextProcessDock(&g, &req);

	KGGuiID id_at_dir = node->ChildNodes[(split_dir == KGGuiDir_Left || split_dir == KGGuiDir_Up) ? 0 : 1]->ID;
	KGGuiID id_at_opposite_dir = node->ChildNodes[(split_dir == KGGuiDir_Left || split_dir == KGGuiDir_Up) ? 1 : 0]->ID;
	if (out_id_at_dir)
		*out_id_at_dir = id_at_dir;
	if (out_id_at_opposite_dir)
		*out_id_at_opposite_dir = id_at_opposite_dir;
	return id_at_dir;
}

static KGGuiDockNode* DockBuilderCopyNodeRec(KGGuiDockNode* src_node, KGGuiID dst_node_id_if_known, KGVector<KGGuiID>* out_node_remap_pairs)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGGuiDockNode* dst_node = Karma::DockContextAddNode(&g, dst_node_id_if_known);
	dst_node->SharedFlags = src_node->SharedFlags;
	dst_node->LocalFlags = src_node->LocalFlags;
	dst_node->LocalFlagsInWindows = KGGuiDockNodeFlags_None;
	dst_node->Pos = src_node->Pos;
	dst_node->Size = src_node->Size;
	dst_node->SizeRef = src_node->SizeRef;
	dst_node->SplitAxis = src_node->SplitAxis;
	dst_node->UpdateMergedFlags();

	out_node_remap_pairs->push_back(src_node->ID);
	out_node_remap_pairs->push_back(dst_node->ID);

	for (int child_n = 0; child_n < KG_ARRAYSIZE(src_node->ChildNodes); child_n++)
		if (src_node->ChildNodes[child_n])
		{
			dst_node->ChildNodes[child_n] = DockBuilderCopyNodeRec(src_node->ChildNodes[child_n], 0, out_node_remap_pairs);
			dst_node->ChildNodes[child_n]->ParentNode = dst_node;
		}

	KR_CORE_INFO("[docking] Fork node {0} -> {1} ({2} childs)\n", src_node->ID, dst_node->ID, dst_node->IsSplitNode() ? 2 : 0);
	return dst_node;
}

void DockBuilderCopyNode(KGGuiID src_node_id, KGGuiID dst_node_id, KGVector<KGGuiID>* out_node_remap_pairs)
{
	KarmaGuiContext* ctx = Karma::GKarmaGui;
	KR_CORE_ASSERT(src_node_id != 0, "");
	KR_CORE_ASSERT(dst_node_id != 0, "");
	KR_CORE_ASSERT(out_node_remap_pairs != NULL, "");

	Karma::KarmaGuiInternal::DockBuilderRemoveNode(dst_node_id);

	KGGuiDockNode* src_node = Karma::KarmaGuiInternal::DockContextFindNodeByID(ctx, src_node_id);
	KR_CORE_ASSERT(src_node != NULL, "");

	out_node_remap_pairs->clear();
	DockBuilderCopyNodeRec(src_node, dst_node_id, out_node_remap_pairs);

	KR_CORE_ASSERT((out_node_remap_pairs->Size % 2) == 0, "");
}

void DockBuilderCopyWindowSettings(const char* src_name, const char* dst_name)
{
	KGGuiWindow* src_window = Karma::KarmaGuiInternal::FindWindowByName(src_name);
	if (src_window == NULL)
		return;
	if (KGGuiWindow* dst_window = Karma::KarmaGuiInternal::FindWindowByName(dst_name))
	{
		dst_window->Pos = src_window->Pos;
		dst_window->Size = src_window->Size;
		dst_window->SizeFull = src_window->SizeFull;
		dst_window->Collapsed = src_window->Collapsed;
	}
	else if (KGGuiWindowSettings* dst_settings = Karma::KarmaGuiInternal::FindOrCreateWindowSettings(dst_name))
	{
		KGVec2ih window_pos_2ih = KGVec2ih(src_window->Pos);
		if (src_window->ViewportId != 0 && src_window->ViewportId != Karma::KarmaGuiInternal::IMGUI_VIEWPORT_DEFAULT_ID)
		{
			dst_settings->ViewportPos = window_pos_2ih;
			dst_settings->ViewportId = src_window->ViewportId;
			dst_settings->Pos = KGVec2ih(0, 0);
		}
		else
		{
			dst_settings->Pos = window_pos_2ih;
		}
		dst_settings->Size = KGVec2ih(src_window->SizeFull);
		dst_settings->Collapsed = src_window->Collapsed;
	}
}

// FIXME: Will probably want to change this signature, in particular how the window remapping pairs are passed.
void DockBuilderCopyDockSpace(KGGuiID src_dockspace_id, KGGuiID dst_dockspace_id, KGVector<const char*>* in_window_remap_pairs)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KR_CORE_ASSERT(src_dockspace_id != 0, "");
	KR_CORE_ASSERT(dst_dockspace_id != 0, "");
	KR_CORE_ASSERT(in_window_remap_pairs != NULL, "");
	KR_CORE_ASSERT((in_window_remap_pairs->Size % 2) == 0, "");

	// Duplicate entire dock
	// FIXME: When overwriting dst_dockspace_id, windows that aren't part of our dockspace window class but that are docked in a same node will be split apart,
	// whereas we could attempt to at least keep them together in a new, same floating node.
	KGVector<KGGuiID> node_remap_pairs;
	DockBuilderCopyNode(src_dockspace_id, dst_dockspace_id, &node_remap_pairs);

	// Attempt to transition all the upcoming windows associated to dst_dockspace_id into the newly created hierarchy of dock nodes
	// (The windows associated to src_dockspace_id are staying in place)
	KGVector<KGGuiID> src_windows;
	for (int remap_window_n = 0; remap_window_n < in_window_remap_pairs->Size; remap_window_n += 2)
	{
		const char* src_window_name = (*in_window_remap_pairs)[remap_window_n];
		const char* dst_window_name = (*in_window_remap_pairs)[remap_window_n + 1];
		KGGuiID src_window_id = KGHashStr(src_window_name);
		src_windows.push_back(src_window_id);

		// Search in the remapping tables
		KGGuiID src_dock_id = 0;
		if (KGGuiWindow* src_window = Karma::KarmaGuiInternal::FindWindowByID(src_window_id))
			src_dock_id = src_window->DockId;
		else if (KGGuiWindowSettings* src_window_settings = Karma::KarmaGuiInternal::FindWindowSettings(src_window_id))
			src_dock_id = src_window_settings->DockId;
		KGGuiID dst_dock_id = 0;
		for (int dock_remap_n = 0; dock_remap_n < node_remap_pairs.Size; dock_remap_n += 2)
			if (node_remap_pairs[dock_remap_n] == src_dock_id)
			{
				dst_dock_id = node_remap_pairs[dock_remap_n + 1];
				//node_remap_pairs[dock_remap_n] = node_remap_pairs[dock_remap_n + 1] = 0; // Clear
				break;
			}

		if (dst_dock_id != 0)
		{
			// Docked windows gets redocked into the new node hierarchy.
			KR_CORE_INFO("[docking] Remap live window '{0}' {1} -> '{2}' {3}", src_window_name, src_dock_id, dst_window_name, dst_dock_id);
			DockBuilderDockWindow(dst_window_name, dst_dock_id);
		}
		else
		{
			// Floating windows gets their settings transferred (regardless of whether the new window already exist or not)
			// When this is leading to a Copy and not a Move, we would get two overlapping floating windows. Could we possibly dock them together?
			KR_CORE_INFO("[docking] Remap window settings '{0}' -> '{1}'", src_window_name, dst_window_name);
			DockBuilderCopyWindowSettings(src_window_name, dst_window_name);
		}
	}

	// Anything else in the source nodes of 'node_remap_pairs' are windows that are not included in the remapping list.
	// Find those windows and move to them to the cloned dock node. This may be optional?
	// Dock those are a second step as undocking would invalidate source dock nodes.
	struct DockRemainingWindowTask { KGGuiWindow* Window; KGGuiID DockId; DockRemainingWindowTask(KGGuiWindow* window, KGGuiID dock_id) { Window = window; DockId = dock_id; } };
	KGVector<DockRemainingWindowTask> dock_remaining_windows;
	for (int dock_remap_n = 0; dock_remap_n < node_remap_pairs.Size; dock_remap_n += 2)
		if (KGGuiID src_dock_id = node_remap_pairs[dock_remap_n])
		{
			KGGuiID dst_dock_id = node_remap_pairs[dock_remap_n + 1];
			KGGuiDockNode* node = DockBuilderGetNode(src_dock_id);
			for (int window_n = 0; window_n < node->Windows.Size; window_n++)
			{
				KGGuiWindow* window = node->Windows[window_n];
				if (src_windows.contains(window->ID))
					continue;

				// Docked windows gets redocked into the new node hierarchy.
				KR_CORE_INFO("[docking] Remap window '{0}' {1} -> {2}", window->Name, src_dock_id, dst_dock_id);
				dock_remaining_windows.push_back(DockRemainingWindowTask(window, dst_dock_id));
			}
		}
	for (const DockRemainingWindowTask& task : dock_remaining_windows)
		DockBuilderDockWindow(task.Window->Name, task.DockId);
}

// FIXME-DOCK: This is awkward because in series of split user is likely to loose access to its root node.
void DockBuilderFinish(KGGuiID root_id)
{
	KarmaGuiContext* ctx = Karma::GKarmaGui;
	//DockContextRebuild(ctx);
	Karma::DockContextBuildAddWindowsToNodes(ctx, root_id);
}

//-----------------------------------------------------------------------------
// Docking: Begin/End Support Functions (called from Begin/End)
//-----------------------------------------------------------------------------
// - GetWindowAlwaysWantOwnTabBar()
// - DockContextBindNodeToWindow()
// - BeginDocked()
// - BeginDockableDragDropSource()
// - BeginDockableDragDropTarget()
//-----------------------------------------------------------------------------

bool Karma::KarmaGuiInternal::GetWindowAlwaysWantOwnTabBar(KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.IO.ConfigDockingAlwaysTabBar || window->WindowClass.DockingAlwaysTabBar)
		if ((window->Flags & (KGGuiWindowFlags_ChildWindow | KGGuiWindowFlags_NoTitleBar | KGGuiWindowFlags_NoDocking)) == 0)
			if (!window->IsFallbackWindow)    // We don't support AlwaysTabBar on the fallback/implicit window to avoid unused dock-node overhead/noise
				return true;
	return false;
}

static KGGuiDockNode* Karma::DockContextBindNodeToWindow(KarmaGuiContext* ctx, KGGuiWindow* window)
{
	KarmaGuiContext& g = *ctx;
	KGGuiDockNode* node = Karma::KarmaGuiInternal::DockContextFindNodeByID(ctx, window->DockId);
	KR_CORE_ASSERT(window->DockNode == NULL, "");

	// We should not be docking into a split node (SetWindowDock should avoid this)
	if (node && node->IsSplitNode())
	{
		Karma::DockContextProcessUndockWindow(ctx, window);
		return NULL;
	}

	// Create node
	if (node == NULL)
	{
		node = Karma::DockContextAddNode(ctx, window->DockId);
		node->AuthorityForPos = node->AuthorityForSize = node->AuthorityForViewport = KGGuiDataAuthority_Window;
		node->LastFrameAlive = g.FrameCount;
	}

	// If the node just turned visible and is part of a hierarchy, it doesn't have a Size assigned by DockNodeTreeUpdatePosSize() yet,
	// so we're forcing a Pos/Size update from the first ancestor that is already visible (often it will be the root node).
	// If we don't do this, the window will be assigned a zero-size on its first frame, which won't ideally warm up the layout.
	// This is a little wonky because we don't normally update the Pos/Size of visible node mid-frame.
	if (!node->IsVisible)
	{
		KGGuiDockNode* ancestor_node = node;
		while (!ancestor_node->IsVisible && ancestor_node->ParentNode)
			ancestor_node = ancestor_node->ParentNode;
		KR_CORE_ASSERT(ancestor_node->Size.x > 0.0f && ancestor_node->Size.y > 0.0f, "");
		Karma::DockNodeUpdateHasCentralNodeChild(Karma::KarmaGuiInternal::DockNodeGetRootNode(ancestor_node));
		Karma::DockNodeTreeUpdatePosSize(ancestor_node, ancestor_node->Pos, ancestor_node->Size, node);
	}

	// Add window to node
	bool node_was_visible = node->IsVisible;
	Karma::DockNodeAddWindow(node, window, true);
	node->IsVisible = node_was_visible; // Don't mark visible right away (so DockContextEndFrame() doesn't render it, maybe other side effects? will see)
	KR_CORE_ASSERT(node == window->DockNode, "");
	return node;
}

void Karma::KarmaGuiInternal::BeginDocked(KGGuiWindow* window, bool* p_open)
{
	KarmaGuiContext* ctx = GKarmaGui;
	KarmaGuiContext& g = *ctx;

	// Clear fields ahead so most early-out paths don't have to do it
	window->DockIsActive = window->DockNodeIsVisible = window->DockTabIsVisible = false;

	const bool auto_dock_node = GetWindowAlwaysWantOwnTabBar(window);
	if (auto_dock_node)
	{
		if (window->DockId == 0)
		{
			KR_CORE_ASSERT(window->DockNode == NULL, "");
			window->DockId = Karma::KarmaGuiInternal::DockContextGenNodeID(ctx);
		}
	}
	else
	{
		// Calling SetNextWindowPos() undock windows by default (by setting PosUndock)
		bool want_undock = false;
		want_undock |= (window->Flags & KGGuiWindowFlags_NoDocking) != 0;
		want_undock |= (g.NextWindowData.Flags & KGGuiNextWindowDataFlags_HasPos) && (window->SetWindowPosAllowFlags & g.NextWindowData.PosCond) && g.NextWindowData.PosUndock;
		if (want_undock)
		{
			Karma::DockContextProcessUndockWindow(ctx, window);
			return;
		}
	}

	// Bind to our dock node
	KGGuiDockNode* node = window->DockNode;
	if (node != NULL)
	{
		KR_CORE_ASSERT(window->DockId == node->ID, "");
	}
	if (window->DockId != 0 && node == NULL)
	{
		node = Karma::DockContextBindNodeToWindow(ctx, window);
		if (node == NULL)
			return;
	}

#if 0
	// Undock if the KGGuiDockNodeFlags_NoDockingInCentralNode got set
	if (node->IsCentralNode && (node->Flags & KGGuiDockNodeFlags_NoDockingInCentralNode))
	{
		DockContextProcessUndockWindow(ctx, window);
		return;
	}
#endif

	// Undock if our dockspace node disappeared
	// Note how we are testing for LastFrameAlive and NOT LastFrameActive. A DockSpace node can be maintained alive while being inactive with KGGuiDockNodeFlags_KeepAliveOnly.
	if (node->LastFrameAlive < g.FrameCount)
	{
		// If the window has been orphaned, transition the docknode to an implicit node processed in DockContextNewFrameUpdateDocking()
		KGGuiDockNode* root_node = Karma::KarmaGuiInternal::DockNodeGetRootNode(node);
		if (root_node->LastFrameAlive < g.FrameCount)
			Karma::DockContextProcessUndockWindow(ctx, window);
		else
			window->DockIsActive = true;
		return;
	}

	// Store style overrides
	for (int color_n = 0; color_n < KGGuiWindowDockStyleCol_COUNT; color_n++)
		window->DockStyle.Colors[color_n] = Karma::KarmaGui::ColorConvertFloat4ToU32(g.Style.Colors[GWindowDockStyleColors[color_n]]);

	// Fast path return. It is common for windows to hold on a persistent DockId but be the only visible window,
	// and never create neither a host window neither a tab bar.
	// FIXME-DOCK: replace ->HostWindow NULL compare with something more explicit (~was initially intended as a first frame test)
	if (node->HostWindow == NULL)
	{
		if (node->State == KGGuiDockNodeState_HostWindowHiddenBecauseWindowsAreResizing)
			window->DockIsActive = true;
		if (node->Windows.Size > 1)
			DockNodeHideWindowDuringHostWindowCreation(window);
		return;
	}

	// We can have zero-sized nodes (e.g. children of a small-size dockspace)
	KR_CORE_ASSERT(node->HostWindow, "");
	KR_CORE_ASSERT(node->IsLeafNode(), "");
	KR_CORE_ASSERT(node->Size.x >= 0.0f && node->Size.y >= 0.0f, "");
	node->State = KGGuiDockNodeState_HostWindowVisible;

	// Undock if we are submitted earlier than the host window
	if (!(node->MergedFlags & KGGuiDockNodeFlags_KeepAliveOnly) && window->BeginOrderWithinContext < node->HostWindow->BeginOrderWithinContext)
	{
		Karma::DockContextProcessUndockWindow(ctx, window);
		return;
	}

	// Position/Size window
	Karma::KarmaGui::SetNextWindowPos(node->Pos);
	Karma::KarmaGui::SetNextWindowSize(node->Size);
	g.NextWindowData.PosUndock = false; // Cancel implicit undocking of SetNextWindowPos()
	window->DockIsActive = true;
	window->DockNodeIsVisible = true;
	window->DockTabIsVisible = false;
	if (node->MergedFlags & KGGuiDockNodeFlags_KeepAliveOnly)
		return;

	// When the window is selected we mark it as visible.
	if (node->VisibleWindow == window)
		window->DockTabIsVisible = true;

	// Update window flag
	KR_CORE_ASSERT((window->Flags & KGGuiWindowFlags_ChildWindow) == 0, "");
	window->Flags |= KGGuiWindowFlags_ChildWindow | KGGuiWindowFlags_AlwaysUseWindowPadding | KGGuiWindowFlags_NoResize;
	if (node->IsHiddenTabBar() || node->IsNoTabBar())
		window->Flags |= KGGuiWindowFlags_NoTitleBar;
	else
		window->Flags &= ~KGGuiWindowFlags_NoTitleBar;      // Clear the NoTitleBar flag in case the user set it: confusingly enough we need a title bar height so we are correctly offset, but it won't be displayed!

	// Save new dock order only if the window has been visible once already
	// This allows multiple windows to be created in the same frame and have their respective dock orders preserved.
	if (node->TabBar && window->WasActive)
		window->DockOrder = (short)Karma::DockNodeGetTabOrder(window);

	if ((node->WantCloseAll || node->WantCloseTabId == window->TabId) && p_open != NULL)
		*p_open = false;

	// Update ChildId to allow returning from Child to Parent with Escape
	KGGuiWindow* parent_window = window->DockNode->HostWindow;
	window->ChildId = parent_window->GetID(window->Name);
}

void Karma::KarmaGuiInternal::BeginDockableDragDropSource(KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(g.ActiveId == window->MoveId, "");
	KR_CORE_ASSERT(g.MovingWindow == window, "");
	KR_CORE_ASSERT(g.CurrentWindow == window, "");

	g.LastItemData.ID = window->MoveId;
	window = window->RootWindowDockTree;
	KR_CORE_ASSERT((window->Flags & KGGuiWindowFlags_NoDocking) == 0, "");
	bool is_drag_docking = (g.IO.ConfigDockingWithShift) || KGRect(0, 0, window->SizeFull.x, Karma::KarmaGui::GetFrameHeight()).Contains(g.ActiveIdClickOffset); // FIXME-DOCKING: Need to make this stateful and explicit
	if (is_drag_docking && Karma::KarmaGui::BeginDragDropSource(KGGuiDragDropFlags_SourceNoPreviewTooltip | KGGuiDragDropFlags_SourceNoHoldToOpenOthers | KGGuiDragDropFlags_SourceAutoExpirePayload))
	{
		Karma::KarmaGui::SetDragDropPayload(KARMAGUI_PAYLOAD_TYPE_WINDOW, &window, sizeof(window));
		Karma::KarmaGui::EndDragDropSource();

		// Store style overrides
		for (int color_n = 0; color_n < KGGuiWindowDockStyleCol_COUNT; color_n++)
			window->DockStyle.Colors[color_n] = Karma::KarmaGui::ColorConvertFloat4ToU32(g.Style.Colors[GWindowDockStyleColors[color_n]]);
	}
}

void Karma::KarmaGuiInternal::BeginDockableDragDropTarget(KGGuiWindow* window)
{
	KarmaGuiContext* ctx = GKarmaGui;
	KarmaGuiContext& g = *ctx;

	//KR_CORE_ASSERT(window->RootWindowDockTree == window); // May also be a DockSpace
	KR_CORE_ASSERT((window->Flags & KGGuiWindowFlags_NoDocking) == 0, "");
	if (!g.DragDropActive)
		return;
	//GetForegroundDrawList(window)->AddRect(window->Pos, window->Pos + window->Size, KG_COL32(255, 255, 0, 255));
	if (!Karma::KarmaGuiInternal::BeginDragDropTargetCustom(window->Rect(), window->ID))
		return;

	// Peek into the payload before calling AcceptDragDropPayload() so we can handle overlapping dock nodes with filtering
	// (this is a little unusual pattern, normally most code would call AcceptDragDropPayload directly)
	const KarmaGuiPayload* payload = &g.DragDropPayload;
	if (!payload->IsDataType(KARMAGUI_PAYLOAD_TYPE_WINDOW) || !Karma::DockNodeIsDropAllowed(window, *(KGGuiWindow**)payload->Data))
	{
		Karma::KarmaGui::EndDragDropTarget();
		return;
	}

	KGGuiWindow* payload_window = *(KGGuiWindow**)payload->Data;
	if (Karma::KarmaGui::AcceptDragDropPayload(KARMAGUI_PAYLOAD_TYPE_WINDOW, KGGuiDragDropFlags_AcceptBeforeDelivery | KGGuiDragDropFlags_AcceptNoDrawDefaultRect))
	{
		// Select target node
		// (Important: we cannot use g.HoveredDockNode here! Because each of our target node have filters based on payload, each candidate drop target will do its own evaluation)
		bool dock_into_floating_window = false;
		KGGuiDockNode* node = NULL;
		if (window->DockNodeAsHost)
		{
			// Cannot assume that node will != NULL even though we passed the rectangle test: it depends on padding/spacing handled by DockNodeTreeFindVisibleNodeByPos().
			node = Karma::DockNodeTreeFindVisibleNodeByPos(window->DockNodeAsHost, g.IO.MousePos);

			// There is an edge case when docking into a dockspace which only has _inactive_ nodes (because none of the windows are active)
			// In this case we need to fallback into any leaf mode, possibly the central node.
			// FIXME-20181220: We should not have to test for IsLeafNode() here but we have another bug to fix first.
			if (node && node->IsDockSpace() && node->IsRootNode())
				node = (node->CentralNode && node->IsLeafNode()) ? node->CentralNode : Karma::DockNodeTreeFindFallbackLeafNode(node);
		}
		else
		{
			if (window->DockNode)
				node = window->DockNode;
			else
				dock_into_floating_window = true; // Dock into a regular window
		}

		const KGRect explicit_target_rect = (node && node->TabBar && !node->IsHiddenTabBar() && !node->IsNoTabBar()) ? node->TabBar->BarRect : KGRect(window->Pos, window->Pos + KGVec2(window->Size.x, Karma::KarmaGui::GetFrameHeight()));
		const bool is_explicit_target = g.IO.ConfigDockingWithShift || Karma::KarmaGui::IsMouseHoveringRect(explicit_target_rect.Min, explicit_target_rect.Max);

		// Preview docking request and find out split direction/ratio
		//const bool do_preview = true;     // Ignore testing for payload->IsPreview() which removes one frame of delay, but breaks overlapping drop targets within the same window.
		const bool do_preview = payload->IsPreview() || payload->IsDelivery();
		if (do_preview && (node != NULL || dock_into_floating_window))
		{
			// If we have a non-leaf node it means we are hovering the border of a parent node, in which case only outer markers will appear.
			ImGuiDockPreviewData split_inner;
			ImGuiDockPreviewData split_outer;
			ImGuiDockPreviewData* split_data = &split_inner;
			if (node && (node->ParentNode || node->IsCentralNode() || !node->IsLeafNode()))
				if (KGGuiDockNode* root_node = Karma::KarmaGuiInternal::DockNodeGetRootNode(node))
				{
					Karma::DockNodePreviewDockSetup(window, root_node, payload_window, NULL, &split_outer, is_explicit_target, true);
					if (split_outer.IsSplitDirExplicit)
						split_data = &split_outer;
				}
			if (!node || node->IsLeafNode())
				Karma::DockNodePreviewDockSetup(window, node, payload_window, NULL, &split_inner, is_explicit_target, false);
			if (split_data == &split_outer)
				split_inner.IsDropAllowed = false;

			// Draw inner then outer, so that previewed tab (in inner data) will be behind the outer drop boxes
			Karma::DockNodePreviewDockRender(window, node, payload_window, &split_inner);
			Karma::DockNodePreviewDockRender(window, node, payload_window, &split_outer);

			// Queue docking request
			if (split_data->IsDropAllowed && payload->IsDelivery())
				Karma::KarmaGuiInternal::DockContextQueueDock(ctx, window, split_data->SplitNode, payload_window, split_data->SplitDir, split_data->SplitRatio, split_data == &split_outer);
		}
	}
	Karma::KarmaGui::EndDragDropTarget();
}

//-----------------------------------------------------------------------------
// Docking: Settings
//-----------------------------------------------------------------------------
// - DockSettingsRenameNodeReferences()
// - DockSettingsRemoveNodeReferences()
// - DockSettingsFindNodeSettings()
// - DockSettingsHandler_ApplyAll()
// - DockSettingsHandler_ReadOpen()
// - DockSettingsHandler_ReadLine()
// - DockSettingsHandler_DockNodeToSettings()
// - DockSettingsHandler_WriteAll()
//-----------------------------------------------------------------------------

void Karma::DockSettingsRenameNodeReferences(KGGuiID old_node_id, KGGuiID new_node_id)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_INFO("[docking] DockSettingsRenameNodeReferences: from {0} -> to {1}", old_node_id, new_node_id);
	for (int window_n = 0; window_n < g.Windows.Size; window_n++)
	{
		KGGuiWindow* window = g.Windows[window_n];
		if (window->DockId == old_node_id && window->DockNode == NULL)
			window->DockId = new_node_id;
	}
	//// FIXME-OPT: We could remove this loop by storing the index in the map
	for (KGGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
		if (settings->DockId == old_node_id)
			settings->DockId = new_node_id;
}

// Remove references stored in KGGuiWindowSettings to the given KGGuiDockNodeSettings
void Karma::DockSettingsRemoveNodeReferences(KGGuiID* node_ids, int node_ids_count)
{
	KarmaGuiContext& g = *GKarmaGui;
	int found = 0;
	//// FIXME-OPT: We could remove this loop by storing the index in the map
	for (KGGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
		for (int node_n = 0; node_n < node_ids_count; node_n++)
			if (settings->DockId == node_ids[node_n])
			{
				settings->DockId = 0;
				settings->DockOrder = -1;
				if (++found < node_ids_count)
					break;
				return;
			}
}

KGGuiDockNodeSettings* Karma::DockSettingsFindNodeSettings(KarmaGuiContext* ctx, KGGuiID id)
{
	// FIXME-OPT
	KGGuiDockContext* dc = &ctx->DockContext;
	for (int n = 0; n < dc->NodesSettings.Size; n++)
		if (dc->NodesSettings[n].ID == id)
			return &dc->NodesSettings[n];
	return NULL;
}

// Clear settings data
void Karma::DockSettingsHandler_ClearAll(KarmaGuiContext* ctx, KGGuiSettingsHandler*)
{
	KGGuiDockContext* dc = &ctx->DockContext;
	dc->NodesSettings.clear();
	Karma::KarmaGuiInternal::DockContextClearNodes(ctx, 0, true);
}

// Recreate nodes based on settings data
void Karma::DockSettingsHandler_ApplyAll(KarmaGuiContext* ctx, KGGuiSettingsHandler*)
{
	// Prune settings at boot time only
	KGGuiDockContext* dc = &ctx->DockContext;
	if (ctx->Windows.Size == 0)
		Karma::DockContextPruneUnusedSettingsNodes(ctx);
	Karma::DockContextBuildNodesFromSettings(ctx, dc->NodesSettings.Data, dc->NodesSettings.Size);
	Karma::DockContextBuildAddWindowsToNodes(ctx, 0);
}

void* Karma::DockSettingsHandler_ReadOpen(KarmaGuiContext*, KGGuiSettingsHandler*, const char* name)
{
	if (strcmp(name, "Data") != 0)
		return NULL;
	return (void*)1;
}

void Karma::DockSettingsHandler_ReadLine(KarmaGuiContext* ctx, KGGuiSettingsHandler*, void*, const char* line)
{
	char c = 0;
	int x = 0, y = 0;
	int r = 0;

	// Parsing, e.g.
	// " DockNode   ID=0x00000001 Pos=383,193 Size=201,322 Split=Y,0.506 "
	// "   DockNode ID=0x00000002 Parent=0x00000001 "
	// Important: this code expect currently fields in a fixed order.
	KGGuiDockNodeSettings node;
	line = KGStrSkipBlank(line);
	if (strncmp(line, "DockNode", 8) == 0) { line = KGStrSkipBlank(line + strlen("DockNode")); }
	else if (strncmp(line, "DockSpace", 9) == 0) { line = KGStrSkipBlank(line + strlen("DockSpace")); node.Flags |= KGGuiDockNodeFlags_DockSpace; }
	else return;
	if (sscanf(line, "ID=0x%08X%n", &node.ID, &r) == 1) { line += r; }
	else return;
	if (sscanf(line, " Parent=0x%08X%n", &node.ParentNodeId, &r) == 1) { line += r; if (node.ParentNodeId == 0) return; }
	if (sscanf(line, " Window=0x%08X%n", &node.ParentWindowId, &r) == 1) { line += r; if (node.ParentWindowId == 0) return; }
	if (node.ParentNodeId == 0)
	{
		if (sscanf(line, " Pos=%i,%i%n", &x, &y, &r) == 2) { line += r; node.Pos = KGVec2ih((short)x, (short)y); }
		else return;
		if (sscanf(line, " Size=%i,%i%n", &x, &y, &r) == 2) { line += r; node.Size = KGVec2ih((short)x, (short)y); }
		else return;
	}
	else
	{
		if (sscanf(line, " SizeRef=%i,%i%n", &x, &y, &r) == 2) { line += r; node.SizeRef = KGVec2ih((short)x, (short)y); }
	}
	if (sscanf(line, " Split=%c%n", &c, &r) == 1) { line += r; if (c == 'X') node.SplitAxis = KGGuiAxis_X; else if (c == 'Y') node.SplitAxis = KGGuiAxis_Y; }
	if (sscanf(line, " NoResize=%d%n", &x, &r) == 1) { line += r; if (x != 0) node.Flags |= KGGuiDockNodeFlags_NoResize; }
	if (sscanf(line, " CentralNode=%d%n", &x, &r) == 1) { line += r; if (x != 0) node.Flags |= KGGuiDockNodeFlags_CentralNode; }
	if (sscanf(line, " NoTabBar=%d%n", &x, &r) == 1) { line += r; if (x != 0) node.Flags |= KGGuiDockNodeFlags_NoTabBar; }
	if (sscanf(line, " HiddenTabBar=%d%n", &x, &r) == 1) { line += r; if (x != 0) node.Flags |= KGGuiDockNodeFlags_HiddenTabBar; }
	if (sscanf(line, " NoWindowMenuButton=%d%n", &x, &r) == 1) { line += r; if (x != 0) node.Flags |= KGGuiDockNodeFlags_NoWindowMenuButton; }
	if (sscanf(line, " NoCloseButton=%d%n", &x, &r) == 1) { line += r; if (x != 0) node.Flags |= KGGuiDockNodeFlags_NoCloseButton; }
	if (sscanf(line, " Selected=0x%08X%n", &node.SelectedTabId, &r) == 1) { line += r; }
	if (node.ParentNodeId != 0)
		if (KGGuiDockNodeSettings* parent_settings = Karma::DockSettingsFindNodeSettings(ctx, node.ParentNodeId))
			node.Depth = parent_settings->Depth + 1;
	ctx->DockContext.NodesSettings.push_back(node);
}

static void DockSettingsHandler_DockNodeToSettings(KGGuiDockContext* dc, KGGuiDockNode* node, int depth)
{
	KGGuiDockNodeSettings node_settings;
	KR_CORE_ASSERT(depth < (1 << (sizeof(node_settings.Depth) << 3)), "");
	node_settings.ID = node->ID;
	node_settings.ParentNodeId = node->ParentNode ? node->ParentNode->ID : 0;
	node_settings.ParentWindowId = (node->IsDockSpace() && node->HostWindow && node->HostWindow->ParentWindow) ? node->HostWindow->ParentWindow->ID : 0;
	node_settings.SelectedTabId = node->SelectedTabId;
	node_settings.SplitAxis = (signed char)(node->IsSplitNode() ? node->SplitAxis : KGGuiAxis_None);
	node_settings.Depth = (char)depth;
	node_settings.Flags = (node->LocalFlags & KGGuiDockNodeFlags_SavedFlagsMask_);
	node_settings.Pos = KGVec2ih(node->Pos);
	node_settings.Size = KGVec2ih(node->Size);
	node_settings.SizeRef = KGVec2ih(node->SizeRef);
	dc->NodesSettings.push_back(node_settings);
	if (node->ChildNodes[0])
		DockSettingsHandler_DockNodeToSettings(dc, node->ChildNodes[0], depth + 1);
	if (node->ChildNodes[1])
		DockSettingsHandler_DockNodeToSettings(dc, node->ChildNodes[1], depth + 1);
}

void Karma::DockSettingsHandler_WriteAll(KarmaGuiContext* ctx, KGGuiSettingsHandler* handler, KarmaGuiTextBuffer* buf)
{
	KarmaGuiContext& g = *ctx;
	KGGuiDockContext* dc = &ctx->DockContext;
	if (!(g.IO.ConfigFlags & KGGuiConfigFlags_DockingEnable))
		return;

	// Gather settings data
	// (unlike our windows settings, because nodes are always built we can do a full rewrite of the SettingsNode buffer)
	dc->NodesSettings.resize(0);
	dc->NodesSettings.reserve(dc->Nodes.Data.Size);
	for (int n = 0; n < dc->Nodes.Data.Size; n++)
		if (KGGuiDockNode* node = (KGGuiDockNode*)dc->Nodes.Data[n].val_p)
			if (node->IsRootNode())
				DockSettingsHandler_DockNodeToSettings(dc, node, 0);

	int max_depth = 0;
	for (int node_n = 0; node_n < dc->NodesSettings.Size; node_n++)
		max_depth = KGMax((int)dc->NodesSettings[node_n].Depth, max_depth);

	// Write to text buffer
	buf->appendf("[%s][Data]\n", handler->TypeName);
	for (int node_n = 0; node_n < dc->NodesSettings.Size; node_n++)
	{
		const int line_start_pos = buf->size(); (void)line_start_pos;
		const KGGuiDockNodeSettings* node_settings = &dc->NodesSettings[node_n];
		buf->appendf("%*s%s%*s", node_settings->Depth * 2, "", (node_settings->Flags & KGGuiDockNodeFlags_DockSpace) ? "DockSpace" : "DockNode ", (max_depth - node_settings->Depth) * 2, "");  // Text align nodes to facilitate looking at .ini file
		buf->appendf(" ID=0x%08X", node_settings->ID);
		if (node_settings->ParentNodeId)
		{
			buf->appendf(" Parent=0x%08X SizeRef=%d,%d", node_settings->ParentNodeId, node_settings->SizeRef.x, node_settings->SizeRef.y);
		}
		else
		{
			if (node_settings->ParentWindowId)
				buf->appendf(" Window=0x%08X", node_settings->ParentWindowId);
			buf->appendf(" Pos=%d,%d Size=%d,%d", node_settings->Pos.x, node_settings->Pos.y, node_settings->Size.x, node_settings->Size.y);
		}
		if (node_settings->SplitAxis != KGGuiAxis_None)
			buf->appendf(" Split=%c", (node_settings->SplitAxis == KGGuiAxis_X) ? 'X' : 'Y');
		if (node_settings->Flags & KGGuiDockNodeFlags_NoResize)
			buf->appendf(" NoResize=1");
		if (node_settings->Flags & KGGuiDockNodeFlags_CentralNode)
			buf->appendf(" CentralNode=1");
		if (node_settings->Flags & KGGuiDockNodeFlags_NoTabBar)
			buf->appendf(" NoTabBar=1");
		if (node_settings->Flags & KGGuiDockNodeFlags_HiddenTabBar)
			buf->appendf(" HiddenTabBar=1");
		if (node_settings->Flags & KGGuiDockNodeFlags_NoWindowMenuButton)
			buf->appendf(" NoWindowMenuButton=1");
		if (node_settings->Flags & KGGuiDockNodeFlags_NoCloseButton)
			buf->appendf(" NoCloseButton=1");
		if (node_settings->SelectedTabId)
			buf->appendf(" Selected=0x%08X", node_settings->SelectedTabId);

#if KARMAGUI_DEBUG_INI_SETTINGS
		// [DEBUG] Include comments in the .ini file to ease debugging
		if (KGGuiDockNode* node = DockContextFindNodeByID(ctx, node_settings->ID))
		{
			buf->appendf("%*s", KGMax(2, (line_start_pos + 92) - buf->size()), "");     // Align everything
			if (node->IsDockSpace() && node->HostWindow && node->HostWindow->ParentWindow)
				buf->appendf(" ; in '%s'", node->HostWindow->ParentWindow->Name);
			// Iterate settings so we can give info about windows that didn't exist during the session.
			int contains_window = 0;
			for (KGGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
				if (settings->DockId == node_settings->ID)
				{
					if (contains_window++ == 0)
						buf->appendf(" ; contains ");
					buf->appendf("'%s' ", settings->GetName());
				}
		}
#endif
		buf->appendf("\n");
	}
	buf->appendf("\n");
}

//-----------------------------------------------------------------------------
// [SECTION] PLATFORM DEPENDENT HELPERS
//-----------------------------------------------------------------------------

#if defined(KR_CORE_WINDOWS) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS) && !defined(IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS)

#ifdef _MSC_VER
#pragma comment(lib, "user32")
#pragma comment(lib, "kernel32")
#endif

// Win32 clipboard implementation
// We use g.ClipboardHandlerData for temporary storage to ensure it is freed on Shutdown()
static const char* GetClipboardTextFn_DefaultImpl(void*)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.ClipboardHandlerData.clear();
	if (!::OpenClipboard(NULL))
		return NULL;
	HANDLE wbuf_handle = ::GetClipboardData(CF_UNICODETEXT);
	if (wbuf_handle == NULL)
	{
		::CloseClipboard();
		return NULL;
	}
	if (const WCHAR* wbuf_global = (const WCHAR*)::GlobalLock(wbuf_handle))
	{
		int buf_len = ::WideCharToMultiByte(CP_UTF8, 0, wbuf_global, -1, NULL, 0, NULL, NULL);
		g.ClipboardHandlerData.resize(buf_len);
		::WideCharToMultiByte(CP_UTF8, 0, wbuf_global, -1, g.ClipboardHandlerData.Data, buf_len, NULL, NULL);
	}
	::GlobalUnlock(wbuf_handle);
	::CloseClipboard();
	return g.ClipboardHandlerData.Data;
}

static void SetClipboardTextFn_DefaultImpl(void*, const char* text)
{
	if (!::OpenClipboard(NULL))
		return;
	const int wbuf_length = ::MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
	HGLOBAL wbuf_handle = ::GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)wbuf_length * sizeof(WCHAR));
	if (wbuf_handle == NULL)
	{
		::CloseClipboard();
		return;
	}
	WCHAR* wbuf_global = (WCHAR*)::GlobalLock(wbuf_handle);
	::MultiByteToWideChar(CP_UTF8, 0, text, -1, wbuf_global, wbuf_length);
	::GlobalUnlock(wbuf_handle);
	::EmptyClipboard();
	if (::SetClipboardData(CF_UNICODETEXT, wbuf_handle) == NULL)
		::GlobalFree(wbuf_handle);
	::CloseClipboard();
}

#elif defined(__APPLE__) && TARGET_OS_OSX && defined(KGGUI_ENABLE_OSX_DEFAULT_CLIPBOARD_FUNCTIONS)

#include <Carbon/Carbon.h>  // Use old API to avoid need for separate .mm file
static PasteboardRef main_clipboard = 0;

// OSX clipboard implementation
// If you enable this you will need to add '-framework ApplicationServices' to your linker command-line!
static void SetClipboardTextFn_DefaultImpl(void*, const char* text)
{
	if (!main_clipboard)
		PasteboardCreate(kPasteboardClipboard, &main_clipboard);
	PasteboardClear(main_clipboard);
	CFDataRef cf_data = CFDataCreate(kCFAllocatorDefault, (const UInt8*)text, strlen(text));
	if (cf_data)
	{
		PasteboardPutItemFlavor(main_clipboard, (PasteboardItemID)1, CFSTR("public.utf8-plain-text"), cf_data, 0);
		CFRelease(cf_data);
	}
}

static const char* GetClipboardTextFn_DefaultImpl(void*)
{
	if (!main_clipboard)
		PasteboardCreate(kPasteboardClipboard, &main_clipboard);
	PasteboardSynchronize(main_clipboard);

	ItemCount item_count = 0;
	PasteboardGetItemCount(main_clipboard, &item_count);
	for (ItemCount i = 0; i < item_count; i++)
	{
		PasteboardItemID item_id = 0;
		PasteboardGetItemIdentifier(main_clipboard, i + 1, &item_id);
		CFArrayRef flavor_type_array = 0;
		PasteboardCopyItemFlavors(main_clipboard, item_id, &flavor_type_array);
		for (CFIndex j = 0, nj = CFArrayGetCount(flavor_type_array); j < nj; j++)
		{
			CFDataRef cf_data;
			if (PasteboardCopyItemFlavorData(main_clipboard, item_id, CFSTR("public.utf8-plain-text"), &cf_data) == noErr)
			{
				KarmaGuiContext& g = *GKarmaGui;
				g.ClipboardHandlerData.clear();
				int length = (int)CFDataGetLength(cf_data);
				g.ClipboardHandlerData.resize(length + 1);
				CFDataGetBytes(cf_data, CFRangeMake(0, length), (UInt8*)g.ClipboardHandlerData.Data);
				g.ClipboardHandlerData[length] = 0;
				CFRelease(cf_data);
				return g.ClipboardHandlerData.Data;
			}
		}
	}
	return NULL;
}

#else

// Local Dear ImGui-only clipboard implementation, if user hasn't defined better clipboard handlers.
const char* Karma::KarmaGuiInternal::GetClipboardTextFn_DefaultImpl(void*)
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.ClipboardHandlerData.empty() ? NULL : g.ClipboardHandlerData.begin();
}

void Karma::KarmaGuiInternal::SetClipboardTextFn_DefaultImpl(void*, const char* text)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.ClipboardHandlerData.clear();
	const char* text_end = text + strlen(text);
	g.ClipboardHandlerData.resize((int)(text_end - text) + 1);
	memcpy(&g.ClipboardHandlerData[0], text, (size_t)(text_end - text));
	g.ClipboardHandlerData[(int)(text_end - text)] = 0;
}

#endif

// Win32 API IME support (for Asian languages, etc.)
#if defined(KR_CORE_WINDOWS) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS) && !defined(KARMAGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS)

#include <imm.h>
#ifdef _MSC_VER
#pragma comment(lib, "imm32")
#endif

static void SetPlatformImeDataFn_DefaultImpl(KarmaGuiViewport* viewport, KarmaGuiPlatformImeData* data)
{
	// Notify OS Input Method Editor of text input position
	HWND hwnd = (HWND)viewport->PlatformHandleRaw;
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
	if (hwnd == 0)
		hwnd = (HWND)KarmaGui::GetIO().ImeWindowHandle;
#endif
	if (hwnd == 0)
		return;

	//::ImmAssociateContextEx(hwnd, NULL, data->WantVisible ? IACE_DEFAULT : 0);
	if (HIMC himc = ::ImmGetContext(hwnd))
	{
		COMPOSITIONFORM composition_form = {};
		composition_form.ptCurrentPos.x = (LONG)(data->InputPos.x - viewport->Pos.x);
		composition_form.ptCurrentPos.y = (LONG)(data->InputPos.y - viewport->Pos.y);
		composition_form.dwStyle = CFS_FORCE_POSITION;
		::ImmSetCompositionWindow(himc, &composition_form);
		CANDIDATEFORM candidate_form = {};
		candidate_form.dwStyle = CFS_CANDIDATEPOS;
		candidate_form.ptCurrentPos.x = (LONG)(data->InputPos.x - viewport->Pos.x);
		candidate_form.ptCurrentPos.y = (LONG)(data->InputPos.y - viewport->Pos.y);
		::ImmSetCandidateWindow(himc, &candidate_form);
		::ImmReleaseContext(hwnd, himc);
	}
}

#else

void Karma::KarmaGuiInternal::SetPlatformImeDataFn_DefaultImpl(KarmaGuiViewport*, KarmaGuiPlatformImeData*)
{
}

#endif

//-----------------------------------------------------------------------------
// [SECTION] METRICS/DEBUGGER WINDOW
//-----------------------------------------------------------------------------
// - RenderViewportThumbnail() [Internal]
// - RenderViewportsThumbnails() [Internal]
// - DebugTextEncoding()
// - MetricsHelpMarker() [Internal]
// - ShowFontAtlas() [Internal]
// - ShowMetricsWindow()
// - DebugNodeColumns() [Internal]
// - DebugNodeDockNode() [Internal]
// - DebugNodeDrawList() [Internal]
// - DebugNodeDrawCmdShowMeshAndBoundingBox() [Internal]
// - DebugNodeFont() [Internal]
// - DebugNodeFontGlyph() [Internal]
// - DebugNodeStorage() [Internal]
// - DebugNodeTabBar() [Internal]
// - DebugNodeViewport() [Internal]
// - DebugNodeWindow() [Internal]
// - DebugNodeWindowSettings() [Internal]
// - DebugNodeWindowsList() [Internal]
// - DebugNodeWindowsListByBeginStackParent() [Internal]
//-----------------------------------------------------------------------------

#ifndef KARMAGUI_DISABLE_DEBUG_TOOLS

void DebugRenderViewportThumbnail(KGDrawList* draw_list, KGGuiViewportP* viewport, const KGRect& bb)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;

	KGVec2 scale = bb.GetSize() / viewport->Size;
	KGVec2 off = bb.Min - viewport->Pos * scale;
	float alpha_mul = (viewport->Flags & KGGuiViewportFlags_Minimized) ? 0.30f : 1.00f;
	window->DrawList->AddRectFilled(bb.Min, bb.Max, Karma::KarmaGui::GetColorU32(KGGuiCol_Border, alpha_mul * 0.40f));
	for (int i = 0; i != g.Windows.Size; i++)
	{
		KGGuiWindow* thumb_window = g.Windows[i];
		if (!thumb_window->WasActive || (thumb_window->Flags & KGGuiWindowFlags_ChildWindow))
			continue;
		if (thumb_window->Viewport != viewport)
			continue;

		KGRect thumb_r = thumb_window->Rect();
		KGRect title_r = thumb_window->TitleBarRect();
		thumb_r = KGRect(KGFloor(off + thumb_r.Min * scale), KGFloor(off + thumb_r.Max * scale));
		title_r = KGRect(KGFloor(off + title_r.Min * scale), KGFloor(off + KGVec2(title_r.Max.x, title_r.Min.y) * scale) + KGVec2(0, 5)); // Exaggerate title bar height
		thumb_r.ClipWithFull(bb);
		title_r.ClipWithFull(bb);
		const bool window_is_focused = (g.NavWindow && thumb_window->RootWindowForTitleBarHighlight == g.NavWindow->RootWindowForTitleBarHighlight);
		window->DrawList->AddRectFilled(thumb_r.Min, thumb_r.Max, Karma::KarmaGui::GetColorU32(KGGuiCol_WindowBg, alpha_mul));
		window->DrawList->AddRectFilled(title_r.Min, title_r.Max, Karma::KarmaGui::GetColorU32(window_is_focused ? KGGuiCol_TitleBgActive : KGGuiCol_TitleBg, alpha_mul));
		window->DrawList->AddRect(thumb_r.Min, thumb_r.Max, Karma::KarmaGui::GetColorU32(KGGuiCol_Border, alpha_mul));
		window->DrawList->AddText(g.Font, g.FontSize * 1.0f, title_r.Min, Karma::KarmaGui::GetColorU32(KGGuiCol_Text, alpha_mul), thumb_window->Name, Karma::KarmaGuiInternal::FindRenderedTextEnd(thumb_window->Name));
	}
	draw_list->AddRect(bb.Min, bb.Max, Karma::KarmaGui::GetColorU32(KGGuiCol_Border, alpha_mul));
}

static void RenderViewportsThumbnails()
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;

	// We don't display full monitor bounds (we could, but it often looks awkward), instead we display just enough to cover all of our viewports.
	float SCALE = 1.0f / 8.0f;
	KGRect bb_full(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (int n = 0; n < g.Viewports.Size; n++)
		bb_full.Add(g.Viewports[n]->GetMainRect());
	KGVec2 p = window->DC.CursorPos;
	KGVec2 off = p - bb_full.Min * SCALE;
	for (int n = 0; n < g.Viewports.Size; n++)
	{
		KGGuiViewportP* viewport = g.Viewports[n];
		KGRect viewport_draw_bb(off + (viewport->Pos) * SCALE, off + (viewport->Pos + viewport->Size) * SCALE);
		DebugRenderViewportThumbnail(window->DrawList, viewport, viewport_draw_bb);
	}
	Karma::KarmaGui::Dummy(bb_full.GetSize() * SCALE);
}

static int ViewportComparerByFrontMostStampCount(const void* lhs, const void* rhs)
{
	const KGGuiViewportP* a = *(const KGGuiViewportP* const*)lhs;
	const KGGuiViewportP* b = *(const KGGuiViewportP* const*)rhs;
	return b->LastFrontMostStampCount - a->LastFrontMostStampCount;
}

// Draw an arbitrary US keyboard layout to visualize translated keys
void DebugRenderKeyboardPreview(KGDrawList* draw_list)
{
	const KGVec2 key_size = KGVec2(35.0f, 35.0f);
	const float  key_rounding = 3.0f;
	const KGVec2 key_face_size = KGVec2(25.0f, 25.0f);
	const KGVec2 key_face_pos = KGVec2(5.0f, 3.0f);
	const float  key_face_rounding = 2.0f;
	const KGVec2 key_label_pos = KGVec2(7.0f, 4.0f);
	const KGVec2 key_step = KGVec2(key_size.x - 1.0f, key_size.y - 1.0f);
	const float  key_row_offset = 9.0f;

	KGVec2 board_min = Karma::KarmaGui::GetCursorScreenPos();
	KGVec2 board_max = KGVec2(board_min.x + 3 * key_step.x + 2 * key_row_offset + 10.0f, board_min.y + 3 * key_step.y + 10.0f);
	KGVec2 start_pos = KGVec2(board_min.x + 5.0f - key_step.x, board_min.y);

	struct KeyLayoutData { int Row, Col; const char* Label; KarmaGuiKey Key; };
	const KeyLayoutData keys_to_display[] =
	{
		{ 0, 0, "", KGGuiKey_Tab },      { 0, 1, "Q", KGGuiKey_Q }, { 0, 2, "W", KGGuiKey_W }, { 0, 3, "E", KGGuiKey_E }, { 0, 4, "R", KGGuiKey_R },
		{ 1, 0, "", KGGuiKey_CapsLock }, { 1, 1, "A", KGGuiKey_A }, { 1, 2, "S", KGGuiKey_S }, { 1, 3, "D", KGGuiKey_D }, { 1, 4, "F", KGGuiKey_F },
		{ 2, 0, "", KGGuiKey_LeftShift },{ 2, 1, "Z", KGGuiKey_Z }, { 2, 2, "X", KGGuiKey_X }, { 2, 3, "C", KGGuiKey_C }, { 2, 4, "V", KGGuiKey_V }
	};

	// Elements rendered manually via KGDrawList API are not clipped automatically.
	// While not strictly necessary, here IsItemVisible() is used to avoid rendering these shapes when they are out of view.
	Karma::KarmaGui::Dummy(board_max - board_min);
	if (!Karma::KarmaGui::IsItemVisible())
		return;
	draw_list->PushClipRect(board_min, board_max, true);
	for (int n = 0; n < KG_ARRAYSIZE(keys_to_display); n++)
	{
		const KeyLayoutData* key_data = &keys_to_display[n];
		KGVec2 key_min = KGVec2(start_pos.x + key_data->Col * key_step.x + key_data->Row * key_row_offset, start_pos.y + key_data->Row * key_step.y);
		KGVec2 key_max = key_min + key_size;
		draw_list->AddRectFilled(key_min, key_max, KG_COL32(204, 204, 204, 255), key_rounding);
		draw_list->AddRect(key_min, key_max, KG_COL32(24, 24, 24, 255), key_rounding);
		KGVec2 face_min = KGVec2(key_min.x + key_face_pos.x, key_min.y + key_face_pos.y);
		KGVec2 face_max = KGVec2(face_min.x + key_face_size.x, face_min.y + key_face_size.y);
		draw_list->AddRect(face_min, face_max, KG_COL32(193, 193, 193, 255), key_face_rounding, KGDrawFlags_None, 2.0f);
		draw_list->AddRectFilled(face_min, face_max, KG_COL32(252, 252, 252, 255), key_face_rounding);
		KGVec2 label_min = KGVec2(key_min.x + key_label_pos.x, key_min.y + key_label_pos.y);
		draw_list->AddText(label_min, KG_COL32(64, 64, 64, 255), key_data->Label);
		if (Karma::KarmaGui::IsKeyDown(key_data->Key))
			draw_list->AddRectFilled(key_min, key_max, KG_COL32(255, 0, 0, 128), key_rounding);
	}
	draw_list->PopClipRect();
}

// Helper tool to diagnose between text encoding issues and font loading issues. Pass your UTF-8 string and verify that there are correct.
void Karma::KarmaGui::DebugTextEncoding(const char* str)
{
	Text("Text: \"%s\"", str);
	if (!BeginTable("list", 4, KGGuiTableFlags_Borders | KGGuiTableFlags_RowBg | KGGuiTableFlags_SizingFixedFit))
		return;
	TableSetupColumn("Offset");
	TableSetupColumn("UTF-8");
	TableSetupColumn("Glyph");
	TableSetupColumn("Codepoint");
	TableHeadersRow();
	for (const char* p = str; *p != 0; )
	{
		unsigned int c;
		const int c_utf8_len = KGTextCharFromUtf8(&c, p, NULL);
		TableNextColumn();
		Text("%d", (int)(p - str));
		TableNextColumn();
		for (int byte_index = 0; byte_index < c_utf8_len; byte_index++)
		{
			if (byte_index > 0)
				SameLine();
			Text("0x%02X", (int)(unsigned char)p[byte_index]);
		}
		TableNextColumn();
		if (GetFont()->FindGlyphNoFallback((KGWchar)c))
			TextUnformatted(p, p + c_utf8_len);
		else
			TextUnformatted((c == KG_UNICODE_CODEPOINT_INVALID) ? "[invalid]" : "[missing]");
		TableNextColumn();
		Text("U+%04X", (int)c);
		p += c_utf8_len;
	}
	EndTable();
}

// Avoid naming collision with imgui_demo.cpp's HelpMarker() for unity builds.
static void MetricsHelpMarker(const char* desc)
{
	Karma::KarmaGui::TextDisabled("(?)");
	if (Karma::KarmaGui::IsItemHovered(KGGuiHoveredFlags_DelayShort))
	{
		Karma::KarmaGui::BeginTooltip();
		Karma::KarmaGui::PushTextWrapPos(Karma::KarmaGui::GetFontSize() * 35.0f);
		Karma::KarmaGui::TextUnformatted(desc);
		Karma::KarmaGui::PopTextWrapPos();
		Karma::KarmaGui::EndTooltip();
	}
}

// [DEBUG] List fonts in a font atlas and display its texture
void ShowFontAtlas(KGFontAtlas* atlas)
{
	for (int i = 0; i < atlas->Fonts.Size; i++)
	{
		KGFont* font = atlas->Fonts[i];
		Karma::KarmaGui::PushID(font);
		Karma::KarmaGuiInternal::DebugNodeFont(font);
		Karma::KarmaGui::PopID();
	}
	if (Karma::KarmaGui::TreeNode("Atlas texture", "Atlas texture (%dx%d pixels)", atlas->TexWidth, atlas->TexHeight))
	{
		KGVec4 tint_col = KGVec4(1.0f, 1.0f, 1.0f, 1.0f);
		KGVec4 border_col = KGVec4(1.0f, 1.0f, 1.0f, 0.5f);
		Karma::KarmaGui::Image(atlas->TexID, KGVec2((float)atlas->TexWidth, (float)atlas->TexHeight), KGVec2(0.0f, 0.0f), KGVec2(1.0f, 1.0f), tint_col, border_col);
		Karma::KarmaGui::TreePop();
	}
}

void Karma::KarmaGui::ShowMetricsWindow(bool* p_open)
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiIO& io = g.IO;
	KGGuiMetricsConfig* cfg = &g.DebugMetricsConfig;
	if (cfg->ShowDebugLog)
		ShowDebugLogWindow(&cfg->ShowDebugLog);
	if (cfg->ShowStackTool)
		ShowStackToolWindow(&cfg->ShowStackTool);

	if (!Begin("Dear ImGui Metrics/Debugger", p_open) || KarmaGuiInternal::GetCurrentWindow()->BeginCount > 1)
	{
		End();
		return;
	}

	// Basic info
	Text("KarmaGui");
	Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	Text("%d vertices, %d indices (%d triangles)", io.MetricsRenderVertices, io.MetricsRenderIndices, io.MetricsRenderIndices / 3);
	Text("%d visible windows, %d active allocations", io.MetricsRenderWindows, io.MetricsActiveAllocations);
	//SameLine(); if (SmallButton("GC")) { g.GcCompactAll = true; }

	Separator();

	// Debugging enums
	enum { WRT_OuterRect, WRT_OuterRectClipped, WRT_InnerRect, WRT_InnerClipRect, WRT_WorkRect, WRT_Content, WRT_ContentIdeal, WRT_ContentRegionRect, WRT_Count }; // Windows Rect Type
	const char* wrt_rects_names[WRT_Count] = { "OuterRect", "OuterRectClipped", "InnerRect", "InnerClipRect", "WorkRect", "Content", "ContentIdeal", "ContentRegionRect" };
	enum { TRT_OuterRect, TRT_InnerRect, TRT_WorkRect, TRT_HostClipRect, TRT_InnerClipRect, TRT_BackgroundClipRect, TRT_ColumnsRect, TRT_ColumnsWorkRect, TRT_ColumnsClipRect, TRT_ColumnsContentHeadersUsed, TRT_ColumnsContentHeadersIdeal, TRT_ColumnsContentFrozen, TRT_ColumnsContentUnfrozen, TRT_Count }; // Tables Rect Type
	const char* trt_rects_names[TRT_Count] = { "OuterRect", "InnerRect", "WorkRect", "HostClipRect", "InnerClipRect", "BackgroundClipRect", "ColumnsRect", "ColumnsWorkRect", "ColumnsClipRect", "ColumnsContentHeadersUsed", "ColumnsContentHeadersIdeal", "ColumnsContentFrozen", "ColumnsContentUnfrozen" };
	if (cfg->ShowWindowsRectsType < 0)
		cfg->ShowWindowsRectsType = WRT_WorkRect;
	if (cfg->ShowTablesRectsType < 0)
		cfg->ShowTablesRectsType = TRT_WorkRect;

	struct Funcs
	{
		static KGRect GetTableRect(KGGuiTable* table, int rect_type, int n)
		{
			KGGuiTableInstanceData* table_instance = KarmaGuiInternal::TableGetInstanceData(table, table->InstanceCurrent); // Always using last submitted instance
			if (rect_type == TRT_OuterRect) { return table->OuterRect; }
			else if (rect_type == TRT_InnerRect) { return table->InnerRect; }
			else if (rect_type == TRT_WorkRect) { return table->WorkRect; }
			else if (rect_type == TRT_HostClipRect) { return table->HostClipRect; }
			else if (rect_type == TRT_InnerClipRect) { return table->InnerClipRect; }
			else if (rect_type == TRT_BackgroundClipRect) { return table->BgClipRect; }
			else if (rect_type == TRT_ColumnsRect) { KGGuiTableColumn* c = &table->Columns[n]; return KGRect(c->MinX, table->InnerClipRect.Min.y, c->MaxX, table->InnerClipRect.Min.y + table_instance->LastOuterHeight); }
			else if (rect_type == TRT_ColumnsWorkRect) { KGGuiTableColumn* c = &table->Columns[n]; return KGRect(c->WorkMinX, table->WorkRect.Min.y, c->WorkMaxX, table->WorkRect.Max.y); }
			else if (rect_type == TRT_ColumnsClipRect) { KGGuiTableColumn* c = &table->Columns[n]; return c->ClipRect; }
			else if (rect_type == TRT_ColumnsContentHeadersUsed) { KGGuiTableColumn* c = &table->Columns[n]; return KGRect(c->WorkMinX, table->InnerClipRect.Min.y, c->ContentMaxXHeadersUsed, table->InnerClipRect.Min.y + table_instance->LastFirstRowHeight); } // Note: y1/y2 not always accurate
			else if (rect_type == TRT_ColumnsContentHeadersIdeal) { KGGuiTableColumn* c = &table->Columns[n]; return KGRect(c->WorkMinX, table->InnerClipRect.Min.y, c->ContentMaxXHeadersIdeal, table->InnerClipRect.Min.y + table_instance->LastFirstRowHeight); }
			else if (rect_type == TRT_ColumnsContentFrozen) { KGGuiTableColumn* c = &table->Columns[n]; return KGRect(c->WorkMinX, table->InnerClipRect.Min.y, c->ContentMaxXFrozen, table->InnerClipRect.Min.y + table_instance->LastFrozenHeight); }
			else if (rect_type == TRT_ColumnsContentUnfrozen) { KGGuiTableColumn* c = &table->Columns[n]; return KGRect(c->WorkMinX, table->InnerClipRect.Min.y + table_instance->LastFrozenHeight, c->ContentMaxXUnfrozen, table->InnerClipRect.Max.y); }
			KR_CORE_ASSERT(0, "");
			return KGRect();
		}

		static KGRect GetWindowRect(KGGuiWindow* window, int rect_type)
		{
			if (rect_type == WRT_OuterRect) { return window->Rect(); }
			else if (rect_type == WRT_OuterRectClipped) { return window->OuterRectClipped; }
			else if (rect_type == WRT_InnerRect) { return window->InnerRect; }
			else if (rect_type == WRT_InnerClipRect) { return window->InnerClipRect; }
			else if (rect_type == WRT_WorkRect) { return window->WorkRect; }
			else if (rect_type == WRT_Content) { KGVec2 min = window->InnerRect.Min - window->Scroll + window->WindowPadding; return KGRect(min, min + window->ContentSize); }
			else if (rect_type == WRT_ContentIdeal) { KGVec2 min = window->InnerRect.Min - window->Scroll + window->WindowPadding; return KGRect(min, min + window->ContentSizeIdeal); }
			else if (rect_type == WRT_ContentRegionRect) { return window->ContentRegionRect; }
			KR_CORE_ASSERT(0, "");
			return KGRect();
		}
	};

	// Tools
	if (TreeNode("Tools"))
	{
		bool show_encoding_viewer = TreeNode("UTF-8 Encoding viewer");
		SameLine();
		MetricsHelpMarker("You can also call KarmaGui::DebugTextEncoding() from your code with a given string to test that your UTF-8 encoding settings are correct.");
		if (show_encoding_viewer)
		{
			static char buf[100] = "";
			SetNextItemWidth(-FLT_MIN);
			InputText("##Text", buf, KG_ARRAYSIZE(buf));
			if (buf[0] != 0)
				DebugTextEncoding(buf);
			TreePop();
		}

		// The Item Picker tool is super useful to visually select an item and break into the call-stack of where it was submitted.
		if (Checkbox("Show Item Picker", &g.DebugItemPickerActive) && g.DebugItemPickerActive)
			KarmaGuiInternal::DebugStartItemPicker();
		SameLine();
		MetricsHelpMarker("Will call the KR_CORE_ASSERT() macro to break in debugger.\nWarning: If you don't have a debugger attached, this will probably crash.");

		// Stack Tool is your best friend!
		Checkbox("Show Debug Log", &cfg->ShowDebugLog);
		SameLine();
		MetricsHelpMarker("You can also call KarmaGui::ShowDebugLogWindow() from your code.");

		// Stack Tool is your best friend!
		Checkbox("Show Stack Tool", &cfg->ShowStackTool);
		SameLine();
		MetricsHelpMarker("You can also call KarmaGui::ShowStackToolWindow() from your code.");

		Checkbox("Show windows begin order", &cfg->ShowWindowsBeginOrder);
		Checkbox("Show windows rectangles", &cfg->ShowWindowsRects);
		SameLine();
		SetNextItemWidth(GetFontSize() * 12);
		cfg->ShowWindowsRects |= Combo("##show_windows_rect_type", &cfg->ShowWindowsRectsType, wrt_rects_names, WRT_Count, WRT_Count);
		if (cfg->ShowWindowsRects && g.NavWindow != NULL)
		{
			BulletText("'%s':", g.NavWindow->Name);
			Indent();
			for (int rect_n = 0; rect_n < WRT_Count; rect_n++)
			{
				KGRect r = Funcs::GetWindowRect(g.NavWindow, rect_n);
				Text("(%6.1f,%6.1f) (%6.1f,%6.1f) Size (%6.1f,%6.1f) %s", r.Min.x, r.Min.y, r.Max.x, r.Max.y, r.GetWidth(), r.GetHeight(), wrt_rects_names[rect_n]);
			}
			Unindent();
		}

		Checkbox("Show tables rectangles", &cfg->ShowTablesRects);
		SameLine();
		SetNextItemWidth(GetFontSize() * 12);
		cfg->ShowTablesRects |= Combo("##show_table_rects_type", &cfg->ShowTablesRectsType, trt_rects_names, TRT_Count, TRT_Count);
		if (cfg->ShowTablesRects && g.NavWindow != NULL)
		{
			for (int table_n = 0; table_n < g.Tables.GetMapSize(); table_n++)
			{
				KGGuiTable* table = g.Tables.TryGetMapData(table_n);
				if (table == NULL || table->LastFrameActive < g.FrameCount - 1 || (table->OuterWindow != g.NavWindow && table->InnerWindow != g.NavWindow))
					continue;

				BulletText("Table 0x%08X (%d columns, in '%s')", table->ID, table->ColumnsCount, table->OuterWindow->Name);
				if (IsItemHovered())
					GetForegroundDrawList()->AddRect(table->OuterRect.Min - KGVec2(1, 1), table->OuterRect.Max + KGVec2(1, 1), KG_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
				Indent();
				char buf[128];
				for (int rect_n = 0; rect_n < TRT_Count; rect_n++)
				{
					if (rect_n >= TRT_ColumnsRect)
					{
						if (rect_n != TRT_ColumnsRect && rect_n != TRT_ColumnsClipRect)
							continue;
						for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
						{
							KGRect r = Funcs::GetTableRect(table, rect_n, column_n);
							KGFormatString(buf, KG_ARRAYSIZE(buf), "(%6.1f,%6.1f) (%6.1f,%6.1f) Size (%6.1f,%6.1f) Col %d %s", r.Min.x, r.Min.y, r.Max.x, r.Max.y, r.GetWidth(), r.GetHeight(), column_n, trt_rects_names[rect_n]);
							Selectable(buf);
							if (IsItemHovered())
								GetForegroundDrawList()->AddRect(r.Min - KGVec2(1, 1), r.Max + KGVec2(1, 1), KG_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
						}
					}
					else
					{
						KGRect r = Funcs::GetTableRect(table, rect_n, -1);
						KGFormatString(buf, KG_ARRAYSIZE(buf), "(%6.1f,%6.1f) (%6.1f,%6.1f) Size (%6.1f,%6.1f) %s", r.Min.x, r.Min.y, r.Max.x, r.Max.y, r.GetWidth(), r.GetHeight(), trt_rects_names[rect_n]);
						Selectable(buf);
						if (IsItemHovered())
							GetForegroundDrawList()->AddRect(r.Min - KGVec2(1, 1), r.Max + KGVec2(1, 1), KG_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
					}
				}
				Unindent();
			}
		}

		TreePop();
	}

	// Windows
	if (TreeNode("Windows", "Windows (%d)", g.Windows.Size))
	{
		//SetNextItemOpen(true, KGGuiCond_Once);
		KarmaGuiInternal::DebugNodeWindowsList(&g.Windows, "By display order");
		KarmaGuiInternal::DebugNodeWindowsList(&g.WindowsFocusOrder, "By focus order (root windows)");
		if (TreeNode("By submission order (begin stack)"))
		{
			// Here we display windows in their submitted order/hierarchy, however note that the Begin stack doesn't constitute a Parent<>Child relationship!
			KGVector<KGGuiWindow*>& temp_buffer = g.WindowsTempSortBuffer;
			temp_buffer.resize(0);
			for (int i = 0; i < g.Windows.Size; i++)
				if (g.Windows[i]->LastFrameActive + 1 >= g.FrameCount)
					temp_buffer.push_back(g.Windows[i]);

			struct Func
			{
				static int WindowComparerByBeginOrder(const void* lhs, const void* rhs)
				{
					return ((int)(*(const KGGuiWindow* const*)lhs)->BeginOrderWithinContext - (*(const KGGuiWindow* const*)rhs)->BeginOrderWithinContext);
				}
			};

			KGQsort(temp_buffer.Data, (size_t)temp_buffer.Size, sizeof(KGGuiWindow*), Func::WindowComparerByBeginOrder);

			KarmaGuiInternal::DebugNodeWindowsListByBeginStackParent(temp_buffer.Data, temp_buffer.Size, NULL);
			TreePop();
		}

		TreePop();
	}

	// DrawLists
	int drawlist_count = 0;
	for (int viewport_i = 0; viewport_i < g.Viewports.Size; viewport_i++)
		drawlist_count += g.Viewports[viewport_i]->DrawDataBuilder.GetDrawListCount();
	if (TreeNode("DrawLists", "DrawLists (%d)", drawlist_count))
	{
		Checkbox("Show KGDrawCmd mesh when hovering", &cfg->ShowDrawCmdMesh);
		Checkbox("Show KGDrawCmd bounding boxes when hovering", &cfg->ShowDrawCmdBoundingBoxes);
		for (int viewport_i = 0; viewport_i < g.Viewports.Size; viewport_i++)
		{
			KGGuiViewportP* viewport = g.Viewports[viewport_i];
			bool viewport_has_drawlist = false;
			for (int layer_i = 0; layer_i < KG_ARRAYSIZE(viewport->DrawDataBuilder.Layers); layer_i++)
				for (int draw_list_i = 0; draw_list_i < viewport->DrawDataBuilder.Layers[layer_i].Size; draw_list_i++)
				{
					if (!viewport_has_drawlist)
						Text("Active DrawLists in Viewport #%d, ID: 0x%08X", viewport->Idx, viewport->ID);
					viewport_has_drawlist = true;
					KarmaGuiInternal::DebugNodeDrawList(NULL, viewport, viewport->DrawDataBuilder.Layers[layer_i][draw_list_i], "DrawList");
				}
		}
		TreePop();
	}

	// Viewports
	if (TreeNode("Viewports", "Viewports (%d)", g.Viewports.Size))
	{
		Indent(GetTreeNodeToLabelSpacing());
		RenderViewportsThumbnails();
		Unindent(GetTreeNodeToLabelSpacing());

		bool open = TreeNode("Monitors", "Monitors (%d)", g.PlatformIO.Monitors.Size);
		SameLine();
		MetricsHelpMarker("Dear ImGui uses monitor data:\n- to query DPI settings on a per monitor basis\n- to position popup/tooltips so they don't straddle monitors.");
		if (open)
		{
			for (int i = 0; i < g.PlatformIO.Monitors.Size; i++)
			{
				const KarmaGuiPlatformMonitor& mon = g.PlatformIO.Monitors[i];
				BulletText("Monitor #%d: DPI %.0f%%\n MainMin (%.0f,%.0f), MainMax (%.0f,%.0f), MainSize (%.0f,%.0f)\n WorkMin (%.0f,%.0f), WorkMax (%.0f,%.0f), WorkSize (%.0f,%.0f)",
					i, mon.DpiScale * 100.0f,
					mon.MainPos.x, mon.MainPos.y, mon.MainPos.x + mon.MainSize.x, mon.MainPos.y + mon.MainSize.y, mon.MainSize.x, mon.MainSize.y,
					mon.WorkPos.x, mon.WorkPos.y, mon.WorkPos.x + mon.WorkSize.x, mon.WorkPos.y + mon.WorkSize.y, mon.WorkSize.x, mon.WorkSize.y);
			}
			TreePop();
		}

		BulletText("MouseViewport: 0x%08X (UserHovered 0x%08X, LastHovered 0x%08X)", g.MouseViewport ? g.MouseViewport->ID : 0, g.IO.MouseHoveredViewport, g.MouseLastHoveredViewport ? g.MouseLastHoveredViewport->ID : 0);
		if (TreeNode("Inferred Z order (front-to-back)"))
		{
			static KGVector<KGGuiViewportP*> viewports;
			viewports.resize(g.Viewports.Size);
			memcpy(viewports.Data, g.Viewports.Data, g.Viewports.size_in_bytes());
			if (viewports.Size > 1)
				KGQsort(viewports.Data, viewports.Size, sizeof(KarmaGuiViewport*), ViewportComparerByFrontMostStampCount);
			for (int i = 0; i < viewports.Size; i++)
				BulletText("Viewport #%d, ID: 0x%08X, FrontMostStampCount = %08d, Window: \"%s\"", viewports[i]->Idx, viewports[i]->ID, viewports[i]->LastFrontMostStampCount, viewports[i]->Window ? viewports[i]->Window->Name : "N/A");
			TreePop();
		}

		for (int i = 0; i < g.Viewports.Size; i++)
			KarmaGuiInternal::DebugNodeViewport(g.Viewports[i]);
		TreePop();
	}

	// Details for Popups
	if (TreeNode("Popups", "Popups (%d)", g.OpenPopupStack.Size))
	{
		for (int i = 0; i < g.OpenPopupStack.Size; i++)
		{
			// As it's difficult to interact with tree nodes while popups are open, we display everything inline.
			const KGGuiPopupData* popup_data = &g.OpenPopupStack[i];
			KGGuiWindow* window = popup_data->Window;
			BulletText("PopupID: %08x, Window: '%s' (%s%s), BackupNavWindow '%s', ParentWindow '%s'",
				popup_data->PopupId, window ? window->Name : "NULL", window && (window->Flags & KGGuiWindowFlags_ChildWindow) ? "Child;" : "", window && (window->Flags & KGGuiWindowFlags_ChildMenu) ? "Menu;" : "",
				popup_data->BackupNavWindow ? popup_data->BackupNavWindow->Name : "NULL", window && window->ParentWindow ? window->ParentWindow->Name : "NULL");
		}
		TreePop();
	}

	// Details for TabBars
	if (TreeNode("TabBars", "Tab Bars (%d)", g.TabBars.GetAliveCount()))
	{
		for (int n = 0; n < g.TabBars.GetMapSize(); n++)
			if (KGGuiTabBar* tab_bar = g.TabBars.TryGetMapData(n))
			{
				PushID(tab_bar);
				KarmaGuiInternal::DebugNodeTabBar(tab_bar, "TabBar");
				PopID();
			}
		TreePop();
	}

	// Details for Tables
	if (TreeNode("Tables", "Tables (%d)", g.Tables.GetAliveCount()))
	{
		for (int n = 0; n < g.Tables.GetMapSize(); n++)
			if (KGGuiTable* table = g.Tables.TryGetMapData(n))
				KarmaGuiInternal::DebugNodeTable(table);
		TreePop();
	}

	// Details for Fonts
	KGFontAtlas* atlas = g.IO.Fonts;
	if (TreeNode("Fonts", "Fonts (%d)", atlas->Fonts.Size))
	{
		ShowFontAtlas(atlas);
		TreePop();
	}

	// Details for InputText
	if (TreeNode("InputText"))
	{
		KarmaGuiInternal::DebugNodeInputTextState(&g.InputTextState);
		TreePop();
	}

	// Details for Docking
#ifdef KARMAGUI_HAS_DOCK
	if (TreeNode("Docking"))
	{
		static bool root_nodes_only = true;
		KGGuiDockContext* dc = &g.DockContext;
		Checkbox("List root nodes", &root_nodes_only);
		Checkbox("Ctrl shows window dock info", &cfg->ShowDockingNodes);
		if (SmallButton("Clear nodes")) { KarmaGuiInternal::DockContextClearNodes(&g, 0, true); }
		SameLine();
		if (SmallButton("Rebuild all")) { dc->WantFullRebuild = true; }
		for (int n = 0; n < dc->Nodes.Data.Size; n++)
			if (KGGuiDockNode* node = (KGGuiDockNode*)dc->Nodes.Data[n].val_p)
				if (!root_nodes_only || node->IsRootNode())
					KarmaGuiInternal::DebugNodeDockNode(node, "Node");
		TreePop();
	}
#endif // #ifdef KARMAGUI_HAS_DOCK

	// Settings
	if (TreeNode("Settings"))
	{
		if (SmallButton("Clear"))
			KarmaGuiInternal::ClearIniSettings();
		SameLine();
		if (SmallButton("Save to memory"))
			SaveIniSettingsToMemory();
		SameLine();
		if (SmallButton("Save to disk"))
			SaveIniSettingsToDisk(g.IO.IniFilename);
		SameLine();
		if (g.IO.IniFilename)
			Text("\"%s\"", g.IO.IniFilename);
		else
			TextUnformatted("<NULL>");
		Text("SettingsDirtyTimer %.2f", g.SettingsDirtyTimer);
		if (TreeNode("SettingsHandlers", "Settings handlers: (%d)", g.SettingsHandlers.Size))
		{
			for (int n = 0; n < g.SettingsHandlers.Size; n++)
				BulletText("%s", g.SettingsHandlers[n].TypeName);
			TreePop();
		}
		if (TreeNode("SettingsWindows", "Settings packed data: Windows: %d bytes", g.SettingsWindows.size()))
		{
			for (KGGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
				KarmaGuiInternal::DebugNodeWindowSettings(settings);
			TreePop();
		}

		if (TreeNode("SettingsTables", "Settings packed data: Tables: %d bytes", g.SettingsTables.size()))
		{
			for (KGGuiTableSettings* settings = g.SettingsTables.begin(); settings != NULL; settings = g.SettingsTables.next_chunk(settings))
				KarmaGuiInternal::DebugNodeTableSettings(settings);
			TreePop();
		}

#ifdef KARMAGUI_HAS_DOCK
		if (TreeNode("SettingsDocking", "Settings packed data: Docking"))
		{
			KGGuiDockContext* dc = &g.DockContext;
			Text("In SettingsWindows:");
			for (KGGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
				if (settings->DockId != 0)
					BulletText("Window '%s' -> DockId %08X", settings->GetName(), settings->DockId);
			Text("In SettingsNodes:");
			for (int n = 0; n < dc->NodesSettings.Size; n++)
			{
				KGGuiDockNodeSettings* settings = &dc->NodesSettings[n];
				const char* selected_tab_name = NULL;
				if (settings->SelectedTabId)
				{
					if (KGGuiWindow* window = KarmaGuiInternal::FindWindowByID(settings->SelectedTabId))
						selected_tab_name = window->Name;
					else if (KGGuiWindowSettings* window_settings = KarmaGuiInternal::FindWindowSettings(settings->SelectedTabId))
						selected_tab_name = window_settings->GetName();
				}
				BulletText("Node %08X, Parent %08X, SelectedTab %08X ('%s')", settings->ID, settings->ParentNodeId, settings->SelectedTabId, selected_tab_name ? selected_tab_name : settings->SelectedTabId ? "N/A" : "");
			}
			TreePop();
		}
#endif // #ifdef KARMAGUI_HAS_DOCK

		if (TreeNode("SettingsIniData", "Settings unpacked data (.ini): %d bytes", g.SettingsIniData.size()))
		{
			InputTextMultiline("##Ini", (char*)(void*)g.SettingsIniData.c_str(), g.SettingsIniData.Buf.Size, KGVec2(-FLT_MIN, GetTextLineHeight() * 20), KGGuiInputTextFlags_ReadOnly);
			TreePop();
		}
		TreePop();
	}

	if (TreeNode("Inputs"))
	{
		Text("KEYBOARD/GAMEPAD/MOUSE KEYS");
		{
			// We iterate both legacy native range and named KarmaGuiKey ranges, which is a little odd but this allows displaying the data for old/new backends.
			// User code should never have to go through such hoops: old code may use native keycodes, new code may use KarmaGuiKey codes.
			Indent();
#ifdef IMGUI_DISABLE_OBSOLETE_KEYIO
			struct funcs { static bool IsLegacyNativeDupe(KarmaGuiKey) { return false; } };
#else
			struct funcs { static bool IsLegacyNativeDupe(KarmaGuiKey key) { return key < 512 && GetIO().KeyMap[key] != -1; } }; // Hide Native<>KarmaGuiKey duplicates when both exists in the array
			//Text("Legacy raw:");      for (KarmaGuiKey key = KGGuiKey_KeysData_OFFSET; key < KGGuiKey_COUNT; key++) { if (io.KeysDown[key]) { SameLine(); Text("\"%s\" %d", GetKeyName(key), key); } }
#endif
			Text("Keys down:");         for (KarmaGuiKey key = KGGuiKey_KeysData_OFFSET; key < KGGuiKey_COUNT; key = (KarmaGuiKey)(key + 1)) { if (funcs::IsLegacyNativeDupe(key) || !IsKeyDown(key)) continue;     SameLine(); Text(KarmaGuiInternal::IsNamedKey(key) ? "\"%s\"" : "\"%s\" %d", GetKeyName(key), key); SameLine(); Text("(%.02f)", KarmaGuiInternal::GetKeyData(key)->DownDuration); }
			Text("Keys pressed:");      for (KarmaGuiKey key = KGGuiKey_KeysData_OFFSET; key < KGGuiKey_COUNT; key = (KarmaGuiKey)(key + 1)) { if (funcs::IsLegacyNativeDupe(key) || !IsKeyPressed(key)) continue;  SameLine(); Text(KarmaGuiInternal::IsNamedKey(key) ? "\"%s\"" : "\"%s\" %d", GetKeyName(key), key); }
			Text("Keys released:");     for (KarmaGuiKey key = KGGuiKey_KeysData_OFFSET; key < KGGuiKey_COUNT; key = (KarmaGuiKey)(key + 1)) { if (funcs::IsLegacyNativeDupe(key) || !IsKeyReleased(key)) continue; SameLine(); Text(KarmaGuiInternal::IsNamedKey(key) ? "\"%s\"" : "\"%s\" %d", GetKeyName(key), key); }
			Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "CTRL " : "", io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "", io.KeySuper ? "SUPER " : "");
			Text("Chars queue:");       for (int i = 0; i < io.InputQueueCharacters.Size; i++) { KGWchar c = io.InputQueueCharacters[i]; SameLine(); Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c); } // FIXME: We should convert 'c' to UTF-8 here but the functions are not public.
			DebugRenderKeyboardPreview(GetWindowDrawList());
			Unindent();
		}

		Text("MOUSE STATE");
		{
			Indent();
			if (IsMousePosValid())
				Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
			else
				Text("Mouse pos: <INVALID>");
			Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
			int count = KG_ARRAYSIZE(io.MouseDown);
			Text("Mouse down:");     for (int i = 0; i < count; i++) if (IsMouseDown(i)) { SameLine(); Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]); }
			Text("Mouse clicked:");  for (int i = 0; i < count; i++) if (IsMouseClicked(i)) { SameLine(); Text("b%d (%d)", i, io.MouseClickedCount[i]); }
			Text("Mouse released:"); for (int i = 0; i < count; i++) if (IsMouseReleased(i)) { SameLine(); Text("b%d", i); }
			Text("Mouse wheel: %.1f", io.MouseWheel);
			Text("Pen Pressure: %.1f", io.PenPressure); // Note: currently unused
			Unindent();
		}

		Text("MOUSE WHEELING");
		{
			Indent();
			Text("WheelingWindow: '%s'", g.WheelingWindow ? g.WheelingWindow->Name : "NULL");
			Text("WheelingWindowReleaseTimer: %.2f", g.WheelingWindowReleaseTimer);
			Text("WheelingAxisAvg[] = { %.3f, %.3f }, Main Axis: %s", g.WheelingAxisAvg.x, g.WheelingAxisAvg.y, (g.WheelingAxisAvg.x > g.WheelingAxisAvg.y) ? "X" : (g.WheelingAxisAvg.x < g.WheelingAxisAvg.y) ? "Y" : "<none>");
			Unindent();
		}

		Text("KEY OWNERS");
		{
			Indent();
			if (BeginListBox("##owners", KGVec2(-FLT_MIN, GetTextLineHeightWithSpacing() * 6)))
			{
				for (KarmaGuiKey key = KGGuiKey_NamedKey_BEGIN; key < KGGuiKey_NamedKey_END; key = (KarmaGuiKey)(key + 1))
				{
					KGGuiKeyOwnerData* owner_data = KarmaGuiInternal::GetKeyOwnerData(key);
					if (owner_data->OwnerCurr == KGGuiKeyOwner_None)
						continue;
					Text("%s: 0x%08X%s", GetKeyName(key), owner_data->OwnerCurr,
						owner_data->LockUntilRelease ? " LockUntilRelease" : owner_data->LockThisFrame ? " LockThisFrame" : "");
					KarmaGuiInternal::DebugLocateItemOnHover(owner_data->OwnerCurr);
				}
				EndListBox();
			}
			Unindent();
		}
		Text("SHORTCUT ROUTING");
		{
			Indent();
			if (BeginListBox("##routes", KGVec2(-FLT_MIN, GetTextLineHeightWithSpacing() * 6)))
			{
				for (KarmaGuiKey key = KGGuiKey_NamedKey_BEGIN; key < KGGuiKey_NamedKey_END; key = (KarmaGuiKey)(key + 1))
				{
					KGGuiKeyRoutingTable* rt = &g.KeysRoutingTable;
					for (KGGuiKeyRoutingIndex idx = rt->Index[key - KGGuiKey_NamedKey_BEGIN]; idx != -1; )
					{
						char key_chord_name[64];
						KGGuiKeyRoutingData* routing_data = &rt->Entries[idx];
						KarmaGuiInternal::GetKeyChordName(key | routing_data->Mods, key_chord_name, KG_ARRAYSIZE(key_chord_name));
						Text("%s: 0x%08X", key_chord_name, routing_data->RoutingCurr);
						KarmaGuiInternal::DebugLocateItemOnHover(routing_data->RoutingCurr);
						idx = routing_data->NextEntryIndex;
					}
				}
				EndListBox();
			}
			Text("(ActiveIdUsing: AllKeyboardKeys: %d, NavDirMask: 0x%X)", g.ActiveIdUsingAllKeyboardKeys, g.ActiveIdUsingNavDirMask);
			Unindent();
		}
		TreePop();
	}

	if (TreeNode("Internal state"))
	{
		Text("WINDOWING");
		Indent();
		Text("HoveredWindow: '%s'", g.HoveredWindow ? g.HoveredWindow->Name : "NULL");
		Text("HoveredWindow->Root: '%s'", g.HoveredWindow ? g.HoveredWindow->RootWindowDockTree->Name : "NULL");
		Text("HoveredWindowUnderMovingWindow: '%s'", g.HoveredWindowUnderMovingWindow ? g.HoveredWindowUnderMovingWindow->Name : "NULL");
		Text("HoveredDockNode: 0x%08X", g.DebugHoveredDockNode ? g.DebugHoveredDockNode->ID : 0);
		Text("MovingWindow: '%s'", g.MovingWindow ? g.MovingWindow->Name : "NULL");
		Text("MouseViewport: 0x%08X (UserHovered 0x%08X, LastHovered 0x%08X)", g.MouseViewport->ID, g.IO.MouseHoveredViewport, g.MouseLastHoveredViewport ? g.MouseLastHoveredViewport->ID : 0);
		Unindent();

		Text("ITEMS");
		Indent();
		Text("ActiveId: 0x%08X/0x%08X (%.2f sec), AllowOverlap: %d, Source: %s", g.ActiveId, g.ActiveIdPreviousFrame, g.ActiveIdTimer, g.ActiveIdAllowOverlap, GetInputSourceName(g.ActiveIdSource));
		KarmaGuiInternal::DebugLocateItemOnHover(g.ActiveId);
		Text("ActiveIdWindow: '%s'", g.ActiveIdWindow ? g.ActiveIdWindow->Name : "NULL");
		Text("ActiveIdUsing: AllKeyboardKeys: %d, NavDirMask: %X", g.ActiveIdUsingAllKeyboardKeys, g.ActiveIdUsingNavDirMask);
		Text("HoveredId: 0x%08X (%.2f sec), AllowOverlap: %d", g.HoveredIdPreviousFrame, g.HoveredIdTimer, g.HoveredIdAllowOverlap); // Not displaying g.HoveredId as it is update mid-frame
		Text("HoverDelayId: 0x%08X, Timer: %.2f, ClearTimer: %.2f", g.HoverDelayId, g.HoverDelayTimer, g.HoverDelayClearTimer);
		Text("DragDrop: %d, SourceId = 0x%08X, Payload \"%s\" (%d bytes)", g.DragDropActive, g.DragDropPayload.SourceId, g.DragDropPayload.DataType, g.DragDropPayload.DataSize);
		KarmaGuiInternal::DebugLocateItemOnHover(g.DragDropPayload.SourceId);
		Unindent();

		Text("NAV,FOCUS");
		Indent();
		Text("NavWindow: '%s'", g.NavWindow ? g.NavWindow->Name : "NULL");
		Text("NavId: 0x%08X, NavLayer: %d", g.NavId, g.NavLayer);
		KarmaGuiInternal::DebugLocateItemOnHover(g.NavId);
		Text("NavInputSource: %s", GetInputSourceName(g.NavInputSource));
		Text("NavActive: %d, NavVisible: %d", g.IO.NavActive, g.IO.NavVisible);
		Text("NavActivateId/DownId/PressedId/InputId: %08X/%08X/%08X/%08X", g.NavActivateId, g.NavActivateDownId, g.NavActivatePressedId, g.NavActivateInputId);
		Text("NavActivateFlags: %04X", g.NavActivateFlags);
		Text("NavDisableHighlight: %d, NavDisableMouseHover: %d", g.NavDisableHighlight, g.NavDisableMouseHover);
		Text("NavFocusScopeId = 0x%08X", g.NavFocusScopeId);
		Text("NavWindowingTarget: '%s'", g.NavWindowingTarget ? g.NavWindowingTarget->Name : "NULL");
		Unindent();

		TreePop();
	}

	// Overlay: Display windows Rectangles and Begin Order
	if (cfg->ShowWindowsRects || cfg->ShowWindowsBeginOrder)
	{
		for (int n = 0; n < g.Windows.Size; n++)
		{
			KGGuiWindow* window = g.Windows[n];
			if (!window->WasActive)
				continue;
			KGDrawList* draw_list = Karma::KarmaGuiInternal::GetForegroundDrawList(window);
			if (cfg->ShowWindowsRects)
			{
				KGRect r = Funcs::GetWindowRect(window, cfg->ShowWindowsRectsType);
				draw_list->AddRect(r.Min, r.Max, KG_COL32(255, 0, 128, 255));
			}
			if (cfg->ShowWindowsBeginOrder && !(window->Flags & KGGuiWindowFlags_ChildWindow))
			{
				char buf[32];
				KGFormatString(buf, KG_ARRAYSIZE(buf), "%d", window->BeginOrderWithinContext);
				float font_size = GetFontSize();
				draw_list->AddRectFilled(window->Pos, window->Pos + KGVec2(font_size, font_size), KG_COL32(200, 100, 100, 255));
				draw_list->AddText(window->Pos, KG_COL32(255, 255, 255, 255), buf);
			}
		}
	}

	// Overlay: Display Tables Rectangles
	if (cfg->ShowTablesRects)
	{
		for (int table_n = 0; table_n < g.Tables.GetMapSize(); table_n++)
		{
			KGGuiTable* table = g.Tables.TryGetMapData(table_n);
			if (table == NULL || table->LastFrameActive < g.FrameCount - 1)
				continue;
			KGDrawList* draw_list = Karma::KarmaGuiInternal::GetForegroundDrawList(table->OuterWindow);
			if (cfg->ShowTablesRectsType >= TRT_ColumnsRect)
			{
				for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
				{
					KGRect r = Funcs::GetTableRect(table, cfg->ShowTablesRectsType, column_n);
					KGU32 col = (table->HoveredColumnBody == column_n) ? KG_COL32(255, 255, 128, 255) : KG_COL32(255, 0, 128, 255);
					float thickness = (table->HoveredColumnBody == column_n) ? 3.0f : 1.0f;
					draw_list->AddRect(r.Min, r.Max, col, 0.0f, 0, thickness);
				}
			}
			else
			{
				KGRect r = Funcs::GetTableRect(table, cfg->ShowTablesRectsType, -1);
				draw_list->AddRect(r.Min, r.Max, KG_COL32(255, 0, 128, 255));
			}
		}
	}

#ifdef KARMAGUI_HAS_DOCK
	// Overlay: Display Docking info
	if (cfg->ShowDockingNodes && g.IO.KeyCtrl && g.DebugHoveredDockNode)
	{
		char buf[64] = "";
		char* p = buf;
		KGGuiDockNode* node = g.DebugHoveredDockNode;
		KGDrawList* overlay_draw_list = node->HostWindow ? Karma::KarmaGuiInternal::GetForegroundDrawList(node->HostWindow) : GetForegroundDrawList(GetMainViewport());
		p += KGFormatString(p, buf + KG_ARRAYSIZE(buf) - p, "DockId: %X%s\n", node->ID, node->IsCentralNode() ? " *CentralNode*" : "");
		p += KGFormatString(p, buf + KG_ARRAYSIZE(buf) - p, "WindowClass: %08X\n", node->WindowClass.ClassId);
		p += KGFormatString(p, buf + KG_ARRAYSIZE(buf) - p, "Size: (%.0f, %.0f)\n", node->Size.x, node->Size.y);
		p += KGFormatString(p, buf + KG_ARRAYSIZE(buf) - p, "SizeRef: (%.0f, %.0f)\n", node->SizeRef.x, node->SizeRef.y);
		int depth = KarmaGuiInternal::DockNodeGetDepth(node);
		overlay_draw_list->AddRect(node->Pos + KGVec2(3, 3) * (float)depth, node->Pos + node->Size - KGVec2(3, 3) * (float)depth, KG_COL32(200, 100, 100, 255));
		KGVec2 pos = node->Pos + KGVec2(3, 3) * (float)depth;
		overlay_draw_list->AddRectFilled(pos - KGVec2(1, 1), pos + CalcTextSize(buf) + KGVec2(1, 1), KG_COL32(200, 100, 100, 255));
		overlay_draw_list->AddText(NULL, 0.0f, pos, KG_COL32(255, 255, 255, 255), buf);
	}
#endif // #ifdef KARMAGUI_HAS_DOCK

	End();
}

// [DEBUG] Display contents of Columns
void Karma::KarmaGuiInternal::DebugNodeColumns(KGGuiOldColumns* columns)
{
	if (!KarmaGui::TreeNode((void*)(uintptr_t)columns->ID, "Columns Id: 0x%08X, Count: %d, Flags: 0x%04X", columns->ID, columns->Count, columns->Flags))
		return;
	KarmaGui::BulletText("Width: %.1f (MinX: %.1f, MaxX: %.1f)", columns->OffMaxX - columns->OffMinX, columns->OffMinX, columns->OffMaxX);
	for (int column_n = 0; column_n < columns->Columns.Size; column_n++)
		KarmaGui::BulletText("Column %02d: OffsetNorm %.3f (= %.1f px)", column_n, columns->Columns[column_n].OffsetNorm, GetColumnOffsetFromNorm(columns, columns->Columns[column_n].OffsetNorm));
	KarmaGui::TreePop();
}

static void DebugNodeDockNodeFlags(KarmaGuiDockNodeFlags* p_flags, const char* label, bool enabled)
{
	using namespace Karma;
	Karma::KarmaGui::PushID(label);
	Karma::KarmaGui::PushStyleVar(KGGuiStyleVar_FramePadding, KGVec2(0.0f, 0.0f));
	KarmaGui::Text("%s:", label);
	if (!enabled)
		Karma::KarmaGui::BeginDisabled();
	KarmaGui::CheckboxFlags("NoSplit", p_flags, KGGuiDockNodeFlags_NoSplit);
	KarmaGui::CheckboxFlags("NoResize", p_flags, KGGuiDockNodeFlags_NoResize);
	KarmaGui::CheckboxFlags("NoResizeX", p_flags, KGGuiDockNodeFlags_NoResizeX);
	KarmaGui::CheckboxFlags("NoResizeY", p_flags, KGGuiDockNodeFlags_NoResizeY);
	KarmaGui::CheckboxFlags("NoTabBar", p_flags, KGGuiDockNodeFlags_NoTabBar);
	KarmaGui::CheckboxFlags("HiddenTabBar", p_flags, KGGuiDockNodeFlags_HiddenTabBar);
	KarmaGui::CheckboxFlags("NoWindowMenuButton", p_flags, KGGuiDockNodeFlags_NoWindowMenuButton);
	KarmaGui::CheckboxFlags("NoCloseButton", p_flags, KGGuiDockNodeFlags_NoCloseButton);
	KarmaGui::CheckboxFlags("NoDocking", p_flags, KGGuiDockNodeFlags_NoDocking);
	KarmaGui::CheckboxFlags("NoDockingSplitMe", p_flags, KGGuiDockNodeFlags_NoDockingSplitMe);
	KarmaGui::CheckboxFlags("NoDockingSplitOther", p_flags, KGGuiDockNodeFlags_NoDockingSplitOther);
	KarmaGui::CheckboxFlags("NoDockingOverMe", p_flags, KGGuiDockNodeFlags_NoDockingOverMe);
	KarmaGui::CheckboxFlags("NoDockingOverOther", p_flags, KGGuiDockNodeFlags_NoDockingOverOther);
	KarmaGui::CheckboxFlags("NoDockingOverEmpty", p_flags, KGGuiDockNodeFlags_NoDockingOverEmpty);
	if (!enabled)
		Karma::KarmaGui::EndDisabled();
	Karma::KarmaGui::PopStyleVar();
	KarmaGui::PopID();
}

// [DEBUG] Display contents of KGDockNode
void Karma::KarmaGuiInternal::DebugNodeDockNode(KGGuiDockNode* node, const char* label)
{
	KarmaGuiContext& g = *GKarmaGui;
	const bool is_alive = (g.FrameCount - node->LastFrameAlive < 2);    // Submitted with KGGuiDockNodeFlags_KeepAliveOnly
	const bool is_active = (g.FrameCount - node->LastFrameActive < 2);  // Submitted
	if (!is_alive) { Karma::KarmaGui::PushStyleColor(KGGuiCol_Text, Karma::KarmaGui::GetStyleColorVec4(KGGuiCol_TextDisabled)); }
	bool open;
	KarmaGuiTreeNodeFlags tree_node_flags = node->IsFocused ? KGGuiTreeNodeFlags_Selected : KGGuiTreeNodeFlags_None;
	if (node->Windows.Size > 0)
		open = Karma::KarmaGui::TreeNodeEx((void*)(intptr_t)node->ID, tree_node_flags, "%s 0x%04X%s: %d windows (vis: '%s')", label, node->ID, node->IsVisible ? "" : " (hidden)", node->Windows.Size, node->VisibleWindow ? node->VisibleWindow->Name : "NULL");
	else
		open = Karma::KarmaGui::TreeNodeEx((void*)(intptr_t)node->ID, tree_node_flags, "%s 0x%04X%s: %s (vis: '%s')", label, node->ID, node->IsVisible ? "" : " (hidden)", (node->SplitAxis == KGGuiAxis_X) ? "horizontal split" : (node->SplitAxis == KGGuiAxis_Y) ? "vertical split" : "empty", node->VisibleWindow ? node->VisibleWindow->Name : "NULL");
	if (!is_alive) { Karma::KarmaGui::PopStyleColor(); }
	if (is_active && Karma::KarmaGui::IsItemHovered())
		if (KGGuiWindow* window = node->HostWindow ? node->HostWindow : node->VisibleWindow)
			Karma::KarmaGuiInternal::GetForegroundDrawList(window)->AddRect(node->Pos, node->Pos + node->Size, KG_COL32(255, 255, 0, 255));
	if (open)
	{
		KR_CORE_ASSERT(node->ChildNodes[0] == NULL || node->ChildNodes[0]->ParentNode == node, "");
		KR_CORE_ASSERT(node->ChildNodes[1] == NULL || node->ChildNodes[1]->ParentNode == node, "");
		Karma::KarmaGui::BulletText("Pos (%.0f,%.0f), Size (%.0f, %.0f) Ref (%.0f, %.0f)",
			node->Pos.x, node->Pos.y, node->Size.x, node->Size.y, node->SizeRef.x, node->SizeRef.y);
		Karma::KarmaGuiInternal::DebugNodeWindow(node->HostWindow, "HostWindow");
		Karma::KarmaGuiInternal::DebugNodeWindow(node->VisibleWindow, "VisibleWindow");
		Karma::KarmaGui::BulletText("SelectedTabID: 0x%08X, LastFocusedNodeID: 0x%08X", node->SelectedTabId, node->LastFocusedNodeId);
		Karma::KarmaGui::BulletText("Misc:%s%s%s%s%s%s%s",
			node->IsDockSpace() ? " IsDockSpace" : "",
			node->IsCentralNode() ? " IsCentralNode" : "",
			is_alive ? " IsAlive" : "", is_active ? " IsActive" : "", node->IsFocused ? " IsFocused" : "",
			node->WantLockSizeOnce ? " WantLockSizeOnce" : "",
			node->HasCentralNodeChild ? " HasCentralNodeChild" : "");
		if (Karma::KarmaGui::TreeNode("flags", "Flags Merged: 0x%04X, Local: 0x%04X, InWindows: 0x%04X, Shared: 0x%04X", node->MergedFlags, node->LocalFlags, node->LocalFlagsInWindows, node->SharedFlags))
		{
			if (Karma::KarmaGui::BeginTable("flags", 4))
			{
				Karma::KarmaGui::TableNextColumn(); DebugNodeDockNodeFlags(&node->MergedFlags, "MergedFlags", false);
				Karma::KarmaGui::TableNextColumn(); DebugNodeDockNodeFlags(&node->LocalFlags, "LocalFlags", true);
				Karma::KarmaGui::TableNextColumn(); DebugNodeDockNodeFlags(&node->LocalFlagsInWindows, "LocalFlagsInWindows", false);
				Karma::KarmaGui::TableNextColumn(); DebugNodeDockNodeFlags(&node->SharedFlags, "SharedFlags", true);
				Karma::KarmaGui::EndTable();
			}
			Karma::KarmaGui::TreePop();
		}
		if (node->ParentNode)
			DebugNodeDockNode(node->ParentNode, "ParentNode");
		if (node->ChildNodes[0])
			DebugNodeDockNode(node->ChildNodes[0], "Child[0]");
		if (node->ChildNodes[1])
			DebugNodeDockNode(node->ChildNodes[1], "Child[1]");
		if (node->TabBar)
			Karma::KarmaGuiInternal::DebugNodeTabBar(node->TabBar, "TabBar");
		Karma::KarmaGuiInternal::DebugNodeWindowsList(&node->Windows, "Windows");

		Karma::KarmaGui::TreePop();
	}
}

// [DEBUG] Display contents of KGDrawList
// Note that both 'window' and 'viewport' may be NULL here. Viewport is generally null of destroyed popups which previously owned a viewport.
void Karma::KarmaGuiInternal::DebugNodeDrawList(KGGuiWindow* window, KGGuiViewportP* viewport, const KGDrawList* draw_list, const char* label)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiMetricsConfig* cfg = &g.DebugMetricsConfig;
	int cmd_count = draw_list->CmdBuffer.Size;
	if (cmd_count > 0 && draw_list->CmdBuffer.back().ElemCount == 0 && draw_list->CmdBuffer.back().UserCallback == NULL)
		cmd_count--;
	bool node_open = Karma::KarmaGui::TreeNode(draw_list, "%s: '%s' %d vtx, %d indices, %d cmds", label, draw_list->_OwnerName ? draw_list->_OwnerName : "", draw_list->VtxBuffer.Size, draw_list->IdxBuffer.Size, cmd_count);
	if (draw_list == Karma::KarmaGui::GetWindowDrawList())
	{
		Karma::KarmaGui::SameLine();
		Karma::KarmaGui::TextColored(KGVec4(1.0f, 0.4f, 0.4f, 1.0f), "CURRENTLY APPENDING"); // Can't display stats for active draw list! (we don't have the data double-buffered)
		if (node_open)
			Karma::KarmaGui::TreePop();
		return;
	}

	KGDrawList* fg_draw_list = viewport ? Karma::KarmaGui::GetForegroundDrawList(viewport) : NULL; // Render additional visuals into the top-most draw list
	if (window && Karma::KarmaGui::IsItemHovered() && fg_draw_list)
		fg_draw_list->AddRect(window->Pos, window->Pos + window->Size, KG_COL32(255, 255, 0, 255));
	if (!node_open)
		return;

	if (window && !window->WasActive)
		Karma::KarmaGui::TextDisabled("Warning: owning Window is inactive. This DrawList is not being rendered!");

	for (const KGDrawCmd* pcmd = draw_list->CmdBuffer.Data; pcmd < draw_list->CmdBuffer.Data + cmd_count; pcmd++)
	{
		if (pcmd->UserCallback)
		{
			Karma::KarmaGui::BulletText("Callback %p, user_data %p", pcmd->UserCallback, pcmd->UserCallbackData);
			continue;
		}

		char buf[300];
		KGFormatString(buf, KG_ARRAYSIZE(buf), "DrawCmd:%5d tris, Tex 0x%p, ClipRect (%4.0f,%4.0f)-(%4.0f,%4.0f)",
			pcmd->ElemCount / 3, (void*)(intptr_t)pcmd->TextureId,
			pcmd->ClipRect.x, pcmd->ClipRect.y, pcmd->ClipRect.z, pcmd->ClipRect.w);
		bool pcmd_node_open = Karma::KarmaGui::TreeNode((void*)(pcmd - draw_list->CmdBuffer.begin()), "%s", buf);
		if (Karma::KarmaGui::IsItemHovered() && (cfg->ShowDrawCmdMesh || cfg->ShowDrawCmdBoundingBoxes) && fg_draw_list)
			Karma::KarmaGuiInternal::DebugNodeDrawCmdShowMeshAndBoundingBox(fg_draw_list, draw_list, pcmd, cfg->ShowDrawCmdMesh, cfg->ShowDrawCmdBoundingBoxes);
		if (!pcmd_node_open)
			continue;

		// Calculate approximate coverage area (touched pixel count)
		// This will be in pixels squared as long there's no post-scaling happening to the renderer output.
		const KGDrawIdx* idx_buffer = (draw_list->IdxBuffer.Size > 0) ? draw_list->IdxBuffer.Data : NULL;
		const KGDrawVert* vtx_buffer = draw_list->VtxBuffer.Data + pcmd->VtxOffset;
		float total_area = 0.0f;
		for (unsigned int idx_n = pcmd->IdxOffset; idx_n < pcmd->IdxOffset + pcmd->ElemCount; )
		{
			KGVec2 triangle[3];
			for (int n = 0; n < 3; n++, idx_n++)
				triangle[n] = vtx_buffer[idx_buffer ? idx_buffer[idx_n] : idx_n].pos;
			total_area += KGTriangleArea(triangle[0], triangle[1], triangle[2]);
		}

		// Display vertex information summary. Hover to get all triangles drawn in wire-frame
		KGFormatString(buf, KG_ARRAYSIZE(buf), "Mesh: ElemCount: %d, VtxOffset: +%d, IdxOffset: +%d, Area: ~%0.f px", pcmd->ElemCount, pcmd->VtxOffset, pcmd->IdxOffset, total_area);
		Karma::KarmaGui::Selectable(buf);
		if (Karma::KarmaGui::IsItemHovered() && fg_draw_list)
			Karma::KarmaGuiInternal::DebugNodeDrawCmdShowMeshAndBoundingBox(fg_draw_list, draw_list, pcmd, true, false);

		// Display individual triangles/vertices. Hover on to get the corresponding triangle highlighted.
		KarmaGuiListClipper clipper;
		clipper.Begin(pcmd->ElemCount / 3); // Manually coarse clip our print out of individual vertices to save CPU, only items that may be visible.
		while (clipper.Step())
			for (int prim = clipper.DisplayStart, idx_i = pcmd->IdxOffset + clipper.DisplayStart * 3; prim < clipper.DisplayEnd; prim++)
			{
				char* buf_p = buf, * buf_end = buf + KG_ARRAYSIZE(buf);
				KGVec2 triangle[3];
				for (int n = 0; n < 3; n++, idx_i++)
				{
					const KGDrawVert& v = vtx_buffer[idx_buffer ? idx_buffer[idx_i] : idx_i];
					triangle[n] = v.pos;
					buf_p += KGFormatString(buf_p, buf_end - buf_p, "%s %04d: pos (%8.2f,%8.2f), uv (%.6f,%.6f), col %08X\n",
						(n == 0) ? "Vert:" : "     ", idx_i, v.pos.x, v.pos.y, v.uv.x, v.uv.y, v.col);
				}

				Karma::KarmaGui::Selectable(buf, false);
				if (fg_draw_list && Karma::KarmaGui::IsItemHovered())
				{
					KGDrawListFlags backup_flags = fg_draw_list->Flags;
					fg_draw_list->Flags &= ~KGDrawListFlags_AntiAliasedLines; // Disable AA on triangle outlines is more readable for very large and thin triangles.
					fg_draw_list->AddPolyline(triangle, 3, KG_COL32(255, 255, 0, 255), KGDrawFlags_Closed, 1.0f);
					fg_draw_list->Flags = backup_flags;
				}
			}
		Karma::KarmaGui::TreePop();
	}
	Karma::KarmaGui::TreePop();
}

// [DEBUG] Display mesh/aabb of a KGDrawCmd
void Karma::KarmaGuiInternal::DebugNodeDrawCmdShowMeshAndBoundingBox(KGDrawList* out_draw_list, const KGDrawList* draw_list, const KGDrawCmd* draw_cmd, bool show_mesh, bool show_aabb)
{
	KR_CORE_ASSERT(show_mesh || show_aabb, "");

	// Draw wire-frame version of all triangles
	KGRect clip_rect = draw_cmd->ClipRect;
	KGRect vtxs_rect(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
	KGDrawListFlags backup_flags = out_draw_list->Flags;
	out_draw_list->Flags &= ~KGDrawListFlags_AntiAliasedLines; // Disable AA on triangle outlines is more readable for very large and thin triangles.
	for (unsigned int idx_n = draw_cmd->IdxOffset, idx_end = draw_cmd->IdxOffset + draw_cmd->ElemCount; idx_n < idx_end; )
	{
		KGDrawIdx* idx_buffer = (draw_list->IdxBuffer.Size > 0) ? draw_list->IdxBuffer.Data : NULL; // We don't hold on those pointers past iterations as ->AddPolyline() may invalidate them if out_draw_list==draw_list
		KGDrawVert* vtx_buffer = draw_list->VtxBuffer.Data + draw_cmd->VtxOffset;

		KGVec2 triangle[3];
		for (int n = 0; n < 3; n++, idx_n++)
			vtxs_rect.Add((triangle[n] = vtx_buffer[idx_buffer ? idx_buffer[idx_n] : idx_n].pos));
		if (show_mesh)
			out_draw_list->AddPolyline(triangle, 3, KG_COL32(255, 255, 0, 255), KGDrawFlags_Closed, 1.0f); // In yellow: mesh triangles
	}
	// Draw bounding boxes
	if (show_aabb)
	{
		out_draw_list->AddRect(KGFloor(clip_rect.Min), KGFloor(clip_rect.Max), KG_COL32(255, 0, 255, 255)); // In pink: clipping rectangle submitted to GPU
		out_draw_list->AddRect(KGFloor(vtxs_rect.Min), KGFloor(vtxs_rect.Max), KG_COL32(0, 255, 255, 255)); // In cyan: bounding box of triangles
	}
	out_draw_list->Flags = backup_flags;
}

// [DEBUG] Display details for a single font, called by ShowStyleEditor().
void Karma::KarmaGuiInternal::DebugNodeFont(KGFont* font)
{
	bool opened = Karma::KarmaGui::TreeNode(font, "Font: \"%s\"\n%.2f px, %d glyphs, %d file(s)",
		font->ConfigData ? font->ConfigData[0].Name : "", font->FontSize, font->Glyphs.Size, font->ConfigDataCount);
	Karma::KarmaGui::SameLine();
	if (Karma::KarmaGui::SmallButton("Set as default"))
		Karma::KarmaGui::GetIO().FontDefault = font;
	if (!opened)
		return;

	// Display preview text
	Karma::KarmaGui::PushFont(font);
	Karma::KarmaGui::Text("The quick brown fox jumps over the lazy dog");
	Karma::KarmaGui::PopFont();

	// Display details
	Karma::KarmaGui::SetNextItemWidth(Karma::KarmaGui::GetFontSize() * 8);
	Karma::KarmaGui::DragFloat("Font scale", &font->Scale, 0.005f, 0.3f, 2.0f, "%.1f");
	Karma::KarmaGui::SameLine(); MetricsHelpMarker(
		"Note than the default embedded font is NOT meant to be scaled.\n\n"
		"Font are currently rendered into bitmaps at a given size at the time of building the atlas. "
		"You may oversample them to get some flexibility with scaling. "
		"You can also render at multiple sizes and select which one to use at runtime.\n\n"
		"(Glimmer of hope: the atlas system will be rewritten in the future to make scaling more flexible.)");
	Karma::KarmaGui::Text("Ascent: %f, Descent: %f, Height: %f", font->Ascent, font->Descent, font->Ascent - font->Descent);
	char c_str[5];
	Karma::KarmaGui::Text("Fallback character: '%s' (U+%04X)", KGTextCharToUtf8(c_str, font->FallbackChar), font->FallbackChar);
	Karma::KarmaGui::Text("Ellipsis character: '%s' (U+%04X)", KGTextCharToUtf8(c_str, font->EllipsisChar), font->EllipsisChar);
	const int surface_sqrt = (int)KGSqrt((float)font->MetricsTotalSurface);
	Karma::KarmaGui::Text("Texture Area: about %d px ~%dx%d px", font->MetricsTotalSurface, surface_sqrt, surface_sqrt);
	for (int config_i = 0; config_i < font->ConfigDataCount; config_i++)
		if (font->ConfigData)
			if (const KGFontConfig* cfg = &font->ConfigData[config_i])
				Karma::KarmaGui::BulletText("Input %d: \'%s\', Oversample: (%d,%d), PixelSnapH: %d, Offset: (%.1f,%.1f)",
					config_i, cfg->Name, cfg->OversampleH, cfg->OversampleV, cfg->PixelSnapH, cfg->GlyphOffset.x, cfg->GlyphOffset.y);

	// Display all glyphs of the fonts in separate pages of 256 characters
	if (Karma::KarmaGui::TreeNode("Glyphs", "Glyphs (%d)", font->Glyphs.Size))
	{
		KGDrawList* draw_list = Karma::KarmaGui::GetWindowDrawList();
		const KGU32 glyph_col = Karma::KarmaGui::GetColorU32(KGGuiCol_Text);
		const float cell_size = font->FontSize * 1;
		const float cell_spacing = Karma::KarmaGui::GetStyle().ItemSpacing.y;
		for (unsigned int base = 0; base <= KG_UNICODE_CODEPOINT_MAX; base += 256)
		{
			// Skip ahead if a large bunch of glyphs are not present in the font (test in chunks of 4k)
			// This is only a small optimization to reduce the number of iterations when IM_UNICODE_MAX_CODEPOINT
			// is large // (if KGWchar==KGWchar32 we will do at least about 272 queries here)
			if (!(base & 4095) && font->IsGlyphRangeUnused(base, base + 4095))
			{
				base += 4096 - 256;
				continue;
			}

			int count = 0;
			for (unsigned int n = 0; n < 256; n++)
				if (font->FindGlyphNoFallback((KGWchar)(base + n)))
					count++;
			if (count <= 0)
				continue;
			if (!Karma::KarmaGui::TreeNode((void*)(intptr_t)base, "U+%04X..U+%04X (%d %s)", base, base + 255, count, count > 1 ? "glyphs" : "glyph"))
				continue;

			// Draw a 16x16 grid of glyphs
			KGVec2 base_pos = Karma::KarmaGui::GetCursorScreenPos();
			for (unsigned int n = 0; n < 256; n++)
			{
				// We use KGFont::RenderChar as a shortcut because we don't have UTF-8 conversion functions
				// available here and thus cannot easily generate a zero-terminated UTF-8 encoded string.
				KGVec2 cell_p1(base_pos.x + (n % 16) * (cell_size + cell_spacing), base_pos.y + (n / 16) * (cell_size + cell_spacing));
				KGVec2 cell_p2(cell_p1.x + cell_size, cell_p1.y + cell_size);
				const KGFontGlyph* glyph = font->FindGlyphNoFallback((KGWchar)(base + n));
				draw_list->AddRect(cell_p1, cell_p2, glyph ? KG_COL32(255, 255, 255, 100) : KG_COL32(255, 255, 255, 50));
				if (!glyph)
					continue;
				font->RenderChar(draw_list, cell_size, cell_p1, glyph_col, (KGWchar)(base + n));
				if (Karma::KarmaGui::IsMouseHoveringRect(cell_p1, cell_p2))
				{
					Karma::KarmaGui::BeginTooltip();
					Karma::KarmaGuiInternal::DebugNodeFontGlyph(font, glyph);
					Karma::KarmaGui::EndTooltip();
				}
			}
			Karma::KarmaGui::Dummy(KGVec2((cell_size + cell_spacing) * 16, (cell_size + cell_spacing) * 16));
			Karma::KarmaGui::TreePop();
		}
		Karma::KarmaGui::TreePop();
	}
	Karma::KarmaGui::TreePop();
}

void Karma::KarmaGuiInternal::DebugNodeFontGlyph(KGFont*, const KGFontGlyph* glyph)
{
	Karma::KarmaGui::Text("Codepoint: U+%04X", glyph->Codepoint);
	Karma::KarmaGui::Separator();
	Karma::KarmaGui::Text("Visible: %d", glyph->Visible);
	Karma::KarmaGui::Text("AdvanceX: %.1f", glyph->AdvanceX);
	Karma::KarmaGui::Text("Pos: (%.2f,%.2f)->(%.2f,%.2f)", glyph->X0, glyph->Y0, glyph->X1, glyph->Y1);
	Karma::KarmaGui::Text("UV: (%.3f,%.3f)->(%.3f,%.3f)", glyph->U0, glyph->V0, glyph->U1, glyph->V1);
}

// [DEBUG] Display contents of KarmaGuiStorage
void Karma::KarmaGuiInternal::DebugNodeStorage(KarmaGuiStorage* storage, const char* label)
{
	if (!Karma::KarmaGui::TreeNode(label, "%s: %d entries, %d bytes", label, storage->Data.Size, storage->Data.size_in_bytes()))
		return;
	for (int n = 0; n < storage->Data.Size; n++)
	{
		const KarmaGuiStorage::ImGuiStoragePair& p = storage->Data[n];
		Karma::KarmaGui::BulletText("Key 0x%08X Value { i: %d }", p.key, p.val_i); // Important: we currently don't store a type, real value may not be integer.
	}
	Karma::KarmaGui::TreePop();
}

void Karma::KarmaGuiInternal::DebugStartItemPicker()
{
	KarmaGuiContext& g = *GKarmaGui;
	g.DebugItemPickerActive = true;
}

// [DEBUG] Display contents of KGGuiTabBar
void Karma::KarmaGuiInternal::DebugNodeTabBar(KGGuiTabBar* tab_bar, const char* label)
{
	// Standalone tab bars (not associated to docking/windows functionality) currently hold no discernible strings.
	char buf[256];
	char* p = buf;
	const char* buf_end = buf + KG_ARRAYSIZE(buf);
	const bool is_active = (tab_bar->PrevFrameVisible >= Karma::KarmaGui::GetFrameCount() - 2);
	p += KGFormatString(p, buf_end - p, "%s 0x%08X (%d tabs)%s", label, tab_bar->ID, tab_bar->Tabs.Size, is_active ? "" : " *Inactive*");
	p += KGFormatString(p, buf_end - p, "  { ");
	for (int tab_n = 0; tab_n < KGMin(tab_bar->Tabs.Size, 3); tab_n++)
	{
		KGGuiTabItem* tab = &tab_bar->Tabs[tab_n];
		p += KGFormatString(p, buf_end - p, "%s'%s'",
			tab_n > 0 ? ", " : "", (tab->Window || tab->NameOffset != -1) ? tab_bar->GetTabName(tab) : "???");
	}
	p += KGFormatString(p, buf_end - p, (tab_bar->Tabs.Size > 3) ? " ... }" : " } ");
	if (!is_active) { Karma::KarmaGui::PushStyleColor(KGGuiCol_Text, Karma::KarmaGui::GetStyleColorVec4(KGGuiCol_TextDisabled)); }
	bool open = Karma::KarmaGui::TreeNode(label, "%s", buf);
	if (!is_active) { Karma::KarmaGui::PopStyleColor(); }
	if (is_active && Karma::KarmaGui::IsItemHovered())
	{
		KGDrawList* draw_list = Karma::KarmaGui::GetForegroundDrawList();
		draw_list->AddRect(tab_bar->BarRect.Min, tab_bar->BarRect.Max, KG_COL32(255, 255, 0, 255));
		draw_list->AddLine(KGVec2(tab_bar->ScrollingRectMinX, tab_bar->BarRect.Min.y), KGVec2(tab_bar->ScrollingRectMinX, tab_bar->BarRect.Max.y), KG_COL32(0, 255, 0, 255));
		draw_list->AddLine(KGVec2(tab_bar->ScrollingRectMaxX, tab_bar->BarRect.Min.y), KGVec2(tab_bar->ScrollingRectMaxX, tab_bar->BarRect.Max.y), KG_COL32(0, 255, 0, 255));
	}
	if (open)
	{
		for (int tab_n = 0; tab_n < tab_bar->Tabs.Size; tab_n++)
		{
			const KGGuiTabItem* tab = &tab_bar->Tabs[tab_n];
			Karma::KarmaGui::PushID(tab);
			if (Karma::KarmaGui::SmallButton("<")) { Karma::KarmaGuiInternal::TabBarQueueReorder(tab_bar, tab, -1); } Karma::KarmaGui::SameLine(0, 2);
			if (Karma::KarmaGui::SmallButton(">")) { Karma::KarmaGuiInternal::TabBarQueueReorder(tab_bar, tab, +1); } Karma::KarmaGui::SameLine();
			Karma::KarmaGui::Text("%02d%c Tab 0x%08X '%s' Offset: %.2f, Width: %.2f/%.2f",
				tab_n, (tab->ID == tab_bar->SelectedTabId) ? '*' : ' ', tab->ID, (tab->Window || tab->NameOffset != -1) ? tab_bar->GetTabName(tab) : "???", tab->Offset, tab->Width, tab->ContentWidth);
			Karma::KarmaGui::PopID();
		}
		Karma::KarmaGui::TreePop();
	}
}

void Karma::KarmaGuiInternal::DebugNodeViewport(KGGuiViewportP* viewport)
{
	Karma::KarmaGui::SetNextItemOpen(true, KGGuiCond_Once);
	if (Karma::KarmaGui::TreeNode((void*)(intptr_t)viewport->ID, "Viewport #%d, ID: 0x%08X, Parent: 0x%08X, Window: \"%s\"", viewport->Idx, viewport->ID, viewport->ParentViewportId, viewport->Window ? viewport->Window->Name : "N/A"))
	{
		KarmaGuiWindowFlags flags = viewport->Flags;
		Karma::KarmaGui::BulletText("Main Pos: (%.0f,%.0f), Size: (%.0f,%.0f)\nWorkArea Offset Left: %.0f Top: %.0f, Right: %.0f, Bottom: %.0f\nMonitor: %d, DpiScale: %.0f%%",
			viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y,
			viewport->WorkOffsetMin.x, viewport->WorkOffsetMin.y, viewport->WorkOffsetMax.x, viewport->WorkOffsetMax.y,
			viewport->PlatformMonitor, viewport->DpiScale * 100.0f);
		if (viewport->Idx > 0) { Karma::KarmaGui::SameLine(); if (Karma::KarmaGui::SmallButton("Reset Pos")) { viewport->Pos = KGVec2(200, 200); viewport->UpdateWorkRect(); if (viewport->Window) viewport->Window->Pos = viewport->Pos; } }
		Karma::KarmaGui::BulletText("Flags: 0x%04X =%s%s%s%s%s%s%s%s%s%s%s%s", viewport->Flags,
			//(flags & KGGuiViewportFlags_IsPlatformWindow) ? " IsPlatformWindow" : "", // Omitting because it is the standard
			(flags & KGGuiViewportFlags_IsPlatformMonitor) ? " IsPlatformMonitor" : "",
			(flags & KGGuiViewportFlags_OwnedByApp) ? " OwnedByApp" : "",
			(flags & KGGuiViewportFlags_NoDecoration) ? " NoDecoration" : "",
			(flags & KGGuiViewportFlags_NoTaskBarIcon) ? " NoTaskBarIcon" : "",
			(flags & KGGuiViewportFlags_NoFocusOnAppearing) ? " NoFocusOnAppearing" : "",
			(flags & KGGuiViewportFlags_NoFocusOnClick) ? " NoFocusOnClick" : "",
			(flags & KGGuiViewportFlags_NoInputs) ? " NoInputs" : "",
			(flags & KGGuiViewportFlags_NoRendererClear) ? " NoRendererClear" : "",
			(flags & KGGuiViewportFlags_TopMost) ? " TopMost" : "",
			(flags & KGGuiViewportFlags_Minimized) ? " Minimized" : "",
			(flags & KGGuiViewportFlags_NoAutoMerge) ? " NoAutoMerge" : "",
			(flags & KGGuiViewportFlags_CanHostOtherWindows) ? " CanHostOtherWindows" : "");
		for (int layer_i = 0; layer_i < KG_ARRAYSIZE(viewport->DrawDataBuilder.Layers); layer_i++)
			for (int draw_list_i = 0; draw_list_i < viewport->DrawDataBuilder.Layers[layer_i].Size; draw_list_i++)
				Karma::KarmaGuiInternal::DebugNodeDrawList(NULL, viewport, viewport->DrawDataBuilder.Layers[layer_i][draw_list_i], "DrawList");
		Karma::KarmaGui::TreePop();
	}
}

void Karma::KarmaGuiInternal::DebugNodeWindow(KGGuiWindow* window, const char* label)
{
	if (window == NULL)
	{
		Karma::KarmaGui::BulletText("%s: NULL", label);
		return;
	}

	KarmaGuiContext& g = *GKarmaGui;
	const bool is_active = window->WasActive;
	KarmaGuiTreeNodeFlags tree_node_flags = (window == g.NavWindow) ? KGGuiTreeNodeFlags_Selected : KGGuiTreeNodeFlags_None;
	if (!is_active) { Karma::KarmaGui::PushStyleColor(KGGuiCol_Text, Karma::KarmaGui::GetStyleColorVec4(KGGuiCol_TextDisabled)); }
	const bool open = Karma::KarmaGui::TreeNodeEx(label, tree_node_flags, "%s '%s'%s", label, window->Name, is_active ? "" : " *Inactive*");
	if (!is_active) { Karma::KarmaGui::PopStyleColor(); }
	if (Karma::KarmaGui::IsItemHovered() && is_active)
		Karma::KarmaGuiInternal::GetForegroundDrawList(window)->AddRect(window->Pos, window->Pos + window->Size, KG_COL32(255, 255, 0, 255));
	if (!open)
		return;

	if (window->MemoryCompacted)
		Karma::KarmaGui::TextDisabled("Note: some memory buffers have been compacted/freed.");

	KarmaGuiWindowFlags flags = window->Flags;
	DebugNodeDrawList(window, window->Viewport, window->DrawList, "DrawList");
	Karma::KarmaGui::BulletText("Pos: (%.1f,%.1f), Size: (%.1f,%.1f), ContentSize (%.1f,%.1f) Ideal (%.1f,%.1f)", window->Pos.x, window->Pos.y, window->Size.x, window->Size.y, window->ContentSize.x, window->ContentSize.y, window->ContentSizeIdeal.x, window->ContentSizeIdeal.y);
	Karma::KarmaGui::BulletText("Flags: 0x%08X (%s%s%s%s%s%s%s%s%s..)", flags,
		(flags & KGGuiWindowFlags_ChildWindow) ? "Child " : "", (flags & KGGuiWindowFlags_Tooltip) ? "Tooltip " : "", (flags & KGGuiWindowFlags_Popup) ? "Popup " : "",
		(flags & KGGuiWindowFlags_Modal) ? "Modal " : "", (flags & KGGuiWindowFlags_ChildMenu) ? "ChildMenu " : "", (flags & KGGuiWindowFlags_NoSavedSettings) ? "NoSavedSettings " : "",
		(flags & KGGuiWindowFlags_NoMouseInputs) ? "NoMouseInputs" : "", (flags & KGGuiWindowFlags_NoNavInputs) ? "NoNavInputs" : "", (flags & KGGuiWindowFlags_AlwaysAutoResize) ? "AlwaysAutoResize" : "");
	Karma::KarmaGui::BulletText("WindowClassId: 0x%08X", window->WindowClass.ClassId);
	Karma::KarmaGui::BulletText("Scroll: (%.2f/%.2f,%.2f/%.2f) Scrollbar:%s%s", window->Scroll.x, window->ScrollMax.x, window->Scroll.y, window->ScrollMax.y, window->ScrollbarX ? "X" : "", window->ScrollbarY ? "Y" : "");
	Karma::KarmaGui::BulletText("Active: %d/%d, WriteAccessed: %d, BeginOrderWithinContext: %d", window->Active, window->WasActive, window->WriteAccessed, (window->Active || window->WasActive) ? window->BeginOrderWithinContext : -1);
	Karma::KarmaGui::BulletText("Appearing: %d, Hidden: %d (CanSkip %d Cannot %d), SkipItems: %d", window->Appearing, window->Hidden, window->HiddenFramesCanSkipItems, window->HiddenFramesCannotSkipItems, window->SkipItems);
	for (int layer = 0; layer < KGGuiNavLayer_COUNT; layer++)
	{
		KGRect r = window->NavRectRel[layer];
		if (r.Min.x >= r.Max.y && r.Min.y >= r.Max.y)
			Karma::KarmaGui::BulletText("NavLastIds[%d]: 0x%08X", layer, window->NavLastIds[layer]);
		else
			Karma::KarmaGui::BulletText("NavLastIds[%d]: 0x%08X at +(%.1f,%.1f)(%.1f,%.1f)", layer, window->NavLastIds[layer], r.Min.x, r.Min.y, r.Max.x, r.Max.y);
		Karma::KarmaGuiInternal::DebugLocateItemOnHover(window->NavLastIds[layer]);
	}
	Karma::KarmaGui::BulletText("NavLayersActiveMask: %X, NavLastChildNavWindow: %s", window->DC.NavLayersActiveMask, window->NavLastChildNavWindow ? window->NavLastChildNavWindow->Name : "NULL");

	Karma::KarmaGui::BulletText("Viewport: %d%s, ViewportId: 0x%08X, ViewportPos: (%.1f,%.1f)", window->Viewport ? window->Viewport->Idx : -1, window->ViewportOwned ? " (Owned)" : "", window->ViewportId, window->ViewportPos.x, window->ViewportPos.y);
	Karma::KarmaGui::BulletText("ViewportMonitor: %d", window->Viewport ? window->Viewport->PlatformMonitor : -1);
	Karma::KarmaGui::BulletText("DockId: 0x%04X, DockOrder: %d, Act: %d, Vis: %d", window->DockId, window->DockOrder, window->DockIsActive, window->DockTabIsVisible);
	if (window->DockNode || window->DockNodeAsHost)
		DebugNodeDockNode(window->DockNodeAsHost ? window->DockNodeAsHost : window->DockNode, window->DockNodeAsHost ? "DockNodeAsHost" : "DockNode");

	if (window->RootWindow != window) { DebugNodeWindow(window->RootWindow, "RootWindow"); }
	if (window->RootWindowDockTree != window->RootWindow) { DebugNodeWindow(window->RootWindowDockTree, "RootWindowDockTree"); }
	if (window->ParentWindow != NULL) { DebugNodeWindow(window->ParentWindow, "ParentWindow"); }
	if (window->DC.ChildWindows.Size > 0) { Karma::KarmaGuiInternal::DebugNodeWindowsList(&window->DC.ChildWindows, "ChildWindows"); }
	if (window->ColumnsStorage.Size > 0 && Karma::KarmaGui::TreeNode("Columns", "Columns sets (%d)", window->ColumnsStorage.Size))
	{
		for (int n = 0; n < window->ColumnsStorage.Size; n++)
			Karma::KarmaGuiInternal::DebugNodeColumns(&window->ColumnsStorage[n]);
		Karma::KarmaGui::TreePop();
	}
	DebugNodeStorage(&window->StateStorage, "Storage");
	Karma::KarmaGui::TreePop();
}

void Karma::KarmaGuiInternal::DebugNodeWindowSettings(KGGuiWindowSettings* settings)
{
	Karma::KarmaGui::Text("0x%08X \"%s\" Pos (%d,%d) Size (%d,%d) Collapsed=%d",
		settings->ID, settings->GetName(), settings->Pos.x, settings->Pos.y, settings->Size.x, settings->Size.y, settings->Collapsed);
}

void Karma::KarmaGuiInternal::DebugNodeWindowsList(KGVector<KGGuiWindow*>* windows, const char* label)
{
	if (!Karma::KarmaGui::TreeNode(label, "%s (%d)", label, windows->Size))
		return;
	for (int i = windows->Size - 1; i >= 0; i--) // Iterate front to back
	{
		Karma::KarmaGui::PushID((*windows)[i]);
		Karma::KarmaGuiInternal::DebugNodeWindow((*windows)[i], "Window");
		Karma::KarmaGui::PopID();
	}
	Karma::KarmaGui::TreePop();
}

// FIXME-OPT: This is technically suboptimal, but it is simpler this way.
void Karma::KarmaGuiInternal::DebugNodeWindowsListByBeginStackParent(KGGuiWindow** windows, int windows_size, KGGuiWindow* parent_in_begin_stack)
{
	for (int i = 0; i < windows_size; i++)
	{
		KGGuiWindow* window = windows[i];
		if (window->ParentWindowInBeginStack != parent_in_begin_stack)
			continue;
		char buf[20];
		KGFormatString(buf, KG_ARRAYSIZE(buf), "[%04d] Window", window->BeginOrderWithinContext);
		//BulletText("[%04d] Window '%s'", window->BeginOrderWithinContext, window->Name);
		Karma::KarmaGuiInternal::DebugNodeWindow(window, buf);
		Karma::KarmaGui::Indent();
		DebugNodeWindowsListByBeginStackParent(windows + i + 1, windows_size - i - 1, window);
		Karma::KarmaGui::Unindent();
	}
}

void DebugLogV(const char* fmt, va_list args);

//-----------------------------------------------------------------------------
// [SECTION] DEBUG LOG WINDOW
//-----------------------------------------------------------------------------

void DebugLog(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	DebugLogV(fmt, args);
	va_end(args);
}

void DebugLogV(const char* fmt, va_list args)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	const int old_size = g.DebugLogBuf.size();
	g.DebugLogBuf.appendf("[%05d] ", g.FrameCount);
	g.DebugLogBuf.appendfv(fmt, args);
	if (g.DebugLogFlags & KGGuiDebugLogFlags_OutputToTTY)
	{
		KR_CORE_INFO("{0}", g.DebugLogBuf.begin() + old_size);
	}
	g.DebugLogIndex.append(g.DebugLogBuf.c_str(), old_size, g.DebugLogBuf.size());
}

void Karma::KarmaGui::ShowDebugLogWindow(bool* p_open)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (!(g.NextWindowData.Flags & KGGuiNextWindowDataFlags_HasSize))
		Karma::KarmaGui::SetNextWindowSize(KGVec2(0.0f, Karma::KarmaGui::GetFontSize() * 12.0f), KGGuiCond_FirstUseEver);
	if (!Karma::KarmaGui::Begin("KarmaGui Debug Log", p_open) || Karma::KarmaGuiInternal::GetCurrentWindow()->BeginCount > 1)
	{
		Karma::KarmaGui::End();
		return;
	}

	Karma::KarmaGui::AlignTextToFramePadding();
	Karma::KarmaGui::Text("Log events:");
	Karma::KarmaGui::SameLine(); Karma::KarmaGui::CheckboxFlags("All", &g.DebugLogFlags, KGGuiDebugLogFlags_EventMask_);
	Karma::KarmaGui::SameLine(); Karma::KarmaGui::CheckboxFlags("ActiveId", &g.DebugLogFlags, KGGuiDebugLogFlags_EventActiveId);
	Karma::KarmaGui::SameLine(); Karma::KarmaGui::CheckboxFlags("Focus", &g.DebugLogFlags, KGGuiDebugLogFlags_EventFocus);
	Karma::KarmaGui::SameLine(); Karma::KarmaGui::CheckboxFlags("Popup", &g.DebugLogFlags, KGGuiDebugLogFlags_EventPopup);
	Karma::KarmaGui::SameLine(); Karma::KarmaGui::CheckboxFlags("Nav", &g.DebugLogFlags, KGGuiDebugLogFlags_EventNav);
	Karma::KarmaGui::SameLine(); Karma::KarmaGui::CheckboxFlags("Clipper", &g.DebugLogFlags, KGGuiDebugLogFlags_EventClipper);
	Karma::KarmaGui::SameLine(); Karma::KarmaGui::CheckboxFlags("IO", &g.DebugLogFlags, KGGuiDebugLogFlags_EventIO);
	Karma::KarmaGui::SameLine(); Karma::KarmaGui::CheckboxFlags("Docking", &g.DebugLogFlags, KGGuiDebugLogFlags_EventDocking);
	Karma::KarmaGui::SameLine(); Karma::KarmaGui::CheckboxFlags("Viewport", &g.DebugLogFlags, KGGuiDebugLogFlags_EventViewport);

	if (Karma::KarmaGui::SmallButton("Clear"))
	{
		g.DebugLogBuf.clear();
		g.DebugLogIndex.clear();
	}
	Karma::KarmaGui::SameLine();
	if (Karma::KarmaGui::SmallButton("Copy"))
		Karma::KarmaGui::SetClipboardText(g.DebugLogBuf.c_str());
	Karma::KarmaGui::BeginChild("##log", KGVec2(0.0f, 0.0f), true, KGGuiWindowFlags_AlwaysVerticalScrollbar | KGGuiWindowFlags_AlwaysHorizontalScrollbar);

	KarmaGuiListClipper clipper;
	clipper.Begin(g.DebugLogIndex.size());
	while (clipper.Step())
		for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
		{
			const char* line_begin = g.DebugLogIndex.get_line_begin(g.DebugLogBuf.c_str(), line_no);
			const char* line_end = g.DebugLogIndex.get_line_end(g.DebugLogBuf.c_str(), line_no);
			Karma::KarmaGui::TextUnformatted(line_begin, line_end);
			KGRect text_rect = g.LastItemData.Rect;
			if (Karma::KarmaGui::IsItemHovered())
				for (const char* p = line_begin; p < line_end - 10; p++)
				{
					KGGuiID id = 0;
					if (p[0] != '0' || (p[1] != 'x' && p[1] != 'X') || sscanf(p + 2, "%X", &id) != 1)
						continue;
					KGVec2 p0 = Karma::KarmaGui::CalcTextSize(line_begin, p);
					KGVec2 p1 = Karma::KarmaGui::CalcTextSize(p, p + 10);
					g.LastItemData.Rect = KGRect(text_rect.Min + KGVec2(p0.x, 0.0f), text_rect.Min + KGVec2(p0.x + p1.x, p1.y));
					if (Karma::KarmaGui::IsMouseHoveringRect(g.LastItemData.Rect.Min, g.LastItemData.Rect.Max, true))
						Karma::KarmaGuiInternal::DebugLocateItemOnHover(id);
					p += 10;
				}
		}
	if (Karma::KarmaGui::GetScrollY() >= Karma::KarmaGui::GetScrollMaxY())
		Karma::KarmaGui::SetScrollHereY(1.0f);
	Karma::KarmaGui::EndChild();

	Karma::KarmaGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] OTHER DEBUG TOOLS (ITEM PICKER, STACK TOOL)
//-----------------------------------------------------------------------------

static const KGU32 DEBUG_LOCATE_ITEM_COLOR = KG_COL32(0, 255, 0, 255);  // Green

void Karma::KarmaGuiInternal::DebugLocateItem(KGGuiID target_id)
{
	KarmaGuiContext& g = *GKarmaGui;
	g.DebugLocateId = target_id;
	g.DebugLocateFrames = 2;
}

void Karma::KarmaGuiInternal::DebugLocateItemOnHover(KGGuiID target_id)
{
	if (target_id == 0 || !Karma::KarmaGui::IsItemHovered(KGGuiHoveredFlags_AllowWhenBlockedByActiveItem | KGGuiHoveredFlags_AllowWhenBlockedByPopup))
		return;
	KarmaGuiContext& g = *GKarmaGui;
	DebugLocateItem(target_id);
	Karma::KarmaGuiInternal::GetForegroundDrawList(g.CurrentWindow)->AddRect(g.LastItemData.Rect.Min - KGVec2(3.0f, 3.0f), g.LastItemData.Rect.Max + KGVec2(3.0f, 3.0f), DEBUG_LOCATE_ITEM_COLOR);
}

void Karma::KarmaGuiInternal::DebugLocateItemResolveWithLastItem()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiLastItemData item_data = g.LastItemData;
	g.DebugLocateId = 0;
	KGDrawList* draw_list = Karma::KarmaGuiInternal::GetForegroundDrawList(g.CurrentWindow);
	KGRect r = item_data.Rect;
	r.Expand(3.0f);
	KGVec2 p1 = g.IO.MousePos;
	KGVec2 p2 = KGVec2((p1.x < r.Min.x) ? r.Min.x : (p1.x > r.Max.x) ? r.Max.x : p1.x, (p1.y < r.Min.y) ? r.Min.y : (p1.y > r.Max.y) ? r.Max.y : p1.y);
	draw_list->AddRect(r.Min, r.Max, DEBUG_LOCATE_ITEM_COLOR);
	draw_list->AddLine(p1, p2, DEBUG_LOCATE_ITEM_COLOR);
}

// [DEBUG] Item picker tool - start with DebugStartItemPicker() - useful to visually select an item and break into its call-stack.
void Karma::KarmaGuiInternal::UpdateDebugToolItemPicker()
{
	KarmaGuiContext& g = *GKarmaGui;
	g.DebugItemPickerBreakId = 0;
	if (!g.DebugItemPickerActive)
		return;

	const KGGuiID hovered_id = g.HoveredIdPreviousFrame;
	Karma::KarmaGui::SetMouseCursor(KGGuiMouseCursor_Hand);
	if (Karma::KarmaGui::IsKeyPressed(KGGuiKey_Escape))
		g.DebugItemPickerActive = false;
	const bool change_mapping = g.IO.KeyMods == (KGGuiMod_Ctrl | KGGuiMod_Shift);
	if (!change_mapping && Karma::KarmaGui::IsMouseClicked(g.DebugItemPickerMouseButton) && hovered_id)
	{
		g.DebugItemPickerBreakId = hovered_id;
		g.DebugItemPickerActive = false;
	}
	for (int mouse_button = 0; mouse_button < 3; mouse_button++)
		if (change_mapping && Karma::KarmaGui::IsMouseClicked(mouse_button))
			g.DebugItemPickerMouseButton = (KGU8)mouse_button;
	Karma::KarmaGui::SetNextWindowBgAlpha(0.70f);
	Karma::KarmaGui::BeginTooltip();
	Karma::KarmaGui::Text("HoveredId: 0x%08X", hovered_id);
	Karma::KarmaGui::Text("Press ESC to abort picking.");
	const char* mouse_button_names[] = { "Left", "Right", "Middle" };
	if (change_mapping)
		Karma::KarmaGui::Text("Remap w/ Ctrl+Shift: click anywhere to select new mouse button.");
	else
		Karma::KarmaGui::TextColored(Karma::KarmaGui::GetStyleColorVec4(hovered_id ? KGGuiCol_Text : KGGuiCol_TextDisabled), "Click %s Button to break in debugger! (remap w/ Ctrl+Shift)", mouse_button_names[g.DebugItemPickerMouseButton]);
	Karma::KarmaGui::EndTooltip();
}

// [DEBUG] Stack Tool: update queries. Called by NewFrame()
void Karma::KarmaGuiInternal::UpdateDebugToolStackQueries()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiStackTool* tool = &g.DebugStackTool;

	// Clear hook when stack tool is not visible
	g.DebugHookIdInfo = 0;
	if (g.FrameCount != tool->LastActiveFrame + 1)
		return;

	// Update queries. The steps are: -1: query Stack, >= 0: query each stack item
	// We can only perform 1 ID Info query every frame. This is designed so the GetID() tests are cheap and constant-time
	const KGGuiID query_id = g.HoveredIdPreviousFrame ? g.HoveredIdPreviousFrame : g.ActiveId;
	if (tool->QueryId != query_id)
	{
		tool->QueryId = query_id;
		tool->StackLevel = -1;
		tool->Results.resize(0);
	}
	if (query_id == 0)
		return;

	// Advance to next stack level when we got our result, or after 2 frames (in case we never get a result)
	int stack_level = tool->StackLevel;
	if (stack_level >= 0 && stack_level < tool->Results.Size)
		if (tool->Results[stack_level].QuerySuccess || tool->Results[stack_level].QueryFrameCount > 2)
			tool->StackLevel++;

	// Update hook
	stack_level = tool->StackLevel;
	if (stack_level == -1)
		g.DebugHookIdInfo = query_id;
	if (stack_level >= 0 && stack_level < tool->Results.Size)
	{
		g.DebugHookIdInfo = tool->Results[stack_level].ID;
		tool->Results[stack_level].QueryFrameCount++;
	}
}

// [DEBUG] Stack tool: hooks called by GetID() family functions
void Karma::KarmaGuiInternal::DebugHookIdInfo(KGGuiID id, KarmaGuiDataType data_type, const void* data_id, const void* data_id_end)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	KGGuiStackTool* tool = &g.DebugStackTool;

	// Step 0: stack query
	// This assumes that the ID was computed with the current ID stack, which tends to be the case for our widget.
	if (tool->StackLevel == -1)
	{
		tool->StackLevel++;
		tool->Results.resize(window->IDStack.Size + 1, KGGuiStackLevelInfo());
		for (int n = 0; n < window->IDStack.Size + 1; n++)
			tool->Results[n].ID = (n < window->IDStack.Size) ? window->IDStack[n] : id;
		return;
	}

	// Step 1+: query for individual level
	KR_CORE_ASSERT(tool->StackLevel >= 0, "");
	if (tool->StackLevel != window->IDStack.Size)
		return;
	KGGuiStackLevelInfo* info = &tool->Results[tool->StackLevel];
	KR_CORE_ASSERT(info->ID == id && info->QueryFrameCount > 0, "");

	switch (data_type)
	{
	case KGGuiDataType_S32:
		KGFormatString(info->Desc, KG_ARRAYSIZE(info->Desc), "%d", (int)(intptr_t)data_id);
		break;
	case KGGuiDataType_String:
		KGFormatString(info->Desc, KG_ARRAYSIZE(info->Desc), "%.*s", data_id_end ? (int)((const char*)data_id_end - (const char*)data_id) : (int)strlen((const char*)data_id), (const char*)data_id);
		break;
	case KGGuiDataType_Pointer:
		KGFormatString(info->Desc, KG_ARRAYSIZE(info->Desc), "(void*)0x%p", data_id);
		break;
	case KGGuiDataType_ID:
		if (info->Desc[0] != 0) // PushOverrideID() is often used to avoid hashing twice, which would lead to 2 calls to DebugHookIdInfo(). We prioritize the first one.
			return;
		KGFormatString(info->Desc, KG_ARRAYSIZE(info->Desc), "0x%08X [override]", id);
		break;
	default:
		KR_CORE_ASSERT(0, "");
	}
	info->QuerySuccess = true;
	info->DataType = data_type;
}

static int StackToolFormatLevelInfo(KGGuiStackTool* tool, int n, bool format_for_ui, char* buf, size_t buf_size)
{
	KGGuiStackLevelInfo* info = &tool->Results[n];
	KGGuiWindow* window = (info->Desc[0] == 0 && n == 0) ? Karma::KarmaGuiInternal::FindWindowByID(info->ID) : NULL;
	if (window)                                                                 // Source: window name (because the root ID don't call GetID() and so doesn't get hooked)
		return KGFormatString(buf, buf_size, format_for_ui ? "\"%s\" [window]" : "%s", window->Name);
	if (info->QuerySuccess)                                                     // Source: GetID() hooks (prioritize over ItemInfo() because we frequently use patterns like: PushID(str), Button("") where they both have same id)
		return KGFormatString(buf, buf_size, (format_for_ui && info->DataType == KGGuiDataType_String) ? "\"%s\"" : "%s", info->Desc);
	if (tool->StackLevel < tool->Results.Size)                                  // Only start using fallback below when all queries are done, so during queries we don't flickering ??? markers.
		return (*buf = 0);
#ifdef IMGUI_ENABLE_TEST_ENGINE
	if (const char* label = ImGuiTestEngine_FindItemDebugLabel(Karma::GKarmaGui, info->ID))   // Source: ImGuiTestEngine's ItemInfo()
		return KGFormatString(buf, buf_size, format_for_ui ? "??? \"%s\"" : "%s", label);
#endif
	return KGFormatString(buf, buf_size, "???");
}

// Stack Tool: Display UI
void Karma::KarmaGui::ShowStackToolWindow(bool* p_open)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (!(g.NextWindowData.Flags & KGGuiNextWindowDataFlags_HasSize))
		Karma::KarmaGui::SetNextWindowSize(KGVec2(0.0f, Karma::KarmaGui::GetFontSize() * 8.0f), KGGuiCond_FirstUseEver);
	if (!Karma::KarmaGui::Begin("Dear ImGui Stack Tool", p_open) || Karma::KarmaGuiInternal::GetCurrentWindow()->BeginCount > 1)
	{
		Karma::KarmaGui::End();
		return;
	}

	// Display hovered/active status
	KGGuiStackTool* tool = &g.DebugStackTool;
	const KGGuiID hovered_id = g.HoveredIdPreviousFrame;
	const KGGuiID active_id = g.ActiveId;
#ifdef IMGUI_ENABLE_TEST_ENGINE
	Karma::KarmaGui::Text("HoveredId: 0x%08X (\"%s\"), ActiveId:  0x%08X (\"%s\")", hovered_id, hovered_id ? ImGuiTestEngine_FindItemDebugLabel(&g, hovered_id) : "", active_id, active_id ? ImGuiTestEngine_FindItemDebugLabel(&g, active_id) : "");
#else
	Karma::KarmaGui::Text("HoveredId: 0x%08X, ActiveId:  0x%08X", hovered_id, active_id);
#endif
	Karma::KarmaGui::SameLine();
	MetricsHelpMarker("Hover an item with the mouse to display elements of the ID Stack leading to the item's final ID.\nEach level of the stack correspond to a PushID() call.\nAll levels of the stack are hashed together to make the final ID of a widget (ID displayed at the bottom level of the stack).\nRead FAQ entry about the ID stack for details.");

	// CTRL+C to copy path
	const float time_since_copy = (float)g.Time - tool->CopyToClipboardLastTime;
	Karma::KarmaGui::Checkbox("Ctrl+C: copy path to clipboard", &tool->CopyToClipboardOnCtrlC);
	Karma::KarmaGui::SameLine();
	Karma::KarmaGui::TextColored((time_since_copy >= 0.0f && time_since_copy < 0.75f && KGFmod(time_since_copy, 0.25f) < 0.25f * 0.5f) ? KGVec4(1.f, 1.f, 0.3f, 1.f) : KGVec4(), "*COPIED*");
	if (tool->CopyToClipboardOnCtrlC && Karma::KarmaGui::IsKeyDown(KGGuiMod_Ctrl) && Karma::KarmaGui::IsKeyPressed(KGGuiKey_C))
	{
		tool->CopyToClipboardLastTime = (float)g.Time;
		char* p = g.TempBuffer.Data;
		char* p_end = p + g.TempBuffer.Size;
		for (int stack_n = 0; stack_n < tool->Results.Size && p + 3 < p_end; stack_n++)
		{
			*p++ = '/';
			char level_desc[256];
			StackToolFormatLevelInfo(tool, stack_n, false, level_desc, KG_ARRAYSIZE(level_desc));
			for (int n = 0; level_desc[n] && p + 2 < p_end; n++)
			{
				if (level_desc[n] == '/')
					*p++ = '\\';
				*p++ = level_desc[n];
			}
		}
		*p = '\0';
		Karma::KarmaGui::SetClipboardText(g.TempBuffer.Data);
	}

	// Display decorated stack
	tool->LastActiveFrame = g.FrameCount;
	if (tool->Results.Size > 0 && Karma::KarmaGui::BeginTable("##table", 3, KGGuiTableFlags_Borders))
	{
		const float id_width = Karma::KarmaGui::CalcTextSize("0xDDDDDDDD").x;
		Karma::KarmaGui::TableSetupColumn("Seed", KGGuiTableColumnFlags_WidthFixed, id_width);
		Karma::KarmaGui::TableSetupColumn("PushID", KGGuiTableColumnFlags_WidthStretch);
		Karma::KarmaGui::TableSetupColumn("Result", KGGuiTableColumnFlags_WidthFixed, id_width);
		Karma::KarmaGui::TableHeadersRow();
		for (int n = 0; n < tool->Results.Size; n++)
		{
			KGGuiStackLevelInfo* info = &tool->Results[n];
			Karma::KarmaGui::TableNextColumn();
			Karma::KarmaGui::Text("0x%08X", (n > 0) ? tool->Results[n - 1].ID : 0);
			Karma::KarmaGui::TableNextColumn();
			StackToolFormatLevelInfo(tool, n, true, g.TempBuffer.Data, g.TempBuffer.Size);
			Karma::KarmaGui::TextUnformatted(g.TempBuffer.Data);
			Karma::KarmaGui::TableNextColumn();
			Karma::KarmaGui::Text("0x%08X", info->ID);
			if (n == tool->Results.Size - 1)
				Karma::KarmaGui::TableSetBgColor(KGGuiTableBgTarget_CellBg, Karma::KarmaGui::GetColorU32(KGGuiCol_Header));
		}
		Karma::KarmaGui::EndTable();
	}
	Karma::KarmaGui::End();
}

#else

void KarmaGui::ShowMetricsWindow(bool*) {}
void KarmaGui::ShowFontAtlas(KGFontAtlas*) {}
void KarmaGui::DebugNodeColumns(KGGuiOldColumns*) {}
void Karma::KarmaGuiInternal::DebugNodeDrawList(KGGuiWindow*, KGGuiViewportP*, const KGDrawList*, const char*) {}
void KarmaGui::DebugNodeDrawCmdShowMeshAndBoundingBox(KGDrawList*, const KGDrawList*, const KGDrawCmd*, bool, bool) {}
void KarmaGui::DebugNodeFont(KGFont*) {}
void KarmaGui::DebugNodeStorage(KarmaGuiStorage*, const char*) {}
void KarmaGui::DebugNodeTabBar(KGGuiTabBar*, const char*) {}
void KarmaGui::DebugNodeWindow(KGGuiWindow*, const char*) {}
void Karma::KarmaGuiInternal::DebugNodeWindowSettings(KGGuiWindowSettings*) {}
void KarmaGui::DebugNodeWindowsList(KGVector<KGGuiWindow*>*, const char*) {}
void KarmaGui::DebugNodeViewport(KGGuiViewportP*) {}

void KarmaGui::DebugLog(const char*, ...) {}
void KarmaGui::DebugLogV(const char*, va_list) {}
void KarmaGui::ShowDebugLogWindow(bool*) {}
void KarmaGui::ShowStackToolWindow(bool*) {}
void KarmaGui::DebugHookIdInfo(KGGuiID, KarmaGuiDataType, const void*, const void*) {}
void KarmaGui::UpdateDebugToolItemPicker() {}
void KarmaGui::UpdateDebugToolStackQueries() {}

#endif // #ifndef KARMAGUI_DISABLE_DEBUG_TOOLS

//-----------------------------------------------------------------------------
