#pragma once

#include "krpch.h"

#include "imgui.h"
#include "imgui_internal.h"

namespace Karma
{
	struct KarmaLogMesa
	{
		ImGuiTextBuffer     TextBuffer;
		ImGuiTextFilter     TextFilter;
		ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
		bool                AutoScroll;  // Keep scrolling if already at the bottom.

		KarmaLogMesa()
		{
			AutoScroll = true;
			Clear();
		}

		void Clear()
		{
			TextBuffer.clear();
			LineOffsets.clear();
			LineOffsets.push_back(0);
		}

		void AddLog(const char* fmt, ...) IM_FMTARGS(2) // <- what in the name of WYSIWYG is this?
		{
			int cacheSize = TextBuffer.size();
			va_list args;
			va_start(args, fmt);
			TextBuffer.appendfv(fmt, args);
			va_end(args);

			for (int newSize = TextBuffer.size(); cacheSize < newSize; cacheSize++)
			{
				if (TextBuffer[cacheSize] == '\n')
				{
					LineOffsets.push_back(cacheSize + 1);
				}
			}
		}

		void Draw(const char* title, bool* pOpen = nullptr)
		{
			if (!ImGui::Begin(title, pOpen))
			{
				ImGui::End();
				return;
			}

			// Options menu
			if (ImGui::BeginPopup("Options"))
			{
				ImGui::Checkbox("Auto-scroll", &AutoScroll);
				ImGui::EndPopup();
			}

			// Main window
			if (ImGui::Button("Options"))
			{
				ImGui::OpenPopup("Options");
			}

			ImGui::SameLine();
			bool clear = ImGui::Button("Clear");
			ImGui::SameLine();
			bool copy = ImGui::Button("Copy");
			ImGui::SameLine();

			TextFilter.Draw("Filter", -100.0f);

			ImGui::Separator();
			ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

			if (clear)
			{
				Clear();
			}
			if (copy)
			{
				ImGui::LogToClipboard();
			}

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

			const char* buAlpha = TextBuffer.begin();
			const char* buOmega = TextBuffer.end();

			if (TextFilter.IsActive())
			{
				// In this example we don't use the clipper when TextFilter is enabled.
				// This is because we don't have a random access on the result on our filter.
				// A real application processing logs with ten of thousands of entries may want to store the result of
				// search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
				for (int lineNumber = 0; lineNumber < LineOffsets.Size; lineNumber++)
				{
					const char* lineStart = buAlpha + LineOffsets[lineNumber];
					const char* lineEnd = (lineNumber + 1 < LineOffsets.Size) ? (buAlpha + LineOffsets[lineNumber + 1] - 1) : buOmega;
					if (TextFilter.PassFilter(lineStart, lineEnd))
					{
						ImGui::TextUnformatted(lineStart, lineEnd);
					}
				}
			}
			else
			{
				// The simplest and easy way to display the entire buffer:
				//   ImGui::TextUnformatted(buBegin, buEnd);
				// And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
				// to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
				// within the visible area.
				// If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
				// on your side is recommended. Using ImGuiListClipper requires
				// - A) random access into your data
				// - B) items all being the same height,
				// both of which we can handle since we have an array pointing to the beginning of each line of text.
				// When using the filter (in the block of code above) we don't have random access into the data to display
				// anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
				// it possible (and would be recommended if you want to search through tens of thousands of entries).
				ImGuiListClipper clipper;
				clipper.Begin(LineOffsets.Size);

				while (clipper.Step())
				{
					for (int lineNumber = clipper.DisplayStart; lineNumber < clipper.DisplayEnd; lineNumber++)
					{
						const char* lineStart = buAlpha + LineOffsets[lineNumber];
						const char* lineEnd = (lineNumber + 1 < LineOffsets.Size) ? (buAlpha + LineOffsets[lineNumber + 1] - 1) : buOmega;
						ImGui::TextUnformatted(lineStart, lineEnd);
					}
				}
				clipper.End();
			}
			ImGui::PopStyleVar();

			if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			{
				ImGui::SetScrollHereY(1.0f);
			}

			ImGui::EndChild();
			ImGui::End();
		}
	};

	struct ImGuiDockPreviewData
	{
		ImGuiDockNode   FutureNode;
		bool            IsDropAllowed;
		bool            IsCenterAvailable;
		bool            IsSidesAvailable;           // Hold your breath, grammar freaks..
		bool            IsSplitDirExplicit;         // Set when hovered the drop rect (vs. implicit SplitDir==None when hovered the window)
		ImGuiDockNode* SplitNode;
		ImGuiDir        SplitDir;
		float           SplitRatio;
		ImRect          DropRectsDraw[ImGuiDir_COUNT + 1];  // May be slightly different from hit-testing drop rects used in DockNodeCalcDropRects()

		ImGuiDockPreviewData() : FutureNode(0) { IsDropAllowed = IsCenterAvailable = IsSidesAvailable = IsSplitDirExplicit = false; SplitNode = NULL; SplitDir = ImGuiDir_None; SplitRatio = 0.f; for (int n = 0; n < IM_ARRAYSIZE(DropRectsDraw); n++) DropRectsDraw[n] = ImRect(+FLT_MAX, +FLT_MAX, -FLT_MAX, -FLT_MAX); }
	};

	struct KarmaTuringMachineElectronics
	{
		bool bHasQueried;

		// Bios Information
		std::string vendorName;
		std::string biosVersion;
		std::string biosReleaseDate;
		std::string biosCharacteristics;
		std::string biosROMSize;

		KarmaTuringMachineElectronics()
		{
			bHasQueried = false;
		}
	};

	class KARMA_API ImGuiMesa
	{
	public:
		// Showtime!
		static void RevealMainFrame(ImGuiID mainMesaDockID);
		static void DrawKarmaMainMenuBarMesa();
		static void DrawMainMenuFileListMesa();
		static void DrawKarmaLogMesa(ImGuiID mainMesaDockID);
		static void DrawKarmaSceneHierarchyPanelMesa();

		// Mesas!
		static void ShowAboutKarmaMesa(bool* pbOpen);

		static ImGuiDockNode* DockNodeTreeFindFallbackLeafNode(ImGuiDockNode* node);

		// Helpers
		static int ImStrlenW(const ImWchar* str);
		static void QueryForTuringMachineElectronics();

	private:
		static KarmaTuringMachineElectronics electronicsItems;
	};
}
