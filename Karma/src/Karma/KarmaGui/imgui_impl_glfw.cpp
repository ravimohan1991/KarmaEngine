// dear imgui: Platform Backend for GLFW
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan, WebGPU..)
// (Info: GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
// (Requires: GLFW 3.1+. Prefer GLFW 3.3+ for full feature support.)

// Implemented features:
//  [X] Platform: Clipboard support.
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass ImGuiKey values to all key functions e.g. ImGui::IsKeyPressed(ImGuiKey_Space). [Legacy GLFW_KEY_* values will also be supported unless IMGUI_DISABLE_OBSOLETE_KEYIO is set]
//  [X] Platform: Gamepad support. Enable with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange' (note: the resizing cursors requires GLFW 3.4+).
//  [X] Platform: Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable'.

// Issues:
//  [ ] Platform: Multi-viewport support: ParentViewportID not honored, and so io.ConfigViewportsNoDefaultParent has no effect (minor).

// You can use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire imgui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2022-XX-XX: Platform: Added support for multiple windows via the ImGuiPlatformIO interface.
//  2022-11-22: Perform a dummy glfwGetError() read to cancel missing names with glfwGetKeyName(). (#5908)
//  2022-10-18: Perform a dummy glfwGetError() read to cancel missing mouse cursors errors. Using GLFW_VERSION_COMBINED directly. (#5785)
//  2022-10-11: Using 'nullptr' instead of 'NULL' as per our switch to C++11.
//  2022-09-26: Inputs: Renamed ImGuiKey_ModXXX introduced in 1.87 to ImGuiMod_XXX (old names still supported).
//  2022-09-01: Inputs: Honor GLFW_CURSOR_DISABLED by not setting mouse position.
//  2022-04-30: Inputs: Fixed KarmaGui_ImplGlfw_TranslateUntranslatedKey() for lower case letters on OSX.
//  2022-03-23: Inputs: Fixed a regression in 1.87 which resulted in keyboard modifiers events being reported incorrectly on Linux/X11.
//  2022-02-07: Added KarmaGui_ImplGlfw_InstallCallbacks()/KarmaGui_ImplGlfw_RestoreCallbacks() helpers to facilitate user installing callbacks after initializing backend.
//  2022-01-26: Inputs: replaced short-lived io.AddKeyModsEvent() (added two weeks ago) with io.AddKeyEvent() using ImGuiKey_ModXXX flags. Sorry for the confusion.
//  2021-01-20: Inputs: calling new io.AddKeyAnalogEvent() for gamepad support, instead of writing directly to io.NavInputs[].
//  2022-01-17: Inputs: calling new io.AddMousePosEvent(), io.AddMouseButtonEvent(), io.AddMouseWheelEvent() API (1.87+).
//  2022-01-17: Inputs: always update key mods next and before key event (not in NewFrame) to fix input queue with very low framerates.
//  2022-01-12: *BREAKING CHANGE*: Now using glfwSetCursorPosCallback(). If you called KarmaGui_ImplGlfw_InitXXX() with install_callbacks = false, you MUST install glfwSetCursorPosCallback() and forward it to the backend via KarmaGui_ImplGlfw_CursorPosCallback().
//  2022-01-10: Inputs: calling new io.AddKeyEvent(), io.AddKeyModsEvent() + io.SetKeyEventNativeData() API (1.87+). Support for full ImGuiKey range.
//  2022-01-05: Inputs: Converting GLFW untranslated keycodes back to translated keycodes (in the KarmaGui_ImplGlfw_KeyCallback() function) in order to match the behavior of every other backend, and facilitate the use of GLFW with lettered-shortcuts API.
//  2021-08-17: *BREAKING CHANGE*: Now using glfwSetWindowFocusCallback() to calling io.AddFocusEvent(). If you called KarmaGui_ImplGlfw_InitXXX() with install_callbacks = false, you MUST install glfwSetWindowFocusCallback() and forward it to the backend via KarmaGui_ImplGlfw_WindowFocusCallback().
//  2021-07-29: *BREAKING CHANGE*: Now using glfwSetCursorEnterCallback(). MousePos is correctly reported when the host platform window is hovered but not focused. If you called KarmaGui_ImplGlfw_InitXXX() with install_callbacks = false, you MUST install glfwSetWindowFocusCallback() callback and forward it to the backend via KarmaGui_ImplGlfw_CursorEnterCallback().
//  2021-06-29: Reorganized backend to pull data from a single structure to facilitate usage with multiple-contexts (all g_XXXX access changed to bd->XXXX).
//  2020-01-17: Inputs: Disable error callback while assigning mouse cursors because some X11 setup don't have them and it generates errors.
//  2019-12-05: Inputs: Added support for new mouse cursors added in GLFW 3.4+ (resizing cursors, not allowed cursor).
//  2019-10-18: Misc: Previously installed user callbacks are now restored on shutdown.
//  2019-07-21: Inputs: Added mapping for ImGuiKey_KeyPadEnter.
//  2019-05-11: Inputs: Don't filter value from character callback before calling AddInputCharacter().
//  2019-03-12: Misc: Preserve DisplayFramebufferScale when main window is minimized.
//  2018-11-30: Misc: Setting up io.BackendPlatformName so it can be displayed in the About Window.
//  2018-11-07: Inputs: When installing our GLFW callbacks, we save user's previously installed ones - if any - and chain call them.
//  2018-08-01: Inputs: Workaround for Emscripten which doesn't seem to handle focus related calls.
//  2018-06-29: Inputs: Added support for the ImGuiMouseCursor_Hand cursor.
//  2018-06-08: Misc: Extracted imgui_impl_glfw.cpp/.h away from the old combined GLFW+OpenGL/Vulkan examples.
//  2018-03-20: Misc: Setup io.BackendFlags ImGuiBackendFlags_HasMouseCursors flag + honor ImGuiConfigFlags_NoMouseCursorChange flag.
//  2018-02-20: Inputs: Added support for mouse cursors (ImGui::GetMouseCursor() value, passed to glfwSetCursor()).
//  2018-02-06: Misc: Removed call to ImGui::Shutdown() which is not available from 1.60 WIP, user needs to call CreateContext/DestroyContext themselves.
//  2018-02-06: Inputs: Added mapping for ImGuiKey_Space.
//  2018-01-25: Inputs: Added gamepad support if ImGuiConfigFlags_NavEnableGamepad is set.
//  2018-01-25: Inputs: Honoring the io.WantSetMousePos by repositioning the mouse (when using navigation and ImGuiConfigFlags_NavMoveMouse is set).
//  2018-01-20: Inputs: Added Horizontal Mouse Wheel support.
//  2018-01-18: Inputs: Added mapping for ImGuiKey_Insert.
//  2017-08-25: Inputs: MousePos set to -FLT_MAX,-FLT_MAX when mouse is unavailable/missing (instead of -1,-1).
//  2016-10-15: Misc: Added a void* user_data parameter to Clipboard function handlers.

#include "KarmaGui.h"
#include "imgui_impl_glfw.h"

// Clang warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"     // warning: use of old-style cast
#pragma clang diagnostic ignored "-Wsign-conversion"    // warning: implicit conversion changes signedness
#endif

// GLFW
#include <GLFW/glfw3.h>

#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>   // for glfwGetWin32Window()
#endif
#ifdef __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>   // for glfwGetCocoaWindow()
#endif

// We gather version tests as define in order to easily see which features are version-dependent.
#define GLFW_VERSION_COMBINED           (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 + GLFW_VERSION_REVISION)
#define GLFW_HAS_WINDOW_TOPMOST         (GLFW_VERSION_COMBINED >= 3200) // 3.2+ GLFW_FLOATING
#define GLFW_HAS_WINDOW_HOVERED         (GLFW_VERSION_COMBINED >= 3300) // 3.3+ GLFW_HOVERED
#define GLFW_HAS_WINDOW_ALPHA           (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwSetWindowOpacity
#define GLFW_HAS_PER_MONITOR_DPI        (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwGetMonitorContentScale
#define GLFW_HAS_VULKAN                 (GLFW_VERSION_COMBINED >= 3200) // 3.2+ glfwCreateWindowSurface
#define GLFW_HAS_FOCUS_WINDOW           (GLFW_VERSION_COMBINED >= 3200) // 3.2+ glfwFocusWindow
#define GLFW_HAS_FOCUS_ON_SHOW          (GLFW_VERSION_COMBINED >= 3300) // 3.3+ GLFW_FOCUS_ON_SHOW
#define GLFW_HAS_MONITOR_WORK_AREA      (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwGetMonitorWorkarea
#define GLFW_HAS_OSX_WINDOW_POS_FIX     (GLFW_VERSION_COMBINED >= 3301) // 3.3.1+ Fixed: Resizing window repositions it on MacOS #1553
#ifdef GLFW_RESIZE_NESW_CURSOR          // Let's be nice to people who pulled GLFW between 2019-04-16 (3.4 define) and 2019-11-29 (cursors defines) // FIXME: Remove when GLFW 3.4 is released?
#define GLFW_HAS_NEW_CURSORS            (GLFW_VERSION_COMBINED >= 3400) // 3.4+ GLFW_RESIZE_ALL_CURSOR, GLFW_RESIZE_NESW_CURSOR, GLFW_RESIZE_NWSE_CURSOR, GLFW_NOT_ALLOWED_CURSOR
#else
#define GLFW_HAS_NEW_CURSORS            (0)
#endif
#ifdef GLFW_MOUSE_PASSTHROUGH           // Let's be nice to people who pulled GLFW between 2019-04-16 (3.4 define) and 2020-07-17 (passthrough)
#define GLFW_HAS_MOUSE_PASSTHROUGH      (GLFW_VERSION_COMBINED >= 3400) // 3.4+ GLFW_MOUSE_PASSTHROUGH
#else
#define GLFW_HAS_MOUSE_PASSTHROUGH      (0)
#endif
#define GLFW_HAS_GAMEPAD_API            (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwGetGamepadState() new api
#define GLFW_HAS_GETKEYNAME             (GLFW_VERSION_COMBINED >= 3200) // 3.2+ glfwGetKeyName()

// GLFW data
enum GlfwClientApi
{
    GlfwClientApi_Unknown,
    GlfwClientApi_OpenGL,
    GlfwClientApi_Vulkan
};

struct KarmaGui_ImplGlfw_Data
{
    GLFWwindow*             Window;
    GlfwClientApi           ClientApi;
    double                  Time;
    GLFWwindow*             MouseWindow;
    GLFWcursor*             MouseCursors[KGGuiMouseCursor_COUNT];
    KGVec2                  LastValidMousePos;
    GLFWwindow*             KeyOwnerWindows[GLFW_KEY_LAST];
    bool                    InstalledCallbacks;
    bool                    WantUpdateMonitors;
#ifdef _WIN32
    WNDPROC                 GlfwWndProc;
#endif

    // Chain GLFW callbacks: our callbacks will call the user's previously installed callbacks, if any.
    GLFWwindowfocusfun      PrevUserCallbackWindowFocus;
    GLFWcursorposfun        PrevUserCallbackCursorPos;
    GLFWcursorenterfun      PrevUserCallbackCursorEnter;
    GLFWmousebuttonfun      PrevUserCallbackMousebutton;
    GLFWscrollfun           PrevUserCallbackScroll;
    GLFWkeyfun              PrevUserCallbackKey;
    GLFWcharfun             PrevUserCallbackChar;
    GLFWmonitorfun          PrevUserCallbackMonitor;

    KarmaGui_ImplGlfw_Data()   { memset((void*)this, 0, sizeof(*this)); }
};

namespace Karma
{
    // Backend data stored in io.BackendPlatformUserData to allow support for multiple Dear ImGui contexts
    // It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
    // FIXME: multi-context support is not well tested and probably dysfunctional in this backend.
    // - Because glfwPollEvents() process all windows and some events may be called outside of it, you will need to register your own callbacks
    //   (passing install_callbacks=false in KarmaGui_ImplGlfw_InitXXX functions), set the current dear imgui context and then call our callbacks.
    // - Otherwise we may need to store a GLFWWindow* -> ImGuiContext* map and handle this in the backend, adding a little bit of extra complexity to it.
    // FIXME: some shared resources (mouse cursor shape, gamepad) are mishandled when using multi-context.
    static KarmaGui_ImplGlfw_Data* KarmaGui_ImplGlfw_GetBackendData()
    {
        return KarmaGui::GetCurrentContext() ? (KarmaGui_ImplGlfw_Data*)KarmaGui::GetIO().BackendPlatformUserData : nullptr;
    }

    // Forward Declarations
    static void KarmaGui_ImplGlfw_UpdateMonitors();
    static void KarmaGui_ImplGlfw_InitPlatformInterface();
    static void KarmaGui_ImplGlfw_ShutdownPlatformInterface();

    // Functions
    static const char* KarmaGui_ImplGlfw_GetClipboardText(void* user_data)
    {
        return glfwGetClipboardString((GLFWwindow*)user_data);
    }

    static void KarmaGui_ImplGlfw_SetClipboardText(void* user_data, const char* text)
    {
        glfwSetClipboardString((GLFWwindow*)user_data, text);
    }

    static KarmaGuiKey KarmaGui_ImplGlfw_KeyToImGuiKey(int key)
    {
        switch (key)
        {
        case GLFW_KEY_TAB: return KGGuiKey_Tab;
        case GLFW_KEY_LEFT: return KGGuiKey_LeftArrow;
        case GLFW_KEY_RIGHT: return KGGuiKey_RightArrow;
        case GLFW_KEY_UP: return KGGuiKey_UpArrow;
        case GLFW_KEY_DOWN: return KGGuiKey_DownArrow;
        case GLFW_KEY_PAGE_UP: return KGGuiKey_PageUp;
        case GLFW_KEY_PAGE_DOWN: return KGGuiKey_PageDown;
        case GLFW_KEY_HOME: return KGGuiKey_Home;
        case GLFW_KEY_END: return KGGuiKey_End;
        case GLFW_KEY_INSERT: return KGGuiKey_Insert;
        case GLFW_KEY_DELETE: return KGGuiKey_Delete;
        case GLFW_KEY_BACKSPACE: return KGGuiKey_Backspace;
        case GLFW_KEY_SPACE: return KGGuiKey_Space;
        case GLFW_KEY_ENTER: return KGGuiKey_Enter;
        case GLFW_KEY_ESCAPE: return KGGuiKey_Escape;
        case GLFW_KEY_APOSTROPHE: return KGGuiKey_Apostrophe;
        case GLFW_KEY_COMMA: return KGGuiKey_Comma;
        case GLFW_KEY_MINUS: return KGGuiKey_Minus;
        case GLFW_KEY_PERIOD: return KGGuiKey_Period;
        case GLFW_KEY_SLASH: return KGGuiKey_Slash;
        case GLFW_KEY_SEMICOLON: return KGGuiKey_Semicolon;
        case GLFW_KEY_EQUAL: return KGGuiKey_Equal;
        case GLFW_KEY_LEFT_BRACKET: return KGGuiKey_LeftBracket;
        case GLFW_KEY_BACKSLASH: return KGGuiKey_Backslash;
        case GLFW_KEY_RIGHT_BRACKET: return KGGuiKey_RightBracket;
        case GLFW_KEY_GRAVE_ACCENT: return KGGuiKey_GraveAccent;
        case GLFW_KEY_CAPS_LOCK: return KGGuiKey_CapsLock;
        case GLFW_KEY_SCROLL_LOCK: return KGGuiKey_ScrollLock;
        case GLFW_KEY_NUM_LOCK: return KGGuiKey_NumLock;
        case GLFW_KEY_PRINT_SCREEN: return KGGuiKey_PrintScreen;
        case GLFW_KEY_PAUSE: return KGGuiKey_Pause;
        case GLFW_KEY_KP_0: return KGGuiKey_Keypad0;
        case GLFW_KEY_KP_1: return KGGuiKey_Keypad1;
        case GLFW_KEY_KP_2: return KGGuiKey_Keypad2;
        case GLFW_KEY_KP_3: return KGGuiKey_Keypad3;
        case GLFW_KEY_KP_4: return KGGuiKey_Keypad4;
        case GLFW_KEY_KP_5: return KGGuiKey_Keypad5;
        case GLFW_KEY_KP_6: return KGGuiKey_Keypad6;
        case GLFW_KEY_KP_7: return KGGuiKey_Keypad7;
        case GLFW_KEY_KP_8: return KGGuiKey_Keypad8;
        case GLFW_KEY_KP_9: return KGGuiKey_Keypad9;
        case GLFW_KEY_KP_DECIMAL: return KGGuiKey_KeypadDecimal;
        case GLFW_KEY_KP_DIVIDE: return KGGuiKey_KeypadDivide;
        case GLFW_KEY_KP_MULTIPLY: return KGGuiKey_KeypadMultiply;
        case GLFW_KEY_KP_SUBTRACT: return KGGuiKey_KeypadSubtract;
        case GLFW_KEY_KP_ADD: return KGGuiKey_KeypadAdd;
        case GLFW_KEY_KP_ENTER: return KGGuiKey_KeypadEnter;
        case GLFW_KEY_KP_EQUAL: return KGGuiKey_KeypadEqual;
        case GLFW_KEY_LEFT_SHIFT: return KGGuiKey_LeftShift;
        case GLFW_KEY_LEFT_CONTROL: return KGGuiKey_LeftCtrl;
        case GLFW_KEY_LEFT_ALT: return KGGuiKey_LeftAlt;
        case GLFW_KEY_LEFT_SUPER: return KGGuiKey_LeftSuper;
        case GLFW_KEY_RIGHT_SHIFT: return KGGuiKey_RightShift;
        case GLFW_KEY_RIGHT_CONTROL: return KGGuiKey_RightCtrl;
        case GLFW_KEY_RIGHT_ALT: return KGGuiKey_RightAlt;
        case GLFW_KEY_RIGHT_SUPER: return KGGuiKey_RightSuper;
        case GLFW_KEY_MENU: return KGGuiKey_Menu;
        case GLFW_KEY_0: return KGGuiKey_0;
        case GLFW_KEY_1: return KGGuiKey_1;
        case GLFW_KEY_2: return KGGuiKey_2;
        case GLFW_KEY_3: return KGGuiKey_3;
        case GLFW_KEY_4: return KGGuiKey_4;
        case GLFW_KEY_5: return KGGuiKey_5;
        case GLFW_KEY_6: return KGGuiKey_6;
        case GLFW_KEY_7: return KGGuiKey_7;
        case GLFW_KEY_8: return KGGuiKey_8;
        case GLFW_KEY_9: return KGGuiKey_9;
        case GLFW_KEY_A: return KGGuiKey_A;
        case GLFW_KEY_B: return KGGuiKey_B;
        case GLFW_KEY_C: return KGGuiKey_C;
        case GLFW_KEY_D: return KGGuiKey_D;
        case GLFW_KEY_E: return KGGuiKey_E;
        case GLFW_KEY_F: return KGGuiKey_F;
        case GLFW_KEY_G: return KGGuiKey_G;
        case GLFW_KEY_H: return KGGuiKey_H;
        case GLFW_KEY_I: return KGGuiKey_I;
        case GLFW_KEY_J: return KGGuiKey_J;
        case GLFW_KEY_K: return KGGuiKey_K;
        case GLFW_KEY_L: return KGGuiKey_L;
        case GLFW_KEY_M: return KGGuiKey_M;
        case GLFW_KEY_N: return KGGuiKey_N;
        case GLFW_KEY_O: return KGGuiKey_O;
        case GLFW_KEY_P: return KGGuiKey_P;
        case GLFW_KEY_Q: return KGGuiKey_Q;
        case GLFW_KEY_R: return KGGuiKey_R;
        case GLFW_KEY_S: return KGGuiKey_S;
        case GLFW_KEY_T: return KGGuiKey_T;
        case GLFW_KEY_U: return KGGuiKey_U;
        case GLFW_KEY_V: return KGGuiKey_V;
        case GLFW_KEY_W: return KGGuiKey_W;
        case GLFW_KEY_X: return KGGuiKey_X;
        case GLFW_KEY_Y: return KGGuiKey_Y;
        case GLFW_KEY_Z: return KGGuiKey_Z;
        case GLFW_KEY_F1: return KGGuiKey_F1;
        case GLFW_KEY_F2: return KGGuiKey_F2;
        case GLFW_KEY_F3: return KGGuiKey_F3;
        case GLFW_KEY_F4: return KGGuiKey_F4;
        case GLFW_KEY_F5: return KGGuiKey_F5;
        case GLFW_KEY_F6: return KGGuiKey_F6;
        case GLFW_KEY_F7: return KGGuiKey_F7;
        case GLFW_KEY_F8: return KGGuiKey_F8;
        case GLFW_KEY_F9: return KGGuiKey_F9;
        case GLFW_KEY_F10: return KGGuiKey_F10;
        case GLFW_KEY_F11: return KGGuiKey_F11;
        case GLFW_KEY_F12: return KGGuiKey_F12;
        default: return KGGuiKey_None;
        }
    }

    static int KarmaGui_ImplGlfw_KeyToModifier(int key)
    {
        if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
            return GLFW_MOD_CONTROL;
        if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT)
            return GLFW_MOD_SHIFT;
        if (key == GLFW_KEY_LEFT_ALT || key == GLFW_KEY_RIGHT_ALT)
            return GLFW_MOD_ALT;
        if (key == GLFW_KEY_LEFT_SUPER || key == GLFW_KEY_RIGHT_SUPER)
            return GLFW_MOD_SUPER;
        return 0;
    }

    static void KarmaGui_ImplGlfw_UpdateKeyModifiers(int mods)
    {
        KarmaGuiIO& io = KarmaGui::GetIO();
        io.AddKeyEvent(KGGuiMod_Ctrl, (mods & GLFW_MOD_CONTROL) != 0);
        io.AddKeyEvent(KGGuiMod_Shift, (mods & GLFW_MOD_SHIFT) != 0);
        io.AddKeyEvent(KGGuiMod_Alt, (mods & GLFW_MOD_ALT) != 0);
        io.AddKeyEvent(KGGuiMod_Super, (mods & GLFW_MOD_SUPER) != 0);
    }

    void KarmaGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        if (bd->PrevUserCallbackMousebutton != nullptr && window == bd->Window)
            bd->PrevUserCallbackMousebutton(window, button, action, mods);

        KarmaGui_ImplGlfw_UpdateKeyModifiers(mods);

        KarmaGuiIO& io = KarmaGui::GetIO();
        if (button >= 0 && button < KGGuiMouseButton_COUNT)
            io.AddMouseButtonEvent(button, action == GLFW_PRESS);
    }

    void KarmaGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        if (bd->PrevUserCallbackScroll != nullptr && window == bd->Window)
            bd->PrevUserCallbackScroll(window, xoffset, yoffset);

        KarmaGuiIO& io = KarmaGui::GetIO();
        io.AddMouseWheelEvent((float)xoffset, (float)yoffset);
    }

    static int KarmaGui_ImplGlfw_TranslateUntranslatedKey(int key, int scancode)
    {
#if GLFW_HAS_GETKEYNAME && !defined(__EMSCRIPTEN__)
        // GLFW 3.1+ attempts to "untranslate" keys, which goes the opposite of what every other framework does, making using lettered shortcuts difficult.
        // (It had reasons to do so: namely GLFW is/was more likely to be used for WASD-type game controls rather than lettered shortcuts, but IHMO the 3.1 change could have been done differently)
        // See https://github.com/glfw/glfw/issues/1502 for details.
        // Adding a workaround to undo this (so our keys are translated->untranslated->translated, likely a lossy process).
        // This won't cover edge cases but this is at least going to cover common cases.
        if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_EQUAL)
            return key;
        GLFWerrorfun prev_error_callback = glfwSetErrorCallback(nullptr);
        const char* key_name = glfwGetKeyName(key, scancode);
        glfwSetErrorCallback(prev_error_callback);
#if (GLFW_VERSION_COMBINED >= 3300) // Eat errors (see #5908)
        (void)glfwGetError(NULL);
#endif
        if (key_name && key_name[0] != 0 && key_name[1] == 0)
        {
            const char char_names[] = "`-=[]\\,;\'./";
            const int char_keys[] = { GLFW_KEY_GRAVE_ACCENT, GLFW_KEY_MINUS, GLFW_KEY_EQUAL, GLFW_KEY_LEFT_BRACKET, GLFW_KEY_RIGHT_BRACKET, GLFW_KEY_BACKSLASH, GLFW_KEY_COMMA, GLFW_KEY_SEMICOLON, GLFW_KEY_APOSTROPHE, GLFW_KEY_PERIOD, GLFW_KEY_SLASH, 0 };
            KR_CORE_ASSERT(KG_ARRAYSIZE(char_names) == KG_ARRAYSIZE(char_keys), "");
            if (key_name[0] >= '0' && key_name[0] <= '9') { key = GLFW_KEY_0 + (key_name[0] - '0'); }
            else if (key_name[0] >= 'A' && key_name[0] <= 'Z') { key = GLFW_KEY_A + (key_name[0] - 'A'); }
            else if (key_name[0] >= 'a' && key_name[0] <= 'z') { key = GLFW_KEY_A + (key_name[0] - 'a'); }
            else if (const char* p = strchr(char_names, key_name[0])) { key = char_keys[p - char_names]; }
        }
        // if (action == GLFW_PRESS) printf("key %d scancode %d name '%s'\n", key, scancode, key_name);
#else
        IM_UNUSED(scancode);
#endif
        return key;
    }

    void KarmaGui_ImplGlfw_KeyCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
    {
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        if (bd->PrevUserCallbackKey != nullptr && window == bd->Window)
            bd->PrevUserCallbackKey(window, keycode, scancode, action, mods);

        if (action != GLFW_PRESS && action != GLFW_RELEASE)
            return;

        // Workaround: X11 does not include current pressed/released modifier key in 'mods' flags. https://github.com/glfw/glfw/issues/1630
        if (int keycode_to_mod = KarmaGui_ImplGlfw_KeyToModifier(keycode))
            mods = (action == GLFW_PRESS) ? (mods | keycode_to_mod) : (mods & ~keycode_to_mod);
        KarmaGui_ImplGlfw_UpdateKeyModifiers(mods);

        if (keycode >= 0 && keycode < KG_ARRAYSIZE(bd->KeyOwnerWindows))
            bd->KeyOwnerWindows[keycode] = (action == GLFW_PRESS) ? window : nullptr;

        keycode = KarmaGui_ImplGlfw_TranslateUntranslatedKey(keycode, scancode);

        KarmaGuiIO& io = KarmaGui::GetIO();
        KarmaGuiKey imgui_key = KarmaGui_ImplGlfw_KeyToImGuiKey(keycode);
        io.AddKeyEvent(imgui_key, (action == GLFW_PRESS));
        io.SetKeyEventNativeData(imgui_key, keycode, scancode); // To support legacy indexing (<1.87 user code)
    }

    void KarmaGui_ImplGlfw_WindowFocusCallback(GLFWwindow* window, int focused)
    {
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        if (bd->PrevUserCallbackWindowFocus != nullptr && window == bd->Window)
            bd->PrevUserCallbackWindowFocus(window, focused);

        KarmaGuiIO& io = KarmaGui::GetIO();
        io.AddFocusEvent(focused != 0);
    }

    void KarmaGui_ImplGlfw_CursorPosCallback(GLFWwindow* window, double x, double y)
    {
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        if (bd->PrevUserCallbackCursorPos != nullptr && window == bd->Window)
            bd->PrevUserCallbackCursorPos(window, x, y);
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
            return;

        KarmaGuiIO& io = KarmaGui::GetIO();
        if (io.ConfigFlags & KGGuiConfigFlags_ViewportsEnable)
        {
            int window_x, window_y;
            glfwGetWindowPos(window, &window_x, &window_y);
            x += window_x;
            y += window_y;
        }
        io.AddMousePosEvent((float)x, (float)y);
        bd->LastValidMousePos = KGVec2((float)x, (float)y);
    }

    // Workaround: X11 seems to send spurious Leave/Enter events which would make us lose our position,
    // so we back it up and restore on Leave/Enter (see https://github.com/ocornut/imgui/issues/4984)
    void KarmaGui_ImplGlfw_CursorEnterCallback(GLFWwindow* window, int entered)
    {
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        if (bd->PrevUserCallbackCursorEnter != nullptr && window == bd->Window)
            bd->PrevUserCallbackCursorEnter(window, entered);
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
            return;

        KarmaGuiIO& io = KarmaGui::GetIO();
        if (entered)
        {
            bd->MouseWindow = window;
            io.AddMousePosEvent(bd->LastValidMousePos.x, bd->LastValidMousePos.y);
        }
        else if (!entered && bd->MouseWindow == window)
        {
            bd->LastValidMousePos = io.MousePos;
            bd->MouseWindow = nullptr;
            io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
        }
    }

    void KarmaGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c)
    {
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        if (bd->PrevUserCallbackChar != nullptr && window == bd->Window)
            bd->PrevUserCallbackChar(window, c);

        KarmaGuiIO& io = KarmaGui::GetIO();
        io.AddInputCharacter(c);
    }

    void KarmaGui_ImplGlfw_MonitorCallback(GLFWmonitor*, int)
    {
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        bd->WantUpdateMonitors = true;
    }

    void KarmaGui_ImplGlfw_InstallCallbacks(GLFWwindow* window)
    {
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        KR_CORE_ASSERT(bd->InstalledCallbacks == false, "Callbacks already installed!");
        KR_CORE_ASSERT(bd->Window == window, "");

        bd->PrevUserCallbackWindowFocus = glfwSetWindowFocusCallback(window, KarmaGui_ImplGlfw_WindowFocusCallback);
        bd->PrevUserCallbackCursorEnter = glfwSetCursorEnterCallback(window, KarmaGui_ImplGlfw_CursorEnterCallback);
        bd->PrevUserCallbackCursorPos = glfwSetCursorPosCallback(window, KarmaGui_ImplGlfw_CursorPosCallback);
        bd->PrevUserCallbackMousebutton = glfwSetMouseButtonCallback(window, KarmaGui_ImplGlfw_MouseButtonCallback);
        bd->PrevUserCallbackScroll = glfwSetScrollCallback(window, KarmaGui_ImplGlfw_ScrollCallback);
        bd->PrevUserCallbackKey = glfwSetKeyCallback(window, KarmaGui_ImplGlfw_KeyCallback);
        bd->PrevUserCallbackChar = glfwSetCharCallback(window, KarmaGui_ImplGlfw_CharCallback);
        bd->PrevUserCallbackMonitor = glfwSetMonitorCallback(KarmaGui_ImplGlfw_MonitorCallback);
        bd->InstalledCallbacks = true;
    }

    void KarmaGui_ImplGlfw_RestoreCallbacks(GLFWwindow* window)
    {
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        KR_CORE_ASSERT(bd->InstalledCallbacks == true, "Callbacks not installed!");
        KR_CORE_ASSERT(bd->Window == window, "");

        glfwSetWindowFocusCallback(window, bd->PrevUserCallbackWindowFocus);
        glfwSetCursorEnterCallback(window, bd->PrevUserCallbackCursorEnter);
        glfwSetCursorPosCallback(window, bd->PrevUserCallbackCursorPos);
        glfwSetMouseButtonCallback(window, bd->PrevUserCallbackMousebutton);
        glfwSetScrollCallback(window, bd->PrevUserCallbackScroll);
        glfwSetKeyCallback(window, bd->PrevUserCallbackKey);
        glfwSetCharCallback(window, bd->PrevUserCallbackChar);
        glfwSetMonitorCallback(bd->PrevUserCallbackMonitor);
        bd->InstalledCallbacks = false;
        bd->PrevUserCallbackWindowFocus = nullptr;
        bd->PrevUserCallbackCursorEnter = nullptr;
        bd->PrevUserCallbackCursorPos = nullptr;
        bd->PrevUserCallbackMousebutton = nullptr;
        bd->PrevUserCallbackScroll = nullptr;
        bd->PrevUserCallbackKey = nullptr;
        bd->PrevUserCallbackChar = nullptr;
        bd->PrevUserCallbackMonitor = nullptr;
    }

    static bool KarmaGui_ImplGlfw_Init(GLFWwindow* window, bool install_callbacks, GlfwClientApi client_api)
    {
        KarmaGuiIO& io = KarmaGui::GetIO();
        KR_CORE_ASSERT(io.BackendPlatformUserData == nullptr, "Already initialized a platform backend!");
        //printf("GLFW_VERSION: %d.%d.%d (%d)", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION, GLFW_VERSION_COMBINED);

        // Setup backend capabilities flags
        KarmaGui_ImplGlfw_Data* bd = KG_NEW(KarmaGui_ImplGlfw_Data)();
        io.BackendPlatformUserData = (void*)bd;
        io.BackendPlatformName = "GLFW got back";
        io.BackendFlags |= KGGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
        io.BackendFlags |= KGGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
        io.BackendFlags |= KGGuiBackendFlags_PlatformHasViewports;    // We can create multi-viewports on the Platform side (optional)
#if GLFW_HAS_MOUSE_PASSTHROUGH || (GLFW_HAS_WINDOW_HOVERED && defined(_WIN32))
        io.BackendFlags |= KGGuiBackendFlags_HasMouseHoveredViewport; // We can call io.AddMouseViewportEvent() with correct data (optional)
#endif

        bd->Window = window;
        bd->Time = 0.0;
        bd->WantUpdateMonitors = true;

        io.SetClipboardTextFn = KarmaGui_ImplGlfw_SetClipboardText;
        io.GetClipboardTextFn = KarmaGui_ImplGlfw_GetClipboardText;
        io.ClipboardUserData = bd->Window;

        // Create mouse cursors
        // (By design, on X11 cursors are user configurable and some cursors may be missing. When a cursor doesn't exist,
        // GLFW will emit an error which will often be printed by the app, so we temporarily disable error reporting.
        // Missing cursors will return nullptr and our _UpdateMouseCursor() function will use the Arrow cursor instead.)
        GLFWerrorfun prev_error_callback = glfwSetErrorCallback(nullptr);
        bd->MouseCursors[KGGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        bd->MouseCursors[KGGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
        bd->MouseCursors[KGGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
        bd->MouseCursors[KGGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        bd->MouseCursors[KGGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
#if GLFW_HAS_NEW_CURSORS
        bd->MouseCursors[KGGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
        bd->MouseCursors[KGGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
        bd->MouseCursors[KGGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
        bd->MouseCursors[KGGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);
#else
		bd->MouseCursors[KGGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		bd->MouseCursors[KGGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		bd->MouseCursors[KGGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		bd->MouseCursors[KGGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
#endif
        glfwSetErrorCallback(prev_error_callback);
#if (GLFW_VERSION_COMBINED >= 3300) // Eat errors (see #5785)
        (void)glfwGetError(NULL);
#endif

        // Chain GLFW callbacks: our callbacks will call the user's previously installed callbacks, if any.
        if (install_callbacks)
            KarmaGui_ImplGlfw_InstallCallbacks(window);

        // Update monitors the first time (note: monitor callback are broken in GLFW 3.2 and earlier, see github.com/glfw/glfw/issues/784)
        KarmaGui_ImplGlfw_UpdateMonitors();
        glfwSetMonitorCallback(KarmaGui_ImplGlfw_MonitorCallback);

        // Our mouse update function expect PlatformHandle to be filled for the main viewport
        KarmaGuiViewport* main_viewport = KarmaGui::GetMainViewport();
        main_viewport->PlatformHandle = (void*)bd->Window;
#ifdef _WIN32
        main_viewport->PlatformHandleRaw = glfwGetWin32Window(bd->Window);
#elif defined(__APPLE__)
        main_viewport->PlatformHandleRaw = (void*)glfwGetCocoaWindow(bd->Window);
#endif
        if (io.ConfigFlags & KGGuiConfigFlags_ViewportsEnable)
            KarmaGui_ImplGlfw_InitPlatformInterface();

        bd->ClientApi = client_api;
        return true;
    }

    bool KarmaGui_ImplGlfw_InitForOpenGL(GLFWwindow* window, bool install_callbacks)
    {
        return KarmaGui_ImplGlfw_Init(window, install_callbacks, GlfwClientApi_OpenGL);
    }

    bool KarmaGui_ImplGlfw_InitForVulkan(GLFWwindow* window, bool install_callbacks)
    {
        return KarmaGui_ImplGlfw_Init(window, install_callbacks, GlfwClientApi_Vulkan);
    }

    bool KarmaGui_ImplGlfw_InitForOther(GLFWwindow* window, bool install_callbacks)
    {
        return KarmaGui_ImplGlfw_Init(window, install_callbacks, GlfwClientApi_Unknown);
    }

    void KarmaGui_ImplGlfw_Shutdown()
    {
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        KR_CORE_ASSERT(bd != nullptr, "No platform backend to shutdown, or already shutdown?");
        KarmaGuiIO& io = KarmaGui::GetIO();

        KarmaGui_ImplGlfw_ShutdownPlatformInterface();

        if (bd->InstalledCallbacks)
            KarmaGui_ImplGlfw_RestoreCallbacks(bd->Window);

        for (KarmaGuiMouseCursor cursor_n = 0; cursor_n < KGGuiMouseCursor_COUNT; cursor_n++)
            glfwDestroyCursor(bd->MouseCursors[cursor_n]);

        io.BackendPlatformName = nullptr;
        io.BackendPlatformUserData = nullptr;
        KG_DELETE(bd);
    }

    static void KarmaGui_ImplGlfw_UpdateMouseData()
    {
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        KarmaGuiIO& io = KarmaGui::GetIO();
        KarmaGuiPlatformIO& platform_io = KarmaGui::GetPlatformIO();

        if (glfwGetInputMode(bd->Window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        {
            io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
            return;
        }

        KGGuiID mouse_viewport_id = 0;
        const KGVec2 mouse_pos_prev = io.MousePos;
        for (int n = 0; n < platform_io.Viewports.Size; n++)
        {
            KarmaGuiViewport* viewport = platform_io.Viewports[n];
            GLFWwindow* window = (GLFWwindow*)viewport->PlatformHandle;

#ifdef __EMSCRIPTEN__
            const bool is_window_focused = true;
#else
            const bool is_window_focused = glfwGetWindowAttrib(window, GLFW_FOCUSED) != 0;
#endif
            if (is_window_focused)
            {
                // (Optional) Set OS mouse position from Dear ImGui if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
                // When multi-viewports are enabled, all Dear ImGui positions are same as OS positions.
                if (io.WantSetMousePos)
                    glfwSetCursorPos(window, (double)(mouse_pos_prev.x - viewport->Pos.x), (double)(mouse_pos_prev.y - viewport->Pos.y));

                // (Optional) Fallback to provide mouse position when focused (KarmaGui_ImplGlfw_CursorPosCallback already provides this when hovered or captured)
                if (bd->MouseWindow == nullptr)
                {
                    double mouse_x, mouse_y;
                    glfwGetCursorPos(window, &mouse_x, &mouse_y);
                    if (io.ConfigFlags & KGGuiConfigFlags_ViewportsEnable)
                    {
                        // Single viewport mode: mouse position in client window coordinates (io.MousePos is (0,0) when the mouse is on the upper-left corner of the app window)
                        // Multi-viewport mode: mouse position in OS absolute coordinates (io.MousePos is (0,0) when the mouse is on the upper-left of the primary monitor)
                        int window_x, window_y;
                        glfwGetWindowPos(window, &window_x, &window_y);
                        mouse_x += window_x;
                        mouse_y += window_y;
                    }
                    bd->LastValidMousePos = KGVec2((float)mouse_x, (float)mouse_y);
                    io.AddMousePosEvent((float)mouse_x, (float)mouse_y);
                }
            }

            // (Optional) When using multiple viewports: call io.AddMouseViewportEvent() with the viewport the OS mouse cursor is hovering.
            // If ImGuiBackendFlags_HasMouseHoveredViewport is not set by the backend, Dear imGui will ignore this field and infer the information using its flawed heuristic.
            // - [X] GLFW >= 3.3 backend ON WINDOWS ONLY does correctly ignore viewports with the _NoInputs flag.
            // - [!] GLFW <= 3.2 backend CANNOT correctly ignore viewports with the _NoInputs flag, and CANNOT reported Hovered Viewport because of mouse capture.
            //       Some backend are not able to handle that correctly. If a backend report an hovered viewport that has the _NoInputs flag (e.g. when dragging a window
            //       for docking, the viewport has the _NoInputs flag in order to allow us to find the viewport under), then Dear ImGui is forced to ignore the value reported
            //       by the backend, and use its flawed heuristic to guess the viewport behind.
            // - [X] GLFW backend correctly reports this regardless of another viewport behind focused and dragged from (we need this to find a useful drag and drop target).
            // FIXME: This is currently only correct on Win32. See what we do below with the WM_NCHITTEST, missing an equivalent for other systems.
            // See https://github.com/glfw/glfw/issues/1236 if you want to help in making this a GLFW feature.
#if GLFW_HAS_MOUSE_PASSTHROUGH || (GLFW_HAS_WINDOW_HOVERED && defined(_WIN32))
            const bool window_no_input = (viewport->Flags & KGGuiViewportFlags_NoInputs) != 0;
#if GLFW_HAS_MOUSE_PASSTHROUGH
            glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, window_no_input);
#endif
            if (glfwGetWindowAttrib(window, GLFW_HOVERED) && !window_no_input)
                mouse_viewport_id = viewport->ID;
#else
        // We cannot use bd->MouseWindow maintained from CursorEnter/Leave callbacks, because it is locked to the window capturing mouse.
#endif
        }

        if (io.BackendFlags & KGGuiBackendFlags_HasMouseHoveredViewport)
            io.AddMouseViewportEvent(mouse_viewport_id);
    }

    static void KarmaGui_ImplGlfw_UpdateMouseCursor()
    {
        KarmaGuiIO& io = KarmaGui::GetIO();
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        if ((io.ConfigFlags & KGGuiConfigFlags_NoMouseCursorChange) || glfwGetInputMode(bd->Window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
            return;

        KarmaGuiMouseCursor imgui_cursor = KarmaGui::GetMouseCursor();
        KarmaGuiPlatformIO& platform_io = KarmaGui::GetPlatformIO();
        for (int n = 0; n < platform_io.Viewports.Size; n++)
        {
            GLFWwindow* window = (GLFWwindow*)platform_io.Viewports[n]->PlatformHandle;
            if (imgui_cursor == KGGuiMouseCursor_None || io.MouseDrawCursor)
            {
                // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            }
            else
            {
                // Show OS mouse cursor
                // FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse cursor with GLFW 3.2, but 3.3 works here.
                glfwSetCursor(window, bd->MouseCursors[imgui_cursor] ? bd->MouseCursors[imgui_cursor] : bd->MouseCursors[KGGuiMouseCursor_Arrow]);
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
    }

    // Update gamepad inputs
    static inline float Saturate(float v) { return v < 0.0f ? 0.0f : v  > 1.0f ? 1.0f : v; }
    static void KarmaGui_ImplGlfw_UpdateGamepads()
    {
        KarmaGuiIO& io = KarmaGui::GetIO();
        if ((io.ConfigFlags & KGGuiConfigFlags_NavEnableGamepad) == 0) // FIXME: Technically feeding gamepad shouldn't depend on this now that they are regular inputs.
            return;

        io.BackendFlags &= ~KGGuiBackendFlags_HasGamepad;
#if GLFW_HAS_GAMEPAD_API
        GLFWgamepadstate gamepad;
        if (!glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad))
            return;
#define MAP_BUTTON(KEY_NO, BUTTON_NO, _UNUSED)          do { io.AddKeyEvent(KEY_NO, gamepad.buttons[BUTTON_NO] != 0); } while (0)
#define MAP_ANALOG(KEY_NO, AXIS_NO, _UNUSED, V0, V1)    do { float v = gamepad.axes[AXIS_NO]; v = (v - V0) / (V1 - V0); io.AddKeyAnalogEvent(KEY_NO, v > 0.10f, Saturate(v)); } while (0)
#else
        int axes_count = 0, buttons_count = 0;
        const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
        const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);
        if (axes_count == 0 || buttons_count == 0)
            return;
#define MAP_BUTTON(KEY_NO, _UNUSED, BUTTON_NO)          do { io.AddKeyEvent(KEY_NO, (buttons_count > BUTTON_NO && buttons[BUTTON_NO] == GLFW_PRESS)); } while (0)
#define MAP_ANALOG(KEY_NO, _UNUSED, AXIS_NO, V0, V1)    do { float v = (axes_count > AXIS_NO) ? axes[AXIS_NO] : V0; v = (v - V0) / (V1 - V0); io.AddKeyAnalogEvent(KEY_NO, v > 0.10f, Saturate(v)); } while (0)
#endif
        io.BackendFlags |= KGGuiBackendFlags_HasGamepad;
        MAP_BUTTON(KGGuiKey_GamepadStart, GLFW_GAMEPAD_BUTTON_START, 7);
        MAP_BUTTON(KGGuiKey_GamepadBack, GLFW_GAMEPAD_BUTTON_BACK, 6);
        MAP_BUTTON(KGGuiKey_GamepadFaceLeft, GLFW_GAMEPAD_BUTTON_X, 2);     // Xbox X, PS Square
        MAP_BUTTON(KGGuiKey_GamepadFaceRight, GLFW_GAMEPAD_BUTTON_B, 1);     // Xbox B, PS Circle
        MAP_BUTTON(KGGuiKey_GamepadFaceUp, GLFW_GAMEPAD_BUTTON_Y, 3);     // Xbox Y, PS Triangle
        MAP_BUTTON(KGGuiKey_GamepadFaceDown, GLFW_GAMEPAD_BUTTON_A, 0);     // Xbox A, PS Cross
        MAP_BUTTON(KGGuiKey_GamepadDpadLeft, GLFW_GAMEPAD_BUTTON_DPAD_LEFT, 13);
        MAP_BUTTON(KGGuiKey_GamepadDpadRight, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, 11);
        MAP_BUTTON(KGGuiKey_GamepadDpadUp, GLFW_GAMEPAD_BUTTON_DPAD_UP, 10);
        MAP_BUTTON(KGGuiKey_GamepadDpadDown, GLFW_GAMEPAD_BUTTON_DPAD_DOWN, 12);
        MAP_BUTTON(KGGuiKey_GamepadL1, GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, 4);
        MAP_BUTTON(KGGuiKey_GamepadR1, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, 5);
        MAP_ANALOG(KGGuiKey_GamepadL2, GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, 4, -0.75f, +1.0f);
        MAP_ANALOG(KGGuiKey_GamepadR2, GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER, 5, -0.75f, +1.0f);
        MAP_BUTTON(KGGuiKey_GamepadL3, GLFW_GAMEPAD_BUTTON_LEFT_THUMB, 8);
        MAP_BUTTON(KGGuiKey_GamepadR3, GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, 9);
        MAP_ANALOG(KGGuiKey_GamepadLStickLeft, GLFW_GAMEPAD_AXIS_LEFT_X, 0, -0.25f, -1.0f);
        MAP_ANALOG(KGGuiKey_GamepadLStickRight, GLFW_GAMEPAD_AXIS_LEFT_X, 0, +0.25f, +1.0f);
        MAP_ANALOG(KGGuiKey_GamepadLStickUp, GLFW_GAMEPAD_AXIS_LEFT_Y, 1, -0.25f, -1.0f);
        MAP_ANALOG(KGGuiKey_GamepadLStickDown, GLFW_GAMEPAD_AXIS_LEFT_Y, 1, +0.25f, +1.0f);
        MAP_ANALOG(KGGuiKey_GamepadRStickLeft, GLFW_GAMEPAD_AXIS_RIGHT_X, 2, -0.25f, -1.0f);
        MAP_ANALOG(KGGuiKey_GamepadRStickRight, GLFW_GAMEPAD_AXIS_RIGHT_X, 2, +0.25f, +1.0f);
        MAP_ANALOG(KGGuiKey_GamepadRStickUp, GLFW_GAMEPAD_AXIS_RIGHT_Y, 3, -0.25f, -1.0f);
        MAP_ANALOG(KGGuiKey_GamepadRStickDown, GLFW_GAMEPAD_AXIS_RIGHT_Y, 3, +0.25f, +1.0f);
#undef MAP_BUTTON
#undef MAP_ANALOG
    }

    static void KarmaGui_ImplGlfw_UpdateMonitors()
    {
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        KarmaGuiPlatformIO& platform_io = KarmaGui::GetPlatformIO();
        int monitors_count = 0;
        GLFWmonitor** glfw_monitors = glfwGetMonitors(&monitors_count);
        platform_io.Monitors.resize(0);
        for (int n = 0; n < monitors_count; n++)
        {
            KarmaGuiPlatformMonitor monitor;
            int x, y;
            glfwGetMonitorPos(glfw_monitors[n], &x, &y);
            const GLFWvidmode* vid_mode = glfwGetVideoMode(glfw_monitors[n]);
            monitor.MainPos = monitor.WorkPos = KGVec2((float)x, (float)y);
            monitor.MainSize = monitor.WorkSize = KGVec2((float)vid_mode->width, (float)vid_mode->height);
#if GLFW_HAS_MONITOR_WORK_AREA
            int w, h;
            glfwGetMonitorWorkarea(glfw_monitors[n], &x, &y, &w, &h);
            if (w > 0 && h > 0) // Workaround a small GLFW issue reporting zero on monitor changes: https://github.com/glfw/glfw/pull/1761
            {
                monitor.WorkPos = KGVec2((float)x, (float)y);
                monitor.WorkSize = KGVec2((float)w, (float)h);
            }
#endif
#if GLFW_HAS_PER_MONITOR_DPI
            // Warning: the validity of monitor DPI information on Windows depends on the application DPI awareness settings, which generally needs to be set in the manifest or at runtime.
            float x_scale, y_scale;
            glfwGetMonitorContentScale(glfw_monitors[n], &x_scale, &y_scale);
            monitor.DpiScale = x_scale;
#endif
            platform_io.Monitors.push_back(monitor);
        }
        bd->WantUpdateMonitors = false;
    }

    void KarmaGui_ImplGlfw_NewFrame()
    {
        KarmaGuiIO& io = KarmaGui::GetIO();
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        KR_CORE_ASSERT(bd != nullptr, "Did you call KarmaGui_ImplGlfw_InitForXXX()?");

        // Setup display size (every frame to accommodate for window resizing)
        int w, h;
        int display_w, display_h;
        glfwGetWindowSize(bd->Window, &w, &h);
        glfwGetFramebufferSize(bd->Window, &display_w, &display_h);
        io.DisplaySize = KGVec2((float)w, (float)h);
        if (w > 0 && h > 0)
            io.DisplayFramebufferScale = KGVec2((float)display_w / (float)w, (float)display_h / (float)h);
        if (bd->WantUpdateMonitors)
            KarmaGui_ImplGlfw_UpdateMonitors();

        // Setup time step
        double current_time = glfwGetTime();
        io.DeltaTime = bd->Time > 0.0 ? (float)(current_time - bd->Time) : (float)(1.0f / 60.0f);
        bd->Time = current_time;

        KarmaGui_ImplGlfw_UpdateMouseData();
        KarmaGui_ImplGlfw_UpdateMouseCursor();

        // Update game controllers (if enabled and available)
        KarmaGui_ImplGlfw_UpdateGamepads();
    }

    //--------------------------------------------------------------------------------------------------------
    // MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
    // This is an _advanced_ and _optional_ feature, allowing the backend to create and handle multiple viewports simultaneously.
    // If you are new to dear imgui or creating a new binding for dear imgui, it is recommended that you completely ignore this section first..
    //--------------------------------------------------------------------------------------------------------

    // Helper structure we store in the void* RenderUserData field of each ImGuiViewport to easily retrieve our backend data.
    struct KarmaGui_ImplGlfw_ViewportData
    {
        GLFWwindow* Window;
        bool        WindowOwned;
        int         IgnoreWindowPosEventFrame;
        int         IgnoreWindowSizeEventFrame;

        KarmaGui_ImplGlfw_ViewportData() { Window = nullptr; WindowOwned = false; IgnoreWindowSizeEventFrame = IgnoreWindowPosEventFrame = -1; }
        ~KarmaGui_ImplGlfw_ViewportData() { KR_CORE_ASSERT(Window == nullptr, ""); }
    };

    static void KarmaGui_ImplGlfw_WindowCloseCallback(GLFWwindow* window)
    {
        if (KarmaGuiViewport* viewport = KarmaGui::FindViewportByPlatformHandle(window))
            viewport->PlatformRequestClose = true;
    }

    // GLFW may dispatch window pos/size events after calling glfwSetWindowPos()/glfwSetWindowSize().
    // However: depending on the platform the callback may be invoked at different time:
    // - on Windows it appears to be called within the glfwSetWindowPos()/glfwSetWindowSize() call
    // - on Linux it is queued and invoked during glfwPollEvents()
    // Because the event doesn't always fire on glfwSetWindowXXX() we use a frame counter tag to only
    // ignore recent glfwSetWindowXXX() calls.
    static void KarmaGui_ImplGlfw_WindowPosCallback(GLFWwindow* window, int, int)
    {
        if (KarmaGuiViewport* viewport = KarmaGui::FindViewportByPlatformHandle(window))
        {
            if (KarmaGui_ImplGlfw_ViewportData* vd = (KarmaGui_ImplGlfw_ViewportData*)viewport->PlatformUserData)
            {
                bool ignore_event = (KarmaGui::GetFrameCount() <= vd->IgnoreWindowPosEventFrame + 1);
                //data->IgnoreWindowPosEventFrame = -1;
                if (ignore_event)
                    return;
            }
            viewport->PlatformRequestMove = true;
        }
    }

    static void KarmaGui_ImplGlfw_WindowSizeCallback(GLFWwindow* window, int, int)
    {
        if (KarmaGuiViewport* viewport = KarmaGui::FindViewportByPlatformHandle(window))
        {
            if (KarmaGui_ImplGlfw_ViewportData* vd = (KarmaGui_ImplGlfw_ViewportData*)viewport->PlatformUserData)
            {
                bool ignore_event = (KarmaGui::GetFrameCount() <= vd->IgnoreWindowSizeEventFrame + 1);
                //data->IgnoreWindowSizeEventFrame = -1;
                if (ignore_event)
                    return;
            }
            viewport->PlatformRequestResize = true;
        }
    }

    static void KarmaGui_ImplGlfw_CreateWindow(KarmaGuiViewport* viewport)
    {
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        KarmaGui_ImplGlfw_ViewportData* vd = KG_NEW(KarmaGui_ImplGlfw_ViewportData)();
        viewport->PlatformUserData = vd;

        // GLFW 3.2 unfortunately always set focus on glfwCreateWindow() if GLFW_VISIBLE is set, regardless of GLFW_FOCUSED
        // With GLFW 3.3, the hint GLFW_FOCUS_ON_SHOW fixes this problem
        glfwWindowHint(GLFW_VISIBLE, false);
        glfwWindowHint(GLFW_FOCUSED, false);
#if GLFW_HAS_FOCUS_ON_SHOW
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, false);
#endif
        glfwWindowHint(GLFW_DECORATED, (viewport->Flags & KGGuiViewportFlags_NoDecoration) ? false : true);
#if GLFW_HAS_WINDOW_TOPMOST
        glfwWindowHint(GLFW_FLOATING, (viewport->Flags & KGGuiViewportFlags_TopMost) ? true : false);
#endif
        GLFWwindow* share_window = (bd->ClientApi == GlfwClientApi_OpenGL) ? bd->Window : nullptr;
        vd->Window = glfwCreateWindow((int)viewport->Size.x, (int)viewport->Size.y, "No Title Yet", nullptr, share_window);
        vd->WindowOwned = true;
        viewport->PlatformHandle = (void*)vd->Window;
#ifdef _WIN32
        viewport->PlatformHandleRaw = glfwGetWin32Window(vd->Window);
#elif defined(__APPLE__)
        viewport->PlatformHandleRaw = (void*)glfwGetCocoaWindow(vd->Window);
#endif
        glfwSetWindowPos(vd->Window, (int)viewport->Pos.x, (int)viewport->Pos.y);

        // Install GLFW callbacks for secondary viewports
        glfwSetWindowFocusCallback(vd->Window, KarmaGui_ImplGlfw_WindowFocusCallback);
        glfwSetCursorEnterCallback(vd->Window, KarmaGui_ImplGlfw_CursorEnterCallback);
        glfwSetCursorPosCallback(vd->Window, KarmaGui_ImplGlfw_CursorPosCallback);
        glfwSetMouseButtonCallback(vd->Window, KarmaGui_ImplGlfw_MouseButtonCallback);
        glfwSetScrollCallback(vd->Window, KarmaGui_ImplGlfw_ScrollCallback);
        glfwSetKeyCallback(vd->Window, KarmaGui_ImplGlfw_KeyCallback);
        glfwSetCharCallback(vd->Window, KarmaGui_ImplGlfw_CharCallback);
        glfwSetWindowCloseCallback(vd->Window, KarmaGui_ImplGlfw_WindowCloseCallback);
        glfwSetWindowPosCallback(vd->Window, KarmaGui_ImplGlfw_WindowPosCallback);
        glfwSetWindowSizeCallback(vd->Window, KarmaGui_ImplGlfw_WindowSizeCallback);
        if (bd->ClientApi == GlfwClientApi_OpenGL)
        {
            glfwMakeContextCurrent(vd->Window);
            glfwSwapInterval(0);
        }
    }

    static void KarmaGui_ImplGlfw_DestroyWindow(KarmaGuiViewport* viewport)
    {
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        if (KarmaGui_ImplGlfw_ViewportData* vd = (KarmaGui_ImplGlfw_ViewportData*)viewport->PlatformUserData)
        {
            if (vd->WindowOwned)
            {
#if !GLFW_HAS_MOUSE_PASSTHROUGH && GLFW_HAS_WINDOW_HOVERED && defined(_WIN32)
                HWND hwnd = (HWND)viewport->PlatformHandleRaw;
                ::RemovePropA(hwnd, "IMGUI_VIEWPORT");
#endif

                // Release any keys that were pressed in the window being destroyed and are still held down,
                // because we will not receive any release events after window is destroyed.
                for (int i = 0; i < KG_ARRAYSIZE(bd->KeyOwnerWindows); i++)
                    if (bd->KeyOwnerWindows[i] == vd->Window)
                        KarmaGui_ImplGlfw_KeyCallback(vd->Window, i, 0, GLFW_RELEASE, 0); // Later params are only used for main viewport, on which this function is never called.

                glfwDestroyWindow(vd->Window);
            }
            vd->Window = nullptr;
            KG_DELETE(vd);
        }
        viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
    }

    // We have submitted https://github.com/glfw/glfw/pull/1568 to allow GLFW to support "transparent inputs".
    // In the meanwhile we implement custom per-platform workarounds here (FIXME-VIEWPORT: Implement same work-around for Linux/OSX!)
#if !GLFW_HAS_MOUSE_PASSTHROUGH && GLFW_HAS_WINDOW_HOVERED && defined(_WIN32)
    static LRESULT CALLBACK WndProcNoInputs(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        if (msg == WM_NCHITTEST)
        {
            // Let mouse pass-through the window. This will allow the backend to call io.AddMouseViewportEvent() properly (which is OPTIONAL).
            // The ImGuiViewportFlags_NoInputs flag is set while dragging a viewport, as want to detect the window behind the one we are dragging.
            // If you cannot easily access those viewport flags from your windowing/event code: you may manually synchronize its state e.g. in
            // your main loop after calling UpdatePlatformWindows(). Iterate all viewports/platform windows and pass the flag to your windowing system.
            ImGuiViewport* viewport = (ImGuiViewport*)::GetPropA(hWnd, "IMGUI_VIEWPORT");
            if (viewport->Flags & ImGuiViewportFlags_NoInputs)
                return HTTRANSPARENT;
        }
        return ::CallWindowProc(bd->GlfwWndProc, hWnd, msg, wParam, lParam);
    }
#endif

    static void KarmaGui_ImplGlfw_ShowWindow(KarmaGuiViewport* viewport)
    {
        KarmaGui_ImplGlfw_ViewportData* vd = (KarmaGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;

#if defined(_WIN32)
        // GLFW hack: Hide icon from task bar
        HWND hwnd = (HWND)viewport->PlatformHandleRaw;
        if (viewport->Flags & KGGuiViewportFlags_NoTaskBarIcon)
        {
            LONG ex_style = ::GetWindowLong(hwnd, GWL_EXSTYLE);
            ex_style &= ~WS_EX_APPWINDOW;
            ex_style |= WS_EX_TOOLWINDOW;
            ::SetWindowLong(hwnd, GWL_EXSTYLE, ex_style);
        }

        // GLFW hack: install hook for WM_NCHITTEST message handler
#if !GLFW_HAS_MOUSE_PASSTHROUGH && GLFW_HAS_WINDOW_HOVERED && defined(_WIN32)
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        ::SetPropA(hwnd, "IMGUI_VIEWPORT", viewport);
        if (bd->GlfwWndProc == nullptr)
            bd->GlfwWndProc = (WNDPROC)::GetWindowLongPtr(hwnd, GWLP_WNDPROC);
        ::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProcNoInputs);
#endif

#if !GLFW_HAS_FOCUS_ON_SHOW
        // GLFW hack: GLFW 3.2 has a bug where glfwShowWindow() also activates/focus the window.
        // The fix was pushed to GLFW repository on 2018/01/09 and should be included in GLFW 3.3 via a GLFW_FOCUS_ON_SHOW window attribute.
        // See https://github.com/glfw/glfw/issues/1189
        // FIXME-VIEWPORT: Implement same work-around for Linux/OSX in the meanwhile.
        if (viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing)
        {
            ::ShowWindow(hwnd, SW_SHOWNA);
            return;
        }
#endif
#endif

        glfwShowWindow(vd->Window);
    }

    static KGVec2 KarmaGui_ImplGlfw_GetWindowPos(KarmaGuiViewport* viewport)
    {
        KarmaGui_ImplGlfw_ViewportData* vd = (KarmaGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
        int x = 0, y = 0;
        glfwGetWindowPos(vd->Window, &x, &y);
        return KGVec2((float)x, (float)y);
    }

    static void KarmaGui_ImplGlfw_SetWindowPos(KarmaGuiViewport* viewport, KGVec2 pos)
    {
        KarmaGui_ImplGlfw_ViewportData* vd = (KarmaGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
        vd->IgnoreWindowPosEventFrame = KarmaGui::GetFrameCount();
        glfwSetWindowPos(vd->Window, (int)pos.x, (int)pos.y);
    }

    static KGVec2 KarmaGui_ImplGlfw_GetWindowSize(KarmaGuiViewport* viewport)
    {
        KarmaGui_ImplGlfw_ViewportData* vd = (KarmaGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
        int w = 0, h = 0;
        glfwGetWindowSize(vd->Window, &w, &h);
        return KGVec2((float)w, (float)h);
    }

    static void KarmaGui_ImplGlfw_SetWindowSize(KarmaGuiViewport* viewport, KGVec2 size)
    {
        KarmaGui_ImplGlfw_ViewportData* vd = (KarmaGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
#if __APPLE__ && !GLFW_HAS_OSX_WINDOW_POS_FIX
        // Native OS windows are positioned from the bottom-left corner on macOS, whereas on other platforms they are
        // positioned from the upper-left corner. GLFW makes an effort to convert macOS style coordinates, however it
        // doesn't handle it when changing size. We are manually moving the window in order for changes of size to be based
        // on the upper-left corner.
        int x, y, width, height;
        glfwGetWindowPos(vd->Window, &x, &y);
        glfwGetWindowSize(vd->Window, &width, &height);
        glfwSetWindowPos(vd->Window, x, y - height + size.y);
#endif
        vd->IgnoreWindowSizeEventFrame = KarmaGui::GetFrameCount();
        glfwSetWindowSize(vd->Window, (int)size.x, (int)size.y);
    }

    static void KarmaGui_ImplGlfw_SetWindowTitle(KarmaGuiViewport* viewport, const char* title)
    {
        KarmaGui_ImplGlfw_ViewportData* vd = (KarmaGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
        glfwSetWindowTitle(vd->Window, title);
    }

    static void KarmaGui_ImplGlfw_SetWindowFocus(KarmaGuiViewport* viewport)
    {
#if GLFW_HAS_FOCUS_WINDOW
        KarmaGui_ImplGlfw_ViewportData* vd = (KarmaGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
        glfwFocusWindow(vd->Window);
#else
        // FIXME: What are the effect of not having this function? At the moment imgui doesn't actually call SetWindowFocus - we set that up ahead, will answer that question later.
        (void)viewport;
#endif
    }

    static bool KarmaGui_ImplGlfw_GetWindowFocus(KarmaGuiViewport* viewport)
    {
        KarmaGui_ImplGlfw_ViewportData* vd = (KarmaGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
        return glfwGetWindowAttrib(vd->Window, GLFW_FOCUSED) != 0;
    }

    static bool KarmaGui_ImplGlfw_GetWindowMinimized(KarmaGuiViewport* viewport)
    {
        KarmaGui_ImplGlfw_ViewportData* vd = (KarmaGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
        return glfwGetWindowAttrib(vd->Window, GLFW_ICONIFIED) != 0;
    }

#if GLFW_HAS_WINDOW_ALPHA
    static void KarmaGui_ImplGlfw_SetWindowAlpha(KarmaGuiViewport* viewport, float alpha)
    {
        KarmaGui_ImplGlfw_ViewportData* vd = (KarmaGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
        glfwSetWindowOpacity(vd->Window, alpha);
    }
#endif

    static void KarmaGui_ImplGlfw_RenderWindow(KarmaGuiViewport* viewport, void*)
    {
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        KarmaGui_ImplGlfw_ViewportData* vd = (KarmaGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
        if (bd->ClientApi == GlfwClientApi_OpenGL)
            glfwMakeContextCurrent(vd->Window);
    }

    static void KarmaGui_ImplGlfw_SwapBuffers(KarmaGuiViewport* viewport, void*)
    {
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        KarmaGui_ImplGlfw_ViewportData* vd = (KarmaGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
        if (bd->ClientApi == GlfwClientApi_OpenGL)
        {
            glfwMakeContextCurrent(vd->Window);
            glfwSwapBuffers(vd->Window);
        }
    }
}
//--------------------------------------------------------------------------------------------------------
// Vulkan support (the Vulkan renderer needs to call a platform-side support function to create the surface)
//--------------------------------------------------------------------------------------------------------

// Avoid including <vulkan.h> so we can build without it
#if GLFW_HAS_VULKAN
#ifndef VULKAN_H_
#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;
#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;
#else
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
#endif
VK_DEFINE_HANDLE(VkInstance)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)
struct VkAllocationCallbacks;
enum VkResult { VK_RESULT_MAX_ENUM = 0x7FFFFFFF };
#endif // VULKAN_H_
extern "C" { extern GLFWAPI VkResult glfwCreateWindowSurface(VkInstance instance, GLFWwindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface); }

namespace Karma
{

    static int KarmaGui_ImplGlfw_CreateVkSurface(KarmaGuiViewport* viewport, KGU64 vk_instance, const void* vk_allocator, KGU64* out_vk_surface)
    {
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        KarmaGui_ImplGlfw_ViewportData* vd = (KarmaGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
        KG_UNUSED(bd);
        KR_CORE_ASSERT(bd->ClientApi == GlfwClientApi_Vulkan, "");
        VkResult err = glfwCreateWindowSurface((VkInstance)vk_instance, vd->Window, (const VkAllocationCallbacks*)vk_allocator, (VkSurfaceKHR*)out_vk_surface);
        return (int)err;
    }
#endif // GLFW_HAS_VULKAN

    static void KarmaGui_ImplGlfw_InitPlatformInterface()
    {
        // Register platform interface (will be coupled with a renderer interface)
        KarmaGui_ImplGlfw_Data* bd = KarmaGui_ImplGlfw_GetBackendData();
        KarmaGuiPlatformIO& platform_io = KarmaGui::GetPlatformIO();
        platform_io.Platform_CreateWindow = KarmaGui_ImplGlfw_CreateWindow;
        platform_io.Platform_DestroyWindow = KarmaGui_ImplGlfw_DestroyWindow;
        platform_io.Platform_ShowWindow = KarmaGui_ImplGlfw_ShowWindow;
        platform_io.Platform_SetWindowPos = KarmaGui_ImplGlfw_SetWindowPos;
        platform_io.Platform_GetWindowPos = KarmaGui_ImplGlfw_GetWindowPos;
        platform_io.Platform_SetWindowSize = KarmaGui_ImplGlfw_SetWindowSize;
        platform_io.Platform_GetWindowSize = KarmaGui_ImplGlfw_GetWindowSize;
        platform_io.Platform_SetWindowFocus = KarmaGui_ImplGlfw_SetWindowFocus;
        platform_io.Platform_GetWindowFocus = KarmaGui_ImplGlfw_GetWindowFocus;
        platform_io.Platform_GetWindowMinimized = KarmaGui_ImplGlfw_GetWindowMinimized;
        platform_io.Platform_SetWindowTitle = KarmaGui_ImplGlfw_SetWindowTitle;
        platform_io.Platform_RenderWindow = KarmaGui_ImplGlfw_RenderWindow;
        platform_io.Platform_SwapBuffers = KarmaGui_ImplGlfw_SwapBuffers;
#if GLFW_HAS_WINDOW_ALPHA
        platform_io.Platform_SetWindowAlpha = KarmaGui_ImplGlfw_SetWindowAlpha;
#endif
#if GLFW_HAS_VULKAN
        platform_io.Platform_CreateVkSurface = KarmaGui_ImplGlfw_CreateVkSurface;
#endif

        // Register main window handle (which is owned by the main application, not by us)
        // This is mostly for simplicity and consistency, so that our code (e.g. mouse handling etc.) can use same logic for main and secondary viewports.
        KarmaGuiViewport* main_viewport = KarmaGui::GetMainViewport();
        KarmaGui_ImplGlfw_ViewportData* vd = KG_NEW(KarmaGui_ImplGlfw_ViewportData)();
        vd->Window = bd->Window;
        vd->WindowOwned = false;
        main_viewport->PlatformUserData = vd;
        main_viewport->PlatformHandle = (void*)bd->Window;
    }

    static void KarmaGui_ImplGlfw_ShutdownPlatformInterface()
    {
        KarmaGui::DestroyPlatformWindows();
    }
}


#if defined(__clang__)
#pragma clang diagnostic pop
#endif
