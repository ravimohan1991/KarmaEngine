
/*

Index of this file:

// [SECTION] Header mess
// [SECTION] Forward declarations
// [SECTION] Context pointer
// [SECTION] STB libraries includes
// [SECTION] Macros
// [SECTION] Generic helpers
// [SECTION] KGDrawList support
// [SECTION] Widgets support: flags, enums, data structures
// [SECTION] Inputs support
// [SECTION] Clipper support
// [SECTION] Navigation support
// [SECTION] Columns support
// [SECTION] Multi-select support
// [SECTION] Docking support
// [SECTION] Viewport support
// [SECTION] Settings support
// [SECTION] Localization support
// [SECTION] Metrics, Debug tools
// [SECTION] Generic context hooks
// [SECTION] KarmaGuiContext (main imgui context)
// [SECTION] KGGuiWindowTempData, KGGuiWindow
// [SECTION] Tab bar, Tab item support
// [SECTION] Table support
// [SECTION] ImGui internal API
// [SECTION] KGFontAtlas internal API
// [SECTION] Test Engine specific hooks (imgui_test_engine)

*/

#pragma once

#include "krpch.h"
#include "KarmaGui.h"

// Enable stb_truetype by default unless FreeType is enabled.
// You can compile with both by defining both KGGUI_ENABLE_FREETYPE and KGGUI_ENABLE_STB_TRUETYPE together.
#ifndef KGGUI_ENABLE_FREETYPE
#define KGGUI_ENABLE_STB_TRUETYPE
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations
//-----------------------------------------------------------------------------

struct KGBitVector;                 // Store 1-bit per value
struct KGRect;                      // An axis-aligned rectangle (2 points)
struct KGDrawDataBuilder;           // Helper to build a KGDrawData instance
struct KGDrawListSharedData;        // Data shared between all KGDrawList instances
struct KGGuiColorMod;               // Stacked color modifier, backup of modified data so we can restore it
struct KarmaGuiContext;                // Main Dear ImGui context
struct KGGuiContextHook;            // Hook for extensions like ImGuiTestEngine
struct KGGuiDataTypeInfo;           // Type information associated to a KarmaGuiDataType enum
struct KGGuiDockContext;            // Docking system context
struct KGGuiDockRequest;            // Docking system dock/undock queued request
struct KGGuiDockNode;               // Docking system node (hold a list of Windows OR two child dock nodes)
struct KGGuiDockNodeSettings;       // Storage for a dock node in .ini file (we preserve those even if the associated dock node isn't active during the session)
struct KGGuiGroupData;              // Stacked storage data for BeginGroup()/EndGroup()
struct KGGuiInputTextState;         // Internal state of the currently focused/edited text input box
struct KGGuiLastItemData;           // Status storage for last submitted items
struct KGGuiLocEntry;               // A localization entry.
struct KGGuiMenuColumns;            // Simple column measurement, currently used for MenuItem() only
struct KGGuiNavItemData;            // Result of a gamepad/keyboard directional navigation move query result
struct KGGuiMetricsConfig;          // Storage for ShowMetricsWindow() and DebugNodeXXX() functions
struct KGGuiNextWindowData;         // Storage for SetNextWindow** functions
struct KGGuiNextItemData;           // Storage for SetNextItem** functions
struct KGGuiOldColumnData;          // Storage data for a single column for legacy Columns() api
struct KGGuiOldColumns;             // Storage data for a columns set for legacy Columns() api
struct KGGuiPopupData;              // Storage for current popup stack
struct KGGuiSettingsHandler;        // Storage for one type registered in the .ini file
struct KGGuiStackSizes;             // Storage of stack sizes for debugging/asserting
struct KGGuiStyleMod;               // Stacked style modifier, backup of modified data so we can restore it
struct KGGuiTabBar;                 // Storage for a tab bar
struct KGGuiTabItem;                // Storage for a tab item (within a tab bar)
struct KGGuiTable;                  // Storage for a table
struct KGGuiTableColumn;            // Storage for one column of a table
struct KGGuiTableInstanceData;      // Storage for one instance of a same table
struct KGGuiTableTempData;          // Temporary storage for one table (one per table in the stack), shared between tables.
struct KGGuiTableSettings;          // Storage for a table .ini settings
struct KGGuiTableColumnsSettings;   // Storage for a column .ini settings
struct KGGuiWindow;                 // Storage for one window
struct KGGuiWindowTempData;         // Temporary storage for one window (that's the data which in theory we could ditch at the end of the frame, in practice we currently keep it for each window)
struct KGGuiWindowSettings;         // Storage for a window .ini settings (we keep one of those even if the actual window wasn't instanced during this session)

// Enumerations
// Use your programming IDE "Go to definition" facility on the names of the center columns to find the actual flags/enum lists.
enum KGGuiLocKey : int;                 // -> enum KGGuiLocKey              // Enum: a localization entry for translation.
typedef int KGGuiDataAuthority;         // -> enum KGGuiDataAuthority_      // Enum: for storing the source authority (dock node vs window) of a field
typedef int KGGuiLayoutType;            // -> enum KGGuiLayoutType_         // Enum: Horizontal or vertical

// Flags
typedef int KGGuiActivateFlags;         // -> enum KGGuiActivateFlags_      // Flags: for navigation/focus function (will be for ActivateItem() later)
typedef int KGGuiDebugLogFlags;         // -> enum KGGuiDebugLogFlags_      // Flags: for ShowDebugLogWindow(), g.DebugLogFlags
typedef int KGGuiItemFlags;             // -> enum KGGuiItemFlags_          // Flags: for PushItemFlag(), g.LastItemData.InFlags
typedef int KGGuiItemStatusFlags;       // -> enum KGGuiItemStatusFlags_    // Flags: for g.LastItemData.StatusFlags
typedef int KGGuiOldColumnFlags;        // -> enum KGGuiOldColumnFlags_     // Flags: for BeginColumns()
typedef int KGGuiNavHighlightFlags;     // -> enum KGGuiNavHighlightFlags_  // Flags: for RenderNavHighlight()
typedef int KGGuiNavMoveFlags;          // -> enum KGGuiNavMoveFlags_       // Flags: for navigation requests
typedef int KGGuiNextItemDataFlags;     // -> enum KGGuiNextItemDataFlags_  // Flags: for SetNextItemXXX() functions
typedef int KGGuiNextWindowDataFlags;   // -> enum KGGuiNextWindowDataFlags_// Flags: for SetNextWindowXXX() functions
typedef int KGGuiScrollFlags;           // -> enum KGGuiScrollFlags_        // Flags: for ScrollToItem() and navigation requests
typedef int KGGuiSeparatorFlags;        // -> enum KGGuiSeparatorFlags_     // Flags: for SeparatorEx()
typedef int KGGuiTextFlags;             // -> enum KGGuiTextFlags_          // Flags: for TextEx()
typedef int KGGuiTooltipFlags;          // -> enum KGGuiTooltipFlags_       // Flags: for BeginTooltipEx()

typedef void (*KGGuiErrorLogCallback)(void* user_data, const char* fmt, ...);

//-----------------------------------------------------------------------------
// [SECTION] Context pointer
// See implementation of this variable in imgui.cpp for comments and details.
//-----------------------------------------------------------------------------

#ifndef GKarmaGui
//extern  KarmaGuiContext* GKarmaGui;  // Current implicit context pointer
#endif

//-------------------------------------------------------------------------
// [SECTION] STB libraries includes
//-------------------------------------------------------------------------
namespace KGStb
{
	#undef STB_TEXTEDIT_STRING
	#undef STB_TEXTEDIT_CHARTYPE
	#define STB_TEXTEDIT_STRING             KGGuiInputTextState
	#define STB_TEXTEDIT_CHARTYPE           KGWchar
	#define STB_TEXTEDIT_GETWIDTH_NEWLINE   (-1.0f)
	#define STB_TEXTEDIT_UNDOSTATECOUNT     99
	#define STB_TEXTEDIT_UNDOCHARCOUNT      999
	#include "KarmaSTBTextEdit.h"
} // namespace KGStb

//-----------------------------------------------------------------------------
// [SECTION] Macros
//-----------------------------------------------------------------------------

// Internal Drag and Drop payload types. String starting with '_' are reserved for Dear KarmaGui.
#define KARMAGUI_PAYLOAD_TYPE_WINDOW       "_IMWINDOW"     // Payload == KGGuiWindow*

// Debug Logging for ShowDebugLogWindow(). This is designed for relatively rare events so please don't spam.
#ifndef KARMAGUI_DISABLE_DEBUG_TOOLS
#define KARMAGUI_DEBUG_LOG(...)            KR_CORE_INFO(...)
#else
#define KARMAGUI_DEBUG_LOG(...)
#endif
#define KARMAGUI_DEBUG_LOG_ACTIVEID(...)   do { if (g.DebugLogFlags & KGGuiDebugLogFlags_EventActiveId) KARMAGUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define KARMAGUI_DEBUG_LOG_FOCUS(...)      do { if (g.DebugLogFlags & KGGuiDebugLogFlags_EventFocus)    KARMAGUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define KARMAGUI_DEBUG_LOG_POPUP(...)      do { if (g.DebugLogFlags & KGGuiDebugLogFlags_EventPopup)    KARMAGUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define KARMAGUI_DEBUG_LOG_NAV(...)        do { if (g.DebugLogFlags & KGGuiDebugLogFlags_EventNav)      KARMAGUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define ye_right(...)    do { if (g.DebugLogFlags & KGGuiDebugLogFlags_EventClipper)  KARMAGUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define KARMAGUI_DEBUG_LOG_IO(...)         do { if (g.DebugLogFlags & KGGuiDebugLogFlags_EventIO)       KARMAGUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define KARMAGUI_DEBUG_LOG_DOCKING(...)    do { if (g.DebugLogFlags & KGGuiDebugLogFlags_EventDocking)  KARMAGUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define KARMAGUI_DEBUG_LOG_VIEWPORT(...)   do { if (g.DebugLogFlags & KGGuiDebugLogFlags_EventViewport) KARMAGUI_DEBUG_LOG(__VA_ARGS__); } while (0)

// Static Asserts
#define KG_STATIC_ASSERT(_COND)         KR_CORE_ASSERT(_COND, "")

// Error handling
// Down the line in some frameworks/languages we would like to have a way to redirect those to the programmer and recover from more faults.
#ifndef KG_ASSERT_USER_ERROR
#define KG_ASSERT_USER_ERROR(_EXP,_MSG) KR_CORE_ASSERT((_EXP), _MSG)   // Recoverable User Error
#endif

// Misc Macros
#define KG_PI                           3.14159265358979323846f
#ifdef KR_CORE_WINDOWS
#define KG_NEWLINE                      "\r\n"   // Play it nice with Windows users (Update: since 2018-05, Notepad finally appears to support Unix-style carriage returns!)
#else
#define KG_NEWLINE                      "\n"
#endif
#ifndef KG_TABSIZE                      // Until we move this to runtime and/or add proper tab support, at least allow users to compile-time override
#define KG_TABSIZE                      (4)
#endif
#define KG_MEMALIGN(_OFF,_ALIGN)        (((_OFF) + ((_ALIGN) - 1)) & ~((_ALIGN) - 1))           // Memory align e.g. IM_ALIGN(0,4)=0, IM_ALIGN(1,4)=4, IM_ALIGN(4,4)=4, IM_ALIGN(5,4)=8
#define KG_F32_TO_INT8_UNBOUND(_VAL)    ((int)((_VAL) * 255.0f + ((_VAL)>=0 ? 0.5f : -0.5f)))   // Unsaturated, for display purpose
#define KG_F32_TO_INT8_SAT(_VAL)        ((int)(KGSaturate(_VAL) * 255.0f + 0.5f))               // Saturated, always output 0..255
#define KG_FLOOR(_VAL)                  ((float)(int)(_VAL))                                    // KGFloor() is not inlined in MSVC debug builds
#define KG_ROUND(_VAL)                  ((float)(int)((_VAL) + 0.5f))                           //

// Enforce cdecl calling convention for functions called by the standard library, in case compilation settings changed the default to e.g. __vectorcall
// May encounter weired scenario when, in the future, Karma is being compiled on MSVC in Mac
#ifdef KR_CORE_WINDOWS
#define KARMAGUI_CDECL __cdecl
#else
#define KARMAGUI_CDECL
#endif

// Warnings
#if defined(_MSC_VER) && !defined(__clang__)
#define KG_MSVC_WARNING_SUPPRESS(XXXX)  __pragma(warning(suppress: XXXX))
#else
#define KG_MSVC_WARNING_SUPPRESS(XXXX)
#endif

//-----------------------------------------------------------------------------
// [SECTION] Generic helpers
// Note that the ImXXX helpers functions are lower-level than ImGui functions.
// ImGui functions or the ImGui context are never called/used from other ImXXX functions.
//-----------------------------------------------------------------------------
// - Helpers: Hashing
// - Helpers: Sorting
// - Helpers: Bit manipulation
// - Helpers: String
// - Helpers: Formatting
// - Helpers: UTF-8 <> wchar conversions
// - Helpers: KGVec2/KGVec4 operators
// - Helpers: Maths
// - Helpers: Geometry
// - Helper: KGVec1
// - Helper: KGVec2ih
// - Helper: KGRect
// - Helper: KGBitArray
// - Helper: KGBitVector
// - Helper: KGSpan<>, KGSpanAllocator<>
// - Helper: KGPool<>
// - Helper: KGChunkStream<>
// - Helper: KGGuiTextIndex
//-----------------------------------------------------------------------------

// Helpers: Hashing
 KGGuiID       KGHashData(const void* data, size_t data_size, KGU32 seed = 0);
 KGGuiID       KGHashStr(const char* data, size_t data_size = 0, KGU32 seed = 0);

// Helpers: Sorting
#ifndef KGQsort
static inline void      KGQsort(void* base, size_t count, size_t size_of_element, int(KARMAGUI_CDECL *compare_func)(void const*, void const*)) { if (count > 1) qsort(base, count, size_of_element, compare_func); }
#endif

// Helpers: Color Blending
 KGU32         KGAlphaBlendColors(KGU32 col_a, KGU32 col_b);

// Helpers: Bit manipulation
static inline bool      KGIsPowerOfTwo(int v)           { return v != 0 && (v & (v - 1)) == 0; }
static inline bool      KGIsPowerOfTwo(KGU64 v)         { return v != 0 && (v & (v - 1)) == 0; }
static inline int       KGUpperPowerOfTwo(int v)        { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v++; return v; }

// Helpers: String
 int           KGStricmp(const char* str1, const char* str2);
 int           KGStrnicmp(const char* str1, const char* str2, size_t count);
 void          KGStrncpy(char* dst, const char* src, size_t count);
 char*         KGStrdup(const char* str);
 char*         KGStrdupcpy(char* dst, size_t* p_dst_size, const char* str);
 const char*   KGStrchrRange(const char* str_begin, const char* str_end, char c);
 int           KGStrlenW(const KGWchar* str);
 const char*   KGStreolRange(const char* str, const char* str_end);                // End end-of-line
 const KGWchar*KGStrbolW(const KGWchar* buf_mid_line, const KGWchar* buf_begin);   // Find beginning-of-line
 const char*   KGStristr(const char* haystack, const char* haystack_end, const char* needle, const char* needle_end);
 void          KGStrTrimBlanks(char* str);
 const char*   KGStrSkipBlank(const char* str);

static inline char      KGToUpper(char c)               { return (c >= 'a' && c <= 'z') ? c &= ~32 : c; }
static inline bool      KGCharIsBlankA(char c)          { return c == ' ' || c == '\t'; }
static inline bool      KGCharIsBlankW(unsigned int c)  { return c == ' ' || c == '\t' || c == 0x3000; }


// Helpers: Formatting
 int           KGFormatString(char* buf, size_t buf_size, const char* fmt, ...) KG_FMTARGS(3);
 int           KGFormatStringV(char* buf, size_t buf_size, const char* fmt, va_list args) KG_FMTLIST(3);
 void          KGFormatStringToTempBuffer(const char** out_buf, const char** out_buf_end, const char* fmt, ...) KG_FMTARGS(3);
 void          KGFormatStringToTempBufferV(const char** out_buf, const char** out_buf_end, const char* fmt, va_list args) KG_FMTLIST(3);
 const char*   KGParseFormatFindStart(const char* format);
 const char*   KGParseFormatFindEnd(const char* format);
 const char*   KGParseFormatTrimDecorations(const char* format, char* buf, size_t buf_size);
 void          KGParseFormatSanitizeForPrinting(const char* fmt_in, char* fmt_out, size_t fmt_out_size);
 const char*   KGParseFormatSanitizeForScanning(const char* fmt_in, char* fmt_out, size_t fmt_out_size);
 int           KGParseFormatPrecision(const char* format, int default_value);

// Helpers: UTF-8 <> wchar conversions
 const char*   KGTextCharToUtf8(char out_buf[5], unsigned int c);                                                      // return out_buf
 int           KGTextStrToUtf8(char* out_buf, int out_buf_size, const KGWchar* in_text, const KGWchar* in_text_end);   // return output UTF-8 bytes count
 int           KGTextCharFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end);               // read one character. return input UTF-8 bytes count
 int           KGTextStrFromUtf8(KGWchar* out_buf, int out_buf_size, const char* in_text, const char* in_text_end, const char** in_remaining = NULL);   // return input UTF-8 bytes count
 int           KGTextCountCharsFromUtf8(const char* in_text, const char* in_text_end);                                 // return number of UTF-8 code-points (NOT bytes count)
 int           KGTextCountUtf8BytesFromChar(const char* in_text, const char* in_text_end);                             // return number of bytes to express one char in UTF-8
 int           KGTextCountUtf8BytesFromStr(const KGWchar* in_text, const KGWchar* in_text_end);                        // return number of bytes to express string in UTF-8

#ifndef KARMAGUI_DISABLE_DEFAULT_FILE_FUNCTIONS
typedef FILE* ImFileHandle;
 ImFileHandle      KGFileOpen(const char* filename, const char* mode);
 bool              KGFileClose(ImFileHandle file);
 KGU64             KGFileGetSize(ImFileHandle file);
 KGU64             KGFileRead(void* data, KGU64 size, KGU64 count, ImFileHandle file);
 KGU64             KGFileWrite(const void* data, KGU64 size, KGU64 count, ImFileHandle file);
#else
#define IMGUI_DISABLE_TTY_FUNCTIONS // Can't use stdout, fflush if we are not using default file functions
#endif
 void*             KGFileLoadToMemory(const char* filename, const char* mode, size_t* out_file_size = NULL, int padding_bytes = 0);

// Helpers: Maths
// - Wrapper for standard libs functions. Hmm how about normal usage?
#ifndef KARMAGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
#define KGFabs(X)           fabsf(X)
#define KGSqrt(X)           sqrtf(X)
#define KGFmod(X, Y)        fmodf((X), (Y))
#define KGCos(X)            cosf(X)
#define KGSin(X)            sinf(X)
#define KGAcos(X)           acosf(X)
#define KGAtan2(Y, X)       atan2f((Y), (X))
#define KGAtof(STR)         atof(STR)
//#define ImFloorStd(X)     floorf(X)           // We use our own, see KGFloor() and KGFloorSigned()
#define KGCeil(X)           ceilf(X)


 static inline KGVec2 operator*(const KGVec2& lhs, const float rhs)              { return KGVec2(lhs.x * rhs, lhs.y * rhs); }
 static inline KGVec2 operator/(const KGVec2& lhs, const float rhs)              { return KGVec2(lhs.x / rhs, lhs.y / rhs); }
 static inline KGVec2 operator+(const KGVec2& lhs, const KGVec2& rhs)            { return KGVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
 static inline KGVec2 operator-(const KGVec2& lhs, const KGVec2& rhs)            { return KGVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
 static inline KGVec2 operator*(const KGVec2& lhs, const KGVec2& rhs)            { return KGVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
 static inline KGVec2 operator/(const KGVec2& lhs, const KGVec2& rhs)            { return KGVec2(lhs.x / rhs.x, lhs.y / rhs.y); }
 static inline KGVec2& operator*=(KGVec2& lhs, const float rhs)                  { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
 static inline KGVec2& operator/=(KGVec2& lhs, const float rhs)                  { lhs.x /= rhs; lhs.y /= rhs; return lhs; }
 static inline KGVec2& operator+=(KGVec2& lhs, const KGVec2& rhs)                { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
 static inline KGVec2& operator-=(KGVec2& lhs, const KGVec2& rhs)                { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
 static inline KGVec2& operator*=(KGVec2& lhs, const KGVec2& rhs)                { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
 static inline KGVec2& operator/=(KGVec2& lhs, const KGVec2& rhs)                { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }
 static inline KGVec4 operator+(const KGVec4& lhs, const KGVec4& rhs)            { return KGVec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w); }
 static inline KGVec4 operator-(const KGVec4& lhs, const KGVec4& rhs)            { return KGVec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w); }
 static inline KGVec4 operator*(const KGVec4& lhs, const KGVec4& rhs)            { return KGVec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w); }

static inline float  KGPow(float x, float y)    { return powf(x, y); }          // DragBehaviorT/SliderBehaviorT uses KGPow with either float/double and need the precision
static inline double KGPow(double x, double y)  { return pow(x, y); }
static inline float  KGLog(float x)             { return logf(x); }             // DragBehaviorT/SliderBehaviorT uses KGLog with either float/double and need the precision
static inline double KGLog(double x)            { return log(x); }
static inline int    KGAbs(int x)               { return x < 0 ? -x : x; }
static inline float  KGAbs(float x)             { return fabsf(x); }
static inline double KGAbs(double x)            { return fabs(x); }
static inline float  KGSign(float x)            { return (x < 0.0f) ? -1.0f : (x > 0.0f) ? 1.0f : 0.0f; } // Sign operator - returns -1, 0 or 1 based on sign of argument
static inline double KGSign(double x)           { return (x < 0.0) ? -1.0 : (x > 0.0) ? 1.0 : 0.0; }
static inline float  KGRsqrt(float x)           { return 1.0f / sqrtf(x); }
static inline double KGRsqrt(double x)          { return 1.0 / sqrt(x); }
#endif

// - KGMin/KGMax/KGClamp/KGLerp/KGSwap are used by widgets which support variety of types: signed/unsigned int/long long float/double
// (Exceptionally using templates here but we could also redefine them for those types)
template<typename T> static inline T KGMin(T lhs, T rhs)                        { return lhs < rhs ? lhs : rhs; }
template<typename T> static inline T KGMax(T lhs, T rhs)                        { return lhs >= rhs ? lhs : rhs; }
template<typename T> static inline T KGClamp(T v, T mn, T mx)                   { return (v < mn) ? mn : (v > mx) ? mx : v; }
template<typename T> static inline T KGLerp(T a, T b, float t)                  { return (T)(a + (b - a) * t); }
template<typename T> static inline void KGSwap(T& a, T& b)                      { T tmp = a; a = b; b = tmp; }
template<typename T> static inline T KGAddClampOverflow(T a, T b, T mn, T mx)   { if (b < 0 && (a < mn - b)) return mn; if (b > 0 && (a > mx - b)) return mx; return a + b; }
template<typename T> static inline T KGSubClampOverflow(T a, T b, T mn, T mx)   { if (b > 0 && (a < mn + b)) return mn; if (b < 0 && (a > mx + b)) return mx; return a - b; }
// - Misc maths helpers
static inline KGVec2 KGMin(const KGVec2& lhs, const KGVec2& rhs)                { return KGVec2(lhs.x < rhs.x ? lhs.x : rhs.x, lhs.y < rhs.y ? lhs.y : rhs.y); }
static inline KGVec2 KGMax(const KGVec2& lhs, const KGVec2& rhs)                { return KGVec2(lhs.x >= rhs.x ? lhs.x : rhs.x, lhs.y >= rhs.y ? lhs.y : rhs.y); }
static inline KGVec2 KGClamp(const KGVec2& v, const KGVec2& mn, KGVec2 mx)      { return KGVec2((v.x < mn.x) ? mn.x : (v.x > mx.x) ? mx.x : v.x, (v.y < mn.y) ? mn.y : (v.y > mx.y) ? mx.y : v.y); }
static inline KGVec2 KGLerp(const KGVec2& a, const KGVec2& b, float t)          { return KGVec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t); }
static inline KGVec2 KGLerp(const KGVec2& a, const KGVec2& b, const KGVec2& t)  { return KGVec2(a.x + (b.x - a.x) * t.x, a.y + (b.y - a.y) * t.y); }
static inline KGVec4 KGLerp(const KGVec4& a, const KGVec4& b, float t)          { return KGVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t); }
static inline float  KGSaturate(float f)                                        { return (f < 0.0f) ? 0.0f : (f > 1.0f) ? 1.0f : f; }
static inline float  KGLengthSqr(const KGVec2& lhs)                             { return (lhs.x * lhs.x) + (lhs.y * lhs.y); }
static inline float  KGLengthSqr(const KGVec4& lhs)                             { return (lhs.x * lhs.x) + (lhs.y * lhs.y) + (lhs.z * lhs.z) + (lhs.w * lhs.w); }
static inline float  KGInvLength(const KGVec2& lhs, float fail_value)           { float d = (lhs.x * lhs.x) + (lhs.y * lhs.y); if (d > 0.0f) return KGRsqrt(d); return fail_value; }
static inline float  KGFloor(float f)                                           { return (float)(int)(f); }
static inline float  KGFloorSigned(float f)                                     { return (float)((f >= 0 || (float)(int)f == f) ? (int)f : (int)f - 1); } // Decent replacement for floorf()
static inline KGVec2 KGFloor(const KGVec2& v)                                   { return KGVec2((float)(int)(v.x), (float)(int)(v.y)); }
static inline KGVec2 KGFloorSigned(const KGVec2& v)                             { return KGVec2(KGFloorSigned(v.x), KGFloorSigned(v.y)); }
static inline int    KGModPositive(int a, int b)                                { return (a + b) % b; }
static inline float  KGDot(const KGVec2& a, const KGVec2& b)                    { return a.x * b.x + a.y * b.y; }
static inline KGVec2 KGRotate(const KGVec2& v, float cos_a, float sin_a)        { return KGVec2(v.x * cos_a - v.y * sin_a, v.x * sin_a + v.y * cos_a); }
static inline float  KGLinearSweep(float current, float target, float speed)    { if (current < target) return KGMin(current + speed, target); if (current > target) return KGMax(current - speed, target); return current; }
static inline KGVec2 KGMul(const KGVec2& lhs, const KGVec2& rhs)                { return KGVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
static inline bool   KGIsFloatAboveGuaranteedIntegerPrecision(float f)          { return f <= -16777216 || f >= 16777216; }
static inline float  KGExponentialMovingAverage(float avg, float sample, int n) { avg -= avg / n; avg += sample / n; return avg; }

// Helpers: Geometry
 KGVec2     KGBezierCubicCalc(const KGVec2& p1, const KGVec2& p2, const KGVec2& p3, const KGVec2& p4, float t);
 KGVec2     KGBezierCubicClosestPoint(const KGVec2& p1, const KGVec2& p2, const KGVec2& p3, const KGVec2& p4, const KGVec2& p, int num_segments);       // For curves with explicit number of segments
 KGVec2     KGBezierCubicClosestPointCasteljau(const KGVec2& p1, const KGVec2& p2, const KGVec2& p3, const KGVec2& p4, const KGVec2& p, float tess_tol);// For auto-tessellated curves you can use tess_tol = style.CurveTessellationTol
 KGVec2     KGBezierQuadraticCalc(const KGVec2& p1, const KGVec2& p2, const KGVec2& p3, float t);
 KGVec2     KGLineClosestPoint(const KGVec2& a, const KGVec2& b, const KGVec2& p);
 bool       KGTriangleContainsPoint(const KGVec2& a, const KGVec2& b, const KGVec2& c, const KGVec2& p);
 KGVec2     KGTriangleClosestPoint(const KGVec2& a, const KGVec2& b, const KGVec2& c, const KGVec2& p);
 void       KGTriangleBarycentricCoords(const KGVec2& a, const KGVec2& b, const KGVec2& c, const KGVec2& p, float& out_u, float& out_v, float& out_w);
inline float         KGTriangleArea(const KGVec2& a, const KGVec2& b, const KGVec2& c) { return KGFabs((a.x * (b.y - c.y)) + (b.x * (c.y - a.y)) + (c.x * (a.y - b.y))) * 0.5f; }
 KarmaGuiDir   KGGetDirQuadrantFromDelta(float dx, float dy);

// Helper: KGVec1 (1D vector)
// (this odd construct is used to facilitate the transition between 1D and 2D, and the maintenance of some branches/patches)

struct KGVec1
{
    float   x;
    constexpr KGVec1()         : x(0.0f) { }
    constexpr KGVec1(float _x) : x(_x) { }
};

// Helper: KGVec2ih (2D vector, half-size integer, for long-term packed storage)
struct KGVec2ih
{
    short   x, y;
    constexpr KGVec2ih()                           : x(0), y(0) {}
    constexpr KGVec2ih(short _x, short _y)         : x(_x), y(_y) {}
    constexpr explicit KGVec2ih(const KGVec2& rhs) : x((short)rhs.x), y((short)rhs.y) {}
};

// Helper: KGRect (2D axis aligned bounding-box)
// NB: we can't rely on KGVec2 math operators being available here!
struct  KGRect
{
    KGVec2      Min;    // Upper-left
    KGVec2      Max;    // Lower-right

    constexpr KGRect()                                        : Min(0.0f, 0.0f), Max(0.0f, 0.0f)  {}
    constexpr KGRect(const KGVec2& min, const KGVec2& max)    : Min(min), Max(max)                {}
    constexpr KGRect(const KGVec4& v)                         : Min(v.x, v.y), Max(v.z, v.w)      {}
    constexpr KGRect(float x1, float y1, float x2, float y2)  : Min(x1, y1), Max(x2, y2)          {}

    KGVec2      GetCenter() const                   { return KGVec2((Min.x + Max.x) * 0.5f, (Min.y + Max.y) * 0.5f); }
    KGVec2      GetSize() const                     { return KGVec2(Max.x - Min.x, Max.y - Min.y); }
    float       GetWidth() const                    { return Max.x - Min.x; }
    float       GetHeight() const                   { return Max.y - Min.y; }
    float       GetArea() const                     { return (Max.x - Min.x) * (Max.y - Min.y); }
    KGVec2      GetTL() const                       { return Min; }                   // Top-left
    KGVec2      GetTR() const                       { return KGVec2(Max.x, Min.y); }  // Top-right
    KGVec2      GetBL() const                       { return KGVec2(Min.x, Max.y); }  // Bottom-left
    KGVec2      GetBR() const                       { return Max; }                   // Bottom-right
    bool        Contains(const KGVec2& p) const     { return p.x     >= Min.x && p.y     >= Min.y && p.x     <  Max.x && p.y     <  Max.y; }
    bool        Contains(const KGRect& r) const     { return r.Min.x >= Min.x && r.Min.y >= Min.y && r.Max.x <= Max.x && r.Max.y <= Max.y; }
    bool        Overlaps(const KGRect& r) const     { return r.Min.y <  Max.y && r.Max.y >  Min.y && r.Min.x <  Max.x && r.Max.x >  Min.x; }
    void        Add(const KGVec2& p)                { if (Min.x > p.x)     Min.x = p.x;     if (Min.y > p.y)     Min.y = p.y;     if (Max.x < p.x)     Max.x = p.x;     if (Max.y < p.y)     Max.y = p.y; }
    void        Add(const KGRect& r)                { if (Min.x > r.Min.x) Min.x = r.Min.x; if (Min.y > r.Min.y) Min.y = r.Min.y; if (Max.x < r.Max.x) Max.x = r.Max.x; if (Max.y < r.Max.y) Max.y = r.Max.y; }
    void        Expand(const float amount)          { Min.x -= amount;   Min.y -= amount;   Max.x += amount;   Max.y += amount; }
    void        Expand(const KGVec2& amount)        { Min.x -= amount.x; Min.y -= amount.y; Max.x += amount.x; Max.y += amount.y; }
    void        Translate(const KGVec2& d)          { Min.x += d.x; Min.y += d.y; Max.x += d.x; Max.y += d.y; }
    void        TranslateX(float dx)                { Min.x += dx; Max.x += dx; }
    void        TranslateY(float dy)                { Min.y += dy; Max.y += dy; }
    void        ClipWith(const KGRect& r)           { Min = KGMax(Min, r.Min); Max = KGMin(Max, r.Max); }                   // Simple version, may lead to an inverted rectangle, which is fine for Contains/Overlaps test but not for display.
    void        ClipWithFull(const KGRect& r)       { Min = KGClamp(Min, r.Min, r.Max); Max = KGClamp(Max, r.Min, r.Max); } // Full version, ensure both points are fully clipped.
    void        Floor()                             { Min.x = KG_FLOOR(Min.x); Min.y = KG_FLOOR(Min.y); Max.x = KG_FLOOR(Max.x); Max.y = KG_FLOOR(Max.y); }
    bool        IsInverted() const                  { return Min.x > Max.x || Min.y > Max.y; }
    KGVec4      ToVec4() const                      { return KGVec4(Min.x, Min.y, Max.x, Max.y); }
};

// Helper: KGBitArray
inline bool     KGBitArrayTestBit(const KGU32* arr, int n)      { KGU32 mask = (KGU32)1 << (n & 31); return (arr[n >> 5] & mask) != 0; }
inline void     KGBitArrayClearBit(KGU32* arr, int n)           { KGU32 mask = (KGU32)1 << (n & 31); arr[n >> 5] &= ~mask; }
inline void     KGBitArraySetBit(KGU32* arr, int n)             { KGU32 mask = (KGU32)1 << (n & 31); arr[n >> 5] |= mask; }
inline void     KGBitArraySetBitRange(KGU32* arr, int n, int n2) // Works on range [n..n2)
{
    n2--;
    while (n <= n2)
    {
        int a_mod = (n & 31);
        int b_mod = (n2 > (n | 31) ? 31 : (n2 & 31)) + 1;
        KGU32 mask = (KGU32)(((KGU64)1 << b_mod) - 1) & ~(KGU32)(((KGU64)1 << a_mod) - 1);
        arr[n >> 5] |= mask;
        n = (n + 32) & ~31;
    }
}

// Helper: KGBitArray class (wrapper over KGBitArray functions)
// Store 1-bit per value.
template<int BITCOUNT, int OFFSET = 0>
struct KGBitArray
{
    KGU32           Storage[(BITCOUNT + 31) >> 5];
    KGBitArray()                                { ClearAllBits(); }
    void            ClearAllBits()              { memset(Storage, 0, sizeof(Storage)); }
    void            SetAllBits()                { memset(Storage, 255, sizeof(Storage)); }
    bool            TestBit(int n) const        { n += OFFSET; KR_CORE_ASSERT(n >= 0 && n < BITCOUNT, ""); return KGBitArrayTestBit(Storage, n); }
    void            SetBit(int n)               { n += OFFSET; KR_CORE_ASSERT(n >= 0 && n < BITCOUNT, ""); KGBitArraySetBit(Storage, n); }
    void            ClearBit(int n)             { n += OFFSET; KR_CORE_ASSERT(n >= 0 && n < BITCOUNT, ""); KGBitArrayClearBit(Storage, n); }
    void            SetBitRange(int n, int n2)  { n += OFFSET; n2 += OFFSET; KR_CORE_ASSERT(n >= 0 && n < BITCOUNT && n2 > n && n2 <= BITCOUNT, ""); KGBitArraySetBitRange(Storage, n, n2); } // Works on range [n..n2)
    bool            operator[](int n) const     { n += OFFSET; KR_CORE_ASSERT(n >= 0 && n < BITCOUNT, ""); return KGBitArrayTestBit(Storage, n); }
};

// Helper: KGBitVector
// Store 1-bit per value.
struct  KGBitVector
{
    KGVector<KGU32> Storage;
    void            Create(int sz)              { Storage.resize((sz + 31) >> 5); memset(Storage.Data, 0, (size_t)Storage.Size * sizeof(Storage.Data[0])); }
    void            Clear()                     { Storage.clear(); }
    bool            TestBit(int n) const        { KR_CORE_ASSERT(n < (Storage.Size << 5), ""); return KGBitArrayTestBit(Storage.Data, n); }
    void            SetBit(int n)               { KR_CORE_ASSERT(n < (Storage.Size << 5), ""); KGBitArraySetBit(Storage.Data, n); }
    void            ClearBit(int n)             { KR_CORE_ASSERT(n < (Storage.Size << 5), ""); KGBitArrayClearBit(Storage.Data, n); }
};

// Helper: KGSpan<>
// Pointing to a span of data we don't own.
template<typename T>
struct KGSpan
{
    T*                  Data;
    T*                  DataEnd;

    // Constructors, destructor
    inline KGSpan()                                 { Data = DataEnd = NULL; }
    inline KGSpan(T* data, int size)                { Data = data; DataEnd = data + size; }
    inline KGSpan(T* data, T* data_end)             { Data = data; DataEnd = data_end; }

    inline void         set(T* data, int size)      { Data = data; DataEnd = data + size; }
    inline void         set(T* data, T* data_end)   { Data = data; DataEnd = data_end; }
    inline int          size() const                { return (int)(ptrdiff_t)(DataEnd - Data); }
    inline int          size_in_bytes() const       { return (int)(ptrdiff_t)(DataEnd - Data) * (int)sizeof(T); }
    inline T&           operator[](int i)           { T* p = Data + i; KR_CORE_ASSERT(p >= Data && p < DataEnd, ""); return *p; }
    inline const T&     operator[](int i) const     { const T* p = Data + i; KR_CORE_ASSERT(p >= Data && p < DataEnd, ""); return *p; }

    inline T*           begin()                     { return Data; }
    inline const T*     begin() const               { return Data; }
    inline T*           end()                       { return DataEnd; }
    inline const T*     end() const                 { return DataEnd; }

    // Utilities
    inline int  index_from_ptr(const T* it) const   { KR_CORE_ASSERT(it >= Data && it < DataEnd, ""); const ptrdiff_t off = it - Data; return (int)off; }
};

// Helper: KGSpanAllocator<>
// Facilitate storing multiple chunks into a single large block (the "arena")
// - Usage: call Reserve() N times, allocate GetArenaSizeInBytes() worth, pass it to SetArenaBasePtr(), call GetSpan() N times to retrieve the aligned ranges.
template<int CHUNKS>
struct KGSpanAllocator
{
    char*   BasePtr;
    int     CurrOff;
    int     CurrIdx;
    int     Offsets[CHUNKS];
    int     Sizes[CHUNKS];

    KGSpanAllocator()                               { memset(this, 0, sizeof(*this)); }
    inline void  Reserve(int n, size_t sz, int a=4) { KR_CORE_ASSERT(n == CurrIdx && n < CHUNKS, ""); CurrOff = KG_MEMALIGN(CurrOff, a); Offsets[n] = CurrOff; Sizes[n] = (int)sz; CurrIdx++; CurrOff += (int)sz; }
    inline int   GetArenaSizeInBytes()              { return CurrOff; }
    inline void  SetArenaBasePtr(void* base_ptr)    { BasePtr = (char*)base_ptr; }
    inline void* GetSpanPtrBegin(int n)             { KR_CORE_ASSERT(n >= 0 && n < CHUNKS && CurrIdx == CHUNKS, ""); return (void*)(BasePtr + Offsets[n]); }
    inline void* GetSpanPtrEnd(int n)               { KR_CORE_ASSERT(n >= 0 && n < CHUNKS && CurrIdx == CHUNKS, ""); return (void*)(BasePtr + Offsets[n] + Sizes[n]); }
    template<typename T>
    inline void  GetSpan(int n, KGSpan<T>* span)    { span->set((T*)GetSpanPtrBegin(n), (T*)GetSpanPtrEnd(n)); }
};

// Helper: KGPool<>
// Basic keyed storage for contiguous instances, slow/amortized insertion, O(1) indexable, O(Log N) queries by ID over a dense/hot buffer,
// Honor constructor/destructor. Add/remove invalidate all pointers. Indexes have the same lifetime as the associated object.
typedef int KGPoolIdx;
template<typename T>
struct KGPool
{
    KGVector<T>     Buf;        // Contiguous data
    KarmaGuiStorage    Map;        // ID->Index
    KGPoolIdx       FreeIdx;    // Next free idx to use
    KGPoolIdx       AliveCount; // Number of active/alive items (for display purpose)

    KGPool()    { FreeIdx = AliveCount = 0; }
    ~KGPool()   { Clear(); }
    T*          GetByKey(KGGuiID key)               { int idx = Map.GetInt(key, -1); return (idx != -1) ? &Buf[idx] : NULL; }
    T*          GetByIndex(KGPoolIdx n)             { return &Buf[n]; }
    KGPoolIdx   GetIndex(const T* p) const          { KR_CORE_ASSERT(p >= Buf.Data && p < Buf.Data + Buf.Size, ""); return (KGPoolIdx)(p - Buf.Data); }
    T*          GetOrAddByKey(KGGuiID key)          { int* p_idx = Map.GetIntRef(key, -1); if (*p_idx != -1) return &Buf[*p_idx]; *p_idx = FreeIdx; return Add(); }
    bool        Contains(const T* p) const          { return (p >= Buf.Data && p < Buf.Data + Buf.Size); }
    void        Clear()                             { for (int n = 0; n < Map.Data.Size; n++) { int idx = Map.Data[n].val_i; if (idx != -1) Buf[idx].~T(); } Map.Clear(); Buf.clear(); FreeIdx = AliveCount = 0; }
    T*          Add()                               { int idx = FreeIdx; if (idx == Buf.Size) { Buf.resize(Buf.Size + 1); FreeIdx++; } else { FreeIdx = *(int*)&Buf[idx]; } KG_PLACEMENT_NEW(&Buf[idx]) T(); AliveCount++; return &Buf[idx]; }
    void        Remove(KGGuiID key, const T* p)     { Remove(key, GetIndex(p)); }
    void        Remove(KGGuiID key, KGPoolIdx idx)  { Buf[idx].~T(); *(int*)&Buf[idx] = FreeIdx; FreeIdx = idx; Map.SetInt(key, -1); AliveCount--; }
    void        Reserve(int capacity)               { Buf.reserve(capacity); Map.Data.reserve(capacity); }

    // To iterate a KGPool: for (int n = 0; n < pool.GetMapSize(); n++) if (T* t = pool.TryGetMapData(n)) { ... }
    // Can be avoided if you know .Remove() has never been called on the pool, or AliveCount == GetMapSize()
    int         GetAliveCount() const               { return AliveCount; }      // Number of active/alive items in the pool (for display purpose)
    int         GetBufSize() const                  { return Buf.Size; }
    int         GetMapSize() const                  { return Map.Data.Size; }   // It is the map we need iterate to find valid items, since we don't have "alive" storage anywhere
    T*          TryGetMapData(KGPoolIdx n)          { int idx = Map.Data[n].val_i; if (idx == -1) return NULL; return GetByIndex(idx); }
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    int         GetSize()                           { return GetMapSize(); } // For ImPlot: should use GetMapSize() from (IMGUI_VERSION_NUM >= 18304)
#endif
};

// Helper: KGChunkStream<>
// Build and iterate a contiguous stream of variable-sized structures.
// This is used by Settings to store persistent data while reducing allocation count.
// We store the chunk size first, and align the final size on 4 bytes boundaries.
// The tedious/zealous amount of casting is to avoid -Wcast-align warnings.
template<typename T>
struct KGChunkStream
{
    KGVector<char>  Buf;

    void    clear()                     { Buf.clear(); }
    bool    empty() const               { return Buf.Size == 0; }
    int     size() const                { return Buf.Size; }
    T*      alloc_chunk(size_t sz)      { size_t HDR_SZ = 4; sz = KG_MEMALIGN(HDR_SZ + sz, 4u); int off = Buf.Size; Buf.resize(off + (int)sz); ((int*)(void*)(Buf.Data + off))[0] = (int)sz; return (T*)(void*)(Buf.Data + off + (int)HDR_SZ); }
    T*      begin()                     { size_t HDR_SZ = 4; if (!Buf.Data) return NULL; return (T*)(void*)(Buf.Data + HDR_SZ); }
    T*      next_chunk(T* p)            { size_t HDR_SZ = 4; KR_CORE_ASSERT(p >= begin() && p < end(), ""); p = (T*)(void*)((char*)(void*)p + chunk_size(p)); if (p == (T*)(void*)((char*)end() + HDR_SZ)) return (T*)0; KR_CORE_ASSERT(p < end(), ""); return p; }
    int     chunk_size(const T* p)      { return ((const int*)p)[-1]; }
    T*      end()                       { return (T*)(void*)(Buf.Data + Buf.Size); }
    int     offset_from_ptr(const T* p) { KR_CORE_ASSERT(p >= begin() && p < end(), ""); const ptrdiff_t off = (const char*)p - Buf.Data; return (int)off; }
    T*      ptr_from_offset(int off)    { KR_CORE_ASSERT(off >= 4 && off < Buf.Size, ""); return (T*)(void*)(Buf.Data + off); }
    void    swap(KGChunkStream<T>& rhs) { rhs.Buf.swap(Buf); }

};

// Helper: KGGuiTextIndex<>
// Maintain a line index for a text buffer. This is a strong candidate to be moved into the public API.
struct KGGuiTextIndex
{
    KGVector<int>   LineOffsets;
    int             EndOffset = 0;                          // Because we don't own text buffer we need to maintain EndOffset (may bake in LineOffsets?)

    void            clear()                                 { LineOffsets.clear(); EndOffset = 0; }
    int             size()                                  { return LineOffsets.Size; }
    const char*     get_line_begin(const char* base, int n) { return base + LineOffsets[n]; }
    const char*     get_line_end(const char* base, int n)   { return base + (n + 1 < LineOffsets.Size ? (LineOffsets[n + 1] - 1) : EndOffset); }
    void            append(const char* base, int old_size, int new_size);
};

//-----------------------------------------------------------------------------
// [SECTION] KGDrawList support
//-----------------------------------------------------------------------------

// KGDrawList: Helper function to calculate a circle's segment count given its radius and a "maximum error" value.
// Estimation of number of circle segment based on error is derived using method described in https://stackoverflow.com/a/2244088/15194693
// Number of segments (N) is calculated using equation:
//   N = ceil ( pi / acos(1 - error / r) )     where r > 0, error <= r
// Our equation is significantly simpler that one in the post thanks for choosing segment that is
// perpendicular to X axis. Follow steps in the article from this starting condition and you will
// will get this result.
//
// Rendering circles with an odd number of segments, while mathematically correct will produce
// asymmetrical results on the raster grid. Therefore we're rounding N to next even number (7->8, 8->8, 9->10 etc.)
#define KG_ROUNDUP_TO_EVEN(_V)                                  ((((_V) + 1) / 2) * 2)
#define KG_DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN                     4
#define KG_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX                     512
#define KG_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(_RAD,_MAXERROR)    KGClamp(KG_ROUNDUP_TO_EVEN((int)KGCeil(KG_PI / KGAcos(1 - KGMin((_MAXERROR), (_RAD)) / (_RAD)))), KG_DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN, KG_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX)

// Raw equation from KG_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC rewritten for 'r' and 'error'.
#define KG_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_R(_N,_MAXERROR)    ((_MAXERROR) / (1 - KGCos(KG_PI / KGMax((float)(_N), KG_PI))))
#define KG_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_ERROR(_N,_RAD)     ((1 - KGCos(KG_PI / KGMax((float)(_N), KG_PI))) / (_RAD))

// KGDrawList: Lookup table size for adaptive arc drawing, cover full circle.
#ifndef KG_DRAWLIST_ARCFAST_TABLE_SIZE
#define KG_DRAWLIST_ARCFAST_TABLE_SIZE                          48 // Number of samples in lookup table.
#endif
#define IM_DRAWLIST_ARCFAST_SAMPLE_MAX                          KG_DRAWLIST_ARCFAST_TABLE_SIZE // Sample index _PathArcToFastEx() for 360 angle.

// Data shared between all KGDrawList instances
// You may want to create your own instance of this if you want to use KGDrawList completely without ImGui. In that case, watch out for future changes to this structure.
struct  KGDrawListSharedData
{
    KGVec2          TexUvWhitePixel;            // UV of white pixel in the atlas
    KGFont*         Font;                       // Current/default font (optional, for simplified AddText overload)
    float           FontSize;                   // Current/default font size (optional, for simplified AddText overload)
    float           CurveTessellationTol;       // Tessellation tolerance when using PathBezierCurveTo()
    float           CircleSegmentMaxError;      // Number of circle segments to use per pixel of radius for AddCircle() etc
    KGVec4          ClipRectFullscreen;         // Value for PushClipRectFullscreen()
    KGDrawListFlags InitialFlags;               // Initial flags at the beginning of the frame (it is possible to alter flags on a per-drawlist basis afterwards)

    // [Internal] Temp write buffer
    KGVector<KGVec2> TempBuffer;

    // [Internal] Lookup tables
    KGVec2          ArcFastVtx[KG_DRAWLIST_ARCFAST_TABLE_SIZE]; // Sample points on the quarter of the circle.
    float           ArcFastRadiusCutoff;                        // Cutoff radius after which arc drawing will fallback to slower PathArcTo()
    KGU8            CircleSegmentCounts[64];    // Precomputed segment count for given radius before we calculate it dynamically (to avoid calculation overhead)
    const KGVec4*   TexUvLines;                 // UV of anti-aliased lines in the atlas

    KGDrawListSharedData();
    void SetCircleTessellationMaxError(float max_error);
};

struct KGDrawDataBuilder
{
    KGVector<KGDrawList*>   Layers[2];           // Global layers for: regular, tooltip

    void Clear()                    { for (int n = 0; n < KG_ARRAYSIZE(Layers); n++) Layers[n].resize(0); }
    void ClearFreeMemory()          { for (int n = 0; n < KG_ARRAYSIZE(Layers); n++) Layers[n].clear(); }
    int  GetDrawListCount() const   { int count = 0; for (int n = 0; n < KG_ARRAYSIZE(Layers); n++) count += Layers[n].Size; return count; }
     void FlattenIntoSingleLayer();
};

//-----------------------------------------------------------------------------
// [SECTION] Widgets support: flags, enums, data structures
//-----------------------------------------------------------------------------

// Flags used by upcoming items
// - input: PushItemFlag() manipulates g.CurrentItemFlags, ItemAdd() calls may add extra flags.
// - output: stored in g.LastItemData.InFlags
// Current window shared by all windows.
// This is going to be exposed in imgui.h when stabilized enough.
enum KGGuiItemFlags_
{
    // Controlled by user
    KGGuiItemFlags_None                     = 0,
    KGGuiItemFlags_NoTabStop                = 1 << 0,  // false     // Disable keyboard tabbing (FIXME: should merge with _NoNav)
    KGGuiItemFlags_ButtonRepeat             = 1 << 1,  // false     // Button() will return true multiple times based on io.KeyRepeatDelay and io.KeyRepeatRate settings.
    KGGuiItemFlags_Disabled                 = 1 << 2,  // false     // Disable interactions but doesn't affect visuals. See BeginDisabled()/EndDisabled(). See github.com/ocornut/imgui/issues/211
    KGGuiItemFlags_NoNav                    = 1 << 3,  // false     // Disable keyboard/gamepad directional navigation (FIXME: should merge with _NoTabStop)
    KGGuiItemFlags_NoNavDefaultFocus        = 1 << 4,  // false     // Disable item being a candidate for default focus (e.g. used by title bar items)
    KGGuiItemFlags_SelectableDontClosePopup = 1 << 5,  // false     // Disable MenuItem/Selectable() automatically closing their popup window
    KGGuiItemFlags_MixedValue               = 1 << 6,  // false     // [BETA] Represent a mixed/indeterminate value, generally multi-selection where values differ. Currently only supported by Checkbox() (later should support all sorts of widgets)
    KGGuiItemFlags_ReadOnly                 = 1 << 7,  // false     // [ALPHA] Allow hovering interactions but underlying value is not changed.
    KGGuiItemFlags_NoWindowHoverableCheck   = 1 << 8,  // false     // Disable hoverable check in ItemHoverable()

    // Controlled by widget code
    KGGuiItemFlags_Inputable                = 1 << 10, // false     // [WIP] Auto-activate input mode when tab focused. Currently only used and supported by a few items before it becomes a generic feature.
};

// Status flags for an already submitted item
// - output: stored in g.LastItemData.StatusFlags
enum KGGuiItemStatusFlags_
{
    KGGuiItemStatusFlags_None               = 0,
    KGGuiItemStatusFlags_HoveredRect        = 1 << 0,   // Mouse position is within item rectangle (does NOT mean that the window is in correct z-order and can be hovered!, this is only one part of the most-common IsItemHovered test)
    KGGuiItemStatusFlags_HasDisplayRect     = 1 << 1,   // g.LastItemData.DisplayRect is valid
    KGGuiItemStatusFlags_Edited             = 1 << 2,   // Value exposed by item was edited in the current frame (should match the bool return value of most widgets)
    KGGuiItemStatusFlags_ToggledSelection   = 1 << 3,   // Set when Selectable(), TreeNode() reports toggling a selection. We can't report "Selected", only state changes, in order to easily handle clipping with less issues.
    KGGuiItemStatusFlags_ToggledOpen        = 1 << 4,   // Set when TreeNode() reports toggling their open state.
    KGGuiItemStatusFlags_HasDeactivated     = 1 << 5,   // Set if the widget/group is able to provide data for the KGGuiItemStatusFlags_Deactivated flag.
    KGGuiItemStatusFlags_Deactivated        = 1 << 6,   // Only valid if KGGuiItemStatusFlags_HasDeactivated is set.
    KGGuiItemStatusFlags_HoveredWindow      = 1 << 7,   // Override the HoveredWindow test to allow cross-window hover testing.
    KGGuiItemStatusFlags_FocusedByTabbing   = 1 << 8,   // Set when the Focusable item just got focused by Tabbing (FIXME: to be removed soon)
    KGGuiItemStatusFlags_Visible            = 1 << 9,   // [WIP] Set when item is overlapping the current clipping rectangle (Used internally. Please don't use yet: API/system will change as we refactor Itemadd()).
};

// Extend KGGuiInputTextFlags_
enum KGGuiInputTextFlagsPrivate_
{
    // [Internal]
    KGGuiInputTextFlags_Multiline           = 1 << 26,  // For internal use by InputTextMultiline()
    KGGuiInputTextFlags_NoMarkEdited        = 1 << 27,  // For internal use by functions using InputText() before reformatting data
    KGGuiInputTextFlags_MergedItem          = 1 << 28,  // For internal use by TempInputText(), will skip calling ItemAdd(). Require bounding-box to strictly match.
};

// Extend KGGuiButtonFlags_
enum KGGuiButtonFlagsPrivate_
{
    KGGuiButtonFlags_PressedOnClick         = 1 << 4,   // return true on click (mouse down event)
    KGGuiButtonFlags_PressedOnClickRelease  = 1 << 5,   // [Default] return true on click + release on same item <-- this is what the majority of Button are using
    KGGuiButtonFlags_PressedOnClickReleaseAnywhere = 1 << 6, // return true on click + release even if the release event is not done while hovering the item
    KGGuiButtonFlags_PressedOnRelease       = 1 << 7,   // return true on release (default requires click+release)
    KGGuiButtonFlags_PressedOnDoubleClick   = 1 << 8,   // return true on double-click (default requires click+release)
    KGGuiButtonFlags_PressedOnDragDropHold  = 1 << 9,   // return true when held into while we are drag and dropping another item (used by e.g. tree nodes, collapsing headers)
    KGGuiButtonFlags_Repeat                 = 1 << 10,  // hold to repeat
    KGGuiButtonFlags_FlattenChildren        = 1 << 11,  // allow interactions even if a child window is overlapping
    KGGuiButtonFlags_AllowItemOverlap       = 1 << 12,  // require previous frame HoveredId to either match id or be null before being usable, use along with SetItemAllowOverlap()
    KGGuiButtonFlags_DontClosePopups        = 1 << 13,  // disable automatically closing parent popup on press // [UNUSED]
    //KGGuiButtonFlags_Disabled             = 1 << 14,  // disable interactions -> use BeginDisabled() or KGGuiItemFlags_Disabled
    KGGuiButtonFlags_AlignTextBaseLine      = 1 << 15,  // vertically align button to match text baseline - ButtonEx() only // FIXME: Should be removed and handled by SmallButton(), not possible currently because of DC.CursorPosPrevLine
    KGGuiButtonFlags_NoKeyModifiers         = 1 << 16,  // disable mouse interaction if a key modifier is held
    KGGuiButtonFlags_NoHoldingActiveId      = 1 << 17,  // don't set ActiveId while holding the mouse (KGGuiButtonFlags_PressedOnClick only)
    KGGuiButtonFlags_NoNavFocus             = 1 << 18,  // don't override navigation focus when activated (FIXME: this is essentially used everytime an item uses KGGuiItemFlags_NoNav, but because legacy specs don't requires LastItemData to be set ButtonBehavior(), we can't poll g.LastItemData.InFlags)
    KGGuiButtonFlags_NoHoveredOnFocus       = 1 << 19,  // don't report as hovered when nav focus is on this item
    KGGuiButtonFlags_NoSetKeyOwner          = 1 << 20,  // don't set key/input owner on the initial click (note: mouse buttons are keys! often, the key in question will be KGGuiKey_MouseLeft!)
    KGGuiButtonFlags_NoTestKeyOwner         = 1 << 21,  // don't test key/input owner when polling the key (note: mouse buttons are keys! often, the key in question will be KGGuiKey_MouseLeft!)
    KGGuiButtonFlags_PressedOnMask_         = KGGuiButtonFlags_PressedOnClick | KGGuiButtonFlags_PressedOnClickRelease | KGGuiButtonFlags_PressedOnClickReleaseAnywhere | KGGuiButtonFlags_PressedOnRelease | KGGuiButtonFlags_PressedOnDoubleClick | KGGuiButtonFlags_PressedOnDragDropHold,
    KGGuiButtonFlags_PressedOnDefault_      = KGGuiButtonFlags_PressedOnClickRelease,
};

// Extend KGGuiComboFlags_
enum KGGuiComboFlagsPrivate_
{
    KGGuiComboFlags_CustomPreview           = 1 << 20,  // enable BeginComboPreview()
};

// Extend KGGuiSliderFlags_
enum KGGuiSliderFlagsPrivate_
{
    KGGuiSliderFlags_Vertical               = 1 << 20,  // Should this slider be orientated vertically?
    KGGuiSliderFlags_ReadOnly               = 1 << 21,
};

// Extend KGGuiSelectableFlags_
enum KGGuiSelectableFlagsPrivate_
{
    // NB: need to be in sync with last value of KGGuiSelectableFlags_
    KGGuiSelectableFlags_NoHoldingActiveID      = 1 << 20,
    KGGuiSelectableFlags_SelectOnNav            = 1 << 21,  // (WIP) Auto-select when moved into. This is not exposed in public API as to handle multi-select and modifiers we will need user to explicitly control focus scope. May be replaced with a BeginSelection() API.
    KGGuiSelectableFlags_SelectOnClick          = 1 << 22,  // Override button behavior to react on Click (default is Click+Release)
    KGGuiSelectableFlags_SelectOnRelease        = 1 << 23,  // Override button behavior to react on Release (default is Click+Release)
    KGGuiSelectableFlags_SpanAvailWidth         = 1 << 24,  // Span all avail width even if we declared less for layout purpose. FIXME: We may be able to remove this (added in 6251d379, 2bcafc86 for menus)
    KGGuiSelectableFlags_DrawHoveredWhenHeld    = 1 << 25,  // Always show active when held, even is not hovered. This concept could probably be renamed/formalized somehow.
    KGGuiSelectableFlags_SetNavIdOnHover        = 1 << 26,  // Set Nav/Focus ID on mouse hover (used by MenuItem)
    KGGuiSelectableFlags_NoPadWithHalfSpacing   = 1 << 27,  // Disable padding each side with ItemSpacing * 0.5f
    KGGuiSelectableFlags_NoSetKeyOwner          = 1 << 28,  // Don't set key/input owner on the initial click (note: mouse buttons are keys! often, the key in question will be KGGuiKey_MouseLeft!)
};

// Extend KGGuiTreeNodeFlags_
enum KGGuiTreeNodeFlagsPrivate_
{
    KGGuiTreeNodeFlags_ClipLabelForTrailingButton = 1 << 20,
};

enum KGGuiSeparatorFlags_
{
    KGGuiSeparatorFlags_None                    = 0,
    KGGuiSeparatorFlags_Horizontal              = 1 << 0,   // Axis default to current layout type, so generally Horizontal unless e.g. in a menu bar
    KGGuiSeparatorFlags_Vertical                = 1 << 1,
    KGGuiSeparatorFlags_SpanAllColumns          = 1 << 2,
};

enum KGGuiTextFlags_
{
    KGGuiTextFlags_None                         = 0,
    KGGuiTextFlags_NoWidthForLargeClippedText   = 1 << 0,
};

enum KGGuiTooltipFlags_
{
    KGGuiTooltipFlags_None                      = 0,
    KGGuiTooltipFlags_OverridePreviousTooltip   = 1 << 0,   // Override will clear/ignore previously submitted tooltip (defaults to append)
};

// FIXME: this is in development, not exposed/functional as a generic feature yet.
// Horizontal/Vertical enums are fixed to 0/1 so they may be used to index KGVec2
enum KGGuiLayoutType_
{
    KGGuiLayoutType_Horizontal = 0,
    KGGuiLayoutType_Vertical = 1
};

enum KGGuiLogType
{
    KGGuiLogType_None = 0,
    KGGuiLogType_TTY,
    KGGuiLogType_File,
    KGGuiLogType_Buffer,
    KGGuiLogType_Clipboard,
};

// X/Y enums are fixed to 0/1 so they may be used to index KGVec2
enum KGGuiAxis
{
    KGGuiAxis_None = -1,
    KGGuiAxis_X = 0,
    KGGuiAxis_Y = 1
};

enum KGGuiPlotType
{
    KGGuiPlotType_Lines,
    KGGuiPlotType_Histogram,
};

enum KGGuiPopupPositionPolicy
{
    KGGuiPopupPositionPolicy_Default,
    KGGuiPopupPositionPolicy_ComboBox,
    KGGuiPopupPositionPolicy_Tooltip,
};

struct KGGuiDataTypeTempStorage
{
    KGU8        Data[8];        // Can fit any data up to KGGuiDataType_COUNT
};

// Type information associated to one KarmaGuiDataType. Retrieve with DataTypeGetInfo().
struct KGGuiDataTypeInfo
{
    size_t      Size;           // Size in bytes
    const char* Name;           // Short descriptive name for the type, for debugging
    const char* PrintFmt;       // Default printf format for the type
    const char* ScanFmt;        // Default scanf format for the type
};

// Extend KGGuiDataType_
enum KGGuiDataTypePrivate_
{
    KGGuiDataType_String = KGGuiDataType_COUNT + 1,
    KGGuiDataType_Pointer,
    KGGuiDataType_ID,
};

// Stacked color modifier, backup of modified data so we can restore it
struct KGGuiColorMod
{
    KarmaGuiCol        Col;
    KGVec4          BackupValue;
};

// Stacked style modifier, backup of modified data so we can restore it. Data type inferred from the variable.
struct KGGuiStyleMod
{
    KarmaGuiStyleVar   VarIdx;
    union           { int BackupInt[2]; float BackupFloat[2]; };
    KGGuiStyleMod(KarmaGuiStyleVar idx, int v)     { VarIdx = idx; BackupInt[0] = v; }
    KGGuiStyleMod(KarmaGuiStyleVar idx, float v)   { VarIdx = idx; BackupFloat[0] = v; }
    KGGuiStyleMod(KarmaGuiStyleVar idx, KGVec2 v)  { VarIdx = idx; BackupFloat[0] = v.x; BackupFloat[1] = v.y; }
};

// Storage data for BeginComboPreview()/EndComboPreview()
struct  KGGuiComboPreviewData
{
    KGRect          PreviewRect;
    KGVec2          BackupCursorPos;
    KGVec2          BackupCursorMaxPos;
    KGVec2          BackupCursorPosPrevLine;
    float           BackupPrevLineTextBaseOffset;
    KGGuiLayoutType BackupLayout;

    KGGuiComboPreviewData() { memset(this, 0, sizeof(*this)); }
};

// Stacked storage data for BeginGroup()/EndGroup()
struct  KGGuiGroupData
{
    KGGuiID     WindowID;
    KGVec2      BackupCursorPos;
    KGVec2      BackupCursorMaxPos;
    KGVec1      BackupIndent;
    KGVec1      BackupGroupOffset;
    KGVec2      BackupCurrLineSize;
    float       BackupCurrLineTextBaseOffset;
    KGGuiID     BackupActiveIdIsAlive;
    bool        BackupActiveIdPreviousFrameIsAlive;
    bool        BackupHoveredIdIsAlive;
    bool        EmitItem;
};

// Simple column measurement, currently used for MenuItem() only.. This is very short-sighted/throw-away code and NOT a generic helper.
struct  KGGuiMenuColumns
{
    KGU32       TotalWidth;
    KGU32       NextTotalWidth;
    KGU16       Spacing;
    KGU16       OffsetIcon;         // Always zero for now
    KGU16       OffsetLabel;        // Offsets are locked in Update()
    KGU16       OffsetShortcut;
    KGU16       OffsetMark;
    KGU16       Widths[4];          // Width of:   Icon, Label, Shortcut, Mark  (accumulators for current frame)

    KGGuiMenuColumns() { memset(this, 0, sizeof(*this)); }
    void        Update(float spacing, bool window_reappearing);
    float       DeclColumns(float w_icon, float w_label, float w_shortcut, float w_mark);
    void        CalcNextTotalWidth(bool update_offsets);
};

// Internal state of the currently focused/edited text input box
// For a given item ID, access with ImGui::GetInputTextState()
struct  KGGuiInputTextState
{
    KarmaGuiContext*           Ctx;                    // parent dear imgui context
    KGGuiID                 ID;                     // widget id owning the text state
    int                     CurLenW, CurLenA;       // we need to maintain our buffer length in both UTF-8 and wchar format. UTF-8 length is valid even if TextA is not.
    KGVector<KGWchar>       TextW;                  // edit buffer, we need to persist but can't guarantee the persistence of the user-provided buffer. so we copy into own buffer.
    KGVector<char>          TextA;                  // temporary UTF8 buffer for callbacks and other operations. this is not updated in every code-path! size=capacity.
    KGVector<char>          InitialTextA;           // backup of end-user buffer at the time of focus (in UTF-8, unaltered)
    bool                    TextAIsValid;           // temporary UTF8 buffer is not initially valid before we make the widget active (until then we pull the data from user argument)
    int                     BufCapacityA;           // end-user buffer capacity
    float                   ScrollX;                // horizontal scrolling/offset
    KGStb::STB_TexteditState Stb;                   // state for stb_textedit.h
    float                   CursorAnim;             // timer for cursor blink, reset on every user action so the cursor reappears immediately
    bool                    CursorFollow;           // set when we want scrolling to follow the current cursor position (not always!)
    bool                    SelectedAllMouseLock;   // after a double-click to select all, we ignore further mouse drags to update selection
    bool                    Edited;                 // edited this frame
    KarmaGuiInputTextFlags     Flags;                  // copy of InputText() flags. may be used to check if e.g. KGGuiInputTextFlags_Password is set.

    KGGuiInputTextState(KarmaGuiContext* ctx)  { memset(this, 0, sizeof(*this)); Ctx = ctx;}
    void        ClearText()                 { CurLenW = CurLenA = 0; TextW[0] = 0; TextA[0] = 0; CursorClamp(); }
    void        ClearFreeMemory()           { TextW.clear(); TextA.clear(); InitialTextA.clear(); }
    int         GetUndoAvailCount() const   { return Stb.undostate.undo_point; }
    int         GetRedoAvailCount() const   { return STB_TEXTEDIT_UNDOSTATECOUNT - Stb.undostate.redo_point; }
    void        OnKeyPressed(int key);      // Cannot be inline because we call in code in stb_textedit.h implementation

    // Cursor & Selection
    void        CursorAnimReset()           { CursorAnim = -0.30f; }                                   // After a user-input the cursor stays on for a while without blinking
    void        CursorClamp()               { Stb.cursor = KGMin(Stb.cursor, CurLenW); Stb.select_start = KGMin(Stb.select_start, CurLenW); Stb.select_end = KGMin(Stb.select_end, CurLenW); }
    bool        HasSelection() const        { return Stb.select_start != Stb.select_end; }
    void        ClearSelection()            { Stb.select_start = Stb.select_end = Stb.cursor; }
    int         GetCursorPos() const        { return Stb.cursor; }
    int         GetSelectionStart() const   { return Stb.select_start; }
    int         GetSelectionEnd() const     { return Stb.select_end; }
    void        SelectAll()                 { Stb.select_start = 0; Stb.cursor = Stb.select_end = CurLenW; Stb.has_preferred_x = 0; }
};

// Storage for current popup stack
struct KGGuiPopupData
{
    KGGuiID             PopupId;        // Set on OpenPopup()
    KGGuiWindow*        Window;         // Resolved on BeginPopup() - may stay unresolved if user never calls OpenPopup()
    KGGuiWindow*        BackupNavWindow;// Set on OpenPopup(), a NavWindow that will be restored on popup close
    int                 ParentNavLayer; // Resolved on BeginPopup(). Actually a KGGuiNavLayer type (declared down below), initialized to -1 which is not part of an enum, but serves well-enough as "not any of layers" value
    int                 OpenFrameCount; // Set on OpenPopup()
    KGGuiID             OpenParentId;   // Set on OpenPopup(), we need this to differentiate multiple menu sets from each others (e.g. inside menu bar vs loose menu items)
    KGVec2              OpenPopupPos;   // Set on OpenPopup(), preferred popup position (typically == OpenMousePos when using mouse)
    KGVec2              OpenMousePos;   // Set on OpenPopup(), copy of mouse position at the time of opening popup

    KGGuiPopupData()    { memset(this, 0, sizeof(*this)); ParentNavLayer = OpenFrameCount = -1; }
};

enum KGGuiNextWindowDataFlags_
{
    KGGuiNextWindowDataFlags_None               = 0,
    KGGuiNextWindowDataFlags_HasPos             = 1 << 0,
    KGGuiNextWindowDataFlags_HasSize            = 1 << 1,
    KGGuiNextWindowDataFlags_HasContentSize     = 1 << 2,
    KGGuiNextWindowDataFlags_HasCollapsed       = 1 << 3,
    KGGuiNextWindowDataFlags_HasSizeConstraint  = 1 << 4,
    KGGuiNextWindowDataFlags_HasFocus           = 1 << 5,
    KGGuiNextWindowDataFlags_HasBgAlpha         = 1 << 6,
    KGGuiNextWindowDataFlags_HasScroll          = 1 << 7,
    KGGuiNextWindowDataFlags_HasViewport        = 1 << 8,
    KGGuiNextWindowDataFlags_HasDock            = 1 << 9,
    KGGuiNextWindowDataFlags_HasWindowClass     = 1 << 10,
};

// Storage for SetNexWindow** functions
struct KGGuiNextWindowData
{
    KGGuiNextWindowDataFlags    Flags;
    KarmaGuiCond                   PosCond;
    KarmaGuiCond                   SizeCond;
    KarmaGuiCond                   CollapsedCond;
    KarmaGuiCond                   DockCond;
    KGVec2                      PosVal;
    KGVec2                      PosPivotVal;
    KGVec2                      SizeVal;
    KGVec2                      ContentSizeVal;
    KGVec2                      ScrollVal;
    bool                        PosUndock;
    bool                        CollapsedVal;
    KGRect                      SizeConstraintRect;
    KarmaGuiSizeCallback           SizeCallback;
    void*                       SizeCallbackUserData;
    float                       BgAlphaVal;             // Override background alpha
    KGGuiID                     ViewportId;
    KGGuiID                     DockId;
    KarmaGuiWindowClass            WindowClass;
    KGVec2                      MenuBarOffsetMinVal;    // (Always on) This is not exposed publicly, so we don't clear it and it doesn't have a corresponding flag (could we? for consistency?)

    KGGuiNextWindowData()       { memset(this, 0, sizeof(*this)); }
    inline void ClearFlags()    { Flags = KGGuiNextWindowDataFlags_None; }
};

enum KGGuiNextItemDataFlags_
{
    KGGuiNextItemDataFlags_None     = 0,
    KGGuiNextItemDataFlags_HasWidth = 1 << 0,
    KGGuiNextItemDataFlags_HasOpen  = 1 << 1,
};

struct KGGuiNextItemData
{
    KGGuiNextItemDataFlags      Flags;
    float                       Width;          // Set by SetNextItemWidth()
    KGGuiID                     FocusScopeId;   // Set by SetNextItemMultiSelectData() (!= 0 signify value has been set, so it's an alternate version of HasSelectionData, we don't use Flags for this because they are cleared too early. This is mostly used for debugging)
    KarmaGuiCond                   OpenCond;
    bool                        OpenVal;        // Set by SetNextItemOpen()

    KGGuiNextItemData()         { memset(this, 0, sizeof(*this)); }
    inline void ClearFlags()    { Flags = KGGuiNextItemDataFlags_None; } // Also cleared manually by ItemAdd()!
};

// Status storage for the last submitted item
struct KGGuiLastItemData
{
    KGGuiID                 ID;
    KGGuiItemFlags          InFlags;            // See KGGuiItemFlags_
    KGGuiItemStatusFlags    StatusFlags;        // See KGGuiItemStatusFlags_
    KGRect                  Rect;               // Full rectangle
    KGRect                  NavRect;            // Navigation scoring rectangle (not displayed)
    KGRect                  DisplayRect;        // Display rectangle (only if KGGuiItemStatusFlags_HasDisplayRect is set)

    KGGuiLastItemData()     { memset(this, 0, sizeof(*this)); }
};

struct  KGGuiStackSizes
{
    short   SizeOfIDStack;
    short   SizeOfColorStack;
    short   SizeOfStyleVarStack;
    short   SizeOfFontStack;
    short   SizeOfFocusScopeStack;
    short   SizeOfGroupStack;
    short   SizeOfItemFlagsStack;
    short   SizeOfBeginPopupStack;
    short   SizeOfDisabledStack;

    KGGuiStackSizes() { memset(this, 0, sizeof(*this)); }
    void SetToCurrentState();
    void CompareWithCurrentState();
};

// Data saved for each window pushed into the stack
struct KGGuiWindowStackData
{
    KGGuiWindow*            Window;
    KGGuiLastItemData       ParentLastItemDataBackup;
    KGGuiStackSizes         StackSizesOnBegin;      // Store size of various stacks for asserting
};

struct KGGuiShrinkWidthItem
{
    int         Index;
    float       Width;
    float       InitialWidth;
};

struct KGGuiPtrOrIndex
{
    void*       Ptr;            // Either field can be set, not both. e.g. Dock node tab bars are loose while BeginTabBar() ones are in a pool.
    int         Index;          // Usually index in a main pool.

    KGGuiPtrOrIndex(void* ptr)  { Ptr = ptr; Index = -1; }
    KGGuiPtrOrIndex(int index)  { Ptr = NULL; Index = index; }
};

//-----------------------------------------------------------------------------
// [SECTION] Inputs support
//-----------------------------------------------------------------------------

typedef KGBitArray<KGGuiKey_NamedKey_COUNT, -KGGuiKey_NamedKey_BEGIN>    KGBitArrayForNamedKeys;

// [Internal] Key ranges
#define KGGuiKey_LegacyNativeKey_BEGIN  0
#define KGGuiKey_LegacyNativeKey_END    512
#define KGGuiKey_Keyboard_BEGIN         (KGGuiKey_NamedKey_BEGIN)
#define KGGuiKey_Keyboard_END           (KGGuiKey_GamepadStart)
#define KGGuiKey_Gamepad_BEGIN          (KGGuiKey_GamepadStart)
#define KGGuiKey_Gamepad_END            (KGGuiKey_GamepadRStickDown + 1)
#define KGGuiKey_Mouse_BEGIN            (KGGuiKey_MouseLeft)
#define KGGuiKey_Mouse_END              (KGGuiKey_MouseWheelY + 1)
#define KGGuiKey_Aliases_BEGIN          (KGGuiKey_Mouse_BEGIN)
#define KGGuiKey_Aliases_END            (KGGuiKey_Mouse_END)

// [Internal] Named shortcuts for Navigation
#define KGGuiKey_NavKeyboardTweakSlow   KGGuiMod_Ctrl
#define KGGuiKey_NavKeyboardTweakFast   KGGuiMod_Shift
#define KGGuiKey_NavGamepadTweakSlow    KGGuiKey_GamepadL1
#define KGGuiKey_NavGamepadTweakFast    KGGuiKey_GamepadR1
#define KGGuiKey_NavGamepadActivate     KGGuiKey_GamepadFaceDown
#define KGGuiKey_NavGamepadCancel       KGGuiKey_GamepadFaceRight
#define KGGuiKey_NavGamepadMenu         KGGuiKey_GamepadFaceLeft
#define KGGuiKey_NavGamepadInput        KGGuiKey_GamepadFaceUp

enum KGGuiInputEventType
{
    KGGuiInputEventType_None = 0,
    KGGuiInputEventType_MousePos,
    KGGuiInputEventType_MouseWheel,
    KGGuiInputEventType_MouseButton,
    KGGuiInputEventType_MouseViewport,
    KGGuiInputEventType_Key,
    KGGuiInputEventType_Text,
    KGGuiInputEventType_Focus,
    KGGuiInputEventType_COUNT
};

enum KGGuiInputSource
{
    KGGuiInputSource_None = 0,
    KGGuiInputSource_Mouse,
    KGGuiInputSource_Keyboard,
    KGGuiInputSource_Gamepad,
    KGGuiInputSource_Clipboard,     // Currently only used by InputText()
    KGGuiInputSource_Nav,           // Stored in g.ActiveIdSource only
    KGGuiInputSource_COUNT
};

// FIXME: Structures in the union below need to be declared as anonymous unions appears to be an extension?
// Using KGVec2() would fail on Clang 'union member 'MousePos' has a non-trivial default constructor'
struct KGGuiInputEventMousePos      { float PosX, PosY; };
struct KGGuiInputEventMouseWheel    { float WheelX, WheelY; };
struct KGGuiInputEventMouseButton   { int Button; bool Down; };
struct KGGuiInputEventMouseViewport { KGGuiID HoveredViewportID; };
struct KGGuiInputEventKey           { KarmaGuiKey Key; bool Down; float AnalogValue; };
struct KGGuiInputEventText          { unsigned int Char; };
struct KGGuiInputEventAppFocused    { bool Focused; };

struct KGGuiInputEvent
{
    KGGuiInputEventType             Type;
    KGGuiInputSource                Source;
    union
    {
        KGGuiInputEventMousePos     MousePos;       // if Type == KGGuiInputEventType_MousePos
        KGGuiInputEventMouseWheel   MouseWheel;     // if Type == KGGuiInputEventType_MouseWheel
        KGGuiInputEventMouseButton  MouseButton;    // if Type == KGGuiInputEventType_MouseButton
        KGGuiInputEventMouseViewport MouseViewport; // if Type == KGGuiInputEventType_MouseViewport
        KGGuiInputEventKey          Key;            // if Type == KGGuiInputEventType_Key
        KGGuiInputEventText         Text;           // if Type == KGGuiInputEventType_Text
        KGGuiInputEventAppFocused   AppFocused;     // if Type == KGGuiInputEventType_Focus
    };
    bool                            AddedByTestEngine;

    KGGuiInputEvent() { memset(this, 0, sizeof(*this)); }
};

// Input function taking an 'KGGuiID owner_id' argument defaults to (KGGuiKeyOwner_Any == 0) aka don't test ownership, which matches legacy behavior.
#define KGGuiKeyOwner_Any           ((KGGuiID)0)    // Accept key that have an owner, UNLESS a call to SetKeyOwner() explicitly used KGGuiInputFlags_LockThisFrame or KGGuiInputFlags_LockUntilRelease.
#define KGGuiKeyOwner_None          ((KGGuiID)-1)   // Require key to have no owner.

typedef KGS16 KGGuiKeyRoutingIndex;

// Routing table entry (sizeof() == 16 bytes)
struct KGGuiKeyRoutingData
{
    KGGuiKeyRoutingIndex            NextEntryIndex;
    KGU16                           Mods;               // Technically we'd only need 4-bits but for simplify we store KGGuiMod_ values which need 16-bits. KGGuiMod_Shortcut is already translated to Ctrl/Super.
    KGU8                            RoutingNextScore;   // Lower is better (0: perfect score)
    KGGuiID                         RoutingCurr;
    KGGuiID                         RoutingNext;

    KGGuiKeyRoutingData()           { NextEntryIndex = -1; Mods = 0; RoutingNextScore = 255; RoutingCurr = RoutingNext = KGGuiKeyOwner_None; }
};

// Routing table: maintain a desired owner for each possible key-chord (key + mods), and setup owner in NewFrame() when mods are matching.
// Stored in main context (1 instance)
struct KGGuiKeyRoutingTable
{
    KGGuiKeyRoutingIndex            Index[KGGuiKey_NamedKey_COUNT]; // Index of first entry in Entries[]
    KGVector<KGGuiKeyRoutingData>   Entries;
    KGVector<KGGuiKeyRoutingData>   EntriesNext;                    // Double-buffer to avoid reallocation (could use a shared buffer)

    KGGuiKeyRoutingTable()          { Clear(); }
    void Clear()                    { for (int n = 0; n < KG_ARRAYSIZE(Index); n++) Index[n] = -1; Entries.clear(); EntriesNext.clear(); }
};

// This extends KarmaGuiKeyData but only for named keys (legacy keys don't support the new features)
// Stored in main context (1 per named key). In the future it might be merged into KarmaGuiKeyData.
struct KGGuiKeyOwnerData
{
    KGGuiID     OwnerCurr;
    KGGuiID     OwnerNext;
    bool        LockThisFrame;      // Reading this key requires explicit owner id (until end of frame). Set by KGGuiInputFlags_LockThisFrame.
    bool        LockUntilRelease;   // Reading this key requires explicit owner id (until key is released). Set by KGGuiInputFlags_LockUntilRelease. When this is true LockThisFrame is always true as well.

    KGGuiKeyOwnerData()             { OwnerCurr = OwnerNext = KGGuiKeyOwner_None; LockThisFrame = LockUntilRelease = false; }
};

// Extend KGGuiInputFlags_
// Flags for extended versions of IsKeyPressed(), IsMouseClicked(), Shortcut(), SetKeyOwner(), SetItemKeyOwner()
// Don't mistake with KarmaGuiInputTextFlags! (for ImGui::InputText() function)
enum KGGuiInputFlagsPrivate_
{
    // Flags for IsKeyPressed(), IsMouseClicked(), Shortcut()
    KGGuiInputFlags_RepeatRateDefault   = 1 << 1,   // Repeat rate: Regular (default)
    KGGuiInputFlags_RepeatRateNavMove   = 1 << 2,   // Repeat rate: Fast
    KGGuiInputFlags_RepeatRateNavTweak  = 1 << 3,   // Repeat rate: Faster
    KGGuiInputFlags_RepeatRateMask_     = KGGuiInputFlags_RepeatRateDefault | KGGuiInputFlags_RepeatRateNavMove | KGGuiInputFlags_RepeatRateNavTweak,

    // Flags for SetItemKeyOwner()
    KGGuiInputFlags_CondHovered         = 1 << 4,   // Only set if item is hovered (default to both)
    KGGuiInputFlags_CondActive          = 1 << 5,   // Only set if item is active (default to both)
    KGGuiInputFlags_CondDefault_        = KGGuiInputFlags_CondHovered | KGGuiInputFlags_CondActive,
    KGGuiInputFlags_CondMask_           = KGGuiInputFlags_CondHovered | KGGuiInputFlags_CondActive,

    // Flags for SetKeyOwner(), SetItemKeyOwner()
    KGGuiInputFlags_LockThisFrame       = 1 << 6,   // Access to key data will require EXPLICIT owner ID (KGGuiKeyOwner_Any/0 will NOT accepted for polling). Cleared at end of frame. This is useful to make input-owner-aware code steal keys from non-input-owner-aware code.
    KGGuiInputFlags_LockUntilRelease    = 1 << 7,   // Access to key data will require EXPLICIT owner ID (KGGuiKeyOwner_Any/0 will NOT accepted for polling). Cleared when the key is released or at end of each frame if key is released. This is useful to make input-owner-aware code steal keys from non-input-owner-aware code.

    // [Internal] Mask of which function support which flags
    KGGuiInputFlags_RouteMask_                  = KGGuiInputFlags_RouteFocused | KGGuiInputFlags_RouteGlobal | KGGuiInputFlags_RouteGlobalLow | KGGuiInputFlags_RouteGlobalHigh, // _Always not part of this!
    KGGuiInputFlags_RouteExtraMask_             = KGGuiInputFlags_RouteAlways | KGGuiInputFlags_RouteUnlessBgFocused,
    KGGuiInputFlags_SupportedByIsKeyPressed     = KGGuiInputFlags_Repeat | KGGuiInputFlags_RepeatRateMask_,
    KGGuiInputFlags_SupportedByShortcut         = KGGuiInputFlags_Repeat | KGGuiInputFlags_RepeatRateMask_ | KGGuiInputFlags_RouteMask_ | KGGuiInputFlags_RouteExtraMask_,
    KGGuiInputFlags_SupportedBySetKeyOwner      = KGGuiInputFlags_LockThisFrame | KGGuiInputFlags_LockUntilRelease,
    KGGuiInputFlags_SupportedBySetItemKeyOwner  = KGGuiInputFlags_SupportedBySetKeyOwner | KGGuiInputFlags_CondMask_,
};

//-----------------------------------------------------------------------------
// [SECTION] Clipper support
//-----------------------------------------------------------------------------

struct KGGuiListClipperRange
{
    int     Min;
    int     Max;
    bool    PosToIndexConvert;      // Begin/End are absolute position (will be converted to indices later)
    KGS8    PosToIndexOffsetMin;    // Add to Min after converting to indices
    KGS8    PosToIndexOffsetMax;    // Add to Min after converting to indices

    static KGGuiListClipperRange    FromIndices(int min, int max)                               { KGGuiListClipperRange r = { min, max, false, 0, 0 }; return r; }
    static KGGuiListClipperRange    FromPositions(float y1, float y2, int off_min, int off_max) { KGGuiListClipperRange r = { (int)y1, (int)y2, true, (KGS8)off_min, (KGS8)off_max }; return r; }
};

// Temporary clipper data, buffers shared/reused between instances
struct KGGuiListClipperData
{
    KarmaGuiListClipper*               ListClipper;
    float                           LossynessOffset;
    int                             StepNo;
    int                             ItemsFrozen;
    KGVector<KGGuiListClipperRange> Ranges;

    KGGuiListClipperData()          { memset(this, 0, sizeof(*this)); }
    void                            Reset(KarmaGuiListClipper* clipper) { ListClipper = clipper; StepNo = ItemsFrozen = 0; Ranges.resize(0); }
};

//-----------------------------------------------------------------------------
// [SECTION] Navigation support
//-----------------------------------------------------------------------------

enum KGGuiActivateFlags_
{
    KGGuiActivateFlags_None                 = 0,
    KGGuiActivateFlags_PreferInput          = 1 << 0,       // Favor activation that requires keyboard text input (e.g. for Slider/Drag). Default if keyboard is available.
    KGGuiActivateFlags_PreferTweak          = 1 << 1,       // Favor activation for tweaking with arrows or gamepad (e.g. for Slider/Drag). Default if keyboard is not available.
    KGGuiActivateFlags_TryToPreserveState   = 1 << 2,       // Request widget to preserve state if it can (e.g. InputText will try to preserve cursor/selection)
};

// Early work-in-progress API for ScrollToItem()
enum KGGuiScrollFlags_
{
    KGGuiScrollFlags_None                   = 0,
    KGGuiScrollFlags_KeepVisibleEdgeX       = 1 << 0,       // If item is not visible: scroll as little as possible on X axis to bring item back into view [default for X axis]
    KGGuiScrollFlags_KeepVisibleEdgeY       = 1 << 1,       // If item is not visible: scroll as little as possible on Y axis to bring item back into view [default for Y axis for windows that are already visible]
    KGGuiScrollFlags_KeepVisibleCenterX     = 1 << 2,       // If item is not visible: scroll to make the item centered on X axis [rarely used]
    KGGuiScrollFlags_KeepVisibleCenterY     = 1 << 3,       // If item is not visible: scroll to make the item centered on Y axis
    KGGuiScrollFlags_AlwaysCenterX          = 1 << 4,       // Always center the result item on X axis [rarely used]
    KGGuiScrollFlags_AlwaysCenterY          = 1 << 5,       // Always center the result item on Y axis [default for Y axis for appearing window)
    KGGuiScrollFlags_NoScrollParent         = 1 << 6,       // Disable forwarding scrolling to parent window if required to keep item/rect visible (only scroll window the function was applied to).
    KGGuiScrollFlags_MaskX_                 = KGGuiScrollFlags_KeepVisibleEdgeX | KGGuiScrollFlags_KeepVisibleCenterX | KGGuiScrollFlags_AlwaysCenterX,
    KGGuiScrollFlags_MaskY_                 = KGGuiScrollFlags_KeepVisibleEdgeY | KGGuiScrollFlags_KeepVisibleCenterY | KGGuiScrollFlags_AlwaysCenterY,
};

enum KGGuiNavHighlightFlags_
{
    KGGuiNavHighlightFlags_None             = 0,
    KGGuiNavHighlightFlags_TypeDefault      = 1 << 0,
    KGGuiNavHighlightFlags_TypeThin         = 1 << 1,
    KGGuiNavHighlightFlags_AlwaysDraw       = 1 << 2,       // Draw rectangular highlight if (g.NavId == id) _even_ when using the mouse.
    KGGuiNavHighlightFlags_NoRounding       = 1 << 3,
};

enum KGGuiNavMoveFlags_
{
    KGGuiNavMoveFlags_None                  = 0,
    KGGuiNavMoveFlags_LoopX                 = 1 << 0,   // On failed request, restart from opposite side
    KGGuiNavMoveFlags_LoopY                 = 1 << 1,
    KGGuiNavMoveFlags_WrapX                 = 1 << 2,   // On failed request, request from opposite side one line down (when NavDir==right) or one line up (when NavDir==left)
    KGGuiNavMoveFlags_WrapY                 = 1 << 3,   // This is not super useful but provided for completeness
    KGGuiNavMoveFlags_AllowCurrentNavId     = 1 << 4,   // Allow scoring and considering the current NavId as a move target candidate. This is used when the move source is offset (e.g. pressing PageDown actually needs to send a Up move request, if we are pressing PageDown from the bottom-most item we need to stay in place)
    KGGuiNavMoveFlags_AlsoScoreVisibleSet   = 1 << 5,   // Store alternate result in NavMoveResultLocalVisible that only comprise elements that are already fully visible (used by PageUp/PageDown)
    KGGuiNavMoveFlags_ScrollToEdgeY         = 1 << 6,   // Force scrolling to min/max (used by Home/End) // FIXME-NAV: Aim to remove or reword, probably unnecessary
    KGGuiNavMoveFlags_Forwarded             = 1 << 7,
    KGGuiNavMoveFlags_DebugNoResult         = 1 << 8,   // Dummy scoring for debug purpose, don't apply result
    KGGuiNavMoveFlags_FocusApi              = 1 << 9,
    KGGuiNavMoveFlags_Tabbing               = 1 << 10,  // == Focus + Activate if item is Inputable + DontChangeNavHighlight
    KGGuiNavMoveFlags_Activate              = 1 << 11,
    KGGuiNavMoveFlags_DontSetNavHighlight   = 1 << 12,  // Do not alter the visible state of keyboard vs mouse nav highlight
};

enum KGGuiNavLayer
{
    KGGuiNavLayer_Main  = 0,    // Main scrolling layer
    KGGuiNavLayer_Menu  = 1,    // Menu layer (access with Alt)
    KGGuiNavLayer_COUNT
};

struct KGGuiNavItemData
{
    KGGuiWindow*        Window;         // Init,Move    // Best candidate window (result->ItemWindow->RootWindowForNav == request->Window)
    KGGuiID             ID;             // Init,Move    // Best candidate item ID
    KGGuiID             FocusScopeId;   // Init,Move    // Best candidate focus scope ID
    KGRect              RectRel;        // Init,Move    // Best candidate bounding box in window relative space
    KGGuiItemFlags      InFlags;        // ????,Move    // Best candidate item flags
    float               DistBox;        //      Move    // Best candidate box distance to current NavId
    float               DistCenter;     //      Move    // Best candidate center distance to current NavId
    float               DistAxial;      //      Move    // Best candidate axial distance to current NavId

    KGGuiNavItemData()  { Clear(); }
    void Clear()        { Window = NULL; ID = FocusScopeId = 0; InFlags = 0; DistBox = DistCenter = DistAxial = FLT_MAX; }
};

//-----------------------------------------------------------------------------
// [SECTION] Columns support
//-----------------------------------------------------------------------------

// Flags for internal's BeginColumns(). Prefix using BeginTable() nowadays!
enum KGGuiOldColumnFlags_
{
    KGGuiOldColumnFlags_None                    = 0,
    KGGuiOldColumnFlags_NoBorder                = 1 << 0,   // Disable column dividers
    KGGuiOldColumnFlags_NoResize                = 1 << 1,   // Disable resizing columns when clicking on the dividers
    KGGuiOldColumnFlags_NoPreserveWidths        = 1 << 2,   // Disable column width preservation when adjusting columns
    KGGuiOldColumnFlags_NoForceWithinWindow     = 1 << 3,   // Disable forcing columns to fit within window
    KGGuiOldColumnFlags_GrowParentContentsSize  = 1 << 4,   // (WIP) Restore pre-1.51 behavior of extending the parent window contents size but _without affecting the columns width at all_. Will eventually remove.
};

struct KGGuiOldColumnData
{
    float               OffsetNorm;             // Column start offset, normalized 0.0 (far left) -> 1.0 (far right)
    float               OffsetNormBeforeResize;
    KGGuiOldColumnFlags Flags;                  // Not exposed
    KGRect              ClipRect;

    KGGuiOldColumnData() { memset(this, 0, sizeof(*this)); }
};

struct KGGuiOldColumns
{
    KGGuiID             ID;
    KGGuiOldColumnFlags Flags;
    bool                IsFirstFrame;
    bool                IsBeingResized;
    int                 Current;
    int                 Count;
    float               OffMinX, OffMaxX;       // Offsets from HostWorkRect.Min.x
    float               LineMinY, LineMaxY;
    float               HostCursorPosY;         // Backup of CursorPos at the time of BeginColumns()
    float               HostCursorMaxPosX;      // Backup of CursorMaxPos at the time of BeginColumns()
    KGRect              HostInitialClipRect;    // Backup of ClipRect at the time of BeginColumns()
    KGRect              HostBackupClipRect;     // Backup of ClipRect during PushColumnsBackground()/PopColumnsBackground()
    KGRect              HostBackupParentWorkRect;//Backup of WorkRect at the time of BeginColumns()
    KGVector<KGGuiOldColumnData> Columns;
    KGDrawListSplitter  Splitter;

    KGGuiOldColumns()   { memset(this, 0, sizeof(*this)); }
};


//-----------------------------------------------------------------------------
// [SECTION] Docking support
//-----------------------------------------------------------------------------

#define DOCKING_HOST_DRAW_CHANNEL_BG 0  // Dock host: background fill
#define DOCKING_HOST_DRAW_CHANNEL_FG 1  // Dock host: decorations and contents

#ifdef KARMAGUI_HAS_DOCK

// Extend KGGuiDockNodeFlags_
enum KGGuiDockNodeFlagsPrivate_
{
    // [Internal]
    KGGuiDockNodeFlags_DockSpace                = 1 << 10,  // Local, Saved  // A dockspace is a node that occupy space within an existing user window. Otherwise the node is floating and create its own window.
    KGGuiDockNodeFlags_CentralNode              = 1 << 11,  // Local, Saved  // The central node has 2 main properties: stay visible when empty, only use "remaining" spaces from its neighbor.
    KGGuiDockNodeFlags_NoTabBar                 = 1 << 12,  // Local, Saved  // Tab bar is completely unavailable. No triangle in the corner to enable it back.
    KGGuiDockNodeFlags_HiddenTabBar             = 1 << 13,  // Local, Saved  // Tab bar is hidden, with a triangle in the corner to show it again (NB: actual tab-bar instance may be destroyed as this is only used for single-window tab bar)
    KGGuiDockNodeFlags_NoWindowMenuButton       = 1 << 14,  // Local, Saved  // Disable window/docking menu (that one that appears instead of the collapse button)
    KGGuiDockNodeFlags_NoCloseButton            = 1 << 15,  // Local, Saved  //
    KGGuiDockNodeFlags_NoDocking                = 1 << 16,  // Local, Saved  // Disable any form of docking in this dockspace or individual node. (On a whole dockspace, this pretty much defeat the purpose of using a dockspace at all). Note: when turned on, existing docked nodes will be preserved.
    KGGuiDockNodeFlags_NoDockingSplitMe         = 1 << 17,  // [EXPERIMENTAL] Prevent another window/node from splitting this node.
    KGGuiDockNodeFlags_NoDockingSplitOther      = 1 << 18,  // [EXPERIMENTAL] Prevent this node from splitting another window/node.
    KGGuiDockNodeFlags_NoDockingOverMe          = 1 << 19,  // [EXPERIMENTAL] Prevent another window/node to be docked over this node.
    KGGuiDockNodeFlags_NoDockingOverOther       = 1 << 20,  // [EXPERIMENTAL] Prevent this node to be docked over another window or non-empty node.
    KGGuiDockNodeFlags_NoDockingOverEmpty       = 1 << 21,  // [EXPERIMENTAL] Prevent this node to be docked over an empty node (e.g. DockSpace with no other windows)
    KGGuiDockNodeFlags_NoResizeX                = 1 << 22,  // [EXPERIMENTAL]
    KGGuiDockNodeFlags_NoResizeY                = 1 << 23,  // [EXPERIMENTAL]
    KGGuiDockNodeFlags_SharedFlagsInheritMask_  = ~0,
    KGGuiDockNodeFlags_NoResizeFlagsMask_       = KGGuiDockNodeFlags_NoResize | KGGuiDockNodeFlags_NoResizeX | KGGuiDockNodeFlags_NoResizeY,
    KGGuiDockNodeFlags_LocalFlagsMask_          = KGGuiDockNodeFlags_NoSplit | KGGuiDockNodeFlags_NoResizeFlagsMask_ | KGGuiDockNodeFlags_AutoHideTabBar | KGGuiDockNodeFlags_DockSpace | KGGuiDockNodeFlags_CentralNode | KGGuiDockNodeFlags_NoTabBar | KGGuiDockNodeFlags_HiddenTabBar | KGGuiDockNodeFlags_NoWindowMenuButton | KGGuiDockNodeFlags_NoCloseButton | KGGuiDockNodeFlags_NoDocking,
    KGGuiDockNodeFlags_LocalFlagsTransferMask_  = KGGuiDockNodeFlags_LocalFlagsMask_ & ~KGGuiDockNodeFlags_DockSpace,  // When splitting those flags are moved to the inheriting child, never duplicated
    KGGuiDockNodeFlags_SavedFlagsMask_          = KGGuiDockNodeFlags_NoResizeFlagsMask_ | KGGuiDockNodeFlags_DockSpace | KGGuiDockNodeFlags_CentralNode | KGGuiDockNodeFlags_NoTabBar | KGGuiDockNodeFlags_HiddenTabBar | KGGuiDockNodeFlags_NoWindowMenuButton | KGGuiDockNodeFlags_NoCloseButton | KGGuiDockNodeFlags_NoDocking
};

// Store the source authority (dock node vs window) of a field
enum KGGuiDataAuthority_
{
    KGGuiDataAuthority_Auto,
    KGGuiDataAuthority_DockNode,
    KGGuiDataAuthority_Window,
};

enum KGGuiDockNodeState
{
    KGGuiDockNodeState_Unknown,
    KGGuiDockNodeState_HostWindowHiddenBecauseSingleWindow,
    KGGuiDockNodeState_HostWindowHiddenBecauseWindowsAreResizing,
    KGGuiDockNodeState_HostWindowVisible,
};

// sizeof() 156~192
struct  KGGuiDockNode
{
    KGGuiID                 ID;
    KarmaGuiDockNodeFlags      SharedFlags;                // (Write) Flags shared by all nodes of a same dockspace hierarchy (inherited from the root node)
    KarmaGuiDockNodeFlags      LocalFlags;                 // (Write) Flags specific to this node
    KarmaGuiDockNodeFlags      LocalFlagsInWindows;        // (Write) Flags specific to this node, applied from windows
    KarmaGuiDockNodeFlags      MergedFlags;                // (Read)  Effective flags (== SharedFlags | LocalFlagsInNode | LocalFlagsInWindows)
    KGGuiDockNodeState      State;
    KGGuiDockNode*          ParentNode;
    KGGuiDockNode*          ChildNodes[2];              // [Split node only] Child nodes (left/right or top/bottom). Consider switching to an array.
    KGVector<KGGuiWindow*>  Windows;                    // Note: unordered list! Iterate TabBar->Tabs for user-order.
    KGGuiTabBar*            TabBar;
    KGVec2                  Pos;                        // Current position
    KGVec2                  Size;                       // Current size
    KGVec2                  SizeRef;                    // [Split node only] Last explicitly written-to size (overridden when using a splitter affecting the node), used to calculate Size.
    KGGuiAxis               SplitAxis;                  // [Split node only] Split axis (X or Y)
    KarmaGuiWindowClass        WindowClass;                // [Root node only]
    KGU32                   LastBgColor;

    KGGuiWindow*            HostWindow;
    KGGuiWindow*            VisibleWindow;              // Generally point to window which is ID is == SelectedTabID, but when CTRL+Tabbing this can be a different window.
    KGGuiDockNode*          CentralNode;                // [Root node only] Pointer to central node.
    KGGuiDockNode*          OnlyNodeWithWindows;        // [Root node only] Set when there is a single visible node within the hierarchy.
    int                     CountNodeWithWindows;       // [Root node only]
    int                     LastFrameAlive;             // Last frame number the node was updated or kept alive explicitly with DockSpace() + KGGuiDockNodeFlags_KeepAliveOnly
    int                     LastFrameActive;            // Last frame number the node was updated.
    int                     LastFrameFocused;           // Last frame number the node was focused.
    KGGuiID                 LastFocusedNodeId;          // [Root node only] Which of our child docking node (any ancestor in the hierarchy) was last focused.
    KGGuiID                 SelectedTabId;              // [Leaf node only] Which of our tab/window is selected.
    KGGuiID                 WantCloseTabId;             // [Leaf node only] Set when closing a specific tab/window.
    KGGuiDataAuthority      AuthorityForPos         :3;
    KGGuiDataAuthority      AuthorityForSize        :3;
    KGGuiDataAuthority      AuthorityForViewport    :3;
    bool                    IsVisible               :1; // Set to false when the node is hidden (usually disabled as it has no active window)
    bool                    IsFocused               :1;
    bool                    IsBgDrawnThisFrame      :1;
    bool                    HasCloseButton          :1; // Provide space for a close button (if any of the docked window has one). Note that button may be hidden on window without one.
    bool                    HasWindowMenuButton     :1;
    bool                    HasCentralNodeChild     :1;
    bool                    WantCloseAll            :1; // Set when closing all tabs at once.
    bool                    WantLockSizeOnce        :1;
    bool                    WantMouseMove           :1; // After a node extraction we need to transition toward moving the newly created host window
    bool                    WantHiddenTabBarUpdate  :1;
    bool                    WantHiddenTabBarToggle  :1;

    KGGuiDockNode(KGGuiID id);
    ~KGGuiDockNode();
    bool                    IsRootNode() const      { return ParentNode == NULL; }
    bool                    IsDockSpace() const     { return (MergedFlags & KGGuiDockNodeFlags_DockSpace) != 0; }
    bool                    IsFloatingNode() const  { return ParentNode == NULL && (MergedFlags & KGGuiDockNodeFlags_DockSpace) == 0; }
    bool                    IsCentralNode() const   { return (MergedFlags & KGGuiDockNodeFlags_CentralNode) != 0; }
    bool                    IsHiddenTabBar() const  { return (MergedFlags & KGGuiDockNodeFlags_HiddenTabBar) != 0; } // Hidden tab bar can be shown back by clicking the small triangle
    bool                    IsNoTabBar() const      { return (MergedFlags & KGGuiDockNodeFlags_NoTabBar) != 0; }     // Never show a tab bar
    bool                    IsSplitNode() const     { return ChildNodes[0] != NULL; }
    bool                    IsLeafNode() const      { return ChildNodes[0] == NULL; }
    bool                    IsEmpty() const         { return ChildNodes[0] == NULL && Windows.Size == 0; }
    KGRect                  Rect() const            { return KGRect(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y); }

    void                    SetLocalFlags(KarmaGuiDockNodeFlags flags) { LocalFlags = flags; UpdateMergedFlags(); }
    void                    UpdateMergedFlags()     { MergedFlags = SharedFlags | LocalFlags | LocalFlagsInWindows; }
};

// List of colors that are stored at the time of Begin() into Docked Windows.
// We currently store the packed colors in a simple array window->DockStyle.Colors[].
// A better solution may involve appending into a log of colors in KarmaGuiContext + store offsets into those arrays in KGGuiWindow,
// but it would be more complex as we'd need to double-buffer both as e.g. drop target may refer to window from last frame.
enum KGGuiWindowDockStyleCol
{
    KGGuiWindowDockStyleCol_Text,
    KGGuiWindowDockStyleCol_Tab,
    KGGuiWindowDockStyleCol_TabHovered,
    KGGuiWindowDockStyleCol_TabActive,
    KGGuiWindowDockStyleCol_TabUnfocused,
    KGGuiWindowDockStyleCol_TabUnfocusedActive,
    KGGuiWindowDockStyleCol_COUNT
};

struct KGGuiWindowDockStyle
{
    KGU32 Colors[KGGuiWindowDockStyleCol_COUNT];
};

struct KGGuiDockContext
{
    KarmaGuiStorage                    Nodes;          // Map ID -> KGGuiDockNode*: Active nodes
    KGVector<KGGuiDockRequest>      Requests;
    KGVector<KGGuiDockNodeSettings> NodesSettings;
    bool                            WantFullRebuild;
    KGGuiDockContext()              { memset(this, 0, sizeof(*this)); }
};

#endif // #ifdef KARMAGUI_HAS_DOCK

//-----------------------------------------------------------------------------
// [SECTION] Viewport support
//-----------------------------------------------------------------------------

// KarmaGuiViewport Private/Internals fields (cardinal sin: we are using inheritance!)
// Every instance of KarmaGuiViewport is in fact a KGGuiViewportP.
struct KGGuiViewportP : public KarmaGuiViewport
{
    int                 Idx;
    int                 LastFrameActive;        // Last frame number this viewport was activated by a window
    int                 LastFrontMostStampCount;// Last stamp number from when a window hosted by this viewport was made front-most (by comparing this value between two viewport we have an implicit viewport z-order
    KGGuiID             LastNameHash;
    KGVec2              LastPos;
    float               Alpha;                  // Window opacity (when dragging dockable windows/viewports we make them transparent)
    float               LastAlpha;
    short               PlatformMonitor;
    KGGuiWindow*        Window;                 // Set when the viewport is owned by a window (and KGGuiViewportFlags_CanHostOtherWindows is NOT set)
    int                 DrawListsLastFrame[2];  // Last frame number the background (0) and foreground (1) draw lists were used
    KGDrawList*         DrawLists[2];           // Convenience background (0) and foreground (1) draw lists. We use them to draw software mouser cursor when io.MouseDrawCursor is set and to draw most debug overlays.
    KGDrawData          DrawDataP;
    KGDrawDataBuilder   DrawDataBuilder;
    KGVec2              LastPlatformPos;
    KGVec2              LastPlatformSize;
    KGVec2              LastRendererSize;
    KGVec2              WorkOffsetMin;          // Work Area: Offset from Pos to top-left corner of Work Area. Generally (0,0) or (0,+main_menu_bar_height). Work Area is Full Area but without menu-bars/status-bars (so WorkArea always fit inside Pos/Size!)
    KGVec2              WorkOffsetMax;          // Work Area: Offset from Pos+Size to bottom-right corner of Work Area. Generally (0,0) or (0,-status_bar_height).
    KGVec2              BuildWorkOffsetMin;     // Work Area: Offset being built during current frame. Generally >= 0.0f.
    KGVec2              BuildWorkOffsetMax;     // Work Area: Offset being built during current frame. Generally <= 0.0f.

    KGGuiViewportP()                    { Idx = -1; LastFrameActive = DrawListsLastFrame[0] = DrawListsLastFrame[1] = LastFrontMostStampCount = -1; LastNameHash = 0; Alpha = LastAlpha = 1.0f; PlatformMonitor = -1; Window = NULL; DrawLists[0] = DrawLists[1] = NULL; LastPlatformPos = LastPlatformSize = LastRendererSize = KGVec2(FLT_MAX, FLT_MAX); }
    ~KGGuiViewportP()                   { if (DrawLists[0]) KG_DELETE(DrawLists[0]); if (DrawLists[1]) KG_DELETE(DrawLists[1]); }
    void    ClearRequestFlags()         { PlatformRequestClose = PlatformRequestMove = PlatformRequestResize = false; }

    // Calculate work rect pos/size given a set of offset (we have 1 pair of offset for rect locked from last frame data, and 1 pair for currently building rect)
    KGVec2  CalcWorkRectPos(const KGVec2& off_min) const                            { return KGVec2(Pos.x + off_min.x, Pos.y + off_min.y); }
    KGVec2  CalcWorkRectSize(const KGVec2& off_min, const KGVec2& off_max) const    { return KGVec2(KGMax(0.0f, Size.x - off_min.x + off_max.x), KGMax(0.0f, Size.y - off_min.y + off_max.y)); }
    void    UpdateWorkRect()            { WorkPos = CalcWorkRectPos(WorkOffsetMin); WorkSize = CalcWorkRectSize(WorkOffsetMin, WorkOffsetMax); } // Update public fields

    // Helpers to retrieve KGRect (we don't need to store BuildWorkRect as every access tend to change it, hence the code asymmetry)
    KGRect  GetMainRect() const         { return KGRect(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y); }
    KGRect  GetWorkRect() const         { return KGRect(WorkPos.x, WorkPos.y, WorkPos.x + WorkSize.x, WorkPos.y + WorkSize.y); }
    KGRect  GetBuildWorkRect() const    { KGVec2 pos = CalcWorkRectPos(BuildWorkOffsetMin); KGVec2 size = CalcWorkRectSize(BuildWorkOffsetMin, BuildWorkOffsetMax); return KGRect(pos.x, pos.y, pos.x + size.x, pos.y + size.y); }
};

//-----------------------------------------------------------------------------
// [SECTION] Settings support
//-----------------------------------------------------------------------------

// Windows data saved in kggui.ini file
// Because we never destroy or rename KGGuiWindowSettings, we can store the names in a separate buffer easily.
// (this is designed to be stored in a KGChunkStream buffer, with the variable-length Name following our structure)
struct KGGuiWindowSettings
{
    KGGuiID     ID;
    KGVec2ih    Pos;            // NB: Settings position are stored RELATIVE to the viewport! Whereas runtime ones are absolute positions.
    KGVec2ih    Size;
    KGVec2ih    ViewportPos;
    KGGuiID     ViewportId;
    KGGuiID     DockId;         // ID of last known DockNode (even if the DockNode is invisible because it has only 1 active window), or 0 if none.
    KGGuiID     ClassId;        // ID of window class if specified
    short       DockOrder;      // Order of the last time the window was visible within its DockNode. This is used to reorder windows that are reappearing on the same frame. Same value between windows that were active and windows that were none are possible.
    bool        Collapsed;
    bool        WantApply;      // Set when loaded from .ini data (to enable merging/loading .ini data into an already running context)

    KGGuiWindowSettings()       { memset(this, 0, sizeof(*this)); DockOrder = -1; }
    char* GetName()             { return (char*)(this + 1); }
};

struct KGGuiSettingsHandler
{
    const char* TypeName;       // Short description stored in .ini file. Disallowed characters: '[' ']'
    KGGuiID     TypeHash;       // == KGHashStr(TypeName)
    void        (*ClearAllFn)(KarmaGuiContext* ctx, KGGuiSettingsHandler* handler);                                // Clear all settings data
    void        (*ReadInitFn)(KarmaGuiContext* ctx, KGGuiSettingsHandler* handler);                                // Read: Called before reading (in registration order)
    void*       (*ReadOpenFn)(KarmaGuiContext* ctx, KGGuiSettingsHandler* handler, const char* name);              // Read: Called when entering into a new ini entry e.g. "[Window][Name]"
    void        (*ReadLineFn)(KarmaGuiContext* ctx, KGGuiSettingsHandler* handler, void* entry, const char* line); // Read: Called for every line of text within an ini entry
    void        (*ApplyAllFn)(KarmaGuiContext* ctx, KGGuiSettingsHandler* handler);                                // Read: Called after reading (in registration order)
    void        (*WriteAllFn)(KarmaGuiContext* ctx, KGGuiSettingsHandler* handler, KarmaGuiTextBuffer* out_buf);      // Write: Output every entries into 'out_buf'
    void*       UserData;

    KGGuiSettingsHandler() { memset(this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] Localization support
//-----------------------------------------------------------------------------

// This is experimental and not officially supported, it'll probably fall short of features, if/when it does we may backtrack.
enum KGGuiLocKey : int
{
    ImGuiLocKey_TableSizeOne,
    ImGuiLocKey_TableSizeAllFit,
    ImGuiLocKey_TableSizeAllDefault,
    ImGuiLocKey_TableResetOrder,
    ImGuiLocKey_WindowingMainMenuBar,
    ImGuiLocKey_WindowingPopup,
    ImGuiLocKey_WindowingUntitled,
    ImGuiLocKey_DockingHideTabBar,
    ImGuiLocKey_COUNT
};

struct KGGuiLocEntry
{
    KGGuiLocKey     Key;
    const char*     Text;
};


//-----------------------------------------------------------------------------
// [SECTION] Metrics, Debug Tools
//-----------------------------------------------------------------------------

enum KGGuiDebugLogFlags_
{
    // Event types
    KGGuiDebugLogFlags_None             = 0,
    KGGuiDebugLogFlags_EventActiveId    = 1 << 0,
    KGGuiDebugLogFlags_EventFocus       = 1 << 1,
    KGGuiDebugLogFlags_EventPopup       = 1 << 2,
    KGGuiDebugLogFlags_EventNav         = 1 << 3,
    KGGuiDebugLogFlags_EventClipper     = 1 << 4,
    KGGuiDebugLogFlags_EventIO          = 1 << 5,
    KGGuiDebugLogFlags_EventDocking     = 1 << 6,
    KGGuiDebugLogFlags_EventViewport    = 1 << 7,
    KGGuiDebugLogFlags_EventMask_       = KGGuiDebugLogFlags_EventActiveId | KGGuiDebugLogFlags_EventFocus | KGGuiDebugLogFlags_EventPopup | KGGuiDebugLogFlags_EventNav | KGGuiDebugLogFlags_EventClipper | KGGuiDebugLogFlags_EventIO | KGGuiDebugLogFlags_EventDocking | KGGuiDebugLogFlags_EventViewport,
    KGGuiDebugLogFlags_OutputToTTY      = 1 << 10,  // Also send output to TTY
};

struct KGGuiMetricsConfig
{
    bool        ShowDebugLog;
    bool        ShowStackTool;
    bool        ShowWindowsRects;
    bool        ShowWindowsBeginOrder;
    bool        ShowTablesRects;
    bool        ShowDrawCmdMesh;
    bool        ShowDrawCmdBoundingBoxes;
    bool        ShowDockingNodes;
    int         ShowWindowsRectsType;
    int         ShowTablesRectsType;

    KGGuiMetricsConfig()
    {
        ShowDebugLog = ShowStackTool = ShowWindowsRects = ShowWindowsBeginOrder = ShowTablesRects = false;
        ShowDrawCmdMesh = true;
        ShowDrawCmdBoundingBoxes = true;
        ShowDockingNodes = false;
        ShowWindowsRectsType = ShowTablesRectsType = -1;
    }
};

struct KGGuiStackLevelInfo
{
    KGGuiID                 ID;
    KGS8                    QueryFrameCount;            // >= 1: Query in progress
    bool                    QuerySuccess;               // Obtained result from DebugHookIdInfo()
    KarmaGuiDataType           DataType : 8;
    char                    Desc[57];                   // Arbitrarily sized buffer to hold a result (FIXME: could replace Results[] with a chunk stream?) FIXME: Now that we added CTRL+C this should be fixed.

    KGGuiStackLevelInfo()   { memset(this, 0, sizeof(*this)); }
};

// State for Stack tool queries
struct KGGuiStackTool
{
    int                     LastActiveFrame;
    int                     StackLevel;                 // -1: query stack and resize Results, >= 0: individual stack level
    KGGuiID                 QueryId;                    // ID to query details for
    KGVector<KGGuiStackLevelInfo> Results;
    bool                    CopyToClipboardOnCtrlC;
    float                   CopyToClipboardLastTime;

    KGGuiStackTool()        { memset(this, 0, sizeof(*this)); CopyToClipboardLastTime = -FLT_MAX; }
};

//-----------------------------------------------------------------------------
// [SECTION] Generic context hooks
//-----------------------------------------------------------------------------

typedef void (*ImGuiContextHookCallback)(KarmaGuiContext* ctx, KGGuiContextHook* hook);
enum ImGuiContextHookType { ImGuiContextHookType_NewFramePre, ImGuiContextHookType_NewFramePost, ImGuiContextHookType_EndFramePre, ImGuiContextHookType_EndFramePost, ImGuiContextHookType_RenderPre, ImGuiContextHookType_RenderPost, ImGuiContextHookType_Shutdown, ImGuiContextHookType_PendingRemoval_ };

struct KGGuiContextHook
{
    KGGuiID                     HookId;     // A unique ID assigned by AddContextHook()
    ImGuiContextHookType        Type;
    KGGuiID                     Owner;
    ImGuiContextHookCallback    Callback;
    void*                       UserData;

    KGGuiContextHook()          { memset(this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] KarmaGuiContext (main Dear ImGui context)
//-----------------------------------------------------------------------------

struct  KarmaGuiContext
{
    bool                    Initialized;
    bool                    FontAtlasOwnedByContext;            // IO.Fonts-> is owned by the KarmaGuiContext and will be destructed along with it.
    KarmaGuiIO                 IO;
    KarmaGuiPlatformIO         PlatformIO;
    KGVector<KGGuiInputEvent> InputEventsQueue;                 // Input events which will be tricked/written into IO structure.
    KGVector<KGGuiInputEvent> InputEventsTrail;                 // Past input events processed in NewFrame(). This is to allow domain-specific application to access e.g mouse/pen trail.
    KarmaGuiStyle              Style;
    KarmaGuiConfigFlags        ConfigFlagsCurrFrame;               // = g.IO.ConfigFlags at the time of NewFrame()
    KarmaGuiConfigFlags        ConfigFlagsLastFrame;
    KGFont*                 Font;                               // (Shortcut) == FontStack.empty() ? IO.Font : FontStack.back()
    float                   FontSize;                           // (Shortcut) == FontBaseSize * g.CurrentWindow->FontWindowScale == window->FontSize(). Text height for current window.
    float                   FontBaseSize;                       // (Shortcut) == IO.FontGlobalScale * Font->Scale * Font->FontSize. Base text height.
    KGDrawListSharedData    DrawListSharedData;
    double                  Time;
    int                     FrameCount;
    int                     FrameCountEnded;
    int                     FrameCountPlatformEnded;
    int                     FrameCountRendered;
    bool                    WithinFrameScope;                   // Set by NewFrame(), cleared by EndFrame()
    bool                    WithinFrameScopeWithImplicitWindow; // Set by NewFrame(), cleared by EndFrame() when the implicit debug window has been pushed
    bool                    WithinEndChild;                     // Set within EndChild()
    bool                    GcCompactAll;                       // Request full GC
    bool                    TestEngineHookItems;                // Will call test engine hooks: ImGuiTestEngineHook_ItemAdd(), ImGuiTestEngineHook_ItemInfo(), ImGuiTestEngineHook_Log()
    void*                   TestEngine;                         // Test engine user data

    // Windows state
    KGVector<KGGuiWindow*>  Windows;                            // Windows, sorted in display order, back to front
    KGVector<KGGuiWindow*>  WindowsFocusOrder;                  // Root windows, sorted in focus order, back to front.
    KGVector<KGGuiWindow*>  WindowsTempSortBuffer;              // Temporary buffer used in EndFrame() to reorder windows so parents are kept before their child
    KGVector<KGGuiWindowStackData> CurrentWindowStack;
    KarmaGuiStorage            WindowsById;                        // Map window's KGGuiID to KGGuiWindow*
    int                     WindowsActiveCount;                 // Number of unique windows submitted by frame
    KGVec2                  WindowsHoverPadding;                // Padding around resizable windows for which hovering on counts as hovering the window == KGMax(style.TouchExtraPadding, WINDOWS_HOVER_PADDING)
    KGGuiWindow*            CurrentWindow;                      // Window being drawn into
    KGGuiWindow*            HoveredWindow;                      // Window the mouse is hovering. Will typically catch mouse inputs.
    KGGuiWindow*            HoveredWindowUnderMovingWindow;     // Hovered window ignoring MovingWindow. Only set if MovingWindow is set.
    KGGuiWindow*            MovingWindow;                       // Track the window we clicked on (in order to preserve focus). The actual window that is moved is generally MovingWindow->RootWindowDockTree.
    KGGuiWindow*            WheelingWindow;                     // Track the window we started mouse-wheeling on. Until a timer elapse or mouse has moved, generally keep scrolling the same window even if during the course of scrolling the mouse ends up hovering a child window.
    KGVec2                  WheelingWindowRefMousePos;
    int                     WheelingWindowStartFrame;           // This may be set one frame before WheelingWindow is != NULL
    float                   WheelingWindowReleaseTimer;
    KGVec2                  WheelingWindowWheelRemainder;
    KGVec2                  WheelingAxisAvg;

    // Item/widgets state and tracking information
    KGGuiID                 DebugHookIdInfo;                    // Will call core hooks: DebugHookIdInfo() from GetID functions, used by Stack Tool [next HoveredId/ActiveId to not pull in an extra cache-line]
    KGGuiID                 HoveredId;                          // Hovered widget, filled during the frame
    KGGuiID                 HoveredIdPreviousFrame;
    bool                    HoveredIdAllowOverlap;
    bool                    HoveredIdDisabled;                  // At least one widget passed the rect test, but has been discarded by disabled flag or popup inhibit. May be true even if HoveredId == 0.
    float                   HoveredIdTimer;                     // Measure contiguous hovering time
    float                   HoveredIdNotActiveTimer;            // Measure contiguous hovering time where the item has not been active
    KGGuiID                 ActiveId;                           // Active widget
    KGGuiID                 ActiveIdIsAlive;                    // Active widget has been seen this frame (we can't use a bool as the ActiveId may change within the frame)
    float                   ActiveIdTimer;
    bool                    ActiveIdIsJustActivated;            // Set at the time of activation for one frame
    bool                    ActiveIdAllowOverlap;               // Active widget allows another widget to steal active id (generally for overlapping widgets, but not always)
    bool                    ActiveIdNoClearOnFocusLoss;         // Disable losing active id if the active id window gets unfocused.
    bool                    ActiveIdHasBeenPressedBefore;       // Track whether the active id led to a press (this is to allow changing between PressOnClick and PressOnRelease without pressing twice). Used by range_select branch.
    bool                    ActiveIdHasBeenEditedBefore;        // Was the value associated to the widget Edited over the course of the Active state.
    bool                    ActiveIdHasBeenEditedThisFrame;
    KGVec2                  ActiveIdClickOffset;                // Clicked offset from upper-left corner, if applicable (currently only set by ButtonBehavior)
    KGGuiWindow*            ActiveIdWindow;
    KGGuiInputSource        ActiveIdSource;                     // Activating with mouse or nav (gamepad/keyboard)
    int                     ActiveIdMouseButton;
    KGGuiID                 ActiveIdPreviousFrame;
    bool                    ActiveIdPreviousFrameIsAlive;
    bool                    ActiveIdPreviousFrameHasBeenEditedBefore;
    KGGuiWindow*            ActiveIdPreviousFrameWindow;
    KGGuiID                 LastActiveId;                       // Store the last non-zero ActiveId, useful for animation.
    float                   LastActiveIdTimer;                  // Store the last non-zero ActiveId timer since the beginning of activation, useful for animation.

    // [EXPERIMENTAL] Key/Input Ownership + Shortcut Routing system
    // - The idea is that instead of "eating" a given key, we can link to an owner.
    // - Input query can then read input by specifying KGGuiKeyOwner_Any (== 0), KGGuiKeyOwner_None (== -1) or a custom ID.
    // - Routing is requested ahead of time for a given chord (Key + Mods) and granted in NewFrame().
    KGGuiKeyOwnerData       KeysOwnerData[KGGuiKey_NamedKey_COUNT];
    KGGuiKeyRoutingTable    KeysRoutingTable;
    KGU32                   ActiveIdUsingNavDirMask;            // Active widget will want to read those nav move requests (e.g. can activate a button and move away from it)
    bool                    ActiveIdUsingAllKeyboardKeys;       // Active widget will want to read all keyboard keys inputs. (FIXME: This is a shortcut for not taking ownership of 100+ keys but perhaps best to not have the inconsistency)
#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
    KGU32                   ActiveIdUsingNavInputMask;          // If you used this. Since (IMGUI_VERSION_NUM >= 18804) : 'g.ActiveIdUsingNavInputMask |= (1 << KGGuiNavInput_Cancel);' becomes 'SetKeyOwner(KGGuiKey_Escape, g.ActiveId) and/or SetKeyOwner(KGGuiKey_NavGamepadCancel, g.ActiveId);'
#endif

    // Next window/item data
    KGGuiID                 CurrentFocusScopeId;                // == g.FocusScopeStack.back()
    KGGuiItemFlags          CurrentItemFlags;                   // == g.ItemFlagsStack.back()
    KGGuiID                 DebugLocateId;                      // Storage for DebugLocateItemOnHover() feature: this is read by ItemAdd() so we keep it in a hot/cached location
    KGGuiNextItemData       NextItemData;                       // Storage for SetNextItem** functions
    KGGuiLastItemData       LastItemData;                       // Storage for last submitted item (setup by ItemAdd)
    KGGuiNextWindowData     NextWindowData;                     // Storage for SetNextWindow** functions

    // Shared stacks
    KGVector<KGGuiColorMod> ColorStack;                         // Stack for PushStyleColor()/PopStyleColor() - inherited by Begin()
    KGVector<KGGuiStyleMod> StyleVarStack;                      // Stack for PushStyleVar()/PopStyleVar() - inherited by Begin()
    KGVector<KGFont*>       FontStack;                          // Stack for PushFont()/PopFont() - inherited by Begin()
    KGVector<KGGuiID>       FocusScopeStack;                    // Stack for PushFocusScope()/PopFocusScope() - inherited by BeginChild(), pushed into by Begin()
    KGVector<KGGuiItemFlags>ItemFlagsStack;                     // Stack for PushItemFlag()/PopItemFlag() - inherited by Begin()
    KGVector<KGGuiGroupData>GroupStack;                         // Stack for BeginGroup()/EndGroup() - not inherited by Begin()
    KGVector<KGGuiPopupData>OpenPopupStack;                     // Which popups are open (persistent)
    KGVector<KGGuiPopupData>BeginPopupStack;                    // Which level of BeginPopup() we are in (reset every frame)
    int                     BeginMenuCount;

    // Viewports
    KGVector<KGGuiViewportP*> Viewports;                        // Active viewports (always 1+, and generally 1 unless multi-viewports are enabled). Each viewports hold their copy of KGDrawData.
    float                   CurrentDpiScale;                    // == CurrentViewport->DpiScale
    KGGuiViewportP*         CurrentViewport;                    // We track changes of viewport (happening in Begin) so we can call Platform_OnChangedViewport()
    KGGuiViewportP*         MouseViewport;
    KGGuiViewportP*         MouseLastHoveredViewport;           // Last known viewport that was hovered by mouse (even if we are not hovering any viewport any more) + honoring the _NoInputs flag.
    KGGuiID                 PlatformLastFocusedViewportId;
    KarmaGuiPlatformMonitor    FallbackMonitor;                    // Virtual monitor used as fallback if backend doesn't provide monitor information.
    int                     ViewportFrontMostStampCount;        // Every time the front-most window changes, we stamp its viewport with an incrementing counter

    // Gamepad/keyboard Navigation
    KGGuiWindow*            NavWindow;                          // Focused window for navigation. Could be called 'FocusedWindow'
    KGGuiID                 NavId;                              // Focused item for navigation
    KGGuiID                 NavFocusScopeId;                    // Identify a selection scope (selection code often wants to "clear other items" when landing on an item of the selection set)
    KGGuiID                 NavActivateId;                      // ~~ (g.ActiveId == 0) && (IsKeyPressed(KGGuiKey_Space) || IsKeyPressed(KGGuiKey_NavGamepadActivate)) ? NavId : 0, also set when calling ActivateItem()
    KGGuiID                 NavActivateDownId;                  // ~~ IsKeyDown(KGGuiKey_Space) || IsKeyDown(KGGuiKey_NavGamepadActivate) ? NavId : 0
    KGGuiID                 NavActivatePressedId;               // ~~ IsKeyPressed(KGGuiKey_Space) || IsKeyPressed(KGGuiKey_NavGamepadActivate) ? NavId : 0 (no repeat)
    KGGuiID                 NavActivateInputId;                 // ~~ IsKeyPressed(KGGuiKey_Enter) || IsKeyPressed(KGGuiKey_NavGamepadInput) ? NavId : 0; KGGuiActivateFlags_PreferInput will be set and NavActivateId will be 0.
    KGGuiActivateFlags      NavActivateFlags;
    KGGuiID                 NavJustMovedToId;                   // Just navigated to this id (result of a successfully MoveRequest).
    KGGuiID                 NavJustMovedToFocusScopeId;         // Just navigated to this focus scope id (result of a successfully MoveRequest).
    KarmaGuiKeyChord           NavJustMovedToKeyMods;
    KGGuiID                 NavNextActivateId;                  // Set by ActivateItem(), queued until next frame.
    KGGuiActivateFlags      NavNextActivateFlags;
    KGGuiInputSource        NavInputSource;                     // Keyboard or Gamepad mode? THIS WILL ONLY BE None or NavGamepad or NavKeyboard.
    KGGuiNavLayer           NavLayer;                           // Layer we are navigating on. For now the system is hard-coded for 0=main contents and 1=menu/title bar, may expose layers later.
    bool                    NavIdIsAlive;                       // Nav widget has been seen this frame ~~ NavRectRel is valid
    bool                    NavMousePosDirty;                   // When set we will update mouse position if (io.ConfigFlags & KGGuiConfigFlags_NavEnableSetMousePos) if set (NB: this not enabled by default)
    bool                    NavDisableHighlight;                // When user starts using mouse, we hide gamepad/keyboard highlight (NB: but they are still available, which is why NavDisableHighlight isn't always != NavDisableMouseHover)
    bool                    NavDisableMouseHover;               // When user starts using gamepad/keyboard, we hide mouse hovering highlight until mouse is touched again.

    // Navigation: Init & Move Requests
    bool                    NavAnyRequest;                      // ~~ NavMoveRequest || NavInitRequest this is to perform early out in ItemAdd()
    bool                    NavInitRequest;                     // Init request for appearing window to select first item
    bool                    NavInitRequestFromMove;
    KGGuiID                 NavInitResultId;                    // Init request result (first item of the window, or one for which SetItemDefaultFocus() was called)
    KGRect                  NavInitResultRectRel;               // Init request result rectangle (relative to parent window)
    bool                    NavMoveSubmitted;                   // Move request submitted, will process result on next NewFrame()
    bool                    NavMoveScoringItems;                // Move request submitted, still scoring incoming items
    bool                    NavMoveForwardToNextFrame;
    KGGuiNavMoveFlags       NavMoveFlags;
    KGGuiScrollFlags        NavMoveScrollFlags;
    KarmaGuiKeyChord           NavMoveKeyMods;
    KarmaGuiDir                NavMoveDir;                         // Direction of the move request (left/right/up/down)
    KarmaGuiDir                NavMoveDirForDebug;
    KarmaGuiDir                NavMoveClipDir;                     // FIXME-NAV: Describe the purpose of this better. Might want to rename?
    KGRect                  NavScoringRect;                     // Rectangle used for scoring, in screen space. Based of window->NavRectRel[], modified for directional navigation scoring.
    KGRect                  NavScoringNoClipRect;               // Some nav operations (such as PageUp/PageDown) enforce a region which clipper will attempt to always keep submitted
    int                     NavScoringDebugCount;               // Metrics for debugging
    int                     NavTabbingDir;                      // Generally -1 or +1, 0 when tabbing without a nav id
    int                     NavTabbingCounter;                  // >0 when counting items for tabbing
    KGGuiNavItemData        NavMoveResultLocal;                 // Best move request candidate within NavWindow
    KGGuiNavItemData        NavMoveResultLocalVisible;          // Best move request candidate within NavWindow that are mostly visible (when using KGGuiNavMoveFlags_AlsoScoreVisibleSet flag)
    KGGuiNavItemData        NavMoveResultOther;                 // Best move request candidate within NavWindow's flattened hierarchy (when using KGGuiWindowFlags_NavFlattened flag)
    KGGuiNavItemData        NavTabbingResultFirst;              // First tabbing request candidate within NavWindow and flattened hierarchy

    // Navigation: Windowing (CTRL+TAB for list, or Menu button + keys or directional pads to move/resize)
    KarmaGuiKeyChord           ConfigNavWindowingKeyNext;          // = KGGuiMod_Ctrl | KGGuiKey_Tab, for reconfiguration (see #4828)
    KarmaGuiKeyChord           ConfigNavWindowingKeyPrev;          // = KGGuiMod_Ctrl | KGGuiMod_Shift | KGGuiKey_Tab
    KGGuiWindow*            NavWindowingTarget;                 // Target window when doing CTRL+Tab (or Pad Menu + FocusPrev/Next), this window is temporarily displayed top-most!
    KGGuiWindow*            NavWindowingTargetAnim;             // Record of last valid NavWindowingTarget until DimBgRatio and NavWindowingHighlightAlpha becomes 0.0f, so the fade-out can stay on it.
    KGGuiWindow*            NavWindowingListWindow;             // Internal window actually listing the CTRL+Tab contents
    float                   NavWindowingTimer;
    float                   NavWindowingHighlightAlpha;
    bool                    NavWindowingToggleLayer;
    KGVec2                  NavWindowingAccumDeltaPos;
    KGVec2                  NavWindowingAccumDeltaSize;

    // Render
    float                   DimBgRatio;                         // 0.0..1.0 animation when fading in a dimming background (for modal window and CTRL+TAB list)
    KarmaGuiMouseCursor        MouseCursor;

    // Drag and Drop
    bool                    DragDropActive;
    bool                    DragDropWithinSource;               // Set when within a BeginDragDropXXX/EndDragDropXXX block for a drag source.
    bool                    DragDropWithinTarget;               // Set when within a BeginDragDropXXX/EndDragDropXXX block for a drag target.
    KarmaGuiDragDropFlags      DragDropSourceFlags;
    int                     DragDropSourceFrameCount;
    int                     DragDropMouseButton;
    KarmaGuiPayload            DragDropPayload;
    KGRect                  DragDropTargetRect;                 // Store rectangle of current target candidate (we favor small targets when overlapping)
    KGGuiID                 DragDropTargetId;
    KarmaGuiDragDropFlags      DragDropAcceptFlags;
    float                   DragDropAcceptIdCurrRectSurface;    // Target item surface (we resolve overlapping targets by prioritizing the smaller surface)
    KGGuiID                 DragDropAcceptIdCurr;               // Target item id (set at the time of accepting the payload)
    KGGuiID                 DragDropAcceptIdPrev;               // Target item id from previous frame (we need to store this to allow for overlapping drag and drop targets)
    int                     DragDropAcceptFrameCount;           // Last time a target expressed a desire to accept the source
    KGGuiID                 DragDropHoldJustPressedId;          // Set when holding a payload just made ButtonBehavior() return a press.
    KGVector<unsigned char> DragDropPayloadBufHeap;             // We don't expose the KGVector<> directly, KarmaGuiPayload only holds pointer+size
    unsigned char           DragDropPayloadBufLocal[16];        // Local buffer for small payloads

    // Clipper
    int                             ClipperTempDataStacked;
    KGVector<KGGuiListClipperData>  ClipperTempData;

    // Tables
    KGGuiTable*                     CurrentTable;
    int                             TablesTempDataStacked;      // Temporary table data size (because we leave previous instances undestructed, we generally don't use TablesTempData.Size)
    KGVector<KGGuiTableTempData>    TablesTempData;             // Temporary table data (buffers reused/shared across instances, support nesting)
    KGPool<KGGuiTable>              Tables;                     // Persistent table data
    KGVector<float>                 TablesLastTimeActive;       // Last used timestamp of each tables (SOA, for efficient GC)
    KGVector<KGDrawChannel>         DrawChannelsTempMergeBuffer;

    // Tab bars
    KGGuiTabBar*                    CurrentTabBar;
    KGPool<KGGuiTabBar>             TabBars;
    KGVector<KGGuiPtrOrIndex>       CurrentTabBarStack;
    KGVector<KGGuiShrinkWidthItem>  ShrinkWidthBuffer;

    // Hover Delay system
    KGGuiID                 HoverDelayId;
    KGGuiID                 HoverDelayIdPreviousFrame;
    float                   HoverDelayTimer;                    // Currently used IsItemHovered(), generally inferred from g.HoveredIdTimer but kept uncleared until clear timer elapse.
    float                   HoverDelayClearTimer;               // Currently used IsItemHovered(): grace time before g.TooltipHoverTimer gets cleared.

    // Widget state
    KGVec2                  MouseLastValidPos;
    KGGuiInputTextState     InputTextState;
    KGFont                  InputTextPasswordFont;
    KGGuiID                 TempInputId;                        // Temporary text input when CTRL+clicking on a slider, etc.
    KarmaGuiColorEditFlags     ColorEditOptions;                   // Store user options for color edit widgets
    float                   ColorEditLastHue;                   // Backup of last Hue associated to LastColor, so we can restore Hue in lossy RGB<>HSV round trips
    float                   ColorEditLastSat;                   // Backup of last Saturation associated to LastColor, so we can restore Saturation in lossy RGB<>HSV round trips
    KGU32                   ColorEditLastColor;                 // RGB value with alpha set to 0.
    KGVec4                  ColorPickerRef;                     // Initial/reference color at the time of opening the color picker.
    KGGuiComboPreviewData   ComboPreviewData;
    float                   SliderGrabClickOffset;
    float                   SliderCurrentAccum;                 // Accumulated slider delta when using navigation controls.
    bool                    SliderCurrentAccumDirty;            // Has the accumulated slider delta changed since last time we tried to apply it?
    bool                    DragCurrentAccumDirty;
    float                   DragCurrentAccum;                   // Accumulator for dragging modification. Always high-precision, not rounded by end-user precision settings
    float                   DragSpeedDefaultRatio;              // If speed == 0.0f, uses (max-min) * DragSpeedDefaultRatio
    float                   ScrollbarClickDeltaToGrabCenter;    // Distance between mouse and center of grab box, normalized in parent space. Use storage?
    float                   DisabledAlphaBackup;                // Backup for style.Alpha for BeginDisabled()
    short                   DisabledStackSize;
    short                   TooltipOverrideCount;
    KGVector<char>          ClipboardHandlerData;               // If no custom clipboard handler is defined
    KGVector<KGGuiID>       MenusIdSubmittedThisFrame;          // A list of menu IDs that were rendered at least once

    // Platform support
    KarmaGuiPlatformImeData    PlatformImeData;                    // Data updated by current frame
    KarmaGuiPlatformImeData    PlatformImeDataPrev;                // Previous frame data (when changing we will call io.SetPlatformImeDataFn
    KGGuiID                 PlatformImeViewport;
    char                    PlatformLocaleDecimalPoint;         // '.' or *localeconv()->decimal_point

    // Extensions
    // FIXME: We could provide an API to register one slot in an array held in KarmaGuiContext?
    KGGuiDockContext        DockContext;

    // Settings
    bool                    SettingsLoaded;
    float                   SettingsDirtyTimer;                 // Save .ini Settings to memory when time reaches zero
    KarmaGuiTextBuffer         SettingsIniData;                    // In memory .ini settings
    KGVector<KGGuiSettingsHandler>      SettingsHandlers;       // List of .ini settings handlers
    KGChunkStream<KGGuiWindowSettings>  SettingsWindows;        // KGGuiWindow .ini settings entries
    KGChunkStream<KGGuiTableSettings>   SettingsTables;         // KGGuiTable .ini settings entries
    KGVector<KGGuiContextHook>          Hooks;                  // Hooks for extensions (e.g. test engine)
    KGGuiID                             HookIdNext;             // Next available HookId

    // Localization
    const char*             LocalizationTable[ImGuiLocKey_COUNT];

    // Capture/Logging
    bool                    LogEnabled;                         // Currently capturing
    KGGuiLogType            LogType;                            // Capture target
    ImFileHandle            LogFile;                            // If != NULL log to stdout/ file
    KarmaGuiTextBuffer         LogBuffer;                          // Accumulation buffer when log to clipboard. This is pointer so our GKarmaGui static constructor doesn't call heap allocators.
    const char*             LogNextPrefix;
    const char*             LogNextSuffix;
    float                   LogLinePosY;
    bool                    LogLineFirstItem;
    int                     LogDepthRef;
    int                     LogDepthToExpand;
    int                     LogDepthToExpandDefault;            // Default/stored value for LogDepthMaxExpand if not specified in the LogXXX function call.

    // Debug Tools
    KGGuiDebugLogFlags      DebugLogFlags;
    KarmaGuiTextBuffer         DebugLogBuf;
    KGGuiTextIndex          DebugLogIndex;
    KGU8                    DebugLocateFrames;                  // For DebugLocateItemOnHover(). This is used together with DebugLocateId which is in a hot/cached spot above.
    bool                    DebugItemPickerActive;              // Item picker is active (started with DebugStartItemPicker())
    KGU8                    DebugItemPickerMouseButton;
    KGGuiID                 DebugItemPickerBreakId;             // Will call KR_CORE_ASSERT() when encountering this ID
    KGGuiMetricsConfig      DebugMetricsConfig;
    KGGuiStackTool          DebugStackTool;
    KGGuiDockNode*          DebugHoveredDockNode;               // Hovered dock node.

    // Misc
    float                   FramerateSecPerFrame[60];           // Calculate estimate of framerate for user over the last 60 frames..
    int                     FramerateSecPerFrameIdx;
    int                     FramerateSecPerFrameCount;
    float                   FramerateSecPerFrameAccum;
    int                     WantCaptureMouseNextFrame;          // Explicit capture override via SetNextFrameWantCaptureMouse()/SetNextFrameWantCaptureKeyboard(). Default to -1.
    int                     WantCaptureKeyboardNextFrame;       // "
    int                     WantTextInputNextFrame;
    KGVector<char>          TempBuffer;                         // Temporary text buffer

    KarmaGuiContext(KGFontAtlas* shared_font_atlas)
        : InputTextState(this)
    {
        Initialized = false;
        ConfigFlagsCurrFrame = ConfigFlagsLastFrame = KGGuiConfigFlags_None;
        FontAtlasOwnedByContext = shared_font_atlas ? false : true;
        Font = NULL;
        FontSize = FontBaseSize = 0.0f;
        IO.Fonts = shared_font_atlas ? shared_font_atlas : KG_NEW(KGFontAtlas)();
        Time = 0.0f;
        FrameCount = 0;
        FrameCountEnded = FrameCountPlatformEnded = FrameCountRendered = -1;
        WithinFrameScope = WithinFrameScopeWithImplicitWindow = WithinEndChild = false;
        GcCompactAll = false;
        TestEngineHookItems = false;
        TestEngine = NULL;

        WindowsActiveCount = 0;
        CurrentWindow = NULL;
        HoveredWindow = NULL;
        HoveredWindowUnderMovingWindow = NULL;
        MovingWindow = NULL;
        WheelingWindow = NULL;
        WheelingWindowStartFrame = -1;
        WheelingWindowReleaseTimer = 0.0f;

        DebugHookIdInfo = 0;
        HoveredId = HoveredIdPreviousFrame = 0;
        HoveredIdAllowOverlap = false;
        HoveredIdDisabled = false;
        HoveredIdTimer = HoveredIdNotActiveTimer = 0.0f;
        ActiveId = 0;
        ActiveIdIsAlive = 0;
        ActiveIdTimer = 0.0f;
        ActiveIdIsJustActivated = false;
        ActiveIdAllowOverlap = false;
        ActiveIdNoClearOnFocusLoss = false;
        ActiveIdHasBeenPressedBefore = false;
        ActiveIdHasBeenEditedBefore = false;
        ActiveIdHasBeenEditedThisFrame = false;
        ActiveIdClickOffset = KGVec2(-1, -1);
        ActiveIdWindow = NULL;
        ActiveIdSource = KGGuiInputSource_None;
        ActiveIdMouseButton = -1;
        ActiveIdPreviousFrame = 0;
        ActiveIdPreviousFrameIsAlive = false;
        ActiveIdPreviousFrameHasBeenEditedBefore = false;
        ActiveIdPreviousFrameWindow = NULL;
        LastActiveId = 0;
        LastActiveIdTimer = 0.0f;

        ActiveIdUsingNavDirMask = 0x00;
        ActiveIdUsingAllKeyboardKeys = false;
#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
        ActiveIdUsingNavInputMask = 0x00;
#endif

        CurrentFocusScopeId = 0;
        CurrentItemFlags = KGGuiItemFlags_None;
        BeginMenuCount = 0;

        CurrentDpiScale = 0.0f;
        CurrentViewport = NULL;
        MouseViewport = MouseLastHoveredViewport = NULL;
        PlatformLastFocusedViewportId = 0;
        ViewportFrontMostStampCount = 0;

        NavWindow = NULL;
        NavId = NavFocusScopeId = NavActivateId = NavActivateDownId = NavActivatePressedId = NavActivateInputId = 0;
        NavJustMovedToId = NavJustMovedToFocusScopeId = NavNextActivateId = 0;
        NavActivateFlags = NavNextActivateFlags = KGGuiActivateFlags_None;
        NavJustMovedToKeyMods = KGGuiMod_None;
        NavInputSource = KGGuiInputSource_None;
        NavLayer = KGGuiNavLayer_Main;
        NavIdIsAlive = false;
        NavMousePosDirty = false;
        NavDisableHighlight = true;
        NavDisableMouseHover = false;
        NavAnyRequest = false;
        NavInitRequest = false;
        NavInitRequestFromMove = false;
        NavInitResultId = 0;
        NavMoveSubmitted = false;
        NavMoveScoringItems = false;
        NavMoveForwardToNextFrame = false;
        NavMoveFlags = KGGuiNavMoveFlags_None;
        NavMoveScrollFlags = KGGuiScrollFlags_None;
        NavMoveKeyMods = KGGuiMod_None;
        NavMoveDir = NavMoveDirForDebug = NavMoveClipDir = KGGuiDir_None;
        NavScoringDebugCount = 0;
        NavTabbingDir = 0;
        NavTabbingCounter = 0;

        ConfigNavWindowingKeyNext = KGGuiMod_Ctrl | KGGuiKey_Tab;
        ConfigNavWindowingKeyPrev = KGGuiMod_Ctrl | KGGuiMod_Shift | KGGuiKey_Tab;
        NavWindowingTarget = NavWindowingTargetAnim = NavWindowingListWindow = NULL;
        NavWindowingTimer = NavWindowingHighlightAlpha = 0.0f;
        NavWindowingToggleLayer = false;

        DimBgRatio = 0.0f;
        MouseCursor = KGGuiMouseCursor_Arrow;

        DragDropActive = DragDropWithinSource = DragDropWithinTarget = false;
        DragDropSourceFlags = KGGuiDragDropFlags_None;
        DragDropSourceFrameCount = -1;
        DragDropMouseButton = -1;
        DragDropTargetId = 0;
        DragDropAcceptFlags = KGGuiDragDropFlags_None;
        DragDropAcceptIdCurrRectSurface = 0.0f;
        DragDropAcceptIdPrev = DragDropAcceptIdCurr = 0;
        DragDropAcceptFrameCount = -1;
        DragDropHoldJustPressedId = 0;
        memset(DragDropPayloadBufLocal, 0, sizeof(DragDropPayloadBufLocal));

        ClipperTempDataStacked = 0;

        CurrentTable = NULL;
        TablesTempDataStacked = 0;
        CurrentTabBar = NULL;

        HoverDelayId = HoverDelayIdPreviousFrame = 0;
        HoverDelayTimer = HoverDelayClearTimer = 0.0f;

        TempInputId = 0;
        ColorEditOptions = KGGuiColorEditFlags_DefaultOptions_;
        ColorEditLastHue = ColorEditLastSat = 0.0f;
        ColorEditLastColor = 0;
        SliderGrabClickOffset = 0.0f;
        SliderCurrentAccum = 0.0f;
        SliderCurrentAccumDirty = false;
        DragCurrentAccumDirty = false;
        DragCurrentAccum = 0.0f;
        DragSpeedDefaultRatio = 1.0f / 100.0f;
        ScrollbarClickDeltaToGrabCenter = 0.0f;
        DisabledAlphaBackup = 0.0f;
        DisabledStackSize = 0;
        TooltipOverrideCount = 0;

        PlatformImeData.InputPos = KGVec2(0.0f, 0.0f);
        PlatformImeDataPrev.InputPos = KGVec2(-1.0f, -1.0f); // Different to ensure initial submission
        PlatformImeViewport = 0;
        PlatformLocaleDecimalPoint = '.';

        SettingsLoaded = false;
        SettingsDirtyTimer = 0.0f;
        HookIdNext = 0;

        memset(LocalizationTable, 0, sizeof(LocalizationTable));

        LogEnabled = false;
        LogType = KGGuiLogType_None;
        LogNextPrefix = LogNextSuffix = NULL;
        LogFile = NULL;
        LogLinePosY = FLT_MAX;
        LogLineFirstItem = false;
        LogDepthRef = 0;
        LogDepthToExpand = LogDepthToExpandDefault = 2;

        DebugLogFlags = KGGuiDebugLogFlags_OutputToTTY;
        DebugLocateId = 0;
        DebugLocateFrames = 0;
        DebugItemPickerActive = false;
        DebugItemPickerMouseButton = KGGuiMouseButton_Left;
        DebugItemPickerBreakId = 0;
        DebugHoveredDockNode = NULL;

        memset(FramerateSecPerFrame, 0, sizeof(FramerateSecPerFrame));
        FramerateSecPerFrameIdx = FramerateSecPerFrameCount = 0;
        FramerateSecPerFrameAccum = 0.0f;
        WantCaptureMouseNextFrame = WantCaptureKeyboardNextFrame = WantTextInputNextFrame = -1;
    }
};

//-----------------------------------------------------------------------------
// [SECTION] KGGuiWindowTempData, KGGuiWindow
//-----------------------------------------------------------------------------

// Transient per-window data, reset at the beginning of the frame. This used to be called ImGuiDrawContext, hence the DC variable name in KGGuiWindow.
// (That's theory, in practice the delimitation between KGGuiWindow and KGGuiWindowTempData is quite tenuous and could be reconsidered..)
// (This doesn't need a constructor because we zero-clear it as part of KGGuiWindow and all frame-temporary data are setup on Begin)
struct  KGGuiWindowTempData
{
    // Layout
    KGVec2                  CursorPos;              // Current emitting position, in absolute coordinates.
    KGVec2                  CursorPosPrevLine;
    KGVec2                  CursorStartPos;         // Initial position after Begin(), generally ~ window position + WindowPadding.
    KGVec2                  CursorMaxPos;           // Used to implicitly calculate ContentSize at the beginning of next frame, for scrolling range and auto-resize. Always growing during the frame.
    KGVec2                  IdealMaxPos;            // Used to implicitly calculate ContentSizeIdeal at the beginning of next frame, for auto-resize only. Always growing during the frame.
    KGVec2                  CurrLineSize;
    KGVec2                  PrevLineSize;
    float                   CurrLineTextBaseOffset; // Baseline offset (0.0f by default on a new line, generally == style.FramePadding.y when a framed item has been added).
    float                   PrevLineTextBaseOffset;
    bool                    IsSameLine;
    bool                    IsSetPos;
    KGVec1                  Indent;                 // Indentation / start position from left of window (increased by TreePush/TreePop, etc.)
    KGVec1                  ColumnsOffset;          // Offset to the current column (if ColumnsCurrent > 0). FIXME: This and the above should be a stack to allow use cases like Tree->Column->Tree. Need revamp columns API.
    KGVec1                  GroupOffset;
    KGVec2                  CursorStartPosLossyness;// Record the loss of precision of CursorStartPos due to really large scrolling amount. This is used by clipper to compensentate and fix the most common use case of large scroll area.

    // Keyboard/Gamepad navigation
    KGGuiNavLayer           NavLayerCurrent;        // Current layer, 0..31 (we currently only use 0..1)
    short                   NavLayersActiveMask;    // Which layers have been written to (result from previous frame)
    short                   NavLayersActiveMaskNext;// Which layers have been written to (accumulator for current frame)
    bool                    NavHideHighlightOneFrame;
    bool                    NavHasScroll;           // Set when scrolling can be used (ScrollMax > 0.0f)

    // Miscellaneous
    bool                    MenuBarAppending;       // FIXME: Remove this
    KGVec2                  MenuBarOffset;          // MenuBarOffset.x is sort of equivalent of a per-layer CursorPos.x, saved/restored as we switch to the menu bar. The only situation when MenuBarOffset.y is > 0 if when (SafeAreaPadding.y > FramePadding.y), often used on TVs.
    KGGuiMenuColumns        MenuColumns;            // Simplified columns storage for menu items measurement
    int                     TreeDepth;              // Current tree depth.
    KGU32                   TreeJumpToParentOnPopMask; // Store a copy of !g.NavIdIsAlive for TreeDepth 0..31.. Could be turned into a KGU64 if necessary.
    KGVector<KGGuiWindow*>  ChildWindows;
    KarmaGuiStorage*           StateStorage;           // Current persistent per-window storage (store e.g. tree node open/close state)
    KGGuiOldColumns*        CurrentColumns;         // Current columns set
    int                     CurrentTableIdx;        // Current table index (into g.Tables)
    KGGuiLayoutType         LayoutType;
    KGGuiLayoutType         ParentLayoutType;       // Layout type of parent window at the time of Begin()

    // Local parameters stacks
    // We store the current settings outside of the vectors to increase memory locality (reduce cache misses). The vectors are rarely modified. Also it allows us to not heap allocate for short-lived windows which are not using those settings.
    float                   ItemWidth;              // Current item width (>0.0: width in pixels, <0.0: align xx pixels to the right of window).
    float                   TextWrapPos;            // Current text wrap pos.
    KGVector<float>         ItemWidthStack;         // Store item widths to restore (attention: .back() is not == ItemWidth)
    KGVector<float>         TextWrapPosStack;       // Store text wrap pos to restore (attention: .back() is not == TextWrapPos)
};

// Storage for one window
struct  KGGuiWindow
{
    char*                   Name;                               // Window name, owned by the window.
    KGGuiID                 ID;                                 // == KGHashStr(Name)
    KarmaGuiWindowFlags        Flags, FlagsPreviousFrame;          // See enum KGGuiWindowFlags_
    KarmaGuiWindowClass        WindowClass;                        // Advanced users only. Set with SetNextWindowClass()
    KGGuiViewportP*         Viewport;                           // Always set in Begin(). Inactive windows may have a NULL value here if their viewport was discarded.
    KGGuiID                 ViewportId;                         // We backup the viewport id (since the viewport may disappear or never be created if the window is inactive)
    KGVec2                  ViewportPos;                        // We backup the viewport position (since the viewport may disappear or never be created if the window is inactive)
    int                     ViewportAllowPlatformMonitorExtend; // Reset to -1 every frame (index is guaranteed to be valid between NewFrame..EndFrame), only used in the Appearing frame of a tooltip/popup to enforce clamping to a given monitor
    KGVec2                  Pos;                                // Position (always rounded-up to nearest pixel)
    KGVec2                  Size;                               // Current size (==SizeFull or collapsed title bar size)
    KGVec2                  SizeFull;                           // Size when non collapsed
    KGVec2                  ContentSize;                        // Size of contents/scrollable client area (calculated from the extents reach of the cursor) from previous frame. Does not include window decoration or window padding.
    KGVec2                  ContentSizeIdeal;
    KGVec2                  ContentSizeExplicit;                // Size of contents/scrollable client area explicitly request by the user via SetNextWindowContentSize().
    KGVec2                  WindowPadding;                      // Window padding at the time of Begin().
    KGU32                   BgColor;                            // Client specified window background color
    float                   WindowRounding;                     // Window rounding at the time of Begin(). May be clamped lower to avoid rendering artifacts with title bar, menu bar etc.
    float                   WindowBorderSize;                   // Window border size at the time of Begin().
    float                   DecoOuterSizeX1, DecoOuterSizeY1;   // Left/Up offsets. Sum of non-scrolling outer decorations (X1 generally == 0.0f. Y1 generally = TitleBarHeight + MenuBarHeight). Locked during Begin().
    float                   DecoOuterSizeX2, DecoOuterSizeY2;   // Right/Down offsets (X2 generally == ScrollbarSize.x, Y2 == ScrollbarSizes.y).
    float                   DecoInnerSizeX1, DecoInnerSizeY1;   // Applied AFTER/OVER InnerRect. Specialized for Tables as they use specialized form of clipping and frozen rows/columns are inside InnerRect (and not part of regular decoration sizes).
    int                     NameBufLen;                         // Size of buffer storing Name. May be larger than strlen(Name)!
    KGGuiID                 MoveId;                             // == window->GetID("#MOVE")
    KGGuiID                 TabId;                              // == window->GetID("#TAB")
    KGGuiID                 ChildId;                            // ID of corresponding item in parent window (for navigation to return from child window to parent window)
    KGVec2                  Scroll;
    KGVec2                  ScrollMax;
    KGVec2                  ScrollTarget;                       // target scroll position. stored as cursor position with scrolling canceled out, so the highest point is always 0.0f. (FLT_MAX for no change)
    KGVec2                  ScrollTargetCenterRatio;            // 0.0f = scroll so that target position is at top, 0.5f = scroll so that target position is centered
    KGVec2                  ScrollTargetEdgeSnapDist;           // 0.0f = no snapping, >0.0f snapping threshold
    KGVec2                  ScrollbarSizes;                     // Size taken by each scrollbars on their smaller axis. Pay attention! ScrollbarSizes.x == width of the vertical scrollbar, ScrollbarSizes.y = height of the horizontal scrollbar.
    bool                    bUseCustomBgColor;                  // Shoule we use custom (client specified) bg color for window?
    bool                    ScrollbarX, ScrollbarY;             // Are scrollbars visible?
    bool                    ViewportOwned;
    bool                    Active;                             // Set to true on Begin(), unless Collapsed
    bool                    WasActive;
    bool                    WriteAccessed;                      // Set to true when any widget access the current window
    bool                    Collapsed;                          // Set when collapsing window to become only title-bar
    bool                    WantCollapseToggle;
    bool                    SkipItems;                          // Set when items can safely be all clipped (e.g. window not visible or collapsed)
    bool                    Appearing;                          // Set during the frame where the window is appearing (or re-appearing)
    bool                    Hidden;                             // Do not display (== HiddenFrames*** > 0)
    bool                    IsFallbackWindow;                   // Set on the "Debug##Default" window.
    bool                    IsExplicitChild;                    // Set when passed _ChildWindow, left to false by BeginDocked()
    bool                    HasCloseButton;                     // Set when the window has a close button (p_open != NULL)
    signed char             ResizeBorderHeld;                   // Current border being held for resize (-1: none, otherwise 0-3)
    short                   BeginCount;                         // Number of Begin() during the current frame (generally 0 or 1, 1+ if appending via multiple Begin/End pairs)
    short                   BeginCountPreviousFrame;            // Number of Begin() during the previous frame
    short                   BeginOrderWithinParent;             // Begin() order within immediate parent window, if we are a child window. Otherwise 0.
    short                   BeginOrderWithinContext;            // Begin() order within entire imgui context. This is mostly used for debugging submission order related issues.
    short                   FocusOrder;                         // Order within WindowsFocusOrder[], altered when windows are focused.
    KGGuiID                 PopupId;                            // ID in the popup stack when this window is used as a popup/menu (because we use generic Name/ID for recycling)
    KGS8                    AutoFitFramesX, AutoFitFramesY;
    KGS8                    AutoFitChildAxises;
    bool                    AutoFitOnlyGrows;
    KarmaGuiDir                AutoPosLastDirection;
    KGS8                    HiddenFramesCanSkipItems;           // Hide the window for N frames
    KGS8                    HiddenFramesCannotSkipItems;        // Hide the window for N frames while allowing items to be submitted so we can measure their size
    KGS8                    HiddenFramesForRenderOnly;          // Hide the window until frame N at Render() time only
    KGS8                    DisableInputsFrames;                // Disable window interactions for N frames
    KarmaGuiCond               SetWindowPosAllowFlags : 8;         // store acceptable condition flags for SetNextWindowPos() use.
    KarmaGuiCond               SetWindowSizeAllowFlags : 8;        // store acceptable condition flags for SetNextWindowSize() use.
    KarmaGuiCond               SetWindowCollapsedAllowFlags : 8;   // store acceptable condition flags for SetNextWindowCollapsed() use.
    KarmaGuiCond               SetWindowDockAllowFlags : 8;        // store acceptable condition flags for SetNextWindowDock() use.
    KGVec2                  SetWindowPosVal;                    // store window position when using a non-zero Pivot (position set needs to be processed when we know the window size)
    KGVec2                  SetWindowPosPivot;                  // store window pivot for positioning. KGVec2(0, 0) when positioning from top-left corner; KGVec2(0.5f, 0.5f) for centering; KGVec2(1, 1) for bottom right.

    KGVector<KGGuiID>       IDStack;                            // ID stack. ID are hashes seeded with the value at the top of the stack. (In theory this should be in the TempData structure)
    KGGuiWindowTempData     DC;                                 // Temporary per-window data, reset at the beginning of the frame. This used to be called ImGuiDrawContext, hence the "DC" variable name.

    // The best way to understand what those rectangles are is to use the 'Metrics->Tools->Show Windows Rectangles' viewer.
    // The main 'OuterRect', omitted as a field, is window->Rect().
    KGRect                  OuterRectClipped;                   // == Window->Rect() just after setup in Begin(). == window->Rect() for root window.
    KGRect                  InnerRect;                          // Inner rectangle (omit title bar, menu bar, scroll bar)
    KGRect                  InnerClipRect;                      // == InnerRect shrunk by WindowPadding*0.5f on each side, clipped within viewport or parent clip rect.
    KGRect                  WorkRect;                           // Initially covers the whole scrolling region. Reduced by containers e.g columns/tables when active. Shrunk by WindowPadding*1.0f on each side. This is meant to replace ContentRegionRect over time (from 1.71+ onward).
    KGRect                  ParentWorkRect;                     // Backup of WorkRect before entering a container such as columns/tables. Used by e.g. SpanAllColumns functions to easily access. Stacked containers are responsible for maintaining this. // FIXME-WORKRECT: Could be a stack?
    KGRect                  ClipRect;                           // Current clipping/scissoring rectangle, evolve as we are using PushClipRect(), etc. == DrawList->clip_rect_stack.back().
    KGRect                  ContentRegionRect;                  // FIXME: This is currently confusing/misleading. It is essentially WorkRect but not handling of scrolling. We currently rely on it as right/bottom aligned sizing operation need some size to rely on.
    KGVec2ih                HitTestHoleSize;                    // Define an optional rectangular hole where mouse will pass-through the window.
    KGVec2ih                HitTestHoleOffset;

    int                     LastFrameActive;                    // Last frame number the window was Active.
    int                     LastFrameJustFocused;               // Last frame number the window was made Focused.
    float                   LastTimeActive;                     // Last timestamp the window was Active (using float as we don't need high precision there)
    float                   ItemWidthDefault;
    KarmaGuiStorage            StateStorage;
    KGVector<KGGuiOldColumns> ColumnsStorage;
    float                   FontWindowScale;                    // User scale multiplier per-window, via SetWindowFontScale()
    float                   FontDpiScale;
    int                     SettingsOffset;                     // Offset into SettingsWindows[] (offsets are always valid as we only grow the array from the back)

    KGDrawList*             DrawList;                           // == &DrawListInst (for backward compatibility reason with code using imgui_internal.h we keep this a pointer)
    KGDrawList              DrawListInst;
    KGGuiWindow*            ParentWindow;                       // If we are a child _or_ popup _or_ docked window, this is pointing to our parent. Otherwise NULL.
    KGGuiWindow*            ParentWindowInBeginStack;
    KGGuiWindow*            RootWindow;                         // Point to ourself or first ancestor that is not a child window. Doesn't cross through popups/dock nodes.
    KGGuiWindow*            RootWindowPopupTree;                // Point to ourself or first ancestor that is not a child window. Cross through popups parent<>child.
    KGGuiWindow*            RootWindowDockTree;                 // Point to ourself or first ancestor that is not a child window. Cross through dock nodes.
    KGGuiWindow*            RootWindowForTitleBarHighlight;     // Point to ourself or first ancestor which will display TitleBgActive color when this window is active.
    KGGuiWindow*            RootWindowForNav;                   // Point to ourself or first ancestor which doesn't have the NavFlattened flag.

    KGGuiWindow*            NavLastChildNavWindow;              // When going to the menu bar, we remember the child window we came from. (This could probably be made implicit if we kept g.Windows sorted by last focused including child window.)
    KGGuiID                 NavLastIds[KGGuiNavLayer_COUNT];    // Last known NavId for this window, per layer (0/1)
    KGRect                  NavRectRel[KGGuiNavLayer_COUNT];    // Reference rectangle, in window relative space
    KGGuiID                 NavRootFocusScopeId;                // Focus Scope ID at the time of Begin()

    int                     MemoryDrawListIdxCapacity;          // Backup of last idx/vtx count, so when waking up the window we can preallocate and avoid iterative alloc/copy
    int                     MemoryDrawListVtxCapacity;
    bool                    MemoryCompacted;                    // Set when window extraneous data have been garbage collected

    // Docking
    bool                    DockIsActive        :1;             // When docking artifacts are actually visible. When this is set, DockNode is guaranteed to be != NULL. ~~ (DockNode != NULL) && (DockNode->Windows.Size > 1).
    bool                    DockNodeIsVisible   :1;
    bool                    DockTabIsVisible    :1;             // Is our window visible this frame? ~~ is the corresponding tab selected?
    bool                    DockTabWantClose    :1;
    short                   DockOrder;                          // Order of the last time the window was visible within its DockNode. This is used to reorder windows that are reappearing on the same frame. Same value between windows that were active and windows that were none are possible.
    KGGuiWindowDockStyle    DockStyle;
    KGGuiDockNode*          DockNode;                           // Which node are we docked into. Important: Prefer testing DockIsActive in many cases as this will still be set when the dock node is hidden.
    KGGuiDockNode*          DockNodeAsHost;                     // Which node are we owning (for parent windows)
    KGGuiID                 DockId;                             // Backup of last valid DockNode->ID, so single window remember their dock node id even when they are not bound any more
    KGGuiItemStatusFlags    DockTabItemStatusFlags;
    KGRect                  DockTabItemRect;

public:
    KGGuiWindow(KarmaGuiContext* context, const char* name);
    ~KGGuiWindow();

    KGGuiID     GetID(const char* str, const char* str_end = NULL);
    KGGuiID     GetID(const void* ptr);
    KGGuiID     GetID(int n);
    KGGuiID     GetIDFromRectangle(const KGRect& r_abs);

    // We don't use g.FontSize because the window may be != g.CurrentWindow.
    KGRect      Rect() const            { return KGRect(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y); }
    float       CalcFontSize() const    { KarmaGuiContext& g = *GKarmaGui; float scale = g.FontBaseSize * FontWindowScale * FontDpiScale; if (ParentWindow) scale *= ParentWindow->FontWindowScale; return scale; }
    float       TitleBarHeight() const  { KarmaGuiContext& g = *GKarmaGui; return (Flags & KGGuiWindowFlags_NoTitleBar) ? 0.0f : CalcFontSize() + g.Style.FramePadding.y * 2.0f; }
    KGRect      TitleBarRect() const    { return KGRect(Pos, KGVec2(Pos.x + SizeFull.x, Pos.y + TitleBarHeight())); }
    float       MenuBarHeight() const   { KarmaGuiContext& g = *GKarmaGui; return (Flags & KGGuiWindowFlags_MenuBar) ? DC.MenuBarOffset.y + CalcFontSize() + g.Style.FramePadding.y * 2.0f : 0.0f; }
    KGRect      MenuBarRect() const     { float y1 = Pos.y + TitleBarHeight(); return KGRect(Pos.x, y1, Pos.x + SizeFull.x, y1 + MenuBarHeight()); }
};

//-----------------------------------------------------------------------------
// [SECTION] Tab bar, Tab item support
//-----------------------------------------------------------------------------

// Extend KGGuiTabBarFlags_
enum KGGuiTabBarFlagsPrivate_
{
    KGGuiTabBarFlags_DockNode                   = 1 << 20,  // Part of a dock node [we don't use this in the master branch but it facilitate branch syncing to keep this around]
    KGGuiTabBarFlags_IsFocused                  = 1 << 21,
    KGGuiTabBarFlags_SaveSettings               = 1 << 22,  // FIXME: Settings are handled by the docking system, this only request the tab bar to mark settings dirty when reordering tabs
};

// Extend KGGuiTabItemFlags_
enum KGGuiTabItemFlagsPrivate_
{
    KGGuiTabItemFlags_SectionMask_              = KGGuiTabItemFlags_Leading | KGGuiTabItemFlags_Trailing,
    KGGuiTabItemFlags_NoCloseButton             = 1 << 20,  // Track whether p_open was set or not (we'll need this info on the next frame to recompute ContentWidth during layout)
    KGGuiTabItemFlags_Button                    = 1 << 21,  // Used by TabItemButton, change the tab item behavior to mimic a button
    KGGuiTabItemFlags_Unsorted                  = 1 << 22,  // [Docking] Trailing tabs with the _Unsorted flag will be sorted based on the DockOrder of their Window.
    KGGuiTabItemFlags_Preview                   = 1 << 23,  // [Docking] Display tab shape for docking preview (height is adjusted slightly to compensate for the yet missing tab bar)
};

// Storage for one active tab item (sizeof() 48 bytes)
struct KGGuiTabItem
{
    KGGuiID             ID;
    KarmaGuiTabItemFlags   Flags;
    KGGuiWindow*        Window;                 // When TabItem is part of a DockNode's TabBar, we hold on to a window.
    int                 LastFrameVisible;
    int                 LastFrameSelected;      // This allows us to infer an ordered list of the last activated tabs with little maintenance
    float               Offset;                 // Position relative to beginning of tab
    float               Width;                  // Width currently displayed
    float               ContentWidth;           // Width of label, stored during BeginTabItem() call
    float               RequestedWidth;         // Width optionally requested by caller, -1.0f is unused
    KGS32               NameOffset;             // When Window==NULL, offset to name within parent KGGuiTabBar::TabsNames
    KGS16               BeginOrder;             // BeginTabItem() order, used to re-order tabs after toggling KGGuiTabBarFlags_Reorderable
    KGS16               IndexDuringLayout;      // Index only used during TabBarLayout()
    bool                WantClose;              // Marked as closed by SetTabItemClosed()

    KGGuiTabItem()      { memset(this, 0, sizeof(*this)); LastFrameVisible = LastFrameSelected = -1; RequestedWidth = -1.0f; NameOffset = -1; BeginOrder = IndexDuringLayout = -1; }
};

// Storage for a tab bar (sizeof() 152 bytes)
struct  KGGuiTabBar
{
    KGVector<KGGuiTabItem> Tabs;
    KarmaGuiTabBarFlags    Flags;
    KGGuiID             ID;                     // Zero for tab-bars used by docking
    KGGuiID             SelectedTabId;          // Selected tab/window
    KGGuiID             NextSelectedTabId;      // Next selected tab/window. Will also trigger a scrolling animation
    KGGuiID             VisibleTabId;           // Can occasionally be != SelectedTabId (e.g. when previewing contents for CTRL+TAB preview)
    int                 CurrFrameVisible;
    int                 PrevFrameVisible;
    KGRect              BarRect;
    float               CurrTabsContentsHeight;
    float               PrevTabsContentsHeight; // Record the height of contents submitted below the tab bar
    float               WidthAllTabs;           // Actual width of all tabs (locked during layout)
    float               WidthAllTabsIdeal;      // Ideal width if all tabs were visible and not clipped
    float               ScrollingAnim;
    float               ScrollingTarget;
    float               ScrollingTargetDistToVisibility;
    float               ScrollingSpeed;
    float               ScrollingRectMinX;
    float               ScrollingRectMaxX;
    KGGuiID             ReorderRequestTabId;
    KGS16               ReorderRequestOffset;
    KGS8                BeginCount;
    bool                WantLayout;
    bool                VisibleTabWasSubmitted;
    bool                TabsAddedNew;           // Set to true when a new tab item or button has been added to the tab bar during last frame
    KGS16               TabsActiveCount;        // Number of tabs submitted this frame.
    KGS16               LastTabItemIdx;         // Index of last BeginTabItem() tab for use by EndTabItem()
    float               ItemSpacingY;
    KGVec2              FramePadding;           // style.FramePadding locked at the time of BeginTabBar()
    KGVec2              BackupCursorPos;
    KarmaGuiTextBuffer     TabsNames;              // For non-docking tab bar we re-append names in a contiguous buffer.

    KGGuiTabBar();
    int                 GetTabOrder(const KGGuiTabItem* tab) const  { return Tabs.index_from_ptr(tab); }
    const char*         GetTabName(const KGGuiTabItem* tab) const
    {
        if (tab->Window)
            return tab->Window->Name;
        KR_CORE_ASSERT(tab->NameOffset != -1 && tab->NameOffset < TabsNames.Buf.Size, "");
        return TabsNames.Buf.Data + tab->NameOffset;
    }
};

//-----------------------------------------------------------------------------
// [SECTION] Table support
//-----------------------------------------------------------------------------

#define KG_COL32_DISABLE                KG_COL32(0,0,0,1)   // Special sentinel code which cannot be used as a regular color.
#define KARMAGUI_TABLE_MAX_COLUMNS         64                  // sizeof(KGU64) * 8. This is solely because we frequently encode columns set in a KGU64.
#define KARMAGUI_TABLE_MAX_DRAW_CHANNELS   (4 + 64 * 2)        // See TableSetupDrawChannels()

// Our current column maximum is 64 but we may raise that in the future.
typedef KGS8 KGGuiTableColumnIdx;
typedef KGU8 KGGuiTableDrawChannelIdx;

// [Internal] sizeof() ~ 104
// We use the terminology "Enabled" to refer to a column that is not Hidden by user/api.
// We use the terminology "Clipped" to refer to a column that is out of sight because of scrolling/clipping.
// This is in contrast with some user-facing api such as IsItemVisible() / IsRectVisible() which use "Visible" to mean "not clipped".
struct KGGuiTableColumn
{
    KarmaGuiTableColumnFlags   Flags;                          // Flags after some patching (not directly same as provided by user). See KGGuiTableColumnFlags_
    float                   WidthGiven;                     // Final/actual width visible == (MaxX - MinX), locked in TableUpdateLayout(). May be > WidthRequest to honor minimum width, may be < WidthRequest to honor shrinking columns down in tight space.
    float                   MinX;                           // Absolute positions
    float                   MaxX;
    float                   WidthRequest;                   // Master width absolute value when !(Flags & _WidthStretch). When Stretch this is derived every frame from StretchWeight in TableUpdateLayout()
    float                   WidthAuto;                      // Automatic width
    float                   StretchWeight;                  // Master width weight when (Flags & _WidthStretch). Often around ~1.0f initially.
    float                   InitStretchWeightOrWidth;       // Value passed to TableSetupColumn(). For Width it is a content width (_without padding_).
    KGRect                  ClipRect;                       // Clipping rectangle for the column
    KGGuiID                 UserID;                         // Optional, value passed to TableSetupColumn()
    float                   WorkMinX;                       // Contents region min ~(MinX + CellPaddingX + CellSpacingX1) == cursor start position when entering column
    float                   WorkMaxX;                       // Contents region max ~(MaxX - CellPaddingX - CellSpacingX2)
    float                   ItemWidth;                      // Current item width for the column, preserved across rows
    float                   ContentMaxXFrozen;              // Contents maximum position for frozen rows (apart from headers), from which we can infer content width.
    float                   ContentMaxXUnfrozen;
    float                   ContentMaxXHeadersUsed;         // Contents maximum position for headers rows (regardless of freezing). TableHeader() automatically softclip itself + report ideal desired size, to avoid creating extraneous draw calls
    float                   ContentMaxXHeadersIdeal;
    KGS16                   NameOffset;                     // Offset into parent ColumnsNames[]
    KGGuiTableColumnIdx     DisplayOrder;                   // Index within Table's IndexToDisplayOrder[] (column may be reordered by users)
    KGGuiTableColumnIdx     IndexWithinEnabledSet;          // Index within enabled/visible set (<= IndexToDisplayOrder)
    KGGuiTableColumnIdx     PrevEnabledColumn;              // Index of prev enabled/visible column within Columns[], -1 if first enabled/visible column
    KGGuiTableColumnIdx     NextEnabledColumn;              // Index of next enabled/visible column within Columns[], -1 if last enabled/visible column
    KGGuiTableColumnIdx     SortOrder;                      // Index of this column within sort specs, -1 if not sorting on this column, 0 for single-sort, may be >0 on multi-sort
    KGGuiTableDrawChannelIdx DrawChannelCurrent;            // Index within DrawSplitter.Channels[]
    KGGuiTableDrawChannelIdx DrawChannelFrozen;             // Draw channels for frozen rows (often headers)
    KGGuiTableDrawChannelIdx DrawChannelUnfrozen;           // Draw channels for unfrozen rows
    bool                    IsEnabled;                      // IsUserEnabled && (Flags & KGGuiTableColumnFlags_Disabled) == 0
    bool                    IsUserEnabled;                  // Is the column not marked Hidden by the user? (unrelated to being off view, e.g. clipped by scrolling).
    bool                    IsUserEnabledNextFrame;
    bool                    IsVisibleX;                     // Is actually in view (e.g. overlapping the host window clipping rectangle, not scrolled).
    bool                    IsVisibleY;
    bool                    IsRequestOutput;                // Return value for TableSetColumnIndex() / TableNextColumn(): whether we request user to output contents or not.
    bool                    IsSkipItems;                    // Do we want item submissions to this column to be completely ignored (no layout will happen).
    bool                    IsPreserveWidthAuto;
    KGS8                    NavLayerCurrent;                // KGGuiNavLayer in 1 byte
    KGU8                    AutoFitQueue;                   // Queue of 8 values for the next 8 frames to request auto-fit
    KGU8                    CannotSkipItemsQueue;           // Queue of 8 values for the next 8 frames to disable Clipped/SkipItem
    KGU8                    SortDirection : 2;              // KGGuiSortDirection_Ascending or KGGuiSortDirection_Descending
    KGU8                    SortDirectionsAvailCount : 2;   // Number of available sort directions (0 to 3)
    KGU8                    SortDirectionsAvailMask : 4;    // Mask of available sort directions (1-bit each)
    KGU8                    SortDirectionsAvailList;        // Ordered of available sort directions (2-bits each)

    KGGuiTableColumn()
    {
        memset(this, 0, sizeof(*this));
        StretchWeight = WidthRequest = -1.0f;
        NameOffset = -1;
        DisplayOrder = IndexWithinEnabledSet = -1;
        PrevEnabledColumn = NextEnabledColumn = -1;
        SortOrder = -1;
        SortDirection = KGGuiSortDirection_None;
        DrawChannelCurrent = DrawChannelFrozen = DrawChannelUnfrozen = (KGU8)-1;
    }
};

// Transient cell data stored per row.
// sizeof() ~ 6
struct KGGuiTableCellData
{
    KGU32                       BgColor;    // Actual color
    KGGuiTableColumnIdx         Column;     // Column number
};

// Per-instance data that needs preserving across frames (seemingly most others do not need to be preserved aside from debug needs, does that needs they could be moved to KGGuiTableTempData ?)
struct KGGuiTableInstanceData
{
    float                       LastOuterHeight;            // Outer height from last frame
    float                       LastFirstRowHeight;         // Height of first row from last frame (FIXME: this is used as "header height" and may be reworked)
    float                       LastFrozenHeight;           // Height of frozen section from last frame

    KGGuiTableInstanceData()    { LastOuterHeight = LastFirstRowHeight = LastFrozenHeight = 0.0f; }
};

// FIXME-TABLE: more transient data could be stored in a stacked KGGuiTableTempData: e.g. SortSpecs, incoming RowData
struct  KGGuiTable
{
    KGGuiID                     ID;
    KarmaGuiTableFlags             Flags;
    void*                       RawData;                    // Single allocation to hold Columns[], DisplayOrderToIndex[] and RowCellData[]
    KGGuiTableTempData*         TempData;                   // Transient data while table is active. Point within g.CurrentTableStack[]
    KGSpan<KGGuiTableColumn>    Columns;                    // Point within RawData[]
    KGSpan<KGGuiTableColumnIdx> DisplayOrderToIndex;        // Point within RawData[]. Store display order of columns (when not reordered, the values are 0...Count-1)
    KGSpan<KGGuiTableCellData>  RowCellData;                // Point within RawData[]. Store cells background requests for current row.
    KGU64                       EnabledMaskByDisplayOrder;  // Column DisplayOrder -> IsEnabled map
    KGU64                       EnabledMaskByIndex;         // Column Index -> IsEnabled map (== not hidden by user/api) in a format adequate for iterating column without touching cold data
    KGU64                       VisibleMaskByIndex;         // Column Index -> IsVisibleX|IsVisibleY map (== not hidden by user/api && not hidden by scrolling/cliprect)
    KGU64                       RequestOutputMaskByIndex;   // Column Index -> IsVisible || AutoFit (== expect user to submit items)
    KarmaGuiTableFlags             SettingsLoadedFlags;        // Which data were loaded from the .ini file (e.g. when order is not altered we won't save order)
    int                         SettingsOffset;             // Offset in g.SettingsTables
    int                         LastFrameActive;
    int                         ColumnsCount;               // Number of columns declared in BeginTable()
    int                         CurrentRow;
    int                         CurrentColumn;
    KGS16                       InstanceCurrent;            // Count of BeginTable() calls with same ID in the same frame (generally 0). This is a little bit similar to BeginCount for a window, but multiple table with same ID look are multiple tables, they are just synched.
    KGS16                       InstanceInteracted;         // Mark which instance (generally 0) of the same ID is being interacted with
    float                       RowPosY1;
    float                       RowPosY2;
    float                       RowMinHeight;               // Height submitted to TableNextRow()
    float                       RowTextBaseline;
    float                       RowIndentOffsetX;
    KarmaGuiTableRowFlags          RowFlags : 16;              // Current row flags, see KGGuiTableRowFlags_
    KarmaGuiTableRowFlags          LastRowFlags : 16;
    int                         RowBgColorCounter;          // Counter for alternating background colors (can be fast-forwarded by e.g clipper), not same as CurrentRow because header rows typically don't increase this.
    KGU32                       RowBgColor[2];              // Background color override for current row.
    KGU32                       BorderColorStrong;
    KGU32                       BorderColorLight;
    float                       BorderX1;
    float                       BorderX2;
    float                       HostIndentX;
    float                       MinColumnWidth;
    float                       OuterPaddingX;
    float                       CellPaddingX;               // Padding from each borders
    float                       CellPaddingY;
    float                       CellSpacingX1;              // Spacing between non-bordered cells
    float                       CellSpacingX2;
    float                       InnerWidth;                 // User value passed to BeginTable(), see comments at the top of BeginTable() for details.
    float                       ColumnsGivenWidth;          // Sum of current column width
    float                       ColumnsAutoFitWidth;        // Sum of ideal column width in order nothing to be clipped, used for auto-fitting and content width submission in outer window
    float                       ColumnsStretchSumWeights;   // Sum of weight of all enabled stretching columns
    float                       ResizedColumnNextWidth;
    float                       ResizeLockMinContentsX2;    // Lock minimum contents width while resizing down in order to not create feedback loops. But we allow growing the table.
    float                       RefScale;                   // Reference scale to be able to rescale columns on font/dpi changes.
    KGRect                      OuterRect;                  // Note: for non-scrolling table, OuterRect.Max.y is often FLT_MAX until EndTable(), unless a height has been specified in BeginTable().
    KGRect                      InnerRect;                  // InnerRect but without decoration. As with OuterRect, for non-scrolling tables, InnerRect.Max.y is
    KGRect                      WorkRect;
    KGRect                      InnerClipRect;
    KGRect                      BgClipRect;                 // We use this to cpu-clip cell background color fill, evolve during the frame as we cross frozen rows boundaries
    KGRect                      Bg0ClipRectForDrawCmd;      // Actual KGDrawCmd clip rect for BG0/1 channel. This tends to be == OuterWindow->ClipRect at BeginTable() because output in BG0/BG1 is cpu-clipped
    KGRect                      Bg2ClipRectForDrawCmd;      // Actual KGDrawCmd clip rect for BG2 channel. This tends to be a correct, tight-fit, because output to BG2 are done by widgets relying on regular ClipRect.
    KGRect                      HostClipRect;               // This is used to check if we can eventually merge our columns draw calls into the current draw call of the current window.
    KGRect                      HostBackupInnerClipRect;    // Backup of InnerWindow->ClipRect during PushTableBackground()/PopTableBackground()
    KGGuiWindow*                OuterWindow;                // Parent window for the table
    KGGuiWindow*                InnerWindow;                // Window holding the table data (== OuterWindow or a child window)
    KarmaGuiTextBuffer             ColumnsNames;               // Contiguous buffer holding columns names
    KGDrawListSplitter*         DrawSplitter;               // Shortcut to TempData->DrawSplitter while in table. Isolate draw commands per columns to avoid switching clip rect constantly
    KGGuiTableInstanceData      InstanceDataFirst;
    KGVector<KGGuiTableInstanceData>    InstanceDataExtra;  // FIXME-OPT: Using a small-vector pattern would be good.
    KarmaGuiTableColumnSortSpecs   SortSpecsSingle;
    KGVector<KarmaGuiTableColumnSortSpecs> SortSpecsMulti;     // FIXME-OPT: Using a small-vector pattern would be good.
    KarmaGuiTableSortSpecs         SortSpecs;                  // Public facing sorts specs, this is what we return in TableGetSortSpecs()
    KGGuiTableColumnIdx         SortSpecsCount;
    KGGuiTableColumnIdx         ColumnsEnabledCount;        // Number of enabled columns (<= ColumnsCount)
    KGGuiTableColumnIdx         ColumnsEnabledFixedCount;   // Number of enabled columns (<= ColumnsCount)
    KGGuiTableColumnIdx         DeclColumnsCount;           // Count calls to TableSetupColumn()
    KGGuiTableColumnIdx         HoveredColumnBody;          // Index of column whose visible region is being hovered. Important: == ColumnsCount when hovering empty region after the right-most column!
    KGGuiTableColumnIdx         HoveredColumnBorder;        // Index of column whose right-border is being hovered (for resizing).
    KGGuiTableColumnIdx         AutoFitSingleColumn;        // Index of single column requesting auto-fit.
    KGGuiTableColumnIdx         ResizedColumn;              // Index of column being resized. Reset when InstanceCurrent==0.
    KGGuiTableColumnIdx         LastResizedColumn;          // Index of column being resized from previous frame.
    KGGuiTableColumnIdx         HeldHeaderColumn;           // Index of column header being held.
    KGGuiTableColumnIdx         ReorderColumn;              // Index of column being reordered. (not cleared)
    KGGuiTableColumnIdx         ReorderColumnDir;           // -1 or +1
    KGGuiTableColumnIdx         LeftMostEnabledColumn;      // Index of left-most non-hidden column.
    KGGuiTableColumnIdx         RightMostEnabledColumn;     // Index of right-most non-hidden column.
    KGGuiTableColumnIdx         LeftMostStretchedColumn;    // Index of left-most stretched column.
    KGGuiTableColumnIdx         RightMostStretchedColumn;   // Index of right-most stretched column.
    KGGuiTableColumnIdx         ContextPopupColumn;         // Column right-clicked on, of -1 if opening context menu from a neutral/empty spot
    KGGuiTableColumnIdx         FreezeRowsRequest;          // Requested frozen rows count
    KGGuiTableColumnIdx         FreezeRowsCount;            // Actual frozen row count (== FreezeRowsRequest, or == 0 when no scrolling offset)
    KGGuiTableColumnIdx         FreezeColumnsRequest;       // Requested frozen columns count
    KGGuiTableColumnIdx         FreezeColumnsCount;         // Actual frozen columns count (== FreezeColumnsRequest, or == 0 when no scrolling offset)
    KGGuiTableColumnIdx         RowCellDataCurrent;         // Index of current RowCellData[] entry in current row
    KGGuiTableDrawChannelIdx    DummyDrawChannel;           // Redirect non-visible columns here.
    KGGuiTableDrawChannelIdx    Bg2DrawChannelCurrent;      // For Selectable() and other widgets drawing across columns after the freezing line. Index within DrawSplitter.Channels[]
    KGGuiTableDrawChannelIdx    Bg2DrawChannelUnfrozen;
    bool                        IsLayoutLocked;             // Set by TableUpdateLayout() which is called when beginning the first row.
    bool                        IsInsideRow;                // Set when inside TableBeginRow()/TableEndRow().
    bool                        IsInitializing;
    bool                        IsSortSpecsDirty;
    bool                        IsUsingHeaders;             // Set when the first row had the KGGuiTableRowFlags_Headers flag.
    bool                        IsContextPopupOpen;         // Set when default context menu is open (also see: ContextPopupColumn, InstanceInteracted).
    bool                        IsSettingsRequestLoad;
    bool                        IsSettingsDirty;            // Set when table settings have changed and needs to be reported into ImGuiTableSetttings data.
    bool                        IsDefaultDisplayOrder;      // Set when display order is unchanged from default (DisplayOrder contains 0...Count-1)
    bool                        IsResetAllRequest;
    bool                        IsResetDisplayOrderRequest;
    bool                        IsUnfrozenRows;             // Set when we got past the frozen row.
    bool                        IsDefaultSizingPolicy;      // Set if user didn't explicitly set a sizing policy in BeginTable()
    bool                        HasScrollbarYCurr;          // Whether ANY instance of this table had a vertical scrollbar during the current frame.
    bool                        HasScrollbarYPrev;          // Whether ANY instance of this table had a vertical scrollbar during the previous.
    bool                        MemoryCompacted;
    bool                        HostSkipItems;              // Backup of InnerWindow->SkipItem at the end of BeginTable(), because we will overwrite InnerWindow->SkipItem on a per-column basis

    KGGuiTable()                { memset(this, 0, sizeof(*this)); LastFrameActive = -1; }
    ~KGGuiTable()               { KG_FREE(RawData); }
};

// Transient data that are only needed between BeginTable() and EndTable(), those buffers are shared (1 per level of stacked table).
// - Accessing those requires chasing an extra pointer so for very frequently used data we leave them in the main table structure.
// - We also leave out of this structure data that tend to be particularly useful for debugging/metrics.
struct  KGGuiTableTempData
{
    int                         TableIndex;                 // Index in g.Tables.Buf[] pool
    float                       LastTimeActive;             // Last timestamp this structure was used

    KGVec2                      UserOuterSize;              // outer_size.x passed to BeginTable()
    KGDrawListSplitter          DrawSplitter;

    KGRect                      HostBackupWorkRect;         // Backup of InnerWindow->WorkRect at the end of BeginTable()
    KGRect                      HostBackupParentWorkRect;   // Backup of InnerWindow->ParentWorkRect at the end of BeginTable()
    KGVec2                      HostBackupPrevLineSize;     // Backup of InnerWindow->DC.PrevLineSize at the end of BeginTable()
    KGVec2                      HostBackupCurrLineSize;     // Backup of InnerWindow->DC.CurrLineSize at the end of BeginTable()
    KGVec2                      HostBackupCursorMaxPos;     // Backup of InnerWindow->DC.CursorMaxPos at the end of BeginTable()
    KGVec1                      HostBackupColumnsOffset;    // Backup of OuterWindow->DC.ColumnsOffset at the end of BeginTable()
    float                       HostBackupItemWidth;        // Backup of OuterWindow->DC.ItemWidth at the end of BeginTable()
    int                         HostBackupItemWidthStackSize;//Backup of OuterWindow->DC.ItemWidthStack.Size at the end of BeginTable()

    KGGuiTableTempData()        { memset(this, 0, sizeof(*this)); LastTimeActive = -1.0f; }
};

// sizeof() ~ 12
struct KGGuiTableColumnSettings
{
    float                   WidthOrWeight;
    KGGuiID                 UserID;
    KGGuiTableColumnIdx     Index;
    KGGuiTableColumnIdx     DisplayOrder;
    KGGuiTableColumnIdx     SortOrder;
    KGU8                    SortDirection : 2;
    KGU8                    IsEnabled : 1; // "Visible" in ini file
    KGU8                    IsStretch : 1;

    KGGuiTableColumnSettings()
    {
        WidthOrWeight = 0.0f;
        UserID = 0;
        Index = -1;
        DisplayOrder = SortOrder = -1;
        SortDirection = KGGuiSortDirection_None;
        IsEnabled = 1;
        IsStretch = 0;
    }
};

// This is designed to be stored in a single KGChunkStream (1 header followed by N KGGuiTableColumnSettings, etc.)
struct KGGuiTableSettings
{
    KGGuiID                     ID;                     // Set to 0 to invalidate/delete the setting
    KarmaGuiTableFlags             SaveFlags;              // Indicate data we want to save using the Resizable/Reorderable/Sortable/Hideable flags (could be using its own flags..)
    float                       RefScale;               // Reference scale to be able to rescale columns on font/dpi changes.
    KGGuiTableColumnIdx         ColumnsCount;
    KGGuiTableColumnIdx         ColumnsCountMax;        // Maximum number of columns this settings instance can store, we can recycle a settings instance with lower number of columns but not higher
    bool                        WantApply;              // Set when loaded from .ini data (to enable merging/loading .ini data into an already running context)

    KGGuiTableSettings()        { memset(this, 0, sizeof(*this)); }
    KGGuiTableColumnSettings*   GetColumnSettings()     { return (KGGuiTableColumnSettings*)(this + 1); }
};


namespace Karma
{
	class  KARMA_API KarmaGuiInternal
	{
	public:
		static KarmaGuiContext*   GKarmaGui;
		static KarmaGuiMemAllocFunc    GImAllocatorAllocFunc ;
		static KarmaGuiMemFreeFunc     GImAllocatorFreeFunc;
		static void*                GImAllocatorUserData;
	public:
		// Some helpers defined in KarmaGui.cpp
		static void             SetCurrentWindow(KGGuiWindow* window);
		static void             FindHoveredWindow();
		static KGGuiWindow*     CreateNewWindow(const char* name, KarmaGuiWindowFlags flags);
		static KGVec2           CalcNextScrollFromScrollTargetAndClamp(KGGuiWindow* window);

		static void             AddDrawListToDrawData(KGVector<KGDrawList*>* out_list, KGDrawList* draw_list);
		static void             AddWindowToSortBuffer(KGVector<KGGuiWindow*>* out_sorted_windows, KGGuiWindow* window);

		// Settings
		static void             WindowSettingsHandler_ClearAll(KarmaGuiContext*, KGGuiSettingsHandler*);
		static void*            WindowSettingsHandler_ReadOpen(KarmaGuiContext*, KGGuiSettingsHandler*, const char* name);
		static void             WindowSettingsHandler_ReadLine(KarmaGuiContext*, KGGuiSettingsHandler*, void* entry, const char* line);
		static void             WindowSettingsHandler_ApplyAll(KarmaGuiContext*, KGGuiSettingsHandler*);
		static void             WindowSettingsHandler_WriteAll(KarmaGuiContext*, KGGuiSettingsHandler*, KarmaGuiTextBuffer* buf);

		// Platform Dependents default implementation for IO functions
		static const char*      GetClipboardTextFn_DefaultImpl(void* user_data);
		static void             SetClipboardTextFn_DefaultImpl(void* user_data, const char* text);
		static void             SetPlatformImeDataFn_DefaultImpl(KarmaGuiViewport* viewport, KarmaGuiPlatformImeData* data);

		// Navigation
		static void             NavUpdate();
		static void             NavUpdateWindowing();
		static void             NavUpdateWindowingOverlay();
		static void             NavUpdateCancelRequest();
		static void             NavUpdateCreateMoveRequest();
		static void             NavUpdateCreateTabbingRequest();
		static float            NavUpdatePageUpPageDown();
		static inline void      NavUpdateAnyRequestFlag();
		static void             NavUpdateCreateWrappingRequest();
		static void             NavEndFrame();
		static bool             NavScoreItem(KGGuiNavItemData* result);
		static void             NavApplyItemToResult(KGGuiNavItemData* result);
		static void             NavProcessItem();
		static void             NavProcessItemForTabbingRequest(KGGuiID id);
		static KGVec2           NavCalcPreferredRefPos();
		static void             NavSaveLastChildNavWindowIntoParent(KGGuiWindow* nav_window);
		static KGGuiWindow*     NavRestoreLastChildNavWindow(KGGuiWindow* window);
		static void             NavRestoreLayer(KGGuiNavLayer layer);
		static void             NavRestoreHighlightAfterMove();
		static int              FindWindowFocusIndex(KGGuiWindow* window);

		// Error Checking and Debug Tools
		static void             ErrorCheckNewFrameSanityChecks();
		static void             ErrorCheckEndFrameSanityChecks();
		static void             UpdateDebugToolItemPicker();
		static void             UpdateDebugToolStackQueries();

		// Inputs
		static void             UpdateKeyboardInputs();
		static void             UpdateMouseInputs();
		static void             UpdateMouseWheel();
		static void             UpdateKeyRoutingTable(KGGuiKeyRoutingTable* rt);

		// Misc
		static void             UpdateSettings();
		static bool             UpdateWindowManualResize(KGGuiWindow* window, const KGVec2& size_auto_fit, int* border_held, int resize_grip_count, KGU32 resize_grip_col[4], const KGRect& visibility_rect);
		static void             RenderWindowOuterBorders(KGGuiWindow* window);
		static void             RenderWindowDecorations(KGGuiWindow* window, const KGRect& title_bar_rect, bool title_bar_is_highlight, bool handle_borders_and_resize_grips, int resize_grip_count, const KGU32 resize_grip_col[4], float resize_grip_draw_size);
		static void             RenderWindowTitleBarContents(KGGuiWindow* window, const KGRect& title_bar_rect, const char* name, bool* p_open);
		static void             RenderDimmedBackgroundBehindWindow(KGGuiWindow* window, KGU32 col);
		static void             RenderDimmedBackgrounds();
		static KGGuiWindow*     FindBlockingModal(KGGuiWindow* window);

		// Viewports
		const KGGuiID           IMGUI_VIEWPORT_DEFAULT_ID = 0x11111111; // Using an arbitrary constant instead of e.g. KGHashStr("ViewportDefault", 0); so it's easier to spot in the debugger. The exact value doesn't matter.
		static KGGuiViewportP*  AddUpdateViewport(KGGuiWindow* window, KGGuiID id, const KGVec2& platform_pos, const KGVec2& size, KarmaGuiViewportFlags flags);
		static void             DestroyViewport(KGGuiViewportP* viewport);
		static void             UpdateViewportsNewFrame();
		static void             UpdateViewportsEndFrame();
		static void             WindowSelectViewport(KGGuiWindow* window);
		static void             WindowSyncOwnedViewport(KGGuiWindow* window, KGGuiWindow* parent_window_in_stack);
		static bool             UpdateTryMergeWindowIntoHostViewport(KGGuiWindow* window, KGGuiViewportP* host_viewport);
		static bool             UpdateTryMergeWindowIntoHostViewports(KGGuiWindow* window);
		static bool             GetWindowAlwaysWantOwnViewport(KGGuiWindow* window);
		static int              FindPlatformMonitorForPos(const KGVec2& pos);
		static int              FindPlatformMonitorForRect(const KGRect& r);
		static void             UpdateViewportPlatformMonitor(KGGuiViewportP* viewport);

		static KarmaGuiStorage::ImGuiStoragePair* LowerBound(KGVector<KarmaGuiStorage::ImGuiStoragePair>& data, KGGuiID key);

	public:
		// Windows
		// We should always have a CurrentWindow in the stack (there is an implicit "Debug" window)
		// If this ever crash because g.CurrentWindow is NULL it means that either
		// - ImGui::NewFrame() has never been called, which is illegal.
		// - You are calling ImGui functions after ImGui::EndFrame()/ImGui::Render() and before the next ImGui::NewFrame(), which is also illegal.
		static inline    KGGuiWindow*  GetCurrentWindowRead()      { KarmaGuiContext& g = *GKarmaGui; return g.CurrentWindow; }
		static inline    KGGuiWindow*  GetCurrentWindow()          { KarmaGuiContext& g = *GKarmaGui; g.CurrentWindow->WriteAccessed = true; return g.CurrentWindow; }
		static KGGuiWindow*  FindWindowByID(KGGuiID id);
		static KGGuiWindow*  FindWindowByName(const char* name);
		static void          UpdateWindowParentAndRootLinks(KGGuiWindow* window, KarmaGuiWindowFlags flags, KGGuiWindow* parent_window);
		static KGVec2        CalcWindowNextAutoFitSize(KGGuiWindow* window);
		static bool          IsWindowChildOf(KGGuiWindow* window, KGGuiWindow* potential_parent, bool popup_hierarchy, bool dock_hierarchy);
		static bool          IsWindowWithinBeginStackOf(KGGuiWindow* window, KGGuiWindow* potential_parent);
		static bool          IsWindowAbove(KGGuiWindow* potential_above, KGGuiWindow* potential_below);
		static bool          IsWindowNavFocusable(KGGuiWindow* window);
		static void          SetWindowPos(KGGuiWindow* window, const KGVec2& pos, KarmaGuiCond cond = 0);
		static void          SetWindowSize(KGGuiWindow* window, const KGVec2& size, KarmaGuiCond cond = 0);
		static void          SetWindowCollapsed(KGGuiWindow* window, bool collapsed, KarmaGuiCond cond = 0);
		static void          SetWindowHitTestHole(KGGuiWindow* window, const KGVec2& pos, const KGVec2& size);
		static inline KGRect           WindowRectAbsToRel(KGGuiWindow* window, const KGRect& r) { KGVec2 off = window->DC.CursorStartPos; return KGRect(r.Min.x - off.x, r.Min.y - off.y, r.Max.x - off.x, r.Max.y - off.y); }
		static inline KGRect           WindowRectRelToAbs(KGGuiWindow* window, const KGRect& r) { KGVec2 off = window->DC.CursorStartPos; return KGRect(r.Min.x + off.x, r.Min.y + off.y, r.Max.x + off.x, r.Max.y + off.y); }

		// Windows: Display Order and Focus Order
		static void          FocusWindow(KGGuiWindow* window);
		static void          FocusTopMostWindowUnderOne(KGGuiWindow* under_this_window, KGGuiWindow* ignore_window);
		static void          BringWindowToFocusFront(KGGuiWindow* window);
		static void          BringWindowToDisplayFront(KGGuiWindow* window);
		static void          BringWindowToDisplayBack(KGGuiWindow* window);
		static void          BringWindowToDisplayBehind(KGGuiWindow* window, KGGuiWindow* above_window);
		static int           FindWindowDisplayIndex(KGGuiWindow* window);
		static KGGuiWindow*  FindBottomMostVisibleWindowWithinBeginStack(KGGuiWindow* window);

		// Fonts, drawing
		static void          SetCurrentFont(KGFont* font);
		static inline KGFont*          GetDefaultFont() { KarmaGuiContext& g = *GKarmaGui; return g.IO.FontDefault ? g.IO.FontDefault : g.IO.Fonts->Fonts[0]; }
		static inline KGDrawList*      GetForegroundDrawList(KGGuiWindow* window) { return KarmaGui::GetForegroundDrawList(window->Viewport); }

		// Init
		static void          Initialize();
		static void          Shutdown();    // Since 1.60 this is a _private_ function. You can call DestroyContext() to destroy the context created by CreateContext().

		static // NewFrame
		static void          UpdateInputEvents(bool trickle_fast_inputs);
		static void          UpdateHoveredWindowAndCaptureFlags();
		static void          StartMouseMovingWindow(KGGuiWindow* window);
		static void          StartMouseMovingWindowOrNode(KGGuiWindow* window, KGGuiDockNode* node, bool undock_floating_node);
		static void          UpdateMouseMovingWindowNewFrame();
		static void          UpdateMouseMovingWindowEndFrame();

		// Generic context hooks
		static KGGuiID       AddContextHook(KarmaGuiContext* context, const KGGuiContextHook* hook);
		static void          RemoveContextHook(KarmaGuiContext* context, KGGuiID hook_to_remove);
		static void          CallContextHooks(KarmaGuiContext* context, ImGuiContextHookType type);

		// Viewports
		static void          TranslateWindowsInViewport(KGGuiViewportP* viewport, const KGVec2& old_pos, const KGVec2& new_pos);
		static void          ScaleWindowsInViewport(KGGuiViewportP* viewport, float scale);
		static void          DestroyPlatformWindow(KGGuiViewportP* viewport);
		static void          SetWindowViewport(KGGuiWindow* window, KGGuiViewportP* viewport);
		static void          SetCurrentViewport(KGGuiWindow* window, KGGuiViewportP* viewport);
		static const KarmaGuiPlatformMonitor*   GetViewportPlatformMonitor(KarmaGuiViewport* viewport);
		static KGGuiViewportP*               FindHoveredViewportFromPlatformWindowStack(const KGVec2& mouse_platform_pos);

		// Settings
		static void                  MarkIniSettingsDirty();
		static void                  MarkIniSettingsDirty(KGGuiWindow* window);
		static void                  ClearIniSettings();
		static KGGuiWindowSettings*  CreateNewWindowSettings(const char* name);
		static KGGuiWindowSettings*  FindWindowSettings(KGGuiID id);
		static KGGuiWindowSettings*  FindOrCreateWindowSettings(const char* name);
		static void                  AddSettingsHandler(const KGGuiSettingsHandler* handler);
		static void                  RemoveSettingsHandler(const char* type_name);
		static KGGuiSettingsHandler* FindSettingsHandler(const char* type_name);

		// Localization
		static void          LocalizeRegisterEntries(const KGGuiLocEntry* entries, int count);
		static inline const char*      LocalizeGetMsg(KGGuiLocKey key) { KarmaGuiContext& g = *GKarmaGui; const char* msg = g.LocalizationTable[key]; return msg ? msg : "*Missing Text*"; }

		// Scrolling
		static void          SetScrollX(KGGuiWindow* window, float scroll_x);
		static void          SetScrollY(KGGuiWindow* window, float scroll_y);
		static void          SetScrollFromPosX(KGGuiWindow* window, float local_x, float center_x_ratio);
		static void          SetScrollFromPosY(KGGuiWindow* window, float local_y, float center_y_ratio);

		// Early work-in-progress API (ScrollToItem() will become public)
		static void          ScrollToItem(KGGuiScrollFlags flags = 0);
		static void          ScrollToRect(KGGuiWindow* window, const KGRect& rect, KGGuiScrollFlags flags = 0);
		static KGVec2        ScrollToRectEx(KGGuiWindow* window, const KGRect& rect, KGGuiScrollFlags flags = 0);

		// Basic Accessors
		static inline KGGuiItemStatusFlags GetItemStatusFlags(){ KarmaGuiContext& g = *GKarmaGui; return g.LastItemData.StatusFlags; }
		static inline KGGuiItemFlags   GetItemFlags()  { KarmaGuiContext& g = *GKarmaGui; return g.LastItemData.InFlags; }
		static inline KGGuiID          GetActiveID()   { KarmaGuiContext& g = *GKarmaGui; return g.ActiveId; }
		static inline KGGuiID          GetFocusID()    { KarmaGuiContext& g = *GKarmaGui; return g.NavId; }
		static void          SetActiveID(KGGuiID id, KGGuiWindow* window);
		static void          SetFocusID(KGGuiID id, KGGuiWindow* window);
		static void          ClearActiveID();
		static KGGuiID       GetHoveredID();
		static void          SetHoveredID(KGGuiID id);
		static void          KeepAliveID(KGGuiID id);
		static void          MarkItemEdited(KGGuiID id);     // Mark data associated to given item as "edited", used by IsItemDeactivatedAfterEdit() function.
		static void          PushOverrideID(KGGuiID id);     // Push given value as-is at the top of the ID stack (whereas PushID combines old and new hashes)
		static KGGuiID       GetIDWithSeed(const char* str_id_begin, const char* str_id_end, KGGuiID seed);

		// Basic Helpers for widget code
		static void          ItemSize(const KGVec2& size, float text_baseline_y = -1.0f);
		static inline void             ItemSize(const KGRect& bb, float text_baseline_y = -1.0f) { ItemSize(bb.GetSize(), text_baseline_y); } // FIXME: This is a misleading API since we expect CursorPos to be bb.Min.
		static bool          ItemAdd(const KGRect& bb, KGGuiID id, const KGRect* nav_bb = NULL, KGGuiItemFlags extra_flags = 0);
		static bool          ItemHoverable(const KGRect& bb, KGGuiID id);
		static bool          IsClippedEx(const KGRect& bb, KGGuiID id);
		static void          SetLastItemData(KGGuiID item_id, KGGuiItemFlags in_flags, KGGuiItemStatusFlags status_flags, const KGRect& item_rect);
		static KGVec2        CalcItemSize(KGVec2 size, float default_w, float default_h);
		static float         CalcWrapWidthForPos(const KGVec2& pos, float wrap_pos_x);
		static void          PushMultiItemsWidths(int components, float width_full);
		static bool          IsItemToggledSelection();                                   // Was the last item selection toggled? (after Selectable(), TreeNode() etc. We only returns toggle _event_ in order to handle clipping correctly)
		static KGVec2        GetContentRegionMaxAbs();
		static void          ShrinkWidths(KGGuiShrinkWidthItem* items, int count, float width_excess);

		// Parameter stacks (shared)
		static void          PushItemFlag(KGGuiItemFlags option, bool enabled);
		static void          PopItemFlag();

		// Logging/Capture
		static void          LogBegin(KGGuiLogType type, int auto_open_depth);           // -> BeginCapture() when we design v2 api, for now stay under the radar by using the old name.
		static void          LogToBuffer(int auto_open_depth = -1);                      // Start logging/capturing to internal buffer
		static void          LogRenderedText(const KGVec2* ref_pos, const char* text, const char* text_end = NULL);
		static void          LogSetNextTextDecoration(const char* prefix, const char* suffix);

		// Popups, Modals, Tooltips
		static bool          BeginChildEx(const char* name, KGGuiID id, const KGVec2& size_arg, bool border, KarmaGuiWindowFlags flags);
		static void          OpenPopupEx(KGGuiID id, KarmaGuiPopupFlags popup_flags = KGGuiPopupFlags_None);
		static void          ClosePopupToLevel(int remaining, bool restore_focus_to_window_under_popup);
		static void          ClosePopupsOverWindow(KGGuiWindow* ref_window, bool restore_focus_to_window_under_popup);
		static void          ClosePopupsExceptModals();
		static bool          IsPopupOpen(KGGuiID id, KarmaGuiPopupFlags popup_flags);
		static bool          BeginPopupEx(KGGuiID id, KarmaGuiWindowFlags extra_flags);
		static void          BeginTooltipEx(KGGuiTooltipFlags tooltip_flags, KarmaGuiWindowFlags extra_window_flags);
		static KGRect        GetPopupAllowedExtentRect(KGGuiWindow* window);
		static KGGuiWindow*  GetTopMostPopupModal();
		static KGGuiWindow*  GetTopMostAndVisiblePopupModal();
		static KGVec2        FindBestWindowPosForPopup(KGGuiWindow* window);
		static KGVec2        FindBestWindowPosForPopupEx(const KGVec2& ref_pos, const KGVec2& size, KarmaGuiDir* last_dir, const KGRect& r_outer, const KGRect& r_avoid, KGGuiPopupPositionPolicy policy);

		// Menus
		static bool          BeginViewportSideBar(const char* name, KarmaGuiViewport* viewport, KarmaGuiDir dir, float size, KarmaGuiWindowFlags window_flags);
		static bool          BeginMenuEx(const char* label, const char* icon, bool enabled = true);
		static bool          MenuItemEx(const char* label, const char* icon, const char* shortcut = NULL, bool selected = false, bool enabled = true);

		// Combos
		static bool          BeginComboPopup(KGGuiID popup_id, const KGRect& bb, KarmaGuiComboFlags flags);
		static bool          BeginComboPreview();
		static void          EndComboPreview();

		// Gamepad/Keyboard Navigation
		static void          NavInitWindow(KGGuiWindow* window, bool force_reinit);
		static void          NavInitRequestApplyResult();
		static bool          NavMoveRequestButNoResultYet();
		static void          NavMoveRequestSubmit(KarmaGuiDir move_dir, KarmaGuiDir clip_dir, KGGuiNavMoveFlags move_flags, KGGuiScrollFlags scroll_flags);
		static void          NavMoveRequestForward(KarmaGuiDir move_dir, KarmaGuiDir clip_dir, KGGuiNavMoveFlags move_flags, KGGuiScrollFlags scroll_flags);
		static void          NavMoveRequestResolveWithLastItem(KGGuiNavItemData* result);
		static void          NavMoveRequestCancel();
		static void          NavMoveRequestApplyResult();
		static void          NavMoveRequestTryWrapping(KGGuiWindow* window, KGGuiNavMoveFlags move_flags);
		static void          ActivateItem(KGGuiID id);   // Remotely activate a button, checkbox, tree node etc. given its unique ID. activation is queued and processed on the next frame when the item is encountered again.
		static void          SetNavWindow(KGGuiWindow* window);
		static void          SetNavID(KGGuiID id, KGGuiNavLayer nav_layer, KGGuiID focus_scope_id, const KGRect& rect_rel);

		// Inputs
		// FIXME: Eventually we should aim to move e.g. IsActiveIdUsingKey() into IsKeyXXX functions.
		static inline bool             IsNamedKey(KarmaGuiKey key)                                    { return key >= KGGuiKey_NamedKey_BEGIN && key < KGGuiKey_NamedKey_END; }
		static inline bool             IsNamedKeyOrModKey(KarmaGuiKey key)                            { return (key >= KGGuiKey_NamedKey_BEGIN && key < KGGuiKey_NamedKey_END) || key == KGGuiMod_Ctrl || key == KGGuiMod_Shift || key == KGGuiMod_Alt || key == KGGuiMod_Super || key == KGGuiMod_Shortcut; }
		static inline bool             IsLegacyKey(KarmaGuiKey key)                                   { return key >= KGGuiKey_LegacyNativeKey_BEGIN && key < KGGuiKey_LegacyNativeKey_END; }
		static inline bool             IsKeyboardKey(KarmaGuiKey key)                                 { return key >= KGGuiKey_Keyboard_BEGIN && key < KGGuiKey_Keyboard_END; }
		static inline bool             IsGamepadKey(KarmaGuiKey key)                                  { return key >= KGGuiKey_Gamepad_BEGIN && key < KGGuiKey_Gamepad_END; }
		static inline bool             IsMouseKey(KarmaGuiKey key)                                    { return key >= KGGuiKey_Mouse_BEGIN && key < KGGuiKey_Mouse_END; }
		static inline bool             IsAliasKey(KarmaGuiKey key)                                    { return key >= KGGuiKey_Aliases_BEGIN && key < KGGuiKey_Aliases_END; }
		static inline KarmaGuiKeyChord    ConvertShortcutMod(KarmaGuiKeyChord key_chord)                 { KarmaGuiContext& g = *GKarmaGui; KR_CORE_ASSERT(key_chord & KGGuiMod_Shortcut, "");return (key_chord & ~KGGuiMod_Shortcut) | (g.IO.ConfigMacOSXBehaviors ? KGGuiMod_Super : KGGuiMod_Ctrl); }
		static inline KarmaGuiKey         ConvertSingleModFlagToKey(KarmaGuiKey key)
		{
			KarmaGuiContext& g = *GKarmaGui;
			if (key == KGGuiMod_Ctrl) return KGGuiKey_ReservedForModCtrl;
			if (key == KGGuiMod_Shift) return KGGuiKey_ReservedForModShift;
			if (key == KGGuiMod_Alt) return KGGuiKey_ReservedForModAlt;
			if (key == KGGuiMod_Super) return KGGuiKey_ReservedForModSuper;
			if (key == KGGuiMod_Shortcut) return (g.IO.ConfigMacOSXBehaviors ? KGGuiKey_ReservedForModSuper : KGGuiKey_ReservedForModCtrl);
			return key;
		}

		static KarmaGuiKeyData* GetKeyData(KarmaGuiKey key);
		static void          GetKeyChordName(KarmaGuiKeyChord key_chord, char* out_buf, int out_buf_size);
		static inline KarmaGuiKey         MouseButtonToKey(KarmaGuiMouseButton button)                   { KR_CORE_ASSERT(button >= 0 && button < KGGuiMouseButton_COUNT, ""); return (KarmaGuiKey)(KGGuiKey_MouseLeft + button); }
		static bool          IsMouseDragPastThreshold(KarmaGuiMouseButton button, float lock_threshold = -1.0f);
		static KGVec2        GetKeyMagnitude2d(KarmaGuiKey key_left, KarmaGuiKey key_right, KarmaGuiKey key_up, KarmaGuiKey key_down);
		static float         GetNavTweakPressedAmount(KGGuiAxis axis);
		static int           CalcTypematicRepeatAmount(float t0, float t1, float repeat_delay, float repeat_rate);
		static void          GetTypematicRepeatRate(KarmaGuiInputFlags flags, float* repeat_delay, float* repeat_rate);
		static void          SetActiveIdUsingAllKeyboardKeys();
		static inline bool             IsActiveIdUsingNavDir(KarmaGuiDir dir)                         { KarmaGuiContext& g = *GKarmaGui; return (g.ActiveIdUsingNavDirMask & (1 << dir)) != 0; }

		// [EXPERIMENTAL] Low-Level: Key/Input Ownership
		// - The idea is that instead of "eating" a given input, we can link to an owner id.
		// - Ownership is most often claimed as a result of reacting to a press/down event (but occasionally may be claimed ahead).
		// - Input queries can then read input by specifying KGGuiKeyOwner_Any (== 0), KGGuiKeyOwner_None (== -1) or a custom ID.
		// - Legacy input queries (without specifying an owner or _Any or _None) are equivalent to using KGGuiKeyOwner_Any (== 0).
		// - Input ownership is automatically released on the frame after a key is released. Therefore:
		//   - for ownership registration happening as a result of a down/press event, the SetKeyOwner() call may be done once (common case).
		//   - for ownership registration happening ahead of a down/press event, the SetKeyOwner() call needs to be made every frame (happens if e.g. claiming ownership on hover).
		// - SetItemKeyOwner() is a shortcut for common simple case. A custom widget will probably want to call SetKeyOwner() multiple times directly based on its interaction state.
		// - This is marked experimental because not all widgets are fully honoring the Set/Test idioms. We will need to move forward step by step.
		//   Please open a GitHub Issue to submit your usage scenario or if there's a use case you need solved.
		static KGGuiID           GetKeyOwner(KarmaGuiKey key);
		static void              SetKeyOwner(KarmaGuiKey key, KGGuiID owner_id, KarmaGuiInputFlags flags = 0);
		static void              SetItemKeyOwner(KarmaGuiKey key, KarmaGuiInputFlags flags = 0);           // Set key owner to last item if it is hovered or active. Equivalent to 'if (IsItemHovered() || IsItemActive()) { SetKeyOwner(key, GetItemID());'.
		static bool              TestKeyOwner(KarmaGuiKey key, KGGuiID owner_id);                       // Test that key is either not owned, either owned by 'owner_id'
		static inline KGGuiKeyOwnerData*   GetKeyOwnerData(KarmaGuiKey key)     { if (key & KGGuiMod_Mask_) key = ConvertSingleModFlagToKey(key); KR_CORE_ASSERT(IsNamedKey(key), ""); return &GKarmaGui->KeysOwnerData[key - KGGuiKey_NamedKey_BEGIN]; }

		// [EXPERIMENTAL] High-Level: Input Access functions w/ support for Key/Input Ownership
		// - Important: legacy IsKeyPressed(KarmaGuiKey, bool repeat=true) _DEFAULTS_ to repeat, new IsKeyPressed() requires _EXPLICIT_ KGGuiInputFlags_Repeat flag.
		// - Expected to be later promoted to public API, the prototypes are designed to replace existing ones (since owner_id can default to Any == 0)
		// - Specifying a value for 'KGGuiID owner' will test that EITHER the key is NOT owned (UNLESS locked), EITHER the key is owned by 'owner'.
		//   Legacy functions use KGGuiKeyOwner_Any meaning that they typically ignore ownership, unless a call to SetKeyOwner() explicitly used KGGuiInputFlags_LockThisFrame or KGGuiInputFlags_LockUntilRelease.
		// - Binding generators may want to ignore those for now, or suffix them with Ex() until we decide if this gets moved into public API.
		static bool              IsKeyDown(KarmaGuiKey key, KGGuiID owner_id);
		static bool              IsKeyPressed(KarmaGuiKey key, KGGuiID owner_id, KarmaGuiInputFlags flags = 0);    // Important: when transitioning from old to new IsKeyPressed(): old API has "bool repeat = true", so would default to repeat. New API requiress explicit KGGuiInputFlags_Repeat.
		static bool              IsKeyReleased(KarmaGuiKey key, KGGuiID owner_id);
		static bool              IsMouseDown(KarmaGuiMouseButton button, KGGuiID owner_id);
		static bool              IsMouseClicked(KarmaGuiMouseButton button, KGGuiID owner_id, KarmaGuiInputFlags flags = 0);
		static bool              IsMouseReleased(KarmaGuiMouseButton button, KGGuiID owner_id);

		// [EXPERIMENTAL] Low-Level: Shortcut Routing
		// - Routes are resolved during NewFrame(): if keyboard modifiers are matching current ones: SetKeyOwner() is called + route is granted for the frame.
		// - Route is granted to a single owner. When multiple requests are made we have policies to select the winning route.
		// - Multiple read sites may use the same owner id and will all get the granted route.
		// - For routing: when owner_id is 0 we use the current Focus Scope ID as a default owner in order to identify our location.
		static bool              SetShortcutRouting(KarmaGuiKeyChord key_chord, KGGuiID owner_id = 0, KarmaGuiInputFlags flags = 0);
		static bool              TestShortcutRouting(KarmaGuiKeyChord key_chord, KGGuiID owner_id);
		static KGGuiKeyRoutingData* GetShortcutRoutingData(KarmaGuiKeyChord key_chord);

		// Docking
		// (some functions are only declared in imgui.cpp, see Docking section)
		static void          DockContextInitialize(KarmaGuiContext* ctx);
		static void          DockContextShutdown(KarmaGuiContext* ctx);
		static void          DockContextClearNodes(KarmaGuiContext* ctx, KGGuiID root_id, bool clear_settings_refs); // Use root_id==0 to clear all
		static void          DockContextRebuildNodes(KarmaGuiContext* ctx);
		static void          DockContextNewFrameUpdateUndocking(KarmaGuiContext* ctx);
		static void          DockContextNewFrameUpdateDocking(KarmaGuiContext* ctx);
		static void          DockContextEndFrame(KarmaGuiContext* ctx);
		static KGGuiID       DockContextGenNodeID(KarmaGuiContext* ctx);
		static void          DockContextQueueDock(KarmaGuiContext* ctx, KGGuiWindow* target, KGGuiDockNode* target_node, KGGuiWindow* payload, KarmaGuiDir split_dir, float split_ratio, bool split_outer);
		static void          DockContextQueueUndockWindow(KarmaGuiContext* ctx, KGGuiWindow* window);
		static void          DockContextQueueUndockNode(KarmaGuiContext* ctx, KGGuiDockNode* node);
		static bool          DockContextCalcDropPosForDocking(KGGuiWindow* target, KGGuiDockNode* target_node, KGGuiWindow* payload_window, KGGuiDockNode* payload_node, KarmaGuiDir split_dir, bool split_outer, KGVec2* out_pos);
		static KGGuiDockNode*DockContextFindNodeByID(KarmaGuiContext* ctx, KGGuiID id);
		static bool          DockNodeBeginAmendTabBar(KGGuiDockNode* node);
		static void          DockNodeEndAmendTabBar();
		static inline KGGuiDockNode*   DockNodeGetRootNode(KGGuiDockNode* node)                 { while (node->ParentNode) node = node->ParentNode; return node; }
		static inline bool             DockNodeIsInHierarchyOf(KGGuiDockNode* node, KGGuiDockNode* parent) { while (node) { if (node == parent) return true; node = node->ParentNode; } return false; }
		static inline int              DockNodeGetDepth(const KGGuiDockNode* node)              { int depth = 0; while (node->ParentNode) { node = node->ParentNode; depth++; } return depth; }
		static inline KGGuiID          DockNodeGetWindowMenuButtonId(const KGGuiDockNode* node) { return KGHashStr("#COLLAPSE", 0, node->ID); }
		static inline KGGuiDockNode*   GetWindowDockNode()                                      { KarmaGuiContext& g = *GKarmaGui; return g.CurrentWindow->DockNode; }
		static bool          GetWindowAlwaysWantOwnTabBar(KGGuiWindow* window);
		static void          BeginDocked(KGGuiWindow* window, bool* p_open);
		static void          BeginDockableDragDropSource(KGGuiWindow* window);
		static void          BeginDockableDragDropTarget(KGGuiWindow* window);
		static void          SetWindowDock(KGGuiWindow* window, KGGuiID dock_id, KarmaGuiCond cond);

		// Docking - Builder function needs to be generally called before the node is used/submitted.
		// - The DockBuilderXXX functions are designed to _eventually_ become a public API, but it is too early to expose it and guarantee stability.
		// - Do not hold on KGGuiDockNode* pointers! They may be invalidated by any split/merge/remove operation and every frame.
		// - To create a DockSpace() node, make sure to set the KGGuiDockNodeFlags_DockSpace flag when calling DockBuilderAddNode().
		//   You can create dockspace nodes (attached to a window) _or_ floating nodes (carry its own window) with this API.
		// - DockBuilderSplitNode() create 2 child nodes within 1 node. The initial node becomes a parent node.
		// - If you intend to split the node immediately after creation using DockBuilderSplitNode(), make sure
		//   to call DockBuilderSetNodeSize() beforehand. If you don't, the resulting split sizes may not be reliable.
		// - Call DockBuilderFinish() after you are done.
		static void          DockBuilderDockWindow(const char* window_name, KGGuiID node_id);
		static KGGuiDockNode*DockBuilderGetNode(KGGuiID node_id);
		static inline KGGuiDockNode*   DockBuilderGetCentralNode(KGGuiID node_id)              { KGGuiDockNode* node = DockBuilderGetNode(node_id); if (!node) return NULL; return DockNodeGetRootNode(node)->CentralNode; }
		static KGGuiID       DockBuilderAddNode(KGGuiID node_id = 0, KarmaGuiDockNodeFlags flags = 0);
		static void          DockBuilderRemoveNode(KGGuiID node_id);                 // Remove node and all its child, undock all windows
		static void          DockBuilderRemoveNodeDockedWindows(KGGuiID node_id, bool clear_settings_refs = true);
		static void          DockBuilderRemoveNodeChildNodes(KGGuiID node_id);       // Remove all split/hierarchy. All remaining docked windows will be re-docked to the remaining root node (node_id).
		static void          DockBuilderSetNodePos(KGGuiID node_id, KGVec2 pos);
		static void          DockBuilderSetNodeSize(KGGuiID node_id, KGVec2 size);
		static KGGuiID       DockBuilderSplitNode(KGGuiID node_id, KarmaGuiDir split_dir, float size_ratio_for_node_at_dir, KGGuiID* out_id_at_dir, KGGuiID* out_id_at_opposite_dir); // Create 2 child nodes in this parent node.
		static void          DockBuilderCopyDockSpace(KGGuiID src_dockspace_id, KGGuiID dst_dockspace_id, KGVector<const char*>* in_window_remap_pairs);
		static void          DockBuilderCopyNode(KGGuiID src_node_id, KGGuiID dst_node_id, KGVector<KGGuiID>* out_node_remap_pairs);
		static void          DockBuilderCopyWindowSettings(const char* src_name, const char* dst_name);
		static void          DockBuilderFinish(KGGuiID node_id);

		// [EXPERIMENTAL] Focus Scope
		// This is generally used to identify a unique input location (for e.g. a selection set)
		// There is one per window (automatically set in Begin), but:
		// - Selection patterns generally need to react (e.g. clear a selection) when landing on one item of the set.
		//   So in order to identify a set multiple lists in same window may each need a focus scope.
		//   If you imagine an hypothetical BeginSelectionGroup()/EndSelectionGroup() api, it would likely call PushFocusScope()/EndFocusScope()
		// - Shortcut routing also use focus scope as a default location identifier if an owner is not provided.
		// We don't use the ID Stack for this as it is common to want them separate.
		static void          PushFocusScope(KGGuiID id);
		static void          PopFocusScope();
		static inline KGGuiID          GetCurrentFocusScope() { KarmaGuiContext& g = *GKarmaGui; return g.CurrentFocusScopeId; }   // Focus scope we are outputting into, set by PushFocusScope()

		// Drag and Drop
		static bool          IsDragDropActive();
		static bool          BeginDragDropTargetCustom(const KGRect& bb, KGGuiID id);
		static void          ClearDragDrop();
		static bool          IsDragDropPayloadBeingAccepted();
		static void          RenderDragDropTargetRect(const KGRect& bb);

		// Internal Columns API (this is not exposed because we will encourage transitioning to the Tables API)
		static void          SetWindowClipRectBeforeSetChannel(KGGuiWindow* window, const KGRect& clip_rect);
		static void          BeginColumns(const char* str_id, int count, KGGuiOldColumnFlags flags = 0); // setup number of columns. use an identifier to distinguish multiple column sets. close with EndColumns().
		static void          EndColumns();                                                               // close columns
		static void          PushColumnClipRect(int column_index);
		static void          PushColumnsBackground();
		static void          PopColumnsBackground();
		static KGGuiID       GetColumnsID(const char* str_id, int count);
		static KGGuiOldColumns* FindOrCreateColumns(KGGuiWindow* window, KGGuiID id);
		static float         GetColumnOffsetFromNorm(const KGGuiOldColumns* columns, float offset_norm);
		static float         GetColumnNormFromOffset(const KGGuiOldColumns* columns, float offset);

		// Tables: Candidates for public API
		static void          TableOpenContextMenu(int column_n = -1);
		static void          TableSetColumnWidth(int column_n, float width);
		static void          TableSetColumnSortDirection(int column_n, KarmaGuiSortDirection sort_direction, bool append_to_sort_specs);
		static int           TableGetHoveredColumn(); // May use (TableGetColumnFlags() & KGGuiTableColumnFlags_IsHovered) instead. Return hovered column. return -1 when table is not hovered. return columns_count if the unused space at the right of visible columns is hovered.
		static float         TableGetHeaderRowHeight();
		static void          TablePushBackgroundChannel();
		static void          TablePopBackgroundChannel();

		// Tables: Internals
		static inline    KGGuiTable*   GetCurrentTable() { KarmaGuiContext& g = *GKarmaGui; return g.CurrentTable; }
		static KGGuiTable*   TableFindByID(KGGuiID id);
		static bool          BeginTableEx(const char* name, KGGuiID id, int columns_count, KarmaGuiTableFlags flags = 0, const KGVec2& outer_size = KGVec2(0, 0), float inner_width = 0.0f);
		static void          TableBeginInitMemory(KGGuiTable* table, int columns_count);
		static void          TableBeginApplyRequests(KGGuiTable* table);
		static void          TableSetupDrawChannels(KGGuiTable* table);
		static void          TableUpdateLayout(KGGuiTable* table);
		static void          TableUpdateBorders(KGGuiTable* table);
		static void          TableUpdateColumnsWeightFromWidth(KGGuiTable* table);
		static void          TableDrawBorders(KGGuiTable* table);
		static void          TableDrawContextMenu(KGGuiTable* table);
		static bool          TableBeginContextMenuPopup(KGGuiTable* table);
		static void          TableMergeDrawChannels(KGGuiTable* table);
		static inline KGGuiTableInstanceData*   TableGetInstanceData(KGGuiTable* table, int instance_no) { if (instance_no == 0) return &table->InstanceDataFirst; return &table->InstanceDataExtra[instance_no - 1]; }
		static void          TableSortSpecsSanitize(KGGuiTable* table);
		static void          TableSortSpecsBuild(KGGuiTable* table);
		static KarmaGuiSortDirection TableGetColumnNextSortDirection(KGGuiTableColumn* column);
		static void          TableFixColumnSortDirection(KGGuiTable* table, KGGuiTableColumn* column);
		static float         TableGetColumnWidthAuto(KGGuiTable* table, KGGuiTableColumn* column);
		static void          TableBeginRow(KGGuiTable* table);
		static void          TableEndRow(KGGuiTable* table);
		static void          TableBeginCell(KGGuiTable* table, int column_n);
		static void          TableEndCell(KGGuiTable* table);
		static KGRect        TableGetCellBgRect(const KGGuiTable* table, int column_n);
		static const char*   TableGetColumnName(const KGGuiTable* table, int column_n);
		static KGGuiID       TableGetColumnResizeID(const KGGuiTable* table, int column_n, int instance_no = 0);
		static float         TableGetMaxColumnWidth(const KGGuiTable* table, int column_n);
		static void          TableSetColumnWidthAutoSingle(KGGuiTable* table, int column_n);
		static void          TableSetColumnWidthAutoAll(KGGuiTable* table);
		static void          TableRemove(KGGuiTable* table);
		static void          TableGcCompactTransientBuffers(KGGuiTable* table);
		static void          TableGcCompactTransientBuffers(KGGuiTableTempData* table);
		static void          TableGcCompactSettings();

		// Tables: Settings
		static void                  TableLoadSettings(KGGuiTable* table);
		static void                  TableSaveSettings(KGGuiTable* table);
		static void                  TableResetSettings(KGGuiTable* table);
		static KGGuiTableSettings*   TableGetBoundSettings(KGGuiTable* table);
		static void                  TableSettingsAddSettingsHandler();
		static KGGuiTableSettings*   TableSettingsCreate(KGGuiID id, int columns_count);
		static KGGuiTableSettings*   TableSettingsFindByID(KGGuiID id);

		// Tab Bars
		static bool          BeginTabBarEx(KGGuiTabBar* tab_bar, const KGRect& bb, KarmaGuiTabBarFlags flags, KGGuiDockNode* dock_node);
		static KGGuiTabItem* TabBarFindTabByID(KGGuiTabBar* tab_bar, KGGuiID tab_id);
		static KGGuiTabItem* TabBarFindMostRecentlySelectedTabForActiveWindow(KGGuiTabBar* tab_bar);
		static void          TabBarAddTab(KGGuiTabBar* tab_bar, KarmaGuiTabItemFlags tab_flags, KGGuiWindow* window);
		static void          TabBarRemoveTab(KGGuiTabBar* tab_bar, KGGuiID tab_id);
		static void          TabBarCloseTab(KGGuiTabBar* tab_bar, KGGuiTabItem* tab);
		static void          TabBarQueueReorder(KGGuiTabBar* tab_bar, const KGGuiTabItem* tab, int offset);
		static void          TabBarQueueReorderFromMousePos(KGGuiTabBar* tab_bar, const KGGuiTabItem* tab, KGVec2 mouse_pos);
		static bool          TabBarProcessReorder(KGGuiTabBar* tab_bar);
		static bool          TabItemEx(KGGuiTabBar* tab_bar, const char* label, bool* p_open, KarmaGuiTabItemFlags flags, KGGuiWindow* docked_window);
		static KGVec2        TabItemCalcSize(const char* label, bool has_close_button_or_unsaved_marker);
		static KGVec2        TabItemCalcSize(KGGuiWindow* window);
		static void          TabItemBackground(KGDrawList* draw_list, const KGRect& bb, KarmaGuiTabItemFlags flags, KGU32 col);
		static void          TabItemLabelAndCloseButton(KGDrawList* draw_list, const KGRect& bb, KarmaGuiTabItemFlags flags, KGVec2 frame_padding, const char* label, KGGuiID tab_id, KGGuiID close_button_id, bool is_contents_visible, bool* out_just_closed, bool* out_text_clipped);

		// Render helpers
		// AVOID USING OUTSIDE OF IMGUI.CPP! NOT FOR PUBLIC CONSUMPTION. THOSE FUNCTIONS ARE A MESS. THEIR SIGNATURE AND BEHAVIOR WILL CHANGE, THEY NEED TO BE REFACTORED INTO SOMETHING DECENT.
		// NB: All position are in absolute pixels coordinates (we are never using window coordinates internally)
		static void          RenderText(KGVec2 pos, const char* text, const char* text_end = NULL, bool hide_text_after_hash = true);
		static void          RenderTextWrapped(KGVec2 pos, const char* text, const char* text_end, float wrap_width);
		static void          RenderTextClipped(const KGVec2& pos_min, const KGVec2& pos_max, const char* text, const char* text_end, const KGVec2* text_size_if_known, const KGVec2& align = KGVec2(0, 0), const KGRect* clip_rect = NULL);
		static void          RenderTextClippedEx(KGDrawList* draw_list, const KGVec2& pos_min, const KGVec2& pos_max, const char* text, const char* text_end, const KGVec2* text_size_if_known, const KGVec2& align = KGVec2(0, 0), const KGRect* clip_rect = NULL);
		static void          RenderTextEllipsis(KGDrawList* draw_list, const KGVec2& pos_min, const KGVec2& pos_max, float clip_max_x, float ellipsis_max_x, const char* text, const char* text_end, const KGVec2* text_size_if_known);
		static void          RenderFrame(KGVec2 p_min, KGVec2 p_max, KGU32 fill_col, bool border = true, float rounding = 0.0f);
		static void          RenderFrameBorder(KGVec2 p_min, KGVec2 p_max, float rounding = 0.0f);
		static void          RenderColorRectWithAlphaCheckerboard(KGDrawList* draw_list, KGVec2 p_min, KGVec2 p_max, KGU32 fill_col, float grid_step, KGVec2 grid_off, float rounding = 0.0f, KGDrawFlags flags = 0);
		static void          RenderNavHighlight(const KGRect& bb, KGGuiID id, KGGuiNavHighlightFlags flags = KGGuiNavHighlightFlags_TypeDefault); // Navigation highlight
		static const char*   FindRenderedTextEnd(const char* text, const char* text_end = NULL); // Find the optional ## from which we stop displaying text.
		static void          RenderMouseCursor(KGVec2 pos, float scale, KarmaGuiMouseCursor mouse_cursor, KGU32 col_fill, KGU32 col_border, KGU32 col_shadow);

		// Render helpers (those functions don't access any ImGui state!)
		static void          RenderArrow(KGDrawList* draw_list, KGVec2 pos, KGU32 col, KarmaGuiDir dir, float scale = 1.0f);
		static void          RenderBullet(KGDrawList* draw_list, KGVec2 pos, KGU32 col);
		static void          RenderCheckMark(KGDrawList* draw_list, KGVec2 pos, KGU32 col, float sz);
		static void          RenderArrowPointingAt(KGDrawList* draw_list, KGVec2 pos, KGVec2 half_sz, KarmaGuiDir direction, KGU32 col);
		static void          RenderArrowDockMenu(KGDrawList* draw_list, KGVec2 p_min, float sz, KGU32 col);
		static void          RenderRectFilledRangeH(KGDrawList* draw_list, const KGRect& rect, KGU32 col, float x_start_norm, float x_end_norm, float rounding);
		static void          RenderRectFilledWithHole(KGDrawList* draw_list, const KGRect& outer, const KGRect& inner, KGU32 col, float rounding);
		static KGDrawFlags   CalcRoundingFlagsForRectInRect(const KGRect& r_in, const KGRect& r_outer, float threshold);

		// Widgets
		static void          TextEx(const char* text, const char* text_end = NULL, KGGuiTextFlags flags = 0);
		static bool          ButtonEx(const char* label, const KGVec2& size_arg = KGVec2(0, 0), KarmaGuiButtonFlags flags = 0);
		static bool          ArrowButtonEx(const char* str_id, KarmaGuiDir dir, KGVec2 size_arg, KarmaGuiButtonFlags flags = 0);
		static bool          ImageButtonEx(KGGuiID id, KGTextureID texture_id, const KGVec2& size, const KGVec2& uv0, const KGVec2& uv1, const KGVec4& bg_col, const KGVec4& tint_col);
		static void          SeparatorEx(KGGuiSeparatorFlags flags);
		static bool          CheckboxFlags(const char* label, KGS64* flags, KGS64 flags_value);
		static bool          CheckboxFlags(const char* label, KGU64* flags, KGU64 flags_value);

		// Widgets: Window Decorations
		static bool          CloseButton(KGGuiID id, const KGVec2& pos);
		static bool          CollapseButton(KGGuiID id, const KGVec2& pos, KGGuiDockNode* dock_node);
		static void          Scrollbar(KGGuiAxis axis);
		static bool          ScrollbarEx(const KGRect& bb, KGGuiID id, KGGuiAxis axis, KGS64* p_scroll_v, KGS64 avail_v, KGS64 contents_v, KGDrawFlags flags);
		static KGRect        GetWindowScrollbarRect(KGGuiWindow* window, KGGuiAxis axis);
		static KGGuiID       GetWindowScrollbarID(KGGuiWindow* window, KGGuiAxis axis);
		static KGGuiID       GetWindowResizeCornerID(KGGuiWindow* window, int n); // 0..3: corners
		static KGGuiID       GetWindowResizeBorderID(KGGuiWindow* window, KarmaGuiDir dir);

		// Widgets low-level behaviors
		static bool          ButtonBehavior(const KGRect& bb, KGGuiID id, bool* out_hovered, bool* out_held, KarmaGuiButtonFlags flags = 0);
		static bool          DragBehavior(KGGuiID id, KarmaGuiDataType data_type, void* p_v, float v_speed, const void* p_min, const void* p_max, const char* format, KarmaGuiSliderFlags flags);
		static bool          SliderBehavior(const KGRect& bb, KGGuiID id, KarmaGuiDataType data_type, void* p_v, const void* p_min, const void* p_max, const char* format, KarmaGuiSliderFlags flags, KGRect* out_grab_bb);
		static bool          SplitterBehavior(const KGRect& bb, KGGuiID id, KGGuiAxis axis, float* size1, float* size2, float min_size1, float min_size2, float hover_extend = 0.0f, float hover_visibility_delay = 0.0f, KGU32 bg_col = 0);
		static bool          TreeNodeBehavior(KGGuiID id, KarmaGuiTreeNodeFlags flags, const char* label, const char* label_end = NULL);
		static void          TreePushOverrideID(KGGuiID id);
		static void          TreeNodeSetOpen(KGGuiID id, bool open);
		static bool          TreeNodeUpdateNextOpen(KGGuiID id, KarmaGuiTreeNodeFlags flags);   // Return open state. Consume previous SetNextItemOpen() data, if any. May return true when logging.

		// Template functions are instantiated in imgui_widgets.cpp for a finite number of types.
		// To use them externally (for custom widget) you may need an "extern template" statement in your code in order to link to existing instances and silence Clang warnings (see #2036).
		// e.g. " extern template  float RoundScalarWithFormatT<float, float>(const char* format, KarmaGuiDataType data_type, float v); "
		template<typename T, typename SIGNED_T, typename FLOAT_T>    float ScaleRatioFromValueT(KarmaGuiDataType data_type, T v, T v_min, T v_max, bool is_logarithmic, float logarithmic_zero_epsilon, float zero_deadzone_size);
		template<typename T, typename SIGNED_T, typename FLOAT_T>    T     ScaleValueFromRatioT(KarmaGuiDataType data_type, float t, T v_min, T v_max, bool is_logarithmic, float logarithmic_zero_epsilon, float zero_deadzone_size);
		template<typename T, typename SIGNED_T, typename FLOAT_T>    bool  DragBehaviorT(KarmaGuiDataType data_type, T* v, float v_speed, T v_min, T v_max, const char* format, KarmaGuiSliderFlags flags);
		template<typename T, typename SIGNED_T, typename FLOAT_T>    bool  SliderBehaviorT(const KGRect& bb, KGGuiID id, KarmaGuiDataType data_type, T* v, T v_min, T v_max, const char* format, KarmaGuiSliderFlags flags, KGRect* out_grab_bb);
		template<typename T>                                         T     RoundScalarWithFormatT(const char* format, KarmaGuiDataType data_type, T v);
		template<typename T>                                         bool  CheckboxFlagsT(const char* label, T* flags, T flags_value);

		// Data type helpers
		static const KGGuiDataTypeInfo*  DataTypeGetInfo(KarmaGuiDataType data_type);
		static int           DataTypeFormatString(char* buf, int buf_size, KarmaGuiDataType data_type, const void* p_data, const char* format);
		static void          DataTypeApplyOp(KarmaGuiDataType data_type, int op, void* output, const void* arg_1, const void* arg_2);
		static bool          DataTypeApplyFromText(const char* buf, KarmaGuiDataType data_type, void* p_data, const char* format);
		static int           DataTypeCompare(KarmaGuiDataType data_type, const void* arg_1, const void* arg_2);
		static bool          DataTypeClamp(KarmaGuiDataType data_type, void* p_data, const void* p_min, const void* p_max);

		// InputText
		static bool          InputTextEx(const char* label, const char* hint, char* buf, int buf_size, const KGVec2& size_arg, KarmaGuiInputTextFlags flags, KarmaGuiInputTextCallback callback = NULL, void* user_data = NULL);
		static bool          TempInputText(const KGRect& bb, KGGuiID id, const char* label, char* buf, int buf_size, KarmaGuiInputTextFlags flags);
		static bool          TempInputScalar(const KGRect& bb, KGGuiID id, const char* label, KarmaGuiDataType data_type, void* p_data, const char* format, const void* p_clamp_min = NULL, const void* p_clamp_max = NULL);
		static inline bool             TempInputIsActive(KGGuiID id)       { KarmaGuiContext& g = *GKarmaGui; return (g.ActiveId == id && g.TempInputId == id); }
		static inline KGGuiInputTextState* GetInputTextState(KGGuiID id)   { KarmaGuiContext& g = *GKarmaGui; return (id != 0 && g.InputTextState.ID == id) ? &g.InputTextState : NULL; } // Get input text state if active

		// Color
		static void          ColorTooltip(const char* text, const float* col, KarmaGuiColorEditFlags flags);
		static void          ColorEditOptionsPopup(const float* col, KarmaGuiColorEditFlags flags);
		static void          ColorPickerOptionsPopup(const float* ref_col, KarmaGuiColorEditFlags flags);

		// Plot
		static int           PlotEx(KGGuiPlotType plot_type, const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, KGVec2 frame_size);

		// Shade functions (write over already created vertices)
		static void          ShadeVertsLinearColorGradientKeepAlpha(KGDrawList* draw_list, int vert_start_idx, int vert_end_idx, KGVec2 gradient_p0, KGVec2 gradient_p1, KGU32 col0, KGU32 col1);
		static void          ShadeVertsLinearUV(KGDrawList* draw_list, int vert_start_idx, int vert_end_idx, const KGVec2& a, const KGVec2& b, const KGVec2& uv_a, const KGVec2& uv_b, bool clamp);

		// Garbage collection
		static void          GcCompactTransientMiscBuffers();
		static void          GcCompactTransientWindowBuffers(KGGuiWindow* window);
		static void          GcAwakeTransientWindowBuffers(KGGuiWindow* window);

		// Debug Tools
		static void          ErrorCheckEndFrameRecover(KGGuiErrorLogCallback log_callback, void* user_data = NULL);
		static void          ErrorCheckEndWindowRecover(KGGuiErrorLogCallback log_callback, void* user_data = NULL);
		static void          ErrorCheckUsingSetCursorPosToExtendParentBoundaries();
		static void          DebugLocateItem(KGGuiID target_id);                     // Call sparingly: only 1 at the same time!
		static void          DebugLocateItemOnHover(KGGuiID target_id);              // Only call on reaction to a mouse Hover: because only 1 at the same time!
		static void          DebugLocateItemResolveWithLastItem();
		static inline void             DebugDrawItemRect(KGU32 col = KG_COL32(255,0,0,255))    { KarmaGuiContext& g = *GKarmaGui; KGGuiWindow* window = g.CurrentWindow; GetForegroundDrawList(window)->AddRect(g.LastItemData.Rect.Min, g.LastItemData.Rect.Max, col); }
		static inline void             DebugStartItemPicker()                                  { KarmaGuiContext& g = *GKarmaGui; g.DebugItemPickerActive = true; }
		static void          ShowFontAtlas(KGFontAtlas* atlas);
		static void          DebugHookIdInfo(KGGuiID id, KarmaGuiDataType data_type, const void* data_id, const void* data_id_end);
		static void          DebugNodeColumns(KGGuiOldColumns* columns);
		static void          DebugNodeDockNode(KGGuiDockNode* node, const char* label);
		static void          DebugNodeDrawList(KGGuiWindow* window, KGGuiViewportP* viewport, const KGDrawList* draw_list, const char* label);
		static void          DebugNodeDrawCmdShowMeshAndBoundingBox(KGDrawList* out_draw_list, const KGDrawList* draw_list, const KGDrawCmd* draw_cmd, bool show_mesh, bool show_aabb);
		static void          DebugNodeFont(KGFont* font);
		static void          DebugNodeFontGlyph(KGFont* font, const KGFontGlyph* glyph);
		static void          DebugNodeStorage(KarmaGuiStorage* storage, const char* label);
		static void          DebugNodeTabBar(KGGuiTabBar* tab_bar, const char* label);
		static void          DebugNodeTable(KGGuiTable* table);
		static void          DebugNodeTableSettings(KGGuiTableSettings* settings);
		static void          DebugNodeInputTextState(KGGuiInputTextState* state);
		static void          DebugNodeWindow(KGGuiWindow* window, const char* label);
		static void          DebugNodeWindowSettings(KGGuiWindowSettings* settings);
		static void          DebugNodeWindowsList(KGVector<KGGuiWindow*>* windows, const char* label);
		static void          DebugNodeWindowsListByBeginStackParent(KGGuiWindow** windows, int windows_size, KGGuiWindow* parent_in_begin_stack);
		static void          DebugNodeViewport(KGGuiViewportP* viewport);
		static void          DebugRenderKeyboardPreview(KGDrawList* draw_list);
		static void          DebugRenderViewportThumbnail(KGDrawList* draw_list, KGGuiViewportP* viewport, const KGRect& bb);
	};
} // namespace Karma


//-----------------------------------------------------------------------------
// [SECTION] KGFontAtlas internal API
//-----------------------------------------------------------------------------

// This structure is likely to evolve as we add support for incremental atlas updates
struct KGFontBuilderIO
{
    bool    (*FontBuilder_Build)(KGFontAtlas* atlas);
};

// Helper for font builder
#ifdef KGGUI_ENABLE_STB_TRUETYPE
 const KGFontBuilderIO* ImFontAtlasGetBuilderForStbTruetype();
#endif
 void      ImFontAtlasBuildInit(KGFontAtlas* atlas);
 void      ImFontAtlasBuildSetupFont(KGFontAtlas* atlas, KGFont* font, KGFontConfig* font_config, float ascent, float descent);
 void      ImFontAtlasBuildPackCustomRects(KGFontAtlas* atlas, void* stbrp_context_opaque);
 void      ImFontAtlasBuildFinish(KGFontAtlas* atlas);
 void      ImFontAtlasBuildRender8bppRectFromString(KGFontAtlas* atlas, int x, int y, int w, int h, const char* in_str, char in_marker_char, unsigned char in_marker_pixel_value);
 void      ImFontAtlasBuildRender32bppRectFromString(KGFontAtlas* atlas, int x, int y, int w, int h, const char* in_str, char in_marker_char, unsigned int in_marker_pixel_value);
 void      ImFontAtlasBuildMultiplyCalcLookupTable(unsigned char out_table[256], float in_multiply_factor);
 void      ImFontAtlasBuildMultiplyRectAlpha8(const unsigned char table[256], unsigned char* pixels, int x, int y, int w, int h, int stride);

//-----------------------------------------------------------------------------
// [SECTION] Test Engine specific hooks (imgui_test_engine)
//-----------------------------------------------------------------------------

#define KARMAGUI_TEST_ENGINE_ITEM_ADD(_BB,_ID)                 ((void)0)
#define KARMAGUI_TEST_ENGINE_ITEM_INFO(_ID,_LABEL,_FLAGS)      ((void)g)





