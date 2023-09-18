#pragma once

#include "Karma.h"

extern "C" {
#include "dmidecode.h"
}

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
		std::string biosVersion;
		std::string biosReleaseDate;
		std::string biosCharacteristics;
		std::string biosROMSize;
		std::string biosCurrentSetLanguage;
		std::string biosRestOfTheSupportedLanguages;

		// System Memory (RAM) overview
		uint32_t numberOfMemoryDevices;// An estimation. I shall manually introduce logical checks

		// I named estimated because of the lies
		// https://github.com/ravimohan1991/BiosReader/wiki/The-Life-and-Lies-of-the-BIOS
		std::string estimatedCapacity;
		std::string supportingArea;

		struct SystemRAM
		{
			std::string formFactor;
			std::string ramSize;
			std::string locator;
			std::string ramType;
			std::string bankLocator;
			std::string manufacturer;

			std::string serialNumber;
			std::string partNumber;
			std::string assetTag;

			std::string memorySpeed;
			std::string configuredMemorySpeed;

			std::string operatingVoltage;
			std::string rank;
		};
		SystemRAM* ramInformation;

		// Let me tell the story of naming. Since the physical slots are the ones
		// present on board, the array slots in software side, getting filled on a query to BIOS,
		// naturally get the name "...SoftSlots" from BiosReader's allocation POV
		std::vector<uint32_t> ramSoftSlots;

		uint32_t totalRamSize;
		std::string ramSizeDimensions;

		// Processor information
		// Asssuming only 1 processor
		std::string cpuDesignation;// Socket designation
		std::string cpuType; // In order to distinguish from GPU processor :) or DSP https://en.wikipedia.org/wiki/Digital_signal_processor
		std::string cpuProcessingfamily;
		std::string cpuManufacturer;
		std::string cpuFlags;
		// Kind of the most important element of this struct. eg Intel(R) Core(TM) i5-7400 CPU @ 3.00GHz (completeprocessingunitidentifier)
		std::string cpuVersion;

		std::string cpuOperatingVoltage;
		std::string cpuExternalClock;
		std::string cpuMaximumSpeed;
		std::string cpuCurrentSpeed;

		// Some OEM specific numbers
		std::string cpuSerialNumber;
		std::string cpuPartNumber;
		std::string cpuAssettag;

		std::string cpuCorescount;
		std::string cpuEnabledCoresCount;
		std::string cpuThreadCount;
		std::string cpuTheCharacterstics;

		// The Cpu ID field contains processor - specific information that describes the processor’s features.
		std::string cpuid; // in the context of motherboard components
		std::string cpuSignature;

		// For now, with due respect, let there be enough content with just model number
		// und vendor. Would be dope to read the GPU just like RAM or CPU, from SMBIOS!!
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

		// Gauging Ram devices
		static void GaugeSystemMemoryDevices(random_access_memory* ramCluster);

		// Obtain the real RAM information
		static void FindRealCapacityOfRam();

		// No-ram conditions. Bit'o hacky stuff
		static bool IsPhysicalRamPresent(const random_access_memory& ramScam);

		// Filling the SystemRAM structure with relevant information
		static void FillTheSystemRamStructure(SystemRAM& destinationStructure, random_access_memory& sourceStructure);
	};

	struct WindowManipulationGaugeData
	{
		float widthCache;
		float heightCache;
		float startXCache;
		float startYCache;
		float ioDisplayXCache;
		float ioDisplayYCache;
	};

	class KarmaGuiMesa
	{
	public:
		// Showtime!
		static void RevealMainFrame(KGGuiID mainMesaDockID, std::shared_ptr<Scene> scene, const CallbacksFromEditor& editorCallbacks);
		static void DrawKarmaMainMenuBarMesa();
		static void DrawMainMenuFileListMesa();
		static void DrawKarmaLogMesa(KGGuiID mainMesaDockID);
		static void DrawKarmaSceneHierarchyPanelMesa();
		static void Draw3DModelExhibitorMesa(std::shared_ptr<Scene> scene);
		static void DrawContentBrowser(const std::function< void(std::string) >& openSceneCallback);
		static void DrawMemoryExhibitor();

		// Mesas!
		static void ShowAboutKarmaMesa(bool* pbOpen);

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
		static uint32_t ChernUint32FromString(const std::string& ramString);
		static std::string ChernDimensionsFromString(const std::string& ramString);
		static double HexStringToDecimal(const std::string& hexString);

		// Statistics
		static void DumpUObjectStatistics(void* InObject, const std::string& InName, size_t InSize, size_t InAlignment, class UClass* InClass);

	public:
		static std::string notAvailableText;
		static KarmaLogMesa m_KarmaLog;

	private:
		static KarmaTuringMachineElectronics electronicsItems;
		static WindowManipulationGaugeData m_3DExhibitor;
		static bool m_EditorInitialized;
		static bool m_RefreshRenderingResources;

		// Content browser
		static std::filesystem::path m_CurrentDirectory;

		// Need agnostic naming scheme
		static uint32_t m_DirectoryIcon;
		static uint32_t m_FileIcon;

	public:
		static bool m_ViewportFocused;
		static bool m_ViewportHovered;
	};
}
