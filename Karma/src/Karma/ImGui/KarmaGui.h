#pragma once

#define KG_FMTARGS(FMT)             __attribute__((format(printf, FMT, FMT+1)))
#define KG_FMTLIST(FMT)             __attribute__((format(printf, FMT, 0)))
#define KG_ARRAYSIZE(_ARR)          ((int)(sizeof(_ARR) / sizeof(*(_ARR))))     // Size of a static C-style array. Don't use on pointers!
#define KG_UNUSED(_VAR)             ((void)(_VAR))
#define KG_OFFSETOF(_TYPE,_MEMBER)  offsetof(_TYPE, _MEMBER)                    // Offset of _MEMBER within _TYPE. Standardized as offsetof() in C++11


// Includes
#include "krpch.h"

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations and basic types
//-----------------------------------------------------------------------------

// Forward declarations (KG = KarmaGui)
struct KGDrawChannel;                   // Temporary storage to output draw commands out of order, used by KGDrawListSplitter and KGDrawList::ChannelsSplit()
struct KGDrawCmd;                       // A single draw command within a parent KGDrawList (generally maps to 1 GPU draw call, unless it is a callback)
struct KGDrawData;                      // All draw command lists required to render the frame + pos/size coordinates to use for the projection matrix.
struct KGDrawList;                      // A single draw command list (generally one per window, conceptually you may see this as a dynamic "mesh" builder)
struct KGDrawListSharedData;            // Data shared among multiple draw lists (typically owned by parent ImGui context, but you may create one yourself)
struct KGDrawListSplitter;              // Helper to split a draw list into different layers which can be drawn into out of order, then flattened back.
struct KGDrawVert;                      // A single vertex (pos + uv + col = 20 bytes by default. Override layout with IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT)
                                        // May need to feed in vertex shader provision
struct KGFont;                          // Runtime data for a single font within a parent KGFontAtlas
struct KGFontAtlas;                     // Runtime data for multiple fonts, bake multiple fonts into a single texture, TTF/OTF font loader
struct KGFontBuilderIO;                 // Opaque interface to a font builder (stb_truetype or FreeType).
struct KGFontConfig;                    // Configuration data when adding a font or merging fonts
struct KGFontGlyph;                     // A single font glyph (code point + coordinates within in KGFontAtlas + offset)
struct KGFontGlyphRangesBuilder;        // Helper to build glyph ranges from text/string data
struct KGColor;                         // Helper functions to create a color that can be converted to either u32 or float4 (*OBSOLETE* please avoid using)
struct KarmaGuiContext;                 // KarmaGui context (opaque structure, unless including KarmaGuiInternal.h)
struct KarmaGuiIO;                      // Main configuration and I/O between your application and ImGui
struct KarmaGuiInputTextCallbackData;   // Shared state of InputText() when using custom KarmaGuiInputTextCallback (rare/advanced use)
struct KarmaGuiKeyData;                 // Storage for KarmaGuiIO and IsKeyDown(), IsKeyPressed() etc functions.
struct KarmaGuiListClipper;             // Helper to manually clip large list of items
struct KarmaGuiOnceUponAFrame;          // Helper for running a block of code not more than once a frame
struct KarmaGuiPayload;                 // User data payload for drag and drop operations
struct KarmaGuiPlatformIO;              // Multi-viewport support: interface for Platform/Renderer backends + viewports to render
struct KarmaGuiPlatformMonitor;         // Multi-viewport support: user-provided bounds for each connected monitor/display. Used when positioning popups and tooltips to avoid them straddling monitors
struct KarmaGuiPlatformImeData;         // Platform IME data for io.SetPlatformImeDataFn() function.
struct KarmaGuiSizeCallbackData;        // Callback data when using SetNextWindowSizeConstraints() (rare/advanced use)
struct KarmaGuiStorage;                 // Helper for key->value storage
struct KarmaGuiStyle;                   // Runtime data for styling/colors
struct KarmaGuiTableSortSpecs;          // Sorting specifications for a table (often handling sort specs for a single column, occasionally more)
struct KarmaGuiTableColumnSortSpecs;    // Sorting specification for one column of a table
struct KarmaGuiTextBuffer;              // Helper to hold and append into a text buffer (~string builder)
struct KarmaGuiTextFilter;              // Helper to parse and apply text filters (e.g. "aaaaa[,bbbbb][,ccccc]")
struct KarmaGuiViewport;                // A Platform Window (always 1 unless multi-viewport are enabled. One per platform window to output to). In the future may represent Platform Monitor
struct KarmaGuiWindowClass;             // Window class (rare/advanced uses: provide hints to the platform backend via altered viewport flags and parent/child info)

// Enumerations
// - We don't use strongly typed enums much because they add constraints (can't extend in private code, can't store typed in bit fields, extra casting on iteration)
// - Tip: Use your programming IDE navigation facilities on the names in the _central column_ below to find the actual flags/enum lists!
//   In Visual Studio IDE: CTRL+comma ("Edit.GoToAll") can follow symbols in comments, whereas CTRL+F12 ("Edit.GoToImplementation") cannot.
//   With Visual Assist installed: ALT+G ("VAssistX.GoToImplementation") can also follow symbols in comments.
//   What about XCode und QTCreator Omar?
enum KarmaGuiKey : int;                 // -> enum KarmaGuiKey              // Enum: A key identifier (KGGuiKey_XXX or KGGuiMod_XXX value)

typedef int KarmaGuiCol;                // -> enum KGGuiCol_             // Enum: A color identifier for styling
typedef int KarmaGuiCond;               // -> enum KGGuiCond_            // Enum: A condition for many Set*() functions
typedef int KarmaGuiDataType;           // -> enum KGGuiDataType_        // Enum: A primary data type
typedef int KarmaGuiDir;                // -> enum KGGuiDir_             // Enum: A cardinal direction
typedef int KarmaGuiMouseButton;        // -> enum KGGuiMouseButton_     // Enum: A mouse button identifier (0=left, 1=right, 2=middle)
typedef int KarmaGuiMouseCursor;        // -> enum KGGuiMouseCursor_     // Enum: A mouse cursor shape
typedef int KarmaGuiSortDirection;      // -> enum KGGuiSortDirection_   // Enum: A sorting direction (ascending or descending)
typedef int KarmaGuiStyleVar;           // -> enum KGGuiStyleVar_        // Enum: A variable identifier for styling
typedef int KarmaGuiTableBgTarget;      // -> enum KGGuiTableBgTarget_   // Enum: A color target for TableSetBgColor()

// Flags (declared as int for compatibility with old C++, to allow using as flags without overhead, and to not pollute the top of this file)
typedef int KGDrawFlags;                // -> enum KGDrawFlags_          // Flags: for KGDrawList functions
typedef int KGDrawListFlags;            // -> enum KGDrawListFlags_      // Flags: for KGDrawList instance
typedef int KGFontAtlasFlags;           // -> enum KGFontAtlasFlags_     // Flags: for KGFontAtlas build
typedef int KarmaGuiBackendFlags;       // -> enum KGGuiBackendFlags_    // Flags: for io.BackendFlags
typedef int KarmaGuiButtonFlags;        // -> enum KGGuiButtonFlags_     // Flags: for InvisibleButton()
typedef int KarmaGuiColorEditFlags;     // -> enum KGGuiColorEditFlags_  // Flags: for ColorEdit4(), ColorPicker4() etc.
typedef int KarmaGuiConfigFlags;        // -> enum KGGuiConfigFlags_     // Flags: for io.ConfigFlags
typedef int KarmaGuiComboFlags;         // -> enum KGGuiComboFlags_      // Flags: for BeginCombo()
typedef int KarmaGuiDockNodeFlags;      // -> enum KGGuiDockNodeFlags_   // Flags: for DockSpace()
typedef int KarmaGuiDragDropFlags;      // -> enum KGGuiDragDropFlags_   // Flags: for BeginDragDropSource(), AcceptDragDropPayload()
typedef int KarmaGuiFocusedFlags;       // -> enum KGGuiFocusedFlags_    // Flags: for IsWindowFocused()
typedef int KarmaGuiHoveredFlags;       // -> enum KGGuiHoveredFlags_    // Flags: for IsItemHovered(), IsWindowHovered() etc.
typedef int KarmaGuiInputFlags;         // -> enum KGGuiInputFlags_      // Flags: for Shortcut() (+ upcoming advanced versions of IsKeyPressed()/IsMouseClicked()/SetKeyOwner()/SetItemKeyOwner() currently in imgui_internal.h)
typedef int KarmaGuiInputTextFlags;     // -> enum KGGuiInputTextFlags_  // Flags: for InputText(), InputTextMultiline()
typedef int KarmaGuiKeyChord;           // -> KarmaGuiKey | KGGuiMod_XXX    // Flags: for storage only for now: an KarmaGuiKey optionally OR-ed with one or more KGGuiMod_XXX values.
typedef int KarmaGuiPopupFlags;         // -> enum KGGuiPopupFlags_      // Flags: for OpenPopup*(), BeginPopupContext*(), IsPopupOpen()
typedef int KarmaGuiSelectableFlags;    // -> enum KGGuiSelectableFlags_ // Flags: for Selectable()
typedef int KarmaGuiSliderFlags;        // -> enum KGGuiSliderFlags_     // Flags: for DragFloat(), DragInt(), SliderFloat(), SliderInt() etc.
typedef int KarmaGuiTabBarFlags;        // -> enum KGGuiTabBarFlags_     // Flags: for BeginTabBar()
typedef int KarmaGuiTabItemFlags;       // -> enum KGGuiTabItemFlags_    // Flags: for BeginTabItem()
typedef int KarmaGuiTableFlags;         // -> enum KGGuiTableFlags_      // Flags: For BeginTable()
typedef int KarmaGuiTableColumnFlags;   // -> enum KGGuiTableColumnFlags_// Flags: For TableSetupColumn()
typedef int KarmaGuiTableRowFlags;      // -> enum KGGuiTableRowFlags_   // Flags: For TableNextRow()
typedef int KarmaGuiTreeNodeFlags;      // -> enum KGGuiTreeNodeFlags_   // Flags: for TreeNode(), TreeNodeEx(), CollapsingHeader()
typedef int KarmaGuiViewportFlags;      // -> enum KGGuiViewportFlags_   // Flags: for KarmaGuiViewport
typedef int KarmaGuiWindowFlags;        // -> enum KGGuiWindowFlags_     // Flags: for Begin(), BeginChild()
typedef void* KGTextureID;              // Default: store a pointer or an integer fitting in a pointer (most renderer backends are ok with that)


// KGDrawIdx: vertex index. [Compile-time configurable type]
// - To use 16-bit indices + allow large meshes: backend need to set 'io.BackendFlags |= KGGuiBackendFlags_RendererHasVtxOffset' and handle KGDrawCmd::VtxOffset (recommended).
// - To use 32-bit indices: override with '#define KGDrawIdx unsigned int' in your imconfig.h file.
#ifndef KGDrawIdx
typedef unsigned short KGDrawIdx;   // Default: 16-bit (for maximum compatibility with renderer backends)
#endif

// ////////////////////////////////////////////////////
// // Needs hooked code with Karma's types if possible
// ////////////////////////////////////////////////////

// Scalar data types
// Seems like be restricted to KarmaGui
typedef unsigned int        KGGuiID;// A unique ID used by widgets (typically the result of hashing a stack of string)
typedef signed char         KGS8;   // 8-bit signed integer
typedef unsigned char       KGU8;   // 8-bit unsigned integer
typedef signed short        KGS16;  // 16-bit signed integer
typedef unsigned short      KGU16;  // 16-bit unsigned integer
typedef signed int          KGS32;  // 32-bit signed integer == int
typedef unsigned int        KGU32;  // 32-bit unsigned integer (often used to store packed colors)
typedef signed   long long  KGS64;  // 64-bit signed integer
typedef unsigned long long  KGU64;  // 64-bit unsigned integer

// Character types
// (we generally use UTF-8 encoded string in the API. This is storage specifically for a decoded character used for keyboard input and display)
typedef unsigned short KGWchar16;   // A single decoded U16 character/code point. We encode them as multi bytes UTF-8 when used in strings.
typedef KGWchar16 KGWchar;

// Callback and functions types
typedef int     (*KarmaGuiInputTextCallback)(KarmaGuiInputTextCallbackData* data);    // Callback function for ImGui::InputText()
typedef void    (*KarmaGuiSizeCallback)(KarmaGuiSizeCallbackData* data);              // Callback function for ImGui::SetNextWindowSizeConstraints()
typedef void*   (*KarmaGuiMemAllocFunc)(size_t sz, void* user_data);                  // Function signature for ImGui::SetAllocatorFunctions()
typedef void    (*KarmaGuiMemFreeFunc)(void* ptr, void* user_data);                   // Function signature for ImGui::SetAllocatorFunctions()

// KGVec2: 2D vector used to store positions, sizes etc. [Compile-time configurable type]
// This is a frequently used type in the API. Consider using IM_VEC2_CLASS_EXTRA to create implicit cast from/to our preferred type.
struct KGVec2
{
	float                                   x, y;
	constexpr KGVec2()                      : x(0.0f), y(0.0f) { }
	constexpr KGVec2(float _x, float _y)    : x(_x), y(_y) { }
	float  operator[] (size_t idx) const    { KR_CORE_ASSERT(idx <= 1, ""); return (&x)[idx]; }    // We very rarely use this [] operator, the assert overhead is fine.
	float& operator[] (size_t idx)          { KR_CORE_ASSERT(idx <= 1, ""); return (&x)[idx]; }    // We very rarely use this [] operator, the assert overhead is fine.
};

// KGVec4: 4D vector used to store clipping rectangles, colors etc. [Compile-time configurable type]
struct KGVec4
{
	float                                                     x, y, z, w;
	constexpr KGVec4()                                        : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }
	constexpr KGVec4(float _x, float _y, float _z, float _w)  : x(_x), y(_y), z(_z), w(_w) { }
#ifdef KG_VEC4_CLASS_EXTRA
	KG_VEC4_CLASS_EXTRA     // Define additional constructors and implicit cast operators in imconfig.h to convert back and forth between your math types and KGVec4.
#endif
};

namespace Karma
{
	class KARMA_API KarmaGui
	{
	public:
		// Context creation and access
		static KarmaGuiContext* CreateContext(KGFontAtlas* shared_font_atlas = NULL);
		static void          DestroyContext(KarmaGuiContext* ctx = NULL);   // NULL = destroy current context
		static KarmaGuiContext* GetCurrentContext();
		static void          SetCurrentContext(KarmaGuiContext* ctx);

		// Main
		static KarmaGuiIO&      GetIO();                                    // access the IO structure (mouse/keyboard/gamepad inputs, time, various configuration options/flags)
		static KarmaGuiStyle&   GetStyle();                                 // access the Style structure (colors, sizes). Always use PushStyleCol(), PushStyleVar() to modify style mid-frame!
		static void          NewFrame();                                 // start a new Dear ImGui frame, you can submit any command from this point until Render()/EndFrame().
		static void          EndFrame();                                 // ends the Dear ImGui frame. automatically called by Render(). If you don't need to render data (skipping rendering) you may call EndFrame() without Render()... but you'll have wasted CPU already! If you don't need to render, better to not create any windows and not call NewFrame() at all!
		static void          Render();                                   // ends the Dear ImGui frame, finalize the draw data. You can then get call GetDrawData().
		static KGDrawData*   GetDrawData();                              // valid after Render() and until the next call to NewFrame(). this is what you have to render.

		// Demo, Debug, Information
		static void          ShowDemoWindow(bool* p_open = NULL);        // create Demo window. demonstrate most ImGui features. call this to learn about the library! try to make it always available in your application!
		static void          ShowMetricsWindow(bool* p_open = NULL);     // create Metrics/Debugger window. display Dear ImGui internals: windows, draw commands, various internal state, etc.
		static void          ShowDebugLogWindow(bool* p_open = NULL);    // create Debug Log window. display a simplified log of important dear imgui events.
		static void          ShowStackToolWindow(bool* p_open = NULL);   // create Stack Tool window. hover items with mouse to query information about the source of their unique ID.
		static void          ShowAboutWindow(bool* p_open = NULL);       // create About window. display Dear ImGui version, credits and build/system information.
		static void          ShowStyleEditor(KarmaGuiStyle* ref = NULL);    // add style editor block (not a window). you can pass in a reference KarmaGuiStyle structure to compare to, revert to and save to (else it uses the default style)
		static bool          ShowStyleSelector(const char* label);       // add style selector block (not a window), essentially a combo listing the default styles.
		static void          ShowFontSelector(const char* label);        // add font selector block (not a window), essentially a combo listing the loaded fonts.
		static void          ShowUserGuide();                            // add basic help/info block (not a window): how to manipulate ImGui as an end-user (mouse/keyboard controls).

		// Styles
		static void          StyleColorsDark(KarmaGuiStyle* dst = NULL);    // new, recommended style (default)
		static void          StyleColorsLight(KarmaGuiStyle* dst = NULL);   // best used with borders and a custom, thicker font
		static void          StyleColorsClassic(KarmaGuiStyle* dst = NULL); // classic imgui style

		// Windows
		// - Begin() = push window to the stack and start appending to it. End() = pop window from the stack.
		// - Passing 'bool* p_open != NULL' shows a window-closing widget in the upper-right corner of the window,
		//   which clicking will set the boolean to false when clicked.
		// - You may append multiple times to the same window during the same frame by calling Begin()/End() pairs multiple times.
		//   Some information such as 'flags' or 'p_open' will only be considered by the first call to Begin().
		// - Begin() return false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting
		//   anything to the window. Always call a matching End() for each Begin() call, regardless of its return value!
		//   [Important: due to legacy reason, this is inconsistent with most other functions such as BeginMenu/EndMenu,
		//    BeginPopup/EndPopup, etc. where the EndXXX call should only be called if the corresponding BeginXXX function
		//    returned true. Begin and BeginChild are the only odd ones out. Will be fixed in a future update.]
		// - Note that the bottom of window stack always contains a window called "Debug".
		static bool          Begin(const char* name, bool* p_open = NULL, KarmaGuiWindowFlags flags = 0);
		static void          End();

		// Child Windows
		// - Use child windows to begin into a self-contained independent scrolling/clipping regions within a host window. Child windows can embed their own child.
		// - For each independent axis of 'size': ==0.0f: use remaining host window size / >0.0f: fixed size / <0.0f: use remaining window size minus abs(size) / Each axis can use a different mode, e.g. KGVec2(0,400).
		// - BeginChild() returns false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting anything to the window.
		//   Always call a matching EndChild() for each BeginChild() call, regardless of its return value.
		//   [Important: due to legacy reason, this is inconsistent with most other functions such as BeginMenu/EndMenu,
		//    BeginPopup/EndPopup, etc. where the EndXXX call should only be called if the corresponding BeginXXX function
		//    returned true. Begin and BeginChild are the only odd ones out. Will be fixed in a future update.]
		static bool          BeginChild(const char* str_id, const KGVec2& size = KGVec2(0, 0), bool border = false, KarmaGuiWindowFlags flags = 0);
		static bool          BeginChild(KGGuiID id, const KGVec2& size = KGVec2(0, 0), bool border = false, KarmaGuiWindowFlags flags = 0);
		static void          EndChild();

		// Windows Utilities
		// - 'current window' = the window we are appending into while inside a Begin()/End() block. 'next window' = next window we will Begin() into.
		static bool          IsWindowAppearing();
		static bool          IsWindowCollapsed();
		static bool          IsWindowFocused(KarmaGuiFocusedFlags flags=0); // is current window focused? or its root/child, depending on flags. see flags for options.
		static bool          IsWindowHovered(KarmaGuiHoveredFlags flags=0); // is current window hovered (and typically: not blocked by a popup/modal)? see flags for options. NB: If you are trying to check whether your mouse should be dispatched to imgui or to your app, you should use the 'io.WantCaptureMouse' boolean for that! Please read the FAQ!
		static KGDrawList*   GetWindowDrawList();                        // get draw list associated to the current window, to append your own drawing primitives
		static float         GetWindowDpiScale();                        // get DPI scale currently associated to the current window's viewport.
		static KGVec2        GetWindowPos();                             // get current window position in screen space (useful if you want to do your own drawing via the DrawList API)
		static KGVec2        GetWindowSize();                            // get current window size
		static float         GetWindowWidth();                           // get current window width (shortcut for GetWindowSize().x)
		static float         GetWindowHeight();                          // get current window height (shortcut for GetWindowSize().y)
		static KarmaGuiViewport*GetWindowViewport();                        // get viewport currently associated to the current window.

		// Window manipulation
		// - Prefer using SetNextXXX functions (before Begin) rather that SetXXX functions (after Begin).
		static void          SetNextWindowPos(const KGVec2& pos, KarmaGuiCond cond = 0, const KGVec2& pivot = KGVec2(0, 0)); // set next window position. call before Begin(). use pivot=(0.5f,0.5f) to center on given point, etc.
		static void          SetNextWindowSize(const KGVec2& size, KarmaGuiCond cond = 0);                  // set next window size. set axis to 0.0f to force an auto-fit on this axis. call before Begin()
		static void          SetNextWindowSizeConstraints(const KGVec2& size_min, const KGVec2& size_max, KarmaGuiSizeCallback custom_callback = NULL, void* custom_callback_data = NULL); // set next window size limits. use -1,-1 on either X/Y axis to preserve the current size. Sizes will be rounded down. Use callback to apply non-trivial programmatic constraints.
		static void          SetNextWindowContentSize(const KGVec2& size);                               // set next window content size (~ scrollable client area, which enforce the range of scrollbars). Not including window decorations (title bar, menu bar, etc.) nor WindowPadding. set an axis to 0.0f to leave it automatic. call before Begin()
		static void          SetNextWindowCollapsed(bool collapsed, KarmaGuiCond cond = 0);                 // set next window collapsed state. call before Begin()
		static void          SetNextWindowFocus();                                                       // set next window to be focused / top-most. call before Begin()
		static void          SetNextWindowScroll(const KGVec2& scroll);                                  // set next window scrolling value (use < 0.0f to not affect a given axis).
		static void          SetNextWindowBgAlpha(float alpha);                                          // set next window background color alpha. helper to easily override the Alpha component of KGGuiCol_WindowBg/ChildBg/PopupBg. you may also use KGGuiWindowFlags_NoBackground.
		static void          SetNextWindowViewport(KGGuiID viewport_id);                                 // set next window viewport
		static void          SetWindowPos(const KGVec2& pos, KarmaGuiCond cond = 0);                        // (not recommended) set current window position - call within Begin()/End(). prefer using SetNextWindowPos(), as this may incur tearing and side-effects.
		static void          SetWindowSize(const KGVec2& size, KarmaGuiCond cond = 0);                      // (not recommended) set current window size - call within Begin()/End(). set to KGVec2(0, 0) to force an auto-fit. prefer using SetNextWindowSize(), as this may incur tearing and minor side-effects.
		static void          SetWindowCollapsed(bool collapsed, KarmaGuiCond cond = 0);                     // (not recommended) set current window collapsed state. prefer using SetNextWindowCollapsed().
		static void          SetWindowFocus();                                                           // (not recommended) set current window to be focused / top-most. prefer using SetNextWindowFocus().
		static void          SetWindowFontScale(float scale);                                            // [OBSOLETE] set font scale. Adjust IO.FontGlobalScale if you want to scale all windows. This is an old API! For correct scaling, prefer to reload font + rebuild KGFontAtlas + call style.ScaleAllSizes().
		static void          SetWindowPos(const char* name, const KGVec2& pos, KarmaGuiCond cond = 0);      // set named window position.
		static void          SetWindowSize(const char* name, const KGVec2& size, KarmaGuiCond cond = 0);    // set named window size. set axis to 0.0f to force an auto-fit on this axis.
		static void          SetWindowCollapsed(const char* name, bool collapsed, KarmaGuiCond cond = 0);   // set named window collapsed state
		static void          SetWindowFocus(const char* name);                                           // set named window to be focused / top-most. use NULL to remove focus.

		// Content region
		// - Retrieve available space from a given point. GetContentRegionAvail() is frequently useful.
		// - Those functions are bound to be redesigned (they are confusing, incomplete and the Min/Max return values are in local window coordinates which increases confusion)
		static KGVec2        GetContentRegionAvail();                                        // == GetContentRegionMax() - GetCursorPos()
		static KGVec2        GetContentRegionMax();                                          // current content boundaries (typically window boundaries including scrolling, or current column boundaries), in windows coordinates
		static KGVec2        GetWindowContentRegionMin();                                    // content boundaries min for the full window (roughly (0,0)-Scroll), in window coordinates
		static KGVec2        GetWindowContentRegionMax();                                    // content boundaries max for the full window (roughly (0,0)+Size-Scroll) where Size can be overridden with SetNextWindowContentSize(), in window coordinates

		// Windows Scrolling
		// - Any change of Scroll will be applied at the beginning of next frame in the first call to Begin().
		// - You may instead use SetNextWindowScroll() prior to calling Begin() to avoid this delay, as an alternative to using SetScrollX()/SetScrollY().
		static float         GetScrollX();                                                   // get scrolling amount [0 .. GetScrollMaxX()]
		static float         GetScrollY();                                                   // get scrolling amount [0 .. GetScrollMaxY()]
		static void          SetScrollX(float scroll_x);                                     // set scrolling amount [0 .. GetScrollMaxX()]
		static void          SetScrollY(float scroll_y);                                     // set scrolling amount [0 .. GetScrollMaxY()]
		static float         GetScrollMaxX();                                                // get maximum scrolling amount ~~ ContentSize.x - WindowSize.x - DecorationsSize.x
		static float         GetScrollMaxY();                                                // get maximum scrolling amount ~~ ContentSize.y - WindowSize.y - DecorationsSize.y
		static void          SetScrollHereX(float center_x_ratio = 0.5f);                    // adjust scrolling amount to make current cursor position visible. center_x_ratio=0.0: left, 0.5: center, 1.0: right. When using to make a "default/current item" visible, consider using SetItemDefaultFocus() instead.
		static void          SetScrollHereY(float center_y_ratio = 0.5f);                    // adjust scrolling amount to make current cursor position visible. center_y_ratio=0.0: top, 0.5: center, 1.0: bottom. When using to make a "default/current item" visible, consider using SetItemDefaultFocus() instead.
		static void          SetScrollFromPosX(float local_x, float center_x_ratio = 0.5f);  // adjust scrolling amount to make given position visible. Generally GetCursorStartPos() + offset to compute a valid position.
		static void          SetScrollFromPosY(float local_y, float center_y_ratio = 0.5f);  // adjust scrolling amount to make given position visible. Generally GetCursorStartPos() + offset to compute a valid position.

		// Parameters stacks (shared)
		static void          PushFont(KGFont* font);                                         // use NULL as a shortcut to push default font
		static void          PopFont();
		static void          PushStyleColor(KarmaGuiCol idx, KGU32 col);                        // modify a style color. always use this if you modify the style after NewFrame().
		static void          PushStyleColor(KarmaGuiCol idx, const KGVec4& col);
		static void          PopStyleColor(int count = 1);
		static void          PushStyleVar(KarmaGuiStyleVar idx, float val);                     // modify a style float variable. always use this if you modify the style after NewFrame().
		static void          PushStyleVar(KarmaGuiStyleVar idx, const KGVec2& val);             // modify a style KGVec2 variable. always use this if you modify the style after NewFrame().
		static void          PopStyleVar(int count = 1);
		static void          PushAllowKeyboardFocus(bool allow_keyboard_focus);              // == tab stop enable. Allow focusing using TAB/Shift-TAB, enabled by default but you can disable it for certain widgets
		static void          PopAllowKeyboardFocus();
		static void          PushButtonRepeat(bool repeat);                                  // in 'repeat' mode, Button*() functions return repeated true in a typematic manner (using io.KeyRepeatDelay/io.KeyRepeatRate setting). Note that you can call IsItemActive() after any Button() to tell if the button is held in the current frame.
		static void          PopButtonRepeat();

		// Parameters stacks (current window)
		static void          PushItemWidth(float item_width);                                // push width of items for common large "item+label" widgets. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side).
		static void          PopItemWidth();
		static void          SetNextItemWidth(float item_width);                             // set width of the _next_ common large "item+label" widget. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side)
		static float         CalcItemWidth();                                                // width of item given pushed settings and current cursor position. NOT necessarily the width of last item unlike most 'Item' functions.
		static void          PushTextWrapPos(float wrap_local_pos_x = 0.0f);                 // push word-wrapping position for Text*() commands. < 0.0f: no wrapping; 0.0f: wrap to end of window (or column); > 0.0f: wrap at 'wrap_pos_x' position in window local space
		static void          PopTextWrapPos();

		// Style read access
		// - Use the style editor (ShowStyleEditor() function) to interactively see what the colors are)
		static KGFont*       GetFont();                                                      // get current font
		static float         GetFontSize();                                                  // get current font size (= height in pixels) of current font with current scale applied
		static KGVec2        GetFontTexUvWhitePixel();                                       // get UV coordinate for a while pixel, useful to draw custom shapes via the KGDrawList API
		static KGU32         GetColorU32(KarmaGuiCol idx, float alpha_mul = 1.0f);              // retrieve given style color with style alpha applied and optional extra alpha multiplier, packed as a 32-bit value suitable for KGDrawList
		static KGU32         GetColorU32(const KGVec4& col);                                 // retrieve given color with style alpha applied, packed as a 32-bit value suitable for KGDrawList
		static KGU32         GetColorU32(KGU32 col);                                         // retrieve given color with style alpha applied, packed as a 32-bit value suitable for KGDrawList
		static const KGVec4& GetStyleColorVec4(KarmaGuiCol idx);                                // retrieve style color as stored in KarmaGuiStyle structure. use to feed back into PushStyleColor(), otherwise use GetColorU32() to get style color with style alpha baked in.

		// Cursor / Layout
		// - By "cursor" we mean the current output position.
		// - The typical widget behavior is to output themselves at the current cursor position, then move the cursor one line down.
		// - You can call SameLine() between widgets to undo the last carriage return and output at the right of the preceding widget.
		// - Attention! We currently have inconsistencies between window-local and absolute positions we will aim to fix with future API:
		//    Window-local coordinates:   SameLine(), GetCursorPos(), SetCursorPos(), GetCursorStartPos(), GetContentRegionMax(), GetWindowContentRegion*(), PushTextWrapPos()
		//    Absolute coordinate:        GetCursorScreenPos(), SetCursorScreenPos(), all KGDrawList:: functions.
		static void          Separator();                                                    // separator, generally horizontal. inside a menu bar or in horizontal layout mode, this becomes a vertical separator.
		static void          SameLine(float offset_from_start_x=0.0f, float spacing=-1.0f);  // call between widgets or groups to layout them horizontally. X position given in window coordinates.
		static void          NewLine();                                                      // undo a SameLine() or force a new line when in a horizontal-layout context.
		static void          Spacing();                                                      // add vertical spacing.
		static void          Dummy(const KGVec2& size);                                      // add a dummy item of given size. unlike InvisibleButton(), Dummy() won't take the mouse click or be navigable into.
		static void          Indent(float indent_w = 0.0f);                                  // move content position toward the right, by indent_w, or style.IndentSpacing if indent_w <= 0
		static void          Unindent(float indent_w = 0.0f);                                // move content position back to the left, by indent_w, or style.IndentSpacing if indent_w <= 0
		static void          BeginGroup();                                                   // lock horizontal starting position
		static void          EndGroup();                                                     // unlock horizontal starting position + capture the whole group bounding box into one "item" (so you can use IsItemHovered() or layout primitives such as SameLine() on whole group, etc.)
		static KGVec2        GetCursorPos();                                                 // cursor position in window coordinates (relative to window position)
		static float         GetCursorPosX();                                                //   (some functions are using window-relative coordinates, such as: GetCursorPos, GetCursorStartPos, GetContentRegionMax, GetWindowContentRegion* etc.
		static float         GetCursorPosY();                                                //    other functions such as GetCursorScreenPos or everything in KGDrawList::
		static void          SetCursorPos(const KGVec2& local_pos);                          //    are using the main, absolute coordinate system.
		static void          SetCursorPosX(float local_x);                                   //    GetWindowPos() + GetCursorPos() == GetCursorScreenPos() etc.)
		static void          SetCursorPosY(float local_y);                                   //
		static KGVec2        GetCursorStartPos();                                            // initial cursor position in window coordinates
		static KGVec2        GetCursorScreenPos();                                           // cursor position in absolute coordinates (useful to work with KGDrawList API). generally top-left == GetMainViewport()->Pos == (0,0) in single viewport mode, and bottom-right == GetMainViewport()->Pos+Size == io.DisplaySize in single-viewport mode.
		static void          SetCursorScreenPos(const KGVec2& pos);                          // cursor position in absolute coordinates
		static void          AlignTextToFramePadding();                                      // vertically align upcoming text baseline to FramePadding.y so that it will align properly to regularly framed items (call if you have text on a line before a framed item)
		static float         GetTextLineHeight();                                            // ~ FontSize
		static float         GetTextLineHeightWithSpacing();                                 // ~ FontSize + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of text)
		static float         GetFrameHeight();                                               // ~ FontSize + style.FramePadding.y * 2
		static float         GetFrameHeightWithSpacing();                                    // ~ FontSize + style.FramePadding.y * 2 + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of framed widgets)

		// ID stack/scopes
		// Read the FAQ (docs/FAQ.md or http://dearimgui.org/faq) for more details about how ID are handled in dear imgui.
		// - Those questions are answered and impacted by understanding of the ID stack system:
		//   - "Q: Why is my widget not reacting when I click on it?"
		//   - "Q: How can I have widgets with an empty label?"
		//   - "Q: How can I have multiple widgets with the same label?"
		// - Short version: ID are hashes of the entire ID stack. If you are creating widgets in a loop you most likely
		//   want to push a unique identifier (e.g. object pointer, loop index) to uniquely differentiate them.
		// - You can also use the "Label##foobar" syntax within widget label to distinguish them from each others.
		// - In this header file we use the "label"/"name" terminology to denote a string that will be displayed + used as an ID,
		//   whereas "str_id" denote a string that is only used as an ID and not normally displayed.
		static void          PushID(const char* str_id);                                     // push string into the ID stack (will hash string).
		static void          PushID(const char* str_id_begin, const char* str_id_end);       // push string into the ID stack (will hash string).
		static void          PushID(const void* ptr_id);                                     // push pointer into the ID stack (will hash pointer).
		static void          PushID(int int_id);                                             // push integer into the ID stack (will hash integer).
		static void          PopID();                                                        // pop from the ID stack.
		static KGGuiID       GetID(const char* str_id);                                      // calculate unique ID (hash of whole ID stack + given parameter). e.g. if you want to query into KarmaGuiStorage yourself
		static KGGuiID       GetID(const char* str_id_begin, const char* str_id_end);
		static KGGuiID       GetID(const void* ptr_id);

		// Widgets: Text
		static void          TextUnformatted(const char* text, const char* text_end = NULL); // raw text without formatting. Roughly equivalent to Text("%s", text) but: A) doesn't require null terminated string if 'text_end' is specified, B) it's faster, no memory copy is done, no buffer size limits, recommended for long chunks of text.
		static void          Text(const char* fmt, ...)                                      KG_FMTARGS(1); // formatted text
		static void          TextV(const char* fmt, va_list args)                            KG_FMTLIST(1);
		static void          TextColored(const KGVec4& col, const char* fmt, ...)            KG_FMTARGS(2); // shortcut for PushStyleColor(KGGuiCol_Text, col); Text(fmt, ...); PopStyleColor();
		static void          TextColoredV(const KGVec4& col, const char* fmt, va_list args)  KG_FMTLIST(2);
		static void          TextDisabled(const char* fmt, ...)                              KG_FMTARGS(1); // shortcut for PushStyleColor(KGGuiCol_Text, style.Colors[KGGuiCol_TextDisabled]); Text(fmt, ...); PopStyleColor();
		static void          TextDisabledV(const char* fmt, va_list args)                    KG_FMTLIST(1);
		static void          TextWrapped(const char* fmt, ...)                               KG_FMTARGS(1); // shortcut for PushTextWrapPos(0.0f); Text(fmt, ...); PopTextWrapPos();. Note that this won't work on an auto-resizing window if there's no other widgets to extend the window width, yoy may need to set a size using SetNextWindowSize().
		static void          TextWrappedV(const char* fmt, va_list args)                     KG_FMTLIST(1);
		static void          LabelText(const char* label, const char* fmt, ...)              KG_FMTARGS(2); // display text+label aligned the same way as value+label widgets
		static void          LabelTextV(const char* label, const char* fmt, va_list args)    KG_FMTLIST(2);
		static void          BulletText(const char* fmt, ...)                                KG_FMTARGS(1); // shortcut for Bullet()+Text()
		static void          BulletTextV(const char* fmt, va_list args)                      KG_FMTLIST(1);

		// Widgets: Main
		// - Most widgets return true when the value has been changed or when pressed/selected
		// - You may also use one of the many IsItemXXX functions (e.g. IsItemActive, IsItemHovered, etc.) to query widget state.
		static bool          Button(const char* label, const KGVec2& size = KGVec2(0, 0));   // button
		static bool          SmallButton(const char* label);                                 // button with FramePadding=(0,0) to easily embed within text
		static bool          InvisibleButton(const char* str_id, const KGVec2& size, KarmaGuiButtonFlags flags = 0); // flexible button behavior without the visuals, frequently useful to build custom behaviors using the public api (along with IsItemActive, IsItemHovered, etc.)
		static bool          ArrowButton(const char* str_id, KarmaGuiDir dir);                  // square button with an arrow shape
		static bool          Checkbox(const char* label, bool* v);
		static bool          CheckboxFlags(const char* label, int* flags, int flags_value);
		static bool          CheckboxFlags(const char* label, unsigned int* flags, unsigned int flags_value);
		static bool          RadioButton(const char* label, bool active);                    // use with e.g. if (RadioButton("one", my_value==1)) { my_value = 1; }
		static bool          RadioButton(const char* label, int* v, int v_button);           // shortcut to handle the above pattern when value is an integer
		static void          ProgressBar(float fraction, const KGVec2& size_arg = KGVec2(-FLT_MIN, 0), const char* overlay = NULL);
		static void          Bullet();                                                       // draw a small circle + keep the cursor on the same line. advance cursor x position by GetTreeNodeToLabelSpacing(), same distance that TreeNode() uses

		// Widgets: Images
		// - Read about KGTextureID here: https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
		static void          Image(KGTextureID user_texture_id, const KGVec2& size, const KGVec2& uv0 = KGVec2(0, 0), const KGVec2& uv1 = KGVec2(1, 1), const KGVec4& tint_col = KGVec4(1, 1, 1, 1), const KGVec4& border_col = KGVec4(0, 0, 0, 0));
		static bool          ImageButton(const char* str_id, KGTextureID user_texture_id, const KGVec2& size, const KGVec2& uv0 = KGVec2(0, 0), const KGVec2& uv1 = KGVec2(1, 1), const KGVec4& bg_col = KGVec4(0, 0, 0, 0), const KGVec4& tint_col = KGVec4(1, 1, 1, 1));

		// Widgets: Combo Box (Dropdown)
		// - The BeginCombo()/EndCombo() api allows you to manage your contents and selection state however you want it, by creating e.g. Selectable() items.
		// - The old Combo() api are helpers over BeginCombo()/EndCombo() which are kept available for convenience purpose. This is analogous to how ListBox are created.
		static bool          BeginCombo(const char* label, const char* preview_value, KarmaGuiComboFlags flags = 0);
		static void          EndCombo(); // only call EndCombo() if BeginCombo() returns true!
		static bool          Combo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1);
		static bool          Combo(const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items = -1);      // Separate items with \0 within a string, end item-list with \0\0. e.g. "One\0Two\0Three\0"
		static bool          Combo(const char* label, int* current_item, bool(*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int popup_max_height_in_items = -1);

		// Widgets: Drag Sliders
		// - CTRL+Click on any drag box to turn them into an input box. Manually input values aren't clamped by default and can go off-bounds. Use KGGuiSliderFlags_AlwaysClamp to always clamp.
		// - For all the Float2/Float3/Float4/Int2/Int3/Int4 versions of every function, note that a 'float v[X]' function argument is the same as 'float* v',
		//   the array syntax is just a way to document the number of elements that are expected to be accessible. You can pass address of your first element out of a contiguous set, e.g. &myvector.x
		// - Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
		// - Format string may also be set to NULL or use the default format ("%f" or "%d").
		// - Speed are per-pixel of mouse movement (v_speed=0.2f: mouse needs to move by 5 pixels to increase value by 1). For gamepad/keyboard navigation, minimum speed is Max(v_speed, minimum_step_at_given_precision).
		// - Use v_min < v_max to clamp edits to given limits. Note that CTRL+Click manual input can override those limits if KGGuiSliderFlags_AlwaysClamp is not used.
		// - Use v_max = FLT_MAX / INT_MAX etc to avoid clamping to a maximum, same with v_min = -FLT_MAX / INT_MIN to avoid clamping to a minimum.
		// - We use the same sets of flags for DragXXX() and SliderXXX() functions as the features are the same and it makes it easier to swap them.
		// - Legacy: Pre-1.78 there are DragXXX() function signatures that take a final `float power=1.0f' argument instead of the `KarmaGuiSliderFlags flags=0' argument.
		//   If you get a warning converting a float to KarmaGuiSliderFlags, read https://github.com/ocornut/imgui/issues/3361
		static bool          DragFloat(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", KarmaGuiSliderFlags flags = 0);     // If v_min >= v_max we have no bound
		static bool          DragFloat2(const char* label, float v[2], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", KarmaGuiSliderFlags flags = 0);
		static bool          DragFloat3(const char* label, float v[3], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", KarmaGuiSliderFlags flags = 0);
		static bool          DragFloat4(const char* label, float v[4], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", KarmaGuiSliderFlags flags = 0);
		static bool          DragFloatRange2(const char* label, float* v_current_min, float* v_current_max, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", const char* format_max = NULL, KarmaGuiSliderFlags flags = 0);
		static bool          DragInt(const char* label, int* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", KarmaGuiSliderFlags flags = 0);  // If v_min >= v_max we have no bound
		static bool          DragInt2(const char* label, int v[2], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", KarmaGuiSliderFlags flags = 0);
		static bool          DragInt3(const char* label, int v[3], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", KarmaGuiSliderFlags flags = 0);
		static bool          DragInt4(const char* label, int v[4], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", KarmaGuiSliderFlags flags = 0);
		static bool          DragIntRange2(const char* label, int* v_current_min, int* v_current_max, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", const char* format_max = NULL, KarmaGuiSliderFlags flags = 0);
		static bool          DragScalar(const char* label, KarmaGuiDataType data_type, void* p_data, float v_speed = 1.0f, const void* p_min = NULL, const void* p_max = NULL, const char* format = NULL, KarmaGuiSliderFlags flags = 0);
		static bool          DragScalarN(const char* label, KarmaGuiDataType data_type, void* p_data, int components, float v_speed = 1.0f, const void* p_min = NULL, const void* p_max = NULL, const char* format = NULL, KarmaGuiSliderFlags flags = 0);

		// Widgets: Regular Sliders
		// - CTRL+Click on any slider to turn them into an input box. Manually input values aren't clamped by default and can go off-bounds. Use KGGuiSliderFlags_AlwaysClamp to always clamp.
		// - Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
		// - Format string may also be set to NULL or use the default format ("%f" or "%d").
		// - Legacy: Pre-1.78 there are SliderXXX() function signatures that take a final `float power=1.0f' argument instead of the `KarmaGuiSliderFlags flags=0' argument.
		//   If you get a warning converting a float to KarmaGuiSliderFlags, read https://github.com/ocornut/imgui/issues/3361
		static bool          SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", KarmaGuiSliderFlags flags = 0);     // adjust format to decorate the value with a prefix or a suffix for in-slider labels or unit display.
		static bool          SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format = "%.3f", KarmaGuiSliderFlags flags = 0);
		static bool          SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format = "%.3f", KarmaGuiSliderFlags flags = 0);
		static bool          SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format = "%.3f", KarmaGuiSliderFlags flags = 0);
		static bool          SliderAngle(const char* label, float* v_rad, float v_degrees_min = -360.0f, float v_degrees_max = +360.0f, const char* format = "%.0f deg", KarmaGuiSliderFlags flags = 0);
		static bool          SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", KarmaGuiSliderFlags flags = 0);
		static bool          SliderInt2(const char* label, int v[2], int v_min, int v_max, const char* format = "%d", KarmaGuiSliderFlags flags = 0);
		static bool          SliderInt3(const char* label, int v[3], int v_min, int v_max, const char* format = "%d", KarmaGuiSliderFlags flags = 0);
		static bool          SliderInt4(const char* label, int v[4], int v_min, int v_max, const char* format = "%d", KarmaGuiSliderFlags flags = 0);
		static bool          SliderScalar(const char* label, KarmaGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, KarmaGuiSliderFlags flags = 0);
		static bool          SliderScalarN(const char* label, KarmaGuiDataType data_type, void* p_data, int components, const void* p_min, const void* p_max, const char* format = NULL, KarmaGuiSliderFlags flags = 0);
		static bool          VSliderFloat(const char* label, const KGVec2& size, float* v, float v_min, float v_max, const char* format = "%.3f", KarmaGuiSliderFlags flags = 0);
		static bool          VSliderInt(const char* label, const KGVec2& size, int* v, int v_min, int v_max, const char* format = "%d", KarmaGuiSliderFlags flags = 0);
		static bool          VSliderScalar(const char* label, const KGVec2& size, KarmaGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, KarmaGuiSliderFlags flags = 0);

		// Widgets: Input with Keyboard
		// - If you want to use InputText() with std::string or any custom dynamic string type, see misc/cpp/imgui_stdlib.h and comments in imgui_demo.cpp.
		// - Most of the KarmaGuiInputTextFlags flags are only useful for InputText() and not for InputFloatX, InputIntX, InputDouble etc.
		static bool          InputText(const char* label, char* buf, size_t buf_size, KarmaGuiInputTextFlags flags = 0, KarmaGuiInputTextCallback callback = NULL, void* user_data = NULL);
		static bool          InputTextMultiline(const char* label, char* buf, size_t buf_size, const KGVec2& size = KGVec2(0, 0), KarmaGuiInputTextFlags flags = 0, KarmaGuiInputTextCallback callback = NULL, void* user_data = NULL);
		static bool          InputTextWithHint(const char* label, const char* hint, char* buf, size_t buf_size, KarmaGuiInputTextFlags flags = 0, KarmaGuiInputTextCallback callback = NULL, void* user_data = NULL);
		static bool          InputFloat(const char* label, float* v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f", KarmaGuiInputTextFlags flags = 0);
		static bool          InputFloat2(const char* label, float v[2], const char* format = "%.3f", KarmaGuiInputTextFlags flags = 0);
		static bool          InputFloat3(const char* label, float v[3], const char* format = "%.3f", KarmaGuiInputTextFlags flags = 0);
		static bool          InputFloat4(const char* label, float v[4], const char* format = "%.3f", KarmaGuiInputTextFlags flags = 0);
		static bool          InputInt(const char* label, int* v, int step = 1, int step_fast = 100, KarmaGuiInputTextFlags flags = 0);
		static bool          InputInt2(const char* label, int v[2], KarmaGuiInputTextFlags flags = 0);
		static bool          InputInt3(const char* label, int v[3], KarmaGuiInputTextFlags flags = 0);
		static bool          InputInt4(const char* label, int v[4], KarmaGuiInputTextFlags flags = 0);
		static bool          InputDouble(const char* label, double* v, double step = 0.0, double step_fast = 0.0, const char* format = "%.6f", KarmaGuiInputTextFlags flags = 0);
		static bool          InputScalar(const char* label, KarmaGuiDataType data_type, void* p_data, const void* p_step = NULL, const void* p_step_fast = NULL, const char* format = NULL, KarmaGuiInputTextFlags flags = 0);
		static bool          InputScalarN(const char* label, KarmaGuiDataType data_type, void* p_data, int components, const void* p_step = NULL, const void* p_step_fast = NULL, const char* format = NULL, KarmaGuiInputTextFlags flags = 0);

		// Widgets: Color Editor/Picker (tip: the ColorEdit* functions have a little color square that can be left-clicked to open a picker, and right-clicked to open an option menu.)
		// - Note that in C++ a 'float v[X]' function argument is the _same_ as 'float* v', the array syntax is just a way to document the number of elements that are expected to be accessible.
		// - You can pass the address of a first float element out of a contiguous structure, e.g. &myvector.x
		static bool          ColorEdit3(const char* label, float col[3], KarmaGuiColorEditFlags flags = 0);
		static bool          ColorEdit4(const char* label, float col[4], KarmaGuiColorEditFlags flags = 0);
		static bool          ColorPicker3(const char* label, float col[3], KarmaGuiColorEditFlags flags = 0);
		static bool          ColorPicker4(const char* label, float col[4], KarmaGuiColorEditFlags flags = 0, const float* ref_col = NULL);
		static bool          ColorButton(const char* desc_id, const KGVec4& col, KarmaGuiColorEditFlags flags = 0, const KGVec2& size = KGVec2(0, 0)); // display a color square/button, hover for details, return true when pressed.
		static void          SetColorEditOptions(KarmaGuiColorEditFlags flags);                     // initialize current options (generally on application startup) if you want to select a default format, picker type, etc. User will be able to change many settings, unless you pass the _NoOptions flag to your calls.

		// Widgets: Trees
		// - TreeNode functions return true when the node is open, in which case you need to also call TreePop() when you are finished displaying the tree node contents.
		static bool          TreeNode(const char* label);
		static bool          TreeNode(const char* str_id, const char* fmt, ...) KG_FMTARGS(2);   // helper variation to easily decorelate the id from the displayed string. Read the FAQ about why and how to use ID. to align arbitrary text at the same level as a TreeNode() you can use Bullet().
		static bool          TreeNode(const void* ptr_id, const char* fmt, ...) KG_FMTARGS(2);   // "
		static bool          TreeNodeV(const char* str_id, const char* fmt, va_list args) KG_FMTLIST(2);
		static bool          TreeNodeV(const void* ptr_id, const char* fmt, va_list args) KG_FMTLIST(2);
		static bool          TreeNodeEx(const char* label, KarmaGuiTreeNodeFlags flags = 0);
		static bool          TreeNodeEx(const char* str_id, KarmaGuiTreeNodeFlags flags, const char* fmt, ...) KG_FMTARGS(3);
		static bool          TreeNodeEx(const void* ptr_id, KarmaGuiTreeNodeFlags flags, const char* fmt, ...) KG_FMTARGS(3);
		static bool          TreeNodeExV(const char* str_id, KarmaGuiTreeNodeFlags flags, const char* fmt, va_list args) KG_FMTLIST(3);
		static bool          TreeNodeExV(const void* ptr_id, KarmaGuiTreeNodeFlags flags, const char* fmt, va_list args) KG_FMTLIST(3);
		static void          TreePush(const char* str_id);                                       // ~ Indent()+PushId(). Already called by TreeNode() when returning true, but you can call TreePush/TreePop yourself if desired.
		static void          TreePush(const void* ptr_id);                                       // "
		static void          TreePop();                                                          // ~ Unindent()+PopId()
		static float         GetTreeNodeToLabelSpacing();                                        // horizontal distance preceding label when using TreeNode*() or Bullet() == (g.FontSize + style.FramePadding.x*2) for a regular unframed TreeNode
		static bool          CollapsingHeader(const char* label, KarmaGuiTreeNodeFlags flags = 0);  // if returning 'true' the header is open. doesn't indent nor push on ID stack. user doesn't have to call TreePop().
		static bool          CollapsingHeader(const char* label, bool* p_visible, KarmaGuiTreeNodeFlags flags = 0); // when 'p_visible != NULL': if '*p_visible==true' display an additional small close button on upper right of the header which will set the bool to false when clicked, if '*p_visible==false' don't display the header.
		static void          SetNextItemOpen(bool is_open, KarmaGuiCond cond = 0);                  // set next TreeNode/CollapsingHeader open state.

		// Widgets: Selectables
		// - A selectable highlights when hovered, and can display another color when selected.
		// - Neighbors selectable extend their highlight bounds in order to leave no gap between them. This is so a series of selected Selectable appear contiguous.
		static bool          Selectable(const char* label, bool selected = false, KarmaGuiSelectableFlags flags = 0, const KGVec2& size = KGVec2(0, 0)); // "bool selected" carry the selection state (read-only). Selectable() is clicked is returns true so you can modify your selection state. size.x==0.0: use remaining width, size.x>0.0: specify width. size.y==0.0: use label height, size.y>0.0: specify height
		static bool          Selectable(const char* label, bool* p_selected, KarmaGuiSelectableFlags flags = 0, const KGVec2& size = KGVec2(0, 0));      // "bool* p_selected" point to the selection state (read-write), as a convenient helper.

		// Widgets: List Boxes
		// - This is essentially a thin wrapper to using BeginChild/EndChild with some stylistic changes.
		// - The BeginListBox()/EndListBox() api allows you to manage your contents and selection state however you want it, by creating e.g. Selectable() or any items.
		// - The simplified/old ListBox() api are helpers over BeginListBox()/EndListBox() which are kept available for convenience purpose. This is analoguous to how Combos are created.
		// - Choose frame width:   size.x > 0.0f: custom  /  size.x < 0.0f or -FLT_MIN: right-align   /  size.x = 0.0f (default): use current ItemWidth
		// - Choose frame height:  size.y > 0.0f: custom  /  size.y < 0.0f or -FLT_MIN: bottom-align  /  size.y = 0.0f (default): arbitrary default height which can fit ~7 items
		static bool          BeginListBox(const char* label, const KGVec2& size = KGVec2(0, 0)); // open a framed scrolling region
		static void          EndListBox();                                                       // only call EndListBox() if BeginListBox() returned true!
		static bool          ListBox(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items = -1);
		static bool          ListBox(const char* label, int* current_item, bool (*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int height_in_items = -1);

		// Widgets: Data Plotting
		// - Consider using ImPlot (https://github.com/epezent/implot) which is much better!
		static void          PlotLines(const char* label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, KGVec2 graph_size = KGVec2(0, 0), int stride = sizeof(float));
		static void          PlotLines(const char* label, float(*values_getter)(void* data, int idx), void* data, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, KGVec2 graph_size = KGVec2(0, 0));
		static void          PlotHistogram(const char* label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, KGVec2 graph_size = KGVec2(0, 0), int stride = sizeof(float));
		static void          PlotHistogram(const char* label, float(*values_getter)(void* data, int idx), void* data, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, KGVec2 graph_size = KGVec2(0, 0));

		// Widgets: Value() Helpers.
		// - Those are merely shortcut to calling Text() with a format string. Output single value in "name: value" format (tip: freely declare more in your code to handle your types. you can add functions to the ImGui namespace)
		static void          Value(const char* prefix, bool b);
		static void          Value(const char* prefix, int v);
		static void          Value(const char* prefix, unsigned int v);
		static void          Value(const char* prefix, float v, const char* float_format = NULL);

		// Widgets: Menus
		// - Use BeginMenuBar() on a window KGGuiWindowFlags_MenuBar to append to its menu bar.
		// - Use BeginMainMenuBar() to create a menu bar at the top of the screen and append to it.
		// - Use BeginMenu() to create a menu. You can call BeginMenu() multiple time with the same identifier to append more items to it.
		// - Not that MenuItem() keyboardshortcuts are displayed as a convenience but _not processed_ by Dear ImGui at the moment.
		static bool          BeginMenuBar();                                                     // append to menu-bar of current window (requires KGGuiWindowFlags_MenuBar flag set on parent window).
		static void          EndMenuBar();                                                       // only call EndMenuBar() if BeginMenuBar() returns true!
		static bool          BeginMainMenuBar();                                                 // create and append to a full screen menu-bar.
		static void          EndMainMenuBar();                                                   // only call EndMainMenuBar() if BeginMainMenuBar() returns true!
		static bool          BeginMenu(const char* label, bool enabled = true);                  // create a sub-menu entry. only call EndMenu() if this returns true!
		static void          EndMenu();                                                          // only call EndMenu() if BeginMenu() returns true!
		static bool          MenuItem(const char* label, const char* shortcut = NULL, bool selected = false, bool enabled = true);  // return true when activated.
		static bool          MenuItem(const char* label, const char* shortcut, bool* p_selected, bool enabled = true);              // return true when activated + toggle (*p_selected) if p_selected != NULL

		// Tooltips
		// - Tooltip are windows following the mouse. They do not take focus away.
		static void          BeginTooltip();                                                     // begin/append a tooltip window. to create full-featured tooltip (with any kind of items).
		static void          EndTooltip();
		static void          SetTooltip(const char* fmt, ...) KG_FMTARGS(1);                     // set a text-only tooltip, typically use with ImGui::IsItemHovered(). override any previous call to SetTooltip().
		static void          SetTooltipV(const char* fmt, va_list args) KG_FMTLIST(1);

		// Popups, Modals
		//  - They block normal mouse hovering detection (and therefore most mouse interactions) behind them.
		//  - If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
		//  - Their visibility state (~bool) is held internally instead of being held by the programmer as we are used to with regular Begin*() calls.
		//  - The 3 properties above are related: we need to retain popup visibility state in the library because popups may be closed as any time.
		//  - You can bypass the hovering restriction by using KGGuiHoveredFlags_AllowWhenBlockedByPopup when calling IsItemHovered() or IsWindowHovered().
		//  - IMPORTANT: Popup identifiers are relative to the current ID stack, so OpenPopup and BeginPopup generally needs to be at the same level of the stack.
		//    This is sometimes leading to confusing mistakes. May rework this in the future.

		// Popups: begin/end functions
		//  - BeginPopup(): query popup state, if open start appending into the window. Call EndPopup() afterwards. KarmaGuiWindowFlags are forwarded to the window.
		//  - BeginPopupModal(): block every interaction behind the window, cannot be closed by user, add a dimming background, has a title bar.
		static bool          BeginPopup(const char* str_id, KarmaGuiWindowFlags flags = 0);                         // return true if the popup is open, and you can start outputting to it.
		static bool          BeginPopupModal(const char* name, bool* p_open = NULL, KarmaGuiWindowFlags flags = 0); // return true if the modal is open, and you can start outputting to it.
		static void          EndPopup();                                                                         // only call EndPopup() if BeginPopupXXX() returns true!

		// Popups: open/close functions
		//  - OpenPopup(): set popup state to open. KarmaGuiPopupFlags are available for opening options.
		//  - If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
		//  - CloseCurrentPopup(): use inside the BeginPopup()/EndPopup() scope to close manually.
		//  - CloseCurrentPopup() is called by default by Selectable()/MenuItem() when activated (FIXME: need some options).
		//  - Use KGGuiPopupFlags_NoOpenOverExistingPopup to avoid opening a popup if there's already one at the same level. This is equivalent to e.g. testing for !IsAnyPopupOpen() prior to OpenPopup().
		//  - Use IsWindowAppearing() after BeginPopup() to tell if a window just opened.
		//  - IMPORTANT: Notice that for OpenPopupOnItemClick() we exceptionally default flags to 1 (== KGGuiPopupFlags_MouseButtonRight) for backward compatibility with older API taking 'int mouse_button = 1' parameter
		static void          OpenPopup(const char* str_id, KarmaGuiPopupFlags popup_flags = 0);                     // call to mark popup as open (don't call every frame!).
		static void          OpenPopup(KGGuiID id, KarmaGuiPopupFlags popup_flags = 0);                             // id overload to facilitate calling from nested stacks
		static void          OpenPopupOnItemClick(const char* str_id = NULL, KarmaGuiPopupFlags popup_flags = 1);   // helper to open popup when clicked on last item. Default to KGGuiPopupFlags_MouseButtonRight == 1. (note: actually triggers on the mouse _released_ event to be consistent with popup behaviors)
		static void          CloseCurrentPopup();                                                                // manually close the popup we have begin-ed into.

		// Popups: open+begin combined functions helpers
		//  - Helpers to do OpenPopup+BeginPopup where the Open action is triggered by e.g. hovering an item and right-clicking.
		//  - They are convenient to easily create context menus, hence the name.
		//  - IMPORTANT: Notice that BeginPopupContextXXX takes KarmaGuiPopupFlags just like OpenPopup() and unlike BeginPopup(). For full consistency, we may add KarmaGuiWindowFlags to the BeginPopupContextXXX functions in the future.
		//  - IMPORTANT: Notice that we exceptionally default their flags to 1 (== KGGuiPopupFlags_MouseButtonRight) for backward compatibility with older API taking 'int mouse_button = 1' parameter, so if you add other flags remember to re-add the KGGuiPopupFlags_MouseButtonRight.
		static bool          BeginPopupContextItem(const char* str_id = NULL, KarmaGuiPopupFlags popup_flags = 1);  // open+begin popup when clicked on last item. Use str_id==NULL to associate the popup to previous item. If you want to use that on a non-interactive item such as Text() you need to pass in an explicit ID here. read comments in .cpp!
		static bool          BeginPopupContextWindow(const char* str_id = NULL, KarmaGuiPopupFlags popup_flags = 1);// open+begin popup when clicked on current window.
		static bool          BeginPopupContextVoid(const char* str_id = NULL, KarmaGuiPopupFlags popup_flags = 1);  // open+begin popup when clicked in void (where there are no windows).

		// Popups: query functions
		//  - IsPopupOpen(): return true if the popup is open at the current BeginPopup() level of the popup stack.
		//  - IsPopupOpen() with KGGuiPopupFlags_AnyPopupId: return true if any popup is open at the current BeginPopup() level of the popup stack.
		//  - IsPopupOpen() with KGGuiPopupFlags_AnyPopupId + KGGuiPopupFlags_AnyPopupLevel: return true if any popup is open.
		static bool          IsPopupOpen(const char* str_id, KarmaGuiPopupFlags flags = 0);                         // return true if the popup is open.

		// Tables
		// - Full-featured replacement for old Columns API.
		// - See Demo->Tables for demo code. See top of imgui_tables.cpp for general commentary.
		// - See KGGuiTableFlags_ and KGGuiTableColumnFlags_ enums for a description of available flags.
		// The typical call flow is:
		// - 1. Call BeginTable(), early out if returning false.
		// - 2. Optionally call TableSetupColumn() to submit column name/flags/defaults.
		// - 3. Optionally call TableSetupScrollFreeze() to request scroll freezing of columns/rows.
		// - 4. Optionally call TableHeadersRow() to submit a header row. Names are pulled from TableSetupColumn() data.
		// - 5. Populate contents:
		//    - In most situations you can use TableNextRow() + TableSetColumnIndex(N) to start appending into a column.
		//    - If you are using tables as a sort of grid, where every column is holding the same type of contents,
		//      you may prefer using TableNextColumn() instead of TableNextRow() + TableSetColumnIndex().
		//      TableNextColumn() will automatically wrap-around into the next row if needed.
		//    - IMPORTANT: Comparatively to the old Columns() API, we need to call TableNextColumn() for the first column!
		//    - Summary of possible call flow:
		//        --------------------------------------------------------------------------------------------------------
		//        TableNextRow() -> TableSetColumnIndex(0) -> Text("Hello 0") -> TableSetColumnIndex(1) -> Text("Hello 1")  // OK
		//        TableNextRow() -> TableNextColumn()      -> Text("Hello 0") -> TableNextColumn()      -> Text("Hello 1")  // OK
		//                          TableNextColumn()      -> Text("Hello 0") -> TableNextColumn()      -> Text("Hello 1")  // OK: TableNextColumn() automatically gets to next row!
		//        TableNextRow()                           -> Text("Hello 0")                                               // Not OK! Missing TableSetColumnIndex() or TableNextColumn()! Text will not appear!
		//        --------------------------------------------------------------------------------------------------------
		// - 5. Call EndTable()
		static bool          BeginTable(const char* str_id, int column, KarmaGuiTableFlags flags = 0, const KGVec2& outer_size = KGVec2(0.0f, 0.0f), float inner_width = 0.0f);
		static void          EndTable();                                         // only call EndTable() if BeginTable() returns true!
		static void          TableNextRow(KarmaGuiTableRowFlags row_flags = 0, float min_row_height = 0.0f); // append into the first cell of a new row.
		static bool          TableNextColumn();                                  // append into the next column (or first column of next row if currently in last column). Return true when column is visible.
		static bool          TableSetColumnIndex(int column_n);                  // append into the specified column. Return true when column is visible.

		// Tables: Headers & Columns declaration
		// - Use TableSetupColumn() to specify label, resizing policy, default width/weight, id, various other flags etc.
		// - Use TableHeadersRow() to create a header row and automatically submit a TableHeader() for each column.
		//   Headers are required to perform: reordering, sorting, and opening the context menu.
		//   The context menu can also be made available in columns body using KGGuiTableFlags_ContextMenuInBody.
		// - You may manually submit headers using TableNextRow() + TableHeader() calls, but this is only useful in
		//   some advanced use cases (e.g. adding custom widgets in header row).
		// - Use TableSetupScrollFreeze() to lock columns/rows so they stay visible when scrolled.
		static void          TableSetupColumn(const char* label, KarmaGuiTableColumnFlags flags = 0, float init_width_or_weight = 0.0f, KGGuiID user_id = 0);
		static void          TableSetupScrollFreeze(int cols, int rows);         // lock columns/rows so they stay visible when scrolled.
		static void          TableHeadersRow();                                  // submit all headers cells based on data provided to TableSetupColumn() + submit context menu
		static void          TableHeader(const char* label);                     // submit one header cell manually (rarely used)

		// Tables: Sorting & Miscellaneous functions
		// - Sorting: call TableGetSortSpecs() to retrieve latest sort specs for the table. NULL when not sorting.
		//   When 'sort_specs->SpecsDirty == true' you should sort your data. It will be true when sorting specs have
		//   changed since last call, or the first time. Make sure to set 'SpecsDirty = false' after sorting,
		//   else you may wastefully sort your data every frame!
		// - Functions args 'int column_n' treat the default value of -1 as the same as passing the current column index.
		static KarmaGuiTableSortSpecs*  TableGetSortSpecs();                        // get latest sort specs for the table (NULL if not sorting).  Lifetime: don't hold on this pointer over multiple frames or past any subsequent call to BeginTable().
		static int                   TableGetColumnCount();                      // return number of columns (value passed to BeginTable)
		static int                   TableGetColumnIndex();                      // return current column index.
		static int                   TableGetRowIndex();                         // return current row index.
		static const char*           TableGetColumnName(int column_n = -1);      // return "" if column didn't have a name declared by TableSetupColumn(). Pass -1 to use current column.
		static KarmaGuiTableColumnFlags TableGetColumnFlags(int column_n = -1);     // return column flags so you can query their Enabled/Visible/Sorted/Hovered status flags. Pass -1 to use current column.
		static void                  TableSetColumnEnabled(int column_n, bool v);// change user accessible enabled/disabled state of a column. Set to false to hide the column. User can use the context menu to change this themselves (right-click in headers, or right-click in columns body with KGGuiTableFlags_ContextMenuInBody)
		static void                  TableSetBgColor(KarmaGuiTableBgTarget target, KGU32 color, int column_n = -1);  // change the color of a cell, row, or column. See KGGuiTableBgTarget_ flags for details.

		// Legacy Columns API (prefer using Tables!)
		// - You can also use SameLine(pos_x) to mimic simplified columns.
		static void          Columns(int count = 1, const char* id = NULL, bool border = true);
		static void          NextColumn();                                                       // next column, defaults to current row or next row if the current row is finished
		static int           GetColumnIndex();                                                   // get current column index
		static float         GetColumnWidth(int column_index = -1);                              // get column width (in pixels). pass -1 to use current column
		static void          SetColumnWidth(int column_index, float width);                      // set column width (in pixels). pass -1 to use current column
		static float         GetColumnOffset(int column_index = -1);                             // get position of column line (in pixels, from the left side of the contents region). pass -1 to use current column, otherwise 0..GetColumnsCount() inclusive. column 0 is typically 0.0f
		static void          SetColumnOffset(int column_index, float offset_x);                  // set position of column line (in pixels, from the left side of the contents region). pass -1 to use current column
		static int           GetColumnsCount();

		// Tab Bars, Tabs
		// - Note: Tabs are automatically created by the docking system (when in 'docking' branch). Use this to create tab bars/tabs yourself.
		static bool          BeginTabBar(const char* str_id, KarmaGuiTabBarFlags flags = 0);        // create and append into a TabBar
		static void          EndTabBar();                                                        // only call EndTabBar() if BeginTabBar() returns true!
		static bool          BeginTabItem(const char* label, bool* p_open = NULL, KarmaGuiTabItemFlags flags = 0); // create a Tab. Returns true if the Tab is selected.
		static void          EndTabItem();                                                       // only call EndTabItem() if BeginTabItem() returns true!
		static bool          TabItemButton(const char* label, KarmaGuiTabItemFlags flags = 0);      // create a Tab behaving like a button. return true when clicked. cannot be selected in the tab bar.
		static void          SetTabItemClosed(const char* tab_or_docked_window_label);           // notify TabBar or Docking system of a closed tab/window ahead (useful to reduce visual flicker on reorderable tab bars). For tab-bar: call after BeginTabBar() and before Tab submissions. Otherwise call with a window name.

		// Docking
		// [BETA API] Enable with io.ConfigFlags |= KGGuiConfigFlags_DockingEnable.
		// Note: You can use most Docking facilities without calling any API. You DO NOT need to call DockSpace() to use Docking!
		// - Drag from window title bar or their tab to dock/undock. Hold SHIFT to disable docking/undocking.
		// - Drag from window menu button (upper-left button) to undock an entire node (all windows).
		// - When io.ConfigDockingWithShift == true, you instead need to hold SHIFT to _enable_ docking/undocking.
		// About dockspaces:
		// - Use DockSpace() to create an explicit dock node _within_ an existing window. See Docking demo for details.
		// - Use DockSpaceOverViewport() to create an explicit dock node covering the screen or a specific viewport.
		//   This is often used with KGGuiDockNodeFlags_PassthruCentralNode.
		// - Important: Dockspaces need to be submitted _before_ any window they can host. Submit it early in your frame!
		// - Important: Dockspaces need to be kept alive if hidden, otherwise windows docked into it will be undocked.
		//   e.g. if you have multiple tabs with a dockspace inside each tab: submit the non-visible dockspaces with KGGuiDockNodeFlags_KeepAliveOnly.
		static KGGuiID       DockSpace(KGGuiID id, const KGVec2& size = KGVec2(0, 0), KarmaGuiDockNodeFlags flags = 0, const KarmaGuiWindowClass* window_class = NULL);
		static KGGuiID       DockSpaceOverViewport(const KarmaGuiViewport* viewport = NULL, KarmaGuiDockNodeFlags flags = 0, const KarmaGuiWindowClass* window_class = NULL);
		static void          SetNextWindowDockID(KGGuiID dock_id, KarmaGuiCond cond = 0);           // set next window dock id
		static void          SetNextWindowClass(const KarmaGuiWindowClass* window_class);           // set next window class (control docking compatibility + provide hints to platform backend via custom viewport flags and platform parent/child relationship)
		static KGGuiID       GetWindowDockID();
		static bool          IsWindowDocked();                                                   // is current window docked into another window?

		// Logging/Capture
		// - All text output from the interface can be captured into tty/file/clipboard. By default, tree nodes are automatically opened during logging.
		static void          LogToTTY(int auto_open_depth = -1);                                 // start logging to tty (stdout)
		static void          LogToFile(int auto_open_depth = -1, const char* filename = NULL);   // start logging to file
		static void          LogToClipboard(int auto_open_depth = -1);                           // start logging to OS clipboard
		static void          LogFinish();                                                        // stop logging (close file, etc.)
		static void          LogButtons();                                                       // helper to display buttons for logging to tty/file/clipboard
		static void          LogText(const char* fmt, ...) KG_FMTARGS(1);                        // pass text data straight to log (without being displayed)
		static void          LogTextV(const char* fmt, va_list args) KG_FMTLIST(1);
		static void LogTextV(KarmaGuiContext& g, const char* fmt, va_list args);

		// Drag and Drop
		// - On source items, call BeginDragDropSource(), if it returns true also call SetDragDropPayload() + EndDragDropSource().
		// - On target candidates, call BeginDragDropTarget(), if it returns true also call AcceptDragDropPayload() + EndDragDropTarget().
		// - If you stop calling BeginDragDropSource() the payload is preserved however it won't have a preview tooltip (we currently display a fallback "..." tooltip, see #1725)
		// - An item can be both drag source and drop target.
		static bool          BeginDragDropSource(KarmaGuiDragDropFlags flags = 0);                                      // call after submitting an item which may be dragged. when this return true, you can call SetDragDropPayload() + EndDragDropSource()
		static bool          SetDragDropPayload(const char* type, const void* data, size_t sz, KarmaGuiCond cond = 0);  // type is a user defined string of maximum 32 characters. Strings starting with '_' are reserved for dear imgui internal types. Data is copied and held by imgui. Return true when payload has been accepted.
		static void          EndDragDropSource();                                                                    // only call EndDragDropSource() if BeginDragDropSource() returns true!
		static bool                  BeginDragDropTarget();                                                          // call after submitting an item that may receive a payload. If this returns true, you can call AcceptDragDropPayload() + EndDragDropTarget()
		static const KarmaGuiPayload*   AcceptDragDropPayload(const char* type, KarmaGuiDragDropFlags flags = 0);          // accept contents of a given type. If KGGuiDragDropFlags_AcceptBeforeDelivery is set you can peek into the payload before the mouse button is released.
		static void                  EndDragDropTarget();                                                            // only call EndDragDropTarget() if BeginDragDropTarget() returns true!
		static const KarmaGuiPayload*   GetDragDropPayload();                                                           // peek directly into the current payload from anywhere. may return NULL. use KarmaGuiPayload::IsDataType() to test for the payload type.

		// Disabling [BETA API]
		// - Disable all user interactions and dim items visuals (applying style.DisabledAlpha over current colors)
		// - Those can be nested but it cannot be used to enable an already disabled section (a single BeginDisabled(true) in the stack is enough to keep everything disabled)
		// - BeginDisabled(false) essentially does nothing useful but is provided to facilitate use of boolean expressions. If you can avoid calling BeginDisabled(False)/EndDisabled() best to avoid it.
		static void          BeginDisabled(bool disabled = true);
		static void          EndDisabled();

		// Clipping
		// - Mouse hovering is affected by ImGui::PushClipRect() calls, unlike direct calls to KGDrawList::PushClipRect() which are render only.
		static void          PushClipRect(const KGVec2& clip_rect_min, const KGVec2& clip_rect_max, bool intersect_with_current_clip_rect);
		static void          PopClipRect();

		// Focus, Activation
		// - Prefer using "SetItemDefaultFocus()" over "if (IsWindowAppearing()) SetScrollHereY()" when applicable to signify "this is the default item"
		static void          SetItemDefaultFocus();                                              // make last item the default focused item of a window.
		static void          SetKeyboardFocusHere(int offset = 0);                               // focus keyboard on the next widget. Use positive 'offset' to access sub components of a multiple component widget. Use -1 to access previous widget.

		// Item/Widgets Utilities and Query Functions
		// - Most of the functions are referring to the previous Item that has been submitted.
		// - See Demo Window under "Widgets->Querying Status" for an interactive visualization of most of those functions.
		static bool          IsItemHovered(KarmaGuiHoveredFlags flags = 0);                         // is the last item hovered? (and usable, aka not blocked by a popup, etc.). See KarmaGuiHoveredFlags for more options.
		static bool          IsItemActive();                                                     // is the last item active? (e.g. button being held, text field being edited. This will continuously return true while holding mouse button on an item. Items that don't interact will always return false)
		static bool          IsItemFocused();                                                    // is the last item focused for keyboard/gamepad navigation?
		static bool          IsItemClicked(KarmaGuiMouseButton mouse_button = 0);                   // is the last item hovered and mouse clicked on? (**)  == IsMouseClicked(mouse_button) && IsItemHovered()Important. (**) this is NOT equivalent to the behavior of e.g. Button(). Read comments in function definition.
		static bool          IsItemVisible();                                                    // is the last item visible? (items may be out of sight because of clipping/scrolling)
		static bool          IsItemEdited();                                                     // did the last item modify its underlying value this frame? or was pressed? This is generally the same as the "bool" return value of many widgets.
		static bool          IsItemActivated();                                                  // was the last item just made active (item was previously inactive).
		static bool          IsItemDeactivated();                                                // was the last item just made inactive (item was previously active). Useful for Undo/Redo patterns with widgets that require continuous editing.
		static bool          IsItemDeactivatedAfterEdit();                                       // was the last item just made inactive and made a value change when it was active? (e.g. Slider/Drag moved). Useful for Undo/Redo patterns with widgets that require continuous editing. Note that you may get false positives (some widgets such as Combo()/ListBox()/Selectable() will return true even when clicking an already selected item).
		static bool          IsItemToggledOpen();                                                // was the last item open state toggled? set by TreeNode().
		static bool          IsAnyItemHovered();                                                 // is any item hovered?
		static bool          IsAnyItemActive();                                                  // is any item active?
		static bool          IsAnyItemFocused();                                                 // is any item focused?
		static KGGuiID       GetItemID();                                                        // get ID of last item (~~ often same ImGui::GetID(label) beforehand)
		static KGVec2        GetItemRectMin();                                                   // get upper-left bounding rectangle of the last item (screen space)
		static KGVec2        GetItemRectMax();                                                   // get lower-right bounding rectangle of the last item (screen space)
		static KGVec2        GetItemRectSize();                                                  // get size of last item
		static void          SetItemAllowOverlap();                                              // allow last item to be overlapped by a subsequent item. sometimes useful with invisible buttons, selectables, etc. to catch unused area.

		// Viewports
		// - Currently represents the Platform Window created by the application which is hosting our Dear ImGui windows.
		// - In 'docking' branch with multi-viewport enabled, we extend this concept to have multiple active viewports.
		// - In the future we will extend this concept further to also represent Platform Monitor and support a "no main platform window" operation mode.
		static KarmaGuiViewport* GetMainViewport();                                                 // return primary/default viewport. This can never be NULL.

		// Background/Foreground Draw Lists
		static KGDrawList*   GetBackgroundDrawList();                                            // get background draw list for the viewport associated to the current window. this draw list will be the first rendering one. Useful to quickly draw shapes/text behind dear imgui contents.
		static KGDrawList*   GetForegroundDrawList();                                            // get foreground draw list for the viewport associated to the current window. this draw list will be the last rendered one. Useful to quickly draw shapes/text over dear imgui contents.
		static KGDrawList*   GetBackgroundDrawList(KarmaGuiViewport* viewport);                     // get background draw list for the given viewport. this draw list will be the first rendering one. Useful to quickly draw shapes/text behind dear imgui contents.
		static KGDrawList*   GetForegroundDrawList(KarmaGuiViewport* viewport);                     // get foreground draw list for the given viewport. this draw list will be the last rendered one. Useful to quickly draw shapes/text over dear imgui contents.

		// Miscellaneous Utilities
		static bool          IsRectVisible(const KGVec2& size);                                  // test if rectangle (of given size, starting from cursor position) is visible / not clipped.
		static bool          IsRectVisible(const KGVec2& rect_min, const KGVec2& rect_max);      // test if rectangle (in screen space) is visible / not clipped. to perform coarse clipping on user's side.
		static double        GetTime();                                                          // get global imgui time. incremented by io.DeltaTime every frame.
		static int           GetFrameCount();                                                    // get global imgui frame count. incremented by 1 every frame.
		static KGDrawListSharedData* GetDrawListSharedData();                                    // you may use this when creating your own KGDrawList instances.
		static const char*   GetStyleColorName(KarmaGuiCol idx);                                    // get a string corresponding to the enum value (for display, saving, etc.).
		static void          SetStateStorage(KarmaGuiStorage* storage);                             // replace current window storage with our own (if you want to manipulate it yourself, typically clear subsection of it)
		static KarmaGuiStorage* GetStateStorage();
		static bool          BeginChildFrame(KGGuiID id, const KGVec2& size, KarmaGuiWindowFlags flags = 0); // helper to create a child window / scrolling region that looks like a normal widget frame
		static void          EndChildFrame();                                                    // always call EndChildFrame() regardless of BeginChildFrame() return values (which indicates a collapsed/clipped window)

		// Text Utilities
		static KGVec2        CalcTextSize(const char* text, const char* text_end = NULL, bool hide_text_after_double_hash = false, float wrap_width = -1.0f);

		// Color Utilities
		static KGVec4        ColorConvertU32ToFloat4(KGU32 in);
		static KGU32         ColorConvertFloat4ToU32(const KGVec4& in);
		static void          ColorConvertRGBtoHSV(float r, float g, float b, float& out_h, float& out_s, float& out_v);
		static void          ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g, float& out_b);

		// Inputs Utilities: Keyboard/Mouse/Gamepad
		// - the KarmaGuiKey enum contains all possible keyboard, mouse and gamepad inputs (e.g. KGGuiKey_A, KGGuiKey_MouseLeft, KGGuiKey_GamepadDpadUp...).
		// - before v1.87, we used KarmaGuiKey to carry native/user indices as defined by each backends. About use of those legacy KarmaGuiKey values:
		//  - without IMGUI_DISABLE_OBSOLETE_KEYIO (legacy support): you can still use your legacy native/user indices (< 512) according to how your backend/engine stored them in io.KeysDown[], but need to cast them to KarmaGuiKey.
		//  - with    IMGUI_DISABLE_OBSOLETE_KEYIO (this is the way forward): any use of KarmaGuiKey will assert with key < 512. GetKeyIndex() is pass-through and therefore deprecated (gone if IMGUI_DISABLE_OBSOLETE_KEYIO is defined).
		static bool          IsKeyDown(KarmaGuiKey key);                                            // is key being held.
		static bool          IsKeyPressed(KarmaGuiKey key, bool repeat = true);                     // was key pressed (went from !Down to Down)? if repeat=true, uses io.KeyRepeatDelay / KeyRepeatRate
		static bool          IsKeyReleased(KarmaGuiKey key);                                        // was key released (went from Down to !Down)?
		static int           GetKeyPressedAmount(KarmaGuiKey key, float repeat_delay, float rate);  // uses provided repeat rate/delay. return a count, most often 0 or 1 but might be >1 if RepeatRate is small enough that DeltaTime > RepeatRate
		static const char*   GetKeyName(KarmaGuiKey key);                                           // [DEBUG] returns English name of the key. Those names a provided for debugging purpose and are not meant to be saved persistently not compared.
		static void          SetNextFrameWantCaptureKeyboard(bool want_capture_keyboard);        // Override io.WantCaptureKeyboard flag next frame (said flag is left for your application to handle, typically when true it instructs your app to ignore inputs). e.g. force capture keyboard when your widget is being hovered. This is equivalent to setting "io.WantCaptureKeyboard = want_capture_keyboard"; after the next NewFrame() call.

		// Inputs Utilities: Shortcut testing (with Routing Resolution)
		// - KarmaGuiKeyChord = a KarmaGuiKey optionally OR-red with KGGuiMod_Alt/KGGuiMod_Ctrl/KGGuiMod_Shift/KGGuiMod_Super/KGGuiMod_Shortcut.
		//     KGGuiKey_C                 (accepted by functions taking KarmaGuiKey or KarmaGuiKeyChord)
		//     KGGuiKey_C | KGGuiMod_Ctrl (accepted by functions taking KarmaGuiKeyChord)
		//   ONLY KGGuiMod_XXX values are legal to 'OR' with an KarmaGuiKey. You CANNOT 'OR' two KarmaGuiKey values.
		// - The general idea of routing is that multiple locations may register interest in a shortcut,
		//   and only one location will be granted access to the shortcut.
		// - The default routing policy (KGGuiInputFlags_RouteFocused) checks for current window being in
		//   the focus stack, and route the shortcut to the deepest requesting window in the focus stack.
		// - Consider Shortcut() to be a widget: the calling location matters + it has side-effects as shortcut routes are
		//   registered into the system (for it to be able to pick the best one). This is why this is not called 'IsShortcutPressed()'.
		// - If this is called for a specific widget, pass its ID as 'owner_id' in order for key ownership and routing priorities
		//   to be honored (e.g. with default KGGuiInputFlags_RouteFocused, the highest priority is given to active item).
		static bool          Shortcut(KarmaGuiKeyChord key_chord, KGGuiID owner_id = 0, KarmaGuiInputFlags flags = 0);

		// Inputs Utilities: Mouse specific
		// - To refer to a mouse button, you may use named enums in your code e.g. KGGuiMouseButton_Left, KGGuiMouseButton_Right.
		// - You can also use regular integer: it is forever guaranteed that 0=Left, 1=Right, 2=Middle.
		// - Dragging operations are only reported after mouse has moved a certain distance away from the initial clicking position (see 'lock_threshold' and 'io.MouseDraggingThreshold')
		static bool          IsMouseDown(KarmaGuiMouseButton button);                               // is mouse button held?
		static bool          IsMouseClicked(KarmaGuiMouseButton button, bool repeat = false);       // did mouse button clicked? (went from !Down to Down). Same as GetMouseClickedCount() == 1.
		static bool          IsMouseReleased(KarmaGuiMouseButton button);                           // did mouse button released? (went from Down to !Down)
		static bool          IsMouseDoubleClicked(KarmaGuiMouseButton button);                      // did mouse button double-clicked? Same as GetMouseClickedCount() == 2. (note that a double-click will also report IsMouseClicked() == true)
		static int           GetMouseClickedCount(KarmaGuiMouseButton button);                      // return the number of successive mouse-clicks at the time where a click happen (otherwise 0).
		static bool          IsMouseHoveringRect(const KGVec2& r_min, const KGVec2& r_max, bool clip = true);// is mouse hovering given bounding rect (in screen space). clipped by current clipping settings, but disregarding of other consideration of focus/window ordering/popup-block.
		static bool          IsMousePosValid(const KGVec2* mouse_pos = NULL);                    // by convention we use (-FLT_MAX,-FLT_MAX) to denote that there is no mouse available
		static bool          IsAnyMouseDown();                                                   // [WILL OBSOLETE] is any mouse button held? This was designed for backends, but prefer having backend maintain a mask of held mouse buttons, because upcoming input queue system will make this invalid.
		static KGVec2        GetMousePos();                                                      // shortcut to ImGui::GetIO().MousePos provided by user, to be consistent with other calls
		static KGVec2        GetMousePosOnOpeningCurrentPopup();                                 // retrieve mouse position at the time of opening popup we have BeginPopup() into (helper to avoid user backing that value themselves)
		static bool          IsMouseDragging(KarmaGuiMouseButton button, float lock_threshold = -1.0f);         // is mouse dragging? (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold)
		static KGVec2        GetMouseDragDelta(KarmaGuiMouseButton button = 0, float lock_threshold = -1.0f);   // return the delta from the initial clicking position while the mouse button is pressed or was just released. This is locked and return 0.0f until the mouse moves past a distance threshold at least once (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold)
		static void          ResetMouseDragDelta(KarmaGuiMouseButton button = 0);                   //
		static KarmaGuiMouseCursor GetMouseCursor();                                                // get desired mouse cursor shape. Important: reset in ImGui::NewFrame(), this is updated during the frame. valid before Render(). If you use software rendering by setting io.MouseDrawCursor ImGui will render those for you
		static void          SetMouseCursor(KarmaGuiMouseCursor cursor_type);                       // set desired mouse cursor shape
		static void          SetNextFrameWantCaptureMouse(bool want_capture_mouse);              // Override io.WantCaptureMouse flag next frame (said flag is left for your application to handle, typical when true it instucts your app to ignore inputs). This is equivalent to setting "io.WantCaptureMouse = want_capture_mouse;" after the next NewFrame() call.

		// Clipboard Utilities
		// - Also see the LogToClipboard() function to capture GUI into clipboard, or easily output text data to the clipboard.
		static const char*   GetClipboardText();
		static void          SetClipboardText(const char* text);

		// Settings/.Ini Utilities
		// - The disk functions are automatically called if io.IniFilename != NULL (default is "kggui.ini").
		// - Set io.IniFilename to NULL to load/save manually. Read io.WantSaveIniSettings description about handling .ini saving manually.
		// - Important: default value "kggui.ini" is relative to current working dir! Most apps will want to lock this to an absolute path (e.g. same path as executables).
		static void          LoadIniSettingsFromDisk(const char* ini_filename);                  // call after CreateContext() and before the first call to NewFrame(). NewFrame() automatically calls LoadIniSettingsFromDisk(io.IniFilename).
		static void          LoadIniSettingsFromMemory(const char* ini_data, size_t ini_size=0); // call after CreateContext() and before the first call to NewFrame() to provide .ini data from your own data source.
		static void          SaveIniSettingsToDisk(const char* ini_filename);                    // this is automatically called (if io.IniFilename is not empty) a few seconds after any modification that should be reflected in the .ini file (and also by DestroyContext).
		static const char*   SaveIniSettingsToMemory(size_t* out_ini_size = NULL);               // return a zero-terminated string with the .ini data which you can save by your own mean. call when io.WantSaveIniSettings is set, then save data by your own mean and clear io.WantSaveIniSettings.

		// Debug Utilities
		static void          DebugTextEncoding(const char* text);
		static bool          DebugCheckVersionAndDataLayout(const char* version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_drawvert, size_t sz_drawidx); // This is called by IMGUI_CHECKVERSION() macro.

		// Memory Allocators
		// - Those functions are not reliant on the current context.
		// - DLL users: heaps and globals are not shared across DLL boundaries! You will need to call SetCurrentContext() + SetAllocatorFunctions()
		//   for each static/DLL boundary you are calling from. Read "Context and Memory Allocators" section of imgui.cpp for more details.
		static void          SetAllocatorFunctions(KarmaGuiMemAllocFunc alloc_func, KarmaGuiMemFreeFunc free_func, void* user_data = NULL);
		static void          GetAllocatorFunctions(KarmaGuiMemAllocFunc* p_alloc_func, KarmaGuiMemFreeFunc* p_free_func, void** p_user_data);
		static void*         MemAlloc(size_t size);
		static void          MemFree(void* ptr);

		// (Optional) Platform/OS interface for multi-viewport support
		// Read comments around the KarmaGuiPlatformIO structure for more details.
		// Note: You may use GetWindowViewport() to get the current viewport of the current window.
		static KarmaGuiPlatformIO&  GetPlatformIO();                                                // platform/renderer functions, for backend to setup + viewports list.
		static void              UpdatePlatformWindows();                                        // call in main loop. will call CreateWindow/ResizeWindow/etc. platform functions for each secondary viewport, and DestroyWindow for each inactive viewport.
		static void              RenderPlatformWindowsDefault(void* platform_render_arg = NULL, void* renderer_render_arg = NULL); // call in main loop. will call RenderWindow/SwapBuffers platform functions for each secondary viewport which doesn't have the KGGuiViewportFlags_Minimized flag set. May be reimplemented by user for custom rendering needs.
		static void              DestroyPlatformWindows();                                       // call DestroyWindow platform functions for all viewports. call from backend Shutdown() if you need to close platform windows before imgui shutdown. otherwise will be called by DestroyContext().
		static KarmaGuiViewport*    FindViewportByID(KGGuiID id);                                   // this is a helper for backends.
		static KarmaGuiViewport*    FindViewportByPlatformHandle(void* platform_handle);            // this is a helper for backends. the type platform_handle is decided by the backend (e.g. HWND, MyWindow*, GLFWwindow* etc.)
		static KarmaGuiKey     GetKeyIndex(KarmaGuiKey key);  // map KGGuiKey_* values into legacy native key index. == io.KeyMap[key]
	private:
		// Those names a provided for debugging purpose and are not meant to be saved persistently not compared.
		static const char* const GKeyNames[];
	};

}// Namespace Karma

//-----------------------------------------------------------------------------
// [SECTION] Flags & Enumerations
//-----------------------------------------------------------------------------

// Flags for ImGui::Begin()
// (Those are per-window flags. There are shared flags in KarmaGuiIO: io.ConfigWindowsResizeFromEdges and io.ConfigWindowsMoveFromTitleBarOnly)
enum KGGuiWindowFlags_
{
    KGGuiWindowFlags_None                   = 0,
    KGGuiWindowFlags_NoTitleBar             = 1 << 0,   // Disable title-bar
    KGGuiWindowFlags_NoResize               = 1 << 1,   // Disable user resizing with the lower-right grip
    KGGuiWindowFlags_NoMove                 = 1 << 2,   // Disable user moving the window
    KGGuiWindowFlags_NoScrollbar            = 1 << 3,   // Disable scrollbars (window can still scroll with mouse or programmatically)
    KGGuiWindowFlags_NoScrollWithMouse      = 1 << 4,   // Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
    KGGuiWindowFlags_NoCollapse             = 1 << 5,   // Disable user collapsing window by double-clicking on it. Also referred to as Window Menu Button (e.g. within a docking node).
    KGGuiWindowFlags_AlwaysAutoResize       = 1 << 6,   // Resize every window to its content every frame
    KGGuiWindowFlags_NoBackground           = 1 << 7,   // Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
    KGGuiWindowFlags_NoSavedSettings        = 1 << 8,   // Never load/save settings in .ini file
    KGGuiWindowFlags_NoMouseInputs          = 1 << 9,   // Disable catching mouse, hovering test with pass through.
    KGGuiWindowFlags_MenuBar                = 1 << 10,  // Has a menu-bar
    KGGuiWindowFlags_HorizontalScrollbar    = 1 << 11,  // Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(KGVec2(width,0.0f)); prior to calling Begin() to specify width. Read code in imgui_demo in the "Horizontal Scrolling" section.
    KGGuiWindowFlags_NoFocusOnAppearing     = 1 << 12,  // Disable taking focus when transitioning from hidden to visible state
    KGGuiWindowFlags_NoBringToFrontOnFocus  = 1 << 13,  // Disable bringing window to front when taking focus (e.g. clicking on it or programmatically giving it focus)
    KGGuiWindowFlags_AlwaysVerticalScrollbar= 1 << 14,  // Always show vertical scrollbar (even if ContentSize.y < Size.y)
    KGGuiWindowFlags_AlwaysHorizontalScrollbar=1<< 15,  // Always show horizontal scrollbar (even if ContentSize.x < Size.x)
    KGGuiWindowFlags_AlwaysUseWindowPadding = 1 << 16,  // Ensure child windows without border uses style.WindowPadding (ignored by default for non-bordered child windows, because more convenient)
    KGGuiWindowFlags_NoNavInputs            = 1 << 18,  // No gamepad/keyboard navigation within the window
    KGGuiWindowFlags_NoNavFocus             = 1 << 19,  // No focusing toward this window with gamepad/keyboard navigation (e.g. skipped by CTRL+TAB)
    KGGuiWindowFlags_UnsavedDocument        = 1 << 20,  // Display a dot next to the title. When used in a tab/docking context, tab is selected when clicking the X + closure is not assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
    KGGuiWindowFlags_NoDocking              = 1 << 21,  // Disable docking of this window

    KGGuiWindowFlags_NoNav                  = KGGuiWindowFlags_NoNavInputs | KGGuiWindowFlags_NoNavFocus,
    KGGuiWindowFlags_NoDecoration           = KGGuiWindowFlags_NoTitleBar | KGGuiWindowFlags_NoResize | KGGuiWindowFlags_NoScrollbar | KGGuiWindowFlags_NoCollapse,
    KGGuiWindowFlags_NoInputs               = KGGuiWindowFlags_NoMouseInputs | KGGuiWindowFlags_NoNavInputs | KGGuiWindowFlags_NoNavFocus,

    // [Internal]
    KGGuiWindowFlags_NavFlattened           = 1 << 23,  // [BETA] On child window: allow gamepad/keyboard navigation to cross over parent border to this child or between sibling child windows.
    KGGuiWindowFlags_ChildWindow            = 1 << 24,  // Don't use! For internal use by BeginChild()
    KGGuiWindowFlags_Tooltip                = 1 << 25,  // Don't use! For internal use by BeginTooltip()
    KGGuiWindowFlags_Popup                  = 1 << 26,  // Don't use! For internal use by BeginPopup()
    KGGuiWindowFlags_Modal                  = 1 << 27,  // Don't use! For internal use by BeginPopupModal()
    KGGuiWindowFlags_ChildMenu              = 1 << 28,  // Don't use! For internal use by BeginMenu()
    KGGuiWindowFlags_DockNodeHost           = 1 << 29,  // Don't use! For internal use by Begin()/NewFrame()
};

// Flags for ImGui::InputText()
// (Those are per-item flags. There are shared flags in KarmaGuiIO: io.ConfigInputTextCursorBlink and io.ConfigInputTextEnterKeepActive)
enum KGGuiInputTextFlags_
{
    KGGuiInputTextFlags_None                = 0,
    KGGuiInputTextFlags_CharsDecimal        = 1 << 0,   // Allow 0123456789.+-*/
    KGGuiInputTextFlags_CharsHexadecimal    = 1 << 1,   // Allow 0123456789ABCDEFabcdef
    KGGuiInputTextFlags_CharsUppercase      = 1 << 2,   // Turn a..z into A..Z
    KGGuiInputTextFlags_CharsNoBlank        = 1 << 3,   // Filter out spaces, tabs
    KGGuiInputTextFlags_AutoSelectAll       = 1 << 4,   // Select entire text when first taking mouse focus
    KGGuiInputTextFlags_EnterReturnsTrue    = 1 << 5,   // Return 'true' when Enter is pressed (as opposed to every time the value was modified). Consider looking at the IsItemDeactivatedAfterEdit() function.
    KGGuiInputTextFlags_CallbackCompletion  = 1 << 6,   // Callback on pressing TAB (for completion handling)
    KGGuiInputTextFlags_CallbackHistory     = 1 << 7,   // Callback on pressing Up/Down arrows (for history handling)
    KGGuiInputTextFlags_CallbackAlways      = 1 << 8,   // Callback on each iteration. User code may query cursor position, modify text buffer.
    KGGuiInputTextFlags_CallbackCharFilter  = 1 << 9,   // Callback on character inputs to replace or discard them. Modify 'EventChar' to replace or discard, or return 1 in callback to discard.
    KGGuiInputTextFlags_AllowTabInput       = 1 << 10,  // Pressing TAB input a '\t' character into the text field
    KGGuiInputTextFlags_CtrlEnterForNewLine = 1 << 11,  // In multi-line mode, unfocus with Enter, add new line with Ctrl+Enter (default is opposite: unfocus with Ctrl+Enter, add line with Enter).
    KGGuiInputTextFlags_NoHorizontalScroll  = 1 << 12,  // Disable following the cursor horizontally
    KGGuiInputTextFlags_AlwaysOverwrite     = 1 << 13,  // Overwrite mode
    KGGuiInputTextFlags_ReadOnly            = 1 << 14,  // Read-only mode
    KGGuiInputTextFlags_Password            = 1 << 15,  // Password mode, display all characters as '*'
    KGGuiInputTextFlags_NoUndoRedo          = 1 << 16,  // Disable undo/redo. Note that input text owns the text data while active, if you want to provide your own undo/redo stack you need e.g. to call ClearActiveID().
    KGGuiInputTextFlags_CharsScientific     = 1 << 17,  // Allow 0123456789.+-*/eE (Scientific notation input)
    KGGuiInputTextFlags_CallbackResize      = 1 << 18,  // Callback on buffer capacity changes request (beyond 'buf_size' parameter value), allowing the string to grow. Notify when the string wants to be resized (for string types which hold a cache of their Size). You will be provided a new BufSize in the callback and NEED to honor it. (see misc/cpp/imgui_stdlib.h for an example of using this)
    KGGuiInputTextFlags_CallbackEdit        = 1 << 19,  // Callback on any edit (note that InputText() already returns true on edit, the callback is useful mainly to manipulate the underlying buffer while focus is active)
    KGGuiInputTextFlags_EscapeClearsAll     = 1 << 20,  // Escape key clears content if not empty, and deactivate otherwise (contrast to default behavior of Escape to revert)

    // Obsolete names (will be removed soon)
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    KGGuiInputTextFlags_AlwaysInsertMode    = KGGuiInputTextFlags_AlwaysOverwrite   // [renamed in 1.82] name was not matching behavior
#endif
};

// Flags for ImGui::TreeNodeEx(), ImGui::CollapsingHeader*()
enum KGGuiTreeNodeFlags_
{
    KGGuiTreeNodeFlags_None                 = 0,
    KGGuiTreeNodeFlags_Selected             = 1 << 0,   // Draw as selected
    KGGuiTreeNodeFlags_Framed               = 1 << 1,   // Draw frame with background (e.g. for CollapsingHeader)
    KGGuiTreeNodeFlags_AllowItemOverlap     = 1 << 2,   // Hit testing to allow subsequent widgets to overlap this one
    KGGuiTreeNodeFlags_NoTreePushOnOpen     = 1 << 3,   // Don't do a TreePush() when open (e.g. for CollapsingHeader) = no extra indent nor pushing on ID stack
    KGGuiTreeNodeFlags_NoAutoOpenOnLog      = 1 << 4,   // Don't automatically and temporarily open node when Logging is active (by default logging will automatically open tree nodes)
    KGGuiTreeNodeFlags_DefaultOpen          = 1 << 5,   // Default node to be open
    KGGuiTreeNodeFlags_OpenOnDoubleClick    = 1 << 6,   // Need double-click to open node
    KGGuiTreeNodeFlags_OpenOnArrow          = 1 << 7,   // Only open when clicking on the arrow part. If KGGuiTreeNodeFlags_OpenOnDoubleClick is also set, single-click arrow or double-click all box to open.
    KGGuiTreeNodeFlags_Leaf                 = 1 << 8,   // No collapsing, no arrow (use as a convenience for leaf nodes).
    KGGuiTreeNodeFlags_Bullet               = 1 << 9,   // Display a bullet instead of arrow
    KGGuiTreeNodeFlags_FramePadding         = 1 << 10,  // Use FramePadding (even for an unframed text node) to vertically align text baseline to regular widget height. Equivalent to calling AlignTextToFramePadding().
    KGGuiTreeNodeFlags_SpanAvailWidth       = 1 << 11,  // Extend hit box to the right-most edge, even if not framed. This is not the default in order to allow adding other items on the same line. In the future we may refactor the hit system to be front-to-back, allowing natural overlaps and then this can become the default.
    KGGuiTreeNodeFlags_SpanFullWidth        = 1 << 12,  // Extend hit box to the left-most and right-most edges (bypass the indented area).
    KGGuiTreeNodeFlags_NavLeftJumpsBackHere = 1 << 13,  // (WIP) Nav: left direction may move to this TreeNode() from any of its child (items submitted between TreeNode and TreePop)
    //KGGuiTreeNodeFlags_NoScrollOnOpen     = 1 << 14,  // FIXME: TODO: Disable automatic scroll on TreePop() if node got just open and contents is not visible
    KGGuiTreeNodeFlags_CollapsingHeader     = KGGuiTreeNodeFlags_Framed | KGGuiTreeNodeFlags_NoTreePushOnOpen | KGGuiTreeNodeFlags_NoAutoOpenOnLog,
};

// Flags for OpenPopup*(), BeginPopupContext*(), IsPopupOpen() functions.
// - To be backward compatible with older API which took an 'int mouse_button = 1' argument, we need to treat
//   small flags values as a mouse button index, so we encode the mouse button in the first few bits of the flags.
//   It is therefore guaranteed to be legal to pass a mouse button index in KarmaGuiPopupFlags.
// - For the same reason, we exceptionally default the KarmaGuiPopupFlags argument of BeginPopupContextXXX functions to 1 instead of 0.
//   IMPORTANT: because the default parameter is 1 (==KGGuiPopupFlags_MouseButtonRight), if you rely on the default parameter
//   and want to use another flag, you need to pass in the KGGuiPopupFlags_MouseButtonRight flag explicitly.
// - Multiple buttons currently cannot be combined/or-ed in those functions (we could allow it later).
enum KGGuiPopupFlags_
{
    KGGuiPopupFlags_None                    = 0,
    KGGuiPopupFlags_MouseButtonLeft         = 0,        // For BeginPopupContext*(): open on Left Mouse release. Guaranteed to always be == 0 (same as KGGuiMouseButton_Left)
    KGGuiPopupFlags_MouseButtonRight        = 1,        // For BeginPopupContext*(): open on Right Mouse release. Guaranteed to always be == 1 (same as KGGuiMouseButton_Right)
    KGGuiPopupFlags_MouseButtonMiddle       = 2,        // For BeginPopupContext*(): open on Middle Mouse release. Guaranteed to always be == 2 (same as KGGuiMouseButton_Middle)
    KGGuiPopupFlags_MouseButtonMask_        = 0x1F,
    KGGuiPopupFlags_MouseButtonDefault_     = 1,
    KGGuiPopupFlags_NoOpenOverExistingPopup = 1 << 5,   // For OpenPopup*(), BeginPopupContext*(): don't open if there's already a popup at the same level of the popup stack
    KGGuiPopupFlags_NoOpenOverItems         = 1 << 6,   // For BeginPopupContextWindow(): don't return true when hovering items, only when hovering empty space
    KGGuiPopupFlags_AnyPopupId              = 1 << 7,   // For IsPopupOpen(): ignore the KGGuiID parameter and test for any popup.
    KGGuiPopupFlags_AnyPopupLevel           = 1 << 8,   // For IsPopupOpen(): search/test at any level of the popup stack (default test in the current level)
    KGGuiPopupFlags_AnyPopup                = KGGuiPopupFlags_AnyPopupId | KGGuiPopupFlags_AnyPopupLevel,
};

// Flags for KarmaGui::Selectable()
enum KGGuiSelectableFlags_
{
    KGGuiSelectableFlags_None               = 0,
    KGGuiSelectableFlags_DontClosePopups    = 1 << 0,   // Clicking this doesn't close parent popup window
    KGGuiSelectableFlags_SpanAllColumns     = 1 << 1,   // Selectable frame can span all columns (text will still fit in current column)
    KGGuiSelectableFlags_AllowDoubleClick   = 1 << 2,   // Generate press events on double clicks too
    KGGuiSelectableFlags_Disabled           = 1 << 3,   // Cannot be selected, display grayed out text
    KGGuiSelectableFlags_AllowItemOverlap   = 1 << 4,   // (WIP) Hit testing to allow subsequent widgets to overlap this one
};

// Flags for KarmaGui::BeginCombo()
enum KGGuiComboFlags_
{
    KGGuiComboFlags_None                    = 0,
    KGGuiComboFlags_PopupAlignLeft          = 1 << 0,   // Align the popup toward the left by default
    KGGuiComboFlags_HeightSmall             = 1 << 1,   // Max ~4 items visible. Tip: If you want your combo popup to be a specific size you can use SetNextWindowSizeConstraints() prior to calling BeginCombo()
    KGGuiComboFlags_HeightRegular           = 1 << 2,   // Max ~8 items visible (default)
    KGGuiComboFlags_HeightLarge             = 1 << 3,   // Max ~20 items visible
    KGGuiComboFlags_HeightLargest           = 1 << 4,   // As many fitting items as possible
    KGGuiComboFlags_NoArrowButton           = 1 << 5,   // Display on the preview box without the square arrow button
    KGGuiComboFlags_NoPreview               = 1 << 6,   // Display only a square arrow button
    KGGuiComboFlags_HeightMask_             = KGGuiComboFlags_HeightSmall | KGGuiComboFlags_HeightRegular | KGGuiComboFlags_HeightLarge | KGGuiComboFlags_HeightLargest,
};

// Flags for KarmaGui::BeginTabBar()
enum KGGuiTabBarFlags_
{
    KGGuiTabBarFlags_None                           = 0,
    KGGuiTabBarFlags_Reorderable                    = 1 << 0,   // Allow manually dragging tabs to re-order them + New tabs are appended at the end of list
    KGGuiTabBarFlags_AutoSelectNewTabs              = 1 << 1,   // Automatically select new tabs when they appear
    KGGuiTabBarFlags_TabListPopupButton             = 1 << 2,   // Disable buttons to open the tab list popup
    KGGuiTabBarFlags_NoCloseWithMiddleMouseButton   = 1 << 3,   // Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You can still repro this behavior on user's side with if (IsItemHovered() && IsMouseClicked(2)) *p_open = false.
    KGGuiTabBarFlags_NoTabListScrollingButtons      = 1 << 4,   // Disable scrolling buttons (apply when fitting policy is KGGuiTabBarFlags_FittingPolicyScroll)
    KGGuiTabBarFlags_NoTooltip                      = 1 << 5,   // Disable tooltips when hovering a tab
    KGGuiTabBarFlags_FittingPolicyResizeDown        = 1 << 6,   // Resize tabs when they don't fit
    KGGuiTabBarFlags_FittingPolicyScroll            = 1 << 7,   // Add scroll buttons when tabs don't fit
    KGGuiTabBarFlags_FittingPolicyMask_             = KGGuiTabBarFlags_FittingPolicyResizeDown | KGGuiTabBarFlags_FittingPolicyScroll,
    KGGuiTabBarFlags_FittingPolicyDefault_          = KGGuiTabBarFlags_FittingPolicyResizeDown,
};

// Flags for KarmaGui::BeginTabItem()
enum KGGuiTabItemFlags_
{
    KGGuiTabItemFlags_None                          = 0,
    KGGuiTabItemFlags_UnsavedDocument               = 1 << 0,   // Display a dot next to the title + tab is selected when clicking the X + closure is not assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
    KGGuiTabItemFlags_SetSelected                   = 1 << 1,   // Trigger flag to programmatically make the tab selected when calling BeginTabItem()
    KGGuiTabItemFlags_NoCloseWithMiddleMouseButton  = 1 << 2,   // Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You can still repro this behavior on user's side with if (IsItemHovered() && IsMouseClicked(2)) *p_open = false.
    KGGuiTabItemFlags_NoPushId                      = 1 << 3,   // Don't call PushID(tab->ID)/PopID() on BeginTabItem()/EndTabItem()
    KGGuiTabItemFlags_NoTooltip                     = 1 << 4,   // Disable tooltip for the given tab
    KGGuiTabItemFlags_NoReorder                     = 1 << 5,   // Disable reordering this tab or having another tab cross over this tab
    KGGuiTabItemFlags_Leading                       = 1 << 6,   // Enforce the tab position to the left of the tab bar (after the tab list popup button)
    KGGuiTabItemFlags_Trailing                      = 1 << 7,   // Enforce the tab position to the right of the tab bar (before the scrolling buttons)
};

// Flags for ImGui::BeginTable()
// - Important! Sizing policies have complex and subtle side effects, much more so than you would expect.
//   Read comments/demos carefully + experiment with live demos to get acquainted with them.
// - The DEFAULT sizing policies are:
//    - Default to KGGuiTableFlags_SizingFixedFit    if ScrollX is on, or if host window has KGGuiWindowFlags_AlwaysAutoResize.
//    - Default to KGGuiTableFlags_SizingStretchSame if ScrollX is off.
// - When ScrollX is off:
//    - Table defaults to KGGuiTableFlags_SizingStretchSame -> all Columns defaults to KGGuiTableColumnFlags_WidthStretch with same weight.
//    - Columns sizing policy allowed: Stretch (default), Fixed/Auto.
//    - Fixed Columns (if any) will generally obtain their requested width (unless the table cannot fit them all).
//    - Stretch Columns will share the remaining width according to their respective weight.
//    - Mixed Fixed/Stretch columns is possible but has various side-effects on resizing behaviors.
//      The typical use of mixing sizing policies is: any number of LEADING Fixed columns, followed by one or two TRAILING Stretch columns.
//      (this is because the visible order of columns have subtle but necessary effects on how they react to manual resizing).
// - When ScrollX is on:
//    - Table defaults to KGGuiTableFlags_SizingFixedFit -> all Columns defaults to KGGuiTableColumnFlags_WidthFixed
//    - Columns sizing policy allowed: Fixed/Auto mostly.
//    - Fixed Columns can be enlarged as needed. Table will show a horizontal scrollbar if needed.
//    - When using auto-resizing (non-resizable) fixed columns, querying the content width to use item right-alignment e.g. SetNextItemWidth(-FLT_MIN) doesn't make sense, would create a feedback loop.
//    - Using Stretch columns OFTEN DOES NOT MAKE SENSE if ScrollX is on, UNLESS you have specified a value for 'inner_width' in BeginTable().
//      If you specify a value for 'inner_width' then effectively the scrolling space is known and Stretch or mixed Fixed/Stretch columns become meaningful again.
// - Read on documentation at the top of imgui_tables.cpp for details.
enum KGGuiTableFlags_
{
    // Features
    KGGuiTableFlags_None                       = 0,
    KGGuiTableFlags_Resizable                  = 1 << 0,   // Enable resizing columns.
    KGGuiTableFlags_Reorderable                = 1 << 1,   // Enable reordering columns in header row (need calling TableSetupColumn() + TableHeadersRow() to display headers)
    KGGuiTableFlags_Hideable                   = 1 << 2,   // Enable hiding/disabling columns in context menu.
    KGGuiTableFlags_Sortable                   = 1 << 3,   // Enable sorting. Call TableGetSortSpecs() to obtain sort specs. Also see KGGuiTableFlags_SortMulti and KGGuiTableFlags_SortTristate.
    KGGuiTableFlags_NoSavedSettings            = 1 << 4,   // Disable persisting columns order, width and sort settings in the .ini file.
    KGGuiTableFlags_ContextMenuInBody          = 1 << 5,   // Right-click on columns body/contents will display table context menu. By default it is available in TableHeadersRow().
    // Decorations
    KGGuiTableFlags_RowBg                      = 1 << 6,   // Set each RowBg color with KGGuiCol_TableRowBg or KGGuiCol_TableRowBgAlt (equivalent of calling TableSetBgColor with ImGuiTableBgFlags_RowBg0 on each row manually)
    KGGuiTableFlags_BordersInnerH              = 1 << 7,   // Draw horizontal borders between rows.
    KGGuiTableFlags_BordersOuterH              = 1 << 8,   // Draw horizontal borders at the top and bottom.
    KGGuiTableFlags_BordersInnerV              = 1 << 9,   // Draw vertical borders between columns.
    KGGuiTableFlags_BordersOuterV              = 1 << 10,  // Draw vertical borders on the left and right sides.
    KGGuiTableFlags_BordersH                   = KGGuiTableFlags_BordersInnerH | KGGuiTableFlags_BordersOuterH, // Draw horizontal borders.
    KGGuiTableFlags_BordersV                   = KGGuiTableFlags_BordersInnerV | KGGuiTableFlags_BordersOuterV, // Draw vertical borders.
    KGGuiTableFlags_BordersInner               = KGGuiTableFlags_BordersInnerV | KGGuiTableFlags_BordersInnerH, // Draw inner borders.
    KGGuiTableFlags_BordersOuter               = KGGuiTableFlags_BordersOuterV | KGGuiTableFlags_BordersOuterH, // Draw outer borders.
    KGGuiTableFlags_Borders                    = KGGuiTableFlags_BordersInner | KGGuiTableFlags_BordersOuter,   // Draw all borders.
    KGGuiTableFlags_NoBordersInBody            = 1 << 11,  // [ALPHA] Disable vertical borders in columns Body (borders will always appear in Headers). -> May move to style
    KGGuiTableFlags_NoBordersInBodyUntilResize = 1 << 12,  // [ALPHA] Disable vertical borders in columns Body until hovered for resize (borders will always appear in Headers). -> May move to style
    // Sizing Policy (read above for defaults)
    KGGuiTableFlags_SizingFixedFit             = 1 << 13,  // Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching contents width.
    KGGuiTableFlags_SizingFixedSame            = 2 << 13,  // Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching the maximum contents width of all columns. Implicitly enable KGGuiTableFlags_NoKeepColumnsVisible.
    KGGuiTableFlags_SizingStretchProp          = 3 << 13,  // Columns default to _WidthStretch with default weights proportional to each columns contents widths.
    KGGuiTableFlags_SizingStretchSame          = 4 << 13,  // Columns default to _WidthStretch with default weights all equal, unless overridden by TableSetupColumn().
    // Sizing Extra Options
    KGGuiTableFlags_NoHostExtendX              = 1 << 16,  // Make outer width auto-fit to columns, overriding outer_size.x value. Only available when ScrollX/ScrollY are disabled and Stretch columns are not used.
    KGGuiTableFlags_NoHostExtendY              = 1 << 17,  // Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit). Only available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.
    KGGuiTableFlags_NoKeepColumnsVisible       = 1 << 18,  // Disable keeping column always minimally visible when ScrollX is off and table gets too small. Not recommended if columns are resizable.
    KGGuiTableFlags_PreciseWidths              = 1 << 19,  // Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.
    // Clipping
    KGGuiTableFlags_NoClip                     = 1 << 20,  // Disable clipping rectangle for every individual columns (reduce draw command count, items will be able to overflow into other columns). Generally incompatible with TableSetupScrollFreeze().
    // Padding
    KGGuiTableFlags_PadOuterX                  = 1 << 21,  // Default if BordersOuterV is on. Enable outermost padding. Generally desirable if you have headers.
    KGGuiTableFlags_NoPadOuterX                = 1 << 22,  // Default if BordersOuterV is off. Disable outermost padding.
    KGGuiTableFlags_NoPadInnerX                = 1 << 23,  // Disable inner padding between columns (double inner padding if BordersOuterV is on, single inner padding if BordersOuterV is off).
    // Scrolling
    KGGuiTableFlags_ScrollX                    = 1 << 24,  // Enable horizontal scrolling. Require 'outer_size' parameter of BeginTable() to specify the container size. Changes default sizing policy. Because this creates a child window, ScrollY is currently generally recommended when using ScrollX.
    KGGuiTableFlags_ScrollY                    = 1 << 25,  // Enable vertical scrolling. Require 'outer_size' parameter of BeginTable() to specify the container size.
    // Sorting
    KGGuiTableFlags_SortMulti                  = 1 << 26,  // Hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).
    KGGuiTableFlags_SortTristate               = 1 << 27,  // Allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).

    // [Internal] Combinations and masks
    KGGuiTableFlags_SizingMask_                = KGGuiTableFlags_SizingFixedFit | KGGuiTableFlags_SizingFixedSame | KGGuiTableFlags_SizingStretchProp | KGGuiTableFlags_SizingStretchSame,
};

// Flags for KarmaGui::TableSetupColumn()
enum KGGuiTableColumnFlags_
{
    // Input configuration flags
    KGGuiTableColumnFlags_None                  = 0,
    KGGuiTableColumnFlags_Disabled              = 1 << 0,   // Overriding/master disable flag: hide column, won't show in context menu (unlike calling TableSetColumnEnabled() which manipulates the user accessible state)
    KGGuiTableColumnFlags_DefaultHide           = 1 << 1,   // Default as a hidden/disabled column.
    KGGuiTableColumnFlags_DefaultSort           = 1 << 2,   // Default as a sorting column.
    KGGuiTableColumnFlags_WidthStretch          = 1 << 3,   // Column will stretch. Preferable with horizontal scrolling disabled (default if table sizing policy is _SizingStretchSame or _SizingStretchProp).
    KGGuiTableColumnFlags_WidthFixed            = 1 << 4,   // Column will not stretch. Preferable with horizontal scrolling enabled (default if table sizing policy is _SizingFixedFit and table is resizable).
    KGGuiTableColumnFlags_NoResize              = 1 << 5,   // Disable manual resizing.
    KGGuiTableColumnFlags_NoReorder             = 1 << 6,   // Disable manual reordering this column, this will also prevent other columns from crossing over this column.
    KGGuiTableColumnFlags_NoHide                = 1 << 7,   // Disable ability to hide/disable this column.
    KGGuiTableColumnFlags_NoClip                = 1 << 8,   // Disable clipping for this column (all NoClip columns will render in a same draw command).
    KGGuiTableColumnFlags_NoSort                = 1 << 9,   // Disable ability to sort on this field (even if KGGuiTableFlags_Sortable is set on the table).
    KGGuiTableColumnFlags_NoSortAscending       = 1 << 10,  // Disable ability to sort in the ascending direction.
    KGGuiTableColumnFlags_NoSortDescending      = 1 << 11,  // Disable ability to sort in the descending direction.
    KGGuiTableColumnFlags_NoHeaderLabel         = 1 << 12,  // TableHeadersRow() will not submit label for this column. Convenient for some small columns. Name will still appear in context menu.
    KGGuiTableColumnFlags_NoHeaderWidth         = 1 << 13,  // Disable header text width contribution to automatic column width.
    KGGuiTableColumnFlags_PreferSortAscending   = 1 << 14,  // Make the initial sort direction Ascending when first sorting on this column (default).
    KGGuiTableColumnFlags_PreferSortDescending  = 1 << 15,  // Make the initial sort direction Descending when first sorting on this column.
    KGGuiTableColumnFlags_IndentEnable          = 1 << 16,  // Use current Indent value when entering cell (default for column 0).
    KGGuiTableColumnFlags_IndentDisable         = 1 << 17,  // Ignore current Indent value when entering cell (default for columns > 0). Indentation changes _within_ the cell will still be honored.

    // Output status flags, read-only via TableGetColumnFlags()
    KGGuiTableColumnFlags_IsEnabled             = 1 << 24,  // Status: is enabled == not hidden by user/api (referred to as "Hide" in _DefaultHide and _NoHide) flags.
    KGGuiTableColumnFlags_IsVisible             = 1 << 25,  // Status: is visible == is enabled AND not clipped by scrolling.
    KGGuiTableColumnFlags_IsSorted              = 1 << 26,  // Status: is currently part of the sort specs
    KGGuiTableColumnFlags_IsHovered             = 1 << 27,  // Status: is hovered by mouse

    // [Internal] Combinations and masks
    KGGuiTableColumnFlags_WidthMask_            = KGGuiTableColumnFlags_WidthStretch | KGGuiTableColumnFlags_WidthFixed,
    KGGuiTableColumnFlags_IndentMask_           = KGGuiTableColumnFlags_IndentEnable | KGGuiTableColumnFlags_IndentDisable,
    KGGuiTableColumnFlags_StatusMask_           = KGGuiTableColumnFlags_IsEnabled | KGGuiTableColumnFlags_IsVisible | KGGuiTableColumnFlags_IsSorted | KGGuiTableColumnFlags_IsHovered,
    KGGuiTableColumnFlags_NoDirectResize_       = 1 << 30,  // [Internal] Disable user resizing this column directly (it may however we resized indirectly from its left edge)
};

// Flags for KarmaGui::TableNextRow()
enum KGGuiTableRowFlags_
{
    KGGuiTableRowFlags_None                     = 0,
    KGGuiTableRowFlags_Headers                  = 1 << 0,   // Identify header row (set default background color + width of its contents accounted differently for auto column width)
};

// Enum for ImGui::TableSetBgColor()
// Background colors are rendering in 3 layers:
//  - Layer 0: draw with RowBg0 color if set, otherwise draw with ColumnBg0 if set.
//  - Layer 1: draw with RowBg1 color if set, otherwise draw with ColumnBg1 if set.
//  - Layer 2: draw with CellBg color if set.
// The purpose of the two row/columns layers is to let you decide if a background color change should override or blend with the existing color.
// When using KGGuiTableFlags_RowBg on the table, each row has the RowBg0 color automatically set for odd/even rows.
// If you set the color of RowBg0 target, your color will override the existing RowBg0 color.
// If you set the color of RowBg1 or ColumnBg1 target, your color will blend over the RowBg0 color.
enum KGGuiTableBgTarget_
{
    KGGuiTableBgTarget_None                     = 0,
    KGGuiTableBgTarget_RowBg0                   = 1,        // Set row background color 0 (generally used for background, automatically set when KGGuiTableFlags_RowBg is used)
    KGGuiTableBgTarget_RowBg1                   = 2,        // Set row background color 1 (generally used for selection marking)
    KGGuiTableBgTarget_CellBg                   = 3,        // Set cell background color (top-most color)
};

// Flags for KarmaGui::IsWindowFocused()
enum KGGuiFocusedFlags_
{
    KGGuiFocusedFlags_None                          = 0,
    KGGuiFocusedFlags_ChildWindows                  = 1 << 0,   // Return true if any children of the window is focused
    KGGuiFocusedFlags_RootWindow                    = 1 << 1,   // Test from root window (top most parent of the current hierarchy)
    KGGuiFocusedFlags_AnyWindow                     = 1 << 2,   // Return true if any window is focused. Important: If you are trying to tell how to dispatch your low-level inputs, do NOT use this. Use 'io.WantCaptureMouse' instead! Please read the FAQ!
    KGGuiFocusedFlags_NoPopupHierarchy              = 1 << 3,   // Do not consider popup hierarchy (do not treat popup emitter as parent of popup) (when used with _ChildWindows or _RootWindow)
    KGGuiFocusedFlags_DockHierarchy                 = 1 << 4,   // Consider docking hierarchy (treat dockspace host as parent of docked window) (when used with _ChildWindows or _RootWindow)
    KGGuiFocusedFlags_RootAndChildWindows           = KGGuiFocusedFlags_RootWindow | KGGuiFocusedFlags_ChildWindows,
};

// Flags for KarmaGui::IsItemHovered(), KarmaGui::IsWindowHovered()
// Note: if you are trying to check whether your mouse should be dispatched to Dear ImGui or to your app, you should use 'io.WantCaptureMouse' instead! Please read the FAQ!
// Note: windows with the KGGuiWindowFlags_NoInputs flag are ignored by IsWindowHovered() calls.
enum KGGuiHoveredFlags_
{
    KGGuiHoveredFlags_None                          = 0,        // Return true if directly over the item/window, not obstructed by another window, not obstructed by an active popup or modal blocking inputs under them.
    KGGuiHoveredFlags_ChildWindows                  = 1 << 0,   // IsWindowHovered() only: Return true if any children of the window is hovered
    KGGuiHoveredFlags_RootWindow                    = 1 << 1,   // IsWindowHovered() only: Test from root window (top most parent of the current hierarchy)
    KGGuiHoveredFlags_AnyWindow                     = 1 << 2,   // IsWindowHovered() only: Return true if any window is hovered
    KGGuiHoveredFlags_NoPopupHierarchy              = 1 << 3,   // IsWindowHovered() only: Do not consider popup hierarchy (do not treat popup emitter as parent of popup) (when used with _ChildWindows or _RootWindow)
    KGGuiHoveredFlags_DockHierarchy                 = 1 << 4,   // IsWindowHovered() only: Consider docking hierarchy (treat dockspace host as parent of docked window) (when used with _ChildWindows or _RootWindow)
    KGGuiHoveredFlags_AllowWhenBlockedByPopup       = 1 << 5,   // Return true even if a popup window is normally blocking access to this item/window
    //KGGuiHoveredFlags_AllowWhenBlockedByModal     = 1 << 6,   // Return true even if a modal popup window is normally blocking access to this item/window. FIXME-TODO: Unavailable yet.
    KGGuiHoveredFlags_AllowWhenBlockedByActiveItem  = 1 << 7,   // Return true even if an active item is blocking access to this item/window. Useful for Drag and Drop patterns.
    KGGuiHoveredFlags_AllowWhenOverlapped           = 1 << 8,   // IsItemHovered() only: Return true even if the position is obstructed or overlapped by another window
    KGGuiHoveredFlags_AllowWhenDisabled             = 1 << 9,   // IsItemHovered() only: Return true even if the item is disabled
    KGGuiHoveredFlags_NoNavOverride                 = 1 << 10,  // Disable using gamepad/keyboard navigation state when active, always query mouse.
    KGGuiHoveredFlags_RectOnly                      = KGGuiHoveredFlags_AllowWhenBlockedByPopup | KGGuiHoveredFlags_AllowWhenBlockedByActiveItem | KGGuiHoveredFlags_AllowWhenOverlapped,
    KGGuiHoveredFlags_RootAndChildWindows           = KGGuiHoveredFlags_RootWindow | KGGuiHoveredFlags_ChildWindows,

    // Hovering delays (for tooltips)
    KGGuiHoveredFlags_DelayNormal                   = 1 << 11,  // Return true after io.HoverDelayNormal elapsed (~0.30 sec)
    KGGuiHoveredFlags_DelayShort                    = 1 << 12,  // Return true after io.HoverDelayShort elapsed (~0.10 sec)
    KGGuiHoveredFlags_NoSharedDelay                 = 1 << 13,  // Disable shared delay system where moving from one item to the next keeps the previous timer for a short time (standard for tooltips with long delays)
};

// Flags for KarmaGui::DockSpace(), shared/inherited by child nodes.
// (Some flags can be applied to individual nodes directly)
// FIXME-DOCK: Also see KGGuiDockNodeFlagsPrivate_ which may involve using the WIP and internal DockBuilder api.
enum KGGuiDockNodeFlags_
{
    KGGuiDockNodeFlags_None                         = 0,
    KGGuiDockNodeFlags_KeepAliveOnly                = 1 << 0,   // Shared       // Don't display the dockspace node but keep it alive. Windows docked into this dockspace node won't be undocked.
    //KGGuiDockNodeFlags_NoCentralNode              = 1 << 1,   // Shared       // Disable Central Node (the node which can stay empty)
    KGGuiDockNodeFlags_NoDockingInCentralNode       = 1 << 2,   // Shared       // Disable docking inside the Central Node, which will be always kept empty.
    KGGuiDockNodeFlags_PassthruCentralNode          = 1 << 3,   // Shared       // Enable passthru dockspace: 1) DockSpace() will render a KGGuiCol_WindowBg background covering everything excepted the Central Node when empty. Meaning the host window should probably use SetNextWindowBgAlpha(0.0f) prior to Begin() when using this. 2) When Central Node is empty: let inputs pass-through + won't display a DockingEmptyBg background. See demo for details.
    KGGuiDockNodeFlags_NoSplit                      = 1 << 4,   // Shared/Local // Disable splitting the node into smaller nodes. Useful e.g. when embedding dockspaces into a main root one (the root one may have splitting disabled to reduce confusion). Note: when turned off, existing splits will be preserved.
    KGGuiDockNodeFlags_NoResize                     = 1 << 5,   // Shared/Local // Disable resizing node using the splitter/separators. Useful with programmatically setup dockspaces.
    KGGuiDockNodeFlags_AutoHideTabBar               = 1 << 6,   // Shared/Local // Tab bar will automatically hide when there is a single window in the dock node.
};

// Flags for KarmaGui::BeginDragDropSource(), ImGui::AcceptDragDropPayload()
enum KGGuiDragDropFlags_
{
    KGGuiDragDropFlags_None                         = 0,
    // BeginDragDropSource() flags
    KGGuiDragDropFlags_SourceNoPreviewTooltip       = 1 << 0,   // By default, a successful call to BeginDragDropSource opens a tooltip so you can display a preview or description of the source contents. This flag disables this behavior.
    KGGuiDragDropFlags_SourceNoDisableHover         = 1 << 1,   // By default, when dragging we clear data so that IsItemHovered() will return false, to avoid subsequent user code submitting tooltips. This flag disables this behavior so you can still call IsItemHovered() on the source item.
    KGGuiDragDropFlags_SourceNoHoldToOpenOthers     = 1 << 2,   // Disable the behavior that allows to open tree nodes and collapsing header by holding over them while dragging a source item.
    KGGuiDragDropFlags_SourceAllowNullID            = 1 << 3,   // Allow items such as Text(), Image() that have no unique identifier to be used as drag source, by manufacturing a temporary identifier based on their window-relative position. This is extremely unusual within the dear imgui ecosystem and so we made it explicit.
    KGGuiDragDropFlags_SourceExtern                 = 1 << 4,   // External source (from outside of dear imgui), won't attempt to read current item/window info. Will always return true. Only one Extern source can be active simultaneously.
    KGGuiDragDropFlags_SourceAutoExpirePayload      = 1 << 5,   // Automatically expire the payload if the source cease to be submitted (otherwise payloads are persisting while being dragged)
    // AcceptDragDropPayload() flags
    KGGuiDragDropFlags_AcceptBeforeDelivery         = 1 << 10,  // AcceptDragDropPayload() will returns true even before the mouse button is released. You can then call IsDelivery() to test if the payload needs to be delivered.
    KGGuiDragDropFlags_AcceptNoDrawDefaultRect      = 1 << 11,  // Do not draw the default highlight rectangle when hovering over target.
    KGGuiDragDropFlags_AcceptNoPreviewTooltip       = 1 << 12,  // Request hiding the BeginDragDropSource tooltip from the BeginDragDropTarget site.
    KGGuiDragDropFlags_AcceptPeekOnly               = KGGuiDragDropFlags_AcceptBeforeDelivery | KGGuiDragDropFlags_AcceptNoDrawDefaultRect, // For peeking ahead and inspecting the payload before delivery.
};

// Standard Drag and Drop payload types. You can define you own payload types using short strings. Types starting with '_' are defined by Dear ImGui.
#define KARMAGUI_PAYLOAD_TYPE_COLOR_3F     "_COL3F"    // float[3]: Standard type for colors, without alpha. User code may use this type.
#define KARMAGUI_PAYLOAD_TYPE_COLOR_4F     "_COL4F"    // float[4]: Standard type for colors. User code may use this type.

// A primary data type
enum KGGuiDataType_
{
    KGGuiDataType_S8,       // signed char / char (with sensible compilers)
    KGGuiDataType_U8,       // unsigned char
    KGGuiDataType_S16,      // short
    KGGuiDataType_U16,      // unsigned short
    KGGuiDataType_S32,      // int
    KGGuiDataType_U32,      // unsigned int
    KGGuiDataType_S64,      // long long / __int64
    KGGuiDataType_U64,      // unsigned long long / unsigned __int64
    KGGuiDataType_Float,    // float
    KGGuiDataType_Double,   // double
    KGGuiDataType_COUNT
};

// A cardinal direction
enum KGGuiDir_
{
    KGGuiDir_None    = -1,
    KGGuiDir_Left    = 0,
    KGGuiDir_Right   = 1,
    KGGuiDir_Up      = 2,
    KGGuiDir_Down    = 3,
    KGGuiDir_COUNT
};

// A sorting direction
enum KGGuiSortDirection_
{
    KGGuiSortDirection_None         = 0,
    KGGuiSortDirection_Ascending    = 1,    // Ascending = 0->9, A->Z etc.
    KGGuiSortDirection_Descending   = 2     // Descending = 9->0, Z->A etc.
};

// A key identifier (KGGuiKey_XXX or KGGuiMod_XXX value): can represent Keyboard, Mouse and Gamepad values.
// All our named keys are >= 512. Keys value 0 to 511 are left unused as legacy native/opaque key values (< 1.87).
// Since >= 1.89 we increased typing (went from int to enum), some legacy code may need a cast to KarmaGuiKey.
// Read details about the 1.87 and 1.89 transition : https://github.com/ocornut/imgui/issues/4921
enum KarmaGuiKey : int
{
    // Keyboard
    KGGuiKey_None = 0,
    KGGuiKey_Tab = 512,             // == KGGuiKey_NamedKey_BEGIN
    KGGuiKey_LeftArrow,
    KGGuiKey_RightArrow,
    KGGuiKey_UpArrow,
    KGGuiKey_DownArrow,
    KGGuiKey_PageUp,
    KGGuiKey_PageDown,
    KGGuiKey_Home,
    KGGuiKey_End,
    KGGuiKey_Insert,
    KGGuiKey_Delete,
    KGGuiKey_Backspace,
    KGGuiKey_Space,
    KGGuiKey_Enter,
    KGGuiKey_Escape,
    KGGuiKey_LeftCtrl, KGGuiKey_LeftShift, KGGuiKey_LeftAlt, KGGuiKey_LeftSuper,
    KGGuiKey_RightCtrl, KGGuiKey_RightShift, KGGuiKey_RightAlt, KGGuiKey_RightSuper,
    KGGuiKey_Menu,
    KGGuiKey_0, KGGuiKey_1, KGGuiKey_2, KGGuiKey_3, KGGuiKey_4, KGGuiKey_5, KGGuiKey_6, KGGuiKey_7, KGGuiKey_8, KGGuiKey_9,
    KGGuiKey_A, KGGuiKey_B, KGGuiKey_C, KGGuiKey_D, KGGuiKey_E, KGGuiKey_F, KGGuiKey_G, KGGuiKey_H, KGGuiKey_I, KGGuiKey_J,
    KGGuiKey_K, KGGuiKey_L, KGGuiKey_M, KGGuiKey_N, KGGuiKey_O, KGGuiKey_P, KGGuiKey_Q, KGGuiKey_R, KGGuiKey_S, KGGuiKey_T,
    KGGuiKey_U, KGGuiKey_V, KGGuiKey_W, KGGuiKey_X, KGGuiKey_Y, KGGuiKey_Z,
    KGGuiKey_F1, KGGuiKey_F2, KGGuiKey_F3, KGGuiKey_F4, KGGuiKey_F5, KGGuiKey_F6,
    KGGuiKey_F7, KGGuiKey_F8, KGGuiKey_F9, KGGuiKey_F10, KGGuiKey_F11, KGGuiKey_F12,
    KGGuiKey_Apostrophe,        // '
    KGGuiKey_Comma,             // ,
    KGGuiKey_Minus,             // -
    KGGuiKey_Period,            // .
    KGGuiKey_Slash,             // /
    KGGuiKey_Semicolon,         // ;
    KGGuiKey_Equal,             // =
    KGGuiKey_LeftBracket,       // [
    KGGuiKey_Backslash,         // \ (this text inhibit multiline comment caused by backslash)
    KGGuiKey_RightBracket,      // ]
    KGGuiKey_GraveAccent,       // `
    KGGuiKey_CapsLock,
    KGGuiKey_ScrollLock,
    KGGuiKey_NumLock,
    KGGuiKey_PrintScreen,
    KGGuiKey_Pause,
    KGGuiKey_Keypad0, KGGuiKey_Keypad1, KGGuiKey_Keypad2, KGGuiKey_Keypad3, KGGuiKey_Keypad4,
    KGGuiKey_Keypad5, KGGuiKey_Keypad6, KGGuiKey_Keypad7, KGGuiKey_Keypad8, KGGuiKey_Keypad9,
    KGGuiKey_KeypadDecimal,
    KGGuiKey_KeypadDivide,
    KGGuiKey_KeypadMultiply,
    KGGuiKey_KeypadSubtract,
    KGGuiKey_KeypadAdd,
    KGGuiKey_KeypadEnter,
    KGGuiKey_KeypadEqual,

    // Gamepad (some of those are analog values, 0.0f to 1.0f)                          // NAVIGATION ACTION
    // (download controller mapping PNG/PSD at http://dearimgui.org/controls_sheets)
    KGGuiKey_GamepadStart,          // Menu (Xbox)      + (Switch)   Start/Options (PS)
    KGGuiKey_GamepadBack,           // View (Xbox)      - (Switch)   Share (PS)
    KGGuiKey_GamepadFaceLeft,       // X (Xbox)         Y (Switch)   Square (PS)        // Tap: Toggle Menu. Hold: Windowing mode (Focus/Move/Resize windows)
    KGGuiKey_GamepadFaceRight,      // B (Xbox)         A (Switch)   Circle (PS)        // Cancel / Close / Exit
    KGGuiKey_GamepadFaceUp,         // Y (Xbox)         X (Switch)   Triangle (PS)      // Text Input / On-screen Keyboard
    KGGuiKey_GamepadFaceDown,       // A (Xbox)         B (Switch)   Cross (PS)         // Activate / Open / Toggle / Tweak
    KGGuiKey_GamepadDpadLeft,       // D-pad Left                                       // Move / Tweak / Resize Window (in Windowing mode)
    KGGuiKey_GamepadDpadRight,      // D-pad Right                                      // Move / Tweak / Resize Window (in Windowing mode)
    KGGuiKey_GamepadDpadUp,         // D-pad Up                                         // Move / Tweak / Resize Window (in Windowing mode)
    KGGuiKey_GamepadDpadDown,       // D-pad Down                                       // Move / Tweak / Resize Window (in Windowing mode)
    KGGuiKey_GamepadL1,             // L Bumper (Xbox)  L (Switch)   L1 (PS)            // Tweak Slower / Focus Previous (in Windowing mode)
    KGGuiKey_GamepadR1,             // R Bumper (Xbox)  R (Switch)   R1 (PS)            // Tweak Faster / Focus Next (in Windowing mode)
    KGGuiKey_GamepadL2,             // L Trig. (Xbox)   ZL (Switch)  L2 (PS) [Analog]
    KGGuiKey_GamepadR2,             // R Trig. (Xbox)   ZR (Switch)  R2 (PS) [Analog]
    KGGuiKey_GamepadL3,             // L Stick (Xbox)   L3 (Switch)  L3 (PS)
    KGGuiKey_GamepadR3,             // R Stick (Xbox)   R3 (Switch)  R3 (PS)
    KGGuiKey_GamepadLStickLeft,     // [Analog]                                         // Move Window (in Windowing mode)
    KGGuiKey_GamepadLStickRight,    // [Analog]                                         // Move Window (in Windowing mode)
    KGGuiKey_GamepadLStickUp,       // [Analog]                                         // Move Window (in Windowing mode)
    KGGuiKey_GamepadLStickDown,     // [Analog]                                         // Move Window (in Windowing mode)
    KGGuiKey_GamepadRStickLeft,     // [Analog]
    KGGuiKey_GamepadRStickRight,    // [Analog]
    KGGuiKey_GamepadRStickUp,       // [Analog]
    KGGuiKey_GamepadRStickDown,     // [Analog]

    // Aliases: Mouse Buttons (auto-submitted from AddMouseButtonEvent() calls)
    // - This is mirroring the data also written to io.MouseDown[], io.MouseWheel, in a format allowing them to be accessed via standard key API.
    KGGuiKey_MouseLeft, KGGuiKey_MouseRight, KGGuiKey_MouseMiddle, KGGuiKey_MouseX1, KGGuiKey_MouseX2, KGGuiKey_MouseWheelX, KGGuiKey_MouseWheelY,

    // [Internal] Reserved for mod storage
    KGGuiKey_ReservedForModCtrl, KGGuiKey_ReservedForModShift, KGGuiKey_ReservedForModAlt, KGGuiKey_ReservedForModSuper,
    KGGuiKey_COUNT,

    // Keyboard Modifiers (explicitly submitted by backend via AddKeyEvent() calls)
    // - This is mirroring the data also written to io.KeyCtrl, io.KeyShift, io.KeyAlt, io.KeySuper, in a format allowing
    //   them to be accessed via standard key API, allowing calls such as IsKeyPressed(), IsKeyReleased(), querying duration etc.
    // - Code polling every key (e.g. an interface to detect a key press for input mapping) might want to ignore those
    //   and prefer using the real keys (e.g. KGGuiKey_LeftCtrl, KGGuiKey_RightCtrl instead of KGGuiMod_Ctrl).
    // - In theory the value of keyboard modifiers should be roughly equivalent to a logical or of the equivalent left/right keys.
    //   In practice: it's complicated; mods are often provided from different sources. Keyboard layout, IME, sticky keys and
    //   backends tend to interfere and break that equivalence. The safer decision is to relay that ambiguity down to the end-user...
    KGGuiMod_None                   = 0,
    KGGuiMod_Ctrl                   = 1 << 12, // Ctrl
    KGGuiMod_Shift                  = 1 << 13, // Shift
    KGGuiMod_Alt                    = 1 << 14, // Option/Menu
    KGGuiMod_Super                  = 1 << 15, // Cmd/Super/Windows
    KGGuiMod_Shortcut               = 1 << 11, // Alias for Ctrl (non-macOS) _or_ Super (macOS).
    KGGuiMod_Mask_                  = 0xF800,  // 5-bits

    // [Internal] Prior to 1.87 we required user to fill io.KeysDown[512] using their own native index + the io.KeyMap[] array.
    // We are ditching this method but keeping a legacy path for user code doing e.g. IsKeyPressed(MY_NATIVE_KEY_CODE)
    KGGuiKey_NamedKey_BEGIN         = 512,
    KGGuiKey_NamedKey_END           = KGGuiKey_COUNT,
    KGGuiKey_NamedKey_COUNT         = KGGuiKey_NamedKey_END - KGGuiKey_NamedKey_BEGIN,
#ifdef KARMAGUI_DISABLE_OBSOLETE_KEYIO
    KGGuiKey_KeysData_SIZE          = KGGuiKey_NamedKey_COUNT,          // Size of KeysData[]: only hold named keys
    KGGuiKey_KeysData_OFFSET        = KGGuiKey_NamedKey_BEGIN,          // First key stored in io.KeysData[0]. Accesses to io.KeysData[] must use (key - KGGuiKey_KeysData_OFFSET).
#else
    KGGuiKey_KeysData_SIZE          = KGGuiKey_COUNT,                   // Size of KeysData[]: hold legacy 0..512 keycodes + named keys
    KGGuiKey_KeysData_OFFSET        = 0,                                // First key stored in io.KeysData[0]. Accesses to io.KeysData[] must use (key - KGGuiKey_KeysData_OFFSET).
#endif

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    KGGuiKey_ModCtrl = KGGuiMod_Ctrl, KGGuiKey_ModShift = KGGuiMod_Shift, KGGuiKey_ModAlt = KGGuiMod_Alt, KGGuiKey_ModSuper = KGGuiMod_Super, // Renamed in 1.89
    KGGuiKey_KeyPadEnter = KGGuiKey_KeypadEnter,    // Renamed in 1.87
#endif
};

// Flags for Shortcut()
// (+ for upcoming advanced versions of IsKeyPressed()/IsMouseClicked()/SetKeyOwner()/SetItemKeyOwner() that are currently in imgui_internal.h)
enum KGGuiInputFlags_
{
    KGGuiInputFlags_None                = 0,
    KGGuiInputFlags_Repeat              = 1 << 0,   // Return true on successive repeats. Default for legacy IsKeyPressed(). NOT Default for legacy IsMouseClicked(). MUST BE == 1.

    // Routing policies for Shortcut() + low-level SetShortcutRouting()
    // - The general idea is that several callers register interest in a shortcut, and only one owner gets it.
    // - When a policy (other than _RouteAlways) is set, Shortcut() will register itself with SetShortcutRouting(),
    //   allowing the system to decide where to route the input among other route-aware calls.
    // - Shortcut() uses KGGuiInputFlags_RouteFocused by default: meaning that a simple Shortcut() poll
    //   will register a route and only succeed when parent window is in the focus stack and if no-one
    //   with a higher priority is claiming the shortcut.
    // - Using KGGuiInputFlags_RouteAlways is roughly equivalent to doing e.g. IsKeyPressed(key) + testing mods.
    // - Priorities: GlobalHigh > Focused (when owner is active item) > Global > Focused (when focused window) > GlobalLow.

    // Policies (can select only 1 policy among all available)
    KGGuiInputFlags_RouteFocused        = 1 << 8,   // (Default) Register focused route: Accept inputs if window is in focus stack. Deep-most focused window takes inputs. ActiveId takes inputs over deep-most focused window.
    KGGuiInputFlags_RouteGlobalLow      = 1 << 9,   // Register route globally (lowest priority: unless a focused window or active item registered the route) -> recommended Global priority.
    KGGuiInputFlags_RouteGlobal         = 1 << 10,  // Register route globally (medium priority: unless an active item registered the route, e.g. CTRL+A registered by InputText).
    KGGuiInputFlags_RouteGlobalHigh     = 1 << 11,  // Register route globally (highest priority: unlikely you need to use that: will interfere with every active items)
    KGGuiInputFlags_RouteAlways         = 1 << 12,  // Do not register route, poll keys directly.

    // Policies Options
    KGGuiInputFlags_RouteUnlessBgFocused= 1 << 13,  // Global routes will not be applied if underlying background/void is focused (== no Dear ImGui windows are focused). Useful for overlay applications.
};

#ifndef KARMAGUI_DISABLE_OBSOLETE_KEYIO
// OBSOLETED in 1.88 (from July 2022): KGGuiNavInput and io.NavInputs[].
// Official backends between 1.60 and 1.86: will keep working and feed gamepad inputs as long as IMGUI_DISABLE_OBSOLETE_KEYIO is not set.
// Custom backends: feed gamepad inputs via io.AddKeyEvent() and KGGuiKey_GamepadXXX enums.
enum KGGuiNavInput
{
    KGGuiNavInput_Activate, KGGuiNavInput_Cancel, KGGuiNavInput_Input, KGGuiNavInput_Menu, KGGuiNavInput_DpadLeft, KGGuiNavInput_DpadRight, KGGuiNavInput_DpadUp, KGGuiNavInput_DpadDown,
    KGGuiNavInput_LStickLeft, KGGuiNavInput_LStickRight, KGGuiNavInput_LStickUp, KGGuiNavInput_LStickDown, KGGuiNavInput_FocusPrev, KGGuiNavInput_FocusNext, KGGuiNavInput_TweakSlow, KGGuiNavInput_TweakFast,
    KGGuiNavInput_COUNT,
};
#endif

// Configuration flags stored in io.ConfigFlags. Set by user/application.
enum KGGuiConfigFlags_
{
    KGGuiConfigFlags_None                   = 0,
    KGGuiConfigFlags_NavEnableKeyboard      = 1 << 0,   // Master keyboard navigation enable flag.
    KGGuiConfigFlags_NavEnableGamepad       = 1 << 1,   // Master gamepad navigation enable flag. Backend also needs to set KGGuiBackendFlags_HasGamepad.
    KGGuiConfigFlags_NavEnableSetMousePos   = 1 << 2,   // Instruct navigation to move the mouse cursor. May be useful on TV/console systems where moving a virtual mouse is awkward. Will update io.MousePos and set io.WantSetMousePos=true. If enabled you MUST honor io.WantSetMousePos requests in your backend, otherwise ImGui will react as if the mouse is jumping around back and forth.
    KGGuiConfigFlags_NavNoCaptureKeyboard   = 1 << 3,   // Instruct navigation to not set the io.WantCaptureKeyboard flag when io.NavActive is set.
    KGGuiConfigFlags_NoMouse                = 1 << 4,   // Instruct imgui to clear mouse position/buttons in NewFrame(). This allows ignoring the mouse information set by the backend.
    KGGuiConfigFlags_NoMouseCursorChange    = 1 << 5,   // Instruct backend to not alter mouse cursor shape and visibility. Use if the backend cursor changes are interfering with yours and you don't want to use SetMouseCursor() to change mouse cursor. You may want to honor requests from imgui by reading GetMouseCursor() yourself instead.

    // [BETA] Docking
    KGGuiConfigFlags_DockingEnable          = 1 << 6,   // Docking enable flags.

    // [BETA] Viewports
    // When using viewports it is recommended that your default value for KGGuiCol_WindowBg is opaque (Alpha=1.0) so transition to a viewport won't be noticeable.
    KGGuiConfigFlags_ViewportsEnable        = 1 << 10,  // Viewport enable flags (require both KGGuiBackendFlags_PlatformHasViewports + KGGuiBackendFlags_RendererHasViewports set by the respective backends)
    KGGuiConfigFlags_DpiEnableScaleViewports= 1 << 14,  // [BETA: Don't use] FIXME-DPI: Reposition and resize imgui windows when the DpiScale of a viewport changed (mostly useful for the main viewport hosting other window). Note that resizing the main window itself is up to your application.
    KGGuiConfigFlags_DpiEnableScaleFonts    = 1 << 15,  // [BETA: Don't use] FIXME-DPI: Request bitmap-scaled fonts to match DpiScale. This is a very low-quality workaround. The correct way to handle DPI is _currently_ to replace the atlas and/or fonts in the Platform_OnChangedViewport callback, but this is all early work in progress.

    // User storage (to allow your backend/engine to communicate to code that may be shared between multiple projects. Those flags are NOT used by core Dear ImGui)
    KGGuiConfigFlags_IsSRGB                 = 1 << 20,  // Application is SRGB-aware.
    KGGuiConfigFlags_IsTouchScreen          = 1 << 21,  // Application is using a touch screen instead of a mouse.
};

// Backend capabilities flags stored in io.BackendFlags. Set by imgui_impl_xxx or custom backend.
enum KGGuiBackendFlags_
{
    KGGuiBackendFlags_None                  = 0,
    KGGuiBackendFlags_HasGamepad            = 1 << 0,   // Backend Platform supports gamepad and currently has one connected.
    KGGuiBackendFlags_HasMouseCursors       = 1 << 1,   // Backend Platform supports honoring GetMouseCursor() value to change the OS cursor shape.
    KGGuiBackendFlags_HasSetMousePos        = 1 << 2,   // Backend Platform supports io.WantSetMousePos requests to reposition the OS mouse position (only used if KGGuiConfigFlags_NavEnableSetMousePos is set).
    KGGuiBackendFlags_RendererHasVtxOffset  = 1 << 3,   // Backend Renderer supports KGDrawCmd::VtxOffset. This enables output of large meshes (64K+ vertices) while still using 16-bit indices.

    // [BETA] Viewports
    KGGuiBackendFlags_PlatformHasViewports  = 1 << 10,  // Backend Platform supports multiple viewports.
    KGGuiBackendFlags_HasMouseHoveredViewport=1 << 11,  // Backend Platform supports calling io.AddMouseViewportEvent() with the viewport under the mouse. IF POSSIBLE, ignore viewports with the KGGuiViewportFlags_NoInputs flag (Win32 backend, GLFW 3.30+ backend can do this, SDL backend cannot). If this cannot be done, Dear ImGui needs to use a flawed heuristic to find the viewport under.
    KGGuiBackendFlags_RendererHasViewports  = 1 << 12,  // Backend Renderer supports multiple viewports.
};

// Enumeration for PushStyleColor() / PopStyleColor()
enum KGGuiCol_
{
    KGGuiCol_Text,
    KGGuiCol_TextDisabled,
    KGGuiCol_WindowBg,              // Background of normal windows
    KGGuiCol_ChildBg,               // Background of child windows
    KGGuiCol_PopupBg,               // Background of popups, menus, tooltips windows
    KGGuiCol_Border,
    KGGuiCol_BorderShadow,
    KGGuiCol_FrameBg,               // Background of checkbox, radio button, plot, slider, text input
    KGGuiCol_FrameBgHovered,
    KGGuiCol_FrameBgActive,
    KGGuiCol_TitleBg,
    KGGuiCol_TitleBgActive,
    KGGuiCol_TitleBgCollapsed,
    KGGuiCol_MenuBarBg,
    KGGuiCol_ScrollbarBg,
    KGGuiCol_ScrollbarGrab,
    KGGuiCol_ScrollbarGrabHovered,
    KGGuiCol_ScrollbarGrabActive,
    KGGuiCol_CheckMark,
    KGGuiCol_SliderGrab,
    KGGuiCol_SliderGrabActive,
    KGGuiCol_Button,
    KGGuiCol_ButtonHovered,
    KGGuiCol_ButtonActive,
    KGGuiCol_Header,                // Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem
    KGGuiCol_HeaderHovered,
    KGGuiCol_HeaderActive,
    KGGuiCol_Separator,
    KGGuiCol_SeparatorHovered,
    KGGuiCol_SeparatorActive,
    KGGuiCol_ResizeGrip,            // Resize grip in lower-right and lower-left corners of windows.
    KGGuiCol_ResizeGripHovered,
    KGGuiCol_ResizeGripActive,
    KGGuiCol_Tab,                   // TabItem in a TabBar
    KGGuiCol_TabHovered,
    KGGuiCol_TabActive,
    KGGuiCol_TabUnfocused,
    KGGuiCol_TabUnfocusedActive,
    KGGuiCol_DockingPreview,        // Preview overlay color when about to docking something
    KGGuiCol_DockingEmptyBg,        // Background color for empty node (e.g. CentralNode with no window docked into it)
    KGGuiCol_PlotLines,
    KGGuiCol_PlotLinesHovered,
    KGGuiCol_PlotHistogram,
    KGGuiCol_PlotHistogramHovered,
    KGGuiCol_TableHeaderBg,         // Table header background
    KGGuiCol_TableBorderStrong,     // Table outer and header borders (prefer using Alpha=1.0 here)
    KGGuiCol_TableBorderLight,      // Table inner borders (prefer using Alpha=1.0 here)
    KGGuiCol_TableRowBg,            // Table row background (even rows)
    KGGuiCol_TableRowBgAlt,         // Table row background (odd rows)
    KGGuiCol_TextSelectedBg,
    KGGuiCol_DragDropTarget,        // Rectangle highlighting a drop target
    KGGuiCol_NavHighlight,          // Gamepad/keyboard: current highlighted item
    KGGuiCol_NavWindowingHighlight, // Highlight window when using CTRL+TAB
    KGGuiCol_NavWindowingDimBg,     // Darken/colorize entire screen behind the CTRL+TAB window list, when active
    KGGuiCol_ModalWindowDimBg,      // Darken/colorize entire screen behind a modal window, when one is active
    KGGuiCol_COUNT
};

// Enumeration for PushStyleVar() / PopStyleVar() to temporarily modify the KarmaGuiStyle structure.
// - The enum only refers to fields of KarmaGuiStyle which makes sense to be pushed/popped inside UI code.
//   During initialization or between frames, feel free to just poke into KarmaGuiStyle directly.
// - Tip: Use your programming IDE navigation facilities on the names in the _second column_ below to find the actual members and their description.
//   In Visual Studio IDE: CTRL+comma ("Edit.GoToAll") can follow symbols in comments, whereas CTRL+F12 ("Edit.GoToImplementation") cannot.
//   With Visual Assist installed: ALT+G ("VAssistX.GoToImplementation") can also follow symbols in comments.
// - When changing this enum, you need to update the associated internal table GStyleVarInfo[] accordingly. This is where we link enum values to members offset/type.
enum KGGuiStyleVar_
{
    // Enum name --------------------- // Member in KarmaGuiStyle structure (see KarmaGuiStyle for descriptions)
    KGGuiStyleVar_Alpha,               // float     Alpha
    KGGuiStyleVar_DisabledAlpha,       // float     DisabledAlpha
    KGGuiStyleVar_WindowPadding,       // KGVec2    WindowPadding
    KGGuiStyleVar_WindowRounding,      // float     WindowRounding
    KGGuiStyleVar_WindowBorderSize,    // float     WindowBorderSize
    KGGuiStyleVar_WindowMinSize,       // KGVec2    WindowMinSize
    KGGuiStyleVar_WindowTitleAlign,    // KGVec2    WindowTitleAlign
    KGGuiStyleVar_ChildRounding,       // float     ChildRounding
    KGGuiStyleVar_ChildBorderSize,     // float     ChildBorderSize
    KGGuiStyleVar_PopupRounding,       // float     PopupRounding
    KGGuiStyleVar_PopupBorderSize,     // float     PopupBorderSize
    KGGuiStyleVar_FramePadding,        // KGVec2    FramePadding
    KGGuiStyleVar_FrameRounding,       // float     FrameRounding
    KGGuiStyleVar_FrameBorderSize,     // float     FrameBorderSize
    KGGuiStyleVar_ItemSpacing,         // KGVec2    ItemSpacing
    KGGuiStyleVar_ItemInnerSpacing,    // KGVec2    ItemInnerSpacing
    KGGuiStyleVar_IndentSpacing,       // float     IndentSpacing
    KGGuiStyleVar_CellPadding,         // KGVec2    CellPadding
    KGGuiStyleVar_ScrollbarSize,       // float     ScrollbarSize
    KGGuiStyleVar_ScrollbarRounding,   // float     ScrollbarRounding
    KGGuiStyleVar_GrabMinSize,         // float     GrabMinSize
    KGGuiStyleVar_GrabRounding,        // float     GrabRounding
    KGGuiStyleVar_TabRounding,         // float     TabRounding
    KGGuiStyleVar_ButtonTextAlign,     // KGVec2    ButtonTextAlign
    KGGuiStyleVar_SelectableTextAlign, // KGVec2    SelectableTextAlign
    KGGuiStyleVar_COUNT
};

// Flags for InvisibleButton() [extended in imgui_internal.h]
enum KGGuiButtonFlags_
{
    KGGuiButtonFlags_None                   = 0,
    KGGuiButtonFlags_MouseButtonLeft        = 1 << 0,   // React on left mouse button (default)
    KGGuiButtonFlags_MouseButtonRight       = 1 << 1,   // React on right mouse button
    KGGuiButtonFlags_MouseButtonMiddle      = 1 << 2,   // React on center mouse button

    // [Internal]
    KGGuiButtonFlags_MouseButtonMask_       = KGGuiButtonFlags_MouseButtonLeft | KGGuiButtonFlags_MouseButtonRight | KGGuiButtonFlags_MouseButtonMiddle,
    KGGuiButtonFlags_MouseButtonDefault_    = KGGuiButtonFlags_MouseButtonLeft,
};

// Flags for ColorEdit3() / ColorEdit4() / ColorPicker3() / ColorPicker4() / ColorButton()
enum KGGuiColorEditFlags_
{
    KGGuiColorEditFlags_None            = 0,
    KGGuiColorEditFlags_NoAlpha         = 1 << 1,   //              // ColorEdit, ColorPicker, ColorButton: ignore Alpha component (will only read 3 components from the input pointer).
    KGGuiColorEditFlags_NoPicker        = 1 << 2,   //              // ColorEdit: disable picker when clicking on color square.
    KGGuiColorEditFlags_NoOptions       = 1 << 3,   //              // ColorEdit: disable toggling options menu when right-clicking on inputs/small preview.
    KGGuiColorEditFlags_NoSmallPreview  = 1 << 4,   //              // ColorEdit, ColorPicker: disable color square preview next to the inputs. (e.g. to show only the inputs)
    KGGuiColorEditFlags_NoInputs        = 1 << 5,   //              // ColorEdit, ColorPicker: disable inputs sliders/text widgets (e.g. to show only the small preview color square).
    KGGuiColorEditFlags_NoTooltip       = 1 << 6,   //              // ColorEdit, ColorPicker, ColorButton: disable tooltip when hovering the preview.
    KGGuiColorEditFlags_NoLabel         = 1 << 7,   //              // ColorEdit, ColorPicker: disable display of inline text label (the label is still forwarded to the tooltip and picker).
    KGGuiColorEditFlags_NoSidePreview   = 1 << 8,   //              // ColorPicker: disable bigger color preview on right side of the picker, use small color square preview instead.
    KGGuiColorEditFlags_NoDragDrop      = 1 << 9,   //              // ColorEdit: disable drag and drop target. ColorButton: disable drag and drop source.
    KGGuiColorEditFlags_NoBorder        = 1 << 10,  //              // ColorButton: disable border (which is enforced by default)

    // User Options (right-click on widget to change some of them).
    KGGuiColorEditFlags_AlphaBar        = 1 << 16,  //              // ColorEdit, ColorPicker: show vertical alpha bar/gradient in picker.
    KGGuiColorEditFlags_AlphaPreview    = 1 << 17,  //              // ColorEdit, ColorPicker, ColorButton: display preview as a transparent color over a checkerboard, instead of opaque.
    KGGuiColorEditFlags_AlphaPreviewHalf= 1 << 18,  //              // ColorEdit, ColorPicker, ColorButton: display half opaque / half checkerboard, instead of opaque.
    KGGuiColorEditFlags_HDR             = 1 << 19,  //              // (WIP) ColorEdit: Currently only disable 0.0f..1.0f limits in RGBA edition (note: you probably want to use KGGuiColorEditFlags_Float flag as well).
    KGGuiColorEditFlags_DisplayRGB      = 1 << 20,  // [Display]    // ColorEdit: override _display_ type among RGB/HSV/Hex. ColorPicker: select any combination using one or more of RGB/HSV/Hex.
    KGGuiColorEditFlags_DisplayHSV      = 1 << 21,  // [Display]    // "
    KGGuiColorEditFlags_DisplayHex      = 1 << 22,  // [Display]    // "
    KGGuiColorEditFlags_Uint8           = 1 << 23,  // [DataType]   // ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0..255.
    KGGuiColorEditFlags_Float           = 1 << 24,  // [DataType]   // ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0.0f..1.0f floats instead of 0..255 integers. No round-trip of value via integers.
    KGGuiColorEditFlags_PickerHueBar    = 1 << 25,  // [Picker]     // ColorPicker: bar for Hue, rectangle for Sat/Value.
    KGGuiColorEditFlags_PickerHueWheel  = 1 << 26,  // [Picker]     // ColorPicker: wheel for Hue, triangle for Sat/Value.
    KGGuiColorEditFlags_InputRGB        = 1 << 27,  // [Input]      // ColorEdit, ColorPicker: input and output data in RGB format.
    KGGuiColorEditFlags_InputHSV        = 1 << 28,  // [Input]      // ColorEdit, ColorPicker: input and output data in HSV format.

    // Defaults Options. You can set application defaults using SetColorEditOptions(). The intent is that you probably don't want to
    // override them in most of your calls. Let the user choose via the option menu and/or call SetColorEditOptions() once during startup.
    KGGuiColorEditFlags_DefaultOptions_ = KGGuiColorEditFlags_Uint8 | KGGuiColorEditFlags_DisplayRGB | KGGuiColorEditFlags_InputRGB | KGGuiColorEditFlags_PickerHueBar,

    // [Internal] Masks
    KGGuiColorEditFlags_DisplayMask_    = KGGuiColorEditFlags_DisplayRGB | KGGuiColorEditFlags_DisplayHSV | KGGuiColorEditFlags_DisplayHex,
    KGGuiColorEditFlags_DataTypeMask_   = KGGuiColorEditFlags_Uint8 | KGGuiColorEditFlags_Float,
    KGGuiColorEditFlags_PickerMask_     = KGGuiColorEditFlags_PickerHueWheel | KGGuiColorEditFlags_PickerHueBar,
    KGGuiColorEditFlags_InputMask_      = KGGuiColorEditFlags_InputRGB | KGGuiColorEditFlags_InputHSV,

    // Obsolete names (will be removed)
    // KGGuiColorEditFlags_RGB = KGGuiColorEditFlags_DisplayRGB, KGGuiColorEditFlags_HSV = KGGuiColorEditFlags_DisplayHSV, KGGuiColorEditFlags_HEX = KGGuiColorEditFlags_DisplayHex  // [renamed in 1.69]
};

// Flags for DragFloat(), DragInt(), SliderFloat(), SliderInt() etc.
// We use the same sets of flags for DragXXX() and SliderXXX() functions as the features are the same and it makes it easier to swap them.
// (Those are per-item flags. There are shared flags in KarmaGuiIO: io.ConfigDragClickToInputText)
enum KGGuiSliderFlags_
{
    KGGuiSliderFlags_None                   = 0,
    KGGuiSliderFlags_AlwaysClamp            = 1 << 4,       // Clamp value to min/max bounds when input manually with CTRL+Click. By default CTRL+Click allows going out of bounds.
    KGGuiSliderFlags_Logarithmic            = 1 << 5,       // Make the widget logarithmic (linear otherwise). Consider using KGGuiSliderFlags_NoRoundToFormat with this if using a format-string with small amount of digits.
    KGGuiSliderFlags_NoRoundToFormat        = 1 << 6,       // Disable rounding underlying value to match precision of the display format string (e.g. %.3f values are rounded to those 3 digits)
    KGGuiSliderFlags_NoInput                = 1 << 7,       // Disable CTRL+Click or Enter key allowing to input text directly into the widget
    KGGuiSliderFlags_InvalidMask_           = 0x7000000F,   // [Internal] We treat using those bits as being potentially a 'float power' argument from the previous API that has got miscast to this enum, and will trigger an assert if needed.

    // Obsolete names (will be removed)
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    KGGuiSliderFlags_ClampOnInput = KGGuiSliderFlags_AlwaysClamp, // [renamed in 1.79]
#endif
};

// Identify a mouse button.
// Those values are guaranteed to be stable and we frequently use 0/1 directly. Named enums provided for convenience.
enum KGGuiMouseButton_
{
    KGGuiMouseButton_Left = 0,
    KGGuiMouseButton_Right = 1,
    KGGuiMouseButton_Middle = 2,
    KGGuiMouseButton_COUNT = 5
};

// Enumeration for GetMouseCursor()
// User code may request backend to display given cursor by calling SetMouseCursor(), which is why we have some cursors that are marked unused here
enum KGGuiMouseCursor_
{
    KGGuiMouseCursor_None = -1,
    KGGuiMouseCursor_Arrow = 0,
    KGGuiMouseCursor_TextInput,         // When hovering over InputText, etc.
    KGGuiMouseCursor_ResizeAll,         // (Unused by Dear ImGui functions)
    KGGuiMouseCursor_ResizeNS,          // When hovering over a horizontal border
    KGGuiMouseCursor_ResizeEW,          // When hovering over a vertical border or a column
    KGGuiMouseCursor_ResizeNESW,        // When hovering over the bottom-left corner of a window
    KGGuiMouseCursor_ResizeNWSE,        // When hovering over the bottom-right corner of a window
    KGGuiMouseCursor_Hand,              // (Unused by Dear ImGui functions. Use for e.g. hyperlinks)
    KGGuiMouseCursor_NotAllowed,        // When hovering something with disallowed interaction. Usually a crossed circle.
    KGGuiMouseCursor_COUNT
};

// Enumeration for ImGui::SetWindow***(), SetNextWindow***(), SetNextItem***() functions
// Represent a condition.
// Important: Treat as a regular enum! Do NOT combine multiple values using binary operators! All the functions above treat 0 as a shortcut to KGGuiCond_Always.
enum KGGuiCond_
{
    KGGuiCond_None          = 0,        // No condition (always set the variable), same as _Always
    KGGuiCond_Always        = 1 << 0,   // No condition (always set the variable), same as _None
    KGGuiCond_Once          = 1 << 1,   // Set the variable once per runtime session (only the first call will succeed)
    KGGuiCond_FirstUseEver  = 1 << 2,   // Set the variable if the object/window has no persistently saved data (no entry in .ini file)
    KGGuiCond_Appearing     = 1 << 3,   // Set the variable if the object/window is appearing after being hidden/inactive (or the first time)
};

//-----------------------------------------------------------------------------
// [SECTION] Helpers: Memory allocations macros, KGVector<>
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// IM_MALLOC(), KG_FREE(), KG_NEW(), KG_PLACEMENT_NEW(), KG_DELETE()
// We call C++ constructor on own allocated memory via the placement "new(ptr) Type()" syntax.
// Defining a custom placement new() with a custom parameter allows us to bypass including <new> which on some platforms complains when user has disabled exceptions.
//-----------------------------------------------------------------------------

struct ImNewWrapper {};
inline void* operator new(size_t, ImNewWrapper, void* ptr) { return ptr; }
inline void  operator delete(void*, ImNewWrapper, void*)   {} // This is only required so we can use the symmetrical new()
#define KG_ALLOC(_SIZE)                     Karma::KarmaGui::MemAlloc(_SIZE)
#define KG_FREE(_PTR)                       Karma::KarmaGui::MemFree(_PTR)
#define KG_PLACEMENT_NEW(_PTR)              new(ImNewWrapper(), _PTR)
#define KG_NEW(_TYPE)                       new(ImNewWrapper(), Karma::KarmaGui::MemAlloc(sizeof(_TYPE))) _TYPE
template<typename T> void KG_DELETE(T* p)   { if (p) { p->~T(); Karma::KarmaGui::MemFree(p); } }

//-----------------------------------------------------------------------------
// KGVector<>
// Lightweight std::vector<>-like class to avoid dragging dependencies (also, some implementations of STL with debug enabled are absurdly slow, we bypass it so our code runs fast in debug).
//-----------------------------------------------------------------------------
// - You generally do NOT need to care or use this ever. But we need to make it available in imgui.h because some of our public structures are relying on it.
// - We use std-like naming convention here, which is a little unusual for this codebase.
// - Important: clear() frees memory, resize(0) keep the allocated buffer. We use resize(0) a lot to intentionally recycle allocated buffers across frames and amortize our costs.
// - Important: our implementation does NOT call C++ constructors/destructors, we treat everything as raw data! This is intentional but be extra mindful of that,
//   Do NOT use this class as a std::vector replacement in your own code! Many of the structures used by dear imgui can be safely initialized by a zero-memset.
//-----------------------------------------------------------------------------

template<typename T>
struct KGVector
{
    int                 Size;
    int                 Capacity;
    T*                  Data;

    // Provide standard typedefs but we don't use them ourselves.
    typedef T                   value_type;
    typedef value_type*         iterator;
    typedef const value_type*   const_iterator;

    // Constructors, destructor
    inline KGVector()                                       { Size = Capacity = 0; Data = NULL; }
    inline KGVector(const KGVector<T>& src)                 { Size = Capacity = 0; Data = NULL; operator=(src); }
    inline KGVector<T>& operator=(const KGVector<T>& src)   { clear(); resize(src.Size); if (src.Data) memcpy(Data, src.Data, (size_t)Size * sizeof(T)); return *this; }
    inline ~KGVector()                                      { if (Data) KG_FREE(Data); } // Important: does not destruct anything

    inline void         clear()                             { if (Data) { Size = Capacity = 0; KG_FREE(Data); Data = NULL; } }  // Important: does not destruct anything
    inline void         clear_delete()                      { for (int n = 0; n < Size; n++) KG_DELETE(Data[n]); clear(); }     // Important: never called automatically! always explicit.
    inline void         clear_destruct()                    { for (int n = 0; n < Size; n++) Data[n].~T(); clear(); }           // Important: never called automatically! always explicit.

    inline bool         empty() const                       { return Size == 0; }
    inline int          size() const                        { return Size; }
    inline int          size_in_bytes() const               { return Size * (int)sizeof(T); }
    inline int          max_size() const                    { return 0x7FFFFFFF / (int)sizeof(T); }
    inline int          capacity() const                    { return Capacity; }
    inline T&           operator[](int i)                   { KR_CORE_ASSERT(i >= 0 && i < Size, ""); return Data[i]; }
    inline const T&     operator[](int i) const             { KR_CORE_ASSERT(i >= 0 && i < Size, ""); return Data[i]; }

    inline T*           begin()                             { return Data; }
    inline const T*     begin() const                       { return Data; }
    inline T*           end()                               { return Data + Size; }
    inline const T*     end() const                         { return Data + Size; }
    inline T&           front()                             { KR_CORE_ASSERT(Size > 0, ""); return Data[0]; }
    inline const T&     front() const                       { KR_CORE_ASSERT(Size > 0, ""); return Data[0]; }
    inline T&           back()                              { KR_CORE_ASSERT(Size > 0, ""); return Data[Size - 1]; }
    inline const T&     back() const                        { KR_CORE_ASSERT(Size > 0, ""); return Data[Size - 1]; }
    inline void         swap(KGVector<T>& rhs)              { int rhs_size = rhs.Size; rhs.Size = Size; Size = rhs_size; int rhs_cap = rhs.Capacity; rhs.Capacity = Capacity; Capacity = rhs_cap; T* rhs_data = rhs.Data; rhs.Data = Data; Data = rhs_data; }

    inline int          _grow_capacity(int sz) const        { int new_capacity = Capacity ? (Capacity + Capacity / 2) : 8; return new_capacity > sz ? new_capacity : sz; }
    inline void         resize(int new_size)                { if (new_size > Capacity) reserve(_grow_capacity(new_size)); Size = new_size; }
    inline void         resize(int new_size, const T& v)    { if (new_size > Capacity) reserve(_grow_capacity(new_size)); if (new_size > Size) for (int n = Size; n < new_size; n++) memcpy(&Data[n], &v, sizeof(v)); Size = new_size; }
    inline void         shrink(int new_size)                { KR_CORE_ASSERT(new_size <= Size, ""); Size = new_size; } // Resize a vector to a smaller size, guaranteed not to cause a reallocation
    inline void         reserve(int new_capacity)           { if (new_capacity <= Capacity) return; T* new_data = (T*)KG_ALLOC((size_t)new_capacity * sizeof(T)); if (Data) { memcpy(new_data, Data, (size_t)Size * sizeof(T)); KG_FREE(Data); } Data = new_data; Capacity = new_capacity; }
    inline void         reserve_discard(int new_capacity)   { if (new_capacity <= Capacity) return; if (Data) KG_FREE(Data); Data = (T*)KG_ALLOC((size_t)new_capacity * sizeof(T)); Capacity = new_capacity; }

    // NB: It is illegal to call push_back/push_front/insert with a reference pointing inside the KGVector data itself! e.g. v.push_back(v[10]) is forbidden.
    inline void         push_back(const T& v)               { if (Size == Capacity) reserve(_grow_capacity(Size + 1)); memcpy(&Data[Size], &v, sizeof(v)); Size++; }
    inline void         pop_back()                          { KR_CORE_ASSERT(Size > 0, ""); Size--; }
    inline void         push_front(const T& v)              { if (Size == 0) push_back(v); else insert(Data, v); }
    inline T*           erase(const T* it)                  { KR_CORE_ASSERT(it >= Data && it < Data + Size, ""); const ptrdiff_t off = it - Data; memmove(Data + off, Data + off + 1, ((size_t)Size - (size_t)off - 1) * sizeof(T)); Size--; return Data + off; }
    inline T*           erase(const T* it, const T* it_last){ KR_CORE_ASSERT(it >= Data && it < Data + Size && it_last >= it && it_last <= Data + Size, ""); const ptrdiff_t count = it_last - it; const ptrdiff_t off = it - Data; memmove(Data + off, Data + off + count, ((size_t)Size - (size_t)off - (size_t)count) * sizeof(T)); Size -= (int)count; return Data + off; }
    inline T*           erase_unsorted(const T* it)         { KR_CORE_ASSERT(it >= Data && it < Data + Size, "");  const ptrdiff_t off = it - Data; if (it < Data + Size - 1) memcpy(Data + off, Data + Size - 1, sizeof(T)); Size--; return Data + off; }
    inline T*           insert(const T* it, const T& v)     { KR_CORE_ASSERT(it >= Data && it <= Data + Size, ""); const ptrdiff_t off = it - Data; if (Size == Capacity) reserve(_grow_capacity(Size + 1)); if (off < (int)Size) memmove(Data + off + 1, Data + off, ((size_t)Size - (size_t)off) * sizeof(T)); memcpy(&Data[off], &v, sizeof(v)); Size++; return Data + off; }
    inline bool         contains(const T& v) const          { const T* data = Data;  const T* data_end = Data + Size; while (data < data_end) if (*data++ == v) return true; return false; }
    inline T*           find(const T& v)                    { T* data = Data;  const T* data_end = Data + Size; while (data < data_end) if (*data == v) break; else ++data; return data; }
    inline const T*     find(const T& v) const              { const T* data = Data;  const T* data_end = Data + Size; while (data < data_end) if (*data == v) break; else ++data; return data; }
    inline bool         find_erase(const T& v)              { const T* it = find(v); if (it < Data + Size) { erase(it); return true; } return false; }
    inline bool         find_erase_unsorted(const T& v)     { const T* it = find(v); if (it < Data + Size) { erase_unsorted(it); return true; } return false; }
    inline int          index_from_ptr(const T* it) const   { KR_CORE_ASSERT(it >= Data && it < Data + Size, ""); const ptrdiff_t off = it - Data; return (int)off; }
};

//-----------------------------------------------------------------------------
// [SECTION] KarmaGuiStyle
//-----------------------------------------------------------------------------
// You may modify the ImGui::GetStyle() main instance during initialization and before NewFrame().
// During the frame, use ImGui::PushStyleVar(KGGuiStyleVar_XXXX)/PopStyleVar() to alter the main style values,
// and ImGui::PushStyleColor(KGGuiCol_XXX)/PopStyleColor() for colors.
//-----------------------------------------------------------------------------

struct KarmaGuiStyle
{
    float       Alpha;                      // Global alpha applies to everything in Dear ImGui.
    float       DisabledAlpha;              // Additional alpha multiplier applied by BeginDisabled(). Multiply over current value of Alpha.
    KGVec2      WindowPadding;              // Padding within a window.
    float       WindowRounding;             // Radius of window corners rounding. Set to 0.0f to have rectangular windows. Large values tend to lead to variety of artifacts and are not recommended.
    float       WindowBorderSize;           // Thickness of border around windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
    KGVec2      WindowMinSize;              // Minimum window size. This is a global setting. If you want to constrain individual windows, use SetNextWindowSizeConstraints().
    KGVec2      WindowTitleAlign;           // Alignment for title bar text. Defaults to (0.0f,0.5f) for left-aligned,vertically centered.
    KarmaGuiDir    WindowMenuButtonPosition;   // Side of the collapsing/docking button in the title bar (None/Left/Right). Defaults to KGGuiDir_Left.
    float       ChildRounding;              // Radius of child window corners rounding. Set to 0.0f to have rectangular windows.
    float       ChildBorderSize;            // Thickness of border around child windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
    float       PopupRounding;              // Radius of popup window corners rounding. (Note that tooltip windows use WindowRounding)
    float       PopupBorderSize;            // Thickness of border around popup/tooltip windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
    KGVec2      FramePadding;               // Padding within a framed rectangle (used by most widgets).
    float       FrameRounding;              // Radius of frame corners rounding. Set to 0.0f to have rectangular frame (used by most widgets).
    float       FrameBorderSize;            // Thickness of border around frames. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
    KGVec2      ItemSpacing;                // Horizontal and vertical spacing between widgets/lines.
    KGVec2      ItemInnerSpacing;           // Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label).
    KGVec2      CellPadding;                // Padding within a table cell
    KGVec2      TouchExtraPadding;          // Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
    float       IndentSpacing;              // Horizontal indentation when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
    float       ColumnsMinSpacing;          // Minimum horizontal spacing between two columns. Preferably > (FramePadding.x + 1).
    float       ScrollbarSize;              // Width of the vertical scrollbar, Height of the horizontal scrollbar.
    float       ScrollbarRounding;          // Radius of grab corners for scrollbar.
    float       GrabMinSize;                // Minimum width/height of a grab box for slider/scrollbar.
    float       GrabRounding;               // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
    float       LogSliderDeadzone;          // The size in pixels of the dead-zone around zero on logarithmic sliders that cross zero.
    float       TabRounding;                // Radius of upper corners of a tab. Set to 0.0f to have rectangular tabs.
    float       TabBorderSize;              // Thickness of border around tabs.
    float       TabMinWidthForCloseButton;  // Minimum width for close button to appear on an unselected tab when hovered. Set to 0.0f to always show when hovering, set to FLT_MAX to never show close button unless selected.
    KarmaGuiDir    ColorButtonPosition;        // Side of the color button in the ColorEdit4 widget (left/right). Defaults to KGGuiDir_Right.
    KGVec2      ButtonTextAlign;            // Alignment of button text when button is larger than text. Defaults to (0.5f, 0.5f) (centered).
    KGVec2      SelectableTextAlign;        // Alignment of selectable text. Defaults to (0.0f, 0.0f) (top-left aligned). It's generally important to keep this left-aligned if you want to lay multiple items on a same line.
    KGVec2      DisplayWindowPadding;       // Window position are clamped to be visible within the display area or monitors by at least this amount. Only applies to regular windows.
    KGVec2      DisplaySafeAreaPadding;     // If you cannot see the edges of your screen (e.g. on a TV) increase the safe area padding. Apply to popups/tooltips as well regular windows. NB: Prefer configuring your TV sets correctly!
    float       MouseCursorScale;           // Scale software rendered mouse cursor (when io.MouseDrawCursor is enabled). We apply per-monitor DPI scaling over this scale. May be removed later.
    bool        AntiAliasedLines;           // Enable anti-aliased lines/borders. Disable if you are really tight on CPU/GPU. Latched at the beginning of the frame (copied to KGDrawList).
    bool        AntiAliasedLinesUseTex;     // Enable anti-aliased lines/borders using textures where possible. Require backend to render with bilinear filtering (NOT point/nearest filtering). Latched at the beginning of the frame (copied to KGDrawList).
    bool        AntiAliasedFill;            // Enable anti-aliased edges around filled shapes (rounded rectangles, circles, etc.). Disable if you are really tight on CPU/GPU. Latched at the beginning of the frame (copied to KGDrawList).
    float       CurveTessellationTol;       // Tessellation tolerance when using PathBezierCurveTo() without a specific number of segments. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.
    float       CircleTessellationMaxError; // Maximum error (in pixels) allowed when using AddCircle()/AddCircleFilled() or drawing rounded corner rectangles with no explicit segment count specified. Decrease for higher quality but more geometry.
    KGVec4      Colors[KGGuiCol_COUNT];

    KarmaGuiStyle();
    void ScaleAllSizes(float scale_factor);
};

//-----------------------------------------------------------------------------
// [SECTION] KarmaGuiIO
//-----------------------------------------------------------------------------
// Communicate most settings and inputs/outputs to Dear ImGui using this structure.
// Access via ImGui::GetIO(). Read 'Programmer guide' section in .cpp file for general usage.
//-----------------------------------------------------------------------------

// [Internal] Storage used by IsKeyDown(), IsKeyPressed() etc functions.
// If prior to 1.87 you used io.KeysDownDuration[] (which was marked as internal), you should use GetKeyData(key)->DownDuration and *NOT* io.KeysData[key]->DownDuration.
struct KARMA_API KarmaGuiKeyData
{
    bool        Down;               // True for if key is down
    float       DownDuration;       // Duration the key has been down (<0.0f: not pressed, 0.0f: just pressed, >0.0f: time held)
    float       DownDurationPrev;   // Last frame duration the key has been down
    float       AnalogValue;        // 0.0f..1.0f for gamepad values
};

struct KARMA_API KarmaGuiIO
{
    //------------------------------------------------------------------
    // Configuration                            // Default value
    //------------------------------------------------------------------

    KarmaGuiConfigFlags   ConfigFlags;             // = 0              // See KGGuiConfigFlags_ enum. Set by user/application. Gamepad/keyboard navigation options, etc.
    KarmaGuiBackendFlags  BackendFlags;            // = 0              // See KGGuiBackendFlags_ enum. Set by backend (imgui_impl_xxx files or custom backend) to communicate features supported by the backend.
    KGVec2      DisplaySize;                    // <unset>          // Main display size, in pixels (generally == GetMainViewport()->Size). May change every frame.
    float       DeltaTime;                      // = 1.0f/60.0f     // Time elapsed since last frame, in seconds. May change every frame.
    float       IniSavingRate;                  // = 5.0f           // Minimum time between saving positions/sizes to .ini file, in seconds.
    const char* IniFilename;                    // = "kggui.ini"    // Path to .ini file (important: default "kggui.ini" is relative to current working dir!). Set NULL to disable automatic .ini loading/saving or if you want to manually call LoadIniSettingsXXX() / SaveIniSettingsXXX() functions.
    const char* LogFilename;                    // = "imgui_log.txt"// Path to .log file (default parameter to ImGui::LogToFile when no file is specified).
    float       MouseDoubleClickTime;           // = 0.30f          // Time for a double-click, in seconds.
    float       MouseDoubleClickMaxDist;        // = 6.0f           // Distance threshold to stay in to validate a double-click, in pixels.
    float       MouseDragThreshold;             // = 6.0f           // Distance threshold before considering we are dragging.
    float       KeyRepeatDelay;                 // = 0.275f         // When holding a key/button, time before it starts repeating, in seconds (for buttons in Repeat mode, etc.).
    float       KeyRepeatRate;                  // = 0.050f         // When holding a key/button, rate at which it repeats, in seconds.
    float       HoverDelayNormal;               // = 0.30 sec       // Delay on hovering before IsItemHovered(KGGuiHoveredFlags_DelayNormal) returns true.
    float       HoverDelayShort;                // = 0.10 sec       // Delay on hovering before IsItemHovered(KGGuiHoveredFlags_DelayShort) returns true.
    void*       UserData;                       // = NULL           // Store your own data.

    KGFontAtlas*Fonts;                          // <auto>           // Font atlas: load, rasterize and pack one or more fonts into a single texture.
    float       FontGlobalScale;                // = 1.0f           // Global scale all fonts
    bool        FontAllowUserScaling;           // = false          // Allow user scaling text of individual window with CTRL+Wheel.
    KGFont*     FontDefault;                    // = NULL           // Font to use on NewFrame(). Use NULL to uses Fonts->Fonts[0].
    KGVec2      DisplayFramebufferScale;        // = (1, 1)         // For retina display or other situations where window coordinates are different from framebuffer coordinates. This generally ends up in KGDrawData::FramebufferScale.

    // Docking options (when KGGuiConfigFlags_DockingEnable is set)
    bool        ConfigDockingNoSplit;           // = false          // Simplified docking mode: disable window splitting, so docking is limited to merging multiple windows together into tab-bars.
    bool        ConfigDockingWithShift;         // = false          // Enable docking with holding Shift key (reduce visual noise, allows dropping in wider space)
    bool        ConfigDockingAlwaysTabBar;      // = false          // [BETA] [FIXME: This currently creates regression with auto-sizing and general overhead] Make every single floating window display within a docking node.
    bool        ConfigDockingTransparentPayload;// = false          // [BETA] Make window or viewport transparent when docking and only display docking boxes on the target viewport. Useful if rendering of multiple viewport cannot be synced. Best used with ConfigViewportsNoAutoMerge.

    // Viewport options (when KGGuiConfigFlags_ViewportsEnable is set)
    bool        ConfigViewportsNoAutoMerge;     // = false;         // Set to make all floating imgui windows always create their own viewport. Otherwise, they are merged into the main host viewports when overlapping it. May also set KGGuiViewportFlags_NoAutoMerge on individual viewport.
    bool        ConfigViewportsNoTaskBarIcon;   // = false          // Disable default OS task bar icon flag for secondary viewports. When a viewport doesn't want a task bar icon, KGGuiViewportFlags_NoTaskBarIcon will be set on it.
    bool        ConfigViewportsNoDecoration;    // = true           // Disable default OS window decoration flag for secondary viewports. When a viewport doesn't want window decorations, KGGuiViewportFlags_NoDecoration will be set on it. Enabling decoration can create subsequent issues at OS levels (e.g. minimum window size).
    bool        ConfigViewportsNoDefaultParent; // = false          // Disable default OS parenting to main viewport for secondary viewports. By default, viewports are marked with ParentViewportId = <main_viewport>, expecting the platform backend to setup a parent/child relationship between the OS windows (some backend may ignore this). Set to true if you want the default to be 0, then all viewports will be top-level OS windows.

    // Miscellaneous options
    bool        MouseDrawCursor;                // = false          // Request ImGui to draw a mouse cursor for you (if you are on a platform without a mouse cursor). Cannot be easily renamed to 'io.ConfigXXX' because this is frequently used by backend implementations.
    bool        ConfigMacOSXBehaviors;          // = defined(__APPLE__) // OS X style: Text editing cursor movement using Alt instead of Ctrl, Shortcuts using Cmd/Super instead of Ctrl, Line/Text Start and End using Cmd+Arrows instead of Home/End, Double click selects by word instead of selecting whole text, Multi-selection in lists uses Cmd/Super instead of Ctrl.
    bool        ConfigInputTrickleEventQueue;   // = true           // Enable input queue trickling: some types of events submitted during the same frame (e.g. button down + up) will be spread over multiple frames, improving interactions with low framerates.
    bool        ConfigInputTextCursorBlink;     // = true           // Enable blinking cursor (optional as some users consider it to be distracting).
    bool        ConfigInputTextEnterKeepActive; // = false          // [BETA] Pressing Enter will keep item active and select contents (single-line only).
    bool        ConfigDragClickToInputText;     // = false          // [BETA] Enable turning DragXXX widgets into text input with a simple mouse click-release (without moving). Not desirable on devices without a keyboard.
    bool        ConfigWindowsResizeFromEdges;   // = true           // Enable resizing of windows from their edges and from the lower-left corner. This requires (io.BackendFlags & KGGuiBackendFlags_HasMouseCursors) because it needs mouse cursor feedback. (This used to be a per-window KGGuiWindowFlags_ResizeFromAnySide flag)
    bool        ConfigWindowsMoveFromTitleBarOnly; // = false       // Enable allowing to move windows only when clicking on their title bar. Does not apply to windows without a title bar.
    float       ConfigMemoryCompactTimer;       // = 60.0f          // Timer (in seconds) to free transient windows/tables memory buffers when unused. Set to -1.0f to disable.

    //------------------------------------------------------------------
    // Platform Functions
    // (the imgui_impl_xxxx backend files are setting those up for you)
    //------------------------------------------------------------------

    // Optional: Platform/Renderer backend name (informational only! will be displayed in About Window) + User data for backend/wrappers to store their own stuff.
    const char* BackendPlatformName;            // = NULL
    const char* BackendRendererName;            // = NULL
    void*       BackendPlatformUserData;        // = NULL           // User data for platform backend
    void*       BackendRendererUserData;        // = NULL           // User data for renderer backend
    void*       BackendLanguageUserData;        // = NULL           // User data for non C++ programming language backend

    // Optional: Access OS clipboard
    // (default to use native Win32 clipboard on Windows, otherwise uses a private clipboard. Override to access OS clipboard on other architectures)
    const char* (*GetClipboardTextFn)(void* user_data);
    void        (*SetClipboardTextFn)(void* user_data, const char* text);
    void*       ClipboardUserData;

    // Optional: Notify OS Input Method Editor of the screen position of your cursor for text input position (e.g. when using Japanese/Chinese IME on Windows)
    // (default to use native imm32 api on Windows)
    void        (*SetPlatformImeDataFn)(KarmaGuiViewport* viewport, KarmaGuiPlatformImeData* data);
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    void*       ImeWindowHandle;                // = NULL           // [Obsolete] Set KarmaGuiViewport::PlatformHandleRaw instead. Set this to your HWND to get automatic IME cursor positioning.
#else
    void*       _UnusedPadding;                                     // Unused field to keep data structure the same size.
#endif

    //------------------------------------------------------------------
    // Input - Call before calling NewFrame()
    //------------------------------------------------------------------

    // Input Functions
    void  AddKeyEvent(KarmaGuiKey key, bool down);                   // Queue a new key down/up event. Key should be "translated" (as in, generally KGGuiKey_A matches the key end-user would use to emit an 'A' character)
    void  AddKeyAnalogEvent(KarmaGuiKey key, bool down, float v);    // Queue a new key down/up event for analog values (e.g. KGGuiKey_Gamepad_ values). Dead-zones should be handled by the backend.
    void  AddMousePosEvent(float x, float y);                     // Queue a mouse position update. Use -FLT_MAX,-FLT_MAX to signify no mouse (e.g. app not focused and not hovered)
    void  AddMouseButtonEvent(int button, bool down);             // Queue a mouse button change
    void  AddMouseWheelEvent(float wh_x, float wh_y);             // Queue a mouse wheel update
    void  AddMouseViewportEvent(KGGuiID id);                      // Queue a mouse hovered viewport. Requires backend to set KGGuiBackendFlags_HasMouseHoveredViewport to call this (for multi-viewport support).
    void  AddFocusEvent(bool focused);                            // Queue a gain/loss of focus for the application (generally based on OS/platform focus of your window)
    void  AddInputCharacter(unsigned int c);                      // Queue a new character input
    void  AddInputCharacterUTF16(KGWchar16 c);                    // Queue a new character input from a UTF-16 character, it can be a surrogate
    void  AddInputCharactersUTF8(const char* str);                // Queue a new characters input from a UTF-8 string

    void  SetKeyEventNativeData(KarmaGuiKey key, int native_keycode, int native_scancode, int native_legacy_index = -1); // [Optional] Specify index for legacy <1.87 IsKeyXXX() functions with native indices + specify native keycode, scancode.
    void  SetAppAcceptingEvents(bool accepting_events);           // Set master flag for accepting key/mouse/text events (default to true). Useful if you have native dialog boxes that are interrupting your application loop/refresh, and you want to disable events being queued while your app is frozen.
    void  ClearInputCharacters();                                 // [Internal] Clear the text input buffer manually
    void  ClearInputKeys();                                       // [Internal] Release all keys

    //------------------------------------------------------------------
    // Output - Updated by NewFrame() or EndFrame()/Render()
    // (when reading from the io.WantCaptureMouse, io.WantCaptureKeyboard flags to dispatch your inputs, it is
    //  generally easier and more correct to use their state BEFORE calling NewFrame(). See FAQ for details!)
    //------------------------------------------------------------------

    bool        WantCaptureMouse;                   // Set when Dear ImGui will use mouse inputs, in this case do not dispatch them to your main game/application (either way, always pass on mouse inputs to imgui). (e.g. unclicked mouse is hovering over an imgui window, widget is active, mouse was clicked over an imgui window, etc.).
    bool        WantCaptureKeyboard;                // Set when Dear ImGui will use keyboard inputs, in this case do not dispatch them to your main game/application (either way, always pass keyboard inputs to imgui). (e.g. InputText active, or an imgui window is focused and navigation is enabled, etc.).
    bool        WantTextInput;                      // Mobile/console: when set, you may display an on-screen keyboard. This is set by Dear ImGui when it wants textual keyboard input to happen (e.g. when a InputText widget is active).
    bool        WantSetMousePos;                    // MousePos has been altered, backend should reposition mouse on next frame. Rarely used! Set only when KGGuiConfigFlags_NavEnableSetMousePos flag is enabled.
    bool        WantSaveIniSettings;                // When manual .ini load/save is active (io.IniFilename == NULL), this will be set to notify your application that you can call SaveIniSettingsToMemory() and save yourself. Important: clear io.WantSaveIniSettings yourself after saving!
    bool        NavActive;                          // Keyboard/Gamepad navigation is currently allowed (will handle KGGuiKey_NavXXX events) = a window is focused and it doesn't use the KGGuiWindowFlags_NoNavInputs flag.
    bool        NavVisible;                         // Keyboard/Gamepad navigation is visible and allowed (will handle KGGuiKey_NavXXX events).
    float       Framerate;                          // Estimate of application framerate (rolling average over 60 frames, based on io.DeltaTime), in frame per second. Solely for convenience. Slow applications may not want to use a moving average or may want to reset underlying buffers occasionally.
    int         MetricsRenderVertices;              // Vertices output during last call to Render()
    int         MetricsRenderIndices;               // Indices output during last call to Render() = number of triangles * 3
    int         MetricsRenderWindows;               // Number of visible windows
    int         MetricsActiveWindows;               // Number of active windows
    int         MetricsActiveAllocations;           // Number of active allocations, updated by MemAlloc/MemFree based on current context. May be off if you have multiple imgui contexts.
    KGVec2      MouseDelta;                         // Mouse delta. Note that this is zero if either current or previous position are invalid (-FLT_MAX,-FLT_MAX), so a disappearing/reappearing mouse won't have a huge delta.

    // Legacy: before 1.87, we required backend to fill io.KeyMap[] (imgui->native map) during initialization and io.KeysDown[] (native indices) every frame.
    // This is still temporarily supported as a legacy feature. However the new preferred scheme is for backend to call io.AddKeyEvent().
    //   Old (<1.87):  ImGui::IsKeyPressed(ImGui::GetIO().KeyMap[KGGuiKey_Space]) --> New (1.87+) ImGui::IsKeyPressed(KGGuiKey_Space)
#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
    int         KeyMap[KGGuiKey_COUNT];             // [LEGACY] Input: map of indices into the KeysDown[512] entries array which represent your "native" keyboard state. The first 512 are now unused and should be kept zero. Legacy backend will write into KeyMap[] using KGGuiKey_ indices which are always >512.
    bool        KeysDown[KGGuiKey_COUNT];           // [LEGACY] Input: Keyboard keys that are pressed (ideally left in the "native" order your engine has access to keyboard keys, so you can use your own defines/enums for keys). This used to be [512] sized. It is now KGGuiKey_COUNT to allow legacy io.KeysDown[GetKeyIndex(...)] to work without an overflow.
    float       NavInputs[KGGuiNavInput_COUNT];     // [LEGACY] Since 1.88, NavInputs[] was removed. Backends from 1.60 to 1.86 won't build. Feed gamepad inputs via io.AddKeyEvent() and KGGuiKey_GamepadXXX enums.
#endif

    //------------------------------------------------------------------
    // [Internal] Dear ImGui will maintain those fields. Forward compatibility not guaranteed!
    //------------------------------------------------------------------

    // Main Input State
    // (this block used to be written by backend, since 1.87 it is best to NOT write to those directly, call the AddXXX functions above instead)
    // (reading from those variables is fair game, as they are extremely unlikely to be moving anywhere)
    KGVec2      MousePos;                           // Mouse position, in pixels. Set to KGVec2(-FLT_MAX, -FLT_MAX) if mouse is unavailable (on another screen, etc.)
    bool        MouseDown[5];                       // Mouse buttons: 0=left, 1=right, 2=middle + extras (KGGuiMouseButton_COUNT == 5). Dear ImGui mostly uses left and right buttons. Other buttons allow us to track if the mouse is being used by your application + available to user as a convenience via IsMouse** API.
    float       MouseWheel;                         // Mouse wheel Vertical: 1 unit scrolls about 5 lines text.
    float       MouseWheelH;                        // Mouse wheel Horizontal. Most users don't have a mouse with a horizontal wheel, may not be filled by all backends.
    KGGuiID     MouseHoveredViewport;               // (Optional) Modify using io.AddMouseViewportEvent(). With multi-viewports: viewport the OS mouse is hovering. If possible _IGNORING_ viewports with the KGGuiViewportFlags_NoInputs flag is much better (few backends can handle that). Set io.BackendFlags |= KGGuiBackendFlags_HasMouseHoveredViewport if you can provide this info. If you don't imgui will infer the value using the rectangles and last focused time of the viewports it knows about (ignoring other OS windows).
    bool        KeyCtrl;                            // Keyboard modifier down: Control
    bool        KeyShift;                           // Keyboard modifier down: Shift
    bool        KeyAlt;                             // Keyboard modifier down: Alt
    bool        KeySuper;                           // Keyboard modifier down: Cmd/Super/Windows

    // Other state maintained from data above + IO function calls
    KarmaGuiKeyChord KeyMods;                          // Key mods flags (any of KGGuiMod_Ctrl/KGGuiMod_Shift/KGGuiMod_Alt/KGGuiMod_Super flags, same as io.KeyCtrl/KeyShift/KeyAlt/KeySuper but merged into flags. DOES NOT CONTAINS KGGuiMod_Shortcut which is pretranslated). Read-only, updated by NewFrame()
    KarmaGuiKeyData  KeysData[KGGuiKey_KeysData_SIZE]; // Key state for all known keys. Use IsKeyXXX() functions to access this.
    bool        WantCaptureMouseUnlessPopupClose;   // Alternative to WantCaptureMouse: (WantCaptureMouse == true && WantCaptureMouseUnlessPopupClose == false) when a click over void is expected to close a popup.
    KGVec2      MousePosPrev;                       // Previous mouse position (note that MouseDelta is not necessary == MousePos-MousePosPrev, in case either position is invalid)
    KGVec2      MouseClickedPos[5];                 // Position at time of clicking
    double      MouseClickedTime[5];                // Time of last click (used to figure out double-click)
    bool        MouseClicked[5];                    // Mouse button went from !Down to Down (same as MouseClickedCount[x] != 0)
    bool        MouseDoubleClicked[5];              // Has mouse button been double-clicked? (same as MouseClickedCount[x] == 2)
    KGU16       MouseClickedCount[5];               // == 0 (not clicked), == 1 (same as MouseClicked[]), == 2 (double-clicked), == 3 (triple-clicked) etc. when going from !Down to Down
    KGU16       MouseClickedLastCount[5];           // Count successive number of clicks. Stays valid after mouse release. Reset after another click is done.
    bool        MouseReleased[5];                   // Mouse button went from Down to !Down
    bool        MouseDownOwned[5];                  // Track if button was clicked inside a dear imgui window or over void blocked by a popup. We don't request mouse capture from the application if click started outside ImGui bounds.
    bool        MouseDownOwnedUnlessPopupClose[5];  // Track if button was clicked inside a dear imgui window.
    float       MouseDownDuration[5];               // Duration the mouse button has been down (0.0f == just clicked)
    float       MouseDownDurationPrev[5];           // Previous time the mouse button has been down
    KGVec2      MouseDragMaxDistanceAbs[5];         // Maximum distance, absolute, on each axis, of how much mouse has traveled from the clicking point
    float       MouseDragMaxDistanceSqr[5];         // Squared maximum distance of how much mouse has traveled from the clicking point (used for moving thresholds)
    float       PenPressure;                        // Touch/Pen pressure (0.0f to 1.0f, should be >0.0f only when MouseDown[0] == true). Helper storage currently unused by Dear ImGui.
    bool        AppFocusLost;                       // Only modify via AddFocusEvent()
    bool        AppAcceptingEvents;                 // Only modify via SetAppAcceptingEvents()
    KGS8        BackendUsingLegacyKeyArrays;        // -1: unknown, 0: using AddKeyEvent(), 1: using legacy io.KeysDown[]
    bool        BackendUsingLegacyNavInputArray;    // 0: using AddKeyAnalogEvent(), 1: writing to legacy io.NavInputs[] directly
    KGWchar16   InputQueueSurrogate;                // For AddInputCharacterUTF16()
    KGVector<KGWchar> InputQueueCharacters;         // Queue of _characters_ input (obtained by platform backend). Fill using AddInputCharacter() helper.

    KarmaGuiIO();
};

//-----------------------------------------------------------------------------
// [SECTION] Misc data structures
//-----------------------------------------------------------------------------

// Shared state of InputText(), passed as an argument to your callback when a KGGuiInputTextFlags_Callback* flag is used.
// The callback function should return 0 by default.
// Callbacks (follow a flag name and see comments in KGGuiInputTextFlags_ declarations for more details)
// - KGGuiInputTextFlags_CallbackEdit:        Callback on buffer edit (note that InputText() already returns true on edit, the callback is useful mainly to manipulate the underlying buffer while focus is active)
// - KGGuiInputTextFlags_CallbackAlways:      Callback on each iteration
// - KGGuiInputTextFlags_CallbackCompletion:  Callback on pressing TAB
// - KGGuiInputTextFlags_CallbackHistory:     Callback on pressing Up/Down arrows
// - KGGuiInputTextFlags_CallbackCharFilter:  Callback on character inputs to replace or discard them. Modify 'EventChar' to replace or discard, or return 1 in callback to discard.
// - KGGuiInputTextFlags_CallbackResize:      Callback on buffer capacity changes request (beyond 'buf_size' parameter value), allowing the string to grow.
struct KARMA_API KarmaGuiInputTextCallbackData
{
    KarmaGuiInputTextFlags EventFlag;      // One KGGuiInputTextFlags_Callback*    // Read-only
    KarmaGuiInputTextFlags Flags;          // What user passed to InputText()      // Read-only
    void*               UserData;       // What user passed to InputText()      // Read-only

    // Arguments for the different callback events
    // - To modify the text buffer in a callback, prefer using the InsertChars() / DeleteChars() function. InsertChars() will take care of calling the resize callback if necessary.
    // - If you know your edits are not going to resize the underlying buffer allocation, you may modify the contents of 'Buf[]' directly. You need to update 'BufTextLen' accordingly (0 <= BufTextLen < BufSize) and set 'BufDirty'' to true so InputText can update its internal state.
    KGWchar             EventChar;      // Character input                      // Read-write   // [CharFilter] Replace character with another one, or set to zero to drop. return 1 is equivalent to setting EventChar=0;
    KarmaGuiKey            EventKey;       // Key pressed (Up/Down/TAB)            // Read-only    // [Completion,History]
    char*               Buf;            // Text buffer                          // Read-write   // [Resize] Can replace pointer / [Completion,History,Always] Only write to pointed data, don't replace the actual pointer!
    int                 BufTextLen;     // Text length (in bytes)               // Read-write   // [Resize,Completion,History,Always] Exclude zero-terminator storage. In C land: == strlen(some_text), in C++ land: string.length()
    int                 BufSize;        // Buffer size (in bytes) = capacity+1  // Read-only    // [Resize,Completion,History,Always] Include zero-terminator storage. In C land == ARRAYSIZE(my_char_array), in C++ land: string.capacity()+1
    bool                BufDirty;       // Set if you modify Buf/BufTextLen!    // Write        // [Completion,History,Always]
    int                 CursorPos;      //                                      // Read-write   // [Completion,History,Always]
    int                 SelectionStart; //                                      // Read-write   // [Completion,History,Always] == to SelectionEnd when no selection)
    int                 SelectionEnd;   //                                      // Read-write   // [Completion,History,Always]

    // Helper functions for text manipulation.
    // Use those function to benefit from the CallbackResize behaviors. Calling those function reset the selection.
    KarmaGuiInputTextCallbackData();
    static void      DeleteChars(int pos, int bytes_count);
    static void      InsertChars(int pos, const char* text, const char* text_end = NULL);
    void                SelectAll()             { SelectionStart = 0; SelectionEnd = BufTextLen; }
    void                ClearSelection()        { SelectionStart = SelectionEnd = BufTextLen; }
    bool                HasSelection() const    { return SelectionStart != SelectionEnd; }
};

// Resizing callback data to apply custom constraint. As enabled by SetNextWindowSizeConstraints(). Callback is called during the next Begin().
// NB: For basic min/max size constraint on each axis you don't need to use the callback! The SetNextWindowSizeConstraints() parameters are enough.
struct KARMA_API KarmaGuiSizeCallbackData
{
    void*   UserData;       // Read-only.   What user passed to SetNextWindowSizeConstraints(). Generally store an integer or float in here (need reinterpret_cast<>).
    KGVec2  Pos;            // Read-only.   Window position, for reference.
    KGVec2  CurrentSize;    // Read-only.   Current window size.
    KGVec2  DesiredSize;    // Read-write.  Desired size, based on user's mouse position. Write to this field to restrain resizing.
};

// [ALPHA] Rarely used / very advanced uses only. Use with SetNextWindowClass() and DockSpace() functions.
// Important: the content of this class is still highly WIP and likely to change and be refactored
// before we stabilize Docking features. Please be mindful if using this.
// Provide hints:
// - To the platform backend via altered viewport flags (enable/disable OS decoration, OS task bar icons, etc.)
// - To the platform backend for OS level parent/child relationships of viewport.
// - To the docking system for various options and filtering.
struct KARMA_API KarmaGuiWindowClass
{
    KGGuiID             ClassId;                    // User data. 0 = Default class (unclassed). Windows of different classes cannot be docked with each others.
    KGGuiID             ParentViewportId;           // Hint for the platform backend. -1: use default. 0: request platform backend to not parent the platform. != 0: request platform backend to create a parent<>child relationship between the platform windows. Not conforming backends are free to e.g. parent every viewport to the main viewport or not.
    KarmaGuiViewportFlags  ViewportFlagsOverrideSet;   // Viewport flags to set when a window of this class owns a viewport. This allows you to enforce OS decoration or task bar icon, override the defaults on a per-window basis.
    KarmaGuiViewportFlags  ViewportFlagsOverrideClear; // Viewport flags to clear when a window of this class owns a viewport. This allows you to enforce OS decoration or task bar icon, override the defaults on a per-window basis.
    KarmaGuiTabItemFlags   TabItemFlagsOverrideSet;    // [EXPERIMENTAL] TabItem flags to set when a window of this class gets submitted into a dock node tab bar. May use with KGGuiTabItemFlags_Leading or KGGuiTabItemFlags_Trailing.
    KarmaGuiDockNodeFlags  DockNodeFlagsOverrideSet;   // [EXPERIMENTAL] Dock node flags to set when a window of this class is hosted by a dock node (it doesn't have to be selected!)
    bool                DockingAlwaysTabBar;        // Set to true to enforce single floating windows of this class always having their own docking node (equivalent of setting the global io.ConfigDockingAlwaysTabBar)
    bool                DockingAllowUnclassed;      // Set to true to allow windows of this class to be docked/merged with an unclassed window. // FIXME-DOCK: Move to DockNodeFlags override?

    KarmaGuiWindowClass() { memset(this, 0, sizeof(*this)); ParentViewportId = (KGGuiID)-1; DockingAllowUnclassed = true; }
};

// Data payload for Drag and Drop operations: AcceptDragDropPayload(), GetDragDropPayload()
struct KARMA_API KarmaGuiPayload
{
    // Members
    void*           Data;               // Data (copied and owned by dear imgui)
    int             DataSize;           // Data size

    // [Internal]
    KGGuiID         SourceId;           // Source item id
    KGGuiID         SourceParentId;     // Source parent id (if available)
    int             DataFrameCount;     // Data timestamp
    char            DataType[32 + 1];   // Data type tag (short user-supplied string, 32 characters max)
    bool            Preview;            // Set when AcceptDragDropPayload() was called and mouse has been hovering the target item (nb: handle overlapping drag targets)
    bool            Delivery;           // Set when AcceptDragDropPayload() was called and mouse button is released over the target item.

    KarmaGuiPayload()  { Clear(); }
    void Clear()    { SourceId = SourceParentId = 0; Data = NULL; DataSize = 0; memset(DataType, 0, sizeof(DataType)); DataFrameCount = -1; Preview = Delivery = false; }
    bool IsDataType(const char* type) const { return DataFrameCount != -1 && strcmp(type, DataType) == 0; }
    bool IsPreview() const                  { return Preview; }
    bool IsDelivery() const                 { return Delivery; }
};

// Sorting specification for one column of a table (sizeof == 12 bytes)
struct KARMA_API KarmaGuiTableColumnSortSpecs
{
    KGGuiID                     ColumnUserID;       // User id of the column (if specified by a TableSetupColumn() call)
    KGS16                       ColumnIndex;        // Index of the column
    KGS16                       SortOrder;          // Index within parent KarmaGuiTableSortSpecs (always stored in order starting from 0, tables sorted on a single criteria will always have a 0 here)
    KarmaGuiSortDirection          SortDirection : 8;  // KGGuiSortDirection_Ascending or KGGuiSortDirection_Descending (you can use this or SortSign, whichever is more convenient for your sort function)

    KarmaGuiTableColumnSortSpecs() { memset(this, 0, sizeof(*this)); }
};

// Sorting specifications for a table (often handling sort specs for a single column, occasionally more)
// Obtained by calling TableGetSortSpecs().
// When 'SpecsDirty == true' you can sort your data. It will be true with sorting specs have changed since last call, or the first time.
// Make sure to set 'SpecsDirty = false' after sorting, else you may wastefully sort your data every frame!
struct KARMA_API KarmaGuiTableSortSpecs
{
    const KarmaGuiTableColumnSortSpecs* Specs;     // Pointer to sort spec array.
    int                         SpecsCount;     // Sort spec count. Most often 1. May be > 1 when KGGuiTableFlags_SortMulti is enabled. May be == 0 when KGGuiTableFlags_SortTristate is enabled.
    bool                        SpecsDirty;     // Set to true when specs have changed since last time! Use this to sort again, then clear the flag.

    KarmaGuiTableSortSpecs()       { memset(this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] Helpers (KarmaGuiOnceUponAFrame, KarmaGuiTextFilter, KarmaGuiTextBuffer, KarmaGuiStorage, KarmaGuiListClipper, KGColor)
//-----------------------------------------------------------------------------

// Helper: Unicode defines
#define KG_UNICODE_CODEPOINT_INVALID 0xFFFD     // Invalid Unicode code point (standard value).
#define KG_UNICODE_CODEPOINT_MAX     0xFFFF     // Maximum Unicode code point supported by this build.


// Helper: Execute a block of code at maximum once a frame. Convenient if you want to quickly create a UI within deep-nested code that runs multiple times every frame.
// Usage: static KarmaGuiOnceUponAFrame oaf; if (oaf) ImGui::Text("This will be called only once per frame");
struct KARMA_API KarmaGuiOnceUponAFrame
{
    KarmaGuiOnceUponAFrame() { RefFrame = -1; }
    mutable int RefFrame;
    operator bool() const { int current_frame = Karma::KarmaGui::GetFrameCount(); if (RefFrame == current_frame) return false; RefFrame = current_frame; return true; }
};

// Helper: Parse and apply text filters. In format "aaaaa[,bbbb][,ccccc]"
struct KARMA_API KarmaGuiTextFilter
{
    KarmaGuiTextFilter(const char* default_filter = "");
    bool      Draw(const char* label = "Filter (inc,-exc)", float width = 0.0f);  // Helper calling InputText+Build
    bool      PassFilter(const char* text, const char* text_end = NULL) const;
    void      Build();
    void                Clear()          { InputBuf[0] = 0; Build(); }
    bool                IsActive() const { return !Filters.empty(); }

    // [Internal]
    struct KARMA_API ImGuiTextRange
    {
        const char*     b;
        const char*     e;

        ImGuiTextRange()                                { b = e = NULL; }
        ImGuiTextRange(const char* _b, const char* _e)  { b = _b; e = _e; }
        bool            empty() const                   { return b == e; }
        void  split(char separator, KGVector<ImGuiTextRange>* out) const;
    };
    char                    InputBuf[256];
    KGVector<ImGuiTextRange>Filters;
    int                     CountGrep;
};

// Helper: Growable text buffer for logging/accumulating text
// (this could be called 'ImGuiTextBuilder' / 'ImGuiStringBuilder')
struct KARMA_API KarmaGuiTextBuffer
{
    KGVector<char>      Buf;
    static char EmptyString[1];

    KarmaGuiTextBuffer()   { }
    inline char         operator[](int i) const { KR_CORE_ASSERT(Buf.Data != NULL, ""); return Buf.Data[i]; }
    const char*         begin() const           { return Buf.Data ? &Buf.front() : EmptyString; }
    const char*         end() const             { return Buf.Data ? &Buf.back() : EmptyString; }   // Buf is zero-terminated, so end() will point on the zero-terminator
    int                 size() const            { return Buf.Size ? Buf.Size - 1 : 0; }
    bool                empty() const           { return Buf.Size <= 1; }
    void                clear()                 { Buf.clear(); }
    void                reserve(int capacity)   { Buf.reserve(capacity); }
    const char*         c_str() const           { return Buf.Data ? Buf.Data : EmptyString; }
    void      append(const char* str, const char* str_end = NULL);
    void      appendf(const char* fmt, ...) KG_FMTARGS(2);
    void      appendfv(const char* fmt, va_list args) KG_FMTLIST(2);
};

// Helper: Key->Value storage
// Typically you don't have to worry about this since a storage is held within each Window.
// We use it to e.g. store collapse state for a tree (Int 0/1)
// This is optimized for efficient lookup (dichotomy into a contiguous buffer) and rare insertion (typically tied to user interactions aka max once a frame)
// You can use it as custom user storage for temporary values. Declare your own storage if, for example:
// - You want to manipulate the open/close state of a particular sub-tree in your interface (tree node uses Int 0/1 to store their state).
// - You want to store custom debug data easily without adding or editing structures in your code (probably not efficient, but convenient)
// Types are NOT stored, so it is up to you to make sure your Key don't collide with different types.
struct KARMA_API KarmaGuiStorage
{
    // [Internal]
    struct ImGuiStoragePair
    {
        KGGuiID key;
        union { int val_i; float val_f; void* val_p; };
        ImGuiStoragePair(KGGuiID _key, int _val_i)      { key = _key; val_i = _val_i; }
        ImGuiStoragePair(KGGuiID _key, float _val_f)    { key = _key; val_f = _val_f; }
        ImGuiStoragePair(KGGuiID _key, void* _val_p)    { key = _key; val_p = _val_p; }
    };

    KGVector<ImGuiStoragePair>      Data;

    // - Get***() functions find pair, never add/allocate. Pairs are sorted so a query is O(log N)
    // - Set***() functions find pair, insertion on demand if missing.
    // - Sorted insertion is costly, paid once. A typical frame shouldn't need to insert any new pair.
    void                Clear() { Data.clear(); }
    int       GetInt(KGGuiID key, int default_val = 0) const;
    void      SetInt(KGGuiID key, int val);
    bool      GetBool(KGGuiID key, bool default_val = false) const;
    void      SetBool(KGGuiID key, bool val);
    float     GetFloat(KGGuiID key, float default_val = 0.0f) const;
    void      SetFloat(KGGuiID key, float val);
    void*     GetVoidPtr(KGGuiID key) const; // default_val is NULL
    void      SetVoidPtr(KGGuiID key, void* val);

    // - Get***Ref() functions finds pair, insert on demand if missing, return pointer. Useful if you intend to do Get+Set.
    // - References are only valid until a new value is added to the storage. Calling a Set***() function or a Get***Ref() function invalidates the pointer.
    // - A typical use case where this is convenient for quick hacking (e.g. add storage during a live Edit&Continue session if you can't modify existing struct)
    //      float* pvar = ImGui::GetFloatRef(key); ImGui::SliderFloat("var", pvar, 0, 100.0f); some_var += *pvar;
    int*      GetIntRef(KGGuiID key, int default_val = 0);
    bool*     GetBoolRef(KGGuiID key, bool default_val = false);
    float*    GetFloatRef(KGGuiID key, float default_val = 0.0f);
    void**    GetVoidPtrRef(KGGuiID key, void* default_val = NULL);

    // Use on your own storage if you know only integer are being stored (open/close all tree nodes)
    void      SetAllInt(int val);

    // For quicker full rebuild of a storage (instead of an incremental one), you may add all your contents and then sort once.
    void      BuildSortByKey();
};

// Helper: Manually clip large list of items.
// If you have lots evenly spaced items and you have random access to the list, you can perform coarse
// clipping based on visibility to only submit items that are in view.
// The clipper calculates the range of visible items and advance the cursor to compensate for the non-visible items we have skipped.
// (Dear ImGui already clip items based on their bounds but: it needs to first layout the item to do so, and generally
//  fetching/submitting your own data incurs additional cost. Coarse clipping using KarmaGuiListClipper allows you to easily
//  scale using lists with tens of thousands of items without a problem)
// Usage:
//   KarmaGuiListClipper clipper;
//   clipper.Begin(1000);         // We have 1000 elements, evenly spaced.
//   while (clipper.Step())
//       for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
//           ImGui::Text("line number %d", i);
// Generally what happens is:
// - Clipper lets you process the first element (DisplayStart = 0, DisplayEnd = 1) regardless of it being visible or not.
// - User code submit that one element.
// - Clipper can measure the height of the first element
// - Clipper calculate the actual range of elements to display based on the current clipping rectangle, position the cursor before the first visible element.
// - User code submit visible elements.
// - The clipper also handles various subtleties related to keyboard/gamepad navigation, wrapping etc.
struct KARMA_API KarmaGuiListClipper
{
    int             DisplayStart;       // First item to display, updated by each call to Step()
    int             DisplayEnd;         // End of items to display (exclusive)
    int             ItemsCount;         // [Internal] Number of items
    float           ItemsHeight;        // [Internal] Height of item after a first step and item submission can calculate it
    float           StartPosY;          // [Internal] Cursor position at the time of Begin() or after table frozen rows are all processed
    void*           TempData;           // [Internal] Internal data

    // items_count: Use INT_MAX if you don't know how many items you have (in which case the cursor won't be advanced in the final step)
    // items_height: Use -1.0f to be calculated automatically on first step. Otherwise pass in the distance between your items, typically GetTextLineHeightWithSpacing() or GetFrameHeightWithSpacing().
    KarmaGuiListClipper();
    ~KarmaGuiListClipper();
    void  Begin(int items_count, float items_height = -1.0f);
    void  End();             // Automatically called on the last call of Step() that returns false.
    bool  Step();            // Call until it returns false. The DisplayStart/DisplayEnd fields will be set and you can process/draw those items.

    // Call ForceDisplayRangeByIndices() before first call to Step() if you need a range of items to be displayed regardless of visibility.
    void  ForceDisplayRangeByIndices(int item_min, int item_max); // item_max is exclusive e.g. use (42, 42+1) to make item 42 always visible BUT due to alignment/padding of certain items it is likely that an extra item may be included on either end of the display range.
    inline KarmaGuiListClipper(int items_count, float items_height = -1.0f) { memset(this, 0, sizeof(*this)); ItemsCount = -1; Begin(items_count, items_height); } // [removed in 1.79]
};

// Helpers macros to generate 32-bit encoded colors
// User can declare their own format by #defining the 5 _SHIFT/_MASK macros in their imconfig file.
#ifndef KG_COL32_R_SHIFT
#define KG_COL32_R_SHIFT    0
#define KG_COL32_G_SHIFT    8
#define KG_COL32_B_SHIFT    16
#define KG_COL32_A_SHIFT    24
#define KG_COL32_A_MASK     0xFF000000
#endif
#define KG_COL32(R,G,B,A)    (((KGU32)(A)<<KG_COL32_A_SHIFT) | ((KGU32)(B)<<KG_COL32_B_SHIFT) | ((KGU32)(G)<<KG_COL32_G_SHIFT) | ((KGU32)(R)<<KG_COL32_R_SHIFT))
#define KG_COL32_WHITE       KG_COL32(255,255,255,255)  // Opaque white = 0xFFFFFFFF
#define KG_COL32_BLACK       KG_COL32(0,0,0,255)        // Opaque black
#define KG_COL32_BLACK_TRANS KG_COL32(0,0,0,0)          // Transparent black = 0x00000000

// Helper: KGColor() implicitly converts colors to either KGU32 (packed 4x1 byte) or KGVec4 (4x1 float)
// Prefer using KG_COL32() macros if you want a guaranteed compile-time KGU32 for usage with KGDrawList API.
// **Avoid storing KGColor! Store either u32 of KGVec4. This is not a full-featured color class. MAY OBSOLETE.
// **None of the ImGui API are using KGColor directly but you can use it as a convenience to pass colors in either KGU32 or KGVec4 formats. Explicitly cast to KGU32 or KGVec4 if needed.
struct KARMA_API KGColor
{
    KGVec4          Value;

    constexpr KGColor()                                             { }
    constexpr KGColor(float r, float g, float b, float a = 1.0f)    : Value(r, g, b, a) { }
    constexpr KGColor(const KGVec4& col)                            : Value(col) {}
    KGColor(int r, int g, int b, int a = 255)                       { float sc = 1.0f / 255.0f; Value.x = (float)r * sc; Value.y = (float)g * sc; Value.z = (float)b * sc; Value.w = (float)a * sc; }
    KGColor(KGU32 rgba)                                             { float sc = 1.0f / 255.0f; Value.x = (float)((rgba >> KG_COL32_R_SHIFT) & 0xFF) * sc; Value.y = (float)((rgba >> KG_COL32_G_SHIFT) & 0xFF) * sc; Value.z = (float)((rgba >> KG_COL32_B_SHIFT) & 0xFF) * sc; Value.w = (float)((rgba >> KG_COL32_A_SHIFT) & 0xFF) * sc; }
    inline operator KGU32() const                                   { return Karma::KarmaGui::ColorConvertFloat4ToU32(Value); }
    inline operator KGVec4() const                                  { return Value; }

    // FIXME-OBSOLETE: May need to obsolete/cleanup those helpers.
    inline void    SetHSV(float h, float s, float v, float a = 1.0f){ Karma::KarmaGui::ColorConvertHSVtoRGB(h, s, v, Value.x, Value.y, Value.z); Value.w = a; }
    KGColor HSV(float h, float s, float v, float a = 1.0f)   { float r, g, b; Karma::KarmaGui::ColorConvertHSVtoRGB(h, s, v, r, g, b); return KGColor(r, g, b, a); }
};

//-----------------------------------------------------------------------------
// [SECTION] Drawing API (KGDrawCmd, KGDrawIdx, KGDrawVert, KGDrawChannel, KGDrawListSplitter, KGDrawListFlags, KGDrawList, KGDrawData)
// Hold a series of drawing commands. The user provides a renderer for KGDrawData which essentially contains an array of KGDrawList.
//-----------------------------------------------------------------------------

// The maximum line width to bake anti-aliased textures for. Build atlas with KGFontAtlasFlags_NoBakedLines to disable baking.
#ifndef KG_DRAWLIST_TEX_LINES_WIDTH_MAX
#define KG_DRAWLIST_TEX_LINES_WIDTH_MAX     (63)
#endif

// KGDrawCallback: Draw callbacks for advanced uses [configurable type: override in imconfig.h]
// NB: You most likely do NOT need to use draw callbacks just to create your own widget or customized UI rendering,
// you can poke into the draw list for that! Draw callback may be useful for example to:
//  A) Change your GPU render state,
//  B) render a complex 3D scene inside a UI element without an intermediate texture/render target, etc. Yes
// The expected behavior from your rendering function is 'if (cmd.UserCallback != NULL) { cmd.UserCallback(parent_list, cmd); } else { RenderTriangles() }'
// If you want to override the signature of KGDrawCallback, you can simply use e.g. '#define KGDrawCallback MyDrawCallback' (in imconfig.h) + update rendering backend accordingly.
#ifndef KGDrawCallback
typedef void (*KGDrawCallback)(const KGDrawList* parent_list, const KGDrawCmd* cmd);
#endif

// Special Draw callback value to request renderer backend to reset the graphics/render state.
// The renderer backend needs to handle this special value, otherwise it will crash trying to call a function at this address.
// This is useful for example if you submitted callbacks which you know have altered the render state and you want it to be restored.
// It is not done by default because they are many perfectly useful way of altering render state for imgui contents (e.g. changing shader/blending settings before an Image call).
#define KGDrawCallback_ResetRenderState     (KGDrawCallback)(-1)

// Typically, 1 command = 1 GPU draw call (unless command is a callback)
// - VtxOffset: When 'io.BackendFlags & KGGuiBackendFlags_RendererHasVtxOffset' is enabled,
//   this fields allow us to render meshes larger than 64K vertices while keeping 16-bit indices.
//   Backends made for <1.71. will typically ignore the VtxOffset fields.
// - The ClipRect/TextureId/VtxOffset fields must be contiguous as we memcmp() them together (this is asserted for).
struct KARMA_API KGDrawCmd
{
    KGVec4          ClipRect;           // 4*4  // Clipping rectangle (x1, y1, x2, y2). Subtract KGDrawData->DisplayPos to get clipping rectangle in "viewport" coordinates
    KGTextureID     TextureId;          // 4-8  // User-provided texture ID. Set by user in ImfontAtlas::SetTexID() for fonts or passed to Image*() functions. Ignore if never using images or multiple fonts atlas.
    unsigned int    VtxOffset;          // 4    // Start offset in vertex buffer. KGGuiBackendFlags_RendererHasVtxOffset: always 0, otherwise may be >0 to support meshes larger than 64K vertices with 16-bit indices.
    unsigned int    IdxOffset;          // 4    // Start offset in index buffer.
    unsigned int    ElemCount;          // 4    // Number of indices (multiple of 3) to be rendered as triangles. Vertices are stored in the callee KGDrawList's vtx_buffer[] array, indices in idx_buffer[].
    KGDrawCallback  UserCallback;       // 4-8  // If != NULL, call the function instead of rendering the vertices. clip_rect and texture_id will be set normally.
    void*           UserCallbackData;   // 4-8  // The draw callback code can access this.

    KGDrawCmd() { memset(this, 0, sizeof(*this)); } // Also ensure our padding fields are zeroed

    // Since 1.83: returns KGTextureID associated with this draw call. Warning: DO NOT assume this is always same as 'TextureId' (we will change this function for an upcoming feature)
    inline KGTextureID GetTexID() const { return TextureId; }
};

// Vertex layout
// try think about shaders rather for better integration with Karma
#ifndef IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT
struct KARMA_API KGDrawVert
{
    KGVec2  pos;
    KGVec2  uv;
    KGU32   col;
};
#else
// You can override the vertex format layout by defining IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT in imconfig.h
// The code expect KGVec2 pos (8 bytes), KGVec2 uv (8 bytes), KGU32 col (4 bytes), but you can re-order them or add other fields as needed to simplify integration in your engine.
// The type has to be described within the macro (you can either declare the struct or use a typedef). This is because KGVec2/KGU32 are likely not declared at the time you'd want to set your type up.
// NOTE: IMGUI DOESN'T CLEAR THE STRUCTURE AND DOESN'T CALL A CONSTRUCTOR SO ANY CUSTOM FIELD WILL BE UNINITIALIZED. IF YOU ADD EXTRA FIELDS (SUCH AS A 'Z' COORDINATES) YOU WILL NEED TO CLEAR THEM DURING RENDER OR TO IGNORE THEM.
IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT;
#endif

// [Internal] For use by KGDrawList
struct ImDrawCmdHeader
{
    KGVec4          ClipRect;
    KGTextureID     TextureId;
    unsigned int    VtxOffset;
};

// [Internal] For use by KGDrawListSplitter
struct KGDrawChannel
{
    KGVector<KGDrawCmd>         _CmdBuffer;
    KGVector<KGDrawIdx>         _IdxBuffer;
};


// Split/Merge functions are used to split the draw list into different layers which can be drawn into out of order.
// This is used by the Columns/Tables API, so items of each column can be batched together in a same draw call.
struct KARMA_API KGDrawListSplitter
{
    int                         _Current;    // Current channel number (0)
    int                         _Count;      // Number of active channels (1+)
    KGVector<KGDrawChannel>     _Channels;   // Draw channels (not resized down so _Count might be < Channels.Size)

    inline KGDrawListSplitter()  { memset(this, 0, sizeof(*this)); }
    inline ~KGDrawListSplitter() { ClearFreeMemory(); }
    inline void                 Clear() { _Current = 0; _Count = 1; } // Do not clear Channels[] so our allocations are reused next frame
    static void              ClearFreeMemory();
    static void              Split(KGDrawList* draw_list, int count);
    static void              Merge(KGDrawList* draw_list);
    static void              SetCurrentChannel(KGDrawList* draw_list, int channel_idx);
};

// Flags for KGDrawList functions
// (Legacy: bit 0 must always correspond to KGDrawFlags_Closed to be backward compatible with old API using a bool. Bits 1..3 must be unused)
enum KGDrawFlags_
{
    KGDrawFlags_None                        = 0,
    KGDrawFlags_Closed                      = 1 << 0, // PathStroke(), AddPolyline(): specify that shape should be closed (Important: this is always == 1 for legacy reason)
    KGDrawFlags_RoundCornersTopLeft         = 1 << 4, // AddRect(), AddRectFilled(), PathRect(): enable rounding top-left corner only (when rounding > 0.0f, we default to all corners). Was 0x01.
    KGDrawFlags_RoundCornersTopRight        = 1 << 5, // AddRect(), AddRectFilled(), PathRect(): enable rounding top-right corner only (when rounding > 0.0f, we default to all corners). Was 0x02.
    KGDrawFlags_RoundCornersBottomLeft      = 1 << 6, // AddRect(), AddRectFilled(), PathRect(): enable rounding bottom-left corner only (when rounding > 0.0f, we default to all corners). Was 0x04.
    KGDrawFlags_RoundCornersBottomRight     = 1 << 7, // AddRect(), AddRectFilled(), PathRect(): enable rounding bottom-right corner only (when rounding > 0.0f, we default to all corners). Wax 0x08.
    KGDrawFlags_RoundCornersNone            = 1 << 8, // AddRect(), AddRectFilled(), PathRect(): disable rounding on all corners (when rounding > 0.0f). This is NOT zero, NOT an implicit flag!
    KGDrawFlags_RoundCornersTop             = KGDrawFlags_RoundCornersTopLeft | KGDrawFlags_RoundCornersTopRight,
    KGDrawFlags_RoundCornersBottom          = KGDrawFlags_RoundCornersBottomLeft | KGDrawFlags_RoundCornersBottomRight,
    KGDrawFlags_RoundCornersLeft            = KGDrawFlags_RoundCornersBottomLeft | KGDrawFlags_RoundCornersTopLeft,
    KGDrawFlags_RoundCornersRight           = KGDrawFlags_RoundCornersBottomRight | KGDrawFlags_RoundCornersTopRight,
    KGDrawFlags_RoundCornersAll             = KGDrawFlags_RoundCornersTopLeft | KGDrawFlags_RoundCornersTopRight | KGDrawFlags_RoundCornersBottomLeft | KGDrawFlags_RoundCornersBottomRight,
    KGDrawFlags_RoundCornersDefault_        = KGDrawFlags_RoundCornersAll, // Default to ALL corners if none of the _RoundCornersXX flags are specified.
    KGDrawFlags_RoundCornersMask_           = KGDrawFlags_RoundCornersAll | KGDrawFlags_RoundCornersNone,
};

// Flags for KGDrawList instance. Those are set automatically by KarmaGui:: functions from KarmaGuiIO settings, and generally not manipulated directly.
// It is however possible to temporarily alter flags between calls to KGDrawList:: functions.
enum KGDrawListFlags_
{
    KGDrawListFlags_None                    = 0,
    KGDrawListFlags_AntiAliasedLines        = 1 << 0,  // Enable anti-aliased lines/borders (*2 the number of triangles for 1.0f wide line or lines thin enough to be drawn using textures, otherwise *3 the number of triangles)
    KGDrawListFlags_AntiAliasedLinesUseTex  = 1 << 1,  // Enable anti-aliased lines/borders using textures when possible. Require backend to render with bilinear filtering (NOT point/nearest filtering).
    KGDrawListFlags_AntiAliasedFill         = 1 << 2,  // Enable anti-aliased edge around filled shapes (rounded rectangles, circles).
    KGDrawListFlags_AllowVtxOffset          = 1 << 3,  // Can emit 'VtxOffset > 0' to allow large meshes. Set when 'KGGuiBackendFlags_RendererHasVtxOffset' is enabled.
};

// Draw command list
// This is the low-level list of polygons that KarmaGui:: functions are filling. At the end of the frame,
// all command lists are passed to your KarmaGuiIO::RenderDrawListFn function for rendering.
// Each dear imgui window contains its own KGDrawList. You can use ImGui::GetWindowDrawList() to
// access the current window draw list and draw custom primitives.
// You can interleave normal KarmaGui:: calls and adding primitives to the current draw list.
// In single viewport mode, top-left is == GetMainViewport()->Pos (generally 0,0), bottom-right is == GetMainViewport()->Pos+Size (generally io.DisplaySize).
// You are totally free to apply whatever transformation matrix to want to the data (depending on the use of the transformation you may want to apply it to ClipRect as well!)
// Important: Primitives are always added to the list and not culled (culling is done at higher-level by KarmaGui:: functions), if you use this API a lot consider coarse culling your drawn objects.
struct KARMA_API KGDrawList
{
    // This is what you have to render
    KGVector<KGDrawCmd>     CmdBuffer;          // Draw commands. Typically 1 command = 1 GPU draw call, unless the command is a callback.
    KGVector<KGDrawIdx>     IdxBuffer;          // Index buffer. Each command consume KGDrawCmd::ElemCount of those
    KGVector<KGDrawVert>    VtxBuffer;          // Vertex buffer.
    KGDrawListFlags         Flags;              // Flags, you may poke into these to adjust anti-aliasing settings per-primitive.

    // [Internal, used while building lists]
    unsigned int            _VtxCurrentIdx;     // [Internal] generally == VtxBuffer.Size unless we are past 64K vertices, in which case this gets reset to 0.
    KGDrawListSharedData*   _Data;              // Pointer to shared draw data (you can use ImGui::GetDrawListSharedData() to get the one from current ImGui context)
    const char*             _OwnerName;         // Pointer to owner window's name for debugging
    KGDrawVert*             _VtxWritePtr;       // [Internal] point within VtxBuffer.Data after each add command (to avoid using the KGVector<> operators too much)
    KGDrawIdx*              _IdxWritePtr;       // [Internal] point within IdxBuffer.Data after each add command (to avoid using the KGVector<> operators too much)
    KGVector<KGVec4>        _ClipRectStack;     // [Internal]
    KGVector<KGTextureID>   _TextureIdStack;    // [Internal]
    KGVector<KGVec2>        _Path;              // [Internal] current path building
    ImDrawCmdHeader         _CmdHeader;         // [Internal] template of active commands. Fields should match those of CmdBuffer.back().
    KGDrawListSplitter      _Splitter;          // [Internal] for channels api (note: prefer using your own persistent instance of KGDrawListSplitter!)
    float                   _FringeScale;       // [Internal] anti-alias fringe is scaled by this value, this helps to keep things sharp while zooming at vertex buffer content

    // If you want to create KGDrawList instances, pass them ImGui::GetDrawListSharedData() or create and use your own KGDrawListSharedData (so you can use KGDrawList without ImGui)
    KGDrawList(KGDrawListSharedData* shared_data) { memset(this, 0, sizeof(*this)); _Data = shared_data; }

    ~KGDrawList() { _ClearFreeMemory(); }
    static void  PushClipRect(const KGVec2& clip_rect_min, const KGVec2& clip_rect_max, bool intersect_with_current_clip_rect = false);  // Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level ImGui::PushClipRect() to affect logic (hit-testing and widget culling)
    static void  PushClipRectFullScreen();
    static void  PopClipRect();
    static void  PushTextureID(KGTextureID texture_id);
    static void  PopTextureID();
    inline KGVec2   GetClipRectMin() const { const KGVec4& cr = _ClipRectStack.back(); return KGVec2(cr.x, cr.y); }
    inline KGVec2   GetClipRectMax() const { const KGVec4& cr = _ClipRectStack.back(); return KGVec2(cr.z, cr.w); }

    // Primitives
    // - Filled shapes must always use clockwise winding order. The anti-aliasing fringe depends on it. Counter-clockwise shapes will have "inward" anti-aliasing.
    // - For rectangular primitives, "p_min" and "p_max" represent the upper-left and lower-right corners.
    // - For circle primitives, use "num_segments == 0" to automatically calculate tessellation (preferred).
    //   In older versions (until Dear ImGui 1.77) the AddCircle functions defaulted to num_segments == 12.
    //   In future versions we will use textures to provide cheaper and higher-quality circles.
    //   Use AddNgon() and AddNgonFilled() functions if you need to guarantee a specific number of sides.
    static void  AddLine(const KGVec2& p1, const KGVec2& p2, KGU32 col, float thickness = 1.0f);
    static void  AddRect(const KGVec2& p_min, const KGVec2& p_max, KGU32 col, float rounding = 0.0f, KGDrawFlags flags = 0, float thickness = 1.0f);   // a: upper-left, b: lower-right (== upper-left + size)
    static void  AddRectFilled(const KGVec2& p_min, const KGVec2& p_max, KGU32 col, float rounding = 0.0f, KGDrawFlags flags = 0);                     // a: upper-left, b: lower-right (== upper-left + size)
    static void  AddRectFilledMultiColor(const KGVec2& p_min, const KGVec2& p_max, KGU32 col_upr_left, KGU32 col_upr_right, KGU32 col_bot_right, KGU32 col_bot_left);
    static void  AddQuad(const KGVec2& p1, const KGVec2& p2, const KGVec2& p3, const KGVec2& p4, KGU32 col, float thickness = 1.0f);
    static void  AddQuadFilled(const KGVec2& p1, const KGVec2& p2, const KGVec2& p3, const KGVec2& p4, KGU32 col);
    static void  AddTriangle(const KGVec2& p1, const KGVec2& p2, const KGVec2& p3, KGU32 col, float thickness = 1.0f);
    static void  AddTriangleFilled(const KGVec2& p1, const KGVec2& p2, const KGVec2& p3, KGU32 col);
    static void  AddCircle(const KGVec2& center, float radius, KGU32 col, int num_segments = 0, float thickness = 1.0f);
    static void  AddCircleFilled(const KGVec2& center, float radius, KGU32 col, int num_segments = 0);
    static void  AddNgon(const KGVec2& center, float radius, KGU32 col, int num_segments, float thickness = 1.0f);
    static void  AddNgonFilled(const KGVec2& center, float radius, KGU32 col, int num_segments);
    static void  AddText(const KGVec2& pos, KGU32 col, const char* text_begin, const char* text_end = NULL);
    static void  AddText(const KGFont* font, float font_size, const KGVec2& pos, KGU32 col, const char* text_begin, const char* text_end = NULL, float wrap_width = 0.0f, const KGVec4* cpu_fine_clip_rect = NULL);
    static void  AddPolyline(const KGVec2* points, int num_points, KGU32 col, KGDrawFlags flags, float thickness);
    static void  AddConvexPolyFilled(const KGVec2* points, int num_points, KGU32 col);
    static void  AddBezierCubic(const KGVec2& p1, const KGVec2& p2, const KGVec2& p3, const KGVec2& p4, KGU32 col, float thickness, int num_segments = 0); // Cubic Bezier (4 control points)
    static void  AddBezierQuadratic(const KGVec2& p1, const KGVec2& p2, const KGVec2& p3, KGU32 col, float thickness, int num_segments = 0);               // Quadratic Bezier (3 control points)

    // Image primitives
    // - Read FAQ to understand what KGTextureID is.
    // - "p_min" and "p_max" represent the upper-left and lower-right corners of the rectangle.
    // - "uv_min" and "uv_max" represent the normalized texture coordinates to use for those corners. Using (0,0)->(1,1) texture coordinates will generally display the entire texture.
    static void  AddImage(KGTextureID user_texture_id, const KGVec2& p_min, const KGVec2& p_max, const KGVec2& uv_min = KGVec2(0, 0), const KGVec2& uv_max = KGVec2(1, 1), KGU32 col = KG_COL32_WHITE);
    static void  AddImageQuad(KGTextureID user_texture_id, const KGVec2& p1, const KGVec2& p2, const KGVec2& p3, const KGVec2& p4, const KGVec2& uv1 = KGVec2(0, 0), const KGVec2& uv2 = KGVec2(1, 0), const KGVec2& uv3 = KGVec2(1, 1), const KGVec2& uv4 = KGVec2(0, 1), KGU32 col = KG_COL32_WHITE);
    static void  AddImageRounded(KGTextureID user_texture_id, const KGVec2& p_min, const KGVec2& p_max, const KGVec2& uv_min, const KGVec2& uv_max, KGU32 col, float rounding, KGDrawFlags flags = 0);

    // Add custom background color to a window
    static void SetWindowBackgroundColor(KGVec4 bgColor);

    // Stateful path API, add points then finish with PathFillConvex() or PathStroke()
    // - Filled shapes must always use clockwise winding order. The anti-aliasing fringe depends on it. Counter-clockwise shapes will have "inward" anti-aliasing.
    inline    void  PathClear()                                                 { _Path.Size = 0; }
    inline    void  PathLineTo(const KGVec2& pos)                               { _Path.push_back(pos); }
    inline    void  PathLineToMergeDuplicate(const KGVec2& pos)                 { if (_Path.Size == 0 || memcmp(&_Path.Data[_Path.Size - 1], &pos, 8) != 0) _Path.push_back(pos); }
    inline    void  PathFillConvex(KGU32 col)                                   { AddConvexPolyFilled(_Path.Data, _Path.Size, col); _Path.Size = 0; }
    inline    void  PathStroke(KGU32 col, KGDrawFlags flags = 0, float thickness = 1.0f) { AddPolyline(_Path.Data, _Path.Size, col, flags, thickness); _Path.Size = 0; }
    static void  PathArcTo(const KGVec2& center, float radius, float a_min, float a_max, int num_segments = 0);
    static void  PathArcToFast(const KGVec2& center, float radius, int a_min_of_12, int a_max_of_12);                // Use precomputed angles for a 12 steps circle
    static void  PathBezierCubicCurveTo(const KGVec2& p2, const KGVec2& p3, const KGVec2& p4, int num_segments = 0); // Cubic Bezier (4 control points)
    static void  PathBezierQuadraticCurveTo(const KGVec2& p2, const KGVec2& p3, int num_segments = 0);               // Quadratic Bezier (3 control points)
    static void  PathRect(const KGVec2& rect_min, const KGVec2& rect_max, float rounding = 0.0f, KGDrawFlags flags = 0);

    // Advanced
    static void  AddCallback(KGDrawCallback callback, void* callback_data);  // Your rendering function must check for 'UserCallback' in KGDrawCmd and call the function instead of rendering triangles.
    static void  AddDrawCmd();                                               // This is useful if you need to forcefully create a new draw call (to allow for dependent rendering / blending). Otherwise primitives are merged into the same draw-call as much as possible
    static KGDrawList* CloneOutput();                                  // Create a clone of the CmdBuffer/IdxBuffer/VtxBuffer.

    // Advanced: Channels
    // - Use to split render into layers. By switching channels to can render out-of-order (e.g. submit FG primitives before BG primitives)
    // - Use to minimize draw calls (e.g. if going back-and-forth between multiple clipping rectangles, prefer to append into separate channels then merge at the end)
    // - FIXME-OBSOLETE: This API shouldn't have been in KGDrawList in the first place!
    //   Prefer using your own persistent instance of KGDrawListSplitter as you can stack them.
    //   Using the KGDrawList::ChannelsXXXX you cannot stack a split over another.
    inline void     ChannelsSplit(int count)    { _Splitter.Split(this, count); }
    inline void     ChannelsMerge()             { _Splitter.Merge(this); }
    inline void     ChannelsSetCurrent(int n)   { _Splitter.SetCurrentChannel(this, n); }

    // Advanced: Primitives allocations
    // - We render triangles (three vertices)
    // - All primitives needs to be reserved via PrimReserve() beforehand.
    static void  PrimReserve(int idx_count, int vtx_count);
    static void  PrimUnreserve(int idx_count, int vtx_count);
    static void  PrimRect(const KGVec2& a, const KGVec2& b, KGU32 col);      // Axis aligned rectangle (composed of two triangles)
    static void  PrimRectUV(const KGVec2& a, const KGVec2& b, const KGVec2& uv_a, const KGVec2& uv_b, KGU32 col);
    static void  PrimQuadUV(const KGVec2& a, const KGVec2& b, const KGVec2& c, const KGVec2& d, const KGVec2& uv_a, const KGVec2& uv_b, const KGVec2& uv_c, const KGVec2& uv_d, KGU32 col);
    inline    void  PrimWriteVtx(const KGVec2& pos, const KGVec2& uv, KGU32 col)    { _VtxWritePtr->pos = pos; _VtxWritePtr->uv = uv; _VtxWritePtr->col = col; _VtxWritePtr++; _VtxCurrentIdx++; }
    inline    void  PrimWriteIdx(KGDrawIdx idx)                                     { *_IdxWritePtr = idx; _IdxWritePtr++; }
    inline    void  PrimVtx(const KGVec2& pos, const KGVec2& uv, KGU32 col)         { PrimWriteIdx((KGDrawIdx)_VtxCurrentIdx); PrimWriteVtx(pos, uv, col); } // Write vertex with unique index

    // [Internal helpers]
    static void  _ResetForNewFrame();
    static void  _ClearFreeMemory();
    static void  _PopUnusedDrawCmd();
    static void  _TryMergeDrawCmds();
    static void  _OnChangedClipRect();
    static void  _OnChangedTextureID();
    static void  _OnChangedVtxOffset();
    static int   _CalcCircleAutoSegmentCount(float radius);
    static void  _PathArcToFastEx(const KGVec2& center, float radius, int a_min_sample, int a_max_sample, int a_step);
    static void  _PathArcToN(const KGVec2& center, float radius, float a_min, float a_max, int num_segments);
};

// All draw data to render a KarmaGui frame
// (NB: the style and the naming convention here is a little inconsistent, we currently preserve them for backward compatibility purpose,
// as this is one of the oldest structure exposed by the library! Basically, KGDrawList == CmdList)
struct KARMA_API KGDrawData
{
    bool            Valid;                  // Only valid after Render() is called and before the next NewFrame() is called.
    int             CmdListsCount;          // Number of KGDrawList* to render
    int             TotalIdxCount;          // For convenience, sum of all KGDrawList's IdxBuffer.Size
    int             TotalVtxCount;          // For convenience, sum of all KGDrawList's VtxBuffer.Size
    KGDrawList**    CmdLists;               // Array of KGDrawList* to render. The KGDrawList are owned by KarmaGuiContext and only pointed to from here.
    KGVec2          DisplayPos;             // Top-left position of the viewport to render (== top-left of the orthogonal projection matrix to use) (== GetMainViewport()->Pos for the main viewport, == (0.0) in most single-viewport applications)
    KGVec2          DisplaySize;            // Size of the viewport to render (== GetMainViewport()->Size for the main viewport, == io.DisplaySize in most single-viewport applications)
    KGVec2          FramebufferScale;       // Amount of pixels for each unit of DisplaySize. Based on io.DisplayFramebufferScale. Generally (1,1) on normal display, (2,2) on OSX with Retina display.
    KarmaGuiViewport*  OwnerViewport;          // Viewport carrying the KGDrawData instance, might be of use to the renderer (generally not).

    // Functions
    KGDrawData()    { Clear(); }
    void Clear()    { memset(this, 0, sizeof(*this)); }     // The KGDrawList are owned by KarmaGuiContext!
    static void  DeIndexAllBuffers();                    // Helper to convert all buffers from indexed to non-indexed, in case you cannot render indexed. Note: this is slow and most likely a waste of resources. Always prefer indexed rendering!
    static void  ScaleClipRects(const KGVec2& fb_scale); // Helper to scale the ClipRect field of each KGDrawCmd. Use if your final output buffer is at a different scale than Dear ImGui expects, or if there is a difference between your window resolution and framebuffer resolution.
};

//----------------------------------------------------------------------------------------------------------------
// [SECTION] Font API (KGFontConfig, KGFontGlyph, KGFontAtlasFlags, KGFontAtlas, KGFontGlyphRangesBuilder, KGFont)
//----------------------------------------------------------------------------------------------------------------

struct KARMA_API KGFontConfig
{
    void*           FontData;               //          // TTF/OTF data
    int             FontDataSize;           //          // TTF/OTF data size
    bool            FontDataOwnedByAtlas;   // true     // TTF/OTF data ownership taken by the container KGFontAtlas (will delete memory itself).
    int             FontNo;                 // 0        // Index of font within TTF/OTF file
    float           SizePixels;             //          // Size in pixels for rasterizer (more or less maps to the resulting font height).
    int             OversampleH;            // 3        // Rasterize at higher quality for sub-pixel positioning. Note the difference between 2 and 3 is minimal so you can reduce this to 2 to save memory. Read https://github.com/nothings/stb/blob/master/tests/oversample/README.md for details.
    int             OversampleV;            // 1        // Rasterize at higher quality for sub-pixel positioning. This is not really useful as we don't use sub-pixel positions on the Y axis.
    bool            PixelSnapH;             // false    // Align every glyph to pixel boundary. Useful e.g. if you are merging a non-pixel aligned font with the default font. If enabled, you can set OversampleH/V to 1.
    KGVec2          GlyphExtraSpacing;      // 0, 0     // Extra spacing (in pixels) between glyphs. Only X axis is supported for now.
    KGVec2          GlyphOffset;            // 0, 0     // Offset all glyphs from this font input.
    const KGWchar*  GlyphRanges;            // NULL     // Pointer to a user-provided list of Unicode range (2 value per range, values are inclusive, zero-terminated list). THE ARRAY DATA NEEDS TO PERSIST AS LONG AS THE FONT IS ALIVE.
    float           GlyphMinAdvanceX;       // 0        // Minimum AdvanceX for glyphs, set Min to align font icons, set both Min/Max to enforce mono-space font
    float           GlyphMaxAdvanceX;       // FLT_MAX  // Maximum AdvanceX for glyphs
    bool            MergeMode;              // false    // Merge into previous KGFont, so you can combine multiple inputs font into one KGFont (e.g. ASCII font + icons + Japanese glyphs). You may want to use GlyphOffset.y when merge font of different heights.
    unsigned int    FontBuilderFlags;       // 0        // Settings for custom font builder. THIS IS BUILDER IMPLEMENTATION DEPENDENT. Leave as zero if unsure.
    float           RasterizerMultiply;     // 1.0f     // Brighten (>1.0f) or darken (<1.0f) font output. Brightening small fonts may be a good workaround to make them more readable.
    KGWchar         EllipsisChar;           // -1       // Explicitly specify unicode codepoint of ellipsis character. When fonts are being merged first specified ellipsis will be used.

    // [Internal]
    char            Name[40];               // Name (strictly to ease debugging)
    KGFont*         DstFont;

    KGFontConfig();
};

// Hold rendering data for one glyph.
// (Note: some language parsers may fail to convert the 31+1 bitfield members, in this case maybe drop store a single u32 or we can rework this)
struct KARMA_API KGFontGlyph
{
    unsigned int    Colored : 1;        // Flag to indicate glyph is colored and should generally ignore tinting (make it usable with no shift on little-endian as this is used in loops)
    unsigned int    Visible : 1;        // Flag to indicate glyph has no visible pixels (e.g. space). Allow early out when rendering.
    unsigned int    Codepoint : 30;     // 0x0000..0x10FFFF
    float           AdvanceX;           // Distance to next character (= data from font + KGFontConfig::GlyphExtraSpacing.x baked in)
    float           X0, Y0, X1, Y1;     // Glyph corners
    float           U0, V0, U1, V1;     // Texture coordinates
};

// Helper to build glyph ranges from text/string data. Feed your application strings/characters to it then call BuildRanges().
// This is essentially a tightly packed of vector of 64k booleans = 8KB storage.
struct KARMA_API KGFontGlyphRangesBuilder
{
    KGVector<KGU32> UsedChars;            // Store 1-bit per Unicode code point (0=unused, 1=used)

    KGFontGlyphRangesBuilder()              { Clear(); }
    inline void     Clear()                 { int size_in_bytes = (KG_UNICODE_CODEPOINT_MAX + 1) / 8; UsedChars.resize(size_in_bytes / (int)sizeof(KGU32)); memset(UsedChars.Data, 0, (size_t)size_in_bytes); }
    inline bool     GetBit(size_t n) const  { int off = (int)(n >> 5); KGU32 mask = 1u << (n & 31); return (UsedChars[off] & mask) != 0; }  // Get bit n in the array
    inline void     SetBit(size_t n)        { int off = (int)(n >> 5); KGU32 mask = 1u << (n & 31); UsedChars[off] |= mask; }               // Set bit n in the array
    inline void     AddChar(KGWchar c)      { SetBit(c); }                      // Add character
    static void  AddText(const char* text, const char* text_end = NULL);     // Add string (each character of the UTF-8 string are added)
    static void  AddRanges(const KGWchar* ranges);                           // Add ranges, e.g. builder.AddRanges(KGFontAtlas::GetGlyphRangesDefault()) to force add all of ASCII/Latin+Ext
    static void  BuildRanges(KGVector<KGWchar>* out_ranges);                 // Output new ranges
};

// See KGFontAtlas::AddCustomRectXXX functions.
struct KARMA_API KGFontAtlasCustomRect
{
    unsigned short  Width, Height;  // Input    // Desired rectangle dimension
    unsigned short  X, Y;           // Output   // Packed position in Atlas
    unsigned int    GlyphID;        // Input    // For custom font glyphs only (ID < 0x110000)
    float           GlyphAdvanceX;  // Input    // For custom font glyphs only: glyph xadvance
    KGVec2          GlyphOffset;    // Input    // For custom font glyphs only: glyph display offset
    KGFont*         Font;           // Input    // For custom font glyphs only: target font
    KGFontAtlasCustomRect()         { Width = Height = 0; X = Y = 0xFFFF; GlyphID = 0; GlyphAdvanceX = 0.0f; GlyphOffset = KGVec2(0, 0); Font = NULL; }
    bool IsPacked() const           { return X != 0xFFFF; }
};

// Flags for KGFontAtlas build
enum KGFontAtlasFlags_
{
    KGFontAtlasFlags_None               = 0,
    KGFontAtlasFlags_NoPowerOfTwoHeight = 1 << 0,   // Don't round the height to next power of two
    KGFontAtlasFlags_NoMouseCursors     = 1 << 1,   // Don't build software mouse cursors into the atlas (save a little texture memory)
    KGFontAtlasFlags_NoBakedLines       = 1 << 2,   // Don't build thick line textures into the atlas (save a little texture memory, allow support for point/nearest filtering). The AntiAliasedLinesUseTex features uses them, otherwise they will be rendered using polygons (more expensive for CPU/GPU).
};

// Load and rasterize multiple TTF/OTF fonts into a same texture. The font atlas will build a single texture holding:
//  - One or more fonts.
//  - Custom graphics data needed to render the shapes needed by Dear ImGui.
//  - Mouse cursor shapes for software cursor rendering (unless setting 'Flags |= KGFontAtlasFlags_NoMouseCursors' in the font atlas).
// It is the user-code responsibility to setup/build the atlas, then upload the pixel data into a texture accessible by your graphics api.
//  - Optionally, call any of the AddFont*** functions. If you don't call any, the default font embedded in the code will be loaded for you.
//  - Call GetTexDataAsAlpha8() or GetTexDataAsRGBA32() to build and retrieve pixels data.
//  - Upload the pixels data into a texture within your graphics system (see imgui_impl_xxxx.cpp examples)
//  - Call SetTexID(my_tex_id); and pass the pointer/identifier to your texture in a format natural to your graphics API.
//    This value will be passed back to you during rendering to identify the texture. Read FAQ entry about KGTextureID for more details.
// Common pitfalls:
// - If you pass a 'glyph_ranges' array to AddFont*** functions, you need to make sure that your array persist up until the
//   atlas is build (when calling GetTexData*** or Build()). We only copy the pointer, not the data.
// - Important: By default, AddFontFromMemoryTTF() takes ownership of the data. Even though we are not writing to it, we will free the pointer on destruction.
//   You can set font_cfg->FontDataOwnedByAtlas=false to keep ownership of your data and it won't be freed,
// - Even though many functions are suffixed with "TTF", OTF data is supported just as well.
// - This is an old API and it is currently awkward for those and various other reasons! We will address them in the future!
struct KARMA_API KGFontAtlas
{
    KGFontAtlas();
    ~KGFontAtlas();
    static KGFont*           AddFont(const KGFontConfig* font_cfg);
    static KGFont*           AddFontDefault(const KGFontConfig* font_cfg = NULL);
    static KGFont*           AddFontFromFileTTF(const char* filename, float size_pixels, const KGFontConfig* font_cfg = NULL, const KGWchar* glyph_ranges = NULL);
    static KGFont*           AddFontFromMemoryTTF(void* font_data, int font_size, float size_pixels, const KGFontConfig* font_cfg = NULL, const KGWchar* glyph_ranges = NULL); // Note: Transfer ownership of 'ttf_data' to KGFontAtlas! Will be deleted after destruction of the atlas. Set font_cfg->FontDataOwnedByAtlas=false to keep ownership of your data and it won't be freed.
    static KGFont*           AddFontFromMemoryCompressedTTF(const void* compressed_font_data, int compressed_font_size, float size_pixels, const KGFontConfig* font_cfg = NULL, const KGWchar* glyph_ranges = NULL); // 'compressed_font_data' still owned by caller. Compress with binary_to_compressed_c.cpp.
    static KGFont*           AddFontFromMemoryCompressedBase85TTF(const char* compressed_font_data_base85, float size_pixels, const KGFontConfig* font_cfg = NULL, const KGWchar* glyph_ranges = NULL);              // 'compressed_font_data_base85' still owned by caller. Compress with binary_to_compressed_c.cpp with -base85 parameter.
    static void              ClearInputData();           // Clear input data (all KGFontConfig structures including sizes, TTF data, glyph ranges, etc.) = all the data used to build the texture and fonts.
    static void              ClearTexData();             // Clear output texture data (CPU side). Saves RAM once the texture has been copied to graphics memory.
    static void              ClearFonts();               // Clear output font data (glyphs storage, UV coordinates).
    static void              Clear();                    // Clear all input and output.

    // Build atlas, retrieve pixel data.
    // User is in charge of copying the pixels into graphics memory (e.g. create a texture with your engine). Then store your texture handle with SetTexID().
    // The pitch is always = Width * BytesPerPixels (1 or 4)
    // Building in RGBA32 format is provided for convenience and compatibility, but note that unless you manually manipulate or copy color data into
    // the texture (e.g. when using the AddCustomRect*** api), then the RGB pixels emitted will always be white (~75% of memory/bandwidth waste.
    static bool              Build();                    // Build pixels data. This is called automatically for you by the GetTexData*** functions.
    static void              GetTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel = NULL);  // 1 byte per-pixel
    static void              GetTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel = NULL);  // 4 bytes-per-pixel
    bool                        IsBuilt() const             { return Fonts.Size > 0 && TexReady; } // Bit ambiguous: used to detect when user didn't build texture but effectively we should check TexID != 0 except that would be backend dependent...
    void                        SetTexID(KGTextureID id)    { TexID = id; }

    //-------------------------------------------
    // Glyph Ranges
    //-------------------------------------------

    // Helpers to retrieve list of common Unicode ranges (2 value per range, values are inclusive, zero-terminated list)
    // NB: Make sure that your string are UTF-8 and NOT in your local code page. In C++11, you can create UTF-8 string literal using the u8"Hello world" syntax. See FAQ for details.
    // NB: Consider using KGFontGlyphRangesBuilder to build glyph ranges from textual data.
    static const KGWchar*    GetGlyphRangesDefault();                // Basic Latin, Extended Latin
    static const KGWchar*    GetGlyphRangesGreek();                  // Default + Greek and Coptic
    static const KGWchar*    GetGlyphRangesKorean();                 // Default + Korean characters
    static const KGWchar*    GetGlyphRangesJapanese();               // Default + Hiragana, Katakana, Half-Width, Selection of 2999 Ideographs
    static const KGWchar*    GetGlyphRangesChineseFull();            // Default + Half-Width + Japanese Hiragana/Katakana + full set of about 21000 CJK Unified Ideographs
    static const KGWchar*    GetGlyphRangesChineseSimplifiedCommon();// Default + Half-Width + Japanese Hiragana/Katakana + set of 2500 CJK Unified Ideographs for common simplified Chinese
    static const KGWchar*    GetGlyphRangesCyrillic();               // Default + about 400 Cyrillic characters
    static const KGWchar*    GetGlyphRangesThai();                   // Default + Thai characters
    static const KGWchar*    GetGlyphRangesVietnamese();             // Default + Vietnamese characters

    //-------------------------------------------
    // [BETA] Custom Rectangles/Glyphs API
    //-------------------------------------------

    // You can request arbitrary rectangles to be packed into the atlas, for your own purposes.
    // - After calling Build(), you can query the rectangle position and render your pixels.
    // - If you render colored output, set 'atlas->TexPixelsUseColors = true' as this may help some backends decide of prefered texture format.
    // - You can also request your rectangles to be mapped as font glyph (given a font + Unicode point),
    //   so you can render e.g. custom colorful icons and use them as regular glyphs.
    // - Read docs/FONTS.md for more details about using colorful icons.
    // - Note: this API may be redesigned later in order to support multi-monitor varying DPI settings.
    static int               AddCustomRectRegular(int width, int height);
    static int               AddCustomRectFontGlyph(KGFont* font, KGWchar id, int width, int height, float advance_x, const KGVec2& offset = KGVec2(0, 0));
    KGFontAtlasCustomRect*      GetCustomRectByIndex(int index) { KR_CORE_ASSERT(index >= 0, ""); return &CustomRects[index]; }

    // [Internal]
    static void              CalcCustomRectUV(const KGFontAtlasCustomRect* rect, KGVec2* out_uv_min, KGVec2* out_uv_max);
    static bool              GetMouseCursorTexData(KarmaGuiMouseCursor cursor, KGVec2* out_offset, KGVec2* out_size, KGVec2 out_uv_border[2], KGVec2 out_uv_fill[2]);

    //-------------------------------------------
    // Members
    //-------------------------------------------

    KGFontAtlasFlags            Flags;              // Build flags (see KGFontAtlasFlags_)
    KGTextureID                 TexID;              // User data to refer to the texture once it has been uploaded to user's graphic systems. It is passed back to you during rendering via the KGDrawCmd structure.
    int                         TexDesiredWidth;    // Texture width desired by user before Build(). Must be a power-of-two. If have many glyphs your graphics API have texture size restrictions you may want to increase texture width to decrease height.
    int                         TexGlyphPadding;    // Padding between glyphs within texture in pixels. Defaults to 1. If your rendering method doesn't rely on bilinear filtering you may set this to 0 (will also need to set AntiAliasedLinesUseTex = false).
    bool                        Locked;             // Marked as Locked by ImGui::NewFrame() so attempt to modify the atlas will assert.
    void*                       UserData;           // Store your own atlas related user-data (if e.g. you have multiple font atlas).

    // [Internal]
    // NB: Access texture data via GetTexData*() calls! Which will setup a default font for you.
    bool                        TexReady;           // Set when texture was built matching current font input
    bool                        TexPixelsUseColors; // Tell whether our texture data is known to use colors (rather than just alpha channel), in order to help backend select a format.
    unsigned char*              TexPixelsAlpha8;    // 1 component per pixel, each component is unsigned 8-bit. Total size = TexWidth * TexHeight
    unsigned int*               TexPixelsRGBA32;    // 4 component per pixel, each component is unsigned 8-bit. Total size = TexWidth * TexHeight * 4
    int                         TexWidth;           // Texture width calculated during Build().
    int                         TexHeight;          // Texture height calculated during Build().
    KGVec2                      TexUvScale;         // = (1.0f/TexWidth, 1.0f/TexHeight)
    KGVec2                      TexUvWhitePixel;    // Texture coordinates to a white pixel
    KGVector<KGFont*>           Fonts;              // Hold all the fonts returned by AddFont*. Fonts[0] is the default font upon calling ImGui::NewFrame(), use ImGui::PushFont()/PopFont() to change the current font.
    KGVector<KGFontAtlasCustomRect> CustomRects;    // Rectangles for packing custom texture data into the atlas.
    KGVector<KGFontConfig>      ConfigData;         // Configuration data
    KGVec4                      TexUvLines[KG_DRAWLIST_TEX_LINES_WIDTH_MAX + 1];  // UVs for baked anti-aliased lines

    // [Internal] Font builder
    const KGFontBuilderIO*      FontBuilderIO;      // Opaque interface to a font builder (default to stb_truetype, can be changed to use FreeType by defining KGGUI_ENABLE_FREETYPE).
    unsigned int                FontBuilderFlags;   // Shared flags (for all fonts) for custom font builder. THIS IS BUILD IMPLEMENTATION DEPENDENT. Per-font override is also available in KGFontConfig.

    // [Internal] Packing data
    int                         PackIdMouseCursors; // Custom texture rectangle ID for white pixel and mouse cursors
    int                         PackIdLines;        // Custom texture rectangle ID for baked anti-aliased lines

    // [Obsolete]
    //typedef KGFontAtlasCustomRect    CustomRect;         // OBSOLETED in 1.72+
    //typedef KGFontGlyphRangesBuilder GlyphRangesBuilder; // OBSOLETED in 1.67+
};

// Font runtime data and rendering
// KGFontAtlas automatically loads a default embedded font for you when you call GetTexDataAsAlpha8() or GetTexDataAsRGBA32().
struct KARMA_API KGFont
{
    // Members: Hot ~20/24 bytes (for CalcTextSize)
    KGVector<float>             IndexAdvanceX;      // 12-16 // out //            // Sparse. Glyphs->AdvanceX in a directly indexable way (cache-friendly for CalcTextSize functions which only this this info, and are often bottleneck in large UI).
    float                       FallbackAdvanceX;   // 4     // out // = FallbackGlyph->AdvanceX
    float                       FontSize;           // 4     // in  //            // Height of characters/line, set during loading (don't change after loading)

    // Members: Hot ~28/40 bytes (for CalcTextSize + render loop)
    KGVector<KGWchar>           IndexLookup;        // 12-16 // out //            // Sparse. Index glyphs by Unicode code-point.
    KGVector<KGFontGlyph>       Glyphs;             // 12-16 // out //            // All glyphs.
    const KGFontGlyph*          FallbackGlyph;      // 4-8   // out // = FindGlyph(FontFallbackChar)

    // Members: Cold ~32/40 bytes
    KGFontAtlas*                ContainerAtlas;     // 4-8   // out //            // What we has been loaded into
    const KGFontConfig*         ConfigData;         // 4-8   // in  //            // Pointer within ContainerAtlas->ConfigData
    short                       ConfigDataCount;    // 2     // in  // ~ 1        // Number of KGFontConfig involved in creating this font. Bigger than 1 when merging multiple font sources into one KGFont.
    KGWchar                     FallbackChar;       // 2     // out // = FFFD/'?' // Character used if a glyph isn't found.
    KGWchar                     EllipsisChar;       // 2     // out // = '...'    // Character used for ellipsis rendering.
    KGWchar                     DotChar;            // 2     // out // = '.'      // Character used for ellipsis rendering (if a single '...' character isn't found)
    bool                        DirtyLookupTables;  // 1     // out //
    float                       Scale;              // 4     // in  // = 1.f      // Base font scale, multiplied by the per-window font scale which you can adjust with SetWindowFontScale()
    float                       Ascent, Descent;    // 4+4   // out //            // Ascent: distance from top to bottom of e.g. 'A' [0..FontSize]
    int                         MetricsTotalSurface;// 4     // out //            // Total surface in pixels to get an idea of the font rasterization/texture cost (not exact, we approximate the cost of padding between glyphs)
    KGU8                        Used4kPagesMap[(KG_UNICODE_CODEPOINT_MAX+1)/4096/8]; // 2 bytes if KGWchar=KGWchar16, 34 bytes if KGWchar==KGWchar32. Store 1-bit for each block of 4K codepoints that has one active glyph. This is mainly used to facilitate iterations across all used codepoints.

    // Methods
    KGFont();
    ~KGFont();
    static const KGFontGlyph*FindGlyph(KGWchar c);
    static const KGFontGlyph*FindGlyphNoFallback(KGWchar c);
    float                       GetCharAdvance(KGWchar c) const     { return ((int)c < IndexAdvanceX.Size) ? IndexAdvanceX[(int)c] : FallbackAdvanceX; }
    bool                        IsLoaded() const                    { return ContainerAtlas != NULL; }
    const char*                 GetDebugName() const                { return ConfigData ? ConfigData->Name : "<unknown>"; }

    // 'max_width' stops rendering after a certain width (could be turned into a 2d size). FLT_MAX to disable.
    // 'wrap_width' enable automatic word-wrapping across multiple lines to fit into given width. 0.0f to disable.
    static KGVec2            CalcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end = NULL, const char** remaining = NULL); // utf8
    static const char*       CalcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width);
    static void              RenderChar(KGDrawList* draw_list, float size, const KGVec2& pos, KGU32 col, KGWchar c);
    static void              RenderText(KGDrawList* draw_list, float size, const KGVec2& pos, KGU32 col, const KGVec4& clip_rect, const char* text_begin, const char* text_end, float wrap_width = 0.0f, bool cpu_fine_clip = false);

    // [Internal] Don't use!
    static void              BuildLookupTable();
    static void              ClearOutputData();
    static void              GrowIndex(int new_size);
    static void              AddGlyph(const KGFontConfig* src_cfg, KGWchar c, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float advance_x);
    static void              AddRemapChar(KGWchar dst, KGWchar src, bool overwrite_dst = true); // Makes 'dst' character/glyph points to 'src' character/glyph. Currently needs to be called AFTER fonts have been built.
    static void              SetGlyphVisible(KGWchar c, bool visible);
    static bool              IsGlyphRangeUnused(unsigned int c_begin, unsigned int c_last);
};

//-----------------------------------------------------------------------------
// [SECTION] Viewports
//-----------------------------------------------------------------------------

// Flags stored in KarmaGuiViewport::Flags, giving indications to the platform backends.
enum KGGuiViewportFlags_
{
    KGGuiViewportFlags_None                     = 0,
    KGGuiViewportFlags_IsPlatformWindow         = 1 << 0,   // Represent a Platform Window
    KGGuiViewportFlags_IsPlatformMonitor        = 1 << 1,   // Represent a Platform Monitor (unused yet)
    KGGuiViewportFlags_OwnedByApp               = 1 << 2,   // Platform Window: is created/managed by the application (rather than a dear imgui backend)
    KGGuiViewportFlags_NoDecoration             = 1 << 3,   // Platform Window: Disable platform decorations: title bar, borders, etc. (generally set all windows, but if KGGuiConfigFlags_ViewportsDecoration is set we only set this on popups/tooltips)
    KGGuiViewportFlags_NoTaskBarIcon            = 1 << 4,   // Platform Window: Disable platform task bar icon (generally set on popups/tooltips, or all windows if KGGuiConfigFlags_ViewportsNoTaskBarIcon is set)
    KGGuiViewportFlags_NoFocusOnAppearing       = 1 << 5,   // Platform Window: Don't take focus when created.
    KGGuiViewportFlags_NoFocusOnClick           = 1 << 6,   // Platform Window: Don't take focus when clicked on.
    KGGuiViewportFlags_NoInputs                 = 1 << 7,   // Platform Window: Make mouse pass through so we can drag this window while peaking behind it.
    KGGuiViewportFlags_NoRendererClear          = 1 << 8,   // Platform Window: Renderer doesn't need to clear the framebuffer ahead (because we will fill it entirely).
    KGGuiViewportFlags_TopMost                  = 1 << 9,   // Platform Window: Display on top (for tooltips only).
    KGGuiViewportFlags_Minimized                = 1 << 10,  // Platform Window: Window is minimized, can skip render. When minimized we tend to avoid using the viewport pos/size for clipping window or testing if they are contained in the viewport.
    KGGuiViewportFlags_NoAutoMerge              = 1 << 11,  // Platform Window: Avoid merging this window into another host window. This can only be set via KarmaGuiWindowClass viewport flags override (because we need to now ahead if we are going to create a viewport in the first place!).
    KGGuiViewportFlags_CanHostOtherWindows      = 1 << 12,  // Main viewport: can host multiple imgui windows (secondary viewports are associated to a single window).
};

// - Currently represents the Platform Window created by the application which is hosting our Dear ImGui windows.
// - With multi-viewport enabled, we extend this concept to have multiple active viewports.
// - In the future we will extend this concept further to also represent Platform Monitor and support a "no main platform window" operation mode.
// - About Main Area vs Work Area:
//   - Main Area = entire viewport.
//   - Work Area = entire viewport minus sections used by main menu bars (for platform windows), or by task bar (for platform monitor).
//   - Windows are generally trying to stay within the Work Area of their host viewport.
struct KARMA_API KarmaGuiViewport
{
    KGGuiID             ID;                     // Unique identifier for the viewport
    KarmaGuiViewportFlags  Flags;                  // See KGGuiViewportFlags_
    KGVec2              Pos;                    // Main Area: Position of the viewport (Dear ImGui coordinates are the same as OS desktop/native coordinates)
    KGVec2              Size;                   // Main Area: Size of the viewport.
    KGVec2              WorkPos;                // Work Area: Position of the viewport minus task bars, menus bars, status bars (>= Pos)
    KGVec2              WorkSize;               // Work Area: Size of the viewport minus task bars, menu bars, status bars (<= Size)
    float               DpiScale;               // 1.0f = 96 DPI = No extra scale.
    KGGuiID             ParentViewportId;       // (Advanced) 0: no parent. Instruct the platform backend to setup a parent/child relationship between platform windows.
    KGDrawData*         DrawData;               // The KGDrawData corresponding to this viewport. Valid after Render() and until the next call to NewFrame().

    // Platform/Backend Dependent Data
    // Our design separate the Renderer and Platform backends to facilitate combining default backends with each others.
    // When our create your own backend for a custom engine, it is possible that both Renderer and Platform will be handled
    // by the same system and you may not need to use all the UserData/Handle fields.
    // The library never uses those fields, they are merely storage to facilitate backend implementation.
    void*               RendererUserData;       // void* to hold custom data structure for the renderer (e.g. swap chain, framebuffers etc.). generally set by your Renderer_CreateWindow function.
    void*               PlatformUserData;       // void* to hold custom data structure for the OS / platform (e.g. windowing info, render context). generally set by your Platform_CreateWindow function.
    void*               PlatformHandle;         // void* for FindViewportByPlatformHandle(). (e.g. suggested to use natural platform handle such as HWND, GLFWWindow*, SDL_Window*)
    void*               PlatformHandleRaw;      // void* to hold lower-level, platform-native window handle (under Win32 this is expected to be a HWND, unused for other platforms), when using an abstraction layer like GLFW or SDL (where PlatformHandle would be a SDL_Window*)
    bool                PlatformWindowCreated;  // Platform window has been created (Platform_CreateWindow() has been called). This is false during the first frame where a viewport is being created.
    bool                PlatformRequestMove;    // Platform window requested move (e.g. window was moved by the OS / host window manager, authoritative position will be OS window position)
    bool                PlatformRequestResize;  // Platform window requested resize (e.g. window was resized by the OS / host window manager, authoritative size will be OS window size)
    bool                PlatformRequestClose;   // Platform window requested closure (e.g. window was moved by the OS / host window manager, e.g. pressing ALT-F4)

    KarmaGuiViewport()     { memset(this, 0, sizeof(*this)); }
    ~KarmaGuiViewport()    { KR_CORE_ASSERT(PlatformUserData == NULL && RendererUserData == NULL, ""); }

    // Helpers
    KGVec2              GetCenter() const       { return KGVec2(Pos.x + Size.x * 0.5f, Pos.y + Size.y * 0.5f); }
    KGVec2              GetWorkCenter() const   { return KGVec2(WorkPos.x + WorkSize.x * 0.5f, WorkPos.y + WorkSize.y * 0.5f); }
};

//-----------------------------------------------------------------------------
// [SECTION] Platform Dependent Interfaces (for e.g. multi-viewport support)
//-----------------------------------------------------------------------------
// [BETA] (Optional) This is completely optional, for advanced users!
// If you are new to Dear ImGui and trying to integrate it into your engine, you can probably ignore this for now.
//
// This feature allows you to seamlessly drag Dear ImGui windows outside of your application viewport.
// This is achieved by creating new Platform/OS windows on the fly, and rendering into them.
// Dear ImGui manages the viewport structures, and the backend create and maintain one Platform/OS window for each of those viewports.
//
// See Glossary https://github.com/ocornut/imgui/wiki/Glossary for details about some of the terminology.
// See Thread https://github.com/ocornut/imgui/issues/1542 for gifs, news and questions about this evolving feature.
//
// About the coordinates system:
// - When multi-viewports are enabled, all Dear ImGui coordinates become absolute coordinates (same as OS coordinates!)
// - So e.g. ImGui::SetNextWindowPos(KGVec2(0,0)) will position a window relative to your primary monitor!
// - If you want to position windows relative to your main application viewport, use ImGui::GetMainViewport()->Pos as a base position.
//
// Steps to use multi-viewports in your application, when using a default backend from the examples/ folder:
// - Application:  Enable feature with 'io.ConfigFlags |= KGGuiConfigFlags_ViewportsEnable'.
// - Backend:      The backend initialization will setup all necessary KarmaGuiPlatformIO's functions and update monitors info every frame.
// - Application:  In your main loop, call ImGui::UpdatePlatformWindows(), ImGui::RenderPlatformWindowsDefault() after EndFrame() or Render().
// - Application:  Fix absolute coordinates used in ImGui::SetWindowPos() or ImGui::SetNextWindowPos() calls.
//
// Steps to use multi-viewports in your application, when using a custom backend:
// - Important:    THIS IS NOT EASY TO DO and comes with many subtleties not described here!
//                 It's also an experimental feature, so some of the requirements may evolve.
//                 Consider using default backends if you can. Either way, carefully follow and refer to examples/ backends for details.
// - Application:  Enable feature with 'io.ConfigFlags |= KGGuiConfigFlags_ViewportsEnable'.
// - Backend:      Hook KarmaGuiPlatformIO's Platform_* and Renderer_* callbacks (see below).
//                 Set 'io.BackendFlags |= KGGuiBackendFlags_PlatformHasViewports' and 'io.BackendFlags |= KGGuiBackendFlags_PlatformHasViewports'.
//                 Update KarmaGuiPlatformIO's Monitors list every frame.
//                 Update MousePos every frame, in absolute coordinates.
// - Application:  In your main loop, call ImGui::UpdatePlatformWindows(), ImGui::RenderPlatformWindowsDefault() after EndFrame() or Render().
//                 You may skip calling RenderPlatformWindowsDefault() if its API is not convenient for your needs. Read comments below.
// - Application:  Fix absolute coordinates used in ImGui::SetWindowPos() or ImGui::SetNextWindowPos() calls.
//
// About ImGui::RenderPlatformWindowsDefault():
// - This function is a mostly a _helper_ for the common-most cases, and to facilitate using default backends.
// - You can check its simple source code to understand what it does.
//   It basically iterates secondary viewports and call 4 functions that are setup in KarmaGuiPlatformIO, if available:
//     Platform_RenderWindow(), Renderer_RenderWindow(), Platform_SwapBuffers(), Renderer_SwapBuffers()
//   Those functions pointers exists only for the benefit of RenderPlatformWindowsDefault().
// - If you have very specific rendering needs (e.g. flipping multiple swap-chain simultaneously, unusual sync/threading issues, etc.),
//   you may be tempted to ignore RenderPlatformWindowsDefault() and write customized code to perform your renderingg.
//   You may decide to setup the platform_io's *RenderWindow and *SwapBuffers pointers and call your functions through those pointers,
//   or you may decide to never setup those pointers and call your code directly. They are a convenience, not an obligatory interface.
//-----------------------------------------------------------------------------

// (Optional) Access via ImGui::GetPlatformIO()
struct KARMA_API KarmaGuiPlatformIO
{
    //------------------------------------------------------------------
    // Input - Backend interface/functions + Monitor List
    //------------------------------------------------------------------

    // (Optional) Platform functions (e.g. Win32, GLFW, SDL2)
    // For reference, the second column shows which function are generally calling the Platform Functions:
    //   N = ImGui::NewFrame()                        ~ beginning of the dear imgui frame: read info from platform/OS windows (latest size/position)
    //   F = ImGui::Begin(), ImGui::EndFrame()        ~ during the dear imgui frame
    //   U = ImGui::UpdatePlatformWindows()           ~ after the dear imgui frame: create and update all platform/OS windows
    //   R = ImGui::RenderPlatformWindowsDefault()    ~ render
    //   D = ImGui::DestroyPlatformWindows()          ~ shutdown
    // The general idea is that NewFrame() we will read the current Platform/OS state, and UpdatePlatformWindows() will write to it.
    //
    // The functions are designed so we can mix and match 2 imgui_impl_xxxx files, one for the Platform (~window/input handling), one for Renderer.
    // Custom engine backends will often provide both Platform and Renderer interfaces and so may not need to use all functions.
    // Platform functions are typically called before their Renderer counterpart, apart from Destroy which are called the other way.

    // Platform function --------------------------------------------------- Called by -----
    void    (*Platform_CreateWindow)(KarmaGuiViewport* vp);                    // . . U . .  // Create a new platform window for the given viewport
    void    (*Platform_DestroyWindow)(KarmaGuiViewport* vp);                   // N . U . D  //
    void    (*Platform_ShowWindow)(KarmaGuiViewport* vp);                      // . . U . .  // Newly created windows are initially hidden so SetWindowPos/Size/Title can be called on them before showing the window
    void    (*Platform_SetWindowPos)(KarmaGuiViewport* vp, KGVec2 pos);        // . . U . .  // Set platform window position (given the upper-left corner of client area)
    KGVec2  (*Platform_GetWindowPos)(KarmaGuiViewport* vp);                    // N . . . .  //
    void    (*Platform_SetWindowSize)(KarmaGuiViewport* vp, KGVec2 size);      // . . U . .  // Set platform window client area size (ignoring OS decorations such as OS title bar etc.)
    KGVec2  (*Platform_GetWindowSize)(KarmaGuiViewport* vp);                   // N . . . .  // Get platform window client area size
    void    (*Platform_SetWindowFocus)(KarmaGuiViewport* vp);                  // N . . . .  // Move window to front and set input focus
    bool    (*Platform_GetWindowFocus)(KarmaGuiViewport* vp);                  // . . U . .  //
    bool    (*Platform_GetWindowMinimized)(KarmaGuiViewport* vp);              // N . . . .  // Get platform window minimized state. When minimized, we generally won't attempt to get/set size and contents will be culled more easily
    void    (*Platform_SetWindowTitle)(KarmaGuiViewport* vp, const char* str); // . . U . .  // Set platform window title (given an UTF-8 string)
    void    (*Platform_SetWindowAlpha)(KarmaGuiViewport* vp, float alpha);     // . . U . .  // (Optional) Setup global transparency (not per-pixel transparency)
    void    (*Platform_UpdateWindow)(KarmaGuiViewport* vp);                    // . . U . .  // (Optional) Called by UpdatePlatformWindows(). Optional hook to allow the platform backend from doing general book-keeping every frame.
    void    (*Platform_RenderWindow)(KarmaGuiViewport* vp, void* render_arg);  // . . . R .  // (Optional) Main rendering (platform side! This is often unused, or just setting a "current" context for OpenGL bindings). 'render_arg' is the value passed to RenderPlatformWindowsDefault().
    void    (*Platform_SwapBuffers)(KarmaGuiViewport* vp, void* render_arg);   // . . . R .  // (Optional) Call Present/SwapBuffers (platform side! This is often unused!). 'render_arg' is the value passed to RenderPlatformWindowsDefault().
    float   (*Platform_GetWindowDpiScale)(KarmaGuiViewport* vp);               // N . . . .  // (Optional) [BETA] FIXME-DPI: DPI handling: Return DPI scale for this viewport. 1.0f = 96 DPI.
    void    (*Platform_OnChangedViewport)(KarmaGuiViewport* vp);               // . F . . .  // (Optional) [BETA] FIXME-DPI: DPI handling: Called during Begin() every time the viewport we are outputting into changes, so backend has a chance to swap fonts to adjust style.
    int     (*Platform_CreateVkSurface)(KarmaGuiViewport* vp, KGU64 vk_inst, const void* vk_allocators, KGU64* out_vk_surface); // (Optional) For a Vulkan Renderer to call into Platform code (since the surface creation needs to tie them both).

    // (Optional) Renderer functions (e.g. DirectX, OpenGL, Vulkan)
    void    (*Renderer_CreateWindow)(KarmaGuiViewport* vp);                    // . . U . .  // Create swap chain, frame buffers etc. (called after Platform_CreateWindow)
    void    (*Renderer_DestroyWindow)(KarmaGuiViewport* vp);                   // N . U . D  // Destroy swap chain, frame buffers etc. (called before Platform_DestroyWindow)
    void    (*Renderer_SetWindowSize)(KarmaGuiViewport* vp, KGVec2 size);      // . . U . .  // Resize swap chain, frame buffers etc. (called after Platform_SetWindowSize)
    void    (*Renderer_RenderWindow)(KarmaGuiViewport* vp, void* render_arg);  // . . . R .  // (Optional) Clear framebuffer, setup render target, then render the viewport->DrawData. 'render_arg' is the value passed to RenderPlatformWindowsDefault().
    void    (*Renderer_SwapBuffers)(KarmaGuiViewport* vp, void* render_arg);   // . . . R .  // (Optional) Call Present/SwapBuffers. 'render_arg' is the value passed to RenderPlatformWindowsDefault().

    // (Optional) Monitor list
    // - Updated by: app/backend. Update every frame to dynamically support changing monitor or DPI configuration.
    // - Used by: dear imgui to query DPI info, clamp popups/tooltips within same monitor and not have them straddle monitors.
    KGVector<KarmaGuiPlatformMonitor>  Monitors;

    //------------------------------------------------------------------
    // Output - List of viewports to render into platform windows
    //------------------------------------------------------------------

    // Viewports list (the list is updated by calling ImGui::EndFrame or ImGui::Render)
    // (in the future we will attempt to organize this feature to remove the need for a "main viewport")
    KGVector<KarmaGuiViewport*>        Viewports;                              // Main viewports, followed by all secondary viewports.
    KarmaGuiPlatformIO()               { memset(this, 0, sizeof(*this)); }     // Zero clear
};

// (Optional) This is required when enabling multi-viewport. Represent the bounds of each connected monitor/display and their DPI.
// We use this information for multiple DPI support + clamping the position of popups and tooltips so they don't straddle multiple monitors.
struct KARMA_API KarmaGuiPlatformMonitor
{
    KGVec2  MainPos, MainSize;      // Coordinates of the area displayed on this monitor (Min = upper left, Max = bottom right)
    KGVec2  WorkPos, WorkSize;      // Coordinates without task bars / side bars / menu bars. Used to avoid positioning popups/tooltips inside this region. If you don't have this info, please copy the value for MainPos/MainSize.
    float   DpiScale;               // 1.0f = 96 DPI
    KarmaGuiPlatformMonitor()          { MainPos = MainSize = WorkPos = WorkSize = KGVec2(0, 0); DpiScale = 1.0f; }
};

// (Optional) Support for IME (Input Method Editor) via the io.SetPlatformImeDataFn() function.
struct KARMA_API KarmaGuiPlatformImeData
{
    bool    WantVisible;        // A widget wants the IME to be visible
    KGVec2  InputPos;           // Position of the input cursor
    float   InputLineHeight;    // Line height

    KarmaGuiPlatformImeData() { memset(this, 0, sizeof(*this)); }
};
