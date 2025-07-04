// Dear KarmaGui is Copyright (c) 2014-2023 Omar Cornut. This code is practically KarmaGui in Karma context!!
// (widgets code)
/*
Index of this file:

// [SECTION] Forward Declarations
// [SECTION] Widgets: Text, etc.
// [SECTION] Widgets: Main (Button, Image, Checkbox, RadioButton, ProgressBar, Bullet, etc.)
// [SECTION] Widgets: Low-level Layout helpers (Spacing, Dummy, NewLine, Separator, etc.)
// [SECTION] Widgets: ComboBox
// [SECTION] Data Type and Data Formatting Helpers
// [SECTION] Widgets: DragScalar, DragFloat, DragInt, etc.
// [SECTION] Widgets: SliderScalar, SliderFloat, SliderInt, etc.
// [SECTION] Widgets: InputScalar, InputFloat, InputInt, etc.
// [SECTION] Widgets: InputText, InputTextMultiline
// [SECTION] Widgets: ColorEdit, ColorPicker, ColorButton, etc.
// [SECTION] Widgets: TreeNode, CollapsingHeader, etc.
// [SECTION] Widgets: Selectable
// [SECTION] Widgets: ListBox
// [SECTION] Widgets: PlotLines, PlotHistogram
// [SECTION] Widgets: Value helpers
// [SECTION] Widgets: MenuItem, BeginMenu, EndMenu, etc.
// [SECTION] Widgets: BeginTabBar, EndTabBar, etc.
// [SECTION] Widgets: BeginTabItem, EndTabItem, etc.
// [SECTION] Widgets: Columns, BeginColumns, EndColumns, etc.
*/

#include "KarmaGui.h"
#include "KarmaGuiInternal.h"

#ifndef KARMAGUI_DEFINE_MATH_OPERATORS
#define KARMAGUI_DEFINE_MATH_OPERATORS
#endif

// System includes
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>     // intptr_t
#else
#include <stdint.h>     // intptr_t
#endif

//-------------------------------------------------------------------------
// Warnings
//-------------------------------------------------------------------------

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4127)     // condition expression is constant
#pragma warning (disable: 4996)     // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#if defined(_MSC_VER) && _MSC_VER >= 1922 // MSVC 2019 16.2 or later
#pragma warning (disable: 5054)     // operator '|': deprecated between enumerations of different types
#endif
#pragma warning (disable: 26451)    // [Static Analyzer] Arithmetic overflow : Using operator 'xxx' on a 4 byte value and then casting the result to a 8 byte value. Cast the value to the wider type before calling operator 'xxx' to avoid overflow(io.2).
#pragma warning (disable: 26812)    // [Static Analyzer] The enum type 'xxx' is unscoped. Prefer 'enum class' over 'enum' (Enum.3).
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored "-Wunknown-warning-option"         // warning: unknown warning group 'xxx'                      // not all warnings are known by all Clang versions and they tend to be rename-happy.. so ignoring warnings triggers new warnings on some configuration. Great!
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"                // warning: unknown warning group 'xxx'
#pragma clang diagnostic ignored "-Wold-style-cast"                 // warning: use of old-style cast                            // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"                    // warning: comparing floating point with == or != is unsafe // storing and comparing against same constants (typically 0.0f) is ok.
#pragma clang diagnostic ignored "-Wformat-nonliteral"              // warning: format string is not a string literal            // passing non-literal to vsnformat(). yes, user passing incorrect format strings can crash the code.
#pragma clang diagnostic ignored "-Wsign-conversion"                // warning: implicit conversion changes signedness
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning: zero as null pointer constant                    // some standard header variations use #define NULL 0
#pragma clang diagnostic ignored "-Wdouble-promotion"               // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#pragma clang diagnostic ignored "-Wenum-enum-conversion"           // warning: bitwise operation between different enumeration types ('XXXFlags_' and 'XXXFlagsPrivate_')
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"// warning: bitwise operation between different enumeration types ('XXXFlags_' and 'XXXFlagsPrivate_') is deprecated
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float' may lose precision
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wpragmas"                          // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wformat-nonliteral"                // warning: format not a string literal, format string not checked
#pragma GCC diagnostic ignored "-Wclass-memaccess"                  // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"  // warning: bitwise operation between different enumeration types ('XXXFlags_' and 'XXXFlagsPrivate_') is deprecated
#endif

//-------------------------------------------------------------------------
// Data
//-------------------------------------------------------------------------

// Widgets
static const float          DRAGDROP_HOLD_TO_OPEN_TIMER = 0.70f;    // Time for drag-hold to activate items accepting the KGGuiButtonFlags_PressedOnDragDropHold button behavior.
static const float          DRAG_MOUSE_THRESHOLD_FACTOR = 0.50f;    // Multiplier for the default value of io.MouseDragThreshold to make DragFloat/DragInt react faster to mouse drags.

// Those MIN/MAX values are not define because we need to point to them
static const signed char    IM_S8_MIN = -128;
static const signed char    IM_S8_MAX = 127;
static const unsigned char  IM_U8_MIN = 0;
static const unsigned char  IM_U8_MAX = 0xFF;
static const signed short   IM_S16_MIN = -32768;
static const signed short   IM_S16_MAX = 32767;
static const unsigned short IM_U16_MIN = 0;
static const unsigned short IM_U16_MAX = 0xFFFF;
static const KGS32          IM_S32_MIN = INT_MIN;    // (-2147483647 - 1), (0x80000000);
static const KGS32          IM_S32_MAX = INT_MAX;    // (2147483647), (0x7FFFFFFF)
static const KGU32          IM_U32_MIN = 0;
static const KGU32          IM_U32_MAX = UINT_MAX;   // (0xFFFFFFFF)
#ifdef LLONG_MIN
static const KGS64          IM_S64_MIN = LLONG_MIN;  // (-9223372036854775807ll - 1ll);
static const KGS64          IM_S64_MAX = LLONG_MAX;  // (9223372036854775807ll);
#else
static const KGS64          IM_S64_MIN = -9223372036854775807LL - 1;
static const KGS64          IM_S64_MAX = 9223372036854775807LL;
#endif
static const KGU64          IM_U64_MIN = 0;
#ifdef ULLONG_MAX
static const KGU64          IM_U64_MAX = ULLONG_MAX; // (0xFFFFFFFFFFFFFFFFull);
#else
static const KGU64          IM_U64_MAX = (2ULL * 9223372036854775807LL + 1);
#endif

//struct StbTexteditRow;

//-------------------------------------------------------------------------
// [SECTION] Forward Declarations
//-------------------------------------------------------------------------
namespace Karma
{
	// For InputTextEx()
	static bool             InputTextFilterCharacter(unsigned int* p_char, KarmaGuiInputTextFlags flags, KarmaGuiInputTextCallback callback, void* user_data, KGGuiInputSource input_source);
	static int              InputTextCalcTextLenAndLineCount(const char* text_begin, const char** out_text_end);
	static KGVec2           InputTextCalcTextSizeW(KarmaGuiContext* ctx, const KGWchar* text_begin, const KGWchar* text_end, const KGWchar** remaining = NULL, KGVec2* out_offset = NULL, bool stop_on_new_line = false);
	const char* KGParseFormatFindStart(const char* format);
	const char* KGParseFormatFindEnd(const char* format);
	int           KGParseFormatPrecision(const char* format, int default_value);
	void          KGParseFormatSanitizeForPrinting(const char* fmt_in, char* fmt_out, size_t fmt_out_size);
	const char* KGParseFormatSanitizeForScanning(const char* fmt_in, char* fmt_out, size_t fmt_out_size);
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: Text, etc.
//-------------------------------------------------------------------------
// - TextEx() [Internal]
// - TextUnformatted()
// - Text()
// - TextV()
// - TextColored()
// - TextColoredV()
// - TextDisabled()
// - TextDisabledV()
// - TextWrapped()
// - TextWrappedV()
// - LabelText()
// - LabelTextV()
// - BulletText()
// - BulletTextV()
//-------------------------------------------------------------------------

namespace  Karma
{
	extern KarmaGuiContext* GKarmaGui; // Current implicit context pointer
	void KarmaGuiInternal::TextEx(const char* text, const char* text_end, KGGuiTextFlags flags)
	{
		KGGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;
		KarmaGuiContext& g = *GKarmaGui;

		// Accept null ranges
		if (text == text_end)
			text = text_end = "";

		// Calculate length
		const char* text_begin = text;
		if (text_end == NULL)
			text_end = text + strlen(text); // FIXME-OPT

		const KGVec2 text_pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);
		const float wrap_pos_x = window->DC.TextWrapPos;
		const bool wrap_enabled = (wrap_pos_x >= 0.0f);
		if (text_end - text <= 2000 || wrap_enabled)
		{
			// Common case
			const float wrap_width = wrap_enabled ? CalcWrapWidthForPos(window->DC.CursorPos, wrap_pos_x) : 0.0f;
			const KGVec2 text_size = KarmaGui::CalcTextSize(text_begin, text_end, false, wrap_width);

			KGRect bb(text_pos, text_pos + text_size);
			ItemSize(text_size, 0.0f);
			if (!ItemAdd(bb, 0))
				return;

			// Render (we don't hide text after ## in this end-user function)
			RenderTextWrapped(bb.Min, text_begin, text_end, wrap_width);
		}
		else
		{
			// Long text!
			// Perform manual coarse clipping to optimize for long multi-line text
			// - From this point we will only compute the width of lines that are visible. Optimization only available when word-wrapping is disabled.
			// - We also don't vertically center the text within the line full height, which is unlikely to matter because we are likely the biggest and only item on the line.
			// - We use memchr(), pay attention that well optimized versions of those str/mem functions are much faster than a casually written loop.
			const char* line = text;
			const float line_height = KarmaGui::GetTextLineHeight();
			KGVec2 text_size(0, 0);

			// Lines to skip (can't skip when logging text)
			KGVec2 pos = text_pos;
			if (!g.LogEnabled)
			{
				int lines_skippable = (int)((window->ClipRect.Min.y - text_pos.y) / line_height);
				if (lines_skippable > 0)
				{
					int lines_skipped = 0;
					while (line < text_end && lines_skipped < lines_skippable)
					{
						const char* line_end = (const char*)memchr(line, '\n', text_end - line);
						if (!line_end)
							line_end = text_end;
						if ((flags & KGGuiTextFlags_NoWidthForLargeClippedText) == 0)
							text_size.x = KGMax(text_size.x, KarmaGui::CalcTextSize(line, line_end).x);
						line = line_end + 1;
						lines_skipped++;
					}
					pos.y += lines_skipped * line_height;
				}
			}

			// Lines to render
			if (line < text_end)
			{
				KGRect line_rect(pos, pos + KGVec2(FLT_MAX, line_height));
				while (line < text_end)
				{
					if (IsClippedEx(line_rect, 0))
						break;

					const char* line_end = (const char*)memchr(line, '\n', text_end - line);
					if (!line_end)
						line_end = text_end;
					text_size.x = KGMax(text_size.x, KarmaGui::CalcTextSize(line, line_end).x);
					RenderText(pos, line, line_end, false);
					line = line_end + 1;
					line_rect.Min.y += line_height;
					line_rect.Max.y += line_height;
					pos.y += line_height;
				}

				// Count remaining lines
				int lines_skipped = 0;
				while (line < text_end)
				{
					const char* line_end = (const char*)memchr(line, '\n', text_end - line);
					if (!line_end)
						line_end = text_end;
					if ((flags & KGGuiTextFlags_NoWidthForLargeClippedText) == 0)
						text_size.x = KGMax(text_size.x, KarmaGui::CalcTextSize(line, line_end).x);
					line = line_end + 1;
					lines_skipped++;
				}
				pos.y += lines_skipped * line_height;
			}
			text_size.y = (pos - text_pos).y;

			KGRect bb(text_pos, text_pos + text_size);
			ItemSize(text_size, 0.0f);
			ItemAdd(bb, 0);
		}
	}

	void KarmaGui::TextUnformatted(const char* text, const char* text_end)
	{
		KarmaGuiInternal::TextEx(text, text_end, KGGuiTextFlags_NoWidthForLargeClippedText);
	}

	void KarmaGui::Text(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		TextV(fmt, args);
		va_end(args);
	}

	void KarmaGui::TextV(const char* fmt, va_list args)
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return;

		// FIXME-OPT: Handle the %s shortcut?
		const char* text, * text_end;
		KGFormatStringToTempBufferV(&text, &text_end, fmt, args);
		KarmaGuiInternal::TextEx(text, text_end, KGGuiTextFlags_NoWidthForLargeClippedText);
	}

	void KarmaGui::TextColored(const KGVec4& col, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		TextColoredV(col, fmt, args);
		va_end(args);
	}

	/// Draws vertical text. The position is the bottom left of the text rect.
	/// Courtsey: https://github.com/ocornut/imgui/issues/705 with my own modular scaling modification
	void KarmaGui::AddTextVertical(KGDrawList* DrawList, const char *text, KGVec2 pos, KGU32 text_color)
	{
		pos.x = KG_ROUND(pos.x);
		pos.y = KG_ROUND(pos.y);
		KGFont *font = GKarmaGui->Font;
		const KGFontGlyph *glyph;
		char c;
		KarmaGuiContext& g = *GKarmaGui;
		//KGVec2 text_size = CalcTextSize(text);
		while ((c = *text++))
		{
			glyph = font->FindGlyph(c);
			if (!glyph) continue;

			DrawList->PrimReserve(6, 4);
			DrawList->PrimQuadUV(
								pos + KGVec2(glyph->Y0 * font->Scale, -glyph->X0 * font->Scale),
								pos + KGVec2(glyph->Y0 * font->Scale, -glyph->X1 * font->Scale),
								pos + KGVec2(glyph->Y1 * font->Scale, -glyph->X1 * font->Scale),
								pos + KGVec2(glyph->Y1 * font->Scale, -glyph->X0 * font->Scale),

								KGVec2(glyph->U0, glyph->V0),
								KGVec2(glyph->U1, glyph->V0),
								KGVec2(glyph->U1, glyph->V1),
								KGVec2(glyph->U0, glyph->V1),
								text_color);
			pos.y -= glyph->AdvanceX * font->Scale;
		}
	}

	void KarmaGui::TextColoredV(const KGVec4& col, const char* fmt, va_list args)
	{
		PushStyleColor(KGGuiCol_Text, col);
		if (fmt[0] == '%' && fmt[1] == 's' && fmt[2] == 0)
			KarmaGuiInternal::TextEx(va_arg(args, const char*), NULL, KGGuiTextFlags_NoWidthForLargeClippedText); // Skip formatting
		else
			TextV(fmt, args);
		PopStyleColor();
	}

	void KarmaGui::TextDisabled(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		TextDisabledV(fmt, args);
		va_end(args);
	}

	void KarmaGui::TextDisabledV(const char* fmt, va_list args)
	{
		KarmaGuiContext& g = *GKarmaGui;
		PushStyleColor(KGGuiCol_Text, g.Style.Colors[KGGuiCol_TextDisabled]);
		if (fmt[0] == '%' && fmt[1] == 's' && fmt[2] == 0)
			KarmaGuiInternal::TextEx(va_arg(args, const char*), NULL, KGGuiTextFlags_NoWidthForLargeClippedText); // Skip formatting
		else
			TextV(fmt, args);
		PopStyleColor();
	}

	void KarmaGui::TextWrapped(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		TextWrappedV(fmt, args);
		va_end(args);
	}

	void KarmaGui::TextWrappedV(const char* fmt, va_list args)
	{
		KarmaGuiContext& g = *GKarmaGui;
		bool need_backup = (g.CurrentWindow->DC.TextWrapPos < 0.0f);  // Keep existing wrap position if one is already set
		if (need_backup)
			PushTextWrapPos(0.0f);
		if (fmt[0] == '%' && fmt[1] == 's' && fmt[2] == 0)
			KarmaGuiInternal::TextEx(va_arg(args, const char*), NULL, KGGuiTextFlags_NoWidthForLargeClippedText); // Skip formatting
		else
			TextV(fmt, args);
		if (need_backup)
			PopTextWrapPos();
	}

	void KarmaGui::LabelText(const char* label, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		LabelTextV(label, fmt, args);
		va_end(args);
	}

	// Add a label+text combo aligned to other label+value widgets
	void KarmaGui::LabelTextV(const char* label, const char* fmt, va_list args)
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return;

		KarmaGuiContext& g = *GKarmaGui;
		const KarmaGuiStyle& style = g.Style;
		const float w = CalcItemWidth();

		const char* value_text_begin, * value_text_end;
		KGFormatStringToTempBufferV(&value_text_begin, &value_text_end, fmt, args);
		const KGVec2 value_size = CalcTextSize(value_text_begin, value_text_end, false);
		const KGVec2 label_size = CalcTextSize(label, NULL, true);

		const KGVec2 pos = window->DC.CursorPos;
		const KGRect value_bb(pos, pos + KGVec2(w, value_size.y + style.FramePadding.y * 2));
		const KGRect total_bb(pos, pos + KGVec2(w + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), KGMax(value_size.y, label_size.y) + style.FramePadding.y * 2));
		KarmaGuiInternal::ItemSize(total_bb, style.FramePadding.y);
		if (!KarmaGuiInternal::ItemAdd(total_bb, 0))
			return;

		// Render
		KarmaGuiInternal::RenderTextClipped(value_bb.Min + style.FramePadding, value_bb.Max, value_text_begin, value_text_end, &value_size, KGVec2(0.0f, 0.0f));
		if (label_size.x > 0.0f)
			KarmaGuiInternal::RenderText(KGVec2(value_bb.Max.x + style.ItemInnerSpacing.x, value_bb.Min.y + style.FramePadding.y), label);
	}

	void KarmaGui::BulletText(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		BulletTextV(fmt, args);
		va_end(args);
	}

	// Text with a little bullet aligned to the typical tree node.
	void KarmaGui::BulletTextV(const char* fmt, va_list args)
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return;

		KarmaGuiContext& g = *GKarmaGui;
		const KarmaGuiStyle& style = g.Style;

		const char* text_begin, * text_end;
		KGFormatStringToTempBufferV(&text_begin, &text_end, fmt, args);
		const KGVec2 label_size = CalcTextSize(text_begin, text_end, false);
		const KGVec2 total_size = KGVec2(g.FontSize + (label_size.x > 0.0f ? (label_size.x + style.FramePadding.x * 2) : 0.0f), label_size.y);  // Empty text doesn't add padding
		KGVec2 pos = window->DC.CursorPos;
		pos.y += window->DC.CurrLineTextBaseOffset;
		KarmaGuiInternal::ItemSize(total_size, 0.0f);
		const KGRect bb(pos, pos + total_size);
		if (!KarmaGuiInternal::ItemAdd(bb, 0))
			return;

		// Render
		KGU32 text_col = GetColorU32(KGGuiCol_Text);
		KarmaGuiInternal::RenderBullet(window->DrawList, bb.Min + KGVec2(style.FramePadding.x + g.FontSize * 0.5f, g.FontSize * 0.5f), text_col);
		KarmaGuiInternal::RenderText(bb.Min + KGVec2(g.FontSize + style.FramePadding.x * 2, 0.0f), text_begin, text_end, false);
	}

	//-------------------------------------------------------------------------
	// [SECTION] Widgets: Main
	//-------------------------------------------------------------------------
	// - ButtonBehavior() [Internal]
	// - Button()
	// - SmallButton()
	// - InvisibleButton()
	// - ArrowButton()
	// - CloseButton() [Internal]
	// - CollapseButton() [Internal]
	// - GetWindowScrollbarID() [Internal]
	// - GetWindowScrollbarRect() [Internal]
	// - Scrollbar() [Internal]
	// - ScrollbarEx() [Internal]
	// - Image()
	// - ImageButton()
	// - Checkbox()
	// - CheckboxFlagsT() [Internal]
	// - CheckboxFlags()
	// - RadioButton()
	// - ProgressBar()
	// - Bullet()
	//-------------------------------------------------------------------------

	// The ButtonBehavior() function is key to many interactions and used by many/most widgets.
	// Because we handle so many cases (keyboard/gamepad navigation, drag and drop) and many specific behavior (via KGGuiButtonFlags_),
	// this code is a little complex.
	// By far the most common path is interacting with the Mouse using the default KGGuiButtonFlags_PressedOnClickRelease button behavior.
	// See the series of events below and the corresponding state reported by dear imgui:
	//------------------------------------------------------------------------------------------------------------------------------------------------
	// with PressedOnClickRelease:             return-value  IsItemHovered()  IsItemActive()  IsItemActivated()  IsItemDeactivated()  IsItemClicked()
	//   Frame N+0 (mouse is outside bb)        -             -                -               -                  -                    -
	//   Frame N+1 (mouse moves inside bb)      -             true             -               -                  -                    -
	//   Frame N+2 (mouse button is down)       -             true             true            true               -                    true
	//   Frame N+3 (mouse button is down)       -             true             true            -                  -                    -
	//   Frame N+4 (mouse moves outside bb)     -             -                true            -                  -                    -
	//   Frame N+5 (mouse moves inside bb)      -             true             true            -                  -                    -
	//   Frame N+6 (mouse button is released)   true          true             -               -                  true                 -
	//   Frame N+7 (mouse button is released)   -             true             -               -                  -                    -
	//   Frame N+8 (mouse moves outside bb)     -             -                -               -                  -                    -
	//------------------------------------------------------------------------------------------------------------------------------------------------
	// with PressedOnClick:                    return-value  IsItemHovered()  IsItemActive()  IsItemActivated()  IsItemDeactivated()  IsItemClicked()
	//   Frame N+2 (mouse button is down)       true          true             true            true               -                    true
	//   Frame N+3 (mouse button is down)       -             true             true            -                  -                    -
	//   Frame N+6 (mouse button is released)   -             true             -               -                  true                 -
	//   Frame N+7 (mouse button is released)   -             true             -               -                  -                    -
	//------------------------------------------------------------------------------------------------------------------------------------------------
	// with PressedOnRelease:                  return-value  IsItemHovered()  IsItemActive()  IsItemActivated()  IsItemDeactivated()  IsItemClicked()
	//   Frame N+2 (mouse button is down)       -             true             -               -                  -                    true
	//   Frame N+3 (mouse button is down)       -             true             -               -                  -                    -
	//   Frame N+6 (mouse button is released)   true          true             -               -                  -                    -
	//   Frame N+7 (mouse button is released)   -             true             -               -                  -                    -
	//------------------------------------------------------------------------------------------------------------------------------------------------
	// with PressedOnDoubleClick:              return-value  IsItemHovered()  IsItemActive()  IsItemActivated()  IsItemDeactivated()  IsItemClicked()
	//   Frame N+0 (mouse button is down)       -             true             -               -                  -                    true
	//   Frame N+1 (mouse button is down)       -             true             -               -                  -                    -
	//   Frame N+2 (mouse button is released)   -             true             -               -                  -                    -
	//   Frame N+3 (mouse button is released)   -             true             -               -                  -                    -
	//   Frame N+4 (mouse button is down)       true          true             true            true               -                    true
	//   Frame N+5 (mouse button is down)       -             true             true            -                  -                    -
	//   Frame N+6 (mouse button is released)   -             true             -               -                  true                 -
	//   Frame N+7 (mouse button is released)   -             true             -               -                  -                    -
	//------------------------------------------------------------------------------------------------------------------------------------------------
	// Note that some combinations are supported,
	// - PressedOnDragDropHold can generally be associated with any flag.
	// - PressedOnDoubleClick can be associated by PressedOnClickRelease/PressedOnRelease, in which case the second release event won't be reported.
	//------------------------------------------------------------------------------------------------------------------------------------------------
	// The behavior of the return-value changes when KGGuiButtonFlags_Repeat is set:
	//                                         Repeat+                  Repeat+           Repeat+             Repeat+
	//                                         PressedOnClickRelease    PressedOnClick    PressedOnRelease    PressedOnDoubleClick
	//-------------------------------------------------------------------------------------------------------------------------------------------------
	//   Frame N+0 (mouse button is down)       -                        true              -                   true
	//   ...                                    -                        -                 -                   -
	//   Frame N + RepeatDelay                  true                     true              -                   true
	//   ...                                    -                        -                 -                   -
	//   Frame N + RepeatDelay + RepeatRate*N   true                     true              -                   true
	//-------------------------------------------------------------------------------------------------------------------------------------------------

	bool KarmaGuiInternal::ButtonBehavior(const KGRect& bb, KGGuiID id, bool* out_hovered, bool* out_held, KarmaGuiButtonFlags flags)
	{
		KarmaGuiContext& g = *GKarmaGui;
		KGGuiWindow* window = GetCurrentWindow();

		// Default only reacts to left mouse button
		if ((flags & KGGuiButtonFlags_MouseButtonMask_) == 0)
			flags |= KGGuiButtonFlags_MouseButtonDefault_;

		// Default behavior requires click + release inside bounding box
		if ((flags & KGGuiButtonFlags_PressedOnMask_) == 0)
			flags |= KGGuiButtonFlags_PressedOnDefault_;

		KGGuiWindow* backup_hovered_window = g.HoveredWindow;
		const bool flatten_hovered_children = (flags & KGGuiButtonFlags_FlattenChildren) && g.HoveredWindow && g.HoveredWindow->RootWindowDockTree == window->RootWindowDockTree;
		if (flatten_hovered_children)
			g.HoveredWindow = window;

#ifdef IMGUI_ENABLE_TEST_ENGINE
		if (id != 0 && g.LastItemData.ID != id)
			KARMAGUI_TEST_ENGINE_ITEM_ADD(bb, id);
#endif

		bool pressed = false;
		bool hovered = ItemHoverable(bb, id);

		// Drag source doesn't report as hovered
		if (hovered && g.DragDropActive && g.DragDropPayload.SourceId == id && !(g.DragDropSourceFlags & KGGuiDragDropFlags_SourceNoDisableHover))
			hovered = false;

		// Special mode for Drag and Drop where holding button pressed for a long time while dragging another item triggers the button
		if (g.DragDropActive && (flags & KGGuiButtonFlags_PressedOnDragDropHold) && !(g.DragDropSourceFlags & KGGuiDragDropFlags_SourceNoHoldToOpenOthers))
			if (KarmaGui::IsItemHovered(KGGuiHoveredFlags_AllowWhenBlockedByActiveItem))
			{
				hovered = true;
				SetHoveredID(id);
				if (g.HoveredIdTimer - g.IO.DeltaTime <= DRAGDROP_HOLD_TO_OPEN_TIMER && g.HoveredIdTimer >= DRAGDROP_HOLD_TO_OPEN_TIMER)
				{
					pressed = true;
					g.DragDropHoldJustPressedId = id;
					FocusWindow(window);
				}
			}

		if (flatten_hovered_children)
			g.HoveredWindow = backup_hovered_window;

		// AllowOverlap mode (rarely used) requires previous frame HoveredId to be null or to match. This allows using patterns where a later submitted widget overlaps a previous one.
		if (hovered && (flags & KGGuiButtonFlags_AllowItemOverlap) && (g.HoveredIdPreviousFrame != id && g.HoveredIdPreviousFrame != 0))
			hovered = false;

		// Mouse handling
		const KGGuiID test_owner_id = (flags & KGGuiButtonFlags_NoTestKeyOwner) ? KGGuiKeyOwner_Any : id;
		if (hovered)
		{
			if (!(flags & KGGuiButtonFlags_NoKeyModifiers) || (!g.IO.KeyCtrl && !g.IO.KeyShift && !g.IO.KeyAlt))
			{
				// Poll buttons
				int mouse_button_clicked = -1;
				if ((flags & KGGuiButtonFlags_MouseButtonLeft) && IsMouseClicked(0, test_owner_id)) { mouse_button_clicked = 0; }
				else if ((flags & KGGuiButtonFlags_MouseButtonRight) && IsMouseClicked(1, test_owner_id)) { mouse_button_clicked = 1; }
				else if ((flags & KGGuiButtonFlags_MouseButtonMiddle) && IsMouseClicked(2, test_owner_id)) { mouse_button_clicked = 2; }
				if (mouse_button_clicked != -1 && g.ActiveId != id)
				{
					if (!(flags & KGGuiButtonFlags_NoSetKeyOwner))
						SetKeyOwner(MouseButtonToKey(mouse_button_clicked), id);
					if (flags & (KGGuiButtonFlags_PressedOnClickRelease | KGGuiButtonFlags_PressedOnClickReleaseAnywhere))
					{
						SetActiveID(id, window);
						g.ActiveIdMouseButton = mouse_button_clicked;
						if (!(flags & KGGuiButtonFlags_NoNavFocus))
							SetFocusID(id, window);
						FocusWindow(window);
					}
					if ((flags & KGGuiButtonFlags_PressedOnClick) || ((flags & KGGuiButtonFlags_PressedOnDoubleClick) && g.IO.MouseClickedCount[mouse_button_clicked] == 2))
					{
						pressed = true;
						if (flags & KGGuiButtonFlags_NoHoldingActiveId)
							ClearActiveID();
						else
							SetActiveID(id, window); // Hold on ID
						if (!(flags & KGGuiButtonFlags_NoNavFocus))
							SetFocusID(id, window);
						g.ActiveIdMouseButton = mouse_button_clicked;
						FocusWindow(window);
					}
				}
				if (flags & KGGuiButtonFlags_PressedOnRelease)
				{
					int mouse_button_released = -1;
					if ((flags & KGGuiButtonFlags_MouseButtonLeft) && IsMouseReleased(0, test_owner_id)) { mouse_button_released = 0; }
					else if ((flags & KGGuiButtonFlags_MouseButtonRight) && IsMouseReleased(1, test_owner_id)) { mouse_button_released = 1; }
					else if ((flags & KGGuiButtonFlags_MouseButtonMiddle) && IsMouseReleased(2, test_owner_id)) { mouse_button_released = 2; }
					if (mouse_button_released != -1)
					{
						const bool has_repeated_at_least_once = (flags & KGGuiButtonFlags_Repeat) && g.IO.MouseDownDurationPrev[mouse_button_released] >= g.IO.KeyRepeatDelay; // Repeat mode trumps on release behavior
						if (!has_repeated_at_least_once)
							pressed = true;
						if (!(flags & KGGuiButtonFlags_NoNavFocus))
							SetFocusID(id, window);
						ClearActiveID();
					}
				}

				// 'Repeat' mode acts when held regardless of _PressedOn flags (see table above).
				// Relies on repeat logic of IsMouseClicked() but we may as well do it ourselves if we end up exposing finer RepeatDelay/RepeatRate settings.
				if (g.ActiveId == id && (flags & KGGuiButtonFlags_Repeat))
					if (g.IO.MouseDownDuration[g.ActiveIdMouseButton] > 0.0f && IsMouseClicked(g.ActiveIdMouseButton, test_owner_id, KGGuiInputFlags_Repeat))
						pressed = true;
			}

			if (pressed)
				g.NavDisableHighlight = true;
		}

		// Gamepad/Keyboard navigation
		// We report navigated item as hovered but we don't set g.HoveredId to not interfere with mouse.
		if (g.NavId == id && !g.NavDisableHighlight && g.NavDisableMouseHover && (g.ActiveId == 0 || g.ActiveId == id || g.ActiveId == window->MoveId))
			if (!(flags & KGGuiButtonFlags_NoHoveredOnFocus))
				hovered = true;
		if (g.NavActivateDownId == id)
		{
			bool nav_activated_by_code = (g.NavActivateId == id);
			bool nav_activated_by_inputs = (g.NavActivatePressedId == id);
			if (!nav_activated_by_inputs && (flags & KGGuiButtonFlags_Repeat))
			{
				// Avoid pressing both keys from triggering double amount of repeat events
				const KarmaGuiKeyData* key1 = GetKeyData(KGGuiKey_Space);
				const KarmaGuiKeyData* key2 = GetKeyData(KGGuiKey_NavGamepadActivate);
				const float t1 = KGMax(key1->DownDuration, key2->DownDuration);
				nav_activated_by_inputs = CalcTypematicRepeatAmount(t1 - g.IO.DeltaTime, t1, g.IO.KeyRepeatDelay, g.IO.KeyRepeatRate) > 0;
			}
			if (nav_activated_by_code || nav_activated_by_inputs)
			{
				// Set active id so it can be queried by user via IsItemActive(), equivalent of holding the mouse button.
				pressed = true;
				SetActiveID(id, window);
				g.ActiveIdSource = KGGuiInputSource_Nav;
				if (!(flags & KGGuiButtonFlags_NoNavFocus))
					SetFocusID(id, window);
			}
		}

		// Process while held
		bool held = false;
		if (g.ActiveId == id)
		{
			if (g.ActiveIdSource == KGGuiInputSource_Mouse)
			{
				if (g.ActiveIdIsJustActivated)
					g.ActiveIdClickOffset = g.IO.MousePos - bb.Min;

				const int mouse_button = g.ActiveIdMouseButton;
				if (IsMouseDown(mouse_button, test_owner_id))
				{
					held = true;
				}
				else
				{
					bool release_in = hovered && (flags & KGGuiButtonFlags_PressedOnClickRelease) != 0;
					bool release_anywhere = (flags & KGGuiButtonFlags_PressedOnClickReleaseAnywhere) != 0;
					if ((release_in || release_anywhere) && !g.DragDropActive)
					{
						// Report as pressed when releasing the mouse (this is the most common path)
						bool is_double_click_release = (flags & KGGuiButtonFlags_PressedOnDoubleClick) && g.IO.MouseReleased[mouse_button] && g.IO.MouseClickedLastCount[mouse_button] == 2;
						bool is_repeating_already = (flags & KGGuiButtonFlags_Repeat) && g.IO.MouseDownDurationPrev[mouse_button] >= g.IO.KeyRepeatDelay; // Repeat mode trumps <on release>
						bool is_button_avail_or_owned = TestKeyOwner(MouseButtonToKey(mouse_button), test_owner_id);
						if (!is_double_click_release && !is_repeating_already && is_button_avail_or_owned)
							pressed = true;
					}
					ClearActiveID();
				}
				if (!(flags & KGGuiButtonFlags_NoNavFocus))
					g.NavDisableHighlight = true;
			}
			else if (g.ActiveIdSource == KGGuiInputSource_Nav)
			{
				// When activated using Nav, we hold on the ActiveID until activation button is released
				if (g.NavActivateDownId != id)
					ClearActiveID();
			}
			if (pressed)
				g.ActiveIdHasBeenPressedBefore = true;
		}

		if (out_hovered) *out_hovered = hovered;
		if (out_held) *out_held = held;

		return pressed;
	}

	bool KarmaGuiInternal::ButtonEx(const char* label, const KGVec2& size_arg, KarmaGuiButtonFlags flags)
	{
		KGGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		KarmaGuiContext& g = *GKarmaGui;
		const KarmaGuiStyle& style = g.Style;
		const KGGuiID id = window->GetID(label);
		const KGVec2 label_size = KarmaGui::CalcTextSize(label, NULL, true);

		KGVec2 pos = window->DC.CursorPos;
		if ((flags & KGGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
			pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
		KGVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

		const KGRect bb(pos, pos + size);
		ItemSize(size, style.FramePadding.y);
		if (!ItemAdd(bb, id))
			return false;

		if (g.LastItemData.InFlags & KGGuiItemFlags_ButtonRepeat)
			flags |= KGGuiButtonFlags_Repeat;

		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

		// Render
		const KGU32 col = KarmaGui::GetColorU32((held && hovered) ? KGGuiCol_ButtonActive : hovered ? KGGuiCol_ButtonHovered : KGGuiCol_Button);
		RenderNavHighlight(bb, id);
		RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

		if (g.LogEnabled)
			LogSetNextTextDecoration("[", "]");
		RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

		// Automatically close popups
		//if (pressed && !(flags & KGGuiButtonFlags_DontClosePopups) && (window->Flags & KGGuiWindowFlags_Popup))
		//    CloseCurrentPopup();

		KARMAGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
		return pressed;
	}

	bool KarmaGui::Button(const char* label, const KGVec2& size_arg)
	{
		return KarmaGuiInternal::ButtonEx(label, size_arg, KGGuiButtonFlags_None);
	}

	// Small buttons fits within text without additional vertical spacing.
	bool KarmaGui::SmallButton(const char* label)
	{
		KarmaGuiContext& g = *GKarmaGui;
		float backup_padding_y = g.Style.FramePadding.y;
		g.Style.FramePadding.y = 0.0f;
		bool pressed = KarmaGuiInternal::ButtonEx(label, KGVec2(0, 0), KGGuiButtonFlags_AlignTextBaseLine);
		g.Style.FramePadding.y = backup_padding_y;
		return pressed;
	}

	// Tip: use KarmaGui::PushID()/PopID() to push indices or pointers in the ID stack.
	// Then you can keep 'str_id' empty or the same for all your buttons (instead of creating a string based on a non-string id)
	bool KarmaGui::InvisibleButton(const char* str_id, const KGVec2& size_arg, KarmaGuiButtonFlags flags)
	{
		KarmaGuiContext& g = *GKarmaGui;
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		// Cannot use zero-size for InvisibleButton(). Unlike Button() there is not way to fallback using the label size.
		KR_CORE_ASSERT(size_arg.x != 0.0f && size_arg.y != 0.0f, "");

		const KGGuiID id = window->GetID(str_id);
		KGVec2 size = KarmaGuiInternal::CalcItemSize(size_arg, 0.0f, 0.0f);
		const KGRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
		KarmaGuiInternal::ItemSize(size);
		if (!KarmaGuiInternal::ItemAdd(bb, id))
			return false;

		bool hovered, held;
		bool pressed = KarmaGuiInternal::ButtonBehavior(bb, id, &hovered, &held, flags);

		KARMAGUI_TEST_ENGINE_ITEM_INFO(id, str_id, g.LastItemData.StatusFlags);
		return pressed;
	}

	bool KarmaGuiInternal::ArrowButtonEx(const char* str_id, KarmaGuiDir dir, KGVec2 size, KarmaGuiButtonFlags flags)
	{
		KarmaGuiContext& g = *GKarmaGui;
		KGGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		const KGGuiID id = window->GetID(str_id);
		const KGRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
		const float default_size = KarmaGui::GetFrameHeight();
		ItemSize(size, (size.y >= default_size) ? g.Style.FramePadding.y : -1.0f);
		if (!ItemAdd(bb, id))
			return false;

		if (g.LastItemData.InFlags & KGGuiItemFlags_ButtonRepeat)
			flags |= KGGuiButtonFlags_Repeat;

		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

		// Render
		const KGU32 bg_col = KarmaGui::GetColorU32((held && hovered) ? KGGuiCol_ButtonActive : hovered ? KGGuiCol_ButtonHovered : KGGuiCol_Button);
		const KGU32 text_col = KarmaGui::GetColorU32(KGGuiCol_Text);
		RenderNavHighlight(bb, id);
		RenderFrame(bb.Min, bb.Max, bg_col, true, g.Style.FrameRounding);
		RenderArrow(window->DrawList, bb.Min + KGVec2(KGMax(0.0f, (size.x - g.FontSize) * 0.5f), KGMax(0.0f, (size.y - g.FontSize) * 0.5f)), text_col, dir);

		KARMAGUI_TEST_ENGINE_ITEM_INFO(id, str_id, g.LastItemData.StatusFlags);
		return pressed;
	}

	bool KarmaGui::ArrowButton(const char* str_id, KarmaGuiDir dir)
	{
		float sz = GetFrameHeight();
		return KarmaGuiInternal::ArrowButtonEx(str_id, dir, KGVec2(sz, sz), KGGuiButtonFlags_None);
	}

	// Button to close a window
	bool KarmaGuiInternal::CloseButton(KGGuiID id, const KGVec2& pos)
	{
		KarmaGuiContext& g = *GKarmaGui;
		KGGuiWindow* window = g.CurrentWindow;

		// Tweak 1: Shrink hit-testing area if button covers an abnormally large proportion of the visible region. That's in order to facilitate moving the window away. (#3825)
		// This may better be applied as a general hit-rect reduction mechanism for all widgets to ensure the area to move window is always accessible?
		const KGRect bb(pos, pos + KGVec2(g.FontSize, g.FontSize) + g.Style.FramePadding * 2.0f);
		KGRect bb_interact = bb;
		const float area_to_visible_ratio = window->OuterRectClipped.GetArea() / bb.GetArea();
		if (area_to_visible_ratio < 1.5f)
			bb_interact.Expand(KGFloor(bb_interact.GetSize() * -0.25f));

		// Tweak 2: We intentionally allow interaction when clipped so that a mechanical Alt,Right,Activate sequence can always close a window.
		// (this isn't the regular behavior of buttons, but it doesn't affect the user much because navigation tends to keep items visible).
		bool is_clipped = !ItemAdd(bb_interact, id);

		bool hovered, held;
		bool pressed = ButtonBehavior(bb_interact, id, &hovered, &held);
		if (is_clipped)
			return pressed;

		// Render
		// FIXME: Clarify this mess
		KGU32 col = KarmaGui::GetColorU32(held ? KGGuiCol_ButtonActive : KGGuiCol_ButtonHovered);
		KGVec2 center = bb.GetCenter();
		if (hovered)
			window->DrawList->AddCircleFilled(center, KGMax(2.0f, g.FontSize * 0.5f + 1.0f), col, 12);

		float cross_extent = g.FontSize * 0.5f * 0.7071f - 1.0f;
		KGU32 cross_col = KarmaGui::GetColorU32(KGGuiCol_Text);
		center -= KGVec2(0.5f, 0.5f);
		window->DrawList->AddLine(center + KGVec2(+cross_extent, +cross_extent), center + KGVec2(-cross_extent, -cross_extent), cross_col, 1.0f);
		window->DrawList->AddLine(center + KGVec2(+cross_extent, -cross_extent), center + KGVec2(-cross_extent, +cross_extent), cross_col, 1.0f);

		return pressed;
	}

	// The Collapse button also functions as a Dock Menu button.
	bool KarmaGuiInternal::CollapseButton(KGGuiID id, const KGVec2& pos, KGGuiDockNode* dock_node)
	{
		KarmaGuiContext& g = *GKarmaGui;
		KGGuiWindow* window = g.CurrentWindow;

		KGRect bb(pos, pos + KGVec2(g.FontSize, g.FontSize) + g.Style.FramePadding * 2.0f);
		ItemAdd(bb, id);
		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held, KGGuiButtonFlags_None);

		// Render
		//bool is_dock_menu = (window->DockNodeAsHost && !window->Collapsed);
		KGU32 bg_col = KarmaGui::GetColorU32((held && hovered) ? KGGuiCol_ButtonActive : hovered ? KGGuiCol_ButtonHovered : KGGuiCol_Button);
		KGU32 text_col = KarmaGui::GetColorU32(KGGuiCol_Text);
		if (hovered || held)
			window->DrawList->AddCircleFilled(bb.GetCenter() + KGVec2(0, -0.5f), g.FontSize * 0.5f + 1.0f, bg_col, 12);

		if (dock_node)
			RenderArrowDockMenu(window->DrawList, bb.Min + g.Style.FramePadding, g.FontSize, text_col);
		else
			RenderArrow(window->DrawList, bb.Min + g.Style.FramePadding, text_col, window->Collapsed ? KGGuiDir_Right : KGGuiDir_Down, 1.0f);

		// Switch to moving the window after mouse is moved beyond the initial drag threshold
		if (KarmaGui::IsItemActive() && KarmaGui::IsMouseDragging(0))
			StartMouseMovingWindowOrNode(window, dock_node, true);

		return pressed;
	}

	KGGuiID KarmaGuiInternal::GetWindowScrollbarID(KGGuiWindow* window, KGGuiAxis axis)
	{
		return window->GetID(axis == KGGuiAxis_X ? "#SCROLLX" : "#SCROLLY");
	}

	// Return scrollbar rectangle, must only be called for corresponding axis if window->ScrollbarX/Y is set.
	KGRect KarmaGuiInternal::GetWindowScrollbarRect(KGGuiWindow* window, KGGuiAxis axis)
	{
		const KGRect outer_rect = window->Rect();
		const KGRect inner_rect = window->InnerRect;
		const float border_size = window->WindowBorderSize;
		const float scrollbar_size = window->ScrollbarSizes[axis ^ 1]; // (ScrollbarSizes.x = width of Y scrollbar; ScrollbarSizes.y = height of X scrollbar)
		KR_CORE_ASSERT(scrollbar_size > 0.0f, "");
		if (axis == KGGuiAxis_X)
			return KGRect(inner_rect.Min.x, KGMax(outer_rect.Min.y, outer_rect.Max.y - border_size - scrollbar_size), inner_rect.Max.x, outer_rect.Max.y);
		else
			return KGRect(KGMax(outer_rect.Min.x, outer_rect.Max.x - border_size - scrollbar_size), inner_rect.Min.y, outer_rect.Max.x, inner_rect.Max.y);
	}

	void KarmaGuiInternal::Scrollbar(KGGuiAxis axis)
	{
		KarmaGuiContext& g = *GKarmaGui;
		KGGuiWindow* window = g.CurrentWindow;
		const KGGuiID id = GetWindowScrollbarID(window, axis);

		// Calculate scrollbar bounding box
		KGRect bb = GetWindowScrollbarRect(window, axis);
		KGDrawFlags rounding_corners = KGDrawFlags_RoundCornersNone;
		if (axis == KGGuiAxis_X)
		{
			rounding_corners |= KGDrawFlags_RoundCornersBottomLeft;
			if (!window->ScrollbarY)
				rounding_corners |= KGDrawFlags_RoundCornersBottomRight;
		}
		else
		{
			if ((window->Flags & KGGuiWindowFlags_NoTitleBar) && !(window->Flags & KGGuiWindowFlags_MenuBar))
				rounding_corners |= KGDrawFlags_RoundCornersTopRight;
			if (!window->ScrollbarX)
				rounding_corners |= KGDrawFlags_RoundCornersBottomRight;
		}
		float size_avail = window->InnerRect.Max[axis] - window->InnerRect.Min[axis];
		float size_contents = window->ContentSize[axis] + window->WindowPadding[axis] * 2.0f;
		KGS64 scroll = (KGS64)window->Scroll[axis];
		ScrollbarEx(bb, id, axis, &scroll, (KGS64)size_avail, (KGS64)size_contents, rounding_corners);
		window->Scroll[axis] = (float)scroll;
	}

	// Vertical/Horizontal scrollbar
	// The entire piece of code below is rather confusing because:
	// - We handle absolute seeking (when first clicking outside the grab) and relative manipulation (afterward or when clicking inside the grab)
	// - We store values as normalized ratio and in a form that allows the window content to change while we are holding on a scrollbar
	// - We handle both horizontal and vertical scrollbars, which makes the terminology not ideal.
	// Still, the code should probably be made simpler..
	bool KarmaGuiInternal::ScrollbarEx(const KGRect& bb_frame, KGGuiID id, KGGuiAxis axis, KGS64* p_scroll_v, KGS64 size_avail_v, KGS64 size_contents_v, KGDrawFlags flags)
	{
		KarmaGuiContext& g = *GKarmaGui;
		KGGuiWindow* window = g.CurrentWindow;
		if (window->SkipItems)
			return false;

		const float bb_frame_width = bb_frame.GetWidth();
		const float bb_frame_height = bb_frame.GetHeight();
		if (bb_frame_width <= 0.0f || bb_frame_height <= 0.0f)
			return false;

		// When we are too small, start hiding and disabling the grab (this reduce visual noise on very small window and facilitate using the window resize grab)
		float alpha = 1.0f;
		if ((axis == KGGuiAxis_Y) && bb_frame_height < g.FontSize + g.Style.FramePadding.y * 2.0f)
			alpha = KGSaturate((bb_frame_height - g.FontSize) / (g.Style.FramePadding.y * 2.0f));
		if (alpha <= 0.0f)
			return false;

		const KarmaGuiStyle& style = g.Style;
		const bool allow_interaction = (alpha >= 1.0f);

		KGRect bb = bb_frame;
		bb.Expand(KGVec2(-KGClamp(KG_FLOOR((bb_frame_width - 2.0f) * 0.5f), 0.0f, 3.0f), -KGClamp(KG_FLOOR((bb_frame_height - 2.0f) * 0.5f), 0.0f, 3.0f)));

		// V denote the main, longer axis of the scrollbar (= height for a vertical scrollbar)
		const float scrollbar_size_v = (axis == KGGuiAxis_X) ? bb.GetWidth() : bb.GetHeight();

		// Calculate the height of our grabbable box. It generally represent the amount visible (vs the total scrollable amount)
		// But we maintain a minimum size in pixel to allow for the user to still aim inside.
		KR_CORE_ASSERT(KGMax(size_contents_v, size_avail_v) > 0.0f, ""); // Adding this assert to check if the KGMax(XXX,1.0f) is still needed. PLEASE CONTACT ME if this triggers.
		const KGS64 win_size_v = KGMax(KGMax(size_contents_v, size_avail_v), (KGS64)1);
		const float grab_h_pixels = KGClamp(scrollbar_size_v * ((float)size_avail_v / (float)win_size_v), style.GrabMinSize, scrollbar_size_v);
		const float grab_h_norm = grab_h_pixels / scrollbar_size_v;

		// Handle input right away. None of the code of Begin() is relying on scrolling position before calling Scrollbar().
		bool held = false;
		bool hovered = false;
		ItemAdd(bb_frame, id, NULL, KGGuiItemFlags_NoNav);
		ButtonBehavior(bb, id, &hovered, &held, KGGuiButtonFlags_NoNavFocus);

		const KGS64 scroll_max = KGMax((KGS64)1, size_contents_v - size_avail_v);
		float scroll_ratio = KGSaturate((float)*p_scroll_v / (float)scroll_max);
		float grab_v_norm = scroll_ratio * (scrollbar_size_v - grab_h_pixels) / scrollbar_size_v; // Grab position in normalized space
		if (held && allow_interaction && grab_h_norm < 1.0f)
		{
			const float scrollbar_pos_v = bb.Min[axis];
			const float mouse_pos_v = g.IO.MousePos[axis];

			// Click position in scrollbar normalized space (0.0f->1.0f)
			const float clicked_v_norm = KGSaturate((mouse_pos_v - scrollbar_pos_v) / scrollbar_size_v);
			SetHoveredID(id);

			bool seek_absolute = false;
			if (g.ActiveIdIsJustActivated)
			{
				// On initial click calculate the distance between mouse and the center of the grab
				seek_absolute = (clicked_v_norm < grab_v_norm || clicked_v_norm > grab_v_norm + grab_h_norm);
				if (seek_absolute)
					g.ScrollbarClickDeltaToGrabCenter = 0.0f;
				else
					g.ScrollbarClickDeltaToGrabCenter = clicked_v_norm - grab_v_norm - grab_h_norm * 0.5f;
			}

			// Apply scroll (p_scroll_v will generally point on one member of window->Scroll)
			// It is ok to modify Scroll here because we are being called in Begin() after the calculation of ContentSize and before setting up our starting position
			const float scroll_v_norm = KGSaturate((clicked_v_norm - g.ScrollbarClickDeltaToGrabCenter - grab_h_norm * 0.5f) / (1.0f - grab_h_norm));
			*p_scroll_v = (KGS64)(scroll_v_norm * scroll_max);

			// Update values for rendering
			scroll_ratio = KGSaturate((float)*p_scroll_v / (float)scroll_max);
			grab_v_norm = scroll_ratio * (scrollbar_size_v - grab_h_pixels) / scrollbar_size_v;

			// Update distance to grab now that we have seeked and saturated
			if (seek_absolute)
				g.ScrollbarClickDeltaToGrabCenter = clicked_v_norm - grab_v_norm - grab_h_norm * 0.5f;
		}

		// Render
		const KGU32 bg_col = KarmaGui::GetColorU32(KGGuiCol_ScrollbarBg);
		const KGU32 grab_col = KarmaGui::GetColorU32(held ? KGGuiCol_ScrollbarGrabActive : hovered ? KGGuiCol_ScrollbarGrabHovered : KGGuiCol_ScrollbarGrab, alpha);
		window->DrawList->AddRectFilled(bb_frame.Min, bb_frame.Max, bg_col, window->WindowRounding, flags);
		KGRect grab_rect;
		if (axis == KGGuiAxis_X)
			grab_rect = KGRect(KGLerp(bb.Min.x, bb.Max.x, grab_v_norm), bb.Min.y, KGLerp(bb.Min.x, bb.Max.x, grab_v_norm) + grab_h_pixels, bb.Max.y);
		else
			grab_rect = KGRect(bb.Min.x, KGLerp(bb.Min.y, bb.Max.y, grab_v_norm), bb.Max.x, KGLerp(bb.Min.y, bb.Max.y, grab_v_norm) + grab_h_pixels);
		window->DrawList->AddRectFilled(grab_rect.Min, grab_rect.Max, grab_col, style.ScrollbarRounding);

		return held;
	}

	void KarmaGui::Image(KGTextureID user_texture_id, const KGVec2& size, const KGVec2& uv0, const KGVec2& uv1, const KGVec4& tint_col, const KGVec4& border_col)
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return;

		KGRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
		if (border_col.w > 0.0f)
			bb.Max += KGVec2(2, 2);
		KarmaGuiInternal::ItemSize(bb);
		if (!KarmaGuiInternal::ItemAdd(bb, 0))
			return;

		if (border_col.w > 0.0f)
		{
			window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(border_col), 0.0f);
			window->DrawList->AddImage(user_texture_id, bb.Min + KGVec2(1, 1), bb.Max - KGVec2(1, 1), uv0, uv1, GetColorU32(tint_col));
		}
		else
		{
			window->DrawList->AddImage(user_texture_id, bb.Min, bb.Max, uv0, uv1, GetColorU32(tint_col));
		}
	}

	// ImageButton() is flawed as 'id' is always derived from 'texture_id' (see #2464 #1390)
	// We provide this internal helper to write your own variant while we figure out how to redesign the public ImageButton() API.
	bool KarmaGuiInternal::ImageButtonEx(KGGuiID id, KGTextureID texture_id, const KGVec2& size, const KGVec2& uv0, const KGVec2& uv1, const KGVec4& bg_col, const KGVec4& tint_col)
	{
		KarmaGuiContext& g = *GKarmaGui;
		KGGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		const KGVec2 padding = g.Style.FramePadding;
		const KGRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2.0f);
		ItemSize(bb);
		if (!ItemAdd(bb, id))
			return false;

		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held);

		// Render
		const KGU32 col = KarmaGui::GetColorU32((held && hovered) ? KGGuiCol_ButtonActive : hovered ? KGGuiCol_ButtonHovered : KGGuiCol_Button);
		RenderNavHighlight(bb, id);
		RenderFrame(bb.Min, bb.Max, col, true, KGClamp((float)KGMin(padding.x, padding.y), 0.0f, g.Style.FrameRounding));
		if (bg_col.w > 0.0f)
			window->DrawList->AddRectFilled(bb.Min + padding, bb.Max - padding, KarmaGui::GetColorU32(bg_col));
		window->DrawList->AddImage(texture_id, bb.Min + padding, bb.Max - padding, uv0, uv1, KarmaGui::GetColorU32(tint_col));

		return pressed;
	}

	bool KarmaGui::ImageButton(const char* str_id, KGTextureID user_texture_id, const KGVec2& size, const KGVec2& uv0, const KGVec2& uv1, const KGVec4& bg_col, const KGVec4& tint_col)
	{
		KarmaGuiContext& g = *GKarmaGui;
		KGGuiWindow* window = g.CurrentWindow;
		if (window->SkipItems)
			return false;

		return KarmaGuiInternal::ImageButtonEx(window->GetID(str_id), user_texture_id, size, uv0, uv1, bg_col, tint_col);
	}
	/*
	 #ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
	 // Legacy API obsoleted in 1.89. Two differences with new ImageButton()
	 // - new ImageButton() requires an explicit 'const char* str_id'    Old ImageButton() used opaque imTextureId (created issue with: multiple buttons with same image, transient texture id values, opaque computation of ID)
	 // - new ImageButton() always use style.FramePadding                Old ImageButton() had an override argument.
	 // If you need to change padding with new ImageButton() you can use PushStyleVar(KGGuiStyleVar_FramePadding, value), consistent with other Button functions.
	 bool KarmaGuiInternal::ImageButton(KGTextureID user_texture_id, const KGVec2& size, const KGVec2& uv0, const KGVec2& uv1, int frame_padding, const KGVec4& bg_col, const KGVec4& tint_col)
	 {
	 KarmaGuiContext& g = *GKarmaGui;
	 KGGuiWindow* window = g.CurrentWindow;
	 if (window->SkipItems)
	 return false;

	 // Default to using texture ID as ID. User can still push string/integer prefixes.
	 PushID((void*)(intptr_t)user_texture_id);
	 const KGGuiID id = window->GetID("#image");
	 PopID();

	 if (frame_padding >= 0)
	 PushStyleVar(KGGuiStyleVar_FramePadding, KGVec2((float)frame_padding, (float)frame_padding));
	 bool ret = ImageButtonEx(id, user_texture_id, size, uv0, uv1, bg_col, tint_col);
	 if (frame_padding >= 0)
	 PopStyleVar();
	 return ret;
	 }
	 #endif // #ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS*/

	bool KarmaGui::Checkbox(const char* label, bool* v)
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		KarmaGuiContext& g = *GKarmaGui;
		const KarmaGuiStyle& style = g.Style;
		const KGGuiID id = window->GetID(label);
		const KGVec2 label_size = CalcTextSize(label, NULL, true);

		const float square_sz = GetFrameHeight();
		const KGVec2 pos = window->DC.CursorPos;
		const KGRect total_bb(pos, pos + KGVec2(square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f));
		KarmaGuiInternal::ItemSize(total_bb, style.FramePadding.y);
		if (!KarmaGuiInternal::ItemAdd(total_bb, id))
		{
			KARMAGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | KGGuiItemStatusFlags_Checkable | (*v ? KGGuiItemStatusFlags_Checked : 0));
			return false;
		}

		bool hovered, held;
		bool pressed = KarmaGuiInternal::ButtonBehavior(total_bb, id, &hovered, &held);
		if (pressed)
		{
			*v = !(*v);
			KarmaGuiInternal::MarkItemEdited(id);
		}

		const KGRect check_bb(pos, pos + KGVec2(square_sz, square_sz));
		KarmaGuiInternal::RenderNavHighlight(total_bb, id);
		KarmaGuiInternal::RenderFrame(check_bb.Min, check_bb.Max, GetColorU32((held && hovered) ? KGGuiCol_FrameBgActive : hovered ? KGGuiCol_FrameBgHovered : KGGuiCol_FrameBg), true, style.FrameRounding);
		KGU32 check_col = GetColorU32(KGGuiCol_CheckMark);
		bool mixed_value = (g.LastItemData.InFlags & KGGuiItemFlags_MixedValue) != 0;
		if (mixed_value)
		{
			// Undocumented tristate/mixed/indeterminate checkbox (#2644)
			// This may seem awkwardly designed because the aim is to make KGGuiItemFlags_MixedValue supported by all widgets (not just checkbox)
			KGVec2 pad(KGMax(1.0f, KG_FLOOR(square_sz / 3.6f)), KGMax(1.0f, KG_FLOOR(square_sz / 3.6f)));
			window->DrawList->AddRectFilled(check_bb.Min + pad, check_bb.Max - pad, check_col, style.FrameRounding);
		}
		else if (*v)
		{
			const float pad = KGMax(1.0f, KG_FLOOR(square_sz / 6.0f));
			KarmaGuiInternal::RenderCheckMark(window->DrawList, check_bb.Min + KGVec2(pad, pad), check_col, square_sz - pad * 2.0f);
		}

		KGVec2 label_pos = KGVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y);
		if (g.LogEnabled)
			KarmaGuiInternal::LogRenderedText(&label_pos, mixed_value ? "[~]" : *v ? "[x]" : "[ ]");
		if (label_size.x > 0.0f)
			KarmaGuiInternal::RenderText(label_pos, label);

		KARMAGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | KGGuiItemStatusFlags_Checkable | (*v ? KGGuiItemStatusFlags_Checked : 0));
		return pressed;
	}

	template<typename T>
	bool KarmaGuiInternal::CheckboxFlagsT(const char* label, T* flags, T flags_value)
	{
		bool all_on = (*flags & flags_value) == flags_value;
		bool any_on = (*flags & flags_value) != 0;
		bool pressed;
		if (!all_on && any_on)
		{
			KarmaGuiContext& g = *GKarmaGui;
			KGGuiItemFlags backup_item_flags = g.CurrentItemFlags;
			g.CurrentItemFlags |= KGGuiItemFlags_MixedValue;
			pressed = KarmaGui::Checkbox(label, &all_on);
			g.CurrentItemFlags = backup_item_flags;
		}
		else
		{
			pressed = KarmaGui::Checkbox(label, &all_on);
		}
		if (pressed)
		{
			if (all_on)
				*flags |= flags_value;
			else
				*flags &= ~flags_value;
		}
		return pressed;
	}

	bool KarmaGui::CheckboxFlags(const char* label, int* flags, int flags_value)
	{
		return KarmaGuiInternal::CheckboxFlagsT(label, flags, flags_value);
	}

	bool KarmaGui::CheckboxFlags(const char* label, unsigned int* flags, unsigned int flags_value)
	{
		return KarmaGuiInternal::CheckboxFlagsT(label, flags, flags_value);
	}

	bool KarmaGuiInternal::CheckboxFlags(const char* label, KGS64* flags, KGS64 flags_value)
	{
		return KarmaGuiInternal::CheckboxFlagsT(label, flags, flags_value);
	}

	bool KarmaGuiInternal::CheckboxFlags(const char* label, KGU64* flags, KGU64 flags_value)
	{
		return CheckboxFlagsT(label, flags, flags_value);
	}

	bool KarmaGui::RadioButton(const char* label, bool active)
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		KarmaGuiContext& g = *GKarmaGui;
		const KarmaGuiStyle& style = g.Style;
		const KGGuiID id = window->GetID(label);
		const KGVec2 label_size = CalcTextSize(label, NULL, true);

		const float square_sz = GetFrameHeight();
		const KGVec2 pos = window->DC.CursorPos;
		const KGRect check_bb(pos, pos + KGVec2(square_sz, square_sz));
		const KGRect total_bb(pos, pos + KGVec2(square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f));
		KarmaGuiInternal::ItemSize(total_bb, style.FramePadding.y);
		if (!KarmaGuiInternal::ItemAdd(total_bb, id))
			return false;

		KGVec2 center = check_bb.GetCenter();
		center.x = KG_ROUND(center.x);
		center.y = KG_ROUND(center.y);
		const float radius = (square_sz - 1.0f) * 0.5f;

		bool hovered, held;
		bool pressed = KarmaGuiInternal::ButtonBehavior(total_bb, id, &hovered, &held);
		if (pressed)
			KarmaGuiInternal::MarkItemEdited(id);

		KarmaGuiInternal::RenderNavHighlight(total_bb, id);
		window->DrawList->AddCircleFilled(center, radius, GetColorU32((held && hovered) ? KGGuiCol_FrameBgActive : hovered ? KGGuiCol_FrameBgHovered : KGGuiCol_FrameBg), 16);
		if (active)
		{
			const float pad = KGMax(1.0f, KG_FLOOR(square_sz / 6.0f));
			window->DrawList->AddCircleFilled(center, radius - pad, GetColorU32(KGGuiCol_CheckMark), 16);
		}

		if (style.FrameBorderSize > 0.0f)
		{
			window->DrawList->AddCircle(center + KGVec2(1, 1), radius, GetColorU32(KGGuiCol_BorderShadow), 16, style.FrameBorderSize);
			window->DrawList->AddCircle(center, radius, GetColorU32(KGGuiCol_Border), 16, style.FrameBorderSize);
		}

		KGVec2 label_pos = KGVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y);
		if (g.LogEnabled)
			KarmaGuiInternal::LogRenderedText(&label_pos, active ? "(x)" : "( )");
		if (label_size.x > 0.0f)
			KarmaGuiInternal::RenderText(label_pos, label);

		KARMAGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
		return pressed;
	}

	// FIXME: This would work nicely if it was a public template, e.g. 'template<T> RadioButton(const char* label, T* v, T v_button)', but I'm not sure how we would expose it..
	bool KarmaGui::RadioButton(const char* label, int* v, int v_button)
	{
		const bool pressed = RadioButton(label, *v == v_button);
		if (pressed)
			*v = v_button;
		return pressed;
	}

	// size_arg (for each axis) < 0.0f: align to end, 0.0f: auto, > 0.0f: specified size
	void KarmaGui::ProgressBar(float fraction, const KGVec2& size_arg, const char* overlay)
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return;

		KarmaGuiContext& g = *GKarmaGui;
		const KarmaGuiStyle& style = g.Style;

		KGVec2 pos = window->DC.CursorPos;
		KGVec2 size = KarmaGuiInternal::CalcItemSize(size_arg, CalcItemWidth(), g.FontSize + style.FramePadding.y * 2.0f);
		KGRect bb(pos, pos + size);
		KarmaGuiInternal::ItemSize(size, style.FramePadding.y);
		if (!KarmaGuiInternal::ItemAdd(bb, 0))
			return;

		// Render
		fraction = KGSaturate(fraction);
		KarmaGuiInternal::RenderFrame(bb.Min, bb.Max, GetColorU32(KGGuiCol_FrameBg), true, style.FrameRounding);
		bb.Expand(KGVec2(-style.FrameBorderSize, -style.FrameBorderSize));
		const KGVec2 fill_br = KGVec2(KGLerp(bb.Min.x, bb.Max.x, fraction), bb.Max.y);
		KarmaGuiInternal::RenderRectFilledRangeH(window->DrawList, bb, GetColorU32(KGGuiCol_PlotHistogram), 0.0f, fraction, style.FrameRounding);

		// Default displaying the fraction as percentage string, but user can override it
		char overlay_buf[32];
		if (!overlay)
		{
			KGFormatString(overlay_buf, KG_ARRAYSIZE(overlay_buf), "%.0f%%", fraction * 100 + 0.01f);
			overlay = overlay_buf;
		}

		KGVec2 overlay_size = CalcTextSize(overlay, NULL);
		if (overlay_size.x > 0.0f)
			KarmaGuiInternal::RenderTextClipped(KGVec2(KGClamp(fill_br.x + style.ItemSpacing.x, bb.Min.x, bb.Max.x - overlay_size.x - style.ItemInnerSpacing.x), bb.Min.y), bb.Max, overlay, NULL, &overlay_size, KGVec2(0.0f, 0.5f), &bb);
	}

	void KarmaGui::Bullet()
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return;

		KarmaGuiContext& g = *GKarmaGui;
		const KarmaGuiStyle& style = g.Style;
		const float line_height = KGMax(KGMin(window->DC.CurrLineSize.y, g.FontSize + style.FramePadding.y * 2), g.FontSize);
		const KGRect bb(window->DC.CursorPos, window->DC.CursorPos + KGVec2(g.FontSize, line_height));
		KarmaGuiInternal::ItemSize(bb);
		if (!KarmaGuiInternal::ItemAdd(bb, 0))
		{
			SameLine(0, style.FramePadding.x * 2);
			return;
		}

		// Render and stay on same line
		KGU32 text_col = GetColorU32(KGGuiCol_Text);
		KarmaGuiInternal::RenderBullet(window->DrawList, bb.Min + KGVec2(style.FramePadding.x + g.FontSize * 0.5f, line_height * 0.5f), text_col);
		SameLine(0, style.FramePadding.x * 2.0f);
	}

	//-------------------------------------------------------------------------
	// [SECTION] Widgets: Low-level Layout helpers
	//-------------------------------------------------------------------------
	// - Spacing()
	// - Dummy()
	// - NewLine()
	// - AlignTextToFramePadding()
	// - SeparatorEx() [Internal]
	// - Separator()
	// - SplitterBehavior() [Internal]
	// - ShrinkWidths() [Internal]
	//-------------------------------------------------------------------------

	void KarmaGui::Spacing()
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return;
		KarmaGuiInternal::ItemSize(KGVec2(0, 0));
	}

	void KarmaGui::Dummy(const KGVec2& size)
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return;

		const KGRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
		KarmaGuiInternal::ItemSize(size);
		KarmaGuiInternal::ItemAdd(bb, 0);
	}

	void KarmaGui::NewLine()
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return;

		KarmaGuiContext& g = *GKarmaGui;
		const KGGuiLayoutType backup_layout_type = window->DC.LayoutType;
		window->DC.LayoutType = KGGuiLayoutType_Vertical;
		window->DC.IsSameLine = false;
		if (window->DC.CurrLineSize.y > 0.0f)     // In the event that we are on a line with items that is smaller that FontSize high, we will preserve its height.
			KarmaGuiInternal::ItemSize(KGVec2(0, 0));
		else
			KarmaGuiInternal::ItemSize(KGVec2(0.0f, g.FontSize));
		window->DC.LayoutType = backup_layout_type;
	}

	void KarmaGui::AlignTextToFramePadding()
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return;

		KarmaGuiContext& g = *GKarmaGui;
		window->DC.CurrLineSize.y = KGMax(window->DC.CurrLineSize.y, g.FontSize + g.Style.FramePadding.y * 2);
		window->DC.CurrLineTextBaseOffset = KGMax(window->DC.CurrLineTextBaseOffset, g.Style.FramePadding.y);
	}

	// Horizontal/vertical separating line
	void KarmaGuiInternal::SeparatorEx(KGGuiSeparatorFlags flags)
	{
		KGGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;

		KarmaGuiContext& g = *GKarmaGui;
		KR_CORE_ASSERT(KGIsPowerOfTwo(flags & (KGGuiSeparatorFlags_Horizontal | KGGuiSeparatorFlags_Vertical)), "");   // Check that only 1 option is selected

		float thickness_draw = 1.0f;
		float thickness_layout = 0.0f;
		if (flags & KGGuiSeparatorFlags_Vertical)
		{
			// Vertical separator, for menu bars (use current line height). Not exposed because it is misleading and it doesn't have an effect on regular layout.
			float y1 = window->DC.CursorPos.y;
			float y2 = window->DC.CursorPos.y + window->DC.CurrLineSize.y;
			const KGRect bb(KGVec2(window->DC.CursorPos.x, y1), KGVec2(window->DC.CursorPos.x + thickness_draw, y2));
			ItemSize(KGVec2(thickness_layout, 0.0f));
			if (!ItemAdd(bb, 0))
				return;

			// Draw
			window->DrawList->AddLine(KGVec2(bb.Min.x, bb.Min.y), KGVec2(bb.Min.x, bb.Max.y), KarmaGui::GetColorU32(KGGuiCol_Separator));
			if (g.LogEnabled)
				KarmaGui::LogText(" |");
		}
		else if (flags & KGGuiSeparatorFlags_Horizontal)
		{
			// Horizontal Separator
			float x1 = window->Pos.x;
			float x2 = window->Pos.x + window->Size.x;

			// FIXME-WORKRECT: old hack (#205) until we decide of consistent behavior with WorkRect/Indent and Separator
			if (g.GroupStack.Size > 0 && g.GroupStack.back().WindowID == window->ID)
				x1 += window->DC.Indent.x;

			// FIXME-WORKRECT: In theory we should simply be using WorkRect.Min.x/Max.x everywhere but it isn't aesthetically what we want,
			// need to introduce a variant of WorkRect for that purpose. (#4787)
			if (KGGuiTable* table = g.CurrentTable)
			{
				x1 = table->Columns[table->CurrentColumn].MinX;
				x2 = table->Columns[table->CurrentColumn].MaxX;
			}

			KGGuiOldColumns* columns = (flags & KGGuiSeparatorFlags_SpanAllColumns) ? window->DC.CurrentColumns : NULL;
			if (columns)
				PushColumnsBackground();

			// We don't provide our width to the layout so that it doesn't get feed back into AutoFit
			// FIXME: This prevents ->CursorMaxPos based bounding box evaluation from working (e.g. TableEndCell)
			const KGRect bb(KGVec2(x1, window->DC.CursorPos.y), KGVec2(x2, window->DC.CursorPos.y + thickness_draw));
			ItemSize(KGVec2(0.0f, thickness_layout));
			const bool item_visible = ItemAdd(bb, 0);
			if (item_visible)
			{
				// Draw
				window->DrawList->AddLine(bb.Min, KGVec2(bb.Max.x, bb.Min.y), KarmaGui::GetColorU32(KGGuiCol_Separator));
				if (g.LogEnabled)
					LogRenderedText(&bb.Min, "--------------------------------\n");
			}
			if (columns)
			{
				PopColumnsBackground();
				columns->LineMinY = window->DC.CursorPos.y;
			}
		}
	}

	void KarmaGui::Separator()
	{
		KarmaGuiContext& g = *GKarmaGui;
		KGGuiWindow* window = g.CurrentWindow;
		if (window->SkipItems)
			return;

		// Those flags should eventually be overridable by the user
		KGGuiSeparatorFlags flags = (window->DC.LayoutType == KGGuiLayoutType_Horizontal) ? KGGuiSeparatorFlags_Vertical : KGGuiSeparatorFlags_Horizontal;
		flags |= KGGuiSeparatorFlags_SpanAllColumns; // NB: this only applies to legacy Columns() api as they relied on Separator() a lot.
		KarmaGuiInternal::SeparatorEx(flags);
	}

	// Using 'hover_visibility_delay' allows us to hide the highlight and mouse cursor for a short time, which can be convenient to reduce visual noise.
	bool KarmaGuiInternal::SplitterBehavior(const KGRect& bb, KGGuiID id, KGGuiAxis axis, float* size1, float* size2, float min_size1, float min_size2, float hover_extend, float hover_visibility_delay, KGU32 bg_col)
	{
		KarmaGuiContext& g = *GKarmaGui;
		KGGuiWindow* window = g.CurrentWindow;

		if (!ItemAdd(bb, id, NULL, KGGuiItemFlags_NoNav))
			return false;

		bool hovered, held;
		KGRect bb_interact = bb;
		bb_interact.Expand(axis == KGGuiAxis_Y ? KGVec2(0.0f, hover_extend) : KGVec2(hover_extend, 0.0f));
		ButtonBehavior(bb_interact, id, &hovered, &held, KGGuiButtonFlags_FlattenChildren | KGGuiButtonFlags_AllowItemOverlap);
		if (hovered)
			g.LastItemData.StatusFlags |= KGGuiItemStatusFlags_HoveredRect; // for IsItemHovered(), because bb_interact is larger than bb
		if (g.ActiveId != id)
			KarmaGui::SetItemAllowOverlap();

		if (held || (hovered && g.HoveredIdPreviousFrame == id && g.HoveredIdTimer >= hover_visibility_delay))
			KarmaGui::SetMouseCursor(axis == KGGuiAxis_Y ? KGGuiMouseCursor_ResizeNS : KGGuiMouseCursor_ResizeEW);

		KGRect bb_render = bb;
		if (held)
		{
			KGVec2 mouse_delta_2d = g.IO.MousePos - g.ActiveIdClickOffset - bb_interact.Min;
			float mouse_delta = (axis == KGGuiAxis_Y) ? mouse_delta_2d.y : mouse_delta_2d.x;

			// Minimum pane size
			float size_1_maximum_delta = KGMax(0.0f, *size1 - min_size1);
			float size_2_maximum_delta = KGMax(0.0f, *size2 - min_size2);
			if (mouse_delta < -size_1_maximum_delta)
				mouse_delta = -size_1_maximum_delta;
			if (mouse_delta > size_2_maximum_delta)
				mouse_delta = size_2_maximum_delta;

			// Apply resize
			if (mouse_delta != 0.0f)
			{
				if (mouse_delta < 0.0f)
				{
					KR_CORE_ASSERT(*size1 + mouse_delta >= min_size1, "");
				}
				if (mouse_delta > 0.0f)
				{
					KR_CORE_ASSERT(*size2 - mouse_delta >= min_size2, "");
				}
				*size1 += mouse_delta;
				*size2 -= mouse_delta;
				bb_render.Translate((axis == KGGuiAxis_X) ? KGVec2(mouse_delta, 0.0f) : KGVec2(0.0f, mouse_delta));
				MarkItemEdited(id);
			}
		}

		// Render at new position
		if (bg_col & KG_COL32_A_MASK)
			window->DrawList->AddRectFilled(bb_render.Min, bb_render.Max, bg_col, 0.0f);
		const KGU32 col = KarmaGui::GetColorU32(held ? KGGuiCol_SeparatorActive : (hovered && g.HoveredIdTimer >= hover_visibility_delay) ? KGGuiCol_SeparatorHovered : KGGuiCol_Separator);
		window->DrawList->AddRectFilled(bb_render.Min, bb_render.Max, col, 0.0f);

		return held;
	}

	static int ShrinkWidthItemComparer(const void* lhs, const void* rhs)
	{
		const KGGuiShrinkWidthItem* a = (const KGGuiShrinkWidthItem*)lhs;
		const KGGuiShrinkWidthItem* b = (const KGGuiShrinkWidthItem*)rhs;
		if (int d = (int)(b->Width - a->Width))
			return d;
		return (b->Index - a->Index);
	}

	// Shrink excess width from a set of item, by removing width from the larger items first.
	// Set items Width to -1.0f to disable shrinking this item.
	void KarmaGuiInternal::ShrinkWidths(KGGuiShrinkWidthItem* items, int count, float width_excess)
	{
		if (count == 1)
		{
			if (items[0].Width >= 0.0f)
				items[0].Width = KGMax(items[0].Width - width_excess, 1.0f);
			return;
		}
		KGQsort(items, (size_t)count, sizeof(KGGuiShrinkWidthItem), ShrinkWidthItemComparer);
		int count_same_width = 1;
		while (width_excess > 0.0f && count_same_width < count)
		{
			while (count_same_width < count && items[0].Width <= items[count_same_width].Width)
				count_same_width++;
			float max_width_to_remove_per_item = (count_same_width < count&& items[count_same_width].Width >= 0.0f) ? (items[0].Width - items[count_same_width].Width) : (items[0].Width - 1.0f);
			if (max_width_to_remove_per_item <= 0.0f)
				break;
			float width_to_remove_per_item = KGMin(width_excess / count_same_width, max_width_to_remove_per_item);
			for (int item_n = 0; item_n < count_same_width; item_n++)
				items[item_n].Width -= width_to_remove_per_item;
			width_excess -= width_to_remove_per_item * count_same_width;
		}

		// Round width and redistribute remainder
		// Ensure that e.g. the right-most tab of a shrunk tab-bar always reaches exactly at the same distance from the right-most edge of the tab bar separator.
		width_excess = 0.0f;
		for (int n = 0; n < count; n++)
		{
			float width_rounded = KGFloor(items[n].Width);
			width_excess += items[n].Width - width_rounded;
			items[n].Width = width_rounded;
		}
		while (width_excess > 0.0f)
			for (int n = 0; n < count && width_excess > 0.0f; n++)
			{
				float width_to_add = KGMin(items[n].InitialWidth - items[n].Width, 1.0f);
				items[n].Width += width_to_add;
				width_excess -= width_to_add;
			}
	}

	//-------------------------------------------------------------------------
	// [SECTION] Widgets: ComboBox
	//-------------------------------------------------------------------------
	// - CalcMaxPopupHeightFromItemCount() [Internal]
	// - BeginCombo()
	// - BeginComboPopup() [Internal]
	// - EndCombo()
	// - BeginComboPreview() [Internal]
	// - EndComboPreview() [Internal]
	// - Combo()
	//-------------------------------------------------------------------------

	static float CalcMaxPopupHeightFromItemCount(int items_count)
	{
		KarmaGuiContext& g = *GKarmaGui;
		if (items_count <= 0)
			return FLT_MAX;
		return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
	}

	bool KarmaGui::BeginCombo(const char* label, const char* preview_value, KarmaGuiComboFlags flags)
	{
		KarmaGuiContext& g = *GKarmaGui;
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();

		KGGuiNextWindowDataFlags backup_next_window_data_flags = g.NextWindowData.Flags;
		g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
		if (window->SkipItems)
			return false;

		const KarmaGuiStyle& style = g.Style;
		const KGGuiID id = window->GetID(label);
		KR_CORE_ASSERT((flags & (KGGuiComboFlags_NoArrowButton | KGGuiComboFlags_NoPreview)) != (KGGuiComboFlags_NoArrowButton | KGGuiComboFlags_NoPreview), ""); // Can't use both flags together

		const float arrow_size = (flags & KGGuiComboFlags_NoArrowButton) ? 0.0f : GetFrameHeight();
		const KGVec2 label_size = CalcTextSize(label, NULL, true);
		const float w = (flags & KGGuiComboFlags_NoPreview) ? arrow_size : CalcItemWidth();
		const KGRect bb(window->DC.CursorPos, window->DC.CursorPos + KGVec2(w, label_size.y + style.FramePadding.y * 2.0f));
		const KGRect total_bb(bb.Min, bb.Max + KGVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
		KarmaGuiInternal::ItemSize(total_bb, style.FramePadding.y);
		if (!KarmaGuiInternal::ItemAdd(total_bb, id, &bb))
			return false;

		// Open on click
		bool hovered, held;
		bool pressed = KarmaGuiInternal::ButtonBehavior(bb, id, &hovered, &held);
		const KGGuiID popup_id = KGHashStr("##ComboPopup", 0, id);
		bool popup_open = KarmaGuiInternal::IsPopupOpen(popup_id, KGGuiPopupFlags_None);
		if (pressed && !popup_open)
		{
			KarmaGuiInternal::OpenPopupEx(popup_id, KGGuiPopupFlags_None);
			popup_open = true;
		}

		// Render shape
		const KGU32 frame_col = GetColorU32(hovered ? KGGuiCol_FrameBgHovered : KGGuiCol_FrameBg);
		const float value_x2 = KGMax(bb.Min.x, bb.Max.x - arrow_size);
		KarmaGuiInternal::RenderNavHighlight(bb, id);
		if (!(flags & KGGuiComboFlags_NoPreview))
			window->DrawList->AddRectFilled(bb.Min, KGVec2(value_x2, bb.Max.y), frame_col, style.FrameRounding, (flags & KGGuiComboFlags_NoArrowButton) ? KGDrawFlags_RoundCornersAll : KGDrawFlags_RoundCornersLeft);
		if (!(flags & KGGuiComboFlags_NoArrowButton))
		{
			KGU32 bg_col = GetColorU32((popup_open || hovered) ? KGGuiCol_ButtonHovered : KGGuiCol_Button);
			KGU32 text_col = GetColorU32(KGGuiCol_Text);
			window->DrawList->AddRectFilled(KGVec2(value_x2, bb.Min.y), bb.Max, bg_col, style.FrameRounding, (w <= arrow_size) ? KGDrawFlags_RoundCornersAll : KGDrawFlags_RoundCornersRight);
			if (value_x2 + arrow_size - style.FramePadding.x <= bb.Max.x)
				KarmaGuiInternal::RenderArrow(window->DrawList, KGVec2(value_x2 + style.FramePadding.y, bb.Min.y + style.FramePadding.y), text_col, KGGuiDir_Down, 1.0f);
		}
		KarmaGuiInternal::RenderFrameBorder(bb.Min, bb.Max, style.FrameRounding);

		// Custom preview
		if (flags & KGGuiComboFlags_CustomPreview)
		{
			g.ComboPreviewData.PreviewRect = KGRect(bb.Min.x, bb.Min.y, value_x2, bb.Max.y);
			KR_CORE_ASSERT(preview_value == NULL || preview_value[0] == 0, "");
			preview_value = NULL;
		}

		// Render preview and label
		if (preview_value != NULL && !(flags & KGGuiComboFlags_NoPreview))
		{
			if (g.LogEnabled)
				KarmaGuiInternal::LogSetNextTextDecoration("{", "}");
			KarmaGuiInternal::RenderTextClipped(bb.Min + style.FramePadding, KGVec2(value_x2, bb.Max.y), preview_value, NULL, NULL);
		}
		if (label_size.x > 0)
			KarmaGuiInternal::RenderText(KGVec2(bb.Max.x + style.ItemInnerSpacing.x, bb.Min.y + style.FramePadding.y), label);

		if (!popup_open)
			return false;

		g.NextWindowData.Flags = backup_next_window_data_flags;
		return KarmaGuiInternal::BeginComboPopup(popup_id, bb, flags);
	}

	bool KarmaGuiInternal::BeginComboPopup(KGGuiID popup_id, const KGRect& bb, KarmaGuiComboFlags flags)
	{
		KarmaGuiContext& g = *GKarmaGui;
		if (!IsPopupOpen(popup_id, KGGuiPopupFlags_None))
		{
			g.NextWindowData.ClearFlags();
			return false;
		}

		// Set popup size
		float w = bb.GetWidth();
		if (g.NextWindowData.Flags & KGGuiNextWindowDataFlags_HasSizeConstraint)
		{
			g.NextWindowData.SizeConstraintRect.Min.x = KGMax(g.NextWindowData.SizeConstraintRect.Min.x, w);
		}
		else
		{
			if ((flags & KGGuiComboFlags_HeightMask_) == 0)
				flags |= KGGuiComboFlags_HeightRegular;
			KR_CORE_ASSERT(KGIsPowerOfTwo(flags & KGGuiComboFlags_HeightMask_), ""); // Only one
			int popup_max_height_in_items = -1;
			if (flags & KGGuiComboFlags_HeightRegular)     popup_max_height_in_items = 8;
			else if (flags & KGGuiComboFlags_HeightSmall)  popup_max_height_in_items = 4;
			else if (flags & KGGuiComboFlags_HeightLarge)  popup_max_height_in_items = 20;
			KarmaGui::SetNextWindowSizeConstraints(KGVec2(w, 0.0f), KGVec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));
		}

		// This is essentially a specialized version of BeginPopupEx()
		char name[16];
		KGFormatString(name, KG_ARRAYSIZE(name), "##Combo_%02d", g.BeginPopupStack.Size); // Recycle windows based on depth

		// Set position given a custom constraint (peak into expected window size so we can position it)
		// FIXME: This might be easier to express with an hypothetical SetNextWindowPosConstraints() function?
		// FIXME: This might be moved to Begin() or at least around the same spot where Tooltips and other Popups are calling FindBestWindowPosForPopupEx()?
		if (KGGuiWindow* popup_window = FindWindowByName(name))
			if (popup_window->WasActive)
			{
				// Always override 'AutoPosLastDirection' to not leave a chance for a past value to affect us.
				KGVec2 size_expected = CalcWindowNextAutoFitSize(popup_window);
				popup_window->AutoPosLastDirection = (flags & KGGuiComboFlags_PopupAlignLeft) ? KGGuiDir_Left : KGGuiDir_Down; // Left = "Below, Toward Left", Down = "Below, Toward Right (default)"
				KGRect r_outer = GetPopupAllowedExtentRect(popup_window);
				KGVec2 pos = FindBestWindowPosForPopupEx(bb.GetBL(), size_expected, &popup_window->AutoPosLastDirection, r_outer, bb, KGGuiPopupPositionPolicy_ComboBox);
				KarmaGui::SetNextWindowPos(pos);
			}

		// We don't use BeginPopupEx() solely because we have a custom name string, which we could make an argument to BeginPopupEx()
		KarmaGuiWindowFlags window_flags = KGGuiWindowFlags_AlwaysAutoResize | KGGuiWindowFlags_Popup | KGGuiWindowFlags_NoTitleBar | KGGuiWindowFlags_NoResize | KGGuiWindowFlags_NoSavedSettings | KGGuiWindowFlags_NoMove;
		KarmaGui::PushStyleVar(KGGuiStyleVar_WindowPadding, KGVec2(g.Style.FramePadding.x, g.Style.WindowPadding.y)); // Horizontally align ourselves with the framed text
		bool ret = KarmaGui::Begin(name, NULL, window_flags);
		KarmaGui::PopStyleVar();
		if (!ret)
		{
			KarmaGui::EndPopup();
			KR_CORE_ASSERT(0, "");   // This should never happen as we tested for IsPopupOpen() above
			return false;
		}
		return true;
	}

	void KarmaGui::EndCombo()
	{
		EndPopup();
	}

	// Call directly after the BeginCombo/EndCombo block. The preview is designed to only host non-interactive elements
	// (Experimental, see GitHub issues: #1658, #4168)
	bool KarmaGuiInternal::BeginComboPreview()
	{
		KarmaGuiContext& g = *GKarmaGui;
		KGGuiWindow* window = g.CurrentWindow;
		KGGuiComboPreviewData* preview_data = &g.ComboPreviewData;

		if (window->SkipItems || !(g.LastItemData.StatusFlags & KGGuiItemStatusFlags_Visible))
			return false;
		KR_CORE_ASSERT(g.LastItemData.Rect.Min.x == preview_data->PreviewRect.Min.x, g.LastItemData.Rect.Min.y == preview_data->PreviewRect.Min.y); // Didn't call after BeginCombo/EndCombo block or forgot to pass KGGuiComboFlags_CustomPreview flag?
		if (!window->ClipRect.Contains(preview_data->PreviewRect)) // Narrower test (optional)
			return false;

		// FIXME: This could be contained in a PushWorkRect() api
		preview_data->BackupCursorPos = window->DC.CursorPos;
		preview_data->BackupCursorMaxPos = window->DC.CursorMaxPos;
		preview_data->BackupCursorPosPrevLine = window->DC.CursorPosPrevLine;
		preview_data->BackupPrevLineTextBaseOffset = window->DC.PrevLineTextBaseOffset;
		preview_data->BackupLayout = window->DC.LayoutType;
		window->DC.CursorPos = preview_data->PreviewRect.Min + g.Style.FramePadding;
		window->DC.CursorMaxPos = window->DC.CursorPos;
		window->DC.LayoutType = KGGuiLayoutType_Horizontal;
		window->DC.IsSameLine = false;
		KarmaGui::PushClipRect(preview_data->PreviewRect.Min, preview_data->PreviewRect.Max, true);

		return true;
	}

	void KarmaGuiInternal::EndComboPreview()
	{
		KarmaGuiContext& g = *GKarmaGui;
		KGGuiWindow* window = g.CurrentWindow;
		KGGuiComboPreviewData* preview_data = &g.ComboPreviewData;

		// FIXME: Using CursorMaxPos approximation instead of correct AABB which we will store in KGDrawCmd in the future
		KGDrawList* draw_list = window->DrawList;
		if (window->DC.CursorMaxPos.x < preview_data->PreviewRect.Max.x && window->DC.CursorMaxPos.y < preview_data->PreviewRect.Max.y)
			if (draw_list->CmdBuffer.Size > 1) // Unlikely case that the PushClipRect() didn't create a command
			{
				draw_list->_CmdHeader.ClipRect = draw_list->CmdBuffer[draw_list->CmdBuffer.Size - 1].ClipRect = draw_list->CmdBuffer[draw_list->CmdBuffer.Size - 2].ClipRect;
				draw_list->_TryMergeDrawCmds();
			}
		KarmaGui::PopClipRect();
		window->DC.CursorPos = preview_data->BackupCursorPos;
		window->DC.CursorMaxPos = KGMax(window->DC.CursorMaxPos, preview_data->BackupCursorMaxPos);
		window->DC.CursorPosPrevLine = preview_data->BackupCursorPosPrevLine;
		window->DC.PrevLineTextBaseOffset = preview_data->BackupPrevLineTextBaseOffset;
		window->DC.LayoutType = preview_data->BackupLayout;
		window->DC.IsSameLine = false;
		preview_data->PreviewRect = KGRect();
	}

	// Getter for the old Combo() API: const char*[]
	static bool Items_ArrayGetter(void* data, int idx, const char** out_text)
	{
		const char* const* items = (const char* const*)data;
		if (out_text)
			*out_text = items[idx];
		return true;
	}

	// Getter for the old Combo() API: "item1\0item2\0item3\0"
	static bool Items_SingleStringGetter(void* data, int idx, const char** out_text)
	{
		// FIXME-OPT: we could pre-compute the indices to fasten this. But only 1 active combo means the waste is limited.
		const char* items_separated_by_zeros = (const char*)data;
		int items_count = 0;
		const char* p = items_separated_by_zeros;
		while (*p)
		{
			if (idx == items_count)
				break;
			p += strlen(p) + 1;
			items_count++;
		}
		if (!*p)
			return false;
		if (out_text)
			*out_text = p;
		return true;
	}

	// Old API, prefer using BeginCombo() nowadays if you can.
	bool KarmaGui::Combo(const char* label, int* current_item, bool (*items_getter)(void*, int, const char**), void* data, int items_count, int popup_max_height_in_items)
	{
		KarmaGuiContext& g = *GKarmaGui;

		// Call the getter to obtain the preview string which is a parameter to BeginCombo()
		const char* preview_value = NULL;
		if (*current_item >= 0 && *current_item < items_count)
			items_getter(data, *current_item, &preview_value);

		// The old Combo() API exposed "popup_max_height_in_items". The new more general BeginCombo() API doesn't have/need it, but we emulate it here.
		if (popup_max_height_in_items != -1 && !(g.NextWindowData.Flags & KGGuiNextWindowDataFlags_HasSizeConstraint))
			SetNextWindowSizeConstraints(KGVec2(0, 0), KGVec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));

		if (!BeginCombo(label, preview_value, KGGuiComboFlags_None))
			return false;

		// Display items
		// FIXME-OPT: Use clipper (but we need to disable it on the appearing frame to make sure our call to SetItemDefaultFocus() is processed)
		bool value_changed = false;
		for (int i = 0; i < items_count; i++)
		{
			PushID(i);
			const bool item_selected = (i == *current_item);
			const char* item_text;
			if (!items_getter(data, i, &item_text))
				item_text = "*Unknown item*";
			if (Selectable(item_text, item_selected))
			{
				value_changed = true;
				*current_item = i;
			}
			if (item_selected)
				SetItemDefaultFocus();
			PopID();
		}

		EndCombo();

		if (value_changed)
			KarmaGuiInternal::MarkItemEdited(g.LastItemData.ID);

		return value_changed;
	}

	// Combo box helper allowing to pass an array of strings.
	bool KarmaGui::Combo(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items)
	{
		const bool value_changed = Combo(label, current_item, Items_ArrayGetter, (void*)items, items_count, height_in_items);
		return value_changed;
	}

	// Combo box helper allowing to pass all items in a single string literal holding multiple zero-terminated items "item1\0item2\0"
	bool KarmaGui::Combo(const char* label, int* current_item, const char* items_separated_by_zeros, int height_in_items)
	{
		int items_count = 0;
		const char* p = items_separated_by_zeros;       // FIXME-OPT: Avoid computing this, or at least only when combo is open
		while (*p)
		{
			p += strlen(p) + 1;
			items_count++;
		}
		bool value_changed = Combo(label, current_item, Items_SingleStringGetter, (void*)items_separated_by_zeros, items_count, height_in_items);
		return value_changed;
	}

	//-------------------------------------------------------------------------
	// [SECTION] Data Type and Data Formatting Helpers [Internal]
	//-------------------------------------------------------------------------
	// - DataTypeGetInfo()
	// - DataTypeFormatString()
	// - DataTypeApplyOp()
	// - DataTypeApplyOpFromText()
	// - DataTypeCompare()
	// - DataTypeClamp()
	// - GetMinimumStepAtDecimalPrecision
	// - RoundScalarWithFormat<>()
	//-------------------------------------------------------------------------

	static const KGGuiDataTypeInfo GDataTypeInfo[] =
	{
		{ sizeof(char),             "S8",   "%d",   "%d"    },  // KGGuiDataType_S8
		{ sizeof(unsigned char),    "U8",   "%u",   "%u"    },
		{ sizeof(short),            "S16",  "%d",   "%d"    },  // KGGuiDataType_S16
		{ sizeof(unsigned short),   "U16",  "%u",   "%u"    },
		{ sizeof(int),              "S32",  "%d",   "%d"    },  // KGGuiDataType_S32
		{ sizeof(unsigned int),     "U32",  "%u",   "%u"    },
	#ifdef _MSC_VER
		{ sizeof(KGS64),            "S64",  "%I64d","%I64d" },  // KGGuiDataType_S64
		{ sizeof(KGU64),            "U64",  "%I64u","%I64u" },
	#else
		{ sizeof(KGS64),            "S64",  "%lld", "%lld"  },  // KGGuiDataType_S64
		{ sizeof(KGU64),            "U64",  "%llu", "%llu"  },
	#endif
		{ sizeof(float),            "float", "%.3f","%f"    },  // KGGuiDataType_Float (float are promoted to double in va_arg)
		{ sizeof(double),           "double","%f",  "%lf"   },  // KGGuiDataType_Double
	};
	//KG_STATIC_ASSERT(KG_ARRAYSIZE(GDataTypeInfo) == KGGuiDataType_COUNT);

	const KGGuiDataTypeInfo* KarmaGuiInternal::DataTypeGetInfo(KarmaGuiDataType data_type)
	{
		KR_CORE_ASSERT(data_type >= 0 && data_type < KGGuiDataType_COUNT, "");
		return &GDataTypeInfo[data_type];
	}

	int KarmaGuiInternal::DataTypeFormatString(char* buf, int buf_size, KarmaGuiDataType data_type, const void* p_data, const char* format)
	{
		// Signedness doesn't matter when pushing integer arguments
		if (data_type == KGGuiDataType_S32 || data_type == KGGuiDataType_U32)
			return KGFormatString(buf, buf_size, format, *(const KGU32*)p_data);
		if (data_type == KGGuiDataType_S64 || data_type == KGGuiDataType_U64)
			return KGFormatString(buf, buf_size, format, *(const KGU64*)p_data);
		if (data_type == KGGuiDataType_Float)
			return KGFormatString(buf, buf_size, format, *(const float*)p_data);
		if (data_type == KGGuiDataType_Double)
			return KGFormatString(buf, buf_size, format, *(const double*)p_data);
		if (data_type == KGGuiDataType_S8)
			return KGFormatString(buf, buf_size, format, *(const KGS8*)p_data);
		if (data_type == KGGuiDataType_U8)
			return KGFormatString(buf, buf_size, format, *(const KGU8*)p_data);
		if (data_type == KGGuiDataType_S16)
			return KGFormatString(buf, buf_size, format, *(const KGS16*)p_data);
		if (data_type == KGGuiDataType_U16)
			return KGFormatString(buf, buf_size, format, *(const KGU16*)p_data);
		KR_CORE_ASSERT(0, "");
		return 0;
	}

	void KarmaGuiInternal::DataTypeApplyOp(KarmaGuiDataType data_type, int op, void* output, const void* arg1, const void* arg2)
	{
		KR_CORE_ASSERT(op == '+' || op == '-', "");
		switch (data_type)
		{
		case KGGuiDataType_S8:
			if (op == '+') { *(KGS8*)output = KGAddClampOverflow(*(const KGS8*)arg1, *(const KGS8*)arg2, IM_S8_MIN, IM_S8_MAX); }
			if (op == '-') { *(KGS8*)output = KGSubClampOverflow(*(const KGS8*)arg1, *(const KGS8*)arg2, IM_S8_MIN, IM_S8_MAX); }
			return;
		case KGGuiDataType_U8:
			if (op == '+') { *(KGU8*)output = KGAddClampOverflow(*(const KGU8*)arg1, *(const KGU8*)arg2, IM_U8_MIN, IM_U8_MAX); }
			if (op == '-') { *(KGU8*)output = KGSubClampOverflow(*(const KGU8*)arg1, *(const KGU8*)arg2, IM_U8_MIN, IM_U8_MAX); }
			return;
		case KGGuiDataType_S16:
			if (op == '+') { *(KGS16*)output = KGAddClampOverflow(*(const KGS16*)arg1, *(const KGS16*)arg2, IM_S16_MIN, IM_S16_MAX); }
			if (op == '-') { *(KGS16*)output = KGSubClampOverflow(*(const KGS16*)arg1, *(const KGS16*)arg2, IM_S16_MIN, IM_S16_MAX); }
			return;
		case KGGuiDataType_U16:
			if (op == '+') { *(KGU16*)output = KGAddClampOverflow(*(const KGU16*)arg1, *(const KGU16*)arg2, IM_U16_MIN, IM_U16_MAX); }
			if (op == '-') { *(KGU16*)output = KGSubClampOverflow(*(const KGU16*)arg1, *(const KGU16*)arg2, IM_U16_MIN, IM_U16_MAX); }
			return;
		case KGGuiDataType_S32:
			if (op == '+') { *(KGS32*)output = KGAddClampOverflow(*(const KGS32*)arg1, *(const KGS32*)arg2, IM_S32_MIN, IM_S32_MAX); }
			if (op == '-') { *(KGS32*)output = KGSubClampOverflow(*(const KGS32*)arg1, *(const KGS32*)arg2, IM_S32_MIN, IM_S32_MAX); }
			return;
		case KGGuiDataType_U32:
			if (op == '+') { *(KGU32*)output = KGAddClampOverflow(*(const KGU32*)arg1, *(const KGU32*)arg2, IM_U32_MIN, IM_U32_MAX); }
			if (op == '-') { *(KGU32*)output = KGSubClampOverflow(*(const KGU32*)arg1, *(const KGU32*)arg2, IM_U32_MIN, IM_U32_MAX); }
			return;
		case KGGuiDataType_S64:
			if (op == '+') { *(KGS64*)output = KGAddClampOverflow(*(const KGS64*)arg1, *(const KGS64*)arg2, IM_S64_MIN, IM_S64_MAX); }
			if (op == '-') { *(KGS64*)output = KGSubClampOverflow(*(const KGS64*)arg1, *(const KGS64*)arg2, IM_S64_MIN, IM_S64_MAX); }
			return;
		case KGGuiDataType_U64:
			if (op == '+') { *(KGU64*)output = KGAddClampOverflow(*(const KGU64*)arg1, *(const KGU64*)arg2, IM_U64_MIN, IM_U64_MAX); }
			if (op == '-') { *(KGU64*)output = KGSubClampOverflow(*(const KGU64*)arg1, *(const KGU64*)arg2, IM_U64_MIN, IM_U64_MAX); }
			return;
		case KGGuiDataType_Float:
			if (op == '+') { *(float*)output = *(const float*)arg1 + *(const float*)arg2; }
			if (op == '-') { *(float*)output = *(const float*)arg1 - *(const float*)arg2; }
			return;
		case KGGuiDataType_Double:
			if (op == '+') { *(double*)output = *(const double*)arg1 + *(const double*)arg2; }
			if (op == '-') { *(double*)output = *(const double*)arg1 - *(const double*)arg2; }
			return;
		case KGGuiDataType_COUNT: break;
		}
		KR_CORE_ASSERT(0, "");
	}

	// User can input math operators (e.g. +100) to edit a numerical values.
	// NB: This is _not_ a full expression evaluator. We should probably add one and replace this dumb mess..
	bool KarmaGuiInternal::DataTypeApplyFromText(const char* buf, KarmaGuiDataType data_type, void* p_data, const char* format)
	{
		while (KGCharIsBlankA(*buf))
			buf++;
		if (!buf[0])
			return false;

		// Copy the value in an opaque buffer so we can compare at the end of the function if it changed at all.
		const KGGuiDataTypeInfo* type_info = DataTypeGetInfo(data_type);
		KGGuiDataTypeTempStorage data_backup;
		memcpy(&data_backup, p_data, type_info->Size);

		// Sanitize format
		// For float/double we have to ignore format with precision (e.g. "%.2f") because sscanf doesn't take them in, so force them into %f and %lf
		char format_sanitized[32];
		if (data_type == KGGuiDataType_Float || data_type == KGGuiDataType_Double)
			format = type_info->ScanFmt;
		else
			format = KGParseFormatSanitizeForScanning(format, format_sanitized, KG_ARRAYSIZE(format_sanitized));

		// Small types need a 32-bit buffer to receive the result from scanf()
		int v32 = 0;
		if (sscanf(buf, format, type_info->Size >= 4 ? p_data : &v32) < 1)
			return false;
		if (type_info->Size < 4)
		{
			if (data_type == KGGuiDataType_S8)
				*(KGS8*)p_data = (KGS8)KGClamp(v32, (int)IM_S8_MIN, (int)IM_S8_MAX);
			else if (data_type == KGGuiDataType_U8)
				*(KGU8*)p_data = (KGU8)KGClamp(v32, (int)IM_U8_MIN, (int)IM_U8_MAX);
			else if (data_type == KGGuiDataType_S16)
				*(KGS16*)p_data = (KGS16)KGClamp(v32, (int)IM_S16_MIN, (int)IM_S16_MAX);
			else if (data_type == KGGuiDataType_U16)
				*(KGU16*)p_data = (KGU16)KGClamp(v32, (int)IM_U16_MIN, (int)IM_U16_MAX);
			else
				KR_CORE_ASSERT(0, "");
		}

		return memcmp(&data_backup, p_data, type_info->Size) != 0;
	}

	template<typename T>
	static int DataTypeCompareT(const T* lhs, const T* rhs)
	{
		if (*lhs < *rhs) return -1;
		if (*lhs > *rhs) return +1;
		return 0;
	}

	int KarmaGuiInternal::DataTypeCompare(KarmaGuiDataType data_type, const void* arg_1, const void* arg_2)
	{
		switch (data_type)
		{
		case KGGuiDataType_S8:     return DataTypeCompareT<KGS8  >((const KGS8*)arg_1, (const KGS8*)arg_2);
		case KGGuiDataType_U8:     return DataTypeCompareT<KGU8  >((const KGU8*)arg_1, (const KGU8*)arg_2);
		case KGGuiDataType_S16:    return DataTypeCompareT<KGS16 >((const KGS16*)arg_1, (const KGS16*)arg_2);
		case KGGuiDataType_U16:    return DataTypeCompareT<KGU16 >((const KGU16*)arg_1, (const KGU16*)arg_2);
		case KGGuiDataType_S32:    return DataTypeCompareT<KGS32 >((const KGS32*)arg_1, (const KGS32*)arg_2);
		case KGGuiDataType_U32:    return DataTypeCompareT<KGU32 >((const KGU32*)arg_1, (const KGU32*)arg_2);
		case KGGuiDataType_S64:    return DataTypeCompareT<KGS64 >((const KGS64*)arg_1, (const KGS64*)arg_2);
		case KGGuiDataType_U64:    return DataTypeCompareT<KGU64 >((const KGU64*)arg_1, (const KGU64*)arg_2);
		case KGGuiDataType_Float:  return DataTypeCompareT<float >((const float*)arg_1, (const float*)arg_2);
		case KGGuiDataType_Double: return DataTypeCompareT<double>((const double*)arg_1, (const double*)arg_2);
		case KGGuiDataType_COUNT:  break;
		}
		KR_CORE_ASSERT(0, "");
		return 0;
	}

	template<typename T>
	static bool DataTypeClampT(T* v, const T* v_min, const T* v_max)
	{
		// Clamp, both sides are optional, return true if modified
		if (v_min && *v < *v_min) { *v = *v_min; return true; }
		if (v_max && *v > *v_max) { *v = *v_max; return true; }
		return false;
	}

	bool KarmaGuiInternal::DataTypeClamp(KarmaGuiDataType data_type, void* p_data, const void* p_min, const void* p_max)
	{
		switch (data_type)
		{
		case KGGuiDataType_S8:     return DataTypeClampT<KGS8  >((KGS8*)p_data, (const KGS8*)p_min, (const KGS8*)p_max);
		case KGGuiDataType_U8:     return DataTypeClampT<KGU8  >((KGU8*)p_data, (const KGU8*)p_min, (const KGU8*)p_max);
		case KGGuiDataType_S16:    return DataTypeClampT<KGS16 >((KGS16*)p_data, (const KGS16*)p_min, (const KGS16*)p_max);
		case KGGuiDataType_U16:    return DataTypeClampT<KGU16 >((KGU16*)p_data, (const KGU16*)p_min, (const KGU16*)p_max);
		case KGGuiDataType_S32:    return DataTypeClampT<KGS32 >((KGS32*)p_data, (const KGS32*)p_min, (const KGS32*)p_max);
		case KGGuiDataType_U32:    return DataTypeClampT<KGU32 >((KGU32*)p_data, (const KGU32*)p_min, (const KGU32*)p_max);
		case KGGuiDataType_S64:    return DataTypeClampT<KGS64 >((KGS64*)p_data, (const KGS64*)p_min, (const KGS64*)p_max);
		case KGGuiDataType_U64:    return DataTypeClampT<KGU64 >((KGU64*)p_data, (const KGU64*)p_min, (const KGU64*)p_max);
		case KGGuiDataType_Float:  return DataTypeClampT<float >((float*)p_data, (const float*)p_min, (const float*)p_max);
		case KGGuiDataType_Double: return DataTypeClampT<double>((double*)p_data, (const double*)p_min, (const double*)p_max);
		case KGGuiDataType_COUNT:  break;
		}
		KR_CORE_ASSERT(0, "");
		return false;
	}

	static float GetMinimumStepAtDecimalPrecision(int decimal_precision)
	{
		static const float min_steps[10] = { 1.0f, 0.1f, 0.01f, 0.001f, 0.0001f, 0.00001f, 0.000001f, 0.0000001f, 0.00000001f, 0.000000001f };
		if (decimal_precision < 0)
			return FLT_MIN;
		return (decimal_precision < KG_ARRAYSIZE(min_steps)) ? min_steps[decimal_precision] : KGPow(10.0f, (float)-decimal_precision);
	}

	template<typename TYPE>
	TYPE KarmaGuiInternal::RoundScalarWithFormatT(const char* format, KarmaGuiDataType data_type, TYPE v)
	{
		KG_UNUSED(data_type);
		KR_CORE_ASSERT(data_type == KGGuiDataType_Float || data_type == KGGuiDataType_Double, "");
		const char* fmt_start = KGParseFormatFindStart(format);
		if (fmt_start[0] != '%' || fmt_start[1] == '%') // Don't apply if the value is not visible in the format string
			return v;

		// Sanitize format
		char fmt_sanitized[32];
		KGParseFormatSanitizeForPrinting(fmt_start, fmt_sanitized, KG_ARRAYSIZE(fmt_sanitized));
		fmt_start = fmt_sanitized;

		// Format value with our rounding, and read back
		char v_str[64];
		KGFormatString(v_str, KG_ARRAYSIZE(v_str), fmt_start, v);
		const char* p = v_str;
		while (*p == ' ')
			p++;
		v = (TYPE)KGAtof(p);

		return v;
	}

	//-------------------------------------------------------------------------
	// [SECTION] Widgets: DragScalar, DragFloat, DragInt, etc.
	//-------------------------------------------------------------------------
	// - DragBehaviorT<>() [Internal]
	// - DragBehavior() [Internal]
	// - DragScalar()
	// - DragScalarN()
	// - DragFloat()
	// - DragFloat2()
	// - DragFloat3()
	// - DragFloat4()
	// - DragFloatRange2()
	// - DragInt()
	// - DragInt2()
	// - DragInt3()
	// - DragInt4()
	// - DragIntRange2()
	//-------------------------------------------------------------------------

	// This is called by DragBehavior() when the widget is active (held by mouse or being manipulated with Nav controls)
	template<typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
	bool KarmaGuiInternal::DragBehaviorT(KarmaGuiDataType data_type, TYPE* v, float v_speed, const TYPE v_min, const TYPE v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		KarmaGuiContext& g = *GKarmaGui;
		const KGGuiAxis axis = (flags & KGGuiSliderFlags_Vertical) ? KGGuiAxis_Y : KGGuiAxis_X;
		const bool is_clamped = (v_min < v_max);
		const bool is_logarithmic = (flags & KGGuiSliderFlags_Logarithmic) != 0;
		const bool is_floating_point = (data_type == KGGuiDataType_Float) || (data_type == KGGuiDataType_Double);

		// Default tweak speed
		if (v_speed == 0.0f && is_clamped && (v_max - v_min < FLT_MAX))
			v_speed = (float)((v_max - v_min) * g.DragSpeedDefaultRatio);

		// Inputs accumulates into g.DragCurrentAccum, which is flushed into the current value as soon as it makes a difference with our precision settings
		float adjust_delta = 0.0f;
		if (g.ActiveIdSource == KGGuiInputSource_Mouse && KarmaGui::IsMousePosValid() && IsMouseDragPastThreshold(0, g.IO.MouseDragThreshold * DRAG_MOUSE_THRESHOLD_FACTOR))
		{
			adjust_delta = g.IO.MouseDelta[axis];
			if (g.IO.KeyAlt)
				adjust_delta *= 1.0f / 100.0f;
			if (g.IO.KeyShift)
				adjust_delta *= 10.0f;
		}
		else if (g.ActiveIdSource == KGGuiInputSource_Nav)
		{
			const int decimal_precision = is_floating_point ? KGParseFormatPrecision(format, 3) : 0;
			const bool tweak_slow = KarmaGui::IsKeyDown((g.NavInputSource == KGGuiInputSource_Gamepad) ? KGGuiKey_NavGamepadTweakSlow : KGGuiKey_NavKeyboardTweakSlow);
			const bool tweak_fast = KarmaGui::IsKeyDown((g.NavInputSource == KGGuiInputSource_Gamepad) ? KGGuiKey_NavGamepadTweakFast : KGGuiKey_NavKeyboardTweakFast);
			const float tweak_factor = tweak_slow ? 1.0f / 1.0f : tweak_fast ? 10.0f : 1.0f;
			adjust_delta = GetNavTweakPressedAmount(axis) * tweak_factor;
			v_speed = KGMax(v_speed, GetMinimumStepAtDecimalPrecision(decimal_precision));
		}
		adjust_delta *= v_speed;

		// For vertical drag we currently assume that Up=higher value (like we do with vertical sliders). This may become a parameter.
		if (axis == KGGuiAxis_Y)
			adjust_delta = -adjust_delta;

		// For logarithmic use our range is effectively 0..1 so scale the delta into that range
		if (is_logarithmic && (v_max - v_min < FLT_MAX) && ((v_max - v_min) > 0.000001f)) // Epsilon to avoid /0
			adjust_delta /= (float)(v_max - v_min);

		// Clear current value on activation
		// Avoid altering values and clamping when we are _already_ past the limits and heading in the same direction, so e.g. if range is 0..255, current value is 300 and we are pushing to the right side, keep the 300.
		bool is_just_activated = g.ActiveIdIsJustActivated;
		bool is_already_past_limits_and_pushing_outward = is_clamped && ((*v >= v_max && adjust_delta > 0.0f) || (*v <= v_min && adjust_delta < 0.0f));
		if (is_just_activated || is_already_past_limits_and_pushing_outward)
		{
			g.DragCurrentAccum = 0.0f;
			g.DragCurrentAccumDirty = false;
		}
		else if (adjust_delta != 0.0f)
		{
			g.DragCurrentAccum += adjust_delta;
			g.DragCurrentAccumDirty = true;
		}

		if (!g.DragCurrentAccumDirty)
			return false;

		TYPE v_cur = *v;
		FLOATTYPE v_old_ref_for_accum_remainder = (FLOATTYPE)0.0f;

		float logarithmic_zero_epsilon = 0.0f; // Only valid when is_logarithmic is true
		const float zero_deadzone_halfsize = 0.0f; // Drag widgets have no deadzone (as it doesn't make sense)
		if (is_logarithmic)
		{
			// When using logarithmic sliders, we need to clamp to avoid hitting zero, but our choice of clamp value greatly affects slider precision. We attempt to use the specified precision to estimate a good lower bound.
			const int decimal_precision = is_floating_point ? KGParseFormatPrecision(format, 3) : 1;
			logarithmic_zero_epsilon = KGPow(0.1f, (float)decimal_precision);

			// Convert to parametric space, apply delta, convert back
			float v_old_parametric = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, v_cur, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);
			float v_new_parametric = v_old_parametric + g.DragCurrentAccum;
			v_cur = ScaleValueFromRatioT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, v_new_parametric, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);
			v_old_ref_for_accum_remainder = v_old_parametric;
		}
		else
		{
			v_cur += (SIGNEDTYPE)g.DragCurrentAccum;
		}

		// Round to user desired precision based on format string
		if (is_floating_point && !(flags & KGGuiSliderFlags_NoRoundToFormat))
			v_cur = RoundScalarWithFormatT<TYPE>(format, data_type, v_cur);

		// Preserve remainder after rounding has been applied. This also allow slow tweaking of values.
		g.DragCurrentAccumDirty = false;
		if (is_logarithmic)
		{
			// Convert to parametric space, apply delta, convert back
			float v_new_parametric = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, v_cur, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);
			g.DragCurrentAccum -= (float)(v_new_parametric - v_old_ref_for_accum_remainder);
		}
		else
		{
			g.DragCurrentAccum -= (float)((SIGNEDTYPE)v_cur - (SIGNEDTYPE)*v);
		}

		// Lose zero sign for float/double
		if (v_cur == (TYPE)-0)
			v_cur = (TYPE)0;

		// Clamp values (+ handle overflow/wrap-around for integer types)
		if (*v != v_cur && is_clamped)
		{
			if (v_cur < v_min || (v_cur > *v && adjust_delta < 0.0f && !is_floating_point))
				v_cur = v_min;
			if (v_cur > v_max || (v_cur < *v && adjust_delta > 0.0f && !is_floating_point))
				v_cur = v_max;
		}

		// Apply result
		if (*v == v_cur)
			return false;
		*v = v_cur;
		return true;
	}

	bool KarmaGuiInternal::DragBehavior(KGGuiID id, KarmaGuiDataType data_type, void* p_v, float v_speed, const void* p_min, const void* p_max, const char* format, KarmaGuiSliderFlags flags)
	{
		// Read imgui.cpp "API BREAKING CHANGES" section for 1.78 if you hit this assert.
		KR_CORE_ASSERT((flags == 1 || (flags & KGGuiSliderFlags_InvalidMask_) == 0), "Invalid KarmaGuiSliderFlags flags! Has the 'float power' argument been mistakenly cast to flags? Call function with KGGuiSliderFlags_Logarithmic flags instead.");

		KarmaGuiContext& g = *GKarmaGui;
		if (g.ActiveId == id)
		{
			// Those are the things we can do easily outside the DragBehaviorT<> template, saves code generation.
			if (g.ActiveIdSource == KGGuiInputSource_Mouse && !g.IO.MouseDown[0])
				ClearActiveID();
			else if (g.ActiveIdSource == KGGuiInputSource_Nav && g.NavActivatePressedId == id && !g.ActiveIdIsJustActivated)
				ClearActiveID();
		}
		if (g.ActiveId != id)
			return false;
		if ((g.LastItemData.InFlags & KGGuiItemFlags_ReadOnly) || (flags & KGGuiSliderFlags_ReadOnly))
			return false;

		switch (data_type)
		{
		case KGGuiDataType_S8: { KGS32 v32 = (KGS32) * (KGS8*)p_v;  bool r = DragBehaviorT<KGS32, KGS32, float>(KGGuiDataType_S32, &v32, v_speed, p_min ? *(const KGS8*)p_min : IM_S8_MIN, p_max ? *(const KGS8*)p_max : IM_S8_MAX, format, flags); if (r) *(KGS8*)p_v = (KGS8)v32; return r; }
		case KGGuiDataType_U8: { KGU32 v32 = (KGU32) * (KGU8*)p_v;  bool r = DragBehaviorT<KGU32, KGS32, float>(KGGuiDataType_U32, &v32, v_speed, p_min ? *(const KGU8*)p_min : IM_U8_MIN, p_max ? *(const KGU8*)p_max : IM_U8_MAX, format, flags); if (r) *(KGU8*)p_v = (KGU8)v32; return r; }
		case KGGuiDataType_S16: { KGS32 v32 = (KGS32) * (KGS16*)p_v; bool r = DragBehaviorT<KGS32, KGS32, float>(KGGuiDataType_S32, &v32, v_speed, p_min ? *(const KGS16*)p_min : IM_S16_MIN, p_max ? *(const KGS16*)p_max : IM_S16_MAX, format, flags); if (r) *(KGS16*)p_v = (KGS16)v32; return r; }
		case KGGuiDataType_U16: { KGU32 v32 = (KGU32) * (KGU16*)p_v; bool r = DragBehaviorT<KGU32, KGS32, float>(KGGuiDataType_U32, &v32, v_speed, p_min ? *(const KGU16*)p_min : IM_U16_MIN, p_max ? *(const KGU16*)p_max : IM_U16_MAX, format, flags); if (r) *(KGU16*)p_v = (KGU16)v32; return r; }
		case KGGuiDataType_S32:    return DragBehaviorT<KGS32, KGS32, float >(data_type, (KGS32*)p_v, v_speed, p_min ? *(const KGS32*)p_min : IM_S32_MIN, p_max ? *(const KGS32*)p_max : IM_S32_MAX, format, flags);
		case KGGuiDataType_U32:    return DragBehaviorT<KGU32, KGS32, float >(data_type, (KGU32*)p_v, v_speed, p_min ? *(const KGU32*)p_min : IM_U32_MIN, p_max ? *(const KGU32*)p_max : IM_U32_MAX, format, flags);
		case KGGuiDataType_S64:    return DragBehaviorT<KGS64, KGS64, double>(data_type, (KGS64*)p_v, v_speed, p_min ? *(const KGS64*)p_min : IM_S64_MIN, p_max ? *(const KGS64*)p_max : IM_S64_MAX, format, flags);
		case KGGuiDataType_U64:    return DragBehaviorT<KGU64, KGS64, double>(data_type, (KGU64*)p_v, v_speed, p_min ? *(const KGU64*)p_min : IM_U64_MIN, p_max ? *(const KGU64*)p_max : IM_U64_MAX, format, flags);
		case KGGuiDataType_Float:  return DragBehaviorT<float, float, float >(data_type, (float*)p_v, v_speed, p_min ? *(const float*)p_min : -FLT_MAX, p_max ? *(const float*)p_max : FLT_MAX, format, flags);
		case KGGuiDataType_Double: return DragBehaviorT<double, double, double>(data_type, (double*)p_v, v_speed, p_min ? *(const double*)p_min : -DBL_MAX, p_max ? *(const double*)p_max : DBL_MAX, format, flags);
		case KGGuiDataType_COUNT:  break;
		}
		KR_CORE_ASSERT(0, "");
		return false;
	}

	bool KarmaGuiInternal::TempInputIsActive(KGGuiID id)
	{
		KarmaGuiContext& g = *GKarmaGui;
		return (g.ActiveId == id && g.TempInputId == id);
	}

	// Note: p_data, p_min and p_max are _pointers_ to a memory address holding the data. For a Drag widget, p_min and p_max are optional.
	// Read code of e.g. DragFloat(), DragInt() etc. or examples in 'Demo->Widgets->Data Types' to understand how to use this function directly.
	bool KarmaGui::DragScalar(const char* label, KarmaGuiDataType data_type, void* p_data, float v_speed, const void* p_min, const void* p_max, const char* format, KarmaGuiSliderFlags flags)
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		KarmaGuiContext& g = *GKarmaGui;
		const KarmaGuiStyle& style = g.Style;
		const KGGuiID id = window->GetID(label);
		const float w = CalcItemWidth();

		const KGVec2 label_size = CalcTextSize(label, NULL, true);
		const KGRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + KGVec2(w, label_size.y + style.FramePadding.y * 2.0f));
		const KGRect total_bb(frame_bb.Min, frame_bb.Max + KGVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

		const bool temp_input_allowed = (flags & KGGuiSliderFlags_NoInput) == 0;
		KarmaGuiInternal::ItemSize(total_bb, style.FramePadding.y);
		if (!KarmaGuiInternal::ItemAdd(total_bb, id, &frame_bb, temp_input_allowed ? KGGuiItemFlags_Inputable : 0))
			return false;

		// Default format string when passing NULL
		if (format == NULL)
			format = KarmaGuiInternal::DataTypeGetInfo(data_type)->PrintFmt;

		const bool hovered = KarmaGuiInternal::ItemHoverable(frame_bb, id);
		bool temp_input_is_active = temp_input_allowed && KarmaGuiInternal::TempInputIsActive(id);
		if (!temp_input_is_active)
		{
			// Tabbing or CTRL-clicking on Drag turns it into an InputText
			const bool input_requested_by_tabbing = temp_input_allowed && (g.LastItemData.StatusFlags & KGGuiItemStatusFlags_FocusedByTabbing) != 0;
			const bool clicked = hovered && IsMouseClicked(0, id);
			const bool double_clicked = (hovered && g.IO.MouseClickedCount[0] == 2 && KarmaGuiInternal::TestKeyOwner(KGGuiKey_MouseLeft, id));
			const bool make_active = (input_requested_by_tabbing || clicked || double_clicked || g.NavActivateId == id || g.NavActivateInputId == id);
			if (make_active && (clicked || double_clicked))
				KarmaGuiInternal::SetKeyOwner(KGGuiKey_MouseLeft, id);
			if (make_active && temp_input_allowed)
				if (input_requested_by_tabbing || (clicked && g.IO.KeyCtrl) || double_clicked || g.NavActivateInputId == id)
					temp_input_is_active = true;

			// (Optional) simple click (without moving) turns Drag into an InputText
			if (g.IO.ConfigDragClickToInputText && temp_input_allowed && !temp_input_is_active)
				if (g.ActiveId == id && hovered && g.IO.MouseReleased[0] && !KarmaGuiInternal::IsMouseDragPastThreshold(0, g.IO.MouseDragThreshold * DRAG_MOUSE_THRESHOLD_FACTOR))
				{
					g.NavActivateId = g.NavActivateInputId = id;
					g.NavActivateFlags = KGGuiActivateFlags_PreferInput;
					temp_input_is_active = true;
				}

			if (make_active && !temp_input_is_active)
			{
				KarmaGuiInternal::SetActiveID(id, window);
				KarmaGuiInternal::SetFocusID(id, window);
				KarmaGuiInternal::FocusWindow(window);
				g.ActiveIdUsingNavDirMask = (1 << KGGuiDir_Left) | (1 << KGGuiDir_Right);
			}
		}

		if (temp_input_is_active)
		{
			// Only clamp CTRL+Click input when KGGuiSliderFlags_AlwaysClamp is set
			const bool is_clamp_input = (flags & KGGuiSliderFlags_AlwaysClamp) != 0 && (p_min == NULL || p_max == NULL || KarmaGuiInternal::DataTypeCompare(data_type, p_min, p_max) < 0);
			return KarmaGuiInternal::TempInputScalar(frame_bb, id, label, data_type, p_data, format, is_clamp_input ? p_min : NULL, is_clamp_input ? p_max : NULL);
		}

		// Draw frame
		const KGU32 frame_col = GetColorU32(g.ActiveId == id ? KGGuiCol_FrameBgActive : hovered ? KGGuiCol_FrameBgHovered : KGGuiCol_FrameBg);
		KarmaGuiInternal::RenderNavHighlight(frame_bb, id);
		KarmaGuiInternal::RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, style.FrameRounding);

		// Drag behavior
		const bool value_changed = KarmaGuiInternal::DragBehavior(id, data_type, p_data, v_speed, p_min, p_max, format, flags);
		if (value_changed)
			KarmaGuiInternal::MarkItemEdited(id);

		// Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
		char value_buf[64];
		const char* value_buf_end = value_buf + KarmaGuiInternal::DataTypeFormatString(value_buf, KG_ARRAYSIZE(value_buf), data_type, p_data, format);
		if (g.LogEnabled)
			KarmaGuiInternal::LogSetNextTextDecoration("{", "}");
		KarmaGuiInternal::RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL, KGVec2(0.5f, 0.5f));

		if (label_size.x > 0.0f)
			KarmaGuiInternal::RenderText(KGVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

		KARMAGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
		return value_changed;
	}

	bool KarmaGui::DragScalarN(const char* label, KarmaGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min, const void* p_max, const char* format, KarmaGuiSliderFlags flags)
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		KarmaGuiContext& g = *GKarmaGui;
		bool value_changed = false;
		BeginGroup();
		PushID(label);
		KarmaGuiInternal::PushMultiItemsWidths(components, CalcItemWidth());
		size_t type_size = GDataTypeInfo[data_type].Size;
		for (int i = 0; i < components; i++)
		{
			PushID(i);
			if (i > 0)
				SameLine(0, g.Style.ItemInnerSpacing.x);
			value_changed |= DragScalar("", data_type, p_data, v_speed, p_min, p_max, format, flags);
			PopID();
			PopItemWidth();
			p_data = (void*)((char*)p_data + type_size);
		}
		PopID();

		const char* label_end = KarmaGuiInternal::FindRenderedTextEnd(label);
		if (label != label_end)
		{
			SameLine(0, g.Style.ItemInnerSpacing.x);
			KarmaGuiInternal::TextEx(label, label_end);
		}

		EndGroup();
		return value_changed;
	}

	bool KarmaGui::DragFloat(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		return DragScalar(label, KGGuiDataType_Float, v, v_speed, &v_min, &v_max, format, flags);
	}

	bool KarmaGui::DragFloat2(const char* label, float v[2], float v_speed, float v_min, float v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		return DragScalarN(label, KGGuiDataType_Float, v, 2, v_speed, &v_min, &v_max, format, flags);
	}

	bool KarmaGui::DragFloat3(const char* label, float v[3], float v_speed, float v_min, float v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		return DragScalarN(label, KGGuiDataType_Float, v, 3, v_speed, &v_min, &v_max, format, flags);
	}

	bool KarmaGui::DragFloat4(const char* label, float v[4], float v_speed, float v_min, float v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		return DragScalarN(label, KGGuiDataType_Float, v, 4, v_speed, &v_min, &v_max, format, flags);
	}

	// NB: You likely want to specify the KGGuiSliderFlags_AlwaysClamp when using this.
	bool KarmaGui::DragFloatRange2(const char* label, float* v_current_min, float* v_current_max, float v_speed, float v_min, float v_max, const char* format, const char* format_max, KarmaGuiSliderFlags flags)
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		KarmaGuiContext& g = *GKarmaGui;
		PushID(label);
		BeginGroup();
		KarmaGuiInternal::PushMultiItemsWidths(2, CalcItemWidth());

		float min_min = (v_min >= v_max) ? -FLT_MAX : v_min;
		float min_max = (v_min >= v_max) ? *v_current_max : KGMin(v_max, *v_current_max);
		KarmaGuiSliderFlags min_flags = flags | ((min_min == min_max) ? KGGuiSliderFlags_ReadOnly : 0);
		bool value_changed = DragScalar("##min", KGGuiDataType_Float, v_current_min, v_speed, &min_min, &min_max, format, min_flags);
		PopItemWidth();
		SameLine(0, g.Style.ItemInnerSpacing.x);

		float max_min = (v_min >= v_max) ? *v_current_min : KGMax(v_min, *v_current_min);
		float max_max = (v_min >= v_max) ? FLT_MAX : v_max;
		KarmaGuiSliderFlags max_flags = flags | ((max_min == max_max) ? KGGuiSliderFlags_ReadOnly : 0);
		value_changed |= DragScalar("##max", KGGuiDataType_Float, v_current_max, v_speed, &max_min, &max_max, format_max ? format_max : format, max_flags);
		PopItemWidth();
		SameLine(0, g.Style.ItemInnerSpacing.x);

		KarmaGuiInternal::TextEx(label, KarmaGuiInternal::FindRenderedTextEnd(label));
		EndGroup();
		PopID();

		return value_changed;
	}

	// NB: v_speed is float to allow adjusting the drag speed with more precision
	bool KarmaGui::DragInt(const char* label, int* v, float v_speed, int v_min, int v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		return DragScalar(label, KGGuiDataType_S32, v, v_speed, &v_min, &v_max, format, flags);
	}

	bool KarmaGui::DragInt2(const char* label, int v[2], float v_speed, int v_min, int v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		return DragScalarN(label, KGGuiDataType_S32, v, 2, v_speed, &v_min, &v_max, format, flags);
	}

	bool KarmaGui::DragInt3(const char* label, int v[3], float v_speed, int v_min, int v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		return DragScalarN(label, KGGuiDataType_S32, v, 3, v_speed, &v_min, &v_max, format, flags);
	}

	bool KarmaGui::DragInt4(const char* label, int v[4], float v_speed, int v_min, int v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		return DragScalarN(label, KGGuiDataType_S32, v, 4, v_speed, &v_min, &v_max, format, flags);
	}

	// NB: You likely want to specify the KGGuiSliderFlags_AlwaysClamp when using this.
	bool KarmaGui::DragIntRange2(const char* label, int* v_current_min, int* v_current_max, float v_speed, int v_min, int v_max, const char* format, const char* format_max, KarmaGuiSliderFlags flags)
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		KarmaGuiContext& g = *GKarmaGui;
		PushID(label);
		BeginGroup();
		KarmaGuiInternal::PushMultiItemsWidths(2, CalcItemWidth());

		int min_min = (v_min >= v_max) ? INT_MIN : v_min;
		int min_max = (v_min >= v_max) ? *v_current_max : KGMin(v_max, *v_current_max);
		KarmaGuiSliderFlags min_flags = flags | ((min_min == min_max) ? KGGuiSliderFlags_ReadOnly : 0);
		bool value_changed = DragInt("##min", v_current_min, v_speed, min_min, min_max, format, min_flags);
		PopItemWidth();
		SameLine(0, g.Style.ItemInnerSpacing.x);

		int max_min = (v_min >= v_max) ? *v_current_min : KGMax(v_min, *v_current_min);
		int max_max = (v_min >= v_max) ? INT_MAX : v_max;
		KarmaGuiSliderFlags max_flags = flags | ((max_min == max_max) ? KGGuiSliderFlags_ReadOnly : 0);
		value_changed |= DragInt("##max", v_current_max, v_speed, max_min, max_max, format_max ? format_max : format, max_flags);
		PopItemWidth();
		SameLine(0, g.Style.ItemInnerSpacing.x);

		KarmaGuiInternal::TextEx(label, KarmaGuiInternal::FindRenderedTextEnd(label));
		EndGroup();
		PopID();

		return value_changed;
	}

	//-------------------------------------------------------------------------
	// [SECTION] Widgets: SliderScalar, SliderFloat, SliderInt, etc.
	//-------------------------------------------------------------------------
	// - ScaleRatioFromValueT<> [Internal]
	// - ScaleValueFromRatioT<> [Internal]
	// - SliderBehaviorT<>() [Internal]
	// - SliderBehavior() [Internal]
	// - SliderScalar()
	// - SliderScalarN()
	// - SliderFloat()
	// - SliderFloat2()
	// - SliderFloat3()
	// - SliderFloat4()
	// - SliderAngle()
	// - SliderInt()
	// - SliderInt2()
	// - SliderInt3()
	// - SliderInt4()
	// - VSliderScalar()
	// - VSliderFloat()
	// - VSliderInt()
	//-------------------------------------------------------------------------

	// Convert a value v in the output space of a slider into a parametric position on the slider itself (the logical opposite of ScaleValueFromRatioT)
	template<typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
	float KarmaGuiInternal::ScaleRatioFromValueT(KarmaGuiDataType data_type, TYPE v, TYPE v_min, TYPE v_max, bool is_logarithmic, float logarithmic_zero_epsilon, float zero_deadzone_halfsize)
	{
		if (v_min == v_max)
			return 0.0f;
		KG_UNUSED(data_type);

		const TYPE v_clamped = (v_min < v_max) ? KGClamp(v, v_min, v_max) : KGClamp(v, v_max, v_min);
		if (is_logarithmic)
		{
			bool flipped = v_max < v_min;

			if (flipped) // Handle the case where the range is backwards
				KGSwap(v_min, v_max);

			// Fudge min/max to avoid getting close to log(0)
			FLOATTYPE v_min_fudged = (KGAbs((FLOATTYPE)v_min) < logarithmic_zero_epsilon) ? ((v_min < 0.0f) ? -logarithmic_zero_epsilon : logarithmic_zero_epsilon) : (FLOATTYPE)v_min;
			FLOATTYPE v_max_fudged = (KGAbs((FLOATTYPE)v_max) < logarithmic_zero_epsilon) ? ((v_max < 0.0f) ? -logarithmic_zero_epsilon : logarithmic_zero_epsilon) : (FLOATTYPE)v_max;

			// Awkward special cases - we need ranges of the form (-100 .. 0) to convert to (-100 .. -epsilon), not (-100 .. epsilon)
			if ((v_min == 0.0f) && (v_max < 0.0f))
				v_min_fudged = -logarithmic_zero_epsilon;
			else if ((v_max == 0.0f) && (v_min < 0.0f))
				v_max_fudged = -logarithmic_zero_epsilon;

			float result;
			if (v_clamped <= v_min_fudged)
				result = 0.0f; // Workaround for values that are in-range but below our fudge
			else if (v_clamped >= v_max_fudged)
				result = 1.0f; // Workaround for values that are in-range but above our fudge
			else if ((v_min * v_max) < 0.0f) // Range crosses zero, so split into two portions
			{
				float zero_point_center = (-(float)v_min) / ((float)v_max - (float)v_min); // The zero point in parametric space.  There's an argument we should take the logarithmic nature into account when calculating this, but for now this should do (and the most common case of a symmetrical range works fine)
				float zero_point_snap_L = zero_point_center - zero_deadzone_halfsize;
				float zero_point_snap_R = zero_point_center + zero_deadzone_halfsize;
				if (v == 0.0f)
					result = zero_point_center; // Special case for exactly zero
				else if (v < 0.0f)
					result = (1.0f - (float)(KGLog(-(FLOATTYPE)v_clamped / logarithmic_zero_epsilon) / KGLog(-v_min_fudged / logarithmic_zero_epsilon))) * zero_point_snap_L;
				else
					result = zero_point_snap_R + ((float)(KGLog((FLOATTYPE)v_clamped / logarithmic_zero_epsilon) / KGLog(v_max_fudged / logarithmic_zero_epsilon)) * (1.0f - zero_point_snap_R));
			}
			else if ((v_min < 0.0f) || (v_max < 0.0f)) // Entirely negative slider
				result = 1.0f - (float)(KGLog(-(FLOATTYPE)v_clamped / -v_max_fudged) / KGLog(-v_min_fudged / -v_max_fudged));
			else
				result = (float)(KGLog((FLOATTYPE)v_clamped / v_min_fudged) / KGLog(v_max_fudged / v_min_fudged));

			return flipped ? (1.0f - result) : result;
		}
		else
		{
			// Linear slider
			return (float)((FLOATTYPE)(SIGNEDTYPE)(v_clamped - v_min) / (FLOATTYPE)(SIGNEDTYPE)(v_max - v_min));
		}
	}

	// Convert a parametric position on a slider into a value v in the output space (the logical opposite of ScaleRatioFromValueT)
	template<typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
	TYPE KarmaGuiInternal::ScaleValueFromRatioT(KarmaGuiDataType data_type, float t, TYPE v_min, TYPE v_max, bool is_logarithmic, float logarithmic_zero_epsilon, float zero_deadzone_halfsize)
	{
		// We special-case the extents because otherwise our logarithmic fudging can lead to "mathematically correct"
		// but non-intuitive behaviors like a fully-left slider not actually reaching the minimum value. Also generally simpler.
		if (t <= 0.0f || v_min == v_max)
			return v_min;
		if (t >= 1.0f)
			return v_max;

		TYPE result = (TYPE)0;
		if (is_logarithmic)
		{
			// Fudge min/max to avoid getting silly results close to zero
			FLOATTYPE v_min_fudged = (KGAbs((FLOATTYPE)v_min) < logarithmic_zero_epsilon) ? ((v_min < 0.0f) ? -logarithmic_zero_epsilon : logarithmic_zero_epsilon) : (FLOATTYPE)v_min;
			FLOATTYPE v_max_fudged = (KGAbs((FLOATTYPE)v_max) < logarithmic_zero_epsilon) ? ((v_max < 0.0f) ? -logarithmic_zero_epsilon : logarithmic_zero_epsilon) : (FLOATTYPE)v_max;

			const bool flipped = v_max < v_min; // Check if range is "backwards"
			if (flipped)
				KGSwap(v_min_fudged, v_max_fudged);

			// Awkward special case - we need ranges of the form (-100 .. 0) to convert to (-100 .. -epsilon), not (-100 .. epsilon)
			if ((v_max == 0.0f) && (v_min < 0.0f))
				v_max_fudged = -logarithmic_zero_epsilon;

			float t_with_flip = flipped ? (1.0f - t) : t; // t, but flipped if necessary to account for us flipping the range

			if ((v_min * v_max) < 0.0f) // Range crosses zero, so we have to do this in two parts
			{
				float zero_point_center = (-(float)KGMin(v_min, v_max)) / KGAbs((float)v_max - (float)v_min); // The zero point in parametric space
				float zero_point_snap_L = zero_point_center - zero_deadzone_halfsize;
				float zero_point_snap_R = zero_point_center + zero_deadzone_halfsize;
				if (t_with_flip >= zero_point_snap_L && t_with_flip <= zero_point_snap_R)
					result = (TYPE)0.0f; // Special case to make getting exactly zero possible (the epsilon prevents it otherwise)
				else if (t_with_flip < zero_point_center)
					result = (TYPE)-(logarithmic_zero_epsilon * KGPow(-v_min_fudged / logarithmic_zero_epsilon, (FLOATTYPE)(1.0f - (t_with_flip / zero_point_snap_L))));
				else
					result = (TYPE)(logarithmic_zero_epsilon * KGPow(v_max_fudged / logarithmic_zero_epsilon, (FLOATTYPE)((t_with_flip - zero_point_snap_R) / (1.0f - zero_point_snap_R))));
			}
			else if ((v_min < 0.0f) || (v_max < 0.0f)) // Entirely negative slider
				result = (TYPE)-(-v_max_fudged * KGPow(-v_min_fudged / -v_max_fudged, (FLOATTYPE)(1.0f - t_with_flip)));
			else
				result = (TYPE)(v_min_fudged * KGPow(v_max_fudged / v_min_fudged, (FLOATTYPE)t_with_flip));
		}
		else
		{
			// Linear slider
			const bool is_floating_point = (data_type == KGGuiDataType_Float) || (data_type == KGGuiDataType_Double);
			if (is_floating_point)
			{
				result = KGLerp(v_min, v_max, t);
			}
			else if (t < 1.0)
			{
				// - For integer values we want the clicking position to match the grab box so we round above
				//   This code is carefully tuned to work with large values (e.g. high ranges of U64) while preserving this property..
				// - Not doing a *1.0 multiply at the end of a range as it tends to be lossy. While absolute aiming at a large s64/u64
				//   range is going to be imprecise anyway, with this check we at least make the edge values matches expected limits.
				FLOATTYPE v_new_off_f = (SIGNEDTYPE)(v_max - v_min) * t;
				result = (TYPE)((SIGNEDTYPE)v_min + (SIGNEDTYPE)(v_new_off_f + (FLOATTYPE)(v_min > v_max ? -0.5 : 0.5)));
			}
		}

		return result;
	}

	// FIXME: Try to move more of the code into shared SliderBehavior()
	template<typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
	bool KarmaGuiInternal::SliderBehaviorT(const KGRect& bb, KGGuiID id, KarmaGuiDataType data_type, TYPE* v, const TYPE v_min, const TYPE v_max, const char* format, KarmaGuiSliderFlags flags, KGRect* out_grab_bb)
	{
		KarmaGuiContext& g = *GKarmaGui;
		const KarmaGuiStyle& style = g.Style;

		const KGGuiAxis axis = (flags & KGGuiSliderFlags_Vertical) ? KGGuiAxis_Y : KGGuiAxis_X;
		const bool is_logarithmic = (flags & KGGuiSliderFlags_Logarithmic) != 0;
		const bool is_floating_point = (data_type == KGGuiDataType_Float) || (data_type == KGGuiDataType_Double);
		const SIGNEDTYPE v_range = (v_min < v_max ? v_max - v_min : v_min - v_max);

		// Calculate bounds
		const float grab_padding = 2.0f; // FIXME: Should be part of style.
		const float slider_sz = (bb.Max[axis] - bb.Min[axis]) - grab_padding * 2.0f;
		float grab_sz = style.GrabMinSize;
		if (!is_floating_point && v_range >= 0)                                     // v_range < 0 may happen on integer overflows
			grab_sz = KGMax((float)(slider_sz / (v_range + 1)), style.GrabMinSize); // For integer sliders: if possible have the grab size represent 1 unit
		grab_sz = KGMin(grab_sz, slider_sz);
		const float slider_usable_sz = slider_sz - grab_sz;
		const float slider_usable_pos_min = bb.Min[axis] + grab_padding + grab_sz * 0.5f;
		const float slider_usable_pos_max = bb.Max[axis] - grab_padding - grab_sz * 0.5f;

		float logarithmic_zero_epsilon = 0.0f; // Only valid when is_logarithmic is true
		float zero_deadzone_halfsize = 0.0f; // Only valid when is_logarithmic is true
		if (is_logarithmic)
		{
			// When using logarithmic sliders, we need to clamp to avoid hitting zero, but our choice of clamp value greatly affects slider precision. We attempt to use the specified precision to estimate a good lower bound.
			const int decimal_precision = is_floating_point ? KGParseFormatPrecision(format, 3) : 1;
			logarithmic_zero_epsilon = KGPow(0.1f, (float)decimal_precision);
			zero_deadzone_halfsize = (style.LogSliderDeadzone * 0.5f) / KGMax(slider_usable_sz, 1.0f);
		}

		// Process interacting with the slider
		bool value_changed = false;
		if (g.ActiveId == id)
		{
			bool set_new_value = false;
			float clicked_t = 0.0f;
			if (g.ActiveIdSource == KGGuiInputSource_Mouse)
			{
				if (!g.IO.MouseDown[0])
				{
					ClearActiveID();
				}
				else
				{
					const float mouse_abs_pos = g.IO.MousePos[axis];
					if (g.ActiveIdIsJustActivated)
					{
						float grab_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, *v, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);
						if (axis == KGGuiAxis_Y)
							grab_t = 1.0f - grab_t;
						const float grab_pos = KGLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
						const bool clicked_around_grab = (mouse_abs_pos >= grab_pos - grab_sz * 0.5f - 1.0f) && (mouse_abs_pos <= grab_pos + grab_sz * 0.5f + 1.0f); // No harm being extra generous here.
						g.SliderGrabClickOffset = (clicked_around_grab && is_floating_point) ? mouse_abs_pos - grab_pos : 0.0f;
					}
					if (slider_usable_sz > 0.0f)
						clicked_t = KGSaturate((mouse_abs_pos - g.SliderGrabClickOffset - slider_usable_pos_min) / slider_usable_sz);
					if (axis == KGGuiAxis_Y)
						clicked_t = 1.0f - clicked_t;
					set_new_value = true;
				}
			}
			else if (g.ActiveIdSource == KGGuiInputSource_Nav)
			{
				if (g.ActiveIdIsJustActivated)
				{
					g.SliderCurrentAccum = 0.0f; // Reset any stored nav delta upon activation
					g.SliderCurrentAccumDirty = false;
				}

				float input_delta = (axis == KGGuiAxis_X) ? GetNavTweakPressedAmount(axis) : -GetNavTweakPressedAmount(axis);
				if (input_delta != 0.0f)
				{
					const bool tweak_slow = KarmaGui::IsKeyDown((g.NavInputSource == KGGuiInputSource_Gamepad) ? KGGuiKey_NavGamepadTweakSlow : KGGuiKey_NavKeyboardTweakSlow);
					const bool tweak_fast = KarmaGui::IsKeyDown((g.NavInputSource == KGGuiInputSource_Gamepad) ? KGGuiKey_NavGamepadTweakFast : KGGuiKey_NavKeyboardTweakFast);
					const int decimal_precision = is_floating_point ? KGParseFormatPrecision(format, 3) : 0;
					if (decimal_precision > 0)
					{
						input_delta /= 100.0f;    // Gamepad/keyboard tweak speeds in % of slider bounds
						if (tweak_slow)
							input_delta /= 10.0f;
					}
					else
					{
						if ((v_range >= -100.0f && v_range <= 100.0f) || tweak_slow)
							input_delta = ((input_delta < 0.0f) ? -1.0f : +1.0f) / (float)v_range; // Gamepad/keyboard tweak speeds in integer steps
						else
							input_delta /= 100.0f;
					}
					if (tweak_fast)
						input_delta *= 10.0f;

					g.SliderCurrentAccum += input_delta;
					g.SliderCurrentAccumDirty = true;
				}

				float delta = g.SliderCurrentAccum;
				if (g.NavActivatePressedId == id && !g.ActiveIdIsJustActivated)
				{
					ClearActiveID();
				}
				else if (g.SliderCurrentAccumDirty)
				{
					clicked_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, *v, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);

					if ((clicked_t >= 1.0f && delta > 0.0f) || (clicked_t <= 0.0f && delta < 0.0f)) // This is to avoid applying the saturation when already past the limits
					{
						set_new_value = false;
						g.SliderCurrentAccum = 0.0f; // If pushing up against the limits, don't continue to accumulate
					}
					else
					{
						set_new_value = true;
						float old_clicked_t = clicked_t;
						clicked_t = KGSaturate(clicked_t + delta);

						// Calculate what our "new" clicked_t will be, and thus how far we actually moved the slider, and subtract this from the accumulator
						TYPE v_new = ScaleValueFromRatioT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, clicked_t, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);
						if (is_floating_point && !(flags & KGGuiSliderFlags_NoRoundToFormat))
							v_new = RoundScalarWithFormatT<TYPE>(format, data_type, v_new);
						float new_clicked_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, v_new, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);

						if (delta > 0)
							g.SliderCurrentAccum -= KGMin(new_clicked_t - old_clicked_t, delta);
						else
							g.SliderCurrentAccum -= KGMax(new_clicked_t - old_clicked_t, delta);
					}

					g.SliderCurrentAccumDirty = false;
				}
			}

			if (set_new_value)
			{
				TYPE v_new = ScaleValueFromRatioT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, clicked_t, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);

				// Round to user desired precision based on format string
				if (is_floating_point && !(flags & KGGuiSliderFlags_NoRoundToFormat))
					v_new = RoundScalarWithFormatT<TYPE>(format, data_type, v_new);

				// Apply result
				if (*v != v_new)
				{
					*v = v_new;
					value_changed = true;
				}
			}
		}

		if (slider_sz < 1.0f)
		{
			*out_grab_bb = KGRect(bb.Min, bb.Min);
		}
		else
		{
			// Output grab position so it can be displayed by the caller
			float grab_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, *v, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);
			if (axis == KGGuiAxis_Y)
				grab_t = 1.0f - grab_t;
			const float grab_pos = KGLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
			if (axis == KGGuiAxis_X)
				*out_grab_bb = KGRect(grab_pos - grab_sz * 0.5f, bb.Min.y + grab_padding, grab_pos + grab_sz * 0.5f, bb.Max.y - grab_padding);
			else
				*out_grab_bb = KGRect(bb.Min.x + grab_padding, grab_pos - grab_sz * 0.5f, bb.Max.x - grab_padding, grab_pos + grab_sz * 0.5f);
		}

		return value_changed;
	}

	// For 32-bit and larger types, slider bounds are limited to half the natural type range.
	// So e.g. an integer Slider between INT_MAX-10 and INT_MAX will fail, but an integer Slider between INT_MAX/2-10 and INT_MAX/2 will be ok.
	// It would be possible to lift that limitation with some work but it doesn't seem to be worth it for sliders.
	bool KarmaGuiInternal::SliderBehavior(const KGRect& bb, KGGuiID id, KarmaGuiDataType data_type, void* p_v, const void* p_min, const void* p_max, const char* format, KarmaGuiSliderFlags flags, KGRect* out_grab_bb)
	{
		// Read imgui.cpp "API BREAKING CHANGES" section for 1.78 if you hit this assert.
		KR_CORE_ASSERT((flags == 1 || (flags & KGGuiSliderFlags_InvalidMask_) == 0), "Invalid KarmaGuiSliderFlags flag!  Has the 'float power' argument been mistakenly cast to flags? Call function with KGGuiSliderFlags_Logarithmic flags instead.");

		// Those are the things we can do easily outside the SliderBehaviorT<> template, saves code generation.
		KarmaGuiContext& g = *GKarmaGui;
		if ((g.LastItemData.InFlags & KGGuiItemFlags_ReadOnly) || (flags & KGGuiSliderFlags_ReadOnly))
			return false;

		switch (data_type)
		{
		case KGGuiDataType_S8: { KGS32 v32 = (KGS32) * (KGS8*)p_v;  bool r = SliderBehaviorT<KGS32, KGS32, float>(bb, id, KGGuiDataType_S32, &v32, *(const KGS8*)p_min, *(const KGS8*)p_max, format, flags, out_grab_bb); if (r) *(KGS8*)p_v = (KGS8)v32;  return r; }
		case KGGuiDataType_U8: { KGU32 v32 = (KGU32) * (KGU8*)p_v;  bool r = SliderBehaviorT<KGU32, KGS32, float>(bb, id, KGGuiDataType_U32, &v32, *(const KGU8*)p_min, *(const KGU8*)p_max, format, flags, out_grab_bb); if (r) *(KGU8*)p_v = (KGU8)v32;  return r; }
		case KGGuiDataType_S16: { KGS32 v32 = (KGS32) * (KGS16*)p_v; bool r = SliderBehaviorT<KGS32, KGS32, float>(bb, id, KGGuiDataType_S32, &v32, *(const KGS16*)p_min, *(const KGS16*)p_max, format, flags, out_grab_bb); if (r) *(KGS16*)p_v = (KGS16)v32; return r; }
		case KGGuiDataType_U16: { KGU32 v32 = (KGU32) * (KGU16*)p_v; bool r = SliderBehaviorT<KGU32, KGS32, float>(bb, id, KGGuiDataType_U32, &v32, *(const KGU16*)p_min, *(const KGU16*)p_max, format, flags, out_grab_bb); if (r) *(KGU16*)p_v = (KGU16)v32; return r; }
		case KGGuiDataType_S32:
			KR_CORE_ASSERT(*(const KGS32*)p_min >= IM_S32_MIN / 2 && *(const KGS32*)p_max <= IM_S32_MAX / 2, "");
			return SliderBehaviorT<KGS32, KGS32, float >(bb, id, data_type, (KGS32*)p_v, *(const KGS32*)p_min, *(const KGS32*)p_max, format, flags, out_grab_bb);
		case KGGuiDataType_U32:
			KR_CORE_ASSERT(*(const KGU32*)p_max <= IM_U32_MAX / 2, "");
			return SliderBehaviorT<KGU32, KGS32, float >(bb, id, data_type, (KGU32*)p_v, *(const KGU32*)p_min, *(const KGU32*)p_max, format, flags, out_grab_bb);
		case KGGuiDataType_S64:
			KR_CORE_ASSERT(*(const KGS64*)p_min >= IM_S64_MIN / 2 && *(const KGS64*)p_max <= IM_S64_MAX / 2, "");
			return SliderBehaviorT<KGS64, KGS64, double>(bb, id, data_type, (KGS64*)p_v, *(const KGS64*)p_min, *(const KGS64*)p_max, format, flags, out_grab_bb);
		case KGGuiDataType_U64:
			KR_CORE_ASSERT(*(const KGU64*)p_max <= IM_U64_MAX / 2, "");
			return SliderBehaviorT<KGU64, KGS64, double>(bb, id, data_type, (KGU64*)p_v, *(const KGU64*)p_min, *(const KGU64*)p_max, format, flags, out_grab_bb);
		case KGGuiDataType_Float:
			KR_CORE_ASSERT(*(const float*)p_min >= -FLT_MAX / 2.0f && *(const float*)p_max <= FLT_MAX / 2.0f, "");
			return SliderBehaviorT<float, float, float >(bb, id, data_type, (float*)p_v, *(const float*)p_min, *(const float*)p_max, format, flags, out_grab_bb);
		case KGGuiDataType_Double:
			KR_CORE_ASSERT(*(const double*)p_min >= -DBL_MAX / 2.0f && *(const double*)p_max <= DBL_MAX / 2.0f, "");
			return SliderBehaviorT<double, double, double>(bb, id, data_type, (double*)p_v, *(const double*)p_min, *(const double*)p_max, format, flags, out_grab_bb);
		case KGGuiDataType_COUNT: break;
		}
		KR_CORE_ASSERT(0, "");
		return false;
	}

	// Note: p_data, p_min and p_max are _pointers_ to a memory address holding the data. For a slider, they are all required.
	// Read code of e.g. SliderFloat(), SliderInt() etc. or examples in 'Demo->Widgets->Data Types' to understand how to use this function directly.
	bool KarmaGui::SliderScalar(const char* label, KarmaGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, KarmaGuiSliderFlags flags)
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		KarmaGuiContext& g = *GKarmaGui;
		const KarmaGuiStyle& style = g.Style;
		const KGGuiID id = window->GetID(label);
		const float w = CalcItemWidth();

		const KGVec2 label_size = CalcTextSize(label, NULL, true);
		const KGRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + KGVec2(w, label_size.y + style.FramePadding.y * 2.0f));
		const KGRect total_bb(frame_bb.Min, frame_bb.Max + KGVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

		const bool temp_input_allowed = (flags & KGGuiSliderFlags_NoInput) == 0;
		KarmaGuiInternal::ItemSize(total_bb, style.FramePadding.y);
		if (!KarmaGuiInternal::ItemAdd(total_bb, id, &frame_bb, temp_input_allowed ? KGGuiItemFlags_Inputable : 0))
			return false;

		// Default format string when passing NULL
		if (format == NULL)
			format = KarmaGuiInternal::DataTypeGetInfo(data_type)->PrintFmt;

		const bool hovered = KarmaGuiInternal::ItemHoverable(frame_bb, id);
		bool temp_input_is_active = temp_input_allowed && KarmaGuiInternal::TempInputIsActive(id);
		if (!temp_input_is_active)
		{
			// Tabbing or CTRL-clicking on Slider turns it into an input box
			const bool input_requested_by_tabbing = temp_input_allowed && (g.LastItemData.StatusFlags & KGGuiItemStatusFlags_FocusedByTabbing) != 0;
			const bool clicked = hovered && IsMouseClicked(0, id);
			const bool make_active = (input_requested_by_tabbing || clicked || g.NavActivateId == id || g.NavActivateInputId == id);
			if (make_active && clicked)
				KarmaGuiInternal::SetKeyOwner(KGGuiKey_MouseLeft, id);
			if (make_active && temp_input_allowed)
				if (input_requested_by_tabbing || (clicked && g.IO.KeyCtrl) || g.NavActivateInputId == id)
					temp_input_is_active = true;

			if (make_active && !temp_input_is_active)
			{
				KarmaGuiInternal::SetActiveID(id, window);
				KarmaGuiInternal::SetFocusID(id, window);
				KarmaGuiInternal::FocusWindow(window);
				g.ActiveIdUsingNavDirMask |= (1 << KGGuiDir_Left) | (1 << KGGuiDir_Right);
			}
		}

		if (temp_input_is_active)
		{
			// Only clamp CTRL+Click input when KGGuiSliderFlags_AlwaysClamp is set
			const bool is_clamp_input = (flags & KGGuiSliderFlags_AlwaysClamp) != 0;
			return KarmaGuiInternal::TempInputScalar(frame_bb, id, label, data_type, p_data, format, is_clamp_input ? p_min : NULL, is_clamp_input ? p_max : NULL);
		}

		// Draw frame
		const KGU32 frame_col = GetColorU32(g.ActiveId == id ? KGGuiCol_FrameBgActive : hovered ? KGGuiCol_FrameBgHovered : KGGuiCol_FrameBg);
		KarmaGuiInternal::RenderNavHighlight(frame_bb, id);
		KarmaGuiInternal::RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, g.Style.FrameRounding);

		// Slider behavior
		KGRect grab_bb;
		const bool value_changed = KarmaGuiInternal::SliderBehavior(frame_bb, id, data_type, p_data, p_min, p_max, format, flags, &grab_bb);
		if (value_changed)
			KarmaGuiInternal::MarkItemEdited(id);

		// Render grab
		if (grab_bb.Max.x > grab_bb.Min.x)
			window->DrawList->AddRectFilled(grab_bb.Min, grab_bb.Max, GetColorU32(g.ActiveId == id ? KGGuiCol_SliderGrabActive : KGGuiCol_SliderGrab), style.GrabRounding);

		// Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
		char value_buf[64];
		const char* value_buf_end = value_buf + KarmaGuiInternal::DataTypeFormatString(value_buf, KG_ARRAYSIZE(value_buf), data_type, p_data, format);
		if (g.LogEnabled)
			KarmaGuiInternal::LogSetNextTextDecoration("{", "}");
		KarmaGuiInternal::RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL, KGVec2(0.5f, 0.5f));

		if (label_size.x > 0.0f)
			KarmaGuiInternal::RenderText(KGVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

		KARMAGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
		return value_changed;
	}

	// Add multiple sliders on 1 line for compact edition of multiple components
	bool KarmaGui::SliderScalarN(const char* label, KarmaGuiDataType data_type, void* v, int components, const void* v_min, const void* v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		KarmaGuiContext& g = *GKarmaGui;
		bool value_changed = false;
		BeginGroup();
		PushID(label);
		KarmaGuiInternal::PushMultiItemsWidths(components, CalcItemWidth());
		size_t type_size = GDataTypeInfo[data_type].Size;
		for (int i = 0; i < components; i++)
		{
			PushID(i);
			if (i > 0)
				SameLine(0, g.Style.ItemInnerSpacing.x);
			value_changed |= SliderScalar("", data_type, v, v_min, v_max, format, flags);
			PopID();
			PopItemWidth();
			v = (void*)((char*)v + type_size);
		}
		PopID();

		const char* label_end = KarmaGuiInternal::FindRenderedTextEnd(label);
		if (label != label_end)
		{
			SameLine(0, g.Style.ItemInnerSpacing.x);
			KarmaGuiInternal::TextEx(label, label_end);
		}

		EndGroup();
		return value_changed;
	}

	bool KarmaGui::SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		return SliderScalar(label, KGGuiDataType_Float, v, &v_min, &v_max, format, flags);
	}

	bool KarmaGui::SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		return SliderScalarN(label, KGGuiDataType_Float, v, 2, &v_min, &v_max, format, flags);
	}

	bool KarmaGui::SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		return SliderScalarN(label, KGGuiDataType_Float, v, 3, &v_min, &v_max, format, flags);
	}

	bool KarmaGui::SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		return SliderScalarN(label, KGGuiDataType_Float, v, 4, &v_min, &v_max, format, flags);
	}

	bool KarmaGui::SliderAngle(const char* label, float* v_rad, float v_degrees_min, float v_degrees_max, const char* format, KarmaGuiSliderFlags flags)
	{
		if (format == NULL)
			format = "%.0f deg";
		float v_deg = (*v_rad) * 360.0f / (2 * KG_PI);
		bool value_changed = SliderFloat(label, &v_deg, v_degrees_min, v_degrees_max, format, flags);
		*v_rad = v_deg * (2 * KG_PI) / 360.0f;
		return value_changed;
	}

	bool KarmaGui::SliderInt(const char* label, int* v, int v_min, int v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		return SliderScalar(label, KGGuiDataType_S32, v, &v_min, &v_max, format, flags);
	}

	bool KarmaGui::SliderInt2(const char* label, int v[2], int v_min, int v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		return SliderScalarN(label, KGGuiDataType_S32, v, 2, &v_min, &v_max, format, flags);
	}

	bool KarmaGui::SliderInt3(const char* label, int v[3], int v_min, int v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		return SliderScalarN(label, KGGuiDataType_S32, v, 3, &v_min, &v_max, format, flags);
	}

	bool KarmaGui::SliderInt4(const char* label, int v[4], int v_min, int v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		return SliderScalarN(label, KGGuiDataType_S32, v, 4, &v_min, &v_max, format, flags);
	}

	bool KarmaGui::VSliderScalar(const char* label, const KGVec2& size, KarmaGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, KarmaGuiSliderFlags flags)
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		KarmaGuiContext& g = *GKarmaGui;
		const KarmaGuiStyle& style = g.Style;
		const KGGuiID id = window->GetID(label);

		const KGVec2 label_size = CalcTextSize(label, NULL, true);
		const KGRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + size);
		const KGRect bb(frame_bb.Min, frame_bb.Max + KGVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

		KarmaGuiInternal::ItemSize(bb, style.FramePadding.y);
		if (!KarmaGuiInternal::ItemAdd(frame_bb, id))
			return false;

		// Default format string when passing NULL
		if (format == NULL)
			format = KarmaGuiInternal::DataTypeGetInfo(data_type)->PrintFmt;

		const bool hovered = KarmaGuiInternal::ItemHoverable(frame_bb, id);
		const bool clicked = hovered && IsMouseClicked(0, id);
		if (clicked || g.NavActivateId == id || g.NavActivateInputId == id)
		{
			if (clicked)
				KarmaGuiInternal::SetKeyOwner(KGGuiKey_MouseLeft, id);
			KarmaGuiInternal::SetActiveID(id, window);
			KarmaGuiInternal::SetFocusID(id, window);
			KarmaGuiInternal::FocusWindow(window);
			g.ActiveIdUsingNavDirMask |= (1 << KGGuiDir_Up) | (1 << KGGuiDir_Down);
		}

		// Draw frame
		const KGU32 frame_col = GetColorU32(g.ActiveId == id ? KGGuiCol_FrameBgActive : hovered ? KGGuiCol_FrameBgHovered : KGGuiCol_FrameBg);
		KarmaGuiInternal::RenderNavHighlight(frame_bb, id);
		KarmaGuiInternal::RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, g.Style.FrameRounding);

		// Slider behavior
		KGRect grab_bb;
		const bool value_changed = KarmaGuiInternal::SliderBehavior(frame_bb, id, data_type, p_data, p_min, p_max, format, flags | KGGuiSliderFlags_Vertical, &grab_bb);
		if (value_changed)
			KarmaGuiInternal::MarkItemEdited(id);

		// Render grab
		if (grab_bb.Max.y > grab_bb.Min.y)
			window->DrawList->AddRectFilled(grab_bb.Min, grab_bb.Max, GetColorU32(g.ActiveId == id ? KGGuiCol_SliderGrabActive : KGGuiCol_SliderGrab), style.GrabRounding);

		// Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
		// For the vertical slider we allow centered text to overlap the frame padding
		char value_buf[64];
		const char* value_buf_end = value_buf + KarmaGuiInternal::DataTypeFormatString(value_buf, KG_ARRAYSIZE(value_buf), data_type, p_data, format);
		KarmaGuiInternal::RenderTextClipped(KGVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, value_buf, value_buf_end, NULL, KGVec2(0.5f, 0.0f));
		if (label_size.x > 0.0f)
			KarmaGuiInternal::RenderText(KGVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

		return value_changed;
	}

	bool KarmaGui::VSliderFloat(const char* label, const KGVec2& size, float* v, float v_min, float v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		return VSliderScalar(label, size, KGGuiDataType_Float, v, &v_min, &v_max, format, flags);
	}

	bool KarmaGui::VSliderInt(const char* label, const KGVec2& size, int* v, int v_min, int v_max, const char* format, KarmaGuiSliderFlags flags)
	{
		return VSliderScalar(label, size, KGGuiDataType_S32, v, &v_min, &v_max, format, flags);
	}

	//-------------------------------------------------------------------------
	// [SECTION] Widgets: InputScalar, InputFloat, InputInt, etc.
	//-------------------------------------------------------------------------
	// - KGParseFormatFindStart() [Internal]
	// - KGParseFormatFindEnd() [Internal]
	// - KGParseFormatTrimDecorations() [Internal]
	// - KGParseFormatSanitizeForPrinting() [Internal]
	// - KGParseFormatSanitizeForScanning() [Internal]
	// - KGParseFormatPrecision() [Internal]
	// - TempInputTextScalar() [Internal]
	// - InputScalar()
	// - InputScalarN()
	// - InputFloat()
	// - InputFloat2()
	// - InputFloat3()
	// - InputFloat4()
	// - InputInt()
	// - InputInt2()
	// - InputInt3()
	// - InputInt4()
	// - InputDouble()
	//-------------------------------------------------------------------------

	// We don't use strchr() because our strings are usually very short and often start with '%'
	const char* KGParseFormatFindStart(const char* fmt)
	{
		while (char c = fmt[0])
		{
			if (c == '%' && fmt[1] != '%')
				return fmt;
			else if (c == '%')
				fmt++;
			fmt++;
		}
		return fmt;
	}

	const char* KGParseFormatFindEnd(const char* fmt)
	{
		// Printf/scanf types modifiers: I/L/h/j/l/t/w/z. Other uppercase letters qualify as types aka end of the format.
		if (fmt[0] != '%')
			return fmt;
		const unsigned int ignored_uppercase_mask = (1 << ('I' - 'A')) | (1 << ('L' - 'A'));
		const unsigned int ignored_lowercase_mask = (1 << ('h' - 'a')) | (1 << ('j' - 'a')) | (1 << ('l' - 'a')) | (1 << ('t' - 'a')) | (1 << ('w' - 'a')) | (1 << ('z' - 'a'));
		for (char c; (c = *fmt) != 0; fmt++)
		{
			if (c >= 'A' && c <= 'Z' && ((1 << (c - 'A')) & ignored_uppercase_mask) == 0)
				return fmt + 1;
			if (c >= 'a' && c <= 'z' && ((1 << (c - 'a')) & ignored_lowercase_mask) == 0)
				return fmt + 1;
		}
		return fmt;
	}

	// Extract the format out of a format string with leading or trailing decorations
	//  fmt = "blah blah"  -> return fmt
	//  fmt = "%.3f"       -> return fmt
	//  fmt = "hello %.3f" -> return fmt + 6
	//  fmt = "%.3f hello" -> return buf written with "%.3f"
	const char* KGParseFormatTrimDecorations(const char* fmt, char* buf, size_t buf_size)
	{
		const char* fmt_start = KGParseFormatFindStart(fmt);
		if (fmt_start[0] != '%')
			return fmt;
		const char* fmt_end = KGParseFormatFindEnd(fmt_start);
		if (fmt_end[0] == 0) // If we only have leading decoration, we don't need to copy the data.
			return fmt_start;
		KGStrncpy(buf, fmt_start, KGMin((size_t)(fmt_end - fmt_start) + 1, buf_size));
		return buf;
	}

	// Sanitize format
	// - Zero terminate so extra characters after format (e.g. "%f123") don't confuse atof/atoi
	// - stb_sprintf.h supports several new modifiers which format numbers in a way that also makes them incompatible atof/atoi.
	void KGParseFormatSanitizeForPrinting(const char* fmt_in, char* fmt_out, size_t fmt_out_size)
	{
		const char* fmt_end = KGParseFormatFindEnd(fmt_in);
		KG_UNUSED(fmt_out_size);
		KR_CORE_ASSERT((size_t)(fmt_end - fmt_in + 1) < fmt_out_size, ""); // Format is too long, let us know if this happens to you!
		while (fmt_in < fmt_end)
		{
			char c = *fmt_in++;
			if (c != '\'' && c != '$' && c != '_') // Custom flags provided by stb_sprintf.h. POSIX 2008 also supports '.
				*(fmt_out++) = c;
		}
		*fmt_out = 0; // Zero-terminate
	}

	// - For scanning we need to remove all width and precision fields "%3.7f" -> "%f". BUT don't strip types like "%I64d" which includes digits. ! "%07I64d" -> "%I64d"
	const char* KGParseFormatSanitizeForScanning(const char* fmt_in, char* fmt_out, size_t fmt_out_size)
	{
		const char* fmt_end = KGParseFormatFindEnd(fmt_in);
		const char* fmt_out_begin = fmt_out;
		KG_UNUSED(fmt_out_size);
		KR_CORE_ASSERT((size_t)(fmt_end - fmt_in + 1) < fmt_out_size, ""); // Format is too long, let us know if this happens to you!
		bool has_type = false;
		while (fmt_in < fmt_end)
		{
			char c = *fmt_in++;
			if (!has_type && ((c >= '0' && c <= '9') || c == '.'))
				continue;
			has_type |= ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')); // Stop skipping digits
			if (c != '\'' && c != '$' && c != '_') // Custom flags provided by stb_sprintf.h. POSIX 2008 also supports '.
				*(fmt_out++) = c;
		}
		*fmt_out = 0; // Zero-terminate
		return fmt_out_begin;
	}

	template<typename TYPE>
	static const char* ImAtoi(const char* src, TYPE* output)
	{
		int negative = 0;
		if (*src == '-') { negative = 1; src++; }
		if (*src == '+') { src++; }
		TYPE v = 0;
		while (*src >= '0' && *src <= '9')
			v = (v * 10) + (*src++ - '0');
		*output = negative ? -v : v;
		return src;
	}

	// Parse display precision back from the display format string
	// FIXME: This is still used by some navigation code path to infer a minimum tweak step, but we should aim to rework widgets so it isn't needed.
	int KGParseFormatPrecision(const char* fmt, int default_precision)
	{
		fmt = KGParseFormatFindStart(fmt);
		if (fmt[0] != '%')
			return default_precision;
		fmt++;
		while (*fmt >= '0' && *fmt <= '9')
			fmt++;
		int precision = INT_MAX;
		if (*fmt == '.')
		{
			fmt = ImAtoi<int>(fmt + 1, &precision);
			if (precision < 0 || precision > 99)
				precision = default_precision;
		}
		if (*fmt == 'e' || *fmt == 'E') // Maximum precision with scientific notation
			precision = -1;
		if ((*fmt == 'g' || *fmt == 'G') && precision == INT_MAX)
			precision = -1;
		return (precision == INT_MAX) ? default_precision : precision;
	}

	// Create text input in place of another active widget (e.g. used when doing a CTRL+Click on drag/slider widgets)
	// FIXME: Facilitate using this in variety of other situations.
	bool KarmaGuiInternal::TempInputText(const KGRect& bb, KGGuiID id, const char* label, char* buf, int buf_size, KarmaGuiInputTextFlags flags)
	{
		// On the first frame, g.TempInputTextId == 0, then on subsequent frames it becomes == id.
		// We clear ActiveID on the first frame to allow the InputText() taking it back.
		KarmaGuiContext& g = *GKarmaGui;
		const bool init = (g.TempInputId != id);
		if (init)
			ClearActiveID();

		g.CurrentWindow->DC.CursorPos = bb.Min;
		bool value_changed = InputTextEx(label, NULL, buf, buf_size, bb.GetSize(), flags | KGGuiInputTextFlags_MergedItem);
		if (init)
		{
			// First frame we started displaying the InputText widget, we expect it to take the active id.
			KR_CORE_ASSERT(g.ActiveId == id, "");
			g.TempInputId = g.ActiveId;
		}
		return value_changed;
	}

	static inline KarmaGuiInputTextFlags InputScalar_DefaultCharsFilter(KarmaGuiDataType data_type, const char* format)
	{
		if (data_type == KGGuiDataType_Float || data_type == KGGuiDataType_Double)
			return KGGuiInputTextFlags_CharsScientific;
		const char format_last_char = format[0] ? format[strlen(format) - 1] : 0;
		return (format_last_char == 'x' || format_last_char == 'X') ? KGGuiInputTextFlags_CharsHexadecimal : KGGuiInputTextFlags_CharsDecimal;
	}

	// Note that Drag/Slider functions are only forwarding the min/max values clamping values if the KGGuiSliderFlags_AlwaysClamp flag is set!
	// This is intended: this way we allow CTRL+Click manual input to set a value out of bounds, for maximum flexibility.
	// However this may not be ideal for all uses, as some user code may break on out of bound values.
	bool KarmaGuiInternal::TempInputScalar(const KGRect& bb, KGGuiID id, const char* label, KarmaGuiDataType data_type, void* p_data, const char* format, const void* p_clamp_min, const void* p_clamp_max)
	{
		char fmt_buf[32];
		char data_buf[32];
		format = KGParseFormatTrimDecorations(format, fmt_buf, KG_ARRAYSIZE(fmt_buf));
		DataTypeFormatString(data_buf, KG_ARRAYSIZE(data_buf), data_type, p_data, format);
		KGStrTrimBlanks(data_buf);

		KarmaGuiInputTextFlags flags = KGGuiInputTextFlags_AutoSelectAll | KGGuiInputTextFlags_NoMarkEdited;
		flags |= InputScalar_DefaultCharsFilter(data_type, format);

		bool value_changed = false;
		if (TempInputText(bb, id, label, data_buf, KG_ARRAYSIZE(data_buf), flags))
		{
			// Backup old value
			size_t data_type_size = DataTypeGetInfo(data_type)->Size;
			KGGuiDataTypeTempStorage data_backup;
			memcpy(&data_backup, p_data, data_type_size);

			// Apply new value (or operations) then clamp
			DataTypeApplyFromText(data_buf, data_type, p_data, format);
			if (p_clamp_min || p_clamp_max)
			{
				if (p_clamp_min && p_clamp_max && DataTypeCompare(data_type, p_clamp_min, p_clamp_max) > 0)
					KGSwap(p_clamp_min, p_clamp_max);
				DataTypeClamp(data_type, p_data, p_clamp_min, p_clamp_max);
			}

			// Only mark as edited if new value is different
			value_changed = memcmp(&data_backup, p_data, data_type_size) != 0;
			if (value_changed)
				MarkItemEdited(id);
		}
		return value_changed;
	}

	// Note: p_data, p_step, p_step_fast are _pointers_ to a memory address holding the data. For an Input widget, p_step and p_step_fast are optional.
	// Read code of e.g. InputFloat(), InputInt() etc. or examples in 'Demo->Widgets->Data Types' to understand how to use this function directly.
	bool KarmaGui::InputScalar(const char* label, KarmaGuiDataType data_type, void* p_data, const void* p_step, const void* p_step_fast, const char* format, KarmaGuiInputTextFlags flags)
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		KarmaGuiContext& g = *GKarmaGui;
		KarmaGuiStyle& style = g.Style;

		if (format == NULL)
			format = KarmaGuiInternal::DataTypeGetInfo(data_type)->PrintFmt;

		char buf[64];
		KarmaGuiInternal::DataTypeFormatString(buf, KG_ARRAYSIZE(buf), data_type, p_data, format);

		// Testing ActiveId as a minor optimization as filtering is not needed until active
		if (g.ActiveId == 0 && (flags & (KGGuiInputTextFlags_CharsDecimal | KGGuiInputTextFlags_CharsHexadecimal | KGGuiInputTextFlags_CharsScientific)) == 0)
			flags |= InputScalar_DefaultCharsFilter(data_type, format);
		flags |= KGGuiInputTextFlags_AutoSelectAll | KGGuiInputTextFlags_NoMarkEdited; // We call MarkItemEdited() ourselves by comparing the actual data rather than the string.

		bool value_changed = false;
		if (p_step != NULL)
		{
			const float button_size = GetFrameHeight();

			BeginGroup(); // The only purpose of the group here is to allow the caller to query item data e.g. IsItemActive()
			PushID(label);
			SetNextItemWidth(KGMax(1.0f, CalcItemWidth() - (button_size + style.ItemInnerSpacing.x) * 2));
			if (InputText("", buf, KG_ARRAYSIZE(buf), flags)) // PushId(label) + "" gives us the expected ID from outside point of view
				value_changed = KarmaGuiInternal::DataTypeApplyFromText(buf, data_type, p_data, format);
			KARMAGUI_TEST_ENGINE_ITEM_INFO(g.LastItemData.ID, label, g.LastItemData.StatusFlags);

			// Step buttons
			const KGVec2 backup_frame_padding = style.FramePadding;
			style.FramePadding.x = style.FramePadding.y;
			KarmaGuiButtonFlags button_flags = KGGuiButtonFlags_Repeat | KGGuiButtonFlags_DontClosePopups;
			if (flags & KGGuiInputTextFlags_ReadOnly)
				BeginDisabled();
			SameLine(0, style.ItemInnerSpacing.x);
			if (KarmaGuiInternal::ButtonEx("-", KGVec2(button_size, button_size), button_flags))
			{
				KarmaGuiInternal::DataTypeApplyOp(data_type, '-', p_data, p_data, g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
				value_changed = true;
			}
			SameLine(0, style.ItemInnerSpacing.x);
			if (KarmaGuiInternal::ButtonEx("+", KGVec2(button_size, button_size), button_flags))
			{
				KarmaGuiInternal::DataTypeApplyOp(data_type, '+', p_data, p_data, g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
				value_changed = true;
			}
			if (flags & KGGuiInputTextFlags_ReadOnly)
				EndDisabled();

			const char* label_end = KarmaGuiInternal::FindRenderedTextEnd(label);
			if (label != label_end)
			{
				SameLine(0, style.ItemInnerSpacing.x);
				KarmaGuiInternal::TextEx(label, label_end);
			}
			style.FramePadding = backup_frame_padding;

			PopID();
			EndGroup();
		}
		else
		{
			if (InputText(label, buf, KG_ARRAYSIZE(buf), flags))
				value_changed = KarmaGuiInternal::DataTypeApplyFromText(buf, data_type, p_data, format);
		}
		if (value_changed)
			KarmaGuiInternal::MarkItemEdited(g.LastItemData.ID);

		return value_changed;
	}

	bool KarmaGui::InputScalarN(const char* label, KarmaGuiDataType data_type, void* p_data, int components, const void* p_step, const void* p_step_fast, const char* format, KarmaGuiInputTextFlags flags)
	{
		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		KarmaGuiContext& g = *GKarmaGui;
		bool value_changed = false;
		BeginGroup();
		PushID(label);
		KarmaGuiInternal::PushMultiItemsWidths(components, CalcItemWidth());
		size_t type_size = GDataTypeInfo[data_type].Size;
		for (int i = 0; i < components; i++)
		{
			PushID(i);
			if (i > 0)
				SameLine(0, g.Style.ItemInnerSpacing.x);
			value_changed |= InputScalar("", data_type, p_data, p_step, p_step_fast, format, flags);
			PopID();
			PopItemWidth();
			p_data = (void*)((char*)p_data + type_size);
		}
		PopID();

		const char* label_end = KarmaGuiInternal::FindRenderedTextEnd(label);
		if (label != label_end)
		{
			SameLine(0.0f, g.Style.ItemInnerSpacing.x);
			KarmaGuiInternal::TextEx(label, label_end);
		}

		EndGroup();
		return value_changed;
	}

	bool KarmaGui::InputFloat(const char* label, float* v, float step, float step_fast, const char* format, KarmaGuiInputTextFlags flags)
	{
		flags |= KGGuiInputTextFlags_CharsScientific;
		return InputScalar(label, KGGuiDataType_Float, (void*)v, (void*)(step > 0.0f ? &step : NULL), (void*)(step_fast > 0.0f ? &step_fast : NULL), format, flags);
	}

	bool KarmaGui::InputFloat2(const char* label, float v[2], const char* format, KarmaGuiInputTextFlags flags)
	{
		return InputScalarN(label, KGGuiDataType_Float, v, 2, NULL, NULL, format, flags);
	}

	bool KarmaGui::InputFloat3(const char* label, float v[3], const char* format, KarmaGuiInputTextFlags flags)
	{
		return InputScalarN(label, KGGuiDataType_Float, v, 3, NULL, NULL, format, flags);
	}

	bool KarmaGui::InputFloat4(const char* label, float v[4], const char* format, KarmaGuiInputTextFlags flags)
	{
		return InputScalarN(label, KGGuiDataType_Float, v, 4, NULL, NULL, format, flags);
	}

	bool KarmaGui::InputInt(const char* label, int* v, int step, int step_fast, KarmaGuiInputTextFlags flags)
	{
		// Hexadecimal input provided as a convenience but the flag name is awkward. Typically you'd use InputText() to parse your own data, if you want to handle prefixes.
		const char* format = (flags & KGGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%d";
		return InputScalar(label, KGGuiDataType_S32, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
	}

	bool KarmaGui::InputInt2(const char* label, int v[2], KarmaGuiInputTextFlags flags)
	{
		return InputScalarN(label, KGGuiDataType_S32, v, 2, NULL, NULL, "%d", flags);
	}

	bool KarmaGui::InputInt3(const char* label, int v[3], KarmaGuiInputTextFlags flags)
	{
		return InputScalarN(label, KGGuiDataType_S32, v, 3, NULL, NULL, "%d", flags);
	}

	bool KarmaGui::InputInt4(const char* label, int v[4], KarmaGuiInputTextFlags flags)
	{
		return InputScalarN(label, KGGuiDataType_S32, v, 4, NULL, NULL, "%d", flags);
	}

	bool KarmaGui::InputDouble(const char* label, double* v, double step, double step_fast, const char* format, KarmaGuiInputTextFlags flags)
	{
		flags |= KGGuiInputTextFlags_CharsScientific;
		return InputScalar(label, KGGuiDataType_Double, (void*)v, (void*)(step > 0.0 ? &step : NULL), (void*)(step_fast > 0.0 ? &step_fast : NULL), format, flags);
	}

	//-------------------------------------------------------------------------
	// [SECTION] Widgets: InputText, InputTextMultiline, InputTextWithHint
	//-------------------------------------------------------------------------
	// - InputText()
	// - InputTextWithHint()
	// - InputTextMultiline()
	// - InputTextGetCharInfo() [Internal]
	// - InputTextReindexLines() [Internal]
	// - InputTextReindexLinesRange() [Internal]
	// - InputTextEx() [Internal]
	// - DebugNodeInputTextState() [Internal]
	//-------------------------------------------------------------------------

	bool KarmaGui::InputText(const char* label, char* buf, size_t buf_size, KarmaGuiInputTextFlags flags, KarmaGuiInputTextCallback callback, void* user_data)
	{
		KR_CORE_ASSERT(!(flags & KGGuiInputTextFlags_Multiline), ""); // call InputTextMultiline()
		return KarmaGuiInternal::InputTextEx(label, NULL, buf, (int)buf_size, KGVec2(0, 0), flags, callback, user_data);
	}

	bool KarmaGui::InputTextMultiline(const char* label, char* buf, size_t buf_size, const KGVec2& size, KarmaGuiInputTextFlags flags, KarmaGuiInputTextCallback callback, void* user_data)
	{
		return KarmaGuiInternal::InputTextEx(label, NULL, buf, (int)buf_size, size, flags | KGGuiInputTextFlags_Multiline, callback, user_data);
	}

	bool KarmaGui::InputTextWithHint(const char* label, const char* hint, char* buf, size_t buf_size, KarmaGuiInputTextFlags flags, KarmaGuiInputTextCallback callback, void* user_data)
	{
		KR_CORE_ASSERT(!(flags & KGGuiInputTextFlags_Multiline), ""); // call InputTextMultiline() or  InputTextEx() manually if you need multi-line + hint.
		return KarmaGuiInternal::InputTextEx(label, hint, buf, (int)buf_size, KGVec2(0, 0), flags, callback, user_data);
	}

	static int InputTextCalcTextLenAndLineCount(const char* text_begin, const char** out_text_end)
	{
		int line_count = 0;
		const char* s = text_begin;
		while (char c = *s++) // We are only matching for \n so we can ignore UTF-8 decoding
			if (c == '\n')
				line_count++;
		s--;
		if (s[0] != '\n' && s[0] != '\r')
			line_count++;
		*out_text_end = s;
		return line_count;
	}

	static KGVec2 InputTextCalcTextSizeW(KarmaGuiContext* ctx, const KGWchar* text_begin, const KGWchar* text_end, const KGWchar** remaining, KGVec2* out_offset, bool stop_on_new_line)
	{
		KarmaGuiContext& g = *ctx;
		KGFont* font = g.Font;
		const float line_height = g.FontSize;
		const float scale = line_height / font->FontSize;

		KGVec2 text_size = KGVec2(0, 0);
		float line_width = 0.0f;

		const KGWchar* s = text_begin;
		while (s < text_end)
		{
			unsigned int c = (unsigned int)(*s++);
			if (c == '\n')
			{
				text_size.x = KGMax(text_size.x, line_width);
				text_size.y += line_height;
				line_width = 0.0f;
				if (stop_on_new_line)
					break;
				continue;
			}
			if (c == '\r')
				continue;

			const float char_width = font->GetCharAdvance((KGWchar)c) * scale;
			line_width += char_width;
		}

		if (text_size.x < line_width)
			text_size.x = line_width;

		if (out_offset)
			*out_offset = KGVec2(line_width, text_size.y + line_height);  // offset allow for the possibility of sitting after a trailing \n

		if (line_width > 0 || text_size.y == 0.0f)                        // whereas size.y will ignore the trailing \n
			text_size.y += line_height;

		if (remaining)
			*remaining = s;

		return text_size;
	}
}

// Wrapper for stb_textedit.h to edit text (our wrapper is for: statically sized buffer, single-line, wchar characters. InputText converts between UTF-8 and wchar)
namespace KGStb
{
	// We don't use an enum so we can build even with conflicting symbols (if another user of stb_textedit.h leak their STB_TEXTEDIT_K_* symbols)
#define STB_TEXTEDIT_K_LEFT         0x200000 // keyboard input to move cursor left
#define STB_TEXTEDIT_K_RIGHT        0x200001 // keyboard input to move cursor right
#define STB_TEXTEDIT_K_UP           0x200002 // keyboard input to move cursor up
#define STB_TEXTEDIT_K_DOWN         0x200003 // keyboard input to move cursor down
#define STB_TEXTEDIT_K_LINESTART    0x200004 // keyboard input to move cursor to start of line
#define STB_TEXTEDIT_K_LINEEND      0x200005 // keyboard input to move cursor to end of line
#define STB_TEXTEDIT_K_TEXTSTART    0x200006 // keyboard input to move cursor to start of text
#define STB_TEXTEDIT_K_TEXTEND      0x200007 // keyboard input to move cursor to end of text
#define STB_TEXTEDIT_K_DELETE       0x200008 // keyboard input to delete selection or character under cursor
#define STB_TEXTEDIT_K_BACKSPACE    0x200009 // keyboard input to delete selection or character left of cursor
#define STB_TEXTEDIT_K_UNDO         0x20000A // keyboard input to perform undo
#define STB_TEXTEDIT_K_REDO         0x20000B // keyboard input to perform redo
#define STB_TEXTEDIT_K_WORDLEFT     0x20000C // keyboard input to move cursor left one word
#define STB_TEXTEDIT_K_WORDRIGHT    0x20000D // keyboard input to move cursor right one word
#define STB_TEXTEDIT_K_PGUP         0x20000E // keyboard input to move cursor up a page
#define STB_TEXTEDIT_K_PGDOWN       0x20000F // keyboard input to move cursor down a page
#define STB_TEXTEDIT_K_SHIFT        0x400000

#ifndef STB_TEXTEDIT_KEYTYPE
#define STB_TEXTEDIT_KEYTYPE int
#endif

	static int     STB_TEXTEDIT_STRINGLEN(const KGGuiInputTextState* obj) { return obj->CurLenW; }
	static KGWchar STB_TEXTEDIT_GETCHAR(const KGGuiInputTextState* obj, int idx) { return obj->TextW[idx]; }
	static float   STB_TEXTEDIT_GETWIDTH(KGGuiInputTextState* obj, int line_start_idx, int char_idx) { KGWchar c = obj->TextW[line_start_idx + char_idx]; if (c == '\n') return STB_TEXTEDIT_GETWIDTH_NEWLINE; KarmaGuiContext& g = *obj->Ctx; return g.Font->GetCharAdvance(c) * (g.FontSize / g.Font->FontSize); }
	static int     STB_TEXTEDIT_KEYTOTEXT(int key) { return key >= 0x200000 ? 0 : key; }
	static void stb_textedit_key(STB_TEXTEDIT_STRING* str, STB_TexteditState* state, STB_TEXTEDIT_KEYTYPE key);
	static STB_TEXTEDIT_CHARTYPE* stb_text_createundo(StbUndoState* state, int pos, int insert_len, int delete_len);
	static void stb_textedit_initialize_state(STB_TexteditState* state, int is_single_line);
	static void stb_textedit_click(STB_TEXTEDIT_STRING* str, STB_TexteditState* state, float x, float y);
	static void stb_textedit_prep_selection_at_cursor(STB_TexteditState* state);
	static void stb_textedit_clamp(STB_TEXTEDIT_STRING* str, STB_TexteditState* state);
	static void stb_textedit_drag(STB_TEXTEDIT_STRING* str, STB_TexteditState* state, float x, float y);
	static int stb_textedit_paste_internal(STB_TEXTEDIT_STRING* str, STB_TexteditState* state, STB_TEXTEDIT_CHARTYPE* text, int len);
	static int stb_textedit_paste(STB_TEXTEDIT_STRING* str, STB_TexteditState* state, STB_TEXTEDIT_CHARTYPE const* ctext, int len);

	static KGWchar STB_TEXTEDIT_NEWLINE = '\n';
	static void    STB_TEXTEDIT_LAYOUTROW(StbTexteditRow* r, KGGuiInputTextState* obj, int line_start_idx)
	{
		const KGWchar* text = obj->TextW.Data;
		const KGWchar* text_remaining = NULL;
		const KGVec2 size = Karma::InputTextCalcTextSizeW(obj->Ctx, text + line_start_idx, text + obj->CurLenW, &text_remaining, NULL, true);
		r->x0 = 0.0f;
		r->x1 = size.x;
		r->baseline_y_delta = size.y;
		r->ymin = 0.0f;
		r->ymax = size.y;
		r->num_chars = (int)(text_remaining - (text + line_start_idx));
	}

	// When KGGuiInputTextFlags_Password is set, we don't want actions such as CTRL+Arrow to leak the fact that underlying data are blanks or separators.
	static bool is_separator(unsigned int c) { return KGCharIsBlankW(c) || c == ',' || c == ';' || c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' || c == '|' || c == '\n' || c == '\r'; }
	static int  is_word_boundary_from_right(KGGuiInputTextState* obj, int idx) { if (obj->Flags & KGGuiInputTextFlags_Password) return 0; return idx > 0 ? (is_separator(obj->TextW[idx - 1]) && !is_separator(obj->TextW[idx])) : 1; }
	static int  is_word_boundary_from_left(KGGuiInputTextState* obj, int idx) { if (obj->Flags & KGGuiInputTextFlags_Password) return 0; return idx > 0 ? (!is_separator(obj->TextW[idx - 1]) && is_separator(obj->TextW[idx])) : 1; }
	static int  STB_TEXTEDIT_MOVEWORDLEFT_IMPL(KGGuiInputTextState* obj, int idx) { idx--; while (idx >= 0 && !is_word_boundary_from_right(obj, idx)) idx--; return idx < 0 ? 0 : idx; }
	static int  STB_TEXTEDIT_MOVEWORDRIGHT_MAC(KGGuiInputTextState* obj, int idx) { idx++; int len = obj->CurLenW; while (idx < len && !is_word_boundary_from_left(obj, idx)) idx++; return idx > len ? len : idx; }
	static int  STB_TEXTEDIT_MOVEWORDRIGHT_WIN(KGGuiInputTextState* obj, int idx) { idx++; int len = obj->CurLenW; while (idx < len && !is_word_boundary_from_right(obj, idx)) idx++; return idx > len ? len : idx; }
	static int  STB_TEXTEDIT_MOVEWORDRIGHT_IMPL(KGGuiInputTextState* obj, int idx) { KarmaGuiContext& g = *obj->Ctx; if (g.IO.ConfigMacOSXBehaviors) return STB_TEXTEDIT_MOVEWORDRIGHT_MAC(obj, idx); else return STB_TEXTEDIT_MOVEWORDRIGHT_WIN(obj, idx); }
#define STB_TEXTEDIT_MOVEWORDLEFT   STB_TEXTEDIT_MOVEWORDLEFT_IMPL  // They need to be #define for stb_textedit.h
#define STB_TEXTEDIT_MOVEWORDRIGHT  STB_TEXTEDIT_MOVEWORDRIGHT_IMPL

	static void STB_TEXTEDIT_DELETECHARS(KGGuiInputTextState* obj, int pos, int n)
	{
		KGWchar* dst = obj->TextW.Data + pos;

		// We maintain our buffer length in both UTF-8 and wchar formats
		obj->Edited = true;
		obj->CurLenA -= KGTextCountUtf8BytesFromStr(dst, dst + n);
		obj->CurLenW -= n;

		// Offset remaining text (FIXME-OPT: Use memmove)
		const KGWchar* src = obj->TextW.Data + pos + n;
		while (KGWchar c = *src++)
			*dst++ = c;
		*dst = '\0';
	}

	static bool STB_TEXTEDIT_INSERTCHARS(KGGuiInputTextState* obj, int pos, const KGWchar* new_text, int new_text_len)
	{
		const bool is_resizable = (obj->Flags & KGGuiInputTextFlags_CallbackResize) != 0;
		const int text_len = obj->CurLenW;
		KR_CORE_ASSERT(pos <= text_len, "");

		const int new_text_len_utf8 = KGTextCountUtf8BytesFromStr(new_text, new_text + new_text_len);
		if (!is_resizable && (new_text_len_utf8 + obj->CurLenA + 1 > obj->BufCapacityA))
			return false;

		// Grow internal buffer if needed
		if (new_text_len + text_len + 1 > obj->TextW.Size)
		{
			if (!is_resizable)
				return false;
			KR_CORE_ASSERT(text_len < obj->TextW.Size, "");
			obj->TextW.resize(text_len + KGClamp(new_text_len * 4, 32, KGMax(256, new_text_len)) + 1);
		}

		KGWchar* text = obj->TextW.Data;
		if (pos != text_len)
			memmove(text + pos + new_text_len, text + pos, (size_t)(text_len - pos) * sizeof(KGWchar));
		memcpy(text + pos, new_text, (size_t)new_text_len * sizeof(KGWchar));

		obj->Edited = true;
		obj->CurLenW += new_text_len;
		obj->CurLenA += new_text_len_utf8;
		obj->TextW[obj->CurLenW] = '\0';

		return true;
	}

#define STB_TEXTEDIT_IMPLEMENTATION
#include "KarmaSTBTextEdit.h"

	static void stb_text_makeundo_replace(STB_TEXTEDIT_STRING* str, ::KGStb::STB_TexteditState* state, int where, int old_length, int new_length);
	// stb_textedit internally allows for a single undo record to do addition and deletion, but somehow, calling
	// the stb_textedit_paste() function creates two separate records, so we perform it manually. (FIXME: Report to nothings/stb?)
	static void stb_textedit_replace(KGGuiInputTextState* str, ::KGStb::STB_TexteditState* state, const STB_TEXTEDIT_CHARTYPE* text, int text_len)
	{
		stb_text_makeundo_replace(str, state, 0, str->CurLenW, text_len);
		KGStb::STB_TEXTEDIT_DELETECHARS(str, 0, str->CurLenW);
		state->cursor = state->select_start = state->select_end = 0;
		if (text_len <= 0)
			return;
		if (KGStb::STB_TEXTEDIT_INSERTCHARS(str, 0, text, text_len))
		{
			state->cursor = state->select_start = state->select_end = text_len;
			state->has_preferred_x = 0;
			return;
		}
		KR_CORE_ASSERT(0, ""); // Failed to insert character, normally shouldn't happen because of how we currently use stb_textedit_replace()
	}
} // namespace KGStb

void KGGuiInputTextState::OnKeyPressed(int key)
{
	stb_textedit_key(this, &Stb, key);
	CursorFollow = true;
	CursorAnimReset();
}

KarmaGuiInputTextCallbackData::KarmaGuiInputTextCallbackData()
{
	memset(this, 0, sizeof(*this));
}

// Public API to manipulate UTF-8 text
// We expose UTF-8 to the user (unlike the STB_TEXTEDIT_* functions which are manipulating wchar)
// FIXME: The existence of this rarely exercised code path is a bit of a nuisance.
void KarmaGuiInputTextCallbackData::DeleteChars(int pos, int bytes_count)
{
	KR_CORE_ASSERT(pos + bytes_count <= BufTextLen, "");
	char* dst = Buf + pos;
	const char* src = Buf + pos + bytes_count;
	while (char c = *src++)
		*dst++ = c;
	*dst = '\0';

	if (CursorPos >= pos + bytes_count)
		CursorPos -= bytes_count;
	else if (CursorPos >= pos)
		CursorPos = pos;
	SelectionStart = SelectionEnd = CursorPos;
	BufDirty = true;
	BufTextLen -= bytes_count;
}

void KarmaGuiInputTextCallbackData::InsertChars(int pos, const char* new_text, const char* new_text_end)
{
	const bool is_resizable = (Flags & KGGuiInputTextFlags_CallbackResize) != 0;
	const int new_text_len = new_text_end ? (int)(new_text_end - new_text) : (int)strlen(new_text);
	if (new_text_len + BufTextLen >= BufSize)
	{
		if (!is_resizable)
			return;

		// Contrary to STB_TEXTEDIT_INSERTCHARS() this is working in the UTF8 buffer, hence the mildly similar code (until we remove the U16 buffer altogether!)
		KarmaGuiContext& g = *Karma::GKarmaGui;
		KGGuiInputTextState* edit_state = &g.InputTextState;
		KR_CORE_ASSERT(edit_state->ID != 0 && g.ActiveId == edit_state->ID, "");
		KR_CORE_ASSERT(Buf == edit_state->TextA.Data, "");
		int new_buf_size = BufTextLen + KGClamp(new_text_len * 4, 32, KGMax(256, new_text_len)) + 1;
		edit_state->TextA.reserve(new_buf_size + 1);
		Buf = edit_state->TextA.Data;
		BufSize = edit_state->BufCapacityA = new_buf_size;
	}

	if (BufTextLen != pos)
		memmove(Buf + pos + new_text_len, Buf + pos, (size_t)(BufTextLen - pos));
	memcpy(Buf + pos, new_text, (size_t)new_text_len * sizeof(char));
	Buf[BufTextLen + new_text_len] = '\0';

	if (CursorPos >= pos)
		CursorPos += new_text_len;
	SelectionStart = SelectionEnd = CursorPos;
	BufDirty = true;
	BufTextLen += new_text_len;
}

// Return false to discard a character.
static bool Karma::InputTextFilterCharacter(unsigned int* p_char, KarmaGuiInputTextFlags flags, KarmaGuiInputTextCallback callback, void* user_data, KGGuiInputSource input_source)
{
	KR_CORE_ASSERT(input_source == KGGuiInputSource_Keyboard || input_source == KGGuiInputSource_Clipboard, "");
	unsigned int c = *p_char;

	// Filter non-printable (NB: isprint is unreliable! see #2467)
	bool apply_named_filters = true;
	if (c < 0x20)
	{
		bool pass = false;
		pass |= (c == '\n' && (flags & KGGuiInputTextFlags_Multiline)); // Note that an Enter KEY will emit \r and be ignored (we poll for KEY in InputText() code)
		pass |= (c == '\t' && (flags & KGGuiInputTextFlags_AllowTabInput));
		if (!pass)
			return false;
		apply_named_filters = false; // Override named filters below so newline and tabs can still be inserted.
	}

	if (input_source != KGGuiInputSource_Clipboard)
	{
		// We ignore Ascii representation of delete (emitted from Backspace on OSX, see #2578, #2817)
		if (c == 127)
			return false;

		// Filter private Unicode range. GLFW on OSX seems to send private characters for special keys like arrow keys (FIXME)
		if (c >= 0xE000 && c <= 0xF8FF)
			return false;
	}

	// Filter Unicode ranges we are not handling in this build
	if (c > KG_UNICODE_CODEPOINT_MAX)
		return false;

	// Generic named filters
	if (apply_named_filters && (flags & (KGGuiInputTextFlags_CharsDecimal | KGGuiInputTextFlags_CharsHexadecimal | KGGuiInputTextFlags_CharsUppercase | KGGuiInputTextFlags_CharsNoBlank | KGGuiInputTextFlags_CharsScientific)))
	{
		// The libc allows overriding locale, with e.g. 'setlocale(LC_NUMERIC, "de_DE.UTF-8");' which affect the output/input of printf/scanf to use e.g. ',' instead of '.'.
		// The standard mandate that programs starts in the "C" locale where the decimal point is '.'.
		// We don't really intend to provide widespread support for it, but out of empathy for people stuck with using odd API, we support the bare minimum aka overriding the decimal point.
		// Change the default decimal_point with:
		//   KarmaGui::GetCurrentContext()->PlatformLocaleDecimalPoint = *localeconv()->decimal_point;
		// Users of non-default decimal point (in particular ',') may be affected by word-selection logic (is_word_boundary_from_right/is_word_boundary_from_left) functions.
		KarmaGuiContext& g = *GKarmaGui;
		const unsigned c_decimal_point = (unsigned int)g.PlatformLocaleDecimalPoint;

		// Full-width -> half-width conversion for numeric fields (https://en.wikipedia.org/wiki/Halfwidth_and_Fullwidth_Forms_(Unicode_block)
		// While this is mostly convenient, this has the side-effect for uninformed users accidentally inputting full-width characters that they may
		// scratch their head as to why it works in numerical fields vs in generic text fields it would require support in the font.
		if (flags & (KGGuiInputTextFlags_CharsDecimal | KGGuiInputTextFlags_CharsScientific | KGGuiInputTextFlags_CharsHexadecimal))
			if (c >= 0xFF01 && c <= 0xFF5E)
				c = c - 0xFF01 + 0x21;

		// Allow 0-9 . - + * /
		if (flags & KGGuiInputTextFlags_CharsDecimal)
			if (!(c >= '0' && c <= '9') && (c != c_decimal_point) && (c != '-') && (c != '+') && (c != '*') && (c != '/'))
				return false;

		// Allow 0-9 . - + * / e E
		if (flags & KGGuiInputTextFlags_CharsScientific)
			if (!(c >= '0' && c <= '9') && (c != c_decimal_point) && (c != '-') && (c != '+') && (c != '*') && (c != '/') && (c != 'e') && (c != 'E'))
				return false;

		// Allow 0-9 a-F A-F
		if (flags & KGGuiInputTextFlags_CharsHexadecimal)
			if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f') && !(c >= 'A' && c <= 'F'))
				return false;

		// Turn a-z into A-Z
		if (flags & KGGuiInputTextFlags_CharsUppercase)
			if (c >= 'a' && c <= 'z')
				c += (unsigned int)('A' - 'a');

		if (flags & KGGuiInputTextFlags_CharsNoBlank)
			if (KGCharIsBlankW(c))
				return false;

		*p_char = c;
	}

	// Custom callback filter
	if (flags & KGGuiInputTextFlags_CallbackCharFilter)
	{
		KarmaGuiInputTextCallbackData callback_data;
		memset(&callback_data, 0, sizeof(KarmaGuiInputTextCallbackData));
		callback_data.EventFlag = KGGuiInputTextFlags_CallbackCharFilter;
		callback_data.EventChar = (KGWchar)c;
		callback_data.Flags = flags;
		callback_data.UserData = user_data;
		if (callback(&callback_data) != 0)
			return false;
		*p_char = callback_data.EventChar;
		if (!callback_data.EventChar)
			return false;
	}

	return true;
}

// Find the shortest single replacement we can make to get the new text from the old text.
// Important: needs to be run before TextW is rewritten with the new characters because calling STB_TEXTEDIT_GETCHAR() at the end.
// FIXME: Ideally we should transition toward (1) making InsertChars()/DeleteChars() update undo-stack (2) discourage (and keep reconcile) or obsolete (and remove reconcile) accessing buffer directly.
static void InputTextReconcileUndoStateAfterUserCallback(KGGuiInputTextState* state, const char* new_buf_a, int new_length_a)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	const KGWchar* old_buf = state->TextW.Data;
	const int old_length = state->CurLenW;
	const int new_length = KGTextCountCharsFromUtf8(new_buf_a, new_buf_a + new_length_a);
	g.TempBuffer.reserve_discard((new_length + 1) * sizeof(KGWchar));
	KGWchar* new_buf = (KGWchar*)(void*)g.TempBuffer.Data;
	KGTextStrFromUtf8(new_buf, new_length + 1, new_buf_a, new_buf_a + new_length_a);

	const int shorter_length = KGMin(old_length, new_length);
	int first_diff;
	for (first_diff = 0; first_diff < shorter_length; first_diff++)
		if (old_buf[first_diff] != new_buf[first_diff])
			break;
	if (first_diff == old_length && first_diff == new_length)
		return;

	int old_last_diff = old_length - 1;
	int new_last_diff = new_length - 1;
	for (; old_last_diff >= first_diff && new_last_diff >= first_diff; old_last_diff--, new_last_diff--)
		if (old_buf[old_last_diff] != new_buf[new_last_diff])
			break;

	const int insert_len = new_last_diff - first_diff + 1;
	const int delete_len = old_last_diff - first_diff + 1;
	if (insert_len > 0 || delete_len > 0)
		if (STB_TEXTEDIT_CHARTYPE* p = stb_text_createundo(&state->Stb.undostate, first_diff, delete_len, insert_len))
			for (int i = 0; i < delete_len; i++)
				p[i] = KGStb::STB_TEXTEDIT_GETCHAR(state, first_diff + i);
}

#ifndef STB_TEXT_HAS_SELECTION
#define STB_TEXT_HAS_SELECTION(s)   ((s)->select_start != (s)->select_end)
#endif

// Edit a string of text
// - buf_size account for the zero-terminator, so a buf_size of 6 can hold "Hello" but not "Hello!".
//   This is so we can easily call InputText() on static arrays using ARRAYSIZE() and to match
//   Note that in std::string world, capacity() would omit 1 byte used by the zero-terminator.
// - When active, hold on a privately held copy of the text (and apply back to 'buf'). So changing 'buf' while the InputText is active has no effect.
// - If you want to use KarmaGui::InputText() with std::string, see misc/cpp/imgui_stdlib.h
// (FIXME: Rather confusing and messy function, among the worse part of our codebase, expecting to rewrite a V2 at some point.. Partly because we are
//  doing UTF8 > U16 > UTF8 conversions on the go to easily interface with stb_textedit. Ideally should stay in UTF-8 all the time. See https://github.com/nothings/stb/issues/188)
bool Karma::KarmaGuiInternal::InputTextEx(const char* label, const char* hint, char* buf, int buf_size, const KGVec2& size_arg, KarmaGuiInputTextFlags flags, KarmaGuiInputTextCallback callback, void* callback_user_data)
{
	KGGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	KR_CORE_ASSERT(buf != NULL && buf_size >= 0, "");
	KR_CORE_ASSERT(!((flags & KGGuiInputTextFlags_CallbackHistory) && (flags & KGGuiInputTextFlags_Multiline)), "");        // Can't use both together (they both use up/down keys)
	KR_CORE_ASSERT(!((flags & KGGuiInputTextFlags_CallbackCompletion) && (flags & KGGuiInputTextFlags_AllowTabInput)), ""); // Can't use both together (they both use tab key)

	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiIO& io = g.IO;
	const KarmaGuiStyle& style = g.Style;

	const bool RENDER_SELECTION_WHEN_INACTIVE = false;
	const bool is_multiline = (flags & KGGuiInputTextFlags_Multiline) != 0;
	const bool is_readonly = (flags & KGGuiInputTextFlags_ReadOnly) != 0;
	const bool is_password = (flags & KGGuiInputTextFlags_Password) != 0;
	const bool is_undoable = (flags & KGGuiInputTextFlags_NoUndoRedo) == 0;
	const bool is_resizable = (flags & KGGuiInputTextFlags_CallbackResize) != 0;
	if (is_resizable)
	{
		KR_CORE_ASSERT(callback != NULL, ""); // Must provide a callback if you set the KGGuiInputTextFlags_CallbackResize flag!
	}

	if (is_multiline) // Open group before calling GetID() because groups tracks id created within their scope (including the scrollbar)
		Karma::KarmaGui::BeginGroup();
	const KGGuiID id = window->GetID(label);
	const KGVec2 label_size = KarmaGui::CalcTextSize(label, NULL, true);
	const KGVec2 frame_size = CalcItemSize(size_arg, KarmaGui::CalcItemWidth(), (is_multiline ? g.FontSize * 8.0f : label_size.y) + style.FramePadding.y * 2.0f); // Arbitrary default of 8 lines high for multi-line
	const KGVec2 total_size = KGVec2(frame_size.x + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), frame_size.y);

	const KGRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
	const KGRect total_bb(frame_bb.Min, frame_bb.Min + total_size);

	KGGuiWindow* draw_window = window;
	KGVec2 inner_size = frame_size;
	KGGuiItemStatusFlags item_status_flags = 0;
	KGGuiLastItemData item_data_backup;
	if (is_multiline)
	{
		KGVec2 backup_pos = window->DC.CursorPos;
		ItemSize(total_bb, style.FramePadding.y);
		if (!ItemAdd(total_bb, id, &frame_bb, KGGuiItemFlags_Inputable))
		{
			KarmaGui::EndGroup();
			return false;
		}
		item_status_flags = g.LastItemData.StatusFlags;
		item_data_backup = g.LastItemData;
		window->DC.CursorPos = backup_pos;

		// We reproduce the contents of BeginChildFrame() in order to provide 'label' so our window internal data are easier to read/debug.
		// FIXME-NAV: Pressing NavActivate will trigger general child activation right before triggering our own below. Harmless but bizarre.
		KarmaGui::PushStyleColor(KGGuiCol_ChildBg, style.Colors[KGGuiCol_FrameBg]);
		KarmaGui::PushStyleVar(KGGuiStyleVar_ChildRounding, style.FrameRounding);
		KarmaGui::PushStyleVar(KGGuiStyleVar_ChildBorderSize, style.FrameBorderSize);
		KarmaGui::PushStyleVar(KGGuiStyleVar_WindowPadding, KGVec2(0, 0)); // Ensure no clip rect so mouse hover can reach FramePadding edges
		bool child_visible = BeginChildEx(label, id, frame_bb.GetSize(), true, KGGuiWindowFlags_NoMove);
		KarmaGui::PopStyleVar(3);
		KarmaGui::PopStyleColor();
		if (!child_visible)
		{
			KarmaGui::EndChild();
			KarmaGui::EndGroup();
			return false;
		}
		draw_window = g.CurrentWindow; // Child window
		draw_window->DC.NavLayersActiveMaskNext |= (1 << draw_window->DC.NavLayerCurrent); // This is to ensure that EndChild() will display a navigation highlight so we can "enter" into it.
		draw_window->DC.CursorPos += style.FramePadding;
		inner_size.x -= draw_window->ScrollbarSizes.x;
	}
	else
	{
		// Support for internal KGGuiInputTextFlags_MergedItem flag, which could be redesigned as an ItemFlags if needed (with test performed in ItemAdd)
		ItemSize(total_bb, style.FramePadding.y);
		if (!(flags & KGGuiInputTextFlags_MergedItem))
			if (!ItemAdd(total_bb, id, &frame_bb, KGGuiItemFlags_Inputable))
				return false;
		item_status_flags = g.LastItemData.StatusFlags;
	}
	const bool hovered = ItemHoverable(frame_bb, id);
	if (hovered)
		g.MouseCursor = KGGuiMouseCursor_TextInput;

	// We are only allowed to access the state if we are already the active widget.
	KGGuiInputTextState* state = GetInputTextState(id);

	const bool input_requested_by_tabbing = (item_status_flags & KGGuiItemStatusFlags_FocusedByTabbing) != 0;
	const bool input_requested_by_nav = (g.ActiveId != id) && ((g.NavActivateInputId == id) || (g.NavActivateId == id && g.NavInputSource == KGGuiInputSource_Keyboard));

	const bool user_clicked = hovered && io.MouseClicked[0];
	const bool user_scroll_finish = is_multiline && state != NULL && g.ActiveId == 0 && g.ActiveIdPreviousFrame == GetWindowScrollbarID(draw_window, KGGuiAxis_Y);
	const bool user_scroll_active = is_multiline && state != NULL && g.ActiveId == GetWindowScrollbarID(draw_window, KGGuiAxis_Y);
	bool clear_active_id = false;
	bool select_all = false;

	float scroll_y = is_multiline ? draw_window->Scroll.y : FLT_MAX;

	const bool init_changed_specs = (state != NULL && state->Stb.single_line != !is_multiline);
	const bool init_make_active = (user_clicked || user_scroll_finish || input_requested_by_nav || input_requested_by_tabbing);
	const bool init_state = (init_make_active || user_scroll_active);
	if ((init_state && g.ActiveId != id) || init_changed_specs)
	{
		// Access state even if we don't own it yet.
		state = &g.InputTextState;
		state->CursorAnimReset();

		// Take a copy of the initial buffer value (both in original UTF-8 format and converted to wchar)
		// From the moment we focused we are ignoring the content of 'buf' (unless we are in read-only mode)
		const int buf_len = (int)strlen(buf);
		state->InitialTextA.resize(buf_len + 1);    // UTF-8. we use +1 to make sure that .Data is always pointing to at least an empty string.
		memcpy(state->InitialTextA.Data, buf, buf_len + 1);

		// Preserve cursor position and undo/redo stack if we come back to same widget
		// FIXME: Since we reworked this on 2022/06, may want to differenciate recycle_cursor vs recycle_undostate?
		bool recycle_state = (state->ID == id && !init_changed_specs);
		if (recycle_state && (state->CurLenA != buf_len || (state->TextAIsValid && strncmp(state->TextA.Data, buf, buf_len) != 0)))
			recycle_state = false;

		// Start edition
		const char* buf_end = NULL;
		state->ID = id;
		state->TextW.resize(buf_size + 1);          // wchar count <= UTF-8 count. we use +1 to make sure that .Data is always pointing to at least an empty string.
		state->TextA.resize(0);
		state->TextAIsValid = false;                // TextA is not valid yet (we will display buf until then)
		state->CurLenW = KGTextStrFromUtf8(state->TextW.Data, buf_size, buf, NULL, &buf_end);
		state->CurLenA = (int)(buf_end - buf);      // We can't get the result from KGStrncpy() above because it is not UTF-8 aware. Here we'll cut off malformed UTF-8.

		if (recycle_state)
		{
			// Recycle existing cursor/selection/undo stack but clamp position
			// Note a single mouse click will override the cursor/position immediately by calling stb_textedit_click handler.
			state->CursorClamp();
		}
		else
		{
			state->ScrollX = 0.0f;
			stb_textedit_initialize_state(&state->Stb, !is_multiline);
		}

		if (!is_multiline)
		{
			if (flags & KGGuiInputTextFlags_AutoSelectAll)
				select_all = true;
			if (input_requested_by_nav && (!recycle_state || !(g.NavActivateFlags & KGGuiActivateFlags_TryToPreserveState)))
				select_all = true;
			if (input_requested_by_tabbing || (user_clicked && io.KeyCtrl))
				select_all = true;
		}

		if (flags & KGGuiInputTextFlags_AlwaysOverwrite)
			state->Stb.insert_mode = 1; // stb field name is indeed incorrect (see #2863)
	}

	if (g.ActiveId != id && init_make_active)
	{
		KR_CORE_ASSERT(state && state->ID == id, "");
		SetActiveID(id, window);
		SetFocusID(id, window);
		FocusWindow(window);
	}
	if (g.ActiveId == id)
	{
		// Declare some inputs, the other are registered and polled via Shortcut() routing system.
		if (user_clicked)
			SetKeyOwner(KGGuiKey_MouseLeft, id);
		g.ActiveIdUsingNavDirMask |= (1 << KGGuiDir_Left) | (1 << KGGuiDir_Right);
		if (is_multiline || (flags & KGGuiInputTextFlags_CallbackHistory))
			g.ActiveIdUsingNavDirMask |= (1 << KGGuiDir_Up) | (1 << KGGuiDir_Down);
		SetKeyOwner(KGGuiKey_Home, id);
		SetKeyOwner(KGGuiKey_End, id);
		if (is_multiline)
		{
			SetKeyOwner(KGGuiKey_PageUp, id);
			SetKeyOwner(KGGuiKey_PageDown, id);
		}
		if (flags & (KGGuiInputTextFlags_CallbackCompletion | KGGuiInputTextFlags_AllowTabInput)) // Disable keyboard tabbing out as we will use the \t character.
			SetKeyOwner(KGGuiKey_Tab, id);
	}

	// We have an edge case if ActiveId was set through another widget (e.g. widget being swapped), clear id immediately (don't wait until the end of the function)
	if (g.ActiveId == id && state == NULL)
		ClearActiveID();

	// Release focus when we click outside
	if (g.ActiveId == id && io.MouseClicked[0] && !init_state && !init_make_active) //-V560
		clear_active_id = true;

	// Lock the decision of whether we are going to take the path displaying the cursor or selection
	bool render_cursor = (g.ActiveId == id) || (state && user_scroll_active);
	bool render_selection = state && (state->HasSelection() || select_all) && (RENDER_SELECTION_WHEN_INACTIVE || render_cursor);
	bool value_changed = false;
	bool validated = false;

	// When read-only we always use the live data passed to the function
	// FIXME-OPT: Because our selection/cursor code currently needs the wide text we need to convert it when active, which is not ideal :(
	if (is_readonly && state != NULL && (render_cursor || render_selection))
	{
		const char* buf_end = NULL;
		state->TextW.resize(buf_size + 1);
		state->CurLenW = KGTextStrFromUtf8(state->TextW.Data, state->TextW.Size, buf, NULL, &buf_end);
		state->CurLenA = (int)(buf_end - buf);
		state->CursorClamp();
		render_selection &= state->HasSelection();
	}

	// Select the buffer to render.
	const bool buf_display_from_state = (render_cursor || render_selection || g.ActiveId == id) && !is_readonly && state && state->TextAIsValid;
	const bool is_displaying_hint = (hint != NULL && (buf_display_from_state ? state->TextA.Data : buf)[0] == 0);

	// Password pushes a temporary font with only a fallback glyph
	if (is_password && !is_displaying_hint)
	{
		const KGFontGlyph* glyph = g.Font->FindGlyph('*');
		KGFont* password_font = &g.InputTextPasswordFont;
		password_font->FontSize = g.Font->FontSize;
		password_font->Scale = g.Font->Scale;
		password_font->Ascent = g.Font->Ascent;
		password_font->Descent = g.Font->Descent;
		password_font->ContainerAtlas = g.Font->ContainerAtlas;
		password_font->FallbackGlyph = glyph;
		password_font->FallbackAdvanceX = glyph->AdvanceX;
		KR_CORE_ASSERT(password_font->Glyphs.empty() && password_font->IndexAdvanceX.empty() && password_font->IndexLookup.empty(), "");
		KarmaGui::PushFont(password_font);
	}

	// Process mouse inputs and character inputs
	int backup_current_text_length = 0;
	if (g.ActiveId == id)
	{
		KR_CORE_ASSERT(state != NULL, "");
		backup_current_text_length = state->CurLenA;
		state->Edited = false;
		state->BufCapacityA = buf_size;
		state->Flags = flags;

		// Although we are active we don't prevent mouse from hovering other elements unless we are interacting right now with the widget.
		// Down the line we should have a cleaner library-wide concept of Selected vs Active.
		g.ActiveIdAllowOverlap = !io.MouseDown[0];
		g.WantTextInputNextFrame = 1;

		// Edit in progress
		const float mouse_x = (io.MousePos.x - frame_bb.Min.x - style.FramePadding.x) + state->ScrollX;
		const float mouse_y = (is_multiline ? (io.MousePos.y - draw_window->DC.CursorPos.y) : (g.FontSize * 0.5f));

		const bool is_osx = io.ConfigMacOSXBehaviors;
		if (select_all)
		{
			state->SelectAll();
			state->SelectedAllMouseLock = true;
		}
		else if (hovered && io.MouseClickedCount[0] >= 2 && !io.KeyShift)
		{
			stb_textedit_click(state, &state->Stb, mouse_x, mouse_y);
			const int multiclick_count = (io.MouseClickedCount[0] - 2);
			if ((multiclick_count % 2) == 0)
			{
				// Double-click: Select word
				// We always use the "Mac" word advance for double-click select vs CTRL+Right which use the platform dependent variant:
				// FIXME: There are likely many ways to improve this behavior, but there's no "right" behavior (depends on use-case, software, OS)
				const bool is_bol = (state->Stb.cursor == 0) || KGStb::STB_TEXTEDIT_GETCHAR(state, state->Stb.cursor - 1) == '\n';
				if (STB_TEXT_HAS_SELECTION(&state->Stb) || !is_bol)
					state->OnKeyPressed(STB_TEXTEDIT_K_WORDLEFT);
				//state->OnKeyPressed(STB_TEXTEDIT_K_WORDRIGHT | STB_TEXTEDIT_K_SHIFT);
				if (!STB_TEXT_HAS_SELECTION(&state->Stb))
					KGStb::stb_textedit_prep_selection_at_cursor(&state->Stb);
				state->Stb.cursor = KGStb::STB_TEXTEDIT_MOVEWORDRIGHT_MAC(state, state->Stb.cursor);
				state->Stb.select_end = state->Stb.cursor;
				KGStb::stb_textedit_clamp(state, &state->Stb);
			}
			else
			{
				// Triple-click: Select line
				const bool is_eol = KGStb::STB_TEXTEDIT_GETCHAR(state, state->Stb.cursor) == '\n';
				state->OnKeyPressed(STB_TEXTEDIT_K_LINESTART);
				state->OnKeyPressed(STB_TEXTEDIT_K_LINEEND | STB_TEXTEDIT_K_SHIFT);
				state->OnKeyPressed(STB_TEXTEDIT_K_RIGHT | STB_TEXTEDIT_K_SHIFT);
				if (!is_eol && is_multiline)
				{
					KGSwap(state->Stb.select_start, state->Stb.select_end);
					state->Stb.cursor = state->Stb.select_end;
				}
				state->CursorFollow = false;
			}
			state->CursorAnimReset();
		}
		else if (io.MouseClicked[0] && !state->SelectedAllMouseLock)
		{
			if (hovered)
			{
				if (io.KeyShift)
					stb_textedit_drag(state, &state->Stb, mouse_x, mouse_y);
				else
					stb_textedit_click(state, &state->Stb, mouse_x, mouse_y);
				state->CursorAnimReset();
			}
		}
		else if (io.MouseDown[0] && !state->SelectedAllMouseLock && (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f))
		{
			stb_textedit_drag(state, &state->Stb, mouse_x, mouse_y);
			state->CursorAnimReset();
			state->CursorFollow = true;
		}
		if (state->SelectedAllMouseLock && !io.MouseDown[0])
			state->SelectedAllMouseLock = false;

		// We expect backends to emit a Tab key but some also emit a Tab character which we ignore (#2467, #1336)
		// (For Tab and Enter: Win32/SFML/Allegro are sending both keys and chars, GLFW and SDL are only sending keys. For Space they all send all threes)
		const bool ignore_char_inputs = (io.KeyCtrl && !io.KeyAlt) || (is_osx && io.KeySuper);
		if ((flags & KGGuiInputTextFlags_AllowTabInput) && KarmaGui::IsKeyPressed(KGGuiKey_Tab) && !ignore_char_inputs && !io.KeyShift && !is_readonly)
		{
			unsigned int c = '\t'; // Insert TAB
			if (Karma::InputTextFilterCharacter(&c, flags, callback, callback_user_data, KGGuiInputSource_Keyboard))
				state->OnKeyPressed((int)c);
		}

		// Process regular text input (before we check for Return because using some IME will effectively send a Return?)
		// We ignore CTRL inputs, but need to allow ALT+CTRL as some keyboards (e.g. German) use AltGR (which _is_ Alt+Ctrl) to input certain characters.
		if (io.InputQueueCharacters.Size > 0)
		{
			if (!ignore_char_inputs && !is_readonly && !input_requested_by_nav)
				for (int n = 0; n < io.InputQueueCharacters.Size; n++)
				{
					// Insert character if they pass filtering
					unsigned int c = (unsigned int)io.InputQueueCharacters[n];
					if (c == '\t') // Skip Tab, see above.
						continue;
					if (Karma::InputTextFilterCharacter(&c, flags, callback, callback_user_data, KGGuiInputSource_Keyboard))
						state->OnKeyPressed((int)c);
				}

			// Consume characters
			io.InputQueueCharacters.resize(0);
		}
	}

	// Process other shortcuts/key-presses
	bool revert_edit = false;
	if (g.ActiveId == id && !g.ActiveIdIsJustActivated && !clear_active_id)
	{
		KR_CORE_ASSERT(state != NULL, "");

		const int row_count_per_page = KGMax((int)((inner_size.y - style.FramePadding.y) / g.FontSize), 1);
		state->Stb.row_count_per_page = row_count_per_page;

		const int k_mask = (io.KeyShift ? STB_TEXTEDIT_K_SHIFT : 0);
		const bool is_osx = io.ConfigMacOSXBehaviors;
		const bool is_wordmove_key_down = is_osx ? io.KeyAlt : io.KeyCtrl;                     // OS X style: Text editing cursor movement using Alt instead of Ctrl
		const bool is_startend_key_down = is_osx && io.KeySuper && !io.KeyCtrl && !io.KeyAlt;  // OS X style: Line/Text Start and End using Cmd+Arrows instead of Home/End

		// Using Shortcut() with KGGuiInputFlags_RouteFocused (default policy) to allow routing operations for other code (e.g. calling window trying to use CTRL+A and CTRL+B: formet would be handled by InputText)
		// Otherwise we could simply assume that we own the keys as we are active.
		const KarmaGuiInputFlags f_repeat = KGGuiInputFlags_Repeat;
		const bool is_cut = (KarmaGui::Shortcut(KGGuiMod_Shortcut | KGGuiKey_X, id, f_repeat) || KarmaGui::Shortcut(KGGuiMod_Shift | KGGuiKey_Delete, id, f_repeat)) && !is_readonly && !is_password && (!is_multiline || state->HasSelection());
		const bool is_copy = (KarmaGui::Shortcut(KGGuiMod_Shortcut | KGGuiKey_C, id) || KarmaGui::Shortcut(KGGuiMod_Ctrl | KGGuiKey_Insert, id)) && !is_password && (!is_multiline || state->HasSelection());
		const bool is_paste = (KarmaGui::Shortcut(KGGuiMod_Shortcut | KGGuiKey_V, id, f_repeat) || KarmaGui::Shortcut(KGGuiMod_Shift | KGGuiKey_Insert, id, f_repeat)) && !is_readonly;
		const bool is_undo = (KarmaGui::Shortcut(KGGuiMod_Shortcut | KGGuiKey_Z, id, f_repeat)) && !is_readonly && is_undoable;
		const bool is_redo = (KarmaGui::Shortcut(KGGuiMod_Shortcut | KGGuiKey_Y, id, f_repeat) || (is_osx && KarmaGui::Shortcut(KGGuiMod_Shortcut | KGGuiMod_Shift | KGGuiKey_Z, id, f_repeat))) && !is_readonly && is_undoable;
		const bool is_select_all = KarmaGui::Shortcut(KGGuiMod_Shortcut | KGGuiKey_A, id);

		// We allow validate/cancel with Nav source (gamepad) to makes it easier to undo an accidental NavInput press with no keyboard wired, but otherwise it isn't very useful.
		const bool nav_gamepad_active = (io.ConfigFlags & KGGuiConfigFlags_NavEnableGamepad) != 0 && (io.BackendFlags & KGGuiBackendFlags_HasGamepad) != 0;
		const bool is_enter_pressed = IsKeyPressed(KGGuiKey_Enter, true) || IsKeyPressed(KGGuiKey_KeypadEnter, true);
		const bool is_gamepad_validate = nav_gamepad_active && (IsKeyPressed(KGGuiKey_NavGamepadActivate, false) || IsKeyPressed(KGGuiKey_NavGamepadInput, false));
		const bool is_cancel = KarmaGui::Shortcut(KGGuiKey_Escape, id, f_repeat) || (nav_gamepad_active && KarmaGui::Shortcut(KGGuiKey_NavGamepadCancel, id, f_repeat));

		// FIXME: Should use more Shortcut() and reduce IsKeyPressed()+SetKeyOwner(), but requires modifiers combination to be taken account of.
		if (KarmaGui::IsKeyPressed(KGGuiKey_LeftArrow)) { state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_LINESTART : is_wordmove_key_down ? STB_TEXTEDIT_K_WORDLEFT : STB_TEXTEDIT_K_LEFT) | k_mask); }
		else if (KarmaGui::IsKeyPressed(KGGuiKey_RightArrow)) { state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_LINEEND : is_wordmove_key_down ? STB_TEXTEDIT_K_WORDRIGHT : STB_TEXTEDIT_K_RIGHT) | k_mask); }
		else if (KarmaGui::IsKeyPressed(KGGuiKey_UpArrow) && is_multiline) { if (io.KeyCtrl) SetScrollY(draw_window, KGMax(draw_window->Scroll.y - g.FontSize, 0.0f)); else state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_TEXTSTART : STB_TEXTEDIT_K_UP) | k_mask); }
		else if (KarmaGui::IsKeyPressed(KGGuiKey_DownArrow) && is_multiline) { if (io.KeyCtrl) SetScrollY(draw_window, KGMin(draw_window->Scroll.y + g.FontSize, KarmaGui::GetScrollMaxY())); else state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_TEXTEND : STB_TEXTEDIT_K_DOWN) | k_mask); }
		else if (KarmaGui::IsKeyPressed(KGGuiKey_PageUp) && is_multiline) { state->OnKeyPressed(STB_TEXTEDIT_K_PGUP | k_mask); scroll_y -= row_count_per_page * g.FontSize; }
		else if (KarmaGui::IsKeyPressed(KGGuiKey_PageDown) && is_multiline) { state->OnKeyPressed(STB_TEXTEDIT_K_PGDOWN | k_mask); scroll_y += row_count_per_page * g.FontSize; }
		else if (KarmaGui::IsKeyPressed(KGGuiKey_Home)) { state->OnKeyPressed(io.KeyCtrl ? STB_TEXTEDIT_K_TEXTSTART | k_mask : STB_TEXTEDIT_K_LINESTART | k_mask); }
		else if (KarmaGui::IsKeyPressed(KGGuiKey_End)) { state->OnKeyPressed(io.KeyCtrl ? STB_TEXTEDIT_K_TEXTEND | k_mask : STB_TEXTEDIT_K_LINEEND | k_mask); }
		else if (KarmaGui::IsKeyPressed(KGGuiKey_Delete) && !is_readonly && !is_cut) { state->OnKeyPressed(STB_TEXTEDIT_K_DELETE | k_mask); }
		else if (KarmaGui::IsKeyPressed(KGGuiKey_Backspace) && !is_readonly)
		{
			if (!state->HasSelection())
			{
				if (is_wordmove_key_down)
					state->OnKeyPressed(STB_TEXTEDIT_K_WORDLEFT | STB_TEXTEDIT_K_SHIFT);
				else if (is_osx && io.KeySuper && !io.KeyAlt && !io.KeyCtrl)
					state->OnKeyPressed(STB_TEXTEDIT_K_LINESTART | STB_TEXTEDIT_K_SHIFT);
			}
			state->OnKeyPressed(STB_TEXTEDIT_K_BACKSPACE | k_mask);
		}
		else if (is_enter_pressed || is_gamepad_validate)
		{
			// Determine if we turn Enter into a \n character
			bool ctrl_enter_for_new_line = (flags & KGGuiInputTextFlags_CtrlEnterForNewLine) != 0;
			if (!is_multiline || is_gamepad_validate || (ctrl_enter_for_new_line && !io.KeyCtrl) || (!ctrl_enter_for_new_line && io.KeyCtrl))
			{
				validated = true;
				if (io.ConfigInputTextEnterKeepActive && !is_multiline)
					state->SelectAll(); // No need to scroll
				else
					clear_active_id = true;
			}
			else if (!is_readonly)
			{
				unsigned int c = '\n'; // Insert new line
				if (Karma::InputTextFilterCharacter(&c, flags, callback, callback_user_data, KGGuiInputSource_Keyboard))
					state->OnKeyPressed((int)c);
			}
		}
		else if (is_cancel)
		{
			if (flags & KGGuiInputTextFlags_EscapeClearsAll)
			{
				if (state->CurLenA > 0)
				{
					revert_edit = true;
				}
				else
				{
					render_cursor = render_selection = false;
					clear_active_id = true;
				}
			}
			else
			{
				clear_active_id = revert_edit = true;
				render_cursor = render_selection = false;
			}
		}
		else if (is_undo || is_redo)
		{
			state->OnKeyPressed(is_undo ? STB_TEXTEDIT_K_UNDO : STB_TEXTEDIT_K_REDO);
			state->ClearSelection();
		}
		else if (is_select_all)
		{
			state->SelectAll();
			state->CursorFollow = true;
		}
		else if (is_cut || is_copy)
		{
			// Cut, Copy
			if (io.SetClipboardTextFn)
			{
				const int ib = state->HasSelection() ? KGMin(state->Stb.select_start, state->Stb.select_end) : 0;
				const int ie = state->HasSelection() ? KGMax(state->Stb.select_start, state->Stb.select_end) : state->CurLenW;
				const int clipboard_data_len = KGTextCountUtf8BytesFromStr(state->TextW.Data + ib, state->TextW.Data + ie) + 1;
				char* clipboard_data = (char*)KG_ALLOC(clipboard_data_len * sizeof(char));
				KGTextStrToUtf8(clipboard_data, clipboard_data_len, state->TextW.Data + ib, state->TextW.Data + ie);
				KarmaGui::SetClipboardText(clipboard_data);
				KarmaGui::MemFree(clipboard_data);
			}
			if (is_cut)
			{
				if (!state->HasSelection())
					state->SelectAll();
				state->CursorFollow = true;
				KGStb::stb_textedit_clamp(state, &state->Stb);
			}
		}
		else if (is_paste)
		{
			if (const char* clipboard = KarmaGui::GetClipboardText())
			{
				// Filter pasted buffer
				const int clipboard_len = (int)strlen(clipboard);
				KGWchar* clipboard_filtered = (KGWchar*)KG_ALLOC((clipboard_len + 1) * sizeof(KGWchar));
				int clipboard_filtered_len = 0;
				for (const char* s = clipboard; *s; )
				{
					unsigned int c;
					s += KGTextCharFromUtf8(&c, s, NULL);
					if (c == 0)
						break;
					if (!Karma::InputTextFilterCharacter(&c, flags, callback, callback_user_data, KGGuiInputSource_Clipboard))
						continue;
					clipboard_filtered[clipboard_filtered_len++] = (KGWchar)c;
				}
				clipboard_filtered[clipboard_filtered_len] = 0;
				if (clipboard_filtered_len > 0) // If everything was filtered, ignore the pasting operation
				{
					stb_textedit_paste(state, &state->Stb, clipboard_filtered, clipboard_filtered_len);
					state->CursorFollow = true;
				}
				KarmaGui::MemFree(clipboard_filtered);
			}
		}

		// Update render selection flag after events have been handled, so selection highlight can be displayed during the same frame.
		render_selection |= state->HasSelection() && (RENDER_SELECTION_WHEN_INACTIVE || render_cursor);
	}

	// Process callbacks and apply result back to user's buffer.
	const char* apply_new_text = NULL;
	int apply_new_text_length = 0;
	if (g.ActiveId == id)
	{
		KR_CORE_ASSERT(state != NULL, "");
		if (revert_edit && !is_readonly)
		{
			if (flags & KGGuiInputTextFlags_EscapeClearsAll)
			{
				// Clear input
				apply_new_text = "";
				apply_new_text_length = 0;
				STB_TEXTEDIT_CHARTYPE empty_string;
				stb_textedit_replace(state, &state->Stb, &empty_string, 0);
			}
			else if (strcmp(buf, state->InitialTextA.Data) != 0)
			{
				// Restore initial value. Only return true if restoring to the initial value changes the current buffer contents.
				// Push records into the undo stack so we can CTRL+Z the revert operation itself
				apply_new_text = state->InitialTextA.Data;
				apply_new_text_length = state->InitialTextA.Size - 1;
				KGVector<KGWchar> w_text;
				if (apply_new_text_length > 0)
				{
					w_text.resize(KGTextCountCharsFromUtf8(apply_new_text, apply_new_text + apply_new_text_length) + 1);
					KGTextStrFromUtf8(w_text.Data, w_text.Size, apply_new_text, apply_new_text + apply_new_text_length);
				}
				stb_textedit_replace(state, &state->Stb, w_text.Data, (apply_new_text_length > 0) ? (w_text.Size - 1) : 0);
			}
		}

		// Apply ASCII value
		if (!is_readonly)
		{
			state->TextAIsValid = true;
			state->TextA.resize(state->TextW.Size * 4 + 1);
			KGTextStrToUtf8(state->TextA.Data, state->TextA.Size, state->TextW.Data, NULL);
		}

		// When using 'KGGuiInputTextFlags_EnterReturnsTrue' as a special case we reapply the live buffer back to the input buffer before clearing ActiveId, even though strictly speaking it wasn't modified on this frame.
		// If we didn't do that, code like InputInt() with KGGuiInputTextFlags_EnterReturnsTrue would fail.
		// This also allows the user to use InputText() with KGGuiInputTextFlags_EnterReturnsTrue without maintaining any user-side storage (please note that if you use this property along KGGuiInputTextFlags_CallbackResize you can end up with your temporary string object unnecessarily allocating once a frame, either store your string data, either if you don't then don't use KGGuiInputTextFlags_CallbackResize).
		const bool apply_edit_back_to_user_buffer = !revert_edit || (validated && (flags & KGGuiInputTextFlags_EnterReturnsTrue) != 0);
		if (apply_edit_back_to_user_buffer)
		{
			// Apply new value immediately - copy modified buffer back
			// Note that as soon as the input box is active, the in-widget value gets priority over any underlying modification of the input buffer
			// FIXME: We actually always render 'buf' when calling DrawList->AddText, making the comment above incorrect.
			// FIXME-OPT: CPU waste to do this every time the widget is active, should mark dirty state from the stb_textedit callbacks.

			// User callback
			if ((flags & (KGGuiInputTextFlags_CallbackCompletion | KGGuiInputTextFlags_CallbackHistory | KGGuiInputTextFlags_CallbackEdit | KGGuiInputTextFlags_CallbackAlways)) != 0)
			{
				KR_CORE_ASSERT(callback != NULL, "");

				// The reason we specify the usage semantic (Completion/History) is that Completion needs to disable keyboard TABBING at the moment.
				KarmaGuiInputTextFlags event_flag = 0;
				KarmaGuiKey event_key = KGGuiKey_None;
				if ((flags & KGGuiInputTextFlags_CallbackCompletion) != 0 && KarmaGui::IsKeyPressed(KGGuiKey_Tab))
				{
					event_flag = KGGuiInputTextFlags_CallbackCompletion;
					event_key = KGGuiKey_Tab;
				}
				else if ((flags & KGGuiInputTextFlags_CallbackHistory) != 0 && KarmaGui::IsKeyPressed(KGGuiKey_UpArrow))
				{
					event_flag = KGGuiInputTextFlags_CallbackHistory;
					event_key = KGGuiKey_UpArrow;
				}
				else if ((flags & KGGuiInputTextFlags_CallbackHistory) != 0 && KarmaGui::IsKeyPressed(KGGuiKey_DownArrow))
				{
					event_flag = KGGuiInputTextFlags_CallbackHistory;
					event_key = KGGuiKey_DownArrow;
				}
				else if ((flags & KGGuiInputTextFlags_CallbackEdit) && state->Edited)
				{
					event_flag = KGGuiInputTextFlags_CallbackEdit;
				}
				else if (flags & KGGuiInputTextFlags_CallbackAlways)
				{
					event_flag = KGGuiInputTextFlags_CallbackAlways;
				}

				if (event_flag)
				{
					KarmaGuiInputTextCallbackData callback_data;
					memset(&callback_data, 0, sizeof(KarmaGuiInputTextCallbackData));
					callback_data.EventFlag = event_flag;
					callback_data.Flags = flags;
					callback_data.UserData = callback_user_data;

					char* callback_buf = is_readonly ? buf : state->TextA.Data;
					callback_data.EventKey = event_key;
					callback_data.Buf = callback_buf;
					callback_data.BufTextLen = state->CurLenA;
					callback_data.BufSize = state->BufCapacityA;
					callback_data.BufDirty = false;

					// We have to convert from wchar-positions to UTF-8-positions, which can be pretty slow (an incentive to ditch the KGWchar buffer, see https://github.com/nothings/stb/issues/188)
					KGWchar* text = state->TextW.Data;
					const int utf8_cursor_pos = callback_data.CursorPos = KGTextCountUtf8BytesFromStr(text, text + state->Stb.cursor);
					const int utf8_selection_start = callback_data.SelectionStart = KGTextCountUtf8BytesFromStr(text, text + state->Stb.select_start);
					const int utf8_selection_end = callback_data.SelectionEnd = KGTextCountUtf8BytesFromStr(text, text + state->Stb.select_end);

					// Call user code
					callback(&callback_data);

					// Read back what user may have modified
					callback_buf = is_readonly ? buf : state->TextA.Data; // Pointer may have been invalidated by a resize callback
					KR_CORE_ASSERT(callback_data.Buf == callback_buf, "");         // Invalid to modify those fields
					KR_CORE_ASSERT(callback_data.BufSize == state->BufCapacityA, "");
					KR_CORE_ASSERT(callback_data.Flags == flags, "");
					const bool buf_dirty = callback_data.BufDirty;
					if (callback_data.CursorPos != utf8_cursor_pos || buf_dirty) { state->Stb.cursor = KGTextCountCharsFromUtf8(callback_data.Buf, callback_data.Buf + callback_data.CursorPos); state->CursorFollow = true; }
					if (callback_data.SelectionStart != utf8_selection_start || buf_dirty) { state->Stb.select_start = (callback_data.SelectionStart == callback_data.CursorPos) ? state->Stb.cursor : KGTextCountCharsFromUtf8(callback_data.Buf, callback_data.Buf + callback_data.SelectionStart); }
					if (callback_data.SelectionEnd != utf8_selection_end || buf_dirty) { state->Stb.select_end = (callback_data.SelectionEnd == callback_data.SelectionStart) ? state->Stb.select_start : KGTextCountCharsFromUtf8(callback_data.Buf, callback_data.Buf + callback_data.SelectionEnd); }
					if (buf_dirty)
					{
						KR_CORE_ASSERT((flags & KGGuiInputTextFlags_ReadOnly) == 0, "");
						KR_CORE_ASSERT(callback_data.BufTextLen == (int)strlen(callback_data.Buf), ""); // You need to maintain BufTextLen if you change the text!
						InputTextReconcileUndoStateAfterUserCallback(state, callback_data.Buf, callback_data.BufTextLen); // FIXME: Move the rest of this block inside function and rename to InputTextReconcileStateAfterUserCallback() ?
						if (callback_data.BufTextLen > backup_current_text_length && is_resizable)
							state->TextW.resize(state->TextW.Size + (callback_data.BufTextLen - backup_current_text_length)); // Worse case scenario resize
						state->CurLenW = KGTextStrFromUtf8(state->TextW.Data, state->TextW.Size, callback_data.Buf, NULL);
						state->CurLenA = callback_data.BufTextLen;  // Assume correct length and valid UTF-8 from user, saves us an extra strlen()
						state->CursorAnimReset();
					}
				}
			}

			// Will copy result string if modified
			if (!is_readonly && strcmp(state->TextA.Data, buf) != 0)
			{
				apply_new_text = state->TextA.Data;
				apply_new_text_length = state->CurLenA;
			}
		}
	}

	// Copy result to user buffer. This can currently only happen when (g.ActiveId == id)
	if (apply_new_text != NULL)
	{
		// We cannot test for 'backup_current_text_length != apply_new_text_length' here because we have no guarantee that the size
		// of our owned buffer matches the size of the string object held by the user, and by design we allow InputText() to be used
		// without any storage on user's side.
		KR_CORE_ASSERT(apply_new_text_length >= 0, "");
		if (is_resizable)
		{
			KarmaGuiInputTextCallbackData callback_data;
			callback_data.EventFlag = KGGuiInputTextFlags_CallbackResize;
			callback_data.Flags = flags;
			callback_data.Buf = buf;
			callback_data.BufTextLen = apply_new_text_length;
			callback_data.BufSize = KGMax(buf_size, apply_new_text_length + 1);
			callback_data.UserData = callback_user_data;
			callback(&callback_data);
			buf = callback_data.Buf;
			buf_size = callback_data.BufSize;
			apply_new_text_length = KGMin(callback_data.BufTextLen, buf_size - 1);
			KR_CORE_ASSERT(apply_new_text_length <= buf_size, "");
		}
		//IMGUI_DEBUG_PRINT("InputText(\"%s\"): apply_new_text length %d\n", label, apply_new_text_length);

		// If the underlying buffer resize was denied or not carried to the next frame, apply_new_text_length+1 may be >= buf_size.
		KGStrncpy(buf, apply_new_text, KGMin(apply_new_text_length + 1, buf_size));
		value_changed = true;
	}

	// Release active ID at the end of the function (so e.g. pressing Return still does a final application of the value)
	if (clear_active_id && g.ActiveId == id)
		ClearActiveID();

	// Render frame
	if (!is_multiline)
	{
		RenderNavHighlight(frame_bb, id);
		RenderFrame(frame_bb.Min, frame_bb.Max, KarmaGui::GetColorU32(KGGuiCol_FrameBg), true, style.FrameRounding);
	}

	const KGVec4 clip_rect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + inner_size.x, frame_bb.Min.y + inner_size.y); // Not using frame_bb.Max because we have adjusted size
	KGVec2 draw_pos = is_multiline ? draw_window->DC.CursorPos : frame_bb.Min + style.FramePadding;
	KGVec2 text_size(0.0f, 0.0f);

	// Set upper limit of single-line InputTextEx() at 2 million characters strings. The current pathological worst case is a long line
	// without any carriage return, which would makes KGFont::RenderText() reserve too many vertices and probably crash. Avoid it altogether.
	// Note that we only use this limit on single-line InputText(), so a pathologically large line on a InputTextMultiline() would still crash.
	const int buf_display_max_length = 2 * 1024 * 1024;
	const char* buf_display = buf_display_from_state ? state->TextA.Data : buf; //-V595
	const char* buf_display_end = NULL; // We have specialized paths below for setting the length
	if (is_displaying_hint)
	{
		buf_display = hint;
		buf_display_end = hint + strlen(hint);
	}

	// Render text. We currently only render selection when the widget is active or while scrolling.
	// FIXME: We could remove the '&& render_cursor' to keep rendering selection when inactive.
	if (render_cursor || render_selection)
	{
		KR_CORE_ASSERT(state != NULL, "");
		if (!is_displaying_hint)
			buf_display_end = buf_display + state->CurLenA;

		// Render text (with cursor and selection)
		// This is going to be messy. We need to:
		// - Display the text (this alone can be more easily clipped)
		// - Handle scrolling, highlight selection, display cursor (those all requires some form of 1d->2d cursor position calculation)
		// - Measure text height (for scrollbar)
		// We are attempting to do most of that in **one main pass** to minimize the computation cost (non-negligible for large amount of text) + 2nd pass for selection rendering (we could merge them by an extra refactoring effort)
		// FIXME: This should occur on buf_display but we'd need to maintain cursor/select_start/select_end for UTF-8.
		const KGWchar* text_begin = state->TextW.Data;
		KGVec2 cursor_offset, select_start_offset;

		{
			// Find lines numbers straddling 'cursor' (slot 0) and 'select_start' (slot 1) positions.
			const KGWchar* searches_input_ptr[2] = { NULL, NULL };
			int searches_result_line_no[2] = { -1000, -1000 };
			int searches_remaining = 0;
			if (render_cursor)
			{
				searches_input_ptr[0] = text_begin + state->Stb.cursor;
				searches_result_line_no[0] = -1;
				searches_remaining++;
			}
			if (render_selection)
			{
				searches_input_ptr[1] = text_begin + KGMin(state->Stb.select_start, state->Stb.select_end);
				searches_result_line_no[1] = -1;
				searches_remaining++;
			}

			// Iterate all lines to find our line numbers
			// In multi-line mode, we never exit the loop until all lines are counted, so add one extra to the searches_remaining counter.
			searches_remaining += is_multiline ? 1 : 0;
			int line_count = 0;
			//for (const KGWchar* s = text_begin; (s = (const KGWchar*)wcschr((const wchar_t*)s, (wchar_t)'\n')) != NULL; s++)  // FIXME-OPT: Could use this when wchar_t are 16-bit
			for (const KGWchar* s = text_begin; *s != 0; s++)
				if (*s == '\n')
				{
					line_count++;
					if (searches_result_line_no[0] == -1 && s >= searches_input_ptr[0]) { searches_result_line_no[0] = line_count; if (--searches_remaining <= 0) break; }
					if (searches_result_line_no[1] == -1 && s >= searches_input_ptr[1]) { searches_result_line_no[1] = line_count; if (--searches_remaining <= 0) break; }
				}
			line_count++;
			if (searches_result_line_no[0] == -1)
				searches_result_line_no[0] = line_count;
			if (searches_result_line_no[1] == -1)
				searches_result_line_no[1] = line_count;

			// Calculate 2d position by finding the beginning of the line and measuring distance
			cursor_offset.x = InputTextCalcTextSizeW(&g, KGStrbolW(searches_input_ptr[0], text_begin), searches_input_ptr[0]).x;
			cursor_offset.y = searches_result_line_no[0] * g.FontSize;
			if (searches_result_line_no[1] >= 0)
			{
				select_start_offset.x = InputTextCalcTextSizeW(&g, KGStrbolW(searches_input_ptr[1], text_begin), searches_input_ptr[1]).x;
				select_start_offset.y = searches_result_line_no[1] * g.FontSize;
			}

			// Store text height (note that we haven't calculated text width at all, see GitHub issues #383, #1224)
			if (is_multiline)
				text_size = KGVec2(inner_size.x, line_count * g.FontSize);
		}

		// Scroll
		if (render_cursor && state->CursorFollow)
		{
			// Horizontal scroll in chunks of quarter width
			if (!(flags & KGGuiInputTextFlags_NoHorizontalScroll))
			{
				const float scroll_increment_x = inner_size.x * 0.25f;
				const float visible_width = inner_size.x - style.FramePadding.x;
				if (cursor_offset.x < state->ScrollX)
					state->ScrollX = KG_FLOOR(KGMax(0.0f, cursor_offset.x - scroll_increment_x));
				else if (cursor_offset.x - visible_width >= state->ScrollX)
					state->ScrollX = KG_FLOOR(cursor_offset.x - visible_width + scroll_increment_x);
			}
			else
			{
				state->ScrollX = 0.0f;
			}

			// Vertical scroll
			if (is_multiline)
			{
				// Test if cursor is vertically visible
				if (cursor_offset.y - g.FontSize < scroll_y)
					scroll_y = KGMax(0.0f, cursor_offset.y - g.FontSize);
				else if (cursor_offset.y - (inner_size.y - style.FramePadding.y * 2.0f) >= scroll_y)
					scroll_y = cursor_offset.y - inner_size.y + style.FramePadding.y * 2.0f;
				const float scroll_max_y = KGMax((text_size.y + style.FramePadding.y * 2.0f) - inner_size.y, 0.0f);
				scroll_y = KGClamp(scroll_y, 0.0f, scroll_max_y);
				draw_pos.y += (draw_window->Scroll.y - scroll_y);   // Manipulate cursor pos immediately avoid a frame of lag
				draw_window->Scroll.y = scroll_y;
			}

			state->CursorFollow = false;
		}

		// Draw selection
		const KGVec2 draw_scroll = KGVec2(state->ScrollX, 0.0f);
		if (render_selection)
		{
			const KGWchar* text_selected_begin = text_begin + KGMin(state->Stb.select_start, state->Stb.select_end);
			const KGWchar* text_selected_end = text_begin + KGMax(state->Stb.select_start, state->Stb.select_end);

			KGU32 bg_color = KarmaGui::GetColorU32(KGGuiCol_TextSelectedBg, render_cursor ? 1.0f : 0.6f); // FIXME: current code flow mandate that render_cursor is always true here, we are leaving the transparent one for tests.
			float bg_offy_up = is_multiline ? 0.0f : -1.0f;    // FIXME: those offsets should be part of the style? they don't play so well with multi-line selection.
			float bg_offy_dn = is_multiline ? 0.0f : 2.0f;
			KGVec2 rect_pos = draw_pos + select_start_offset - draw_scroll;
			for (const KGWchar* p = text_selected_begin; p < text_selected_end; )
			{
				if (rect_pos.y > clip_rect.w + g.FontSize)
					break;
				if (rect_pos.y < clip_rect.y)
				{
					//p = (const KGWchar*)wmemchr((const wchar_t*)p, '\n', text_selected_end - p);  // FIXME-OPT: Could use this when wchar_t are 16-bit
					//p = p ? p + 1 : text_selected_end;
					while (p < text_selected_end)
						if (*p++ == '\n')
							break;
				}
				else
				{
					KGVec2 rect_size = InputTextCalcTextSizeW(&g, p, text_selected_end, &p, NULL, true);
					if (rect_size.x <= 0.0f) rect_size.x = KG_FLOOR(g.Font->GetCharAdvance((KGWchar)' ') * 0.50f); // So we can see selected empty lines
					KGRect rect(rect_pos + KGVec2(0.0f, bg_offy_up - g.FontSize), rect_pos + KGVec2(rect_size.x, bg_offy_dn));
					rect.ClipWith(clip_rect);
					if (rect.Overlaps(clip_rect))
						draw_window->DrawList->AddRectFilled(rect.Min, rect.Max, bg_color);
				}
				rect_pos.x = draw_pos.x - draw_scroll.x;
				rect_pos.y += g.FontSize;
			}
		}

		// We test for 'buf_display_max_length' as a way to avoid some pathological cases (e.g. single-line 1 MB string) which would make KGDrawList crash.
		if (is_multiline || (buf_display_end - buf_display) < buf_display_max_length)
		{
			KGU32 col = KarmaGui::GetColorU32(is_displaying_hint ? KGGuiCol_TextDisabled : KGGuiCol_Text);
			draw_window->DrawList->AddText(g.Font, g.FontSize, draw_pos - draw_scroll, col, buf_display, buf_display_end, 0.0f, is_multiline ? NULL : &clip_rect);
		}

		// Draw blinking cursor
		if (render_cursor)
		{
			state->CursorAnim += io.DeltaTime;
			bool cursor_is_visible = (!g.IO.ConfigInputTextCursorBlink) || (state->CursorAnim <= 0.0f) || KGFmod(state->CursorAnim, 1.20f) <= 0.80f;
			KGVec2 cursor_screen_pos = KGFloor(draw_pos + cursor_offset - draw_scroll);
			KGRect cursor_screen_rect(cursor_screen_pos.x, cursor_screen_pos.y - g.FontSize + 0.5f, cursor_screen_pos.x + 1.0f, cursor_screen_pos.y - 1.5f);
			if (cursor_is_visible && cursor_screen_rect.Overlaps(clip_rect))
				draw_window->DrawList->AddLine(cursor_screen_rect.Min, cursor_screen_rect.GetBL(), KarmaGui::GetColorU32(KGGuiCol_Text));

			// Notify OS of text input position for advanced IME (-1 x offset so that Windows IME can cover our cursor. Bit of an extra nicety.)
			if (!is_readonly)
			{
				g.PlatformImeData.WantVisible = true;
				g.PlatformImeData.InputPos = KGVec2(cursor_screen_pos.x - 1.0f, cursor_screen_pos.y - g.FontSize);
				g.PlatformImeData.InputLineHeight = g.FontSize;
				g.PlatformImeViewport = window->Viewport->ID;
			}
		}
	}
	else
	{
		// Render text only (no selection, no cursor)
		if (is_multiline)
			text_size = KGVec2(inner_size.x, InputTextCalcTextLenAndLineCount(buf_display, &buf_display_end) * g.FontSize); // We don't need width
		else if (!is_displaying_hint && g.ActiveId == id)
			buf_display_end = buf_display + state->CurLenA;
		else if (!is_displaying_hint)
			buf_display_end = buf_display + strlen(buf_display);

		if (is_multiline || (buf_display_end - buf_display) < buf_display_max_length)
		{
			KGU32 col = KarmaGui::GetColorU32(is_displaying_hint ? KGGuiCol_TextDisabled : KGGuiCol_Text);
			draw_window->DrawList->AddText(g.Font, g.FontSize, draw_pos, col, buf_display, buf_display_end, 0.0f, is_multiline ? NULL : &clip_rect);
		}
	}

	if (is_password && !is_displaying_hint)
		KarmaGui::PopFont();

	if (is_multiline)
	{
		// For focus requests to work on our multiline we need to ensure our child ItemAdd() call specifies the KGGuiItemFlags_Inputable (ref issue #4761)...
		KarmaGui::Dummy(KGVec2(text_size.x, text_size.y + style.FramePadding.y));
		KGGuiItemFlags backup_item_flags = g.CurrentItemFlags;
		g.CurrentItemFlags |= KGGuiItemFlags_Inputable | KGGuiItemFlags_NoTabStop;
		KarmaGui::EndChild();
		item_data_backup.StatusFlags |= (g.LastItemData.StatusFlags & KGGuiItemStatusFlags_HoveredWindow);
		g.CurrentItemFlags = backup_item_flags;

		// ...and then we need to undo the group overriding last item data, which gets a bit messy as EndGroup() tries to forward scrollbar being active...
		// FIXME: This quite messy/tricky, should attempt to get rid of the child window.
		KarmaGui::EndGroup();
		if (g.LastItemData.ID == 0)
		{
			g.LastItemData.ID = id;
			g.LastItemData.InFlags = item_data_backup.InFlags;
			g.LastItemData.StatusFlags = item_data_backup.StatusFlags;
		}
	}

	// Log as text
	if (g.LogEnabled && (!is_password || is_displaying_hint))
	{
		LogSetNextTextDecoration("{", "}");
		LogRenderedText(&draw_pos, buf_display, buf_display_end);
	}

	if (label_size.x > 0)
		RenderText(KGVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

	if (value_changed && !(flags & KGGuiInputTextFlags_NoMarkEdited))
		MarkItemEdited(id);

	KARMAGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
	if ((flags & KGGuiInputTextFlags_EnterReturnsTrue) != 0)
		return validated;
	else
		return value_changed;
}

KGGuiInputTextState* Karma::KarmaGuiInternal::GetInputTextState(KGGuiID id)
{
	KarmaGuiContext& g = *GKarmaGui;
	return (id != 0 && g.InputTextState.ID == id) ? &g.InputTextState : NULL;
} // Get input text state if active

void Karma::KarmaGuiInternal::DebugNodeInputTextState(KGGuiInputTextState* state)
{
#ifndef KARMAGUI_DISABLE_DEBUG_TOOLS
	KarmaGuiContext& g = *GKarmaGui;
	KGStb::STB_TexteditState* stb_state = &state->Stb;
	KGStb::StbUndoState* undo_state = &stb_state->undostate;
	KarmaGui::Text("ID: 0x%08X, ActiveID: 0x%08X", state->ID, g.ActiveId);
	DebugLocateItemOnHover(state->ID);
	KarmaGui::Text("CurLenW: %d, CurLenA: %d, Cursor: %d, Selection: %d..%d", state->CurLenA, state->CurLenW, stb_state->cursor, stb_state->select_start, stb_state->select_end);
	KarmaGui::Text("undo_point: %d, redo_point: %d, undo_char_point: %d, redo_char_point: %d", undo_state->undo_point, undo_state->redo_point, undo_state->undo_char_point, undo_state->redo_char_point);
	if (KarmaGui::BeginChild("undopoints", KGVec2(0.0f, KarmaGui::GetTextLineHeight() * 15), true)) // Visualize undo state
	{
		KarmaGui::PushStyleVar(KGGuiStyleVar_ItemSpacing, KGVec2(0, 0));
		for (int n = 0; n < STB_TEXTEDIT_UNDOSTATECOUNT; n++)
		{
			KGStb::StbUndoRecord* undo_rec = &undo_state->undo_rec[n];
			const char undo_rec_type = (n < undo_state->undo_point) ? 'u' : (n >= undo_state->redo_point) ? 'r' : ' ';
			if (undo_rec_type == ' ')
				KarmaGui::BeginDisabled();
			char buf[64] = "";
			if (undo_rec_type != ' ' && undo_rec->char_storage != -1)
				KGTextStrToUtf8(buf, KG_ARRAYSIZE(buf), undo_state->undo_char + undo_rec->char_storage, undo_state->undo_char + undo_rec->char_storage + undo_rec->insert_length);
			KarmaGui::Text("%c [%02d] where %03d, insert %03d, delete %03d, char_storage %03d \"%s\"",
				undo_rec_type, n, undo_rec->where, undo_rec->insert_length, undo_rec->delete_length, undo_rec->char_storage, buf);
			if (undo_rec_type == ' ')
				KarmaGui::EndDisabled();
		}
		KarmaGui::PopStyleVar();
	}
	KarmaGui::EndChild();
#else
	KG_UNUSED(state);
#endif
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: ColorEdit, ColorPicker, ColorButton, etc.
//-------------------------------------------------------------------------
// - ColorEdit3()
// - ColorEdit4()
// - ColorPicker3()
// - RenderColorRectWithAlphaCheckerboard() [Internal]
// - ColorPicker4()
// - ColorButton()
// - SetColorEditOptions()
// - ColorTooltip() [Internal]
// - ColorEditOptionsPopup() [Internal]
// - ColorPickerOptionsPopup() [Internal]
//-------------------------------------------------------------------------

bool Karma::KarmaGui::ColorEdit3(const char* label, float col[3], KarmaGuiColorEditFlags flags)
{
	return ColorEdit4(label, col, flags | KGGuiColorEditFlags_NoAlpha);
}

// ColorEdit supports RGB and HSV inputs. In case of RGB input resulting color may have undefined hue and/or saturation.
// Since widget displays both RGB and HSV values we must preserve hue and saturation to prevent these values resetting.
static void ColorEditRestoreHS(const float* col, float* H, float* S, float* V)
{
	// This check is optional. Suppose we have two color widgets side by side, both widgets display different colors, but both colors have hue and/or saturation undefined.
	// With color check: hue/saturation is preserved in one widget. Editing color in one widget would reset hue/saturation in another one.
	// Without color check: common hue/saturation would be displayed in all widgets that have hue/saturation undefined.
	// g.ColorEditLastColor is stored as KGU32 RGB value: this essentially gives us color equality check with reduced precision.
	// Tiny external color changes would not be detected and this check would still pass. This is OK, since we only restore hue/saturation _only_ if they are undefined,
	// therefore this change flipping hue/saturation from undefined to a very tiny value would still be represented in color picker.
	KarmaGuiContext& g = *Karma::GKarmaGui;
	if (g.ColorEditLastColor != Karma::KarmaGui::ColorConvertFloat4ToU32(KGVec4(col[0], col[1], col[2], 0)))
		return;

	// When S == 0, H is undefined.
	// When H == 1 it wraps around to 0.
	if (*S == 0.0f || (*H == 0.0f && g.ColorEditLastHue == 1))
		*H = g.ColorEditLastHue;

	// When V == 0, S is undefined.
	if (*V == 0.0f)
		*S = g.ColorEditLastSat;
}

// Edit colors components (each component in 0.0f..1.0f range).
// See enum KGGuiColorEditFlags_ for available options. e.g. Only access 3 floats if KGGuiColorEditFlags_NoAlpha flag is set.
// With typical options: Left-click on color square to open color picker. Right-click to open option menu. CTRL-Click over input fields to edit them and TAB to go to next item.
bool Karma::KarmaGui::ColorEdit4(const char* label, float col[4], KarmaGuiColorEditFlags flags)
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	KarmaGuiContext& g = *GKarmaGui;
	const KarmaGuiStyle& style = g.Style;
	const float square_sz = GetFrameHeight();
	const float w_full = CalcItemWidth();
	const float w_button = (flags & KGGuiColorEditFlags_NoSmallPreview) ? 0.0f : (square_sz + style.ItemInnerSpacing.x);
	const float w_inputs = w_full - w_button;
	const char* label_display_end = KarmaGuiInternal::FindRenderedTextEnd(label);
	g.NextItemData.ClearFlags();

	BeginGroup();
	PushID(label);

	// If we're not showing any slider there's no point in doing any HSV conversions
	const KarmaGuiColorEditFlags flags_untouched = flags;
	if (flags & KGGuiColorEditFlags_NoInputs)
		flags = (flags & (~KGGuiColorEditFlags_DisplayMask_)) | KGGuiColorEditFlags_DisplayRGB | KGGuiColorEditFlags_NoOptions;

	// Context menu: display and modify options (before defaults are applied)
	if (!(flags & KGGuiColorEditFlags_NoOptions))
		KarmaGuiInternal::ColorEditOptionsPopup(col, flags);

	// Read stored options
	if (!(flags & KGGuiColorEditFlags_DisplayMask_))
		flags |= (g.ColorEditOptions & KGGuiColorEditFlags_DisplayMask_);
	if (!(flags & KGGuiColorEditFlags_DataTypeMask_))
		flags |= (g.ColorEditOptions & KGGuiColorEditFlags_DataTypeMask_);
	if (!(flags & KGGuiColorEditFlags_PickerMask_))
		flags |= (g.ColorEditOptions & KGGuiColorEditFlags_PickerMask_);
	if (!(flags & KGGuiColorEditFlags_InputMask_))
		flags |= (g.ColorEditOptions & KGGuiColorEditFlags_InputMask_);
	flags |= (g.ColorEditOptions & ~(KGGuiColorEditFlags_DisplayMask_ | KGGuiColorEditFlags_DataTypeMask_ | KGGuiColorEditFlags_PickerMask_ | KGGuiColorEditFlags_InputMask_));
	KR_CORE_ASSERT(KGIsPowerOfTwo(flags & KGGuiColorEditFlags_DisplayMask_), ""); // Check that only 1 is selected
	KR_CORE_ASSERT(KGIsPowerOfTwo(flags & KGGuiColorEditFlags_InputMask_), "");   // Check that only 1 is selected

	const bool alpha = (flags & KGGuiColorEditFlags_NoAlpha) == 0;
	const bool hdr = (flags & KGGuiColorEditFlags_HDR) != 0;
	const int components = alpha ? 4 : 3;

	// Convert to the formats we need
	float f[4] = { col[0], col[1], col[2], alpha ? col[3] : 1.0f };
	if ((flags & KGGuiColorEditFlags_InputHSV) && (flags & KGGuiColorEditFlags_DisplayRGB))
		ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
	else if ((flags & KGGuiColorEditFlags_InputRGB) && (flags & KGGuiColorEditFlags_DisplayHSV))
	{
		// Hue is lost when converting from greyscale rgb (saturation=0). Restore it.
		ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
		ColorEditRestoreHS(col, &f[0], &f[1], &f[2]);
	}
	int i[4] = { KG_F32_TO_INT8_UNBOUND(f[0]), KG_F32_TO_INT8_UNBOUND(f[1]), KG_F32_TO_INT8_UNBOUND(f[2]), KG_F32_TO_INT8_UNBOUND(f[3]) };

	bool value_changed = false;
	bool value_changed_as_float = false;

	const KGVec2 pos = window->DC.CursorPos;
	const float inputs_offset_x = (style.ColorButtonPosition == KGGuiDir_Left) ? w_button : 0.0f;
	window->DC.CursorPos.x = pos.x + inputs_offset_x;

	if ((flags & (KGGuiColorEditFlags_DisplayRGB | KGGuiColorEditFlags_DisplayHSV)) != 0 && (flags & KGGuiColorEditFlags_NoInputs) == 0)
	{
		// RGB/HSV 0..255 Sliders
		const float w_item_one = KGMax(1.0f, KG_FLOOR((w_inputs - (style.ItemInnerSpacing.x) * (components - 1)) / (float)components));
		const float w_item_last = KGMax(1.0f, KG_FLOOR(w_inputs - (w_item_one + style.ItemInnerSpacing.x) * (components - 1)));

		const bool hide_prefix = (w_item_one <= CalcTextSize((flags & KGGuiColorEditFlags_Float) ? "M:0.000" : "M:000").x);
		static const char* ids[4] = { "##X", "##Y", "##Z", "##W" };
		static const char* fmt_table_int[3][4] =
		{
			{   "%3d",   "%3d",   "%3d",   "%3d" }, // Short display
			{ "R:%3d", "G:%3d", "B:%3d", "A:%3d" }, // Long display for RGBA
			{ "H:%3d", "S:%3d", "V:%3d", "A:%3d" }  // Long display for HSVA
		};
		static const char* fmt_table_float[3][4] =
		{
			{   "%0.3f",   "%0.3f",   "%0.3f",   "%0.3f" }, // Short display
			{ "R:%0.3f", "G:%0.3f", "B:%0.3f", "A:%0.3f" }, // Long display for RGBA
			{ "H:%0.3f", "S:%0.3f", "V:%0.3f", "A:%0.3f" }  // Long display for HSVA
		};
		const int fmt_idx = hide_prefix ? 0 : (flags & KGGuiColorEditFlags_DisplayHSV) ? 2 : 1;

		for (int n = 0; n < components; n++)
		{
			if (n > 0)
				SameLine(0, style.ItemInnerSpacing.x);
			SetNextItemWidth((n + 1 < components) ? w_item_one : w_item_last);

			// FIXME: When KGGuiColorEditFlags_HDR flag is passed HS values snap in weird ways when SV values go below 0.
			if (flags & KGGuiColorEditFlags_Float)
			{
				value_changed |= DragFloat(ids[n], &f[n], 1.0f / 255.0f, 0.0f, hdr ? 0.0f : 1.0f, fmt_table_float[fmt_idx][n]);
				value_changed_as_float |= value_changed;
			}
			else
			{
				value_changed |= DragInt(ids[n], &i[n], 1.0f, 0, hdr ? 0 : 255, fmt_table_int[fmt_idx][n]);
			}
			if (!(flags & KGGuiColorEditFlags_NoOptions))
				OpenPopupOnItemClick("context", KGGuiPopupFlags_MouseButtonRight);
		}
	}
	else if ((flags & KGGuiColorEditFlags_DisplayHex) != 0 && (flags & KGGuiColorEditFlags_NoInputs) == 0)
	{
		// RGB Hexadecimal Input
		char buf[64];
		if (alpha)
			KGFormatString(buf, KG_ARRAYSIZE(buf), "#%02X%02X%02X%02X", KGClamp(i[0], 0, 255), KGClamp(i[1], 0, 255), KGClamp(i[2], 0, 255), KGClamp(i[3], 0, 255));
		else
			KGFormatString(buf, KG_ARRAYSIZE(buf), "#%02X%02X%02X", KGClamp(i[0], 0, 255), KGClamp(i[1], 0, 255), KGClamp(i[2], 0, 255));
		SetNextItemWidth(w_inputs);
		if (InputText("##Text", buf, KG_ARRAYSIZE(buf), KGGuiInputTextFlags_CharsHexadecimal | KGGuiInputTextFlags_CharsUppercase))
		{
			value_changed = true;
			char* p = buf;
			while (*p == '#' || KGCharIsBlankA(*p))
				p++;
			i[0] = i[1] = i[2] = 0;
			i[3] = 0xFF; // alpha default to 255 is not parsed by scanf (e.g. inputting #FFFFFF omitting alpha)
			int r;
			if (alpha)
				r = sscanf(p, "%02X%02X%02X%02X", (unsigned int*)&i[0], (unsigned int*)&i[1], (unsigned int*)&i[2], (unsigned int*)&i[3]); // Treat at unsigned (%X is unsigned)
			else
				r = sscanf(p, "%02X%02X%02X", (unsigned int*)&i[0], (unsigned int*)&i[1], (unsigned int*)&i[2]);
			KG_UNUSED(r); // Fixes C6031: Return value ignored: 'sscanf'.
		}
		if (!(flags & KGGuiColorEditFlags_NoOptions))
			OpenPopupOnItemClick("context", KGGuiPopupFlags_MouseButtonRight);
	}

	KGGuiWindow* picker_active_window = NULL;
	if (!(flags & KGGuiColorEditFlags_NoSmallPreview))
	{
		const float button_offset_x = ((flags & KGGuiColorEditFlags_NoInputs) || (style.ColorButtonPosition == KGGuiDir_Left)) ? 0.0f : w_inputs + style.ItemInnerSpacing.x;
		window->DC.CursorPos = KGVec2(pos.x + button_offset_x, pos.y);

		const KGVec4 col_v4(col[0], col[1], col[2], alpha ? col[3] : 1.0f);
		if (ColorButton("##ColorButton", col_v4, flags))
		{
			if (!(flags & KGGuiColorEditFlags_NoPicker))
			{
				// Store current color and open a picker
				g.ColorPickerRef = col_v4;
				OpenPopup("picker");
				SetNextWindowPos(g.LastItemData.Rect.GetBL() + KGVec2(0.0f, style.ItemSpacing.y));
			}
		}
		if (!(flags & KGGuiColorEditFlags_NoOptions))
			OpenPopupOnItemClick("context", KGGuiPopupFlags_MouseButtonRight);

		if (BeginPopup("picker"))
		{
			if (g.CurrentWindow->BeginCount == 1)
			{
				picker_active_window = g.CurrentWindow;
				if (label != label_display_end)
				{
					KarmaGuiInternal::TextEx(label, label_display_end);
					Spacing();
				}
				KarmaGuiColorEditFlags picker_flags_to_forward = KGGuiColorEditFlags_DataTypeMask_ | KGGuiColorEditFlags_PickerMask_ | KGGuiColorEditFlags_InputMask_ | KGGuiColorEditFlags_HDR | KGGuiColorEditFlags_NoAlpha | KGGuiColorEditFlags_AlphaBar;
				KarmaGuiColorEditFlags picker_flags = (flags_untouched & picker_flags_to_forward) | KGGuiColorEditFlags_DisplayMask_ | KGGuiColorEditFlags_NoLabel | KGGuiColorEditFlags_AlphaPreviewHalf;
				SetNextItemWidth(square_sz * 12.0f); // Use 256 + bar sizes?
				value_changed |= ColorPicker4("##picker", col, picker_flags, &g.ColorPickerRef.x);
			}
			EndPopup();
		}
	}

	if (label != label_display_end && !(flags & KGGuiColorEditFlags_NoLabel))
	{
		// Position not necessarily next to last submitted button (e.g. if style.ColorButtonPosition == KGGuiDir_Left),
		// but we need to use SameLine() to setup baseline correctly. Might want to refactor SameLine() to simplify this.
		SameLine(0.0f, style.ItemInnerSpacing.x);
		window->DC.CursorPos.x = pos.x + ((flags & KGGuiColorEditFlags_NoInputs) ? w_button : w_full + style.ItemInnerSpacing.x);
		KarmaGuiInternal::TextEx(label, label_display_end);
	}

	// Convert back
	if (value_changed && picker_active_window == NULL)
	{
		if (!value_changed_as_float)
			for (int n = 0; n < 4; n++)
				f[n] = i[n] / 255.0f;
		if ((flags & KGGuiColorEditFlags_DisplayHSV) && (flags & KGGuiColorEditFlags_InputRGB))
		{
			g.ColorEditLastHue = f[0];
			g.ColorEditLastSat = f[1];
			ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
			g.ColorEditLastColor = ColorConvertFloat4ToU32(KGVec4(f[0], f[1], f[2], 0));
		}
		if ((flags & KGGuiColorEditFlags_DisplayRGB) && (flags & KGGuiColorEditFlags_InputHSV))
			ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);

		col[0] = f[0];
		col[1] = f[1];
		col[2] = f[2];
		if (alpha)
			col[3] = f[3];
	}

	PopID();
	EndGroup();

	// Drag and Drop Target
	// NB: The flag test is merely an optional micro-optimization, BeginDragDropTarget() does the same test.
	if ((g.LastItemData.StatusFlags & KGGuiItemStatusFlags_HoveredRect) && !(flags & KGGuiColorEditFlags_NoDragDrop) && BeginDragDropTarget())
	{
		bool accepted_drag_drop = false;
		if (const KarmaGuiPayload* payload = AcceptDragDropPayload(KARMAGUI_PAYLOAD_TYPE_COLOR_3F))
		{
			memcpy((float*)col, payload->Data, sizeof(float) * 3); // Preserve alpha if any //-V512 //-V1086
			value_changed = accepted_drag_drop = true;
		}
		if (const KarmaGuiPayload* payload = AcceptDragDropPayload(KARMAGUI_PAYLOAD_TYPE_COLOR_4F))
		{
			memcpy((float*)col, payload->Data, sizeof(float) * components);
			value_changed = accepted_drag_drop = true;
		}

		// Drag-drop payloads are always RGB
		if (accepted_drag_drop && (flags & KGGuiColorEditFlags_InputHSV))
			ColorConvertRGBtoHSV(col[0], col[1], col[2], col[0], col[1], col[2]);
		EndDragDropTarget();
	}

	// When picker is being actively used, use its active id so IsItemActive() will function on ColorEdit4().
	if (picker_active_window && g.ActiveId != 0 && g.ActiveIdWindow == picker_active_window)
		g.LastItemData.ID = g.ActiveId;

	if (value_changed && g.LastItemData.ID != 0) // In case of ID collision, the second EndGroup() won't catch g.ActiveId
		KarmaGuiInternal::MarkItemEdited(g.LastItemData.ID);

	return value_changed;
}

bool Karma::KarmaGui::ColorPicker3(const char* label, float col[3], KarmaGuiColorEditFlags flags)
{
	float col4[4] = { col[0], col[1], col[2], 1.0f };
	if (!ColorPicker4(label, col4, flags | KGGuiColorEditFlags_NoAlpha))
		return false;
	col[0] = col4[0]; col[1] = col4[1]; col[2] = col4[2];
	return true;
}

// Helper for ColorPicker4()
static void RenderArrowsForVerticalBar(KGDrawList* draw_list, KGVec2 pos, KGVec2 half_sz, float bar_w, float alpha)
{
	KGU32 alpha8 = KG_F32_TO_INT8_SAT(alpha);
	Karma::KarmaGuiInternal::RenderArrowPointingAt(draw_list, KGVec2(pos.x + half_sz.x + 1, pos.y), KGVec2(half_sz.x + 2, half_sz.y + 1), KGGuiDir_Right, KG_COL32(0, 0, 0, alpha8));
	Karma::KarmaGuiInternal::RenderArrowPointingAt(draw_list, KGVec2(pos.x + half_sz.x, pos.y), half_sz, KGGuiDir_Right, KG_COL32(255, 255, 255, alpha8));
	Karma::KarmaGuiInternal::RenderArrowPointingAt(draw_list, KGVec2(pos.x + bar_w - half_sz.x - 1, pos.y), KGVec2(half_sz.x + 2, half_sz.y + 1), KGGuiDir_Left, KG_COL32(0, 0, 0, alpha8));
	Karma::KarmaGuiInternal::RenderArrowPointingAt(draw_list, KGVec2(pos.x + bar_w - half_sz.x, pos.y), half_sz, KGGuiDir_Left, KG_COL32(255, 255, 255, alpha8));
}

// Note: ColorPicker4() only accesses 3 floats if KGGuiColorEditFlags_NoAlpha flag is set.
// (In C++ the 'float col[4]' notation for a function argument is equivalent to 'float* col', we only specify a size to facilitate understanding of the code.)
// FIXME: we adjust the big color square height based on item width, which may cause a flickering feedback loop (if automatic height makes a vertical scrollbar appears, affecting automatic width..)
// FIXME: this is trying to be aware of style.Alpha but not fully correct. Also, the color wheel will have overlapping glitches with (style.Alpha < 1.0)
bool Karma::KarmaGui::ColorPicker4(const char* label, float col[4], KarmaGuiColorEditFlags flags, const float* ref_col)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	KGDrawList* draw_list = window->DrawList;
	KarmaGuiStyle& style = g.Style;
	KarmaGuiIO& io = g.IO;

	const float width = CalcItemWidth();
	g.NextItemData.ClearFlags();

	PushID(label);
	BeginGroup();

	if (!(flags & KGGuiColorEditFlags_NoSidePreview))
		flags |= KGGuiColorEditFlags_NoSmallPreview;

	// Context menu: display and store options.
	if (!(flags & KGGuiColorEditFlags_NoOptions))
		KarmaGuiInternal::ColorPickerOptionsPopup(col, flags);

	// Read stored options
	if (!(flags & KGGuiColorEditFlags_PickerMask_))
		flags |= ((g.ColorEditOptions & KGGuiColorEditFlags_PickerMask_) ? g.ColorEditOptions : KGGuiColorEditFlags_DefaultOptions_) & KGGuiColorEditFlags_PickerMask_;
	if (!(flags & KGGuiColorEditFlags_InputMask_))
		flags |= ((g.ColorEditOptions & KGGuiColorEditFlags_InputMask_) ? g.ColorEditOptions : KGGuiColorEditFlags_DefaultOptions_) & KGGuiColorEditFlags_InputMask_;
	KR_CORE_ASSERT(KGIsPowerOfTwo(flags & KGGuiColorEditFlags_PickerMask_), ""); // Check that only 1 is selected
	KR_CORE_ASSERT(KGIsPowerOfTwo(flags & KGGuiColorEditFlags_InputMask_), "");  // Check that only 1 is selected
	if (!(flags & KGGuiColorEditFlags_NoOptions))
		flags |= (g.ColorEditOptions & KGGuiColorEditFlags_AlphaBar);

	// Setup
	int components = (flags & KGGuiColorEditFlags_NoAlpha) ? 3 : 4;
	bool alpha_bar = (flags & KGGuiColorEditFlags_AlphaBar) && !(flags & KGGuiColorEditFlags_NoAlpha);
	KGVec2 picker_pos = window->DC.CursorPos;
	float square_sz = GetFrameHeight();
	float bars_width = square_sz; // Arbitrary smallish width of Hue/Alpha picking bars
	float sv_picker_size = KGMax(bars_width * 1, width - (alpha_bar ? 2 : 1) * (bars_width + style.ItemInnerSpacing.x)); // Saturation/Value picking box
	float bar0_pos_x = picker_pos.x + sv_picker_size + style.ItemInnerSpacing.x;
	float bar1_pos_x = bar0_pos_x + bars_width + style.ItemInnerSpacing.x;
	float bars_triangles_half_sz = KG_FLOOR(bars_width * 0.20f);

	float backup_initial_col[4];
	memcpy(backup_initial_col, col, components * sizeof(float));

	float wheel_thickness = sv_picker_size * 0.08f;
	float wheel_r_outer = sv_picker_size * 0.50f;
	float wheel_r_inner = wheel_r_outer - wheel_thickness;
	KGVec2 wheel_center(picker_pos.x + (sv_picker_size + bars_width) * 0.5f, picker_pos.y + sv_picker_size * 0.5f);

	// Note: the triangle is displayed rotated with triangle_pa pointing to Hue, but most coordinates stays unrotated for logic.
	float triangle_r = wheel_r_inner - (int)(sv_picker_size * 0.027f);
	KGVec2 triangle_pa = KGVec2(triangle_r, 0.0f); // Hue point.
	KGVec2 triangle_pb = KGVec2(triangle_r * -0.5f, triangle_r * -0.866025f); // Black point.
	KGVec2 triangle_pc = KGVec2(triangle_r * -0.5f, triangle_r * +0.866025f); // White point.

	float H = col[0], S = col[1], V = col[2];
	float R = col[0], G = col[1], B = col[2];
	if (flags & KGGuiColorEditFlags_InputRGB)
	{
		// Hue is lost when converting from greyscale rgb (saturation=0). Restore it.
		ColorConvertRGBtoHSV(R, G, B, H, S, V);
		ColorEditRestoreHS(col, &H, &S, &V);
	}
	else if (flags & KGGuiColorEditFlags_InputHSV)
	{
		ColorConvertHSVtoRGB(H, S, V, R, G, B);
	}

	bool value_changed = false, value_changed_h = false, value_changed_sv = false;

	KarmaGuiInternal::PushItemFlag(KGGuiItemFlags_NoNav, true);
	if (flags & KGGuiColorEditFlags_PickerHueWheel)
	{
		// Hue wheel + SV triangle logic
		InvisibleButton("hsv", KGVec2(sv_picker_size + style.ItemInnerSpacing.x + bars_width, sv_picker_size));
		if (IsItemActive())
		{
			KGVec2 initial_off = g.IO.MouseClickedPos[0] - wheel_center;
			KGVec2 current_off = g.IO.MousePos - wheel_center;
			float initial_dist2 = KGLengthSqr(initial_off);
			if (initial_dist2 >= (wheel_r_inner - 1) * (wheel_r_inner - 1) && initial_dist2 <= (wheel_r_outer + 1) * (wheel_r_outer + 1))
			{
				// Interactive with Hue wheel
				H = KGAtan2(current_off.y, current_off.x) / KG_PI * 0.5f;
				if (H < 0.0f)
					H += 1.0f;
				value_changed = value_changed_h = true;
			}
			float cos_hue_angle = KGCos(-H * 2.0f * KG_PI);
			float sin_hue_angle = KGSin(-H * 2.0f * KG_PI);
			if (KGTriangleContainsPoint(triangle_pa, triangle_pb, triangle_pc, KGRotate(initial_off, cos_hue_angle, sin_hue_angle)))
			{
				// Interacting with SV triangle
				KGVec2 current_off_unrotated = KGRotate(current_off, cos_hue_angle, sin_hue_angle);
				if (!KGTriangleContainsPoint(triangle_pa, triangle_pb, triangle_pc, current_off_unrotated))
					current_off_unrotated = KGTriangleClosestPoint(triangle_pa, triangle_pb, triangle_pc, current_off_unrotated);
				float uu, vv, ww;
				KGTriangleBarycentricCoords(triangle_pa, triangle_pb, triangle_pc, current_off_unrotated, uu, vv, ww);
				V = KGClamp(1.0f - vv, 0.0001f, 1.0f);
				S = KGClamp(uu / V, 0.0001f, 1.0f);
				value_changed = value_changed_sv = true;
			}
		}
		if (!(flags & KGGuiColorEditFlags_NoOptions))
			OpenPopupOnItemClick("context", KGGuiPopupFlags_MouseButtonRight);
	}
	else if (flags & KGGuiColorEditFlags_PickerHueBar)
	{
		// SV rectangle logic
		InvisibleButton("sv", KGVec2(sv_picker_size, sv_picker_size));
		if (IsItemActive())
		{
			S = KGSaturate((io.MousePos.x - picker_pos.x) / (sv_picker_size - 1));
			V = 1.0f - KGSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1));

			// Greatly reduces hue jitter and reset to 0 when hue == 255 and color is rapidly modified using SV square.
			if (g.ColorEditLastColor == ColorConvertFloat4ToU32(KGVec4(col[0], col[1], col[2], 0)))
				H = g.ColorEditLastHue;
			value_changed = value_changed_sv = true;
		}
		if (!(flags & KGGuiColorEditFlags_NoOptions))
			OpenPopupOnItemClick("context", KGGuiPopupFlags_MouseButtonRight);

		// Hue bar logic
		SetCursorScreenPos(KGVec2(bar0_pos_x, picker_pos.y));
		InvisibleButton("hue", KGVec2(bars_width, sv_picker_size));
		if (IsItemActive())
		{
			H = KGSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1));
			value_changed = value_changed_h = true;
		}
	}

	// Alpha bar logic
	if (alpha_bar)
	{
		SetCursorScreenPos(KGVec2(bar1_pos_x, picker_pos.y));
		InvisibleButton("alpha", KGVec2(bars_width, sv_picker_size));
		if (IsItemActive())
		{
			col[3] = 1.0f - KGSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1));
			value_changed = true;
		}
	}
	KarmaGuiInternal::PopItemFlag(); // KGGuiItemFlags_NoNav

	if (!(flags & KGGuiColorEditFlags_NoSidePreview))
	{
		SameLine(0, style.ItemInnerSpacing.x);
		BeginGroup();
	}

	if (!(flags & KGGuiColorEditFlags_NoLabel))
	{
		const char* label_display_end = KarmaGuiInternal::FindRenderedTextEnd(label);
		if (label != label_display_end)
		{
			if ((flags & KGGuiColorEditFlags_NoSidePreview))
				SameLine(0, style.ItemInnerSpacing.x);
			KarmaGuiInternal::TextEx(label, label_display_end);
		}
	}

	if (!(flags & KGGuiColorEditFlags_NoSidePreview))
	{
		KarmaGuiInternal::PushItemFlag(KGGuiItemFlags_NoNavDefaultFocus, true);
		KGVec4 col_v4(col[0], col[1], col[2], (flags & KGGuiColorEditFlags_NoAlpha) ? 1.0f : col[3]);
		if ((flags & KGGuiColorEditFlags_NoLabel))
			Text("Current");

		KarmaGuiColorEditFlags sub_flags_to_forward = KGGuiColorEditFlags_InputMask_ | KGGuiColorEditFlags_HDR | KGGuiColorEditFlags_AlphaPreview | KGGuiColorEditFlags_AlphaPreviewHalf | KGGuiColorEditFlags_NoTooltip;
		ColorButton("##current", col_v4, (flags & sub_flags_to_forward), KGVec2(square_sz * 3, square_sz * 2));
		if (ref_col != NULL)
		{
			Text("Original");
			KGVec4 ref_col_v4(ref_col[0], ref_col[1], ref_col[2], (flags & KGGuiColorEditFlags_NoAlpha) ? 1.0f : ref_col[3]);
			if (ColorButton("##original", ref_col_v4, (flags & sub_flags_to_forward), KGVec2(square_sz * 3, square_sz * 2)))
			{
				memcpy(col, ref_col, components * sizeof(float));
				value_changed = true;
			}
		}
		KarmaGuiInternal::PopItemFlag();
		EndGroup();
	}

	// Convert back color to RGB
	if (value_changed_h || value_changed_sv)
	{
		if (flags & KGGuiColorEditFlags_InputRGB)
		{
			ColorConvertHSVtoRGB(H, S, V, col[0], col[1], col[2]);
			g.ColorEditLastHue = H;
			g.ColorEditLastSat = S;
			g.ColorEditLastColor = ColorConvertFloat4ToU32(KGVec4(col[0], col[1], col[2], 0));
		}
		else if (flags & KGGuiColorEditFlags_InputHSV)
		{
			col[0] = H;
			col[1] = S;
			col[2] = V;
		}
	}

	// R,G,B and H,S,V slider color editor
	bool value_changed_fix_hue_wrap = false;
	if ((flags & KGGuiColorEditFlags_NoInputs) == 0)
	{
		PushItemWidth((alpha_bar ? bar1_pos_x : bar0_pos_x) + bars_width - picker_pos.x);
		KarmaGuiColorEditFlags sub_flags_to_forward = KGGuiColorEditFlags_DataTypeMask_ | KGGuiColorEditFlags_InputMask_ | KGGuiColorEditFlags_HDR | KGGuiColorEditFlags_NoAlpha | KGGuiColorEditFlags_NoOptions | KGGuiColorEditFlags_NoSmallPreview | KGGuiColorEditFlags_AlphaPreview | KGGuiColorEditFlags_AlphaPreviewHalf;
		KarmaGuiColorEditFlags sub_flags = (flags & sub_flags_to_forward) | KGGuiColorEditFlags_NoPicker;
		if (flags & KGGuiColorEditFlags_DisplayRGB || (flags & KGGuiColorEditFlags_DisplayMask_) == 0)
			if (ColorEdit4("##rgb", col, sub_flags | KGGuiColorEditFlags_DisplayRGB))
			{
				// FIXME: Hackily differentiating using the DragInt (ActiveId != 0 && !ActiveIdAllowOverlap) vs. using the InputText or DropTarget.
				// For the later we don't want to run the hue-wrap canceling code. If you are well versed in HSV picker please provide your input! (See #2050)
				value_changed_fix_hue_wrap = (g.ActiveId != 0 && !g.ActiveIdAllowOverlap);
				value_changed = true;
			}
		if (flags & KGGuiColorEditFlags_DisplayHSV || (flags & KGGuiColorEditFlags_DisplayMask_) == 0)
			value_changed |= ColorEdit4("##hsv", col, sub_flags | KGGuiColorEditFlags_DisplayHSV);
		if (flags & KGGuiColorEditFlags_DisplayHex || (flags & KGGuiColorEditFlags_DisplayMask_) == 0)
			value_changed |= ColorEdit4("##hex", col, sub_flags | KGGuiColorEditFlags_DisplayHex);
		PopItemWidth();
	}

	// Try to cancel hue wrap (after ColorEdit4 call), if any
	if (value_changed_fix_hue_wrap && (flags & KGGuiColorEditFlags_InputRGB))
	{
		float new_H, new_S, new_V;
		ColorConvertRGBtoHSV(col[0], col[1], col[2], new_H, new_S, new_V);
		if (new_H <= 0 && H > 0)
		{
			if (new_V <= 0 && V != new_V)
				ColorConvertHSVtoRGB(H, S, new_V <= 0 ? V * 0.5f : new_V, col[0], col[1], col[2]);
			else if (new_S <= 0)
				ColorConvertHSVtoRGB(H, new_S <= 0 ? S * 0.5f : new_S, new_V, col[0], col[1], col[2]);
		}
	}

	if (value_changed)
	{
		if (flags & KGGuiColorEditFlags_InputRGB)
		{
			R = col[0];
			G = col[1];
			B = col[2];
			ColorConvertRGBtoHSV(R, G, B, H, S, V);
			ColorEditRestoreHS(col, &H, &S, &V);   // Fix local Hue as display below will use it immediately.
		}
		else if (flags & KGGuiColorEditFlags_InputHSV)
		{
			H = col[0];
			S = col[1];
			V = col[2];
			ColorConvertHSVtoRGB(H, S, V, R, G, B);
		}
	}

	const int style_alpha8 = KG_F32_TO_INT8_SAT(style.Alpha);
	const KGU32 col_black = KG_COL32(0, 0, 0, style_alpha8);
	const KGU32 col_white = KG_COL32(255, 255, 255, style_alpha8);
	const KGU32 col_midgrey = KG_COL32(128, 128, 128, style_alpha8);
	const KGU32 col_hues[6 + 1] = { KG_COL32(255,0,0,style_alpha8), KG_COL32(255,255,0,style_alpha8), KG_COL32(0,255,0,style_alpha8), KG_COL32(0,255,255,style_alpha8), KG_COL32(0,0,255,style_alpha8), KG_COL32(255,0,255,style_alpha8), KG_COL32(255,0,0,style_alpha8) };

	KGVec4 hue_color_f(1, 1, 1, style.Alpha); ColorConvertHSVtoRGB(H, 1, 1, hue_color_f.x, hue_color_f.y, hue_color_f.z);
	KGU32 hue_color32 = ColorConvertFloat4ToU32(hue_color_f);
	KGU32 user_col32_striped_of_alpha = ColorConvertFloat4ToU32(KGVec4(R, G, B, style.Alpha)); // Important: this is still including the main rendering/style alpha!!

	KGVec2 sv_cursor_pos;

	if (flags & KGGuiColorEditFlags_PickerHueWheel)
	{
		// Render Hue Wheel
		const float aeps = 0.5f / wheel_r_outer; // Half a pixel arc length in radians (2pi cancels out).
		const int segment_per_arc = KGMax(4, (int)wheel_r_outer / 12);
		for (int n = 0; n < 6; n++)
		{
			const float a0 = (n) / 6.0f * 2.0f * KG_PI - aeps;
			const float a1 = (n + 1.0f) / 6.0f * 2.0f * KG_PI + aeps;
			const int vert_start_idx = draw_list->VtxBuffer.Size;
			draw_list->PathArcTo(wheel_center, (wheel_r_inner + wheel_r_outer) * 0.5f, a0, a1, segment_per_arc);
			draw_list->PathStroke(col_white, 0, wheel_thickness);
			const int vert_end_idx = draw_list->VtxBuffer.Size;

			// Paint colors over existing vertices
			KGVec2 gradient_p0(wheel_center.x + KGCos(a0) * wheel_r_inner, wheel_center.y + KGSin(a0) * wheel_r_inner);
			KGVec2 gradient_p1(wheel_center.x + KGCos(a1) * wheel_r_inner, wheel_center.y + KGSin(a1) * wheel_r_inner);
			KarmaGuiInternal::ShadeVertsLinearColorGradientKeepAlpha(draw_list, vert_start_idx, vert_end_idx, gradient_p0, gradient_p1, col_hues[n], col_hues[n + 1]);
		}

		// Render Cursor + preview on Hue Wheel
		float cos_hue_angle = KGCos(H * 2.0f * KG_PI);
		float sin_hue_angle = KGSin(H * 2.0f * KG_PI);
		KGVec2 hue_cursor_pos(wheel_center.x + cos_hue_angle * (wheel_r_inner + wheel_r_outer) * 0.5f, wheel_center.y + sin_hue_angle * (wheel_r_inner + wheel_r_outer) * 0.5f);
		float hue_cursor_rad = value_changed_h ? wheel_thickness * 0.65f : wheel_thickness * 0.55f;
		int hue_cursor_segments = KGClamp((int)(hue_cursor_rad / 1.4f), 9, 32);
		draw_list->AddCircleFilled(hue_cursor_pos, hue_cursor_rad, hue_color32, hue_cursor_segments);
		draw_list->AddCircle(hue_cursor_pos, hue_cursor_rad + 1, col_midgrey, hue_cursor_segments);
		draw_list->AddCircle(hue_cursor_pos, hue_cursor_rad, col_white, hue_cursor_segments);

		// Render SV triangle (rotated according to hue)
		KGVec2 tra = wheel_center + KGRotate(triangle_pa, cos_hue_angle, sin_hue_angle);
		KGVec2 trb = wheel_center + KGRotate(triangle_pb, cos_hue_angle, sin_hue_angle);
		KGVec2 trc = wheel_center + KGRotate(triangle_pc, cos_hue_angle, sin_hue_angle);
		KGVec2 uv_white = GetFontTexUvWhitePixel();
		draw_list->PrimReserve(6, 6);
		draw_list->PrimVtx(tra, uv_white, hue_color32);
		draw_list->PrimVtx(trb, uv_white, hue_color32);
		draw_list->PrimVtx(trc, uv_white, col_white);
		draw_list->PrimVtx(tra, uv_white, 0);
		draw_list->PrimVtx(trb, uv_white, col_black);
		draw_list->PrimVtx(trc, uv_white, 0);
		draw_list->AddTriangle(tra, trb, trc, col_midgrey, 1.5f);
		sv_cursor_pos = KGLerp(KGLerp(trc, tra, KGSaturate(S)), trb, KGSaturate(1 - V));
	}
	else if (flags & KGGuiColorEditFlags_PickerHueBar)
	{
		// Render SV Square
		draw_list->AddRectFilledMultiColor(picker_pos, picker_pos + KGVec2(sv_picker_size, sv_picker_size), col_white, hue_color32, hue_color32, col_white);
		draw_list->AddRectFilledMultiColor(picker_pos, picker_pos + KGVec2(sv_picker_size, sv_picker_size), 0, 0, col_black, col_black);
		KarmaGuiInternal::RenderFrameBorder(picker_pos, picker_pos + KGVec2(sv_picker_size, sv_picker_size), 0.0f);
		sv_cursor_pos.x = KGClamp(KG_ROUND(picker_pos.x + KGSaturate(S) * sv_picker_size), picker_pos.x + 2, picker_pos.x + sv_picker_size - 2); // Sneakily prevent the circle to stick out too much
		sv_cursor_pos.y = KGClamp(KG_ROUND(picker_pos.y + KGSaturate(1 - V) * sv_picker_size), picker_pos.y + 2, picker_pos.y + sv_picker_size - 2);

		// Render Hue Bar
		for (int i = 0; i < 6; ++i)
			draw_list->AddRectFilledMultiColor(KGVec2(bar0_pos_x, picker_pos.y + i * (sv_picker_size / 6)), KGVec2(bar0_pos_x + bars_width, picker_pos.y + (i + 1) * (sv_picker_size / 6)), col_hues[i], col_hues[i], col_hues[i + 1], col_hues[i + 1]);
		float bar0_line_y = KG_ROUND(picker_pos.y + H * sv_picker_size);
		KarmaGuiInternal::RenderFrameBorder(KGVec2(bar0_pos_x, picker_pos.y), KGVec2(bar0_pos_x + bars_width, picker_pos.y + sv_picker_size), 0.0f);
		RenderArrowsForVerticalBar(draw_list, KGVec2(bar0_pos_x - 1, bar0_line_y), KGVec2(bars_triangles_half_sz + 1, bars_triangles_half_sz), bars_width + 2.0f, style.Alpha);
	}

	// Render cursor/preview circle (clamp S/V within 0..1 range because floating points colors may lead HSV values to be out of range)
	float sv_cursor_rad = value_changed_sv ? 10.0f : 6.0f;
	draw_list->AddCircleFilled(sv_cursor_pos, sv_cursor_rad, user_col32_striped_of_alpha, 12);
	draw_list->AddCircle(sv_cursor_pos, sv_cursor_rad + 1, col_midgrey, 12);
	draw_list->AddCircle(sv_cursor_pos, sv_cursor_rad, col_white, 12);

	// Render alpha bar
	if (alpha_bar)
	{
		float alpha = KGSaturate(col[3]);
		KGRect bar1_bb(bar1_pos_x, picker_pos.y, bar1_pos_x + bars_width, picker_pos.y + sv_picker_size);
		KarmaGuiInternal::RenderColorRectWithAlphaCheckerboard(draw_list, bar1_bb.Min, bar1_bb.Max, 0, bar1_bb.GetWidth() / 2.0f, KGVec2(0.0f, 0.0f));
		draw_list->AddRectFilledMultiColor(bar1_bb.Min, bar1_bb.Max, user_col32_striped_of_alpha, user_col32_striped_of_alpha, user_col32_striped_of_alpha & ~KG_COL32_A_MASK, user_col32_striped_of_alpha & ~KG_COL32_A_MASK);
		float bar1_line_y = KG_ROUND(picker_pos.y + (1.0f - alpha) * sv_picker_size);
		KarmaGuiInternal::RenderFrameBorder(bar1_bb.Min, bar1_bb.Max, 0.0f);
		RenderArrowsForVerticalBar(draw_list, KGVec2(bar1_pos_x - 1, bar1_line_y), KGVec2(bars_triangles_half_sz + 1, bars_triangles_half_sz), bars_width + 2.0f, style.Alpha);
	}

	EndGroup();

	if (value_changed && memcmp(backup_initial_col, col, components * sizeof(float)) == 0)
		value_changed = false;
	if (value_changed && g.LastItemData.ID != 0) // In case of ID collision, the second EndGroup() won't catch g.ActiveId
		KarmaGuiInternal::MarkItemEdited(g.LastItemData.ID);

	PopID();

	return value_changed;
}

// A little color square. Return true when clicked.
// FIXME: May want to display/ignore the alpha component in the color display? Yet show it in the tooltip.
// 'desc_id' is not called 'label' because we don't display it next to the button, but only in the tooltip.
// Note that 'col' may be encoded in HSV if KGGuiColorEditFlags_InputHSV is set.
bool Karma::KarmaGui::ColorButton(const char* desc_id, const KGVec4& col, KarmaGuiColorEditFlags flags, const KGVec2& size_arg)
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	KarmaGuiContext& g = *GKarmaGui;
	const KGGuiID id = window->GetID(desc_id);
	const float default_size = GetFrameHeight();
	const KGVec2 size(size_arg.x == 0.0f ? default_size : size_arg.x, size_arg.y == 0.0f ? default_size : size_arg.y);
	const KGRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	KarmaGuiInternal::ItemSize(bb, (size.y >= default_size) ? g.Style.FramePadding.y : 0.0f);
	if (!KarmaGuiInternal::ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = KarmaGuiInternal::ButtonBehavior(bb, id, &hovered, &held);

	if (flags & KGGuiColorEditFlags_NoAlpha)
		flags &= ~(KGGuiColorEditFlags_AlphaPreview | KGGuiColorEditFlags_AlphaPreviewHalf);

	KGVec4 col_rgb = col;
	if (flags & KGGuiColorEditFlags_InputHSV)
		ColorConvertHSVtoRGB(col_rgb.x, col_rgb.y, col_rgb.z, col_rgb.x, col_rgb.y, col_rgb.z);

	KGVec4 col_rgb_without_alpha(col_rgb.x, col_rgb.y, col_rgb.z, 1.0f);
	float grid_step = KGMin(size.x, size.y) / 2.99f;
	float rounding = KGMin(g.Style.FrameRounding, grid_step * 0.5f);
	KGRect bb_inner = bb;
	float off = 0.0f;
	if ((flags & KGGuiColorEditFlags_NoBorder) == 0)
	{
		off = -0.75f; // The border (using Col_FrameBg) tends to look off when color is near-opaque and rounding is enabled. This offset seemed like a good middle ground to reduce those artifacts.
		bb_inner.Expand(off);
	}
	if ((flags & KGGuiColorEditFlags_AlphaPreviewHalf) && col_rgb.w < 1.0f)
	{
		float mid_x = KG_ROUND((bb_inner.Min.x + bb_inner.Max.x) * 0.5f);
		KarmaGuiInternal::RenderColorRectWithAlphaCheckerboard(window->DrawList, KGVec2(bb_inner.Min.x + grid_step, bb_inner.Min.y), bb_inner.Max, GetColorU32(col_rgb), grid_step, KGVec2(-grid_step + off, off), rounding, KGDrawFlags_RoundCornersRight);
		window->DrawList->AddRectFilled(bb_inner.Min, KGVec2(mid_x, bb_inner.Max.y), GetColorU32(col_rgb_without_alpha), rounding, KGDrawFlags_RoundCornersLeft);
	}
	else
	{
		// Because GetColorU32() multiplies by the global style Alpha and we don't want to display a checkerboard if the source code had no alpha
		KGVec4 col_source = (flags & KGGuiColorEditFlags_AlphaPreview) ? col_rgb : col_rgb_without_alpha;
		if (col_source.w < 1.0f)
			KarmaGuiInternal::RenderColorRectWithAlphaCheckerboard(window->DrawList, bb_inner.Min, bb_inner.Max, GetColorU32(col_source), grid_step, KGVec2(off, off), rounding);
		else
			window->DrawList->AddRectFilled(bb_inner.Min, bb_inner.Max, GetColorU32(col_source), rounding);
	}
	KarmaGuiInternal::RenderNavHighlight(bb, id);
	if ((flags & KGGuiColorEditFlags_NoBorder) == 0)
	{
		if (g.Style.FrameBorderSize > 0.0f)
			KarmaGuiInternal::RenderFrameBorder(bb.Min, bb.Max, rounding);
		else
			window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(KGGuiCol_FrameBg), rounding); // Color button are often in need of some sort of border
	}

	// Drag and Drop Source
	// NB: The ActiveId test is merely an optional micro-optimization, BeginDragDropSource() does the same test.
	if (g.ActiveId == id && !(flags & KGGuiColorEditFlags_NoDragDrop) && BeginDragDropSource())
	{
		if (flags & KGGuiColorEditFlags_NoAlpha)
			SetDragDropPayload(KARMAGUI_PAYLOAD_TYPE_COLOR_3F, &col_rgb, sizeof(float) * 3, KGGuiCond_Once);
		else
			SetDragDropPayload(KARMAGUI_PAYLOAD_TYPE_COLOR_4F, &col_rgb, sizeof(float) * 4, KGGuiCond_Once);
		ColorButton(desc_id, col, flags);
		SameLine();
		KarmaGuiInternal::TextEx("Color");
		EndDragDropSource();
	}

	// Tooltip
	if (!(flags & KGGuiColorEditFlags_NoTooltip) && hovered)
		KarmaGuiInternal::ColorTooltip(desc_id, &col.x, flags & (KGGuiColorEditFlags_InputMask_ | KGGuiColorEditFlags_NoAlpha | KGGuiColorEditFlags_AlphaPreview | KGGuiColorEditFlags_AlphaPreviewHalf));

	return pressed;
}

// Initialize/override default color options
void Karma::KarmaGui::SetColorEditOptions(KarmaGuiColorEditFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	if ((flags & KGGuiColorEditFlags_DisplayMask_) == 0)
		flags |= KGGuiColorEditFlags_DefaultOptions_ & KGGuiColorEditFlags_DisplayMask_;
	if ((flags & KGGuiColorEditFlags_DataTypeMask_) == 0)
		flags |= KGGuiColorEditFlags_DefaultOptions_ & KGGuiColorEditFlags_DataTypeMask_;
	if ((flags & KGGuiColorEditFlags_PickerMask_) == 0)
		flags |= KGGuiColorEditFlags_DefaultOptions_ & KGGuiColorEditFlags_PickerMask_;
	if ((flags & KGGuiColorEditFlags_InputMask_) == 0)
		flags |= KGGuiColorEditFlags_DefaultOptions_ & KGGuiColorEditFlags_InputMask_;
	KR_CORE_ASSERT(KGIsPowerOfTwo(flags & KGGuiColorEditFlags_DisplayMask_), "");    // Check only 1 option is selected
	KR_CORE_ASSERT(KGIsPowerOfTwo(flags & KGGuiColorEditFlags_DataTypeMask_), "");   // Check only 1 option is selected
	KR_CORE_ASSERT(KGIsPowerOfTwo(flags & KGGuiColorEditFlags_PickerMask_), "");     // Check only 1 option is selected
	KR_CORE_ASSERT(KGIsPowerOfTwo(flags & KGGuiColorEditFlags_InputMask_), "");      // Check only 1 option is selected
	g.ColorEditOptions = flags;
}

// Note: only access 3 floats if KGGuiColorEditFlags_NoAlpha flag is set.
void Karma::KarmaGuiInternal::ColorTooltip(const char* text, const float* col, KarmaGuiColorEditFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;

	BeginTooltipEx(KGGuiTooltipFlags_OverridePreviousTooltip, KGGuiWindowFlags_None);
	const char* text_end = text ? FindRenderedTextEnd(text, NULL) : text;
	if (text_end > text)
	{
		TextEx(text, text_end);
		KarmaGui::Separator();
	}

	KGVec2 sz(g.FontSize * 3 + g.Style.FramePadding.y * 2, g.FontSize * 3 + g.Style.FramePadding.y * 2);
	KGVec4 cf(col[0], col[1], col[2], (flags & KGGuiColorEditFlags_NoAlpha) ? 1.0f : col[3]);
	int cr = KG_F32_TO_INT8_SAT(col[0]), cg = KG_F32_TO_INT8_SAT(col[1]), cb = KG_F32_TO_INT8_SAT(col[2]), ca = (flags & KGGuiColorEditFlags_NoAlpha) ? 255 : KG_F32_TO_INT8_SAT(col[3]);
	KarmaGui::ColorButton("##preview", cf, (flags & (KGGuiColorEditFlags_InputMask_ | KGGuiColorEditFlags_NoAlpha | KGGuiColorEditFlags_AlphaPreview | KGGuiColorEditFlags_AlphaPreviewHalf)) | KGGuiColorEditFlags_NoTooltip, sz);
	KarmaGui::SameLine();
	if ((flags & KGGuiColorEditFlags_InputRGB) || !(flags & KGGuiColorEditFlags_InputMask_))
	{
		if (flags & KGGuiColorEditFlags_NoAlpha)
			KarmaGui::Text("#%02X%02X%02X\nR: %d, G: %d, B: %d\n(%.3f, %.3f, %.3f)", cr, cg, cb, cr, cg, cb, col[0], col[1], col[2]);
		else
			KarmaGui::Text("#%02X%02X%02X%02X\nR:%d, G:%d, B:%d, A:%d\n(%.3f, %.3f, %.3f, %.3f)", cr, cg, cb, ca, cr, cg, cb, ca, col[0], col[1], col[2], col[3]);
	}
	else if (flags & KGGuiColorEditFlags_InputHSV)
	{
		if (flags & KGGuiColorEditFlags_NoAlpha)
			KarmaGui::Text("H: %.3f, S: %.3f, V: %.3f", col[0], col[1], col[2]);
		else
			KarmaGui::Text("H: %.3f, S: %.3f, V: %.3f, A: %.3f", col[0], col[1], col[2], col[3]);
	}
	KarmaGui::EndTooltip();
}

void Karma::KarmaGuiInternal::ColorEditOptionsPopup(const float* col, KarmaGuiColorEditFlags flags)
{
	bool allow_opt_inputs = !(flags & KGGuiColorEditFlags_DisplayMask_);
	bool allow_opt_datatype = !(flags & KGGuiColorEditFlags_DataTypeMask_);
	if ((!allow_opt_inputs && !allow_opt_datatype) || !KarmaGui::BeginPopup("context"))
		return;
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiColorEditFlags opts = g.ColorEditOptions;
	if (allow_opt_inputs)
	{
		if (KarmaGui::RadioButton("RGB", (opts & KGGuiColorEditFlags_DisplayRGB) != 0)) opts = (opts & ~KGGuiColorEditFlags_DisplayMask_) | KGGuiColorEditFlags_DisplayRGB;
		if (KarmaGui::RadioButton("HSV", (opts & KGGuiColorEditFlags_DisplayHSV) != 0)) opts = (opts & ~KGGuiColorEditFlags_DisplayMask_) | KGGuiColorEditFlags_DisplayHSV;
		if (KarmaGui::RadioButton("Hex", (opts & KGGuiColorEditFlags_DisplayHex) != 0)) opts = (opts & ~KGGuiColorEditFlags_DisplayMask_) | KGGuiColorEditFlags_DisplayHex;
	}
	if (allow_opt_datatype)
	{
		if (allow_opt_inputs) KarmaGui::Separator();
		if (KarmaGui::RadioButton("0..255", (opts & KGGuiColorEditFlags_Uint8) != 0)) opts = (opts & ~KGGuiColorEditFlags_DataTypeMask_) | KGGuiColorEditFlags_Uint8;
		if (KarmaGui::RadioButton("0.00..1.00", (opts & KGGuiColorEditFlags_Float) != 0)) opts = (opts & ~KGGuiColorEditFlags_DataTypeMask_) | KGGuiColorEditFlags_Float;
	}

	if (allow_opt_inputs || allow_opt_datatype)
		KarmaGui::Separator();
	if (KarmaGui::Button("Copy as..", KGVec2(-1, 0)))
		KarmaGui::OpenPopup("Copy");
	if (KarmaGui::BeginPopup("Copy"))
	{
		int cr = KG_F32_TO_INT8_SAT(col[0]), cg = KG_F32_TO_INT8_SAT(col[1]), cb = KG_F32_TO_INT8_SAT(col[2]), ca = (flags & KGGuiColorEditFlags_NoAlpha) ? 255 : KG_F32_TO_INT8_SAT(col[3]);
		char buf[64];
		KGFormatString(buf, KG_ARRAYSIZE(buf), "(%.3ff, %.3ff, %.3ff, %.3ff)", col[0], col[1], col[2], (flags & KGGuiColorEditFlags_NoAlpha) ? 1.0f : col[3]);
		if (KarmaGui::Selectable(buf))
			KarmaGui::SetClipboardText(buf);
		KGFormatString(buf, KG_ARRAYSIZE(buf), "(%d,%d,%d,%d)", cr, cg, cb, ca);
		if (KarmaGui::Selectable(buf))
			KarmaGui::SetClipboardText(buf);
		KGFormatString(buf, KG_ARRAYSIZE(buf), "#%02X%02X%02X", cr, cg, cb);
		if (KarmaGui::Selectable(buf))
			KarmaGui::SetClipboardText(buf);
		if (!(flags & KGGuiColorEditFlags_NoAlpha))
		{
			KGFormatString(buf, KG_ARRAYSIZE(buf), "#%02X%02X%02X%02X", cr, cg, cb, ca);
			if (KarmaGui::Selectable(buf))
				KarmaGui::SetClipboardText(buf);
		}
		KarmaGui::EndPopup();
	}

	g.ColorEditOptions = opts;
	KarmaGui::EndPopup();
}

void Karma::KarmaGuiInternal::ColorPickerOptionsPopup(const float* ref_col, KarmaGuiColorEditFlags flags)
{
	bool allow_opt_picker = !(flags & KGGuiColorEditFlags_PickerMask_);
	bool allow_opt_alpha_bar = !(flags & KGGuiColorEditFlags_NoAlpha) && !(flags & KGGuiColorEditFlags_AlphaBar);
	if ((!allow_opt_picker && !allow_opt_alpha_bar) || !KarmaGui::BeginPopup("context"))
		return;
	KarmaGuiContext& g = *GKarmaGui;
	if (allow_opt_picker)
	{
		KGVec2 picker_size(g.FontSize * 8, KGMax(g.FontSize * 8 - (KarmaGui::GetFrameHeight() + g.Style.ItemInnerSpacing.x), 1.0f)); // FIXME: Picker size copied from main picker function
		KarmaGui::PushItemWidth(picker_size.x);
		for (int picker_type = 0; picker_type < 2; picker_type++)
		{
			// Draw small/thumbnail version of each picker type (over an invisible button for selection)
			if (picker_type > 0) KarmaGui::Separator();
			KarmaGui::PushID(picker_type);
			KarmaGuiColorEditFlags picker_flags = KGGuiColorEditFlags_NoInputs | KGGuiColorEditFlags_NoOptions | KGGuiColorEditFlags_NoLabel | KGGuiColorEditFlags_NoSidePreview | (flags & KGGuiColorEditFlags_NoAlpha);
			if (picker_type == 0) picker_flags |= KGGuiColorEditFlags_PickerHueBar;
			if (picker_type == 1) picker_flags |= KGGuiColorEditFlags_PickerHueWheel;
			KGVec2 backup_pos = KarmaGui::GetCursorScreenPos();
			if (KarmaGui::Selectable("##selectable", false, 0, picker_size)) // By default, Selectable() is closing popup
				g.ColorEditOptions = (g.ColorEditOptions & ~KGGuiColorEditFlags_PickerMask_) | (picker_flags & KGGuiColorEditFlags_PickerMask_);
			KarmaGui::SetCursorScreenPos(backup_pos);
			KGVec4 previewing_ref_col;
			memcpy(&previewing_ref_col, ref_col, sizeof(float) * ((picker_flags & KGGuiColorEditFlags_NoAlpha) ? 3 : 4));
			KarmaGui::ColorPicker4("##previewing_picker", &previewing_ref_col.x, picker_flags);
			KarmaGui::PopID();
		}
		KarmaGui::PopItemWidth();
	}
	if (allow_opt_alpha_bar)
	{
		if (allow_opt_picker) KarmaGui::Separator();
		KarmaGui::CheckboxFlags("Alpha Bar", &g.ColorEditOptions, KGGuiColorEditFlags_AlphaBar);
	}
	KarmaGui::EndPopup();
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: TreeNode, CollapsingHeader, etc.
//-------------------------------------------------------------------------
// - TreeNode()
// - TreeNodeV()
// - TreeNodeEx()
// - TreeNodeExV()
// - TreeNodeBehavior() [Internal]
// - TreePush()
// - TreePop()
// - GetTreeNodeToLabelSpacing()
// - SetNextItemOpen()
// - CollapsingHeader()
//-------------------------------------------------------------------------

bool Karma::KarmaGui::TreeNode(const char* str_id, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	bool is_open = TreeNodeExV(str_id, 0, fmt, args);
	va_end(args);
	return is_open;
}

bool Karma::KarmaGui::TreeNode(const void* ptr_id, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	bool is_open = TreeNodeExV(ptr_id, 0, fmt, args);
	va_end(args);
	return is_open;
}

bool Karma::KarmaGui::TreeNode(const char* label)
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	if (window->SkipItems)
		return false;
	return KarmaGuiInternal::TreeNodeBehavior(window->GetID(label), 0, label, NULL);
}

bool Karma::KarmaGui::TreeNodeV(const char* str_id, const char* fmt, va_list args)
{
	return TreeNodeExV(str_id, 0, fmt, args);
}

bool Karma::KarmaGui::TreeNodeV(const void* ptr_id, const char* fmt, va_list args)
{
	return TreeNodeExV(ptr_id, 0, fmt, args);
}

bool Karma::KarmaGui::TreeNodeEx(const char* label, KarmaGuiTreeNodeFlags flags)
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	return KarmaGuiInternal::TreeNodeBehavior(window->GetID(label), flags, label, NULL);
}

bool Karma::KarmaGui::TreeNodeEx(const char* str_id, KarmaGuiTreeNodeFlags flags, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	bool is_open = TreeNodeExV(str_id, flags, fmt, args);
	va_end(args);
	return is_open;
}

bool Karma::KarmaGui::TreeNodeEx(const void* ptr_id, KarmaGuiTreeNodeFlags flags, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	bool is_open = TreeNodeExV(ptr_id, flags, fmt, args);
	va_end(args);
	return is_open;
}

bool Karma::KarmaGui::TreeNodeExV(const char* str_id, KarmaGuiTreeNodeFlags flags, const char* fmt, va_list args)
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	const char* label, * label_end;
	KGFormatStringToTempBufferV(&label, &label_end, fmt, args);
	return KarmaGuiInternal::TreeNodeBehavior(window->GetID(str_id), flags, label, label_end);
}

bool Karma::KarmaGui::TreeNodeExV(const void* ptr_id, KarmaGuiTreeNodeFlags flags, const char* fmt, va_list args)
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	const char* label, * label_end;
	KGFormatStringToTempBufferV(&label, &label_end, fmt, args);
	return KarmaGuiInternal::TreeNodeBehavior(window->GetID(ptr_id), flags, label, label_end);
}

void Karma::KarmaGuiInternal::TreeNodeSetOpen(KGGuiID id, bool open)
{
	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiStorage* storage = g.CurrentWindow->DC.StateStorage;
	storage->SetInt(id, open ? 1 : 0);
}

bool Karma::KarmaGuiInternal::TreeNodeUpdateNextOpen(KGGuiID id, KarmaGuiTreeNodeFlags flags)
{
	if (flags & KGGuiTreeNodeFlags_Leaf)
		return true;

	// We only write to the tree storage if the user clicks (or explicitly use the SetNextItemOpen function)
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	KarmaGuiStorage* storage = window->DC.StateStorage;

	bool is_open;
	if (g.NextItemData.Flags & KGGuiNextItemDataFlags_HasOpen)
	{
		if (g.NextItemData.OpenCond & KGGuiCond_Always)
		{
			is_open = g.NextItemData.OpenVal;
			TreeNodeSetOpen(id, is_open);
		}
		else
		{
			// We treat KGGuiCond_Once and KGGuiCond_FirstUseEver the same because tree node state are not saved persistently.
			const int stored_value = storage->GetInt(id, -1);
			if (stored_value == -1)
			{
				is_open = g.NextItemData.OpenVal;
				TreeNodeSetOpen(id, is_open);
			}
			else
			{
				is_open = stored_value != 0;
			}
		}
	}
	else
	{
		is_open = storage->GetInt(id, (flags & KGGuiTreeNodeFlags_DefaultOpen) ? 1 : 0) != 0;
	}

	// When logging is enabled, we automatically expand tree nodes (but *NOT* collapsing headers.. seems like sensible behavior).
	// NB- If we are above max depth we still allow manually opened nodes to be logged.
	if (g.LogEnabled && !(flags & KGGuiTreeNodeFlags_NoAutoOpenOnLog) && (window->DC.TreeDepth - g.LogDepthRef) < g.LogDepthToExpand)
		is_open = true;

	return is_open;
}

bool Karma::KarmaGuiInternal::TreeNodeBehavior(KGGuiID id, KarmaGuiTreeNodeFlags flags, const char* label, const char* label_end)
{
	KGGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	KarmaGuiContext& g = *GKarmaGui;
	const KarmaGuiStyle& style = g.Style;
	const bool display_frame = (flags & KGGuiTreeNodeFlags_Framed) != 0;
	const KGVec2 padding = (display_frame || (flags & KGGuiTreeNodeFlags_FramePadding)) ? style.FramePadding : KGVec2(style.FramePadding.x, KGMin(window->DC.CurrLineTextBaseOffset, style.FramePadding.y));

	if (!label_end)
		label_end = FindRenderedTextEnd(label);
	const KGVec2 label_size = KarmaGui::CalcTextSize(label, label_end, false);

	// We vertically grow up to current line height up the typical widget height.
	const float frame_height = KGMax(KGMin(window->DC.CurrLineSize.y, g.FontSize + style.FramePadding.y * 2), label_size.y + padding.y * 2);
	KGRect frame_bb;
	frame_bb.Min.x = (flags & KGGuiTreeNodeFlags_SpanFullWidth) ? window->WorkRect.Min.x : window->DC.CursorPos.x;
	frame_bb.Min.y = window->DC.CursorPos.y;
	frame_bb.Max.x = window->WorkRect.Max.x;
	frame_bb.Max.y = window->DC.CursorPos.y + frame_height;
	if (display_frame)
	{
		// Framed header expand a little outside the default padding, to the edge of InnerClipRect
		// (FIXME: May remove this at some point and make InnerClipRect align with WindowPadding.x instead of WindowPadding.x*0.5f)
		frame_bb.Min.x -= KG_FLOOR(window->WindowPadding.x * 0.5f - 1.0f);
		frame_bb.Max.x += KG_FLOOR(window->WindowPadding.x * 0.5f);
	}

	const float text_offset_x = g.FontSize + (display_frame ? padding.x * 3 : padding.x * 2);           // Collapser arrow width + Spacing
	const float text_offset_y = KGMax(padding.y, window->DC.CurrLineTextBaseOffset);                    // Latch before ItemSize changes it
	const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);  // Include collapser
	KGVec2 text_pos(window->DC.CursorPos.x + text_offset_x, window->DC.CursorPos.y + text_offset_y);
	ItemSize(KGVec2(text_width, frame_height), padding.y);

	// For regular tree nodes, we arbitrary allow to click past 2 worth of ItemSpacing
	KGRect interact_bb = frame_bb;
	if (!display_frame && (flags & (KGGuiTreeNodeFlags_SpanAvailWidth | KGGuiTreeNodeFlags_SpanFullWidth)) == 0)
		interact_bb.Max.x = frame_bb.Min.x + text_width + style.ItemSpacing.x * 2.0f;

	// Store a flag for the current depth to tell if we will allow closing this node when navigating one of its child.
	// For this purpose we essentially compare if g.NavIdIsAlive went from 0 to 1 between TreeNode() and TreePop().
	// This is currently only support 32 level deep and we are fine with (1 << Depth) overflowing into a zero.
	const bool is_leaf = (flags & KGGuiTreeNodeFlags_Leaf) != 0;
	bool is_open = TreeNodeUpdateNextOpen(id, flags);
	if (is_open && !g.NavIdIsAlive && (flags & KGGuiTreeNodeFlags_NavLeftJumpsBackHere) && !(flags & KGGuiTreeNodeFlags_NoTreePushOnOpen))
		window->DC.TreeJumpToParentOnPopMask |= (1 << window->DC.TreeDepth);

	bool item_add = ItemAdd(interact_bb, id);
	g.LastItemData.StatusFlags |= KGGuiItemStatusFlags_HasDisplayRect;
	g.LastItemData.DisplayRect = frame_bb;

	if (!item_add)
	{
		if (is_open && !(flags & KGGuiTreeNodeFlags_NoTreePushOnOpen))
			TreePushOverrideID(id);
		KARMAGUI_TEST_ENGINE_ITEM_INFO(g.LastItemData.ID, label, g.LastItemData.StatusFlags | (is_leaf ? 0 : KGGuiItemStatusFlags_Openable) | (is_open ? KGGuiItemStatusFlags_Opened : 0));
		return is_open;
	}

	KarmaGuiButtonFlags button_flags = KGGuiTreeNodeFlags_None;
	if (flags & KGGuiTreeNodeFlags_AllowItemOverlap)
		button_flags |= KGGuiButtonFlags_AllowItemOverlap;
	if (!is_leaf)
		button_flags |= KGGuiButtonFlags_PressedOnDragDropHold;

	// We allow clicking on the arrow section with keyboard modifiers held, in order to easily
	// allow browsing a tree while preserving selection with code implementing multi-selection patterns.
	// When clicking on the rest of the tree node we always disallow keyboard modifiers.
	const float arrow_hit_x1 = (text_pos.x - text_offset_x) - style.TouchExtraPadding.x;
	const float arrow_hit_x2 = (text_pos.x - text_offset_x) + (g.FontSize + padding.x * 2.0f) + style.TouchExtraPadding.x;
	const bool is_mouse_x_over_arrow = (g.IO.MousePos.x >= arrow_hit_x1 && g.IO.MousePos.x < arrow_hit_x2);
	if (window != g.HoveredWindow || !is_mouse_x_over_arrow)
		button_flags |= KGGuiButtonFlags_NoKeyModifiers;

	// Open behaviors can be altered with the _OpenOnArrow and _OnOnDoubleClick flags.
	// Some alteration have subtle effects (e.g. toggle on MouseUp vs MouseDown events) due to requirements for multi-selection and drag and drop support.
	// - Single-click on label = Toggle on MouseUp (default, when _OpenOnArrow=0)
	// - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=0)
	// - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=1)
	// - Double-click on label = Toggle on MouseDoubleClick (when _OpenOnDoubleClick=1)
	// - Double-click on arrow = Toggle on MouseDoubleClick (when _OpenOnDoubleClick=1 and _OpenOnArrow=0)
	// It is rather standard that arrow click react on Down rather than Up.
	// We set KGGuiButtonFlags_PressedOnClickRelease on OpenOnDoubleClick because we want the item to be active on the initial MouseDown in order for drag and drop to work.
	if (is_mouse_x_over_arrow)
		button_flags |= KGGuiButtonFlags_PressedOnClick;
	else if (flags & KGGuiTreeNodeFlags_OpenOnDoubleClick)
		button_flags |= KGGuiButtonFlags_PressedOnClickRelease | KGGuiButtonFlags_PressedOnDoubleClick;
	else
		button_flags |= KGGuiButtonFlags_PressedOnClickRelease;

	bool selected = (flags & KGGuiTreeNodeFlags_Selected) != 0;
	const bool was_selected = selected;

	bool hovered, held;
	bool pressed = ButtonBehavior(interact_bb, id, &hovered, &held, button_flags);
	bool toggled = false;
	if (!is_leaf)
	{
		if (pressed && g.DragDropHoldJustPressedId != id)
		{
			if ((flags & (KGGuiTreeNodeFlags_OpenOnArrow | KGGuiTreeNodeFlags_OpenOnDoubleClick)) == 0 || (g.NavActivateId == id))
				toggled = true;
			if (flags & KGGuiTreeNodeFlags_OpenOnArrow)
				toggled |= is_mouse_x_over_arrow && !g.NavDisableMouseHover; // Lightweight equivalent of IsMouseHoveringRect() since ButtonBehavior() already did the job
			if ((flags & KGGuiTreeNodeFlags_OpenOnDoubleClick) && g.IO.MouseClickedCount[0] == 2)
				toggled = true;
		}
		else if (pressed && g.DragDropHoldJustPressedId == id)
		{
			KR_CORE_ASSERT(button_flags & KGGuiButtonFlags_PressedOnDragDropHold, "");
			if (!is_open) // When using Drag and Drop "hold to open" we keep the node highlighted after opening, but never close it again.
				toggled = true;
		}

		if (g.NavId == id && g.NavMoveDir == KGGuiDir_Left && is_open)
		{
			toggled = true;
			NavMoveRequestCancel();
		}
		if (g.NavId == id && g.NavMoveDir == KGGuiDir_Right && !is_open) // If there's something upcoming on the line we may want to give it the priority?
		{
			toggled = true;
			NavMoveRequestCancel();
		}

		if (toggled)
		{
			is_open = !is_open;
			window->DC.StateStorage->SetInt(id, is_open);
			g.LastItemData.StatusFlags |= KGGuiItemStatusFlags_ToggledOpen;
		}
	}
	if (flags & KGGuiTreeNodeFlags_AllowItemOverlap)
		KarmaGui::SetItemAllowOverlap();

	// In this branch, TreeNodeBehavior() cannot toggle the selection so this will never trigger.
	if (selected != was_selected) //-V547
		g.LastItemData.StatusFlags |= KGGuiItemStatusFlags_ToggledSelection;

	// Render
	const KGU32 text_col = KarmaGui::GetColorU32(KGGuiCol_Text);
	KGGuiNavHighlightFlags nav_highlight_flags = KGGuiNavHighlightFlags_TypeThin;
	if (display_frame)
	{
		// Framed type
		const KGU32 bg_col = KarmaGui::GetColorU32((held && hovered) ? KGGuiCol_HeaderActive : hovered ? KGGuiCol_HeaderHovered : KGGuiCol_Header);
		RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, true, style.FrameRounding);
		RenderNavHighlight(frame_bb, id, nav_highlight_flags);
		if (flags & KGGuiTreeNodeFlags_Bullet)
			RenderBullet(window->DrawList, KGVec2(text_pos.x - text_offset_x * 0.60f, text_pos.y + g.FontSize * 0.5f), text_col);
		else if (!is_leaf)
			RenderArrow(window->DrawList, KGVec2(text_pos.x - text_offset_x + padding.x, text_pos.y), text_col, is_open ? KGGuiDir_Down : KGGuiDir_Right, 1.0f);
		else // Leaf without bullet, left-adjusted text
			text_pos.x -= text_offset_x;
		if (flags & KGGuiTreeNodeFlags_ClipLabelForTrailingButton)
			frame_bb.Max.x -= g.FontSize + style.FramePadding.x;

		if (g.LogEnabled)
			LogSetNextTextDecoration("###", "###");
		RenderTextClipped(text_pos, frame_bb.Max, label, label_end, &label_size);
	}
	else
	{
		// Unframed typed for tree nodes
		if (hovered || selected)
		{
			const KGU32 bg_col = KarmaGui::GetColorU32((held && hovered) ? KGGuiCol_HeaderActive : hovered ? KGGuiCol_HeaderHovered : KGGuiCol_Header);
			RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, false);
		}
		RenderNavHighlight(frame_bb, id, nav_highlight_flags);
		if (flags & KGGuiTreeNodeFlags_Bullet)
			RenderBullet(window->DrawList, KGVec2(text_pos.x - text_offset_x * 0.5f, text_pos.y + g.FontSize * 0.5f), text_col);
		else if (!is_leaf)
			RenderArrow(window->DrawList, KGVec2(text_pos.x - text_offset_x + padding.x, text_pos.y + g.FontSize * 0.15f), text_col, is_open ? KGGuiDir_Down : KGGuiDir_Right, 0.70f);
		if (g.LogEnabled)
			LogSetNextTextDecoration(">", NULL);
		RenderText(text_pos, label, label_end, false);
	}

	if (is_open && !(flags & KGGuiTreeNodeFlags_NoTreePushOnOpen))
		TreePushOverrideID(id);
	KARMAGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | (is_leaf ? 0 : KGGuiItemStatusFlags_Openable) | (is_open ? KGGuiItemStatusFlags_Opened : 0));
	return is_open;
}

void Karma::KarmaGui::TreePush(const char* str_id)
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	Indent();
	window->DC.TreeDepth++;
	PushID(str_id);
}

void Karma::KarmaGui::TreePush(const void* ptr_id)
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	Indent();
	window->DC.TreeDepth++;
	PushID(ptr_id);
}

void Karma::KarmaGuiInternal::TreePushOverrideID(KGGuiID id)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	KarmaGui::Indent();
	window->DC.TreeDepth++;
	PushOverrideID(id);
}

void Karma::KarmaGui::TreePop()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	Unindent();

	window->DC.TreeDepth--;
	KGU32 tree_depth_mask = (1 << window->DC.TreeDepth);

	// Handle Left arrow to move to parent tree node (when KGGuiTreeNodeFlags_NavLeftJumpsBackHere is enabled)
	if (g.NavMoveDir == KGGuiDir_Left && g.NavWindow == window && KarmaGuiInternal::NavMoveRequestButNoResultYet())
		if (g.NavIdIsAlive && (window->DC.TreeJumpToParentOnPopMask & tree_depth_mask))
		{
			KarmaGuiInternal::SetNavID(window->IDStack.back(), g.NavLayer, 0, KGRect());
			KarmaGuiInternal::NavMoveRequestCancel();
		}
	window->DC.TreeJumpToParentOnPopMask &= tree_depth_mask - 1;

	KR_CORE_ASSERT(window->IDStack.Size > 1, ""); // There should always be 1 element in the IDStack (pushed during window creation). If this triggers you called TreePop/PopID too much.
	PopID();
}

// Horizontal distance preceding label when using TreeNode() or Bullet()
float Karma::KarmaGui::GetTreeNodeToLabelSpacing()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.FontSize + (g.Style.FramePadding.x * 2.0f);
}

// Set next TreeNode/CollapsingHeader open state.
void Karma::KarmaGui::SetNextItemOpen(bool is_open, KarmaGuiCond cond)
{
	KarmaGuiContext& g = *GKarmaGui;
	if (g.CurrentWindow->SkipItems)
		return;
	g.NextItemData.Flags |= KGGuiNextItemDataFlags_HasOpen;
	g.NextItemData.OpenVal = is_open;
	g.NextItemData.OpenCond = cond ? cond : KGGuiCond_Always;
}

// CollapsingHeader returns true when opened but do not indent nor push into the ID stack (because of the KGGuiTreeNodeFlags_NoTreePushOnOpen flag).
// This is basically the same as calling TreeNodeEx(label, KGGuiTreeNodeFlags_CollapsingHeader). You can remove the _NoTreePushOnOpen flag if you want behavior closer to normal TreeNode().
bool Karma::KarmaGui::CollapsingHeader(const char* label, KarmaGuiTreeNodeFlags flags)
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	return KarmaGuiInternal::TreeNodeBehavior(window->GetID(label), flags | KGGuiTreeNodeFlags_CollapsingHeader, label);
}

// p_visible == NULL                        : regular collapsing header
// p_visible != NULL && *p_visible == true  : show a small close button on the corner of the header, clicking the button will set *p_visible = false
// p_visible != NULL && *p_visible == false : do not show the header at all
// Do not mistake this with the Open state of the header itself, which you can adjust with SetNextItemOpen() or KGGuiTreeNodeFlags_DefaultOpen.
bool Karma::KarmaGui::CollapsingHeader(const char* label, bool* p_visible, KarmaGuiTreeNodeFlags flags)
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	if (p_visible && !*p_visible)
		return false;

	KGGuiID id = window->GetID(label);
	flags |= KGGuiTreeNodeFlags_CollapsingHeader;
	if (p_visible)
		flags |= KGGuiTreeNodeFlags_AllowItemOverlap | KGGuiTreeNodeFlags_ClipLabelForTrailingButton;
	bool is_open = KarmaGuiInternal::TreeNodeBehavior(id, flags, label);
	if (p_visible != NULL)
	{
		// Create a small overlapping close button
		// FIXME: We can evolve this into user accessible helpers to add extra buttons on title bars, headers, etc.
		// FIXME: CloseButton can overlap into text, need find a way to clip the text somehow.
		KarmaGuiContext& g = *GKarmaGui;
		KGGuiLastItemData last_item_backup = g.LastItemData;
		float button_size = g.FontSize;
		float button_x = KGMax(g.LastItemData.Rect.Min.x, g.LastItemData.Rect.Max.x - g.Style.FramePadding.x * 2.0f - button_size);
		float button_y = g.LastItemData.Rect.Min.y;
		KGGuiID close_button_id = KarmaGuiInternal::GetIDWithSeed("#CLOSE", NULL, id);
		if (KarmaGuiInternal::CloseButton(close_button_id, KGVec2(button_x, button_y)))
			*p_visible = false;
		g.LastItemData = last_item_backup;
	}

	return is_open;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: Selectable
//-------------------------------------------------------------------------
// - Selectable()
//-------------------------------------------------------------------------

// Tip: pass a non-visible label (e.g. "##hello") then you can use the space to draw other text or image.
// But you need to make sure the ID is unique, e.g. enclose calls in PushID/PopID or use ##unique_id.
// With this scheme, KGGuiSelectableFlags_SpanAllColumns and KGGuiSelectableFlags_AllowItemOverlap are also frequently used flags.
// FIXME: Selectable() with (size.x == 0.0f) and (SelectableTextAlign.x > 0.0f) followed by SameLine() is currently not supported.
bool Karma::KarmaGui::Selectable(const char* label, bool selected, KarmaGuiSelectableFlags flags, const KGVec2& size_arg)
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	KarmaGuiContext& g = *GKarmaGui;
	const KarmaGuiStyle& style = g.Style;

	// Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning rectangle.
	KGGuiID id = window->GetID(label);
	KGVec2 label_size = CalcTextSize(label, NULL, true);
	KGVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
	KGVec2 pos = window->DC.CursorPos;
	pos.y += window->DC.CurrLineTextBaseOffset;
	KarmaGuiInternal::ItemSize(size, 0.0f);

	// Fill horizontal space
	// We don't support (size < 0.0f) in Selectable() because the ItemSpacing extension would make explicitly right-aligned sizes not visibly match other widgets.
	const bool span_all_columns = (flags & KGGuiSelectableFlags_SpanAllColumns) != 0;
	const float min_x = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
	const float max_x = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
	if (size_arg.x == 0.0f || (flags & KGGuiSelectableFlags_SpanAvailWidth))
		size.x = KGMax(label_size.x, max_x - min_x);

	// Text stays at the submission position, but bounding box may be extended on both sides
	const KGVec2 text_min = pos;
	const KGVec2 text_max(min_x + size.x, pos.y + size.y);

	// Selectables are meant to be tightly packed together with no click-gap, so we extend their box to cover spacing between selectable.
	KGRect bb(min_x, pos.y, text_max.x, text_max.y);
	if ((flags & KGGuiSelectableFlags_NoPadWithHalfSpacing) == 0)
	{
		const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
		const float spacing_y = style.ItemSpacing.y;
		const float spacing_L = KG_FLOOR(spacing_x * 0.50f);
		const float spacing_U = KG_FLOOR(spacing_y * 0.50f);
		bb.Min.x -= spacing_L;
		bb.Min.y -= spacing_U;
		bb.Max.x += (spacing_x - spacing_L);
		bb.Max.y += (spacing_y - spacing_U);
	}
	//if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb.Min, bb.Max, KG_COL32(0, 255, 0, 255)); }

	// Modify ClipRect for the ItemAdd(), faster than doing a PushColumnsBackground/PushTableBackground for every Selectable..
	const float backup_clip_rect_min_x = window->ClipRect.Min.x;
	const float backup_clip_rect_max_x = window->ClipRect.Max.x;
	if (span_all_columns)
	{
		window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
		window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
	}

	const bool disabled_item = (flags & KGGuiSelectableFlags_Disabled) != 0;
	const bool item_add = KarmaGuiInternal::ItemAdd(bb, id, NULL, disabled_item ? KGGuiItemFlags_Disabled : KGGuiItemFlags_None);
	if (span_all_columns)
	{
		window->ClipRect.Min.x = backup_clip_rect_min_x;
		window->ClipRect.Max.x = backup_clip_rect_max_x;
	}

	if (!item_add)
		return false;

	const bool disabled_global = (g.CurrentItemFlags & KGGuiItemFlags_Disabled) != 0;
	if (disabled_item && !disabled_global) // Only testing this as an optimization
		BeginDisabled();

	// FIXME: We can standardize the behavior of those two, we could also keep the fast path of override ClipRect + full push on render only,
	// which would be advantageous since most selectable are not selected.
	if (span_all_columns && window->DC.CurrentColumns)
		KarmaGuiInternal::PushColumnsBackground();
	else if (span_all_columns && g.CurrentTable)
		KarmaGuiInternal::TablePushBackgroundChannel();

	// We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
	KarmaGuiButtonFlags button_flags = 0;
	if (flags & KGGuiSelectableFlags_NoHoldingActiveID) { button_flags |= KGGuiButtonFlags_NoHoldingActiveId; }
	if (flags & KGGuiSelectableFlags_NoSetKeyOwner) { button_flags |= KGGuiButtonFlags_NoSetKeyOwner; }
	if (flags & KGGuiSelectableFlags_SelectOnClick) { button_flags |= KGGuiButtonFlags_PressedOnClick; }
	if (flags & KGGuiSelectableFlags_SelectOnRelease) { button_flags |= KGGuiButtonFlags_PressedOnRelease; }
	if (flags & KGGuiSelectableFlags_AllowDoubleClick) { button_flags |= KGGuiButtonFlags_PressedOnClickRelease | KGGuiButtonFlags_PressedOnDoubleClick; }
	if (flags & KGGuiSelectableFlags_AllowItemOverlap) { button_flags |= KGGuiButtonFlags_AllowItemOverlap; }

	const bool was_selected = selected;
	bool hovered, held;
	bool pressed = KarmaGuiInternal::ButtonBehavior(bb, id, &hovered, &held, button_flags);

	// Auto-select when moved into
	// - This will be more fully fleshed in the range-select branch
	// - This is not exposed as it won't nicely work with some user side handling of shift/control
	// - We cannot do 'if (g.NavJustMovedToId != id) { selected = false; pressed = was_selected; }' for two reasons
	//   - (1) it would require focus scope to be set, need exposing PushFocusScope() or equivalent (e.g. BeginSelection() calling PushFocusScope())
	//   - (2) usage will fail with clipped items
	//   The multi-select API aim to fix those issues, e.g. may be replaced with a BeginSelection() API.
	if ((flags & KGGuiSelectableFlags_SelectOnNav) && g.NavJustMovedToId != 0 && g.NavJustMovedToFocusScopeId == g.CurrentFocusScopeId)
		if (g.NavJustMovedToId == id)
			selected = pressed = true;

	// Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with gamepad/keyboard
	if (pressed || (hovered && (flags & KGGuiSelectableFlags_SetNavIdOnHover)))
	{
		if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
		{
			KarmaGuiInternal::SetNavID(id, window->DC.NavLayerCurrent, g.CurrentFocusScopeId, KarmaGuiInternal::WindowRectAbsToRel(window, bb)); // (bb == NavRect)
			g.NavDisableHighlight = true;
		}
	}
	if (pressed)
		KarmaGuiInternal::MarkItemEdited(id);

	if (flags & KGGuiSelectableFlags_AllowItemOverlap)
		SetItemAllowOverlap();

	// In this branch, Selectable() cannot toggle the selection so this will never trigger.
	if (selected != was_selected) //-V547
		g.LastItemData.StatusFlags |= KGGuiItemStatusFlags_ToggledSelection;

	// Render
	if (held && (flags & KGGuiSelectableFlags_DrawHoveredWhenHeld))
		hovered = true;
	if (hovered || selected)
	{
		const KGU32 col = GetColorU32((held && hovered) ? KGGuiCol_HeaderActive : hovered ? KGGuiCol_HeaderHovered : KGGuiCol_Header);
		KarmaGuiInternal::RenderFrame(bb.Min, bb.Max, col, false, 0.0f);
	}
	KarmaGuiInternal::RenderNavHighlight(bb, id, KGGuiNavHighlightFlags_TypeThin | KGGuiNavHighlightFlags_NoRounding);

	if (span_all_columns && window->DC.CurrentColumns)
		KarmaGuiInternal::PopColumnsBackground();
	else if (span_all_columns && g.CurrentTable)
		KarmaGuiInternal::TablePopBackgroundChannel();

	KarmaGuiInternal::RenderTextClipped(text_min, text_max, label, NULL, &label_size, style.SelectableTextAlign, &bb);

	// Automatically close popups
	if (pressed && (window->Flags & KGGuiWindowFlags_Popup) && !(flags & KGGuiSelectableFlags_DontClosePopups) && !(g.LastItemData.InFlags & KGGuiItemFlags_SelectableDontClosePopup))
		CloseCurrentPopup();

	if (disabled_item && !disabled_global)
		EndDisabled();

	KARMAGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
	return pressed; //-V1020
}

bool Karma::KarmaGui::Selectable(const char* label, bool* p_selected, KarmaGuiSelectableFlags flags, const KGVec2& size_arg)
{
	if (Selectable(label, *p_selected, flags, size_arg))
	{
		*p_selected = !*p_selected;
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: ListBox
//-------------------------------------------------------------------------
// - BeginListBox()
// - EndListBox()
// - ListBox()
//-------------------------------------------------------------------------

// Tip: To have a list filling the entire window width, use size.x = -FLT_MIN and pass an non-visible label e.g. "##empty"
// Tip: If your vertical size is calculated from an item count (e.g. 10 * item_height) consider adding a fractional part to facilitate seeing scrolling boundaries (e.g. 10.25 * item_height).
bool Karma::KarmaGui::BeginListBox(const char* label, const KGVec2& size_arg)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	const KarmaGuiStyle& style = g.Style;
	const KGGuiID id = GetID(label);
	const KGVec2 label_size = CalcTextSize(label, NULL, true);

	// Size default to hold ~7.25 items.
	// Fractional number of items helps seeing that we can scroll down/up without looking at scrollbar.
	KGVec2 size = KGFloor(KarmaGuiInternal::CalcItemSize(size_arg, CalcItemWidth(), GetTextLineHeightWithSpacing() * 7.25f + style.FramePadding.y * 2.0f));
	KGVec2 frame_size = KGVec2(size.x, KGMax(size.y, label_size.y));
	KGRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
	KGRect bb(frame_bb.Min, frame_bb.Max + KGVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
	g.NextItemData.ClearFlags();

	if (!IsRectVisible(bb.Min, bb.Max))
	{
		KarmaGuiInternal::ItemSize(bb.GetSize(), style.FramePadding.y);
		KarmaGuiInternal::ItemAdd(bb, 0, &frame_bb);
		return false;
	}

	// FIXME-OPT: We could omit the BeginGroup() if label_size.x but would need to omit the EndGroup() as well.
	BeginGroup();
	if (label_size.x > 0.0f)
	{
		KGVec2 label_pos = KGVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y);
		KarmaGuiInternal::RenderText(label_pos, label);
		window->DC.CursorMaxPos = KGMax(window->DC.CursorMaxPos, label_pos + label_size);
	}

	BeginChildFrame(id, frame_bb.GetSize());
	return true;
}

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
// OBSOLETED in 1.81 (from February 2021)
/*
bool Karma::KarmaGui::ListBoxHeader(const char* label, int items_count, int height_in_items)
{
	// If height_in_items == -1, default height is maximum 7.
	KarmaGuiContext& g = *GKarmaGui;
	float height_in_items_f = (height_in_items < 0 ? KGMin(items_count, 7) : height_in_items) + 0.25f;
	KGVec2 size;
	size.x = 0.0f;
	size.y = GetTextLineHeightWithSpacing() * height_in_items_f + g.Style.FramePadding.y * 2.0f;
	return BeginListBox(label, size);
}*/
#endif

void Karma::KarmaGui::EndListBox()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	KR_CORE_ASSERT((window->Flags & KGGuiWindowFlags_ChildWindow), "Mismatched BeginListBox/EndListBox calls. Did you test the return value of BeginListBox?");
	KG_UNUSED(window);

	EndChildFrame();
	EndGroup(); // This is only required to be able to do IsItemXXX query on the whole ListBox including label
}

bool Karma::KarmaGui::ListBox(const char* label, int* current_item, const char* const items[], int items_count, int height_items)
{
	const bool value_changed = ListBox(label, current_item, Items_ArrayGetter, (void*)items, items_count, height_items);
	return value_changed;
}

// This is merely a helper around BeginListBox(), EndListBox().
// Considering using those directly to submit custom data or store selection differently.
bool Karma::KarmaGui::ListBox(const char* label, int* current_item, bool (*items_getter)(void*, int, const char**), void* data, int items_count, int height_in_items)
{
	KarmaGuiContext& g = *GKarmaGui;

	// Calculate size from "height_in_items"
	if (height_in_items < 0)
		height_in_items = KGMin(items_count, 7);
	float height_in_items_f = height_in_items + 0.25f;
	KGVec2 size(0.0f, KGFloor(GetTextLineHeightWithSpacing() * height_in_items_f + g.Style.FramePadding.y * 2.0f));

	if (!BeginListBox(label, size))
		return false;

	// Assume all items have even height (= 1 line of text). If you need items of different height,
	// you can create a custom version of ListBox() in your code without using the clipper.
	bool value_changed = false;
	KarmaGuiListClipper clipper;
	clipper.Begin(items_count, GetTextLineHeightWithSpacing()); // We know exactly our line height here so we pass it as a minor optimization, but generally you don't need to.
	while (clipper.Step())
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		{
			const char* item_text;
			if (!items_getter(data, i, &item_text))
				item_text = "*Unknown item*";

			PushID(i);
			const bool item_selected = (i == *current_item);
			if (Selectable(item_text, item_selected))
			{
				*current_item = i;
				value_changed = true;
			}
			if (item_selected)
				SetItemDefaultFocus();
			PopID();
		}
	EndListBox();

	if (value_changed)
		KarmaGuiInternal::MarkItemEdited(g.LastItemData.ID);

	return value_changed;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: PlotLines, PlotHistogram
//-------------------------------------------------------------------------
// - PlotEx() [Internal]
// - PlotLines()
// - PlotHistogram()
//-------------------------------------------------------------------------
// Plot/Graph widgets are not very good.
// Consider writing your own, or using a third-party one, see:
// - ImPlot https://github.com/epezent/implot
// - others https://github.com/ocornut/imgui/wiki/Useful-Extensions
//-------------------------------------------------------------------------

int Karma::KarmaGuiInternal::PlotEx(KGGuiPlotType plot_type, const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, KGVec2 frame_size)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return -1;

	const KarmaGuiStyle& style = g.Style;
	const KGGuiID id = window->GetID(label);

	const KGVec2 label_size = KarmaGui::CalcTextSize(label, NULL, true);
	if (frame_size.x == 0.0f)
		frame_size.x = KarmaGui::CalcItemWidth();
	if (frame_size.y == 0.0f)
		frame_size.y = label_size.y + (style.FramePadding.y * 2);

	const KGRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
	const KGRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
	const KGRect total_bb(frame_bb.Min, frame_bb.Max + KGVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(total_bb, 0, &frame_bb))
		return -1;
	const bool hovered = ItemHoverable(frame_bb, id);

	// Determine scale from values if not specified
	if (scale_min == FLT_MAX || scale_max == FLT_MAX)
	{
		float v_min = FLT_MAX;
		float v_max = -FLT_MAX;
		for (int i = 0; i < values_count; i++)
		{
			const float v = values_getter(data, i);
			if (v != v) // Ignore NaN values
				continue;
			v_min = KGMin(v_min, v);
			v_max = KGMax(v_max, v);
		}
		if (scale_min == FLT_MAX)
			scale_min = v_min;
		if (scale_max == FLT_MAX)
			scale_max = v_max;
	}

	RenderFrame(frame_bb.Min, frame_bb.Max, KarmaGui::GetColorU32(KGGuiCol_FrameBg), true, style.FrameRounding);

	const int values_count_min = (plot_type == KGGuiPlotType_Lines) ? 2 : 1;
	int idx_hovered = -1;
	if (values_count >= values_count_min)
	{
		int res_w = KGMin((int)frame_size.x, values_count) + ((plot_type == KGGuiPlotType_Lines) ? -1 : 0);
		int item_count = values_count + ((plot_type == KGGuiPlotType_Lines) ? -1 : 0);

		// Tooltip on hover
		if (hovered && inner_bb.Contains(g.IO.MousePos))
		{
			const float t = KGClamp((g.IO.MousePos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x), 0.0f, 0.9999f);
			const int v_idx = (int)(t * item_count);
			KR_CORE_ASSERT(v_idx >= 0 && v_idx < values_count, "");

			const float v0 = values_getter(data, (v_idx + values_offset) % values_count);
			const float v1 = values_getter(data, (v_idx + 1 + values_offset) % values_count);
			if (plot_type == KGGuiPlotType_Lines)
				KarmaGui::SetTooltip("%d: %8.4g\n%d: %8.4g", v_idx, v0, v_idx + 1, v1);
			else if (plot_type == KGGuiPlotType_Histogram)
				KarmaGui::SetTooltip("%d: %8.4g", v_idx, v0);
			idx_hovered = v_idx;
		}

		const float t_step = 1.0f / (float)res_w;
		const float inv_scale = (scale_min == scale_max) ? 0.0f : (1.0f / (scale_max - scale_min));

		float v0 = values_getter(data, (0 + values_offset) % values_count);
		float t0 = 0.0f;
		KGVec2 tp0 = KGVec2(t0, 1.0f - KGSaturate((v0 - scale_min) * inv_scale));                       // Point in the normalized space of our target rectangle
		float histogram_zero_line_t = (scale_min * scale_max < 0.0f) ? (1 + scale_min * inv_scale) : (scale_min < 0.0f ? 0.0f : 1.0f);   // Where does the zero line stands

		const KGU32 col_base = KarmaGui::GetColorU32((plot_type == KGGuiPlotType_Lines) ? KGGuiCol_PlotLines : KGGuiCol_PlotHistogram);
		const KGU32 col_hovered = KarmaGui::GetColorU32((plot_type == KGGuiPlotType_Lines) ? KGGuiCol_PlotLinesHovered : KGGuiCol_PlotHistogramHovered);

		for (int n = 0; n < res_w; n++)
		{
			const float t1 = t0 + t_step;
			const int v1_idx = (int)(t0 * item_count + 0.5f);
			KR_CORE_ASSERT(v1_idx >= 0 && v1_idx < values_count, "");
			const float v1 = values_getter(data, (v1_idx + values_offset + 1) % values_count);
			const KGVec2 tp1 = KGVec2(t1, 1.0f - KGSaturate((v1 - scale_min) * inv_scale));

			// NB: Draw calls are merged together by the DrawList system. Still, we should render our batch are lower level to save a bit of CPU.
			KGVec2 pos0 = KGLerp(inner_bb.Min, inner_bb.Max, tp0);
			KGVec2 pos1 = KGLerp(inner_bb.Min, inner_bb.Max, (plot_type == KGGuiPlotType_Lines) ? tp1 : KGVec2(tp1.x, histogram_zero_line_t));
			if (plot_type == KGGuiPlotType_Lines)
			{
				window->DrawList->AddLine(pos0, pos1, idx_hovered == v1_idx ? col_hovered : col_base);
			}
			else if (plot_type == KGGuiPlotType_Histogram)
			{
				if (pos1.x >= pos0.x + 2.0f)
					pos1.x -= 1.0f;
				window->DrawList->AddRectFilled(pos0, pos1, idx_hovered == v1_idx ? col_hovered : col_base);
			}

			t0 = t1;
			tp0 = tp1;
		}
	}

	// Text overlay
	if (overlay_text)
		RenderTextClipped(KGVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, overlay_text, NULL, NULL, KGVec2(0.5f, 0.0f));

	if (label_size.x > 0.0f)
		RenderText(KGVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y), label);

	// Return hovered index or -1 if none are hovered.
	// This is currently not exposed in the public API because we need a larger redesign of the whole thing, but in the short-term we are making it available in PlotEx().
	return idx_hovered;
}

struct KarmaGuiPlotArrayGetterData
{
	const float* Values;
	int Stride;

	KarmaGuiPlotArrayGetterData(const float* values, int stride) { Values = values; Stride = stride; }
};

static float Plot_ArrayGetter(void* data, int idx)
{
	KarmaGuiPlotArrayGetterData* plot_data = (KarmaGuiPlotArrayGetterData*)data;
	const float v = *(const float*)(const void*)((const unsigned char*)plot_data->Values + (size_t)idx * plot_data->Stride);
	return v;
}

void Karma::KarmaGui::PlotLines(const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, KGVec2 graph_size, int stride)
{
	KarmaGuiPlotArrayGetterData data(values, stride);
	KarmaGuiInternal::PlotEx(KGGuiPlotType_Lines, label, &Plot_ArrayGetter, (void*)&data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void Karma::KarmaGui::PlotLines(const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, KGVec2 graph_size)
{
	KarmaGuiInternal::PlotEx(KGGuiPlotType_Lines, label, values_getter, data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void Karma::KarmaGui::PlotHistogram(const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, KGVec2 graph_size, int stride)
{
	KarmaGuiPlotArrayGetterData data(values, stride);
	KarmaGuiInternal::PlotEx(KGGuiPlotType_Histogram, label, &Plot_ArrayGetter, (void*)&data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void Karma::KarmaGui::PlotHistogram(const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, KGVec2 graph_size)
{
	KarmaGuiInternal::PlotEx(KGGuiPlotType_Histogram, label, values_getter, data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: Value helpers
// Those is not very useful, legacy API.
//-------------------------------------------------------------------------
// - Value()
//-------------------------------------------------------------------------

void Karma::KarmaGui::Value(const char* prefix, bool b)
{
	Text("%s: %s", prefix, (b ? "true" : "false"));
}

void Karma::KarmaGui::Value(const char* prefix, int v)
{
	Text("%s: %d", prefix, v);
}

void Karma::KarmaGui::Value(const char* prefix, unsigned int v)
{
	Text("%s: %d", prefix, v);
}

void Karma::KarmaGui::Value(const char* prefix, float v, const char* float_format)
{
	if (float_format)
	{
		char fmt[64];
		KGFormatString(fmt, KG_ARRAYSIZE(fmt), "%%s: %s", float_format);
		Text(fmt, prefix, v);
	}
	else
	{
		Text("%s: %.3f", prefix, v);
	}
}

//-------------------------------------------------------------------------
// [SECTION] MenuItem, BeginMenu, EndMenu, etc.
//-------------------------------------------------------------------------
// - KGGuiMenuColumns [Internal]
// - BeginMenuBar()
// - EndMenuBar()
// - BeginMainMenuBar()
// - EndMainMenuBar()
// - BeginMenu()
// - EndMenu()
// - MenuItemEx() [Internal]
// - MenuItem()
//-------------------------------------------------------------------------

// Helpers for internal use
void KGGuiMenuColumns::Update(float spacing, bool window_reappearing)
{
	if (window_reappearing)
		memset(Widths, 0, sizeof(Widths));
	Spacing = (KGU16)spacing;
	CalcNextTotalWidth(true);
	memset(Widths, 0, sizeof(Widths));
	TotalWidth = NextTotalWidth;
	NextTotalWidth = 0;
}

void KGGuiMenuColumns::CalcNextTotalWidth(bool update_offsets)
{
	KGU16 offset = 0;
	bool want_spacing = false;
	for (int i = 0; i < KG_ARRAYSIZE(Widths); i++)
	{
		KGU16 width = Widths[i];
		if (want_spacing && width > 0)
			offset += Spacing;
		want_spacing |= (width > 0);
		if (update_offsets)
		{
			if (i == 1) { OffsetLabel = offset; }
			if (i == 2) { OffsetShortcut = offset; }
			if (i == 3) { OffsetMark = offset; }
		}
		offset += width;
	}
	NextTotalWidth = offset;
}

float KGGuiMenuColumns::DeclColumns(float w_icon, float w_label, float w_shortcut, float w_mark)
{
	Widths[0] = KGMax(Widths[0], (KGU16)w_icon);
	Widths[1] = KGMax(Widths[1], (KGU16)w_label);
	Widths[2] = KGMax(Widths[2], (KGU16)w_shortcut);
	Widths[3] = KGMax(Widths[3], (KGU16)w_mark);
	CalcNextTotalWidth(false);
	return (float)KGMax(TotalWidth, NextTotalWidth);
}

// FIXME: Provided a rectangle perhaps e.g. a BeginMenuBarEx() could be used anywhere..
// Currently the main responsibility of this function being to setup clip-rect + horizontal layout + menu navigation layer.
// Ideally we also want this to be responsible for claiming space out of the main window scrolling rectangle, in which case KGGuiWindowFlags_MenuBar will become unnecessary.
// Then later the same system could be used for multiple menu-bars, scrollbars, side-bars.
bool Karma::KarmaGui::BeginMenuBar()
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	if (window->SkipItems)
		return false;
	if (!(window->Flags & KGGuiWindowFlags_MenuBar))
		return false;

	KR_CORE_ASSERT(!window->DC.MenuBarAppending, "");
	BeginGroup(); // Backup position on layer 0 // FIXME: Misleading to use a group for that backup/restore
	PushID("##menubar");

	// We don't clip with current window clipping rectangle as it is already set to the area below. However we clip with window full rect.
	// We remove 1 worth of rounding to Max.x to that text in long menus and small windows don't tend to display over the lower-right rounded area, which looks particularly glitchy.
	KGRect bar_rect = window->MenuBarRect();
	KGRect clip_rect(KG_ROUND(bar_rect.Min.x + window->WindowBorderSize), KG_ROUND(bar_rect.Min.y + window->WindowBorderSize), KG_ROUND(KGMax(bar_rect.Min.x, bar_rect.Max.x - KGMax(window->WindowRounding, window->WindowBorderSize))), KG_ROUND(bar_rect.Max.y));
	clip_rect.ClipWith(window->OuterRectClipped);
	PushClipRect(clip_rect.Min, clip_rect.Max, false);

	// We overwrite CursorMaxPos because BeginGroup sets it to CursorPos (essentially the .EmitItem hack in EndMenuBar() would need something analogous here, maybe a BeginGroupEx() with flags).
	window->DC.CursorPos = window->DC.CursorMaxPos = KGVec2(bar_rect.Min.x + window->DC.MenuBarOffset.x, bar_rect.Min.y + window->DC.MenuBarOffset.y);
	window->DC.LayoutType = KGGuiLayoutType_Horizontal;
	window->DC.IsSameLine = false;
	window->DC.NavLayerCurrent = KGGuiNavLayer_Menu;
	window->DC.MenuBarAppending = true;
	AlignTextToFramePadding();
	return true;
}

void Karma::KarmaGui::EndMenuBar()
{
	KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();
	if (window->SkipItems)
		return;
	KarmaGuiContext& g = *GKarmaGui;

	// Nav: When a move request within one of our child menu failed, capture the request to navigate among our siblings.
	if (KarmaGuiInternal::NavMoveRequestButNoResultYet() && (g.NavMoveDir == KGGuiDir_Left || g.NavMoveDir == KGGuiDir_Right) && (g.NavWindow->Flags & KGGuiWindowFlags_ChildMenu))
	{
		// Try to find out if the request is for one of our child menu
		KGGuiWindow* nav_earliest_child = g.NavWindow;
		while (nav_earliest_child->ParentWindow && (nav_earliest_child->ParentWindow->Flags & KGGuiWindowFlags_ChildMenu))
			nav_earliest_child = nav_earliest_child->ParentWindow;
		if (nav_earliest_child->ParentWindow == window && nav_earliest_child->DC.ParentLayoutType == KGGuiLayoutType_Horizontal && (g.NavMoveFlags & KGGuiNavMoveFlags_Forwarded) == 0)
		{
			// To do so we claim focus back, restore NavId and then process the movement request for yet another frame.
			// This involve a one-frame delay which isn't very problematic in this situation. We could remove it by scoring in advance for multiple window (probably not worth bothering)
			const KGGuiNavLayer layer = KGGuiNavLayer_Menu;
			KR_CORE_ASSERT(window->DC.NavLayersActiveMaskNext & (1 << layer), ""); // Sanity check (FIXME: Seems unnecessary)
			KarmaGuiInternal::FocusWindow(window);
			KarmaGuiInternal::SetNavID(window->NavLastIds[layer], layer, 0, window->NavRectRel[layer]);
			g.NavDisableHighlight = true; // Hide highlight for the current frame so we don't see the intermediary selection.
			g.NavDisableMouseHover = g.NavMousePosDirty = true;
			KarmaGuiInternal::NavMoveRequestForward(g.NavMoveDir, g.NavMoveClipDir, g.NavMoveFlags, g.NavMoveScrollFlags); // Repeat
		}
	}

	KG_MSVC_WARNING_SUPPRESS(6011); // Static Analysis false positive "warning C6011: Dereferencing NULL pointer 'window'"
	KR_CORE_ASSERT(window->Flags & KGGuiWindowFlags_MenuBar, "");
	KR_CORE_ASSERT(window->DC.MenuBarAppending, "");
	PopClipRect();
	PopID();
	window->DC.MenuBarOffset.x = window->DC.CursorPos.x - window->Pos.x; // Save horizontal position so next append can reuse it. This is kinda equivalent to a per-layer CursorPos.
	g.GroupStack.back().EmitItem = false;
	EndGroup(); // Restore position on layer 0
	window->DC.LayoutType = KGGuiLayoutType_Vertical;
	window->DC.IsSameLine = false;
	window->DC.NavLayerCurrent = KGGuiNavLayer_Main;
	window->DC.MenuBarAppending = false;
}

// Important: calling order matters!
// FIXME: Somehow overlapping with docking tech.
// FIXME: The "rect-cut" aspect of this could be formalized into a lower-level helper (rect-cut: https://halt.software/dead-simple-layouts)
bool Karma::KarmaGuiInternal::BeginViewportSideBar(const char* name, KarmaGuiViewport* viewport_p, KarmaGuiDir dir, float axis_size, KarmaGuiWindowFlags window_flags)
{
	KR_CORE_ASSERT(dir != KGGuiDir_None, "");

	KGGuiWindow* bar_window = FindWindowByName(name);
	KGGuiViewportP* viewport = (KGGuiViewportP*)(void*)(viewport_p ? viewport_p : KarmaGui::GetMainViewport());
	if (bar_window == NULL || bar_window->BeginCount == 0)
	{
		// Calculate and set window size/position
		KGRect avail_rect = viewport->GetBuildWorkRect();
		KGGuiAxis axis = (dir == KGGuiDir_Up || dir == KGGuiDir_Down) ? KGGuiAxis_Y : KGGuiAxis_X;
		KGVec2 pos = avail_rect.Min;
		if (dir == KGGuiDir_Right || dir == KGGuiDir_Down)
			pos[axis] = avail_rect.Max[axis] - axis_size;
		KGVec2 size = avail_rect.GetSize();
		size[axis] = axis_size;
		KarmaGui::SetNextWindowPos(pos);
		KarmaGui::SetNextWindowSize(size);

		// Report our size into work area (for next frame) using actual window size
		if (dir == KGGuiDir_Up || dir == KGGuiDir_Left)
			viewport->BuildWorkOffsetMin[axis] += axis_size;
		else if (dir == KGGuiDir_Down || dir == KGGuiDir_Right)
			viewport->BuildWorkOffsetMax[axis] -= axis_size;
	}

	window_flags |= KGGuiWindowFlags_NoTitleBar | KGGuiWindowFlags_NoResize | KGGuiWindowFlags_NoMove | KGGuiWindowFlags_NoDocking;
	KarmaGui::SetNextWindowViewport(viewport->ID); // Enforce viewport so we don't create our own viewport when KGGuiConfigFlags_ViewportsNoMerge is set.
	KarmaGui::PushStyleVar(KGGuiStyleVar_WindowRounding, 0.0f);
	KarmaGui::PushStyleVar(KGGuiStyleVar_WindowMinSize, KGVec2(0, 0)); // Lift normal size constraint
	bool is_open = KarmaGui::Begin(name, NULL, window_flags);
	KarmaGui::PopStyleVar(2);

	return is_open;
}

bool Karma::KarmaGui::BeginMainMenuBar()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiViewportP* viewport = (KGGuiViewportP*)(void*)GetMainViewport();

	// Notify of viewport change so GetFrameHeight() can be accurate in case of DPI change
	KarmaGuiInternal::SetCurrentViewport(NULL, viewport);

	// For the main menu bar, which cannot be moved, we honor g.Style.DisplaySafeAreaPadding to ensure text can be visible on a TV set.
	// FIXME: This could be generalized as an opt-in way to clamp window->DC.CursorStartPos to avoid SafeArea?
	// FIXME: Consider removing support for safe area down the line... it's messy. Nowadays consoles have support for TV calibration in OS settings.
	g.NextWindowData.MenuBarOffsetMinVal = KGVec2(g.Style.DisplaySafeAreaPadding.x, KGMax(g.Style.DisplaySafeAreaPadding.y - g.Style.FramePadding.y, 0.0f));
	KarmaGuiWindowFlags window_flags = KGGuiWindowFlags_NoScrollbar | KGGuiWindowFlags_NoSavedSettings | KGGuiWindowFlags_MenuBar;
	float height = GetFrameHeight();
	bool is_open = KarmaGuiInternal::BeginViewportSideBar("##MainMenuBar", viewport, KGGuiDir_Up, height, window_flags);
	g.NextWindowData.MenuBarOffsetMinVal = KGVec2(0.0f, 0.0f);

	if (is_open)
		BeginMenuBar();
	else
		End();
	return is_open;
}

void Karma::KarmaGui::EndMainMenuBar()
{
	EndMenuBar();

	// When the user has left the menu layer (typically: closed menus through activation of an item), we restore focus to the previous window
	// FIXME: With this strategy we won't be able to restore a NULL focus.
	KarmaGuiContext& g = *GKarmaGui;
	if (g.CurrentWindow == g.NavWindow && g.NavLayer == KGGuiNavLayer_Main && !g.NavAnyRequest)
		KarmaGuiInternal::FocusTopMostWindowUnderOne(g.NavWindow, NULL);

	End();
}

static bool IsRootOfOpenMenuSet()
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	if ((g.OpenPopupStack.Size <= g.BeginPopupStack.Size) || (window->Flags & KGGuiWindowFlags_ChildMenu))
		return false;

	// Initially we used 'upper_popup->OpenParentId == window->IDStack.back()' to differentiate multiple menu sets from each others
	// (e.g. inside menu bar vs loose menu items) based on parent ID.
	// This would however prevent the use of e.g. PuhsID() user code submitting menus.
	// Previously this worked between popup and a first child menu because the first child menu always had the _ChildWindow flag,
	// making  hovering on parent popup possible while first child menu was focused - but this was generally a bug with other side effects.
	// Instead we don't treat Popup specifically (in order to consistently support menu features in them), maybe the first child menu of a Popup
	// doesn't have the _ChildWindow flag, and we rely on this IsRootOfOpenMenuSet() check to allow hovering between root window/popup and first child menu.
	// In the end, lack of ID check made it so we could no longer differentiate between separate menu sets. To compensate for that, we at least check parent window nav layer.
	// This fixes the most common case of menu opening on hover when moving between window content and menu bar. Multiple different menu sets in same nav layer would still
	// open on hover, but that should be a lesser problem, because if such menus are close in proximity in window content then it won't feel weird and if they are far apart
	// it likely won't be a problem anyone runs into.
	const KGGuiPopupData* upper_popup = &g.OpenPopupStack[g.BeginPopupStack.Size];
	return (window->DC.NavLayerCurrent == upper_popup->ParentNavLayer && upper_popup->Window && (upper_popup->Window->Flags & KGGuiWindowFlags_ChildMenu));
}

bool Karma::KarmaGuiInternal::BeginMenuEx(const char* label, const char* icon, bool enabled)
{
	KGGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	KarmaGuiContext& g = *GKarmaGui;
	const KarmaGuiStyle& style = g.Style;
	const KGGuiID id = window->GetID(label);
	bool menu_is_open = IsPopupOpen(id, KGGuiPopupFlags_None);

	// Sub-menus are ChildWindow so that mouse can be hovering across them (otherwise top-most popup menu would steal focus and not allow hovering on parent menu)
	// The first menu in a hierarchy isn't so hovering doesn't get across (otherwise e.g. resizing borders with KGGuiButtonFlags_FlattenChildren would react), but top-most BeginMenu() will bypass that limitation.
	KarmaGuiWindowFlags window_flags = KGGuiWindowFlags_ChildMenu | KGGuiWindowFlags_AlwaysAutoResize | KGGuiWindowFlags_NoMove | KGGuiWindowFlags_NoTitleBar | KGGuiWindowFlags_NoSavedSettings | KGGuiWindowFlags_NoNavFocus;
	if (window->Flags & KGGuiWindowFlags_ChildMenu)
		window_flags |= KGGuiWindowFlags_ChildWindow;

	// If a menu with same the ID was already submitted, we will append to it, matching the behavior of Begin().
	// We are relying on a O(N) search - so O(N log N) over the frame - which seems like the most efficient for the expected small amount of BeginMenu() calls per frame.
	// If somehow this is ever becoming a problem we can switch to use e.g. KarmaGuiStorage mapping key to last frame used.
	if (g.MenusIdSubmittedThisFrame.contains(id))
	{
		if (menu_is_open)
			menu_is_open = BeginPopupEx(id, window_flags); // menu_is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
		else
			g.NextWindowData.ClearFlags();          // we behave like Begin() and need to consume those values
		return menu_is_open;
	}

	// Tag menu as used. Next time BeginMenu() with same ID is called it will append to existing menu
	g.MenusIdSubmittedThisFrame.push_back(id);

	KGVec2 label_size = KarmaGui::CalcTextSize(label, NULL, true);

	// Odd hack to allow hovering across menus of a same menu-set (otherwise we wouldn't be able to hover parent without always being a Child window)
	// This is only done for items for the menu set and not the full parent window.
	const bool menuset_is_open = IsRootOfOpenMenuSet();
	if (menuset_is_open)
		PushItemFlag(KGGuiItemFlags_NoWindowHoverableCheck, true);

	// The reference position stored in popup_pos will be used by Begin() to find a suitable position for the child menu,
	// However the final position is going to be different! It is chosen by FindBestWindowPosForPopup().
	// e.g. Menus tend to overlap each other horizontally to amplify relative Z-ordering.
	KGVec2 popup_pos, pos = window->DC.CursorPos;
	KarmaGui::PushID(label);
	if (!enabled)
		KarmaGui::BeginDisabled();
	const KGGuiMenuColumns* offsets = &window->DC.MenuColumns;
	bool pressed;

	// We use KGGuiSelectableFlags_NoSetKeyOwner to allow down on one menu item, move, up on another.
	const KarmaGuiSelectableFlags selectable_flags = KGGuiSelectableFlags_NoHoldingActiveID | KGGuiSelectableFlags_NoSetKeyOwner | KGGuiSelectableFlags_SelectOnClick | KGGuiSelectableFlags_DontClosePopups;
	if (window->DC.LayoutType == KGGuiLayoutType_Horizontal)
	{
		// Menu inside an horizontal menu bar
		// Selectable extend their highlight by half ItemSpacing in each direction.
		// For ChildMenu, the popup position will be overwritten by the call to FindBestWindowPosForPopup() in Begin()
		popup_pos = KGVec2(pos.x - 1.0f - KG_FLOOR(style.ItemSpacing.x * 0.5f), pos.y - style.FramePadding.y + window->MenuBarHeight());
		window->DC.CursorPos.x += KG_FLOOR(style.ItemSpacing.x * 0.5f);
		KarmaGui::PushStyleVar(KGGuiStyleVar_ItemSpacing, KGVec2(style.ItemSpacing.x * 2.0f, style.ItemSpacing.y));
		float w = label_size.x;
		KGVec2 text_pos(window->DC.CursorPos.x + offsets->OffsetLabel, window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);
		pressed = KarmaGui::Selectable("", menu_is_open, selectable_flags, KGVec2(w, 0.0f));
		RenderText(text_pos, label);
		KarmaGui::PopStyleVar();
		window->DC.CursorPos.x += KG_FLOOR(style.ItemSpacing.x * (-1.0f + 0.5f)); // -1 spacing to compensate the spacing added when Selectable() did a SameLine(). It would also work to call SameLine() ourselves after the PopStyleVar().
	}
	else
	{
		// Menu inside a regular/vertical menu
		// (In a typical menu window where all items are BeginMenu() or MenuItem() calls, extra_w will always be 0.0f.
		//  Only when they are other items sticking out we're going to add spacing, yet only register minimum width into the layout system.
		popup_pos = KGVec2(pos.x, pos.y - style.WindowPadding.y);
		float icon_w = (icon && icon[0]) ? KarmaGui::CalcTextSize(icon, NULL).x : 0.0f;
		float checkmark_w = KG_FLOOR(g.FontSize * 1.20f);
		float min_w = window->DC.MenuColumns.DeclColumns(icon_w, label_size.x, 0.0f, checkmark_w); // Feedback to next frame
		float extra_w = KGMax(0.0f, KarmaGui::GetContentRegionAvail().x - min_w);
		KGVec2 text_pos(window->DC.CursorPos.x + offsets->OffsetLabel, window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);
		pressed = KarmaGui::Selectable("", menu_is_open, selectable_flags | KGGuiSelectableFlags_SpanAvailWidth, KGVec2(min_w, 0.0f));
		RenderText(text_pos, label);
		if (icon_w > 0.0f)
			RenderText(pos + KGVec2(offsets->OffsetIcon, 0.0f), icon);
		RenderArrow(window->DrawList, pos + KGVec2(offsets->OffsetMark + extra_w + g.FontSize * 0.30f, 0.0f), KarmaGui::GetColorU32(KGGuiCol_Text), KGGuiDir_Right);
	}
	if (!enabled)
		KarmaGui::EndDisabled();

	const bool hovered = (g.HoveredId == id) && enabled && !g.NavDisableMouseHover;
	if (menuset_is_open)
		PopItemFlag();

	bool want_open = false;
	bool want_close = false;
	if (window->DC.LayoutType == KGGuiLayoutType_Vertical) // (window->Flags & (KGGuiWindowFlags_Popup|KGGuiWindowFlags_ChildMenu))
	{
		// Close menu when not hovering it anymore unless we are moving roughly in the direction of the menu
		// Implement http://bjk5.com/post/44698559168/breaking-down-amazons-mega-dropdown to avoid using timers, so menus feels more reactive.
		bool moving_toward_child_menu = false;
		KGGuiPopupData* child_popup = (g.BeginPopupStack.Size < g.OpenPopupStack.Size) ? &g.OpenPopupStack[g.BeginPopupStack.Size] : NULL; // Popup candidate (testing below)
		KGGuiWindow* child_menu_window = (child_popup && child_popup->Window && child_popup->Window->ParentWindow == window) ? child_popup->Window : NULL;
		if (g.HoveredWindow == window && child_menu_window != NULL)
		{
			float ref_unit = g.FontSize; // FIXME-DPI
			float child_dir = (window->Pos.x < child_menu_window->Pos.x) ? 1.0f : -1.0f;
			KGRect next_window_rect = child_menu_window->Rect();
			KGVec2 ta = (g.IO.MousePos - g.IO.MouseDelta);
			KGVec2 tb = (child_dir > 0.0f) ? next_window_rect.GetTL() : next_window_rect.GetTR();
			KGVec2 tc = (child_dir > 0.0f) ? next_window_rect.GetBL() : next_window_rect.GetBR();
			float extra = KGClamp(KGFabs(ta.x - tb.x) * 0.30f, ref_unit * 0.5f, ref_unit * 2.5f);   // add a bit of extra slack.
			ta.x += child_dir * -0.5f;
			tb.x += child_dir * ref_unit;
			tc.x += child_dir * ref_unit;
			tb.y = ta.y + KGMax((tb.y - extra) - ta.y, -ref_unit * 8.0f);                           // triangle has maximum height to limit the slope and the bias toward large sub-menus
			tc.y = ta.y + KGMin((tc.y + extra) - ta.y, +ref_unit * 8.0f);
			moving_toward_child_menu = KGTriangleContainsPoint(ta, tb, tc, g.IO.MousePos);
			//GetForegroundDrawList()->AddTriangleFilled(ta, tb, tc, moving_toward_child_menu ? KG_COL32(0,128,0,128) : KG_COL32(128,0,0,128)); // [DEBUG]
		}

		// The 'HovereWindow == window' check creates an inconsistency (e.g. moving away from menu slowly tends to hit same window, whereas moving away fast does not)
		// But we also need to not close the top-menu menu when moving over void. Perhaps we should extend the triangle check to a larger polygon.
		// (Remember to test this on BeginPopup("A")->BeginMenu("B") sequence which behaves slightly differently as B isn't a Child of A and hovering isn't shared.)
		if (menu_is_open && !hovered && g.HoveredWindow == window && !moving_toward_child_menu && !g.NavDisableMouseHover)
			want_close = true;

		// Open
		if (!menu_is_open && pressed) // Click/activate to open
			want_open = true;
		else if (!menu_is_open && hovered && !moving_toward_child_menu) // Hover to open
			want_open = true;
		if (g.NavId == id && g.NavMoveDir == KGGuiDir_Right) // Nav-Right to open
		{
			want_open = true;
			NavMoveRequestCancel();
		}
	}
	else
	{
		// Menu bar
		if (menu_is_open && pressed && menuset_is_open) // Click an open menu again to close it
		{
			want_close = true;
			want_open = menu_is_open = false;
		}
		else if (pressed || (hovered && menuset_is_open && !menu_is_open)) // First click to open, then hover to open others
		{
			want_open = true;
		}
		else if (g.NavId == id && g.NavMoveDir == KGGuiDir_Down) // Nav-Down to open
		{
			want_open = true;
			NavMoveRequestCancel();
		}
	}

	if (!enabled) // explicitly close if an open menu becomes disabled, facilitate users code a lot in pattern such as 'if (BeginMenu("options", has_object)) { ..use object.. }'
		want_close = true;
	if (want_close && IsPopupOpen(id, KGGuiPopupFlags_None))
		ClosePopupToLevel(g.BeginPopupStack.Size, true);

	KARMAGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | KGGuiItemStatusFlags_Openable | (menu_is_open ? KGGuiItemStatusFlags_Opened : 0));
	KarmaGui::PopID();

	if (want_open && !menu_is_open && g.OpenPopupStack.Size > g.BeginPopupStack.Size)
	{
		// Don't reopen/recycle same menu level in the same frame, first close the other menu and yield for a frame.
		KarmaGui::OpenPopup(label);
	}
	else if (want_open)
	{
		menu_is_open = true;
		KarmaGui::OpenPopup(label);
	}

	if (menu_is_open)
	{
		KGGuiLastItemData last_item_in_parent = g.LastItemData;
		KarmaGui::SetNextWindowPos(popup_pos, KGGuiCond_Always);                  // Note: misleading: the value will serve as reference for FindBestWindowPosForPopup(), not actual pos.
		KarmaGui::PushStyleVar(KGGuiStyleVar_ChildRounding, style.PopupRounding); // First level will use _PopupRounding, subsequent will use _ChildRounding
		menu_is_open = BeginPopupEx(id, window_flags);                  // menu_is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
		KarmaGui::PopStyleVar();
		if (menu_is_open)
		{
			// Restore LastItemData so IsItemXXXX functions can work after BeginMenu()/EndMenu()
			// (This fixes using IsItemClicked() and IsItemHovered(), but IsItemHovered() also relies on its support for KGGuiItemFlags_NoWindowHoverableCheck)
			g.LastItemData = last_item_in_parent;
			if (g.HoveredWindow == window)
				g.LastItemData.StatusFlags |= KGGuiItemStatusFlags_HoveredWindow;
		}
	}
	else
	{
		g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
	}

	return menu_is_open;
}

bool Karma::KarmaGui::BeginMenu(const char* label, bool enabled)
{
	return KarmaGuiInternal::BeginMenuEx(label, NULL, enabled);
}

void Karma::KarmaGui::EndMenu()
{
	// Nav: When a left move request our menu failed, close ourselves.
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	KR_CORE_ASSERT(window->Flags & KGGuiWindowFlags_Popup, "");  // Mismatched BeginMenu()/EndMenu() calls
	KGGuiWindow* parent_window = window->ParentWindow;  // Should always be != NULL is we passed assert.
	if (window->BeginCount == window->BeginCountPreviousFrame)
		if (g.NavMoveDir == KGGuiDir_Left && KarmaGuiInternal::NavMoveRequestButNoResultYet())
			if (g.NavWindow && (g.NavWindow->RootWindowForNav == window) && parent_window->DC.LayoutType == KGGuiLayoutType_Vertical)
			{
				KarmaGuiInternal::ClosePopupToLevel(g.BeginPopupStack.Size - 1, true);
				KarmaGuiInternal::NavMoveRequestCancel();
			}

	EndPopup();
}

bool Karma::KarmaGuiInternal::MenuItemEx(const char* label, const char* icon, const char* shortcut, bool selected, bool enabled)
{
	KGGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	KarmaGuiContext& g = *GKarmaGui;
	KarmaGuiStyle& style = g.Style;
	KGVec2 pos = window->DC.CursorPos;
	KGVec2 label_size = KarmaGui::CalcTextSize(label, NULL, true);

	// See BeginMenuEx() for comments about this.
	const bool menuset_is_open = IsRootOfOpenMenuSet();
	if (menuset_is_open)
		PushItemFlag(KGGuiItemFlags_NoWindowHoverableCheck, true);

	// We've been using the equivalent of KGGuiSelectableFlags_SetNavIdOnHover on all Selectable() since early Nav system days (commit 43ee5d73),
	// but I am unsure whether this should be kept at all. For now moved it to be an opt-in feature used by menus only.
	bool pressed;
	KarmaGui::PushID(label);
	if (!enabled)
		KarmaGui::BeginDisabled();

	// We use KGGuiSelectableFlags_NoSetKeyOwner to allow down on one menu item, move, up on another.
	const KarmaGuiSelectableFlags selectable_flags = KGGuiSelectableFlags_SelectOnRelease | KGGuiSelectableFlags_NoSetKeyOwner | KGGuiSelectableFlags_SetNavIdOnHover;
	const KGGuiMenuColumns* offsets = &window->DC.MenuColumns;
	if (window->DC.LayoutType == KGGuiLayoutType_Horizontal)
	{
		// Mimic the exact layout spacing of BeginMenu() to allow MenuItem() inside a menu bar, which is a little misleading but may be useful
		// Note that in this situation: we don't render the shortcut, we render a highlight instead of the selected tick mark.
		float w = label_size.x;
		window->DC.CursorPos.x += KG_FLOOR(style.ItemSpacing.x * 0.5f);
		KGVec2 text_pos(window->DC.CursorPos.x + offsets->OffsetLabel, window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);
		KarmaGui::PushStyleVar(KGGuiStyleVar_ItemSpacing, KGVec2(style.ItemSpacing.x * 2.0f, style.ItemSpacing.y));
		pressed = KarmaGui::Selectable("", selected, selectable_flags, KGVec2(w, 0.0f));
		KarmaGui::PopStyleVar();
		if (g.LastItemData.StatusFlags & KGGuiItemStatusFlags_Visible)
			RenderText(text_pos, label);
		window->DC.CursorPos.x += KG_FLOOR(style.ItemSpacing.x * (-1.0f + 0.5f)); // -1 spacing to compensate the spacing added when Selectable() did a SameLine(). It would also work to call SameLine() ourselves after the PopStyleVar().
	}
	else
	{
		// Menu item inside a vertical menu
		// (In a typical menu window where all items are BeginMenu() or MenuItem() calls, extra_w will always be 0.0f.
		//  Only when they are other items sticking out we're going to add spacing, yet only register minimum width into the layout system.
		float icon_w = (icon && icon[0]) ? KarmaGui::CalcTextSize(icon, NULL).x : 0.0f;
		float shortcut_w = (shortcut && shortcut[0]) ? KarmaGui::CalcTextSize(shortcut, NULL).x : 0.0f;
		float checkmark_w = KG_FLOOR(g.FontSize * 1.20f);
		float min_w = window->DC.MenuColumns.DeclColumns(icon_w, label_size.x, shortcut_w, checkmark_w); // Feedback for next frame
		float stretch_w = KGMax(0.0f, KarmaGui::GetContentRegionAvail().x - min_w);
		pressed = KarmaGui::Selectable("", false, selectable_flags | KGGuiSelectableFlags_SpanAvailWidth, KGVec2(min_w, 0.0f));
		if (g.LastItemData.StatusFlags & KGGuiItemStatusFlags_Visible)
		{
			RenderText(pos + KGVec2(offsets->OffsetLabel, 0.0f), label);
			if (icon_w > 0.0f)
				RenderText(pos + KGVec2(offsets->OffsetIcon, 0.0f), icon);
			if (shortcut_w > 0.0f)
			{
				KarmaGui::PushStyleColor(KGGuiCol_Text, style.Colors[KGGuiCol_TextDisabled]);
				RenderText(pos + KGVec2(offsets->OffsetShortcut + stretch_w, 0.0f), shortcut, NULL, false);
				KarmaGui::PopStyleColor();
			}
			if (selected)
				RenderCheckMark(window->DrawList, pos + KGVec2(offsets->OffsetMark + stretch_w + g.FontSize * 0.40f, g.FontSize * 0.134f * 0.5f), KarmaGui::GetColorU32(KGGuiCol_Text), g.FontSize * 0.866f);
		}
	}
	KARMAGUI_TEST_ENGINE_ITEM_INFO(g.LastItemData.ID, label, g.LastItemData.StatusFlags | KGGuiItemStatusFlags_Checkable | (selected ? KGGuiItemStatusFlags_Checked : 0));
	if (!enabled)
		KarmaGui::EndDisabled();
	KarmaGui::PopID();
	if (menuset_is_open)
		PopItemFlag();

	return pressed;
}

bool Karma::KarmaGui::MenuItem(const char* label, const char* shortcut, bool selected, bool enabled)
{
	return KarmaGuiInternal::MenuItemEx(label, NULL, shortcut, selected, enabled);
}

bool Karma::KarmaGui::MenuItem(const char* label, const char* shortcut, bool* p_selected, bool enabled)
{
	if (KarmaGuiInternal::MenuItemEx(label, NULL, shortcut, p_selected ? *p_selected : false, enabled))
	{
		if (p_selected)
			*p_selected = !*p_selected;
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: BeginTabBar, EndTabBar, etc.
//-------------------------------------------------------------------------
// - BeginTabBar()
// - BeginTabBarEx() [Internal]
// - EndTabBar()
// - TabBarLayout() [Internal]
// - TabBarCalcTabID() [Internal]
// - TabBarCalcMaxTabWidth() [Internal]
// - TabBarFindTabById() [Internal]
// - TabBarAddTab() [Internal]
// - TabBarRemoveTab() [Internal]
// - TabBarCloseTab() [Internal]
// - TabBarScrollClamp() [Internal]
// - TabBarScrollToTab() [Internal]
// - TabBarQueueChangeTabOrder() [Internal]
// - TabBarScrollingButtons() [Internal]
// - TabBarTabListPopupButton() [Internal]
//-------------------------------------------------------------------------

struct KarmaGuiTabBarSection
{
	int                 TabCount;               // Number of tabs in this section.
	float               Width;                  // Sum of width of tabs in this section (after shrinking down)
	float               Spacing;                // Horizontal spacing at the end of the section.

	KarmaGuiTabBarSection() { memset(this, 0, sizeof(*this)); }
};

namespace Karma
{
	static void             TabBarLayout(KGGuiTabBar* tab_bar);
	static KGU32            TabBarCalcTabID(KGGuiTabBar* tab_bar, const char* label, KGGuiWindow* docked_window);
	static float            TabBarCalcMaxTabWidth();
	static float            TabBarScrollClamp(KGGuiTabBar* tab_bar, float scrolling);
	static void             TabBarScrollToTab(KGGuiTabBar* tab_bar, KGGuiID tab_id, KarmaGuiTabBarSection* sections);
	static KGGuiTabItem* TabBarScrollingButtons(KGGuiTabBar* tab_bar);
	static KGGuiTabItem* TabBarTabListPopupButton(KGGuiTabBar* tab_bar);
}

KGGuiTabBar::KGGuiTabBar()
{
	memset(this, 0, sizeof(*this));
	CurrFrameVisible = PrevFrameVisible = -1;
	LastTabItemIdx = -1;
}

static inline int TabItemGetSectionIdx(const KGGuiTabItem* tab)
{
	return (tab->Flags & KGGuiTabItemFlags_Leading) ? 0 : (tab->Flags & KGGuiTabItemFlags_Trailing) ? 2 : 1;
}

static int TabItemComparerBySection(const void* lhs, const void* rhs)
{
	const KGGuiTabItem* a = (const KGGuiTabItem*)lhs;
	const KGGuiTabItem* b = (const KGGuiTabItem*)rhs;
	const int a_section = TabItemGetSectionIdx(a);
	const int b_section = TabItemGetSectionIdx(b);
	if (a_section != b_section)
		return a_section - b_section;
	return (int)(a->IndexDuringLayout - b->IndexDuringLayout);
}

static int TabItemComparerByBeginOrder(const void* lhs, const void* rhs)
{
	const KGGuiTabItem* a = (const KGGuiTabItem*)lhs;
	const KGGuiTabItem* b = (const KGGuiTabItem*)rhs;
	return (int)(a->BeginOrder - b->BeginOrder);
}

static KGGuiTabBar* GetTabBarFromTabBarRef(const KGGuiPtrOrIndex& ref)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	return ref.Ptr ? (KGGuiTabBar*)ref.Ptr : g.TabBars.GetByIndex(ref.Index);
}

static KGGuiPtrOrIndex GetTabBarRefFromTabBar(KGGuiTabBar* tab_bar)
{
	KarmaGuiContext& g = *Karma::GKarmaGui;
	if (g.TabBars.Contains(tab_bar))
		return KGGuiPtrOrIndex(g.TabBars.GetIndex(tab_bar));
	return KGGuiPtrOrIndex(tab_bar);
}

bool Karma::KarmaGui::BeginTabBar(const char* str_id, KarmaGuiTabBarFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	if (window->SkipItems)
		return false;

	KGGuiID id = window->GetID(str_id);
	KGGuiTabBar* tab_bar = g.TabBars.GetOrAddByKey(id);
	KGRect tab_bar_bb = KGRect(window->DC.CursorPos.x, window->DC.CursorPos.y, window->WorkRect.Max.x, window->DC.CursorPos.y + g.FontSize + g.Style.FramePadding.y * 2);
	tab_bar->ID = id;
	return KarmaGuiInternal::BeginTabBarEx(tab_bar, tab_bar_bb, flags | KGGuiTabBarFlags_IsFocused, NULL);
}

bool Karma::KarmaGuiInternal::BeginTabBarEx(KGGuiTabBar* tab_bar, const KGRect& tab_bar_bb, KarmaGuiTabBarFlags flags, KGGuiDockNode* dock_node)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	if (window->SkipItems)
		return false;

	if ((flags & KGGuiTabBarFlags_DockNode) == 0)
		PushOverrideID(tab_bar->ID);

	// Add to stack
	g.CurrentTabBarStack.push_back(GetTabBarRefFromTabBar(tab_bar));
	g.CurrentTabBar = tab_bar;

	// Append with multiple BeginTabBar()/EndTabBar() pairs.
	tab_bar->BackupCursorPos = window->DC.CursorPos;
	if (tab_bar->CurrFrameVisible == g.FrameCount)
	{
		window->DC.CursorPos = KGVec2(tab_bar->BarRect.Min.x, tab_bar->BarRect.Max.y + tab_bar->ItemSpacingY);
		tab_bar->BeginCount++;
		return true;
	}

	// Ensure correct ordering when toggling KGGuiTabBarFlags_Reorderable flag, or when a new tab was added while being not reorderable
	if ((flags & KGGuiTabBarFlags_Reorderable) != (tab_bar->Flags & KGGuiTabBarFlags_Reorderable) || (tab_bar->TabsAddedNew && !(flags & KGGuiTabBarFlags_Reorderable)))
		if ((flags & KGGuiTabBarFlags_DockNode) == 0) // FIXME: TabBar with DockNode can now be hybrid
			KGQsort(tab_bar->Tabs.Data, tab_bar->Tabs.Size, sizeof(KGGuiTabItem), TabItemComparerByBeginOrder);
	tab_bar->TabsAddedNew = false;

	// Flags
	if ((flags & KGGuiTabBarFlags_FittingPolicyMask_) == 0)
		flags |= KGGuiTabBarFlags_FittingPolicyDefault_;

	tab_bar->Flags = flags;
	tab_bar->BarRect = tab_bar_bb;
	tab_bar->WantLayout = true; // Layout will be done on the first call to ItemTab()
	tab_bar->PrevFrameVisible = tab_bar->CurrFrameVisible;
	tab_bar->CurrFrameVisible = g.FrameCount;
	tab_bar->PrevTabsContentsHeight = tab_bar->CurrTabsContentsHeight;
	tab_bar->CurrTabsContentsHeight = 0.0f;
	tab_bar->ItemSpacingY = g.Style.ItemSpacing.y;
	tab_bar->FramePadding = g.Style.FramePadding;
	tab_bar->TabsActiveCount = 0;
	tab_bar->BeginCount = 1;

	// Set cursor pos in a way which only be used in the off-chance the user erroneously submits item before BeginTabItem(): items will overlap
	window->DC.CursorPos = KGVec2(tab_bar->BarRect.Min.x, tab_bar->BarRect.Max.y + tab_bar->ItemSpacingY);

	// Draw separator
	const KGU32 col = KarmaGui::GetColorU32((flags & KGGuiTabBarFlags_IsFocused) ? KGGuiCol_TabActive : KGGuiCol_TabUnfocusedActive);
	const float y = tab_bar->BarRect.Max.y - 1.0f;
	if (dock_node != NULL)
	{
		const float separator_min_x = dock_node->Pos.x + window->WindowBorderSize;
		const float separator_max_x = dock_node->Pos.x + dock_node->Size.x - window->WindowBorderSize;
		window->DrawList->AddLine(KGVec2(separator_min_x, y), KGVec2(separator_max_x, y), col, 1.0f);
	}
	else
	{
		const float separator_min_x = tab_bar->BarRect.Min.x - KG_FLOOR(window->WindowPadding.x * 0.5f);
		const float separator_max_x = tab_bar->BarRect.Max.x + KG_FLOOR(window->WindowPadding.x * 0.5f);
		window->DrawList->AddLine(KGVec2(separator_min_x, y), KGVec2(separator_max_x, y), col, 1.0f);
	}
	return true;
}

void Karma::KarmaGui::EndTabBar()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	if (window->SkipItems)
		return;

	KGGuiTabBar* tab_bar = g.CurrentTabBar;
	if (tab_bar == NULL)
	{
		KR_CORE_ASSERT(tab_bar != NULL, "Mismatched BeginTabBar()/EndTabBar()!");
		return;
	}

	// Fallback in case no TabItem have been submitted
	if (tab_bar->WantLayout)
		Karma::TabBarLayout(tab_bar);

	// Restore the last visible height if no tab is visible, this reduce vertical flicker/movement when a tabs gets removed without calling SetTabItemClosed().
	const bool tab_bar_appearing = (tab_bar->PrevFrameVisible + 1 < g.FrameCount);
	if (tab_bar->VisibleTabWasSubmitted || tab_bar->VisibleTabId == 0 || tab_bar_appearing)
	{
		tab_bar->CurrTabsContentsHeight = KGMax(window->DC.CursorPos.y - tab_bar->BarRect.Max.y, tab_bar->CurrTabsContentsHeight);
		window->DC.CursorPos.y = tab_bar->BarRect.Max.y + tab_bar->CurrTabsContentsHeight;
	}
	else
	{
		window->DC.CursorPos.y = tab_bar->BarRect.Max.y + tab_bar->PrevTabsContentsHeight;
	}
	if (tab_bar->BeginCount > 1)
		window->DC.CursorPos = tab_bar->BackupCursorPos;

	if ((tab_bar->Flags & KGGuiTabBarFlags_DockNode) == 0)
		PopID();

	g.CurrentTabBarStack.pop_back();
	g.CurrentTabBar = g.CurrentTabBarStack.empty() ? NULL : GetTabBarFromTabBarRef(g.CurrentTabBarStack.back());
}

// This is called only once a frame before by the first call to ItemTab()
// The reason we're not calling it in BeginTabBar() is to leave a chance to the user to call the SetTabItemClosed() functions.
static void Karma::TabBarLayout(KGGuiTabBar* tab_bar)
{
	KarmaGuiContext& g = *GKarmaGui;
	tab_bar->WantLayout = false;

	// Garbage collect by compacting list
	// Detect if we need to sort out tab list (e.g. in rare case where a tab changed section)
	int tab_dst_n = 0;
	bool need_sort_by_section = false;
	KarmaGuiTabBarSection sections[3]; // Layout sections: Leading, Central, Trailing
	for (int tab_src_n = 0; tab_src_n < tab_bar->Tabs.Size; tab_src_n++)
	{
		KGGuiTabItem* tab = &tab_bar->Tabs[tab_src_n];
		if (tab->LastFrameVisible < tab_bar->PrevFrameVisible || tab->WantClose)
		{
			// Remove tab
			if (tab_bar->VisibleTabId == tab->ID) { tab_bar->VisibleTabId = 0; }
			if (tab_bar->SelectedTabId == tab->ID) { tab_bar->SelectedTabId = 0; }
			if (tab_bar->NextSelectedTabId == tab->ID) { tab_bar->NextSelectedTabId = 0; }
			continue;
		}
		if (tab_dst_n != tab_src_n)
			tab_bar->Tabs[tab_dst_n] = tab_bar->Tabs[tab_src_n];

		tab = &tab_bar->Tabs[tab_dst_n];
		tab->IndexDuringLayout = (KGS16)tab_dst_n;

		// We will need sorting if tabs have changed section (e.g. moved from one of Leading/Central/Trailing to another)
		int curr_tab_section_n = TabItemGetSectionIdx(tab);
		if (tab_dst_n > 0)
		{
			KGGuiTabItem* prev_tab = &tab_bar->Tabs[tab_dst_n - 1];
			int prev_tab_section_n = TabItemGetSectionIdx(prev_tab);
			if (curr_tab_section_n == 0 && prev_tab_section_n != 0)
				need_sort_by_section = true;
			if (prev_tab_section_n == 2 && curr_tab_section_n != 2)
				need_sort_by_section = true;
		}

		sections[curr_tab_section_n].TabCount++;
		tab_dst_n++;
	}
	if (tab_bar->Tabs.Size != tab_dst_n)
		tab_bar->Tabs.resize(tab_dst_n);

	if (need_sort_by_section)
		KGQsort(tab_bar->Tabs.Data, tab_bar->Tabs.Size, sizeof(KGGuiTabItem), TabItemComparerBySection);

	// Calculate spacing between sections
	sections[0].Spacing = sections[0].TabCount > 0 && (sections[1].TabCount + sections[2].TabCount) > 0 ? g.Style.ItemInnerSpacing.x : 0.0f;
	sections[1].Spacing = sections[1].TabCount > 0 && sections[2].TabCount > 0 ? g.Style.ItemInnerSpacing.x : 0.0f;

	// Setup next selected tab
	KGGuiID scroll_to_tab_id = 0;
	if (tab_bar->NextSelectedTabId)
	{
		tab_bar->SelectedTabId = tab_bar->NextSelectedTabId;
		tab_bar->NextSelectedTabId = 0;
		scroll_to_tab_id = tab_bar->SelectedTabId;
	}

	// Process order change request (we could probably process it when requested but it's just saner to do it in a single spot).
	if (tab_bar->ReorderRequestTabId != 0)
	{
		if (KarmaGuiInternal::TabBarProcessReorder(tab_bar))
			if (tab_bar->ReorderRequestTabId == tab_bar->SelectedTabId)
				scroll_to_tab_id = tab_bar->ReorderRequestTabId;
		tab_bar->ReorderRequestTabId = 0;
	}

	// Tab List Popup (will alter tab_bar->BarRect and therefore the available width!)
	const bool tab_list_popup_button = (tab_bar->Flags & KGGuiTabBarFlags_TabListPopupButton) != 0;
	if (tab_list_popup_button)
		if (KGGuiTabItem* tab_to_select = TabBarTabListPopupButton(tab_bar)) // NB: Will alter BarRect.Min.x!
			scroll_to_tab_id = tab_bar->SelectedTabId = tab_to_select->ID;

	// Leading/Trailing tabs will be shrink only if central one aren't visible anymore, so layout the shrink data as: leading, trailing, central
	// (whereas our tabs are stored as: leading, central, trailing)
	int shrink_buffer_indexes[3] = { 0, sections[0].TabCount + sections[2].TabCount, sections[0].TabCount };
	g.ShrinkWidthBuffer.resize(tab_bar->Tabs.Size);

	// Compute ideal tabs widths + store them into shrink buffer
	KGGuiTabItem* most_recently_selected_tab = NULL;
	int curr_section_n = -1;
	bool found_selected_tab_id = false;
	for (int tab_n = 0; tab_n < tab_bar->Tabs.Size; tab_n++)
	{
		KGGuiTabItem* tab = &tab_bar->Tabs[tab_n];
		KR_CORE_ASSERT(tab->LastFrameVisible >= tab_bar->PrevFrameVisible, "");

		if ((most_recently_selected_tab == NULL || most_recently_selected_tab->LastFrameSelected < tab->LastFrameSelected) && !(tab->Flags & KGGuiTabItemFlags_Button))
			most_recently_selected_tab = tab;
		if (tab->ID == tab_bar->SelectedTabId)
			found_selected_tab_id = true;
		if (scroll_to_tab_id == 0 && g.NavJustMovedToId == tab->ID)
			scroll_to_tab_id = tab->ID;

		// Refresh tab width immediately, otherwise changes of style e.g. style.FramePadding.x would noticeably lag in the tab bar.
		// Additionally, when using TabBarAddTab() to manipulate tab bar order we occasionally insert new tabs that don't have a width yet,
		// and we cannot wait for the next BeginTabItem() call. We cannot compute this width within TabBarAddTab() because font size depends on the active window.
		const char* tab_name = tab_bar->GetTabName(tab);
		const bool has_close_button_or_unsaved_marker = (tab->Flags & KGGuiTabItemFlags_NoCloseButton) == 0 || (tab->Flags & KGGuiTabItemFlags_UnsavedDocument);
		tab->ContentWidth = (tab->RequestedWidth >= 0.0f) ? tab->RequestedWidth : KarmaGuiInternal::TabItemCalcSize(tab_name, has_close_button_or_unsaved_marker).x;

		int section_n = TabItemGetSectionIdx(tab);
		KarmaGuiTabBarSection* section = &sections[section_n];
		section->Width += tab->ContentWidth + (section_n == curr_section_n ? g.Style.ItemInnerSpacing.x : 0.0f);
		curr_section_n = section_n;

		// Store data so we can build an array sorted by width if we need to shrink tabs down
		KG_MSVC_WARNING_SUPPRESS(6385);
		KGGuiShrinkWidthItem* shrink_width_item = &g.ShrinkWidthBuffer[shrink_buffer_indexes[section_n]++];
		shrink_width_item->Index = tab_n;
		shrink_width_item->Width = shrink_width_item->InitialWidth = tab->ContentWidth;
		tab->Width = KGMax(tab->ContentWidth, 1.0f);
	}

	// Compute total ideal width (used for e.g. auto-resizing a window)
	tab_bar->WidthAllTabsIdeal = 0.0f;
	for (int section_n = 0; section_n < 3; section_n++)
		tab_bar->WidthAllTabsIdeal += sections[section_n].Width + sections[section_n].Spacing;

	// Horizontal scrolling buttons
	// (note that TabBarScrollButtons() will alter BarRect.Max.x)
	if ((tab_bar->WidthAllTabsIdeal > tab_bar->BarRect.GetWidth() && tab_bar->Tabs.Size > 1) && !(tab_bar->Flags & KGGuiTabBarFlags_NoTabListScrollingButtons) && (tab_bar->Flags & KGGuiTabBarFlags_FittingPolicyScroll))
		if (KGGuiTabItem* scroll_and_select_tab = TabBarScrollingButtons(tab_bar))
		{
			scroll_to_tab_id = scroll_and_select_tab->ID;
			if ((scroll_and_select_tab->Flags & KGGuiTabItemFlags_Button) == 0)
				tab_bar->SelectedTabId = scroll_to_tab_id;
		}

	// Shrink widths if full tabs don't fit in their allocated space
	float section_0_w = sections[0].Width + sections[0].Spacing;
	float section_1_w = sections[1].Width + sections[1].Spacing;
	float section_2_w = sections[2].Width + sections[2].Spacing;
	bool central_section_is_visible = (section_0_w + section_2_w) < tab_bar->BarRect.GetWidth();
	float width_excess;
	if (central_section_is_visible)
		width_excess = KGMax(section_1_w - (tab_bar->BarRect.GetWidth() - section_0_w - section_2_w), 0.0f); // Excess used to shrink central section
	else
		width_excess = (section_0_w + section_2_w) - tab_bar->BarRect.GetWidth(); // Excess used to shrink leading/trailing section

	// With KGGuiTabBarFlags_FittingPolicyScroll policy, we will only shrink leading/trailing if the central section is not visible anymore
	if (width_excess >= 1.0f && ((tab_bar->Flags & KGGuiTabBarFlags_FittingPolicyResizeDown) || !central_section_is_visible))
	{
		int shrink_data_count = (central_section_is_visible ? sections[1].TabCount : sections[0].TabCount + sections[2].TabCount);
		int shrink_data_offset = (central_section_is_visible ? sections[0].TabCount + sections[2].TabCount : 0);
		KarmaGuiInternal::ShrinkWidths(g.ShrinkWidthBuffer.Data + shrink_data_offset, shrink_data_count, width_excess);

		// Apply shrunk values into tabs and sections
		for (int tab_n = shrink_data_offset; tab_n < shrink_data_offset + shrink_data_count; tab_n++)
		{
			KGGuiTabItem* tab = &tab_bar->Tabs[g.ShrinkWidthBuffer[tab_n].Index];
			float shrinked_width = KG_FLOOR(g.ShrinkWidthBuffer[tab_n].Width);
			if (shrinked_width < 0.0f)
				continue;

			shrinked_width = KGMax(1.0f, shrinked_width);
			int section_n = TabItemGetSectionIdx(tab);
			sections[section_n].Width -= (tab->Width - shrinked_width);
			tab->Width = shrinked_width;
		}
	}

	// Layout all active tabs
	int section_tab_index = 0;
	float tab_offset = 0.0f;
	tab_bar->WidthAllTabs = 0.0f;
	for (int section_n = 0; section_n < 3; section_n++)
	{
		KarmaGuiTabBarSection* section = &sections[section_n];
		if (section_n == 2)
			tab_offset = KGMin(KGMax(0.0f, tab_bar->BarRect.GetWidth() - section->Width), tab_offset);

		for (int tab_n = 0; tab_n < section->TabCount; tab_n++)
		{
			KGGuiTabItem* tab = &tab_bar->Tabs[section_tab_index + tab_n];
			tab->Offset = tab_offset;
			tab->NameOffset = -1;
			tab_offset += tab->Width + (tab_n < section->TabCount - 1 ? g.Style.ItemInnerSpacing.x : 0.0f);
		}
		tab_bar->WidthAllTabs += KGMax(section->Width + section->Spacing, 0.0f);
		tab_offset += section->Spacing;
		section_tab_index += section->TabCount;
	}

	// Clear name buffers
	tab_bar->TabsNames.Buf.resize(0);

	// If we have lost the selected tab, select the next most recently active one
	if (found_selected_tab_id == false)
		tab_bar->SelectedTabId = 0;
	if (tab_bar->SelectedTabId == 0 && tab_bar->NextSelectedTabId == 0 && most_recently_selected_tab != NULL)
		scroll_to_tab_id = tab_bar->SelectedTabId = most_recently_selected_tab->ID;

	// Lock in visible tab
	tab_bar->VisibleTabId = tab_bar->SelectedTabId;
	tab_bar->VisibleTabWasSubmitted = false;

	// CTRL+TAB can override visible tab temporarily
	if (g.NavWindowingTarget != NULL && g.NavWindowingTarget->DockNode && g.NavWindowingTarget->DockNode->TabBar == tab_bar)
		tab_bar->VisibleTabId = scroll_to_tab_id = g.NavWindowingTarget->TabId;

	// Update scrolling
	if (scroll_to_tab_id != 0)
		TabBarScrollToTab(tab_bar, scroll_to_tab_id, sections);
	tab_bar->ScrollingAnim = TabBarScrollClamp(tab_bar, tab_bar->ScrollingAnim);
	tab_bar->ScrollingTarget = TabBarScrollClamp(tab_bar, tab_bar->ScrollingTarget);
	if (tab_bar->ScrollingAnim != tab_bar->ScrollingTarget)
	{
		// Scrolling speed adjust itself so we can always reach our target in 1/3 seconds.
		// Teleport if we are aiming far off the visible line
		tab_bar->ScrollingSpeed = KGMax(tab_bar->ScrollingSpeed, 70.0f * g.FontSize);
		tab_bar->ScrollingSpeed = KGMax(tab_bar->ScrollingSpeed, KGFabs(tab_bar->ScrollingTarget - tab_bar->ScrollingAnim) / 0.3f);
		const bool teleport = (tab_bar->PrevFrameVisible + 1 < g.FrameCount) || (tab_bar->ScrollingTargetDistToVisibility > 10.0f * g.FontSize);
		tab_bar->ScrollingAnim = teleport ? tab_bar->ScrollingTarget : KGLinearSweep(tab_bar->ScrollingAnim, tab_bar->ScrollingTarget, g.IO.DeltaTime * tab_bar->ScrollingSpeed);
	}
	else
	{
		tab_bar->ScrollingSpeed = 0.0f;
	}
	tab_bar->ScrollingRectMinX = tab_bar->BarRect.Min.x + sections[0].Width + sections[0].Spacing;
	tab_bar->ScrollingRectMaxX = tab_bar->BarRect.Max.x - sections[2].Width - sections[1].Spacing;

	// Actual layout in host window (we don't do it in BeginTabBar() so as not to waste an extra frame)
	KGGuiWindow* window = g.CurrentWindow;
	window->DC.CursorPos = tab_bar->BarRect.Min;
	KarmaGuiInternal::ItemSize(KGVec2(tab_bar->WidthAllTabs, tab_bar->BarRect.GetHeight()), tab_bar->FramePadding.y);
	window->DC.IdealMaxPos.x = KGMax(window->DC.IdealMaxPos.x, tab_bar->BarRect.Min.x + tab_bar->WidthAllTabsIdeal);
}

// Dockable windows uses Name/ID in the global namespace. Non-dockable items use the ID stack.
KGU32 Karma::TabBarCalcTabID(KGGuiTabBar* tab_bar, const char* label, KGGuiWindow* docked_window)
{
	if (docked_window != NULL)
	{
		KG_UNUSED(tab_bar);
		KR_CORE_ASSERT(tab_bar->Flags & KGGuiTabBarFlags_DockNode, "");
		KGGuiID id = docked_window->TabId;
		KarmaGuiInternal::KeepAliveID(id);
		return id;
	}
	else
	{
		KGGuiWindow* window = GKarmaGui->CurrentWindow;
		return window->GetID(label);
	}
}

float Karma::TabBarCalcMaxTabWidth()
{
	KarmaGuiContext& g = *GKarmaGui;
	return g.FontSize * 20.0f;
}

KGGuiTabItem* Karma::KarmaGuiInternal::TabBarFindTabByID(KGGuiTabBar* tab_bar, KGGuiID tab_id)
{
	if (tab_id != 0)
		for (int n = 0; n < tab_bar->Tabs.Size; n++)
			if (tab_bar->Tabs[n].ID == tab_id)
				return &tab_bar->Tabs[n];
	return NULL;
}

// FIXME: See references to #2304 in TODO.txt
KGGuiTabItem* Karma::KarmaGuiInternal::TabBarFindMostRecentlySelectedTabForActiveWindow(KGGuiTabBar* tab_bar)
{
	KGGuiTabItem* most_recently_selected_tab = NULL;
	for (int tab_n = 0; tab_n < tab_bar->Tabs.Size; tab_n++)
	{
		KGGuiTabItem* tab = &tab_bar->Tabs[tab_n];
		if (most_recently_selected_tab == NULL || most_recently_selected_tab->LastFrameSelected < tab->LastFrameSelected)
			if (tab->Window && tab->Window->WasActive)
				most_recently_selected_tab = tab;
	}
	return most_recently_selected_tab;
}

// The purpose of this call is to register tab in advance so we can control their order at the time they appear.
// Otherwise calling this is unnecessary as tabs are appending as needed by the BeginTabItem() function.
void Karma::KarmaGuiInternal::TabBarAddTab(KGGuiTabBar* tab_bar, KarmaGuiTabItemFlags tab_flags, KGGuiWindow* window)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(TabBarFindTabByID(tab_bar, window->TabId) == NULL, "");
	KR_CORE_ASSERT(g.CurrentTabBar != tab_bar, "");  // Can't work while the tab bar is active as our tab doesn't have an X offset yet, in theory we could/should test something like (tab_bar->CurrFrameVisible < g.FrameCount) but we'd need to solve why triggers the commented early-out assert in BeginTabBarEx() (probably dock node going from implicit to explicit in same frame)

	if (!window->HasCloseButton)
		tab_flags |= KGGuiTabItemFlags_NoCloseButton;       // Set _NoCloseButton immediately because it will be used for first-frame width calculation.

	KGGuiTabItem new_tab;
	new_tab.ID = window->TabId;
	new_tab.Flags = tab_flags;
	new_tab.LastFrameVisible = tab_bar->CurrFrameVisible;   // Required so BeginTabBar() doesn't ditch the tab
	if (new_tab.LastFrameVisible == -1)
		new_tab.LastFrameVisible = g.FrameCount - 1;
	new_tab.Window = window;                                // Required so tab bar layout can compute the tab width before tab submission
	tab_bar->Tabs.push_back(new_tab);
}

// The *TabId fields be already set by the docking system _before_ the actual TabItem was created, so we clear them regardless.
void Karma::KarmaGuiInternal::TabBarRemoveTab(KGGuiTabBar* tab_bar, KGGuiID tab_id)
{
	if (KGGuiTabItem* tab = TabBarFindTabByID(tab_bar, tab_id))
		tab_bar->Tabs.erase(tab);
	if (tab_bar->VisibleTabId == tab_id) { tab_bar->VisibleTabId = 0; }
	if (tab_bar->SelectedTabId == tab_id) { tab_bar->SelectedTabId = 0; }
	if (tab_bar->NextSelectedTabId == tab_id) { tab_bar->NextSelectedTabId = 0; }
}

// Called on manual closure attempt
void Karma::KarmaGuiInternal::TabBarCloseTab(KGGuiTabBar* tab_bar, KGGuiTabItem* tab)
{
	if (tab->Flags & KGGuiTabItemFlags_Button)
		return; // A button appended with TabItemButton().

	if (!(tab->Flags & KGGuiTabItemFlags_UnsavedDocument))
	{
		// This will remove a frame of lag for selecting another tab on closure.
		// However we don't run it in the case where the 'Unsaved' flag is set, so user gets a chance to fully undo the closure
		tab->WantClose = true;
		if (tab_bar->VisibleTabId == tab->ID)
		{
			tab->LastFrameVisible = -1;
			tab_bar->SelectedTabId = tab_bar->NextSelectedTabId = 0;
		}
	}
	else
	{
		// Actually select before expecting closure attempt (on an UnsavedDocument tab user is expect to e.g. show a popup)
		if (tab_bar->VisibleTabId != tab->ID)
			tab_bar->NextSelectedTabId = tab->ID;
	}
}

static float Karma::TabBarScrollClamp(KGGuiTabBar* tab_bar, float scrolling)
{
	scrolling = KGMin(scrolling, tab_bar->WidthAllTabs - tab_bar->BarRect.GetWidth());
	return KGMax(scrolling, 0.0f);
}

// Note: we may scroll to tab that are not selected! e.g. using keyboard arrow keys
void Karma::TabBarScrollToTab(KGGuiTabBar* tab_bar, KGGuiID tab_id, KarmaGuiTabBarSection* sections)
{
	KGGuiTabItem* tab = KarmaGuiInternal::TabBarFindTabByID(tab_bar, tab_id);
	if (tab == NULL)
		return;
	if (tab->Flags & KGGuiTabItemFlags_SectionMask_)
		return;

	KarmaGuiContext& g = *GKarmaGui;
	float margin = g.FontSize * 1.0f; // When to scroll to make Tab N+1 visible always make a bit of N visible to suggest more scrolling area (since we don't have a scrollbar)
	int order = tab_bar->GetTabOrder(tab);

	// Scrolling happens only in the central section (leading/trailing sections are not scrolling)
	// FIXME: This is all confusing.
	float scrollable_width = tab_bar->BarRect.GetWidth() - sections[0].Width - sections[2].Width - sections[1].Spacing;

	// We make all tabs positions all relative Sections[0].Width to make code simpler
	float tab_x1 = tab->Offset - sections[0].Width + (order > sections[0].TabCount - 1 ? -margin : 0.0f);
	float tab_x2 = tab->Offset - sections[0].Width + tab->Width + (order + 1 < tab_bar->Tabs.Size - sections[2].TabCount ? margin : 1.0f);
	tab_bar->ScrollingTargetDistToVisibility = 0.0f;
	if (tab_bar->ScrollingTarget > tab_x1 || (tab_x2 - tab_x1 >= scrollable_width))
	{
		// Scroll to the left
		tab_bar->ScrollingTargetDistToVisibility = KGMax(tab_bar->ScrollingAnim - tab_x2, 0.0f);
		tab_bar->ScrollingTarget = tab_x1;
	}
	else if (tab_bar->ScrollingTarget < tab_x2 - scrollable_width)
	{
		// Scroll to the right
		tab_bar->ScrollingTargetDistToVisibility = KGMax((tab_x1 - scrollable_width) - tab_bar->ScrollingAnim, 0.0f);
		tab_bar->ScrollingTarget = tab_x2 - scrollable_width;
	}
}

void Karma::KarmaGuiInternal::TabBarQueueReorder(KGGuiTabBar* tab_bar, const KGGuiTabItem* tab, int offset)
{
	KR_CORE_ASSERT(offset != 0, "");
	KR_CORE_ASSERT(tab_bar->ReorderRequestTabId == 0, "");
	tab_bar->ReorderRequestTabId = tab->ID;
	tab_bar->ReorderRequestOffset = (KGS16)offset;
}

void Karma::KarmaGuiInternal::TabBarQueueReorderFromMousePos(KGGuiTabBar* tab_bar, const KGGuiTabItem* src_tab, KGVec2 mouse_pos)
{
	KarmaGuiContext& g = *GKarmaGui;
	KR_CORE_ASSERT(tab_bar->ReorderRequestTabId == 0, "");
	if ((tab_bar->Flags & KGGuiTabBarFlags_Reorderable) == 0)
		return;

	const bool is_central_section = (src_tab->Flags & KGGuiTabItemFlags_SectionMask_) == 0;
	const float bar_offset = tab_bar->BarRect.Min.x - (is_central_section ? tab_bar->ScrollingTarget : 0);

	// Count number of contiguous tabs we are crossing over
	const int dir = (bar_offset + src_tab->Offset) > mouse_pos.x ? -1 : +1;
	const int src_idx = tab_bar->Tabs.index_from_ptr(src_tab);
	int dst_idx = src_idx;
	for (int i = src_idx; i >= 0 && i < tab_bar->Tabs.Size; i += dir)
	{
		// Reordered tabs must share the same section
		const KGGuiTabItem* dst_tab = &tab_bar->Tabs[i];
		if (dst_tab->Flags & KGGuiTabItemFlags_NoReorder)
			break;
		if ((dst_tab->Flags & KGGuiTabItemFlags_SectionMask_) != (src_tab->Flags & KGGuiTabItemFlags_SectionMask_))
			break;
		dst_idx = i;

		// Include spacing after tab, so when mouse cursor is between tabs we would not continue checking further tabs that are not hovered.
		const float x1 = bar_offset + dst_tab->Offset - g.Style.ItemInnerSpacing.x;
		const float x2 = bar_offset + dst_tab->Offset + dst_tab->Width + g.Style.ItemInnerSpacing.x;
		//GetForegroundDrawList()->AddRect(KGVec2(x1, tab_bar->BarRect.Min.y), KGVec2(x2, tab_bar->BarRect.Max.y), KG_COL32(255, 0, 0, 255));
		if ((dir < 0 && mouse_pos.x > x1) || (dir > 0 && mouse_pos.x < x2))
			break;
	}

	if (dst_idx != src_idx)
		TabBarQueueReorder(tab_bar, src_tab, dst_idx - src_idx);
}

bool Karma::KarmaGuiInternal::TabBarProcessReorder(KGGuiTabBar* tab_bar)
{
	KGGuiTabItem* tab1 = TabBarFindTabByID(tab_bar, tab_bar->ReorderRequestTabId);
	if (tab1 == NULL || (tab1->Flags & KGGuiTabItemFlags_NoReorder))
		return false;

	//KR_CORE_ASSERT(tab_bar->Flags & KGGuiTabBarFlags_Reorderable); // <- this may happen when using debug tools
	int tab2_order = tab_bar->GetTabOrder(tab1) + tab_bar->ReorderRequestOffset;
	if (tab2_order < 0 || tab2_order >= tab_bar->Tabs.Size)
		return false;

	// Reordered tabs must share the same section
	// (Note: TabBarQueueReorderFromMousePos() also has a similar test but since we allow direct calls to TabBarQueueReorder() we do it here too)
	KGGuiTabItem* tab2 = &tab_bar->Tabs[tab2_order];
	if (tab2->Flags & KGGuiTabItemFlags_NoReorder)
		return false;
	if ((tab1->Flags & KGGuiTabItemFlags_SectionMask_) != (tab2->Flags & KGGuiTabItemFlags_SectionMask_))
		return false;

	KGGuiTabItem item_tmp = *tab1;
	KGGuiTabItem* src_tab = (tab_bar->ReorderRequestOffset > 0) ? tab1 + 1 : tab2;
	KGGuiTabItem* dst_tab = (tab_bar->ReorderRequestOffset > 0) ? tab1 : tab2 + 1;
	const int move_count = (tab_bar->ReorderRequestOffset > 0) ? tab_bar->ReorderRequestOffset : -tab_bar->ReorderRequestOffset;
	memmove(dst_tab, src_tab, move_count * sizeof(KGGuiTabItem));
	*tab2 = item_tmp;

	if (tab_bar->Flags & KGGuiTabBarFlags_SaveSettings)
		MarkIniSettingsDirty();
	return true;
}

KGGuiTabItem* Karma::TabBarScrollingButtons(KGGuiTabBar* tab_bar)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;

	const KGVec2 arrow_button_size(g.FontSize - 2.0f, g.FontSize + g.Style.FramePadding.y * 2.0f);
	const float scrolling_buttons_width = arrow_button_size.x * 2.0f;

	const KGVec2 backup_cursor_pos = window->DC.CursorPos;
	//window->DrawList->AddRect(KGVec2(tab_bar->BarRect.Max.x - scrolling_buttons_width, tab_bar->BarRect.Min.y), KGVec2(tab_bar->BarRect.Max.x, tab_bar->BarRect.Max.y), KG_COL32(255,0,0,255));

	int select_dir = 0;
	KGVec4 arrow_col = g.Style.Colors[KGGuiCol_Text];
	arrow_col.w *= 0.5f;

	KarmaGui::PushStyleColor(KGGuiCol_Text, arrow_col);
	KarmaGui::PushStyleColor(KGGuiCol_Button, KGVec4(0, 0, 0, 0));
	const float backup_repeat_delay = g.IO.KeyRepeatDelay;
	const float backup_repeat_rate = g.IO.KeyRepeatRate;
	g.IO.KeyRepeatDelay = 0.250f;
	g.IO.KeyRepeatRate = 0.200f;
	float x = KGMax(tab_bar->BarRect.Min.x, tab_bar->BarRect.Max.x - scrolling_buttons_width);
	window->DC.CursorPos = KGVec2(x, tab_bar->BarRect.Min.y);
	if (KarmaGuiInternal::ArrowButtonEx("##<", KGGuiDir_Left, arrow_button_size, KGGuiButtonFlags_PressedOnClick | KGGuiButtonFlags_Repeat))
		select_dir = -1;
	window->DC.CursorPos = KGVec2(x + arrow_button_size.x, tab_bar->BarRect.Min.y);
	if (KarmaGuiInternal::ArrowButtonEx("##>", KGGuiDir_Right, arrow_button_size, KGGuiButtonFlags_PressedOnClick | KGGuiButtonFlags_Repeat))
		select_dir = +1;
	KarmaGui::PopStyleColor(2);
	g.IO.KeyRepeatRate = backup_repeat_rate;
	g.IO.KeyRepeatDelay = backup_repeat_delay;

	KGGuiTabItem* tab_to_scroll_to = NULL;
	if (select_dir != 0)
		if (KGGuiTabItem* tab_item = KarmaGuiInternal::TabBarFindTabByID(tab_bar, tab_bar->SelectedTabId))
		{
			int selected_order = tab_bar->GetTabOrder(tab_item);
			int target_order = selected_order + select_dir;

			// Skip tab item buttons until another tab item is found or end is reached
			while (tab_to_scroll_to == NULL)
			{
				// If we are at the end of the list, still scroll to make our tab visible
				tab_to_scroll_to = &tab_bar->Tabs[(target_order >= 0 && target_order < tab_bar->Tabs.Size) ? target_order : selected_order];

				// Cross through buttons
				// (even if first/last item is a button, return it so we can update the scroll)
				if (tab_to_scroll_to->Flags & KGGuiTabItemFlags_Button)
				{
					target_order += select_dir;
					selected_order += select_dir;
					tab_to_scroll_to = (target_order < 0 || target_order >= tab_bar->Tabs.Size) ? tab_to_scroll_to : NULL;
				}
			}
		}
	window->DC.CursorPos = backup_cursor_pos;
	tab_bar->BarRect.Max.x -= scrolling_buttons_width + 1.0f;

	return tab_to_scroll_to;
}

KGGuiTabItem* Karma::TabBarTabListPopupButton(KGGuiTabBar* tab_bar)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;

	// We use g.Style.FramePadding.y to match the square ArrowButton size
	const float tab_list_popup_button_width = g.FontSize + g.Style.FramePadding.y;
	const KGVec2 backup_cursor_pos = window->DC.CursorPos;
	window->DC.CursorPos = KGVec2(tab_bar->BarRect.Min.x - g.Style.FramePadding.y, tab_bar->BarRect.Min.y);
	tab_bar->BarRect.Min.x += tab_list_popup_button_width;

	KGVec4 arrow_col = g.Style.Colors[KGGuiCol_Text];
	arrow_col.w *= 0.5f;
	KarmaGui::PushStyleColor(KGGuiCol_Text, arrow_col);
	KarmaGui::PushStyleColor(KGGuiCol_Button, KGVec4(0, 0, 0, 0));
	bool open = KarmaGui::BeginCombo("##v", NULL, KGGuiComboFlags_NoPreview | KGGuiComboFlags_HeightLargest);
	KarmaGui::PopStyleColor(2);

	KGGuiTabItem* tab_to_select = NULL;
	if (open)
	{
		for (int tab_n = 0; tab_n < tab_bar->Tabs.Size; tab_n++)
		{
			KGGuiTabItem* tab = &tab_bar->Tabs[tab_n];
			if (tab->Flags & KGGuiTabItemFlags_Button)
				continue;

			const char* tab_name = tab_bar->GetTabName(tab);
			if (KarmaGui::Selectable(tab_name, tab_bar->SelectedTabId == tab->ID))
				tab_to_select = tab;
		}
		KarmaGui::EndCombo();
	}

	window->DC.CursorPos = backup_cursor_pos;
	return tab_to_select;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: BeginTabItem, EndTabItem, etc.
//-------------------------------------------------------------------------
// - BeginTabItem()
// - EndTabItem()
// - TabItemButton()
// - TabItemEx() [Internal]
// - SetTabItemClosed()
// - TabItemCalcSize() [Internal]
// - TabItemBackground() [Internal]
// - TabItemLabelAndCloseButton() [Internal]
//-------------------------------------------------------------------------

bool Karma::KarmaGui::BeginTabItem(const char* label, bool* p_open, KarmaGuiTabItemFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	if (window->SkipItems)
		return false;

	KGGuiTabBar* tab_bar = g.CurrentTabBar;
	if (tab_bar == NULL)
	{
		KR_CORE_ASSERT(tab_bar, "Needs to be called between BeginTabBar() and EndTabBar()!");
		return false;
	}
	KR_CORE_ASSERT((flags & KGGuiTabItemFlags_Button) == 0, "");             // BeginTabItem() Can't be used with button flags, use TabItemButton() instead!

	bool ret = KarmaGuiInternal::TabItemEx(tab_bar, label, p_open, flags, NULL);
	if (ret && !(flags & KGGuiTabItemFlags_NoPushId))
	{
		KGGuiTabItem* tab = &tab_bar->Tabs[tab_bar->LastTabItemIdx];
		KarmaGuiInternal::PushOverrideID(tab->ID); // We already hashed 'label' so push into the ID stack directly instead of doing another hash through PushID(label)
	}
	return ret;
}

void  Karma::KarmaGui::EndTabItem()
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	if (window->SkipItems)
		return;

	KGGuiTabBar* tab_bar = g.CurrentTabBar;
	if (tab_bar == NULL)
	{
		KR_CORE_ASSERT(tab_bar != NULL, "Needs to be called between BeginTabBar() and EndTabBar()!");
		return;
	}
	KR_CORE_ASSERT(tab_bar->LastTabItemIdx >= 0, "");
	KGGuiTabItem* tab = &tab_bar->Tabs[tab_bar->LastTabItemIdx];
	if (!(tab->Flags & KGGuiTabItemFlags_NoPushId))
		PopID();
}

bool  Karma::KarmaGui::TabItemButton(const char* label, KarmaGuiTabItemFlags flags)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGGuiWindow* window = g.CurrentWindow;
	if (window->SkipItems)
		return false;

	KGGuiTabBar* tab_bar = g.CurrentTabBar;
	if (tab_bar == NULL)
	{
		KR_CORE_ASSERT(tab_bar != NULL, "Needs to be called between BeginTabBar() and EndTabBar()!");
		return false;
	}
	return KarmaGuiInternal::TabItemEx(tab_bar, label, NULL, flags | KGGuiTabItemFlags_Button | KGGuiTabItemFlags_NoReorder, NULL);
}

bool  Karma::KarmaGuiInternal::TabItemEx(KGGuiTabBar* tab_bar, const char* label, bool* p_open, KarmaGuiTabItemFlags flags, KGGuiWindow* docked_window)
{
	// Layout whole tab bar if not already done
	KarmaGuiContext& g = *GKarmaGui;
	if (tab_bar->WantLayout)
	{
		KGGuiNextItemData backup_next_item_data = g.NextItemData;
		TabBarLayout(tab_bar);
		g.NextItemData = backup_next_item_data;
	}
	KGGuiWindow* window = g.CurrentWindow;
	if (window->SkipItems)
		return false;

	const KarmaGuiStyle& style = g.Style;
	const KGGuiID id = TabBarCalcTabID(tab_bar, label, docked_window);

	// If the user called us with *p_open == false, we early out and don't render.
	// We make a call to ItemAdd() so that attempts to use a contextual popup menu with an implicit ID won't use an older ID.
	KARMAGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
	if (p_open && !*p_open)
	{
		ItemAdd(KGRect(), id, NULL, KGGuiItemFlags_NoNav);
		return false;
	}

	KR_CORE_ASSERT(!p_open || !(flags & KGGuiTabItemFlags_Button), "");
	KR_CORE_ASSERT((flags & (KGGuiTabItemFlags_Leading | KGGuiTabItemFlags_Trailing)) != (KGGuiTabItemFlags_Leading | KGGuiTabItemFlags_Trailing), ""); // Can't use both Leading and Trailing

	// Store into KGGuiTabItemFlags_NoCloseButton, also honor KGGuiTabItemFlags_NoCloseButton passed by user (although not documented)
	if (flags & KGGuiTabItemFlags_NoCloseButton)
		p_open = NULL;
	else if (p_open == NULL)
		flags |= KGGuiTabItemFlags_NoCloseButton;

	// Acquire tab data
	KGGuiTabItem* tab = TabBarFindTabByID(tab_bar, id);
	bool tab_is_new = false;
	if (tab == NULL)
	{
		tab_bar->Tabs.push_back(KGGuiTabItem());
		tab = &tab_bar->Tabs.back();
		tab->ID = id;
		tab_bar->TabsAddedNew = tab_is_new = true;
	}
	tab_bar->LastTabItemIdx = (KGS16)tab_bar->Tabs.index_from_ptr(tab);

	// Calculate tab contents size
	KGVec2 size = TabItemCalcSize(label, (p_open != NULL) || (flags & KGGuiTabItemFlags_UnsavedDocument));
	tab->RequestedWidth = -1.0f;
	if (g.NextItemData.Flags & KGGuiNextItemDataFlags_HasWidth)
		size.x = tab->RequestedWidth = g.NextItemData.Width;
	if (tab_is_new)
		tab->Width = KGMax(1.0f, size.x);
	tab->ContentWidth = size.x;
	tab->BeginOrder = tab_bar->TabsActiveCount++;

	const bool tab_bar_appearing = (tab_bar->PrevFrameVisible + 1 < g.FrameCount);
	const bool tab_bar_focused = (tab_bar->Flags & KGGuiTabBarFlags_IsFocused) != 0;
	const bool tab_appearing = (tab->LastFrameVisible + 1 < g.FrameCount);
	const bool tab_just_unsaved = (flags & KGGuiTabItemFlags_UnsavedDocument) && !(tab->Flags & KGGuiTabItemFlags_UnsavedDocument);
	const bool is_tab_button = (flags & KGGuiTabItemFlags_Button) != 0;
	tab->LastFrameVisible = g.FrameCount;
	tab->Flags = flags;
	tab->Window = docked_window;

	// Append name _WITH_ the zero-terminator
	// (regular tabs are permitted in a DockNode tab bar, but window tabs not permitted in a non-DockNode tab bar)
	if (docked_window != NULL)
	{
		KR_CORE_ASSERT(tab_bar->Flags & KGGuiTabBarFlags_DockNode, "");
		tab->NameOffset = -1;
	}
	else
	{
		tab->NameOffset = (KGS32)tab_bar->TabsNames.size();
		tab_bar->TabsNames.append(label, label + strlen(label) + 1);
	}

	// Update selected tab
	if (!is_tab_button)
	{
		if (tab_appearing && (tab_bar->Flags & KGGuiTabBarFlags_AutoSelectNewTabs) && tab_bar->NextSelectedTabId == 0)
			if (!tab_bar_appearing || tab_bar->SelectedTabId == 0)
				tab_bar->NextSelectedTabId = id;  // New tabs gets activated
		if ((flags & KGGuiTabItemFlags_SetSelected) && (tab_bar->SelectedTabId != id)) // _SetSelected can only be passed on explicit tab bar
			tab_bar->NextSelectedTabId = id;
	}

	// Lock visibility
	// (Note: tab_contents_visible != tab_selected... because CTRL+TAB operations may preview some tabs without selecting them!)
	bool tab_contents_visible = (tab_bar->VisibleTabId == id);
	if (tab_contents_visible)
		tab_bar->VisibleTabWasSubmitted = true;

	// On the very first frame of a tab bar we let first tab contents be visible to minimize appearing glitches
	if (!tab_contents_visible && tab_bar->SelectedTabId == 0 && tab_bar_appearing && docked_window == NULL)
		if (tab_bar->Tabs.Size == 1 && !(tab_bar->Flags & KGGuiTabBarFlags_AutoSelectNewTabs))
			tab_contents_visible = true;

	// Note that tab_is_new is not necessarily the same as tab_appearing! When a tab bar stops being submitted
	// and then gets submitted again, the tabs will have 'tab_appearing=true' but 'tab_is_new=false'.
	if (tab_appearing && (!tab_bar_appearing || tab_is_new))
	{
		ItemAdd(KGRect(), id, NULL, KGGuiItemFlags_NoNav);
		if (is_tab_button)
			return false;
		return tab_contents_visible;
	}

	if (tab_bar->SelectedTabId == id)
		tab->LastFrameSelected = g.FrameCount;

	// Backup current layout position
	const KGVec2 backup_main_cursor_pos = window->DC.CursorPos;

	// Layout
	const bool is_central_section = (tab->Flags & KGGuiTabItemFlags_SectionMask_) == 0;
	size.x = tab->Width;
	if (is_central_section)
		window->DC.CursorPos = tab_bar->BarRect.Min + KGVec2(KG_FLOOR(tab->Offset - tab_bar->ScrollingAnim), 0.0f);
	else
		window->DC.CursorPos = tab_bar->BarRect.Min + KGVec2(tab->Offset, 0.0f);
	KGVec2 pos = window->DC.CursorPos;
	KGRect bb(pos, pos + size);

	// We don't have CPU clipping primitives to clip the CloseButton (until it becomes a texture), so need to add an extra draw call (temporary in the case of vertical animation)
	const bool want_clip_rect = is_central_section && (bb.Min.x < tab_bar->ScrollingRectMinX || bb.Max.x > tab_bar->ScrollingRectMaxX);
	if (want_clip_rect)
		KarmaGui::PushClipRect(KGVec2(KGMax(bb.Min.x, tab_bar->ScrollingRectMinX), bb.Min.y - 1), KGVec2(tab_bar->ScrollingRectMaxX, bb.Max.y), true);

	KGVec2 backup_cursor_max_pos = window->DC.CursorMaxPos;
	ItemSize(bb.GetSize(), style.FramePadding.y);
	window->DC.CursorMaxPos = backup_cursor_max_pos;

	if (!ItemAdd(bb, id))
	{
		if (want_clip_rect)
			KarmaGui::PopClipRect();
		window->DC.CursorPos = backup_main_cursor_pos;
		return tab_contents_visible;
	}

	// Click to Select a tab
	KarmaGuiButtonFlags button_flags = ((is_tab_button ? KGGuiButtonFlags_PressedOnClickRelease : KGGuiButtonFlags_PressedOnClick) | KGGuiButtonFlags_AllowItemOverlap);
	if (g.DragDropActive && !g.DragDropPayload.IsDataType(KARMAGUI_PAYLOAD_TYPE_WINDOW)) // FIXME: May be an opt-in property of the payload to disable this
		button_flags |= KGGuiButtonFlags_PressedOnDragDropHold;
	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);
	if (pressed && !is_tab_button)
		tab_bar->NextSelectedTabId = id;

	// Transfer active id window so the active id is not owned by the dock host (as StartMouseMovingWindow()
	// will only do it on the drag). This allows FocusWindow() to be more conservative in how it clears active id.
	if (held && docked_window && g.ActiveId == id && g.ActiveIdIsJustActivated)
		g.ActiveIdWindow = docked_window;

	// Allow the close button to overlap unless we are dragging (in which case we don't want any overlapping tabs to be hovered)
	if (g.ActiveId != id)
		KarmaGui::SetItemAllowOverlap();

	// Drag and drop a single floating window node moves it
	KGGuiDockNode* node = docked_window ? docked_window->DockNode : NULL;
	const bool single_floating_window_node = node && node->IsFloatingNode() && (node->Windows.Size == 1);
	if (held && single_floating_window_node && KarmaGui::IsMouseDragging(0, 0.0f))
	{
		// Move
		StartMouseMovingWindow(docked_window);
	}
	else if (held && !tab_appearing && KarmaGui::IsMouseDragging(0))
	{
		// Drag and drop: re-order tabs
		int drag_dir = 0;
		float drag_distance_from_edge_x = 0.0f;
		if (!g.DragDropActive && ((tab_bar->Flags & KGGuiTabBarFlags_Reorderable) || (docked_window != NULL)))
		{
			// While moving a tab it will jump on the other side of the mouse, so we also test for MouseDelta.x
			if (g.IO.MouseDelta.x < 0.0f && g.IO.MousePos.x < bb.Min.x)
			{
				drag_dir = -1;
				drag_distance_from_edge_x = bb.Min.x - g.IO.MousePos.x;
				TabBarQueueReorderFromMousePos(tab_bar, tab, g.IO.MousePos);
			}
			else if (g.IO.MouseDelta.x > 0.0f && g.IO.MousePos.x > bb.Max.x)
			{
				drag_dir = +1;
				drag_distance_from_edge_x = g.IO.MousePos.x - bb.Max.x;
				TabBarQueueReorderFromMousePos(tab_bar, tab, g.IO.MousePos);
			}
		}

		// Extract a Dockable window out of it's tab bar
		if (docked_window != NULL && !(docked_window->Flags & KGGuiWindowFlags_NoMove))
		{
			// We use a variable threshold to distinguish dragging tabs within a tab bar and extracting them out of the tab bar
			bool undocking_tab = (g.DragDropActive && g.DragDropPayload.SourceId == id);
			if (!undocking_tab) //&& (!g.IO.ConfigDockingWithShift || g.IO.KeyShift)
			{
				float threshold_base = g.FontSize;
				float threshold_x = (threshold_base * 2.2f);
				float threshold_y = (threshold_base * 1.5f) + KGClamp((KGFabs(g.IO.MouseDragMaxDistanceAbs[0].x) - threshold_base * 2.0f) * 0.20f, 0.0f, threshold_base * 4.0f);
				//GetForegroundDrawList()->AddRect(KGVec2(bb.Min.x - threshold_x, bb.Min.y - threshold_y), KGVec2(bb.Max.x + threshold_x, bb.Max.y + threshold_y), KG_COL32_WHITE); // [DEBUG]

				float distance_from_edge_y = KGMax(bb.Min.y - g.IO.MousePos.y, g.IO.MousePos.y - bb.Max.y);
				if (distance_from_edge_y >= threshold_y)
					undocking_tab = true;
				if (drag_distance_from_edge_x > threshold_x)
					if ((drag_dir < 0 && tab_bar->GetTabOrder(tab) == 0) || (drag_dir > 0 && tab_bar->GetTabOrder(tab) == tab_bar->Tabs.Size - 1))
						undocking_tab = true;
			}

			if (undocking_tab)
			{
				// Undock
				// FIXME: refactor to share more code with e.g. StartMouseMovingWindow
				DockContextQueueUndockWindow(&g, docked_window);
				g.MovingWindow = docked_window;
				SetActiveID(g.MovingWindow->MoveId, g.MovingWindow);
				g.ActiveIdClickOffset -= g.MovingWindow->Pos - bb.Min;
				g.ActiveIdNoClearOnFocusLoss = true;
				SetActiveIdUsingAllKeyboardKeys();
			}
		}
	}

#if 0
	if (hovered && g.HoveredIdNotActiveTimer > TOOLTIP_DELAY && bb.GetWidth() < tab->ContentWidth)
	{
		// Enlarge tab display when hovering
		bb.Max.x = bb.Min.x + KG_FLOOR(KGLerp(bb.GetWidth(), tab->ContentWidth, KGSaturate((g.HoveredIdNotActiveTimer - 0.40f) * 6.0f)));
		display_draw_list = GetForegroundDrawList(window);
		TabItemBackground(display_draw_list, bb, flags, GetColorU32(KGGuiCol_TitleBgActive));
	}
#endif

	// Render tab shape
	KGDrawList* display_draw_list = window->DrawList;
	const KGU32 tab_col = KarmaGui::GetColorU32((held || hovered) ? KGGuiCol_TabHovered : tab_contents_visible ? (tab_bar_focused ? KGGuiCol_TabActive : KGGuiCol_TabUnfocusedActive) : (tab_bar_focused ? KGGuiCol_Tab : KGGuiCol_TabUnfocused));
	TabItemBackground(display_draw_list, bb, flags, tab_col);
	RenderNavHighlight(bb, id);

	// Select with right mouse button. This is so the common idiom for context menu automatically highlight the current widget.
	const bool hovered_unblocked = KarmaGui::IsItemHovered(KGGuiHoveredFlags_AllowWhenBlockedByPopup);
	if (hovered_unblocked && (KarmaGui::IsMouseClicked(1) || KarmaGui::IsMouseReleased(1)))
		if (!is_tab_button)
			tab_bar->NextSelectedTabId = id;

	if (tab_bar->Flags & KGGuiTabBarFlags_NoCloseWithMiddleMouseButton)
		flags |= KGGuiTabItemFlags_NoCloseWithMiddleMouseButton;

	// Render tab label, process close button
	const KGGuiID close_button_id = p_open ? GetIDWithSeed("#CLOSE", NULL, docked_window ? docked_window->ID : id) : 0;
	bool just_closed;
	bool text_clipped;
	TabItemLabelAndCloseButton(display_draw_list, bb, tab_just_unsaved ? (flags & ~KGGuiTabItemFlags_UnsavedDocument) : flags, tab_bar->FramePadding, label, id, close_button_id, tab_contents_visible, &just_closed, &text_clipped);
	if (just_closed && p_open != NULL)
	{
		*p_open = false;
		TabBarCloseTab(tab_bar, tab);
	}

	// Forward Hovered state so IsItemHovered() after Begin() can work (even though we are technically hovering our parent)
	// That state is copied to window->DockTabItemStatusFlags by our caller.
	if (docked_window && (hovered || g.HoveredId == close_button_id))
		g.LastItemData.StatusFlags |= KGGuiItemStatusFlags_HoveredWindow;

	// Restore main window position so user can draw there
	if (want_clip_rect)
		KarmaGui::PopClipRect();
	window->DC.CursorPos = backup_main_cursor_pos;

	// Tooltip
	// (Won't work over the close button because ItemOverlap systems messes up with HoveredIdTimer-> seems ok)
	// (We test IsItemHovered() to discard e.g. when another item is active or drag and drop over the tab bar, which g.HoveredId ignores)
	// FIXME: This is a mess.
	// FIXME: We may want disabled tab to still display the tooltip?
	if (text_clipped && g.HoveredId == id && !held)
		if (!(tab_bar->Flags & KGGuiTabBarFlags_NoTooltip) && !(tab->Flags & KGGuiTabItemFlags_NoTooltip))
			if (KarmaGui::IsItemHovered(KGGuiHoveredFlags_DelayNormal))
				Karma::KarmaGui::SetTooltip("%.*s", (int)(FindRenderedTextEnd(label) - label), label);

	KR_CORE_ASSERT(!is_tab_button || !(tab_bar->SelectedTabId == tab->ID && is_tab_button), ""); // TabItemButton should not be selected
	if (is_tab_button)
		return pressed;
	return tab_contents_visible;
}

// [Public] This is call is 100% optional but it allows to remove some one-frame glitches when a tab has been unexpectedly removed.
// To use it to need to call the function SetTabItemClosed() between BeginTabBar() and EndTabBar().
// Tabs closed by the close button will automatically be flagged to avoid this issue.
void    Karma::KarmaGui::SetTabItemClosed(const char* label)
{
	KarmaGuiContext& g = *GKarmaGui;
	bool is_within_manual_tab_bar = g.CurrentTabBar && !(g.CurrentTabBar->Flags & KGGuiTabBarFlags_DockNode);
	if (is_within_manual_tab_bar)
	{
		KGGuiTabBar* tab_bar = g.CurrentTabBar;
		KGGuiID tab_id = TabBarCalcTabID(tab_bar, label, NULL);
		if (KGGuiTabItem* tab = KarmaGuiInternal::TabBarFindTabByID(tab_bar, tab_id))
			tab->WantClose = true; // Will be processed by next call to TabBarLayout()
	}
	else if (KGGuiWindow* window = KarmaGuiInternal::FindWindowByName(label))
	{
		if (window->DockIsActive)
			if (KGGuiDockNode* node = window->DockNode)
			{
				KGGuiID tab_id = TabBarCalcTabID(node->TabBar, label, window);
				KarmaGuiInternal::TabBarRemoveTab(node->TabBar, tab_id);
				window->DockTabWantClose = true;
			}
	}
}

KGVec2 Karma::KarmaGuiInternal::TabItemCalcSize(const char* label, bool has_close_button_or_unsaved_marker)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGVec2 label_size = KarmaGui::CalcTextSize(label, NULL, true);
	KGVec2 size = KGVec2(label_size.x + g.Style.FramePadding.x, label_size.y + g.Style.FramePadding.y * 2.0f);
	if (has_close_button_or_unsaved_marker)
		size.x += g.Style.FramePadding.x + (g.Style.ItemInnerSpacing.x + g.FontSize); // We use Y intentionally to fit the close button circle.
	else
		size.x += g.Style.FramePadding.x + 1.0f;
	return KGVec2(KGMin(size.x, TabBarCalcMaxTabWidth()), size.y);
}

KGVec2 Karma::KarmaGuiInternal::TabItemCalcSize(KGGuiWindow* window)
{
	return TabItemCalcSize(window->Name, window->HasCloseButton || (window->Flags & KGGuiWindowFlags_UnsavedDocument));
}

void Karma::KarmaGuiInternal::TabItemBackground(KGDrawList* draw_list, const KGRect& bb, KarmaGuiTabItemFlags flags, KGU32 col)
{
	// While rendering tabs, we trim 1 pixel off the top of our bounding box so they can fit within a regular frame height while looking "detached" from it.
	KarmaGuiContext& g = *GKarmaGui;
	const float width = bb.GetWidth();
	KG_UNUSED(flags);
	KR_CORE_ASSERT(width > 0.0f, "");
	const float rounding = KGMax(0.0f, KGMin((flags & KGGuiTabItemFlags_Button) ? g.Style.FrameRounding : g.Style.TabRounding, width * 0.5f - 1.0f));
	const float y1 = bb.Min.y + 1.0f;
	const float y2 = bb.Max.y + ((flags & KGGuiTabItemFlags_Preview) ? 0.0f : -1.0f);
	draw_list->PathLineTo(KGVec2(bb.Min.x, y2));
	draw_list->PathArcToFast(KGVec2(bb.Min.x + rounding, y1 + rounding), rounding, 6, 9);
	draw_list->PathArcToFast(KGVec2(bb.Max.x - rounding, y1 + rounding), rounding, 9, 12);
	draw_list->PathLineTo(KGVec2(bb.Max.x, y2));
	draw_list->PathFillConvex(col);
	if (g.Style.TabBorderSize > 0.0f)
	{
		draw_list->PathLineTo(KGVec2(bb.Min.x + 0.5f, y2));
		draw_list->PathArcToFast(KGVec2(bb.Min.x + rounding + 0.5f, y1 + rounding + 0.5f), rounding, 6, 9);
		draw_list->PathArcToFast(KGVec2(bb.Max.x - rounding - 0.5f, y1 + rounding + 0.5f), rounding, 9, 12);
		draw_list->PathLineTo(KGVec2(bb.Max.x - 0.5f, y2));
		draw_list->PathStroke(KarmaGui::GetColorU32(KGGuiCol_Border), 0, g.Style.TabBorderSize);
	}
}

// Render text label (with custom clipping) + Unsaved Document marker + Close Button logic
// We tend to lock style.FramePadding for a given tab-bar, hence the 'frame_padding' parameter.
void Karma::KarmaGuiInternal::TabItemLabelAndCloseButton(KGDrawList* draw_list, const KGRect& bb, KarmaGuiTabItemFlags flags, KGVec2 frame_padding, const char* label, KGGuiID tab_id, KGGuiID close_button_id, bool is_contents_visible, bool* out_just_closed, bool* out_text_clipped)
{
	KarmaGuiContext& g = *GKarmaGui;
	KGVec2 label_size = Karma::KarmaGui::CalcTextSize(label, NULL, true);

	if (out_just_closed)
		*out_just_closed = false;
	if (out_text_clipped)
		*out_text_clipped = false;

	if (bb.GetWidth() <= 1.0f)
		return;

	// In Style V2 we'll have full override of all colors per state (e.g. focused, selected)
	// But right now if you want to alter text color of tabs this is what you need to do.
#if 0
	const float backup_alpha = g.Style.Alpha;
	if (!is_contents_visible)
		g.Style.Alpha *= 0.7f;
#endif

	// Render text label (with clipping + alpha gradient) + unsaved marker
	KGRect text_pixel_clip_bb(bb.Min.x + frame_padding.x, bb.Min.y + frame_padding.y, bb.Max.x - frame_padding.x, bb.Max.y);
	KGRect text_ellipsis_clip_bb = text_pixel_clip_bb;

	// Return clipped state ignoring the close button
	if (out_text_clipped)
	{
		*out_text_clipped = (text_ellipsis_clip_bb.Min.x + label_size.x) > text_pixel_clip_bb.Max.x;
		//draw_list->AddCircle(text_ellipsis_clip_bb.Min, 3.0f, *out_text_clipped ? KG_COL32(255, 0, 0, 255) : KG_COL32(0, 255, 0, 255));
	}

	const float button_sz = g.FontSize;
	const KGVec2 button_pos(KGMax(bb.Min.x, bb.Max.x - frame_padding.x * 2.0f - button_sz), bb.Min.y);

	// Close Button & Unsaved Marker
	// We are relying on a subtle and confusing distinction between 'hovered' and 'g.HoveredId' which happens because we are using KGGuiButtonFlags_AllowOverlapMode + SetItemAllowOverlap()
	//  'hovered' will be true when hovering the Tab but NOT when hovering the close button
	//  'g.HoveredId==id' will be true when hovering the Tab including when hovering the close button
	//  'g.ActiveId==close_button_id' will be true when we are holding on the close button, in which case both hovered booleans are false
	bool close_button_pressed = false;
	bool close_button_visible = false;
	if (close_button_id != 0)
		if (is_contents_visible || bb.GetWidth() >= KGMax(button_sz, g.Style.TabMinWidthForCloseButton))
			if (g.HoveredId == tab_id || g.HoveredId == close_button_id || g.ActiveId == tab_id || g.ActiveId == close_button_id)
				close_button_visible = true;
	bool unsaved_marker_visible = (flags & KGGuiTabItemFlags_UnsavedDocument) != 0 && (button_pos.x + button_sz <= bb.Max.x);

	if (close_button_visible)
	{
		KGGuiLastItemData last_item_backup = g.LastItemData;
		KarmaGui::PushStyleVar(KGGuiStyleVar_FramePadding, frame_padding);
		if (CloseButton(close_button_id, button_pos))
			close_button_pressed = true;
		KarmaGui::PopStyleVar();
		g.LastItemData = last_item_backup;

		// Close with middle mouse button
		if (!(flags & KGGuiTabItemFlags_NoCloseWithMiddleMouseButton) && KarmaGui::IsMouseClicked(2))
			close_button_pressed = true;
	}
	else if (unsaved_marker_visible)
	{
		const KGRect bullet_bb(button_pos, button_pos + KGVec2(button_sz, button_sz) + g.Style.FramePadding * 2.0f);
		RenderBullet(draw_list, bullet_bb.GetCenter(), KarmaGui::GetColorU32(KGGuiCol_Text));
	}

	// This is all rather complicated
	// (the main idea is that because the close button only appears on hover, we don't want it to alter the ellipsis position)
	// FIXME: if FramePadding is noticeably large, ellipsis_max_x will be wrong here (e.g. #3497), maybe for consistency that parameter of RenderTextEllipsis() shouldn't exist..
	float ellipsis_max_x = close_button_visible ? text_pixel_clip_bb.Max.x : bb.Max.x - 1.0f;
	if (close_button_visible || unsaved_marker_visible)
	{
		text_pixel_clip_bb.Max.x -= close_button_visible ? (button_sz) : (button_sz * 0.80f);
		text_ellipsis_clip_bb.Max.x -= unsaved_marker_visible ? (button_sz * 0.80f) : 0.0f;
		ellipsis_max_x = text_pixel_clip_bb.Max.x;
	}
	RenderTextEllipsis(draw_list, text_ellipsis_clip_bb.Min, text_ellipsis_clip_bb.Max, text_pixel_clip_bb.Max.x, ellipsis_max_x, label, NULL, &label_size);

#if 0
	if (!is_contents_visible)
		g.Style.Alpha = backup_alpha;
#endif

	if (out_just_closed)
		*out_just_closed = close_button_pressed;
}
