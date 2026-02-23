/**
 * @file KarmaGuiMesa.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the classes for Editor
 * @version 1.0
 * @date October 1, 2022
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "Karma.h"
#include "KarmaGui/KarmaGuizmo.h"

namespace Karma
{
	struct CallbacksFromEditor
	{
		std::function< void(std::string) > openSceneCallback;
	};

	struct KarmaLogMesa
	{
		static KarmaGuiTextBuffer     TextBuffer;
		static KarmaGuiTextFilter     TextFilter;
		static KGVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
		static bool                AutoScroll;  // Keep scrolling if already at the bottom.
		static std::shared_ptr<spdlog::logger> s_MesaCoreLogger;
		static std::shared_ptr<spdlog::logger> s_MesaClientLogger;
		static std::shared_ptr<spdlog::pattern_formatter> s_MesaLogFormatter;

		KarmaLogMesa()
		{
			AutoScroll = true;
			Clear();
		}

		static void Clear()
		{
			TextBuffer.clear();
			LineOffsets.clear();
			LineOffsets.push_back(0);
		}

		static void AddLog(const char* fmt, ...) /*KG_FMTARGS(2) */// <- what in the name of WYSIWYG is this?
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

		static void Draw(const char* title, bool* pOpen = nullptr)
		{
			if (!KarmaGui::Begin(title, pOpen))
			{
				KarmaGui::End();
				return;
			}

			// Options menu
			if (KarmaGui::BeginPopup("Options"))
			{
				KarmaGui::Checkbox("Auto-scroll", &AutoScroll);
				KarmaGui::EndPopup();
			}

			// Main window
			if (KarmaGui::Button("Options"))
			{
				KarmaGui::OpenPopup("Options");
			}

			KarmaGui::SameLine();
			bool clear = KarmaGui::Button("Clear");
			KarmaGui::SameLine();
			bool copy = KarmaGui::Button("Copy");
			KarmaGui::SameLine();

			TextFilter.Draw("Filter", -100.0f);

			KarmaGui::Separator();
			KarmaGui::BeginChild("scrolling", KGVec2(0, 0), false, KGGuiWindowFlags_HorizontalScrollbar);

			if (clear)
			{
				Clear();
			}
			if (copy)
			{
				KarmaGui::LogToClipboard();
			}

			KarmaGui::PushStyleVar(KGGuiStyleVar_ItemSpacing, KGVec2(0, 0));

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
						KarmaGui::TextUnformatted(lineStart, lineEnd);
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
				KarmaGuiListClipper clipper;
				clipper.Begin(LineOffsets.Size);

				while (clipper.Step())
				{
					for (int lineNumber = clipper.DisplayStart; lineNumber < clipper.DisplayEnd; lineNumber++)
					{
						const char* lineStart = buAlpha + LineOffsets[lineNumber];
						const char* lineEnd = (lineNumber + 1 < LineOffsets.Size) ? (buAlpha + LineOffsets[lineNumber + 1] - 1) : buOmega;
						KarmaGui::TextUnformatted(lineStart, lineEnd);
					}
				}
				clipper.End();
			}
			KarmaGui::PopStyleVar();

			if (AutoScroll && KarmaGui::GetScrollY() >= KarmaGui::GetScrollMaxY())
			{
				KarmaGui::SetScrollHereY(1.0f);
			}

			KarmaGui::EndChild();
			KarmaGui::End();
		}
	};

	struct KarmaGuiDockPreviewData
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

		KarmaGuiDockPreviewData();
	};

	struct KarmaTuringMachineElectronics
	{
		bool bHasQueried;

		// Bios Information
		std::string biosVendorName;
		std::string biosSerialNumber;
		std::string biosVersion;
		std::string biosBoardName;

		// System Memory (RAM) overview
		uint32_t numberOfMemoryDevices;

		std::string memoryCapacity;
		std::string freeMemory;

		struct SystemRAM
		{
			uint32_t	id;
			std::string vendor;
			std::string model;
			std::string name;
			std::string serialNumber;
			std::string frequency;
			std::string capacity;
		};
		SystemRAM* ramInformation;


		// Processor information
		struct CPU
		{
			std::string cpuVendor;
			std::string cpuModel;
			std::string cpuCurrentFrequency;
			std::string cpuMaximumFrequency;
			std::string cpuPhysicalCores;
			std::string cpuLogicalCores;
			std::string cpuCacheSizeL1;
			std::string cpuCacheSizeL2;
			std::string cpuCacheSizeL3;
		};
		CPU cpuInformation;

		// For now, with due respect, let there be enough content with just model number
		// und vendor. Would be dope to read the GPU just like RAM or CPU, from SMBIOS!!
		
		// Ok thanks to shadPS4 emulator, we are using dope library hwinfo 
		std::string gpuVendor;
		std::string gpuModelIdentification;
		
		std::string gpuVMemory;

		KarmaTuringMachineElectronics()
		{
			bHasQueried = false;
			numberOfMemoryDevices = 0;
			ramInformation = nullptr;
		}

		~KarmaTuringMachineElectronics();

	};

	struct WindowManipulationGaugeData
	{
		float widthCache;
		float heightCache;
		float startXCache;
		float startYCache;
		float ioDisplayXCache;
		float ioDisplayYCache;
		float scrollX;
		float scrollY;
	};

	struct UObjectsStatistics
	{
		void* objectPointer;
		std::string beginAddress;
		std::string endAddress;
		size_t size;
		size_t sizeInPool;
		std::string uobjectName;
		uint32_t alignment;
		UClass* classObject;

		// Placement in memory pool
		KGVec2 placementCoordi;//nates

		UObjectsStatistics()
		{
			placementCoordi.x = placementCoordi.y = 0.0f;
		}
	};

	class KarmaGuiMesa
	{
	public:
		// Showtime!
		static void RevealMainFrame(KGGuiID mainMesaDockID, std::shared_ptr<Scene> scene, const CallbacksFromEditor& editorCallbacks);
		static void EditTransform(std::shared_ptr<Scene> scene);
		static void DrawKarmaMainMenuBarMesa();
		static void DrawMainMenuFileListMesa();
		static void DrawKarmaLogMesa(KGGuiID mainMesaDockID);
		static void DrawKarmaSceneHierarchyPanelMesa(std::shared_ptr<Scene> scene);
		static void Draw3DModelExhibitorMesa(std::shared_ptr<Scene> scene);
		static void DrawContentBrowser(const std::function< void(std::string) >& openSceneCallback);
		static void DrawMemoryExhibitor();

		// Mesas!
		static void ShowAboutKarmaMesa(bool* pbOpen);
		
		// Utility
		static bool InputFloat3XYZ(const char* label, float v[3], const char* format = "%.3f");
		static bool InputAxisFloat(const char* axis_id, const char* axis_label, const KGVec4& color,
								   float* v, const char* format, float width = 60.0f);

		// Shiva the Mesa and rest
		static void MesaShutDownRoutine();

		static KGGuiDockNode* DockNodeTreeFindFallbackLeafNode(KGGuiDockNode* node);

		// Getters
		static KarmaTuringMachineElectronics& GetGatheredElectronicsInformationForModification() { return electronicsItems; }
		static const KarmaTuringMachineElectronics& GetGatheredElectronicsInformation() { return electronicsItems; }

		// Setters
		static void SetElectronicsRamInformationToNull();

		// Helpers
		static int ImStrlenW(const KGWchar* str);
		static void QueryForTuringMachineElectronics();
		static uint32_t ChurnUint32FromString(const std::string& ramString);
		static std::string ChurnDimensionsFromString(const std::string& ramString);
		static double HexStringToDecimal(const std::string& hexString);

		// Statistics
		static void DumpUObjectStatistics(void* InObject, const std::string& InName, size_t InSize, size_t InAlignment, class UClass* InClass);

	public:
		static std::string notAvailableText;
		static KarmaLogMesa m_KarmaLog;

	private:
		static KarmaTuringMachineElectronics electronicsItems;
		static WindowManipulationGaugeData m_3DExhibitor;
		static WindowManipulationGaugeData m_MemoryExhibitor;
		static bool m_EditorInitialized;
		static bool m_RefreshRenderingResources;
		static class AStaticMeshActor* m_SelectedSMActor;

		// Content browser
		static std::filesystem::path m_CurrentDirectory;

		// Need agnostic naming scheme
		static uint32_t m_DirectoryIcon;
		static uint32_t m_FileIcon;
		
		// KarmaGuizmo
		static KarmaGuizmo::OPERATION m_CurrentGizmoOperation;
		static KarmaGuizmo::MODE m_CurrentGizmoMode;

		// UObjects statistics
		static KarmaVector<UObjectsStatistics> m_UObjectStatistics;

	public:
		static bool m_ViewportFocused;
		static bool m_ViewportHovered;
	};
}
