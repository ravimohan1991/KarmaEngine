/*
 A mesa is an isolated, flat-topped elevation, ridge or hill, which is bounded from all sides by steep escarpments and stands distinctly above a surrounding plain.
 Courtsey https://en.wikipedia.org/wiki/Mesa
 */

 /*
  Following code snippets are heavily influenced from the official Dear ImGUI windows (isn't that always the scenario?). Please see https://github.com/ravimohan1991/imgui/blob/9068fd1afbaba56247aacd452bbadebb286800c7/imgui_demo.cpp for the historical and legacy purposes.

  Also see https://en.wikipedia.org/wiki/Indian_Mesa, since I am Indian! And I still love my country!!
  */

#include "ImGuiMesa.h"
#include "imgui.h"

  // Experimental
#include "ImGuiVulkanHandler.h"

namespace Karma
{
	KarmaTuringMachineElectronics ImGuiMesa::electronicsItems;
	std::string ImGuiMesa::notAvailableText = "Kasturi Trishna (The MuskThirst)";

	void ImGuiMesa::RevealMainFrame(ImGuiID mainMesaDockID)
	{
		// The MM (Main Menu) menu bar
		DrawKarmaMainMenuBarMesa();

		// 2. Show a simple window, soon to be the content and relevant information browser.
		{
			static bool show = true;
			static float fValue = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and appeninto it

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show);                  // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show);

			ImGui::SliderFloat("float", &fValue, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a colo

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets returtrue when edited/activated)
				counter++;
			ImGui::SameLine();

			ImGui::Text("counter = %d", counter);
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			ImGuiDockNode* node = nullptr;
			ImGuiWindow* window = nullptr;//ImGui::FindDockSpaceByID(mainMesaDockID);
			ImGuiWindow* payloadWindow = nullptr;//ImGui::FindWindowByName("Karma: Log");

			int boxNumber = -5;
			node = nullptr;//ImGui::FindAppropriateNode(window, payloadWindow, boxNumber);

			ImGui::Text("Node ID = %d at position x = %f, y = %f on docking box %d", node != nullptr ? node->ID : 0, ImGui::GetMousePos().x, ImGui::GetMousePos().y, boxNumber);

			if (payloadWindow)
				ImGui::Text("Karma: Log window is of dimension width = %f und height = %f", payloadWindow->Size.x, payloadWindow->Size.y);
			ImGui::End();
		}

		// 3. Lougging. Sorry about the spelling, but I want to make the pronounciation match with that of that
		// https://youtu.be/dZr-53LAlOw?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT&t=15
		{
			DrawKarmaLogMesa(mainMesaDockID);
		}

		// 4. A panel for scene hierarchy and whatnot
		{
			DrawKarmaSceneHierarchyPanelMesa();
		}
	}

	ImGuiDockNode* ImGuiMesa::DockNodeTreeFindFallbackLeafNode(ImGuiDockNode* node)
	{
		if (node->IsLeafNode())
			return node;
		if (ImGuiDockNode* leaf_node = DockNodeTreeFindFallbackLeafNode(node->ChildNodes[0]))
			return leaf_node;
		if (ImGuiDockNode* leaf_node = DockNodeTreeFindFallbackLeafNode(node->ChildNodes[1]))
			return leaf_node;
		return NULL;
	}

	void ImGuiMesa::DrawKarmaSceneHierarchyPanelMesa()
	{
		ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
		ImGui::Begin("Scene Hierarchy");

		ImGui::Text("Some Stuff 1");
		ImGui::Text("Some stuff 2");
		ImGui::Text("lumbdaa");

		ImGui::End();
	}

	// MM bar
	void ImGuiMesa::DrawKarmaMainMenuBarMesa()
	{
		static bool showKarmaAbout = false;

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				DrawMainMenuFileListMesa();
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Details"))
			{
				if (ImGui::MenuItem("About", nullptr, &showKarmaAbout));
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (showKarmaAbout)
		{
			ShowAboutKarmaMesa(&showKarmaAbout);
		}
		else
		{
			// Should be called when "about" mesa is closed, the first time
			if (electronicsItems.bHasQueried)
			{
				reset_electronics_structures();
				ImGuiMesa::SetElectronicsRamInformationToNull();
				electronicsItems.ramSoftSlots.clear();
				electronicsItems.bHasQueried = false;
			}
		}
	}

	// The lougging window with basic filtering.
	void ImGuiMesa::DrawKarmaLogMesa(ImGuiID mainMesaDockID)
	{
		static KarmaLogMesa log;

		ImVec2 windowSize = ImVec2(680, 420);

		ImGuiCond conditions = ImGuiCond_Once;//ImGuiCond_FirstUseEver;

		// So here goes the reverse engineering
		// 1. imgui.ini is looked. If not found, then window->SizeFull is set to windowSize else
		// 2. well I failed, partially. Ini to the rescue
		// 3. maybe I will find it later, and NOT the cherno later.
		ImGui::SetNextWindowSize(windowSize, conditions);

		ImGuiWindow* payloadWindow;

		// Disable user resize,
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize;

		ImGui::Begin("Karma: Log", nullptr, windowFlags, &payloadWindow);

		if (ImGui::SmallButton("[Debug] Add 5 entries"))
		{
			static int counter = 0;
			const char* categories[3] = { "info", "warn", "error" };
			const char* words[] = { "Bumfuzzled", "Cattywampus", "Snickersnee", "Abibliophobia", "Absquatulate", "Nincompoop", 	"Pauciloquent" };
			for (int n = 0; n < 5; n++)
			{
				const char* category = categories[counter % IM_ARRAYSIZE(categories)];
				const char* word = words[counter % IM_ARRAYSIZE(words)];
				log.AddLog("[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
					ImGui::GetFrameCount(), category, ImGui::GetTime(), word);
				counter++;
			}
		}

		ImGui::End();

		// Actually call in the regular Log helper (which will Begin() into the same window as we just did)
		log.Draw("Karma: Log");

		/*
		if(payloadWindow != nullptr && !payloadWindow->bIsCodeDocked)
		{
			payloadWindow->Size = windowSize;
			ImGui::QueueDockingRequest(mainMesaDockID, payloadWindow, 3);
			//payloadWindow->SizeFull = payloadWindow->Size = windowSize;
			payloadWindow->bIsCodeDocked = true;
			payloadWindow->bAutoFit = true;
		}
		//payloadWindow->Size = windowSize;
		*/
	}

	void ImGuiMesa::DrawMainMenuFileListMesa()
	{
		if (ImGui::MenuItem("Open", "Ctrl+O")) {}
		if (ImGui::BeginMenu("Open Recent"))
		{
			// write code with some memorized history
			ImGui::EndMenu();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Quit", "Alt+F4"))
		{
			// write logic to quit
		}
	}

	void ImGuiMesa::MesaShutDownRoutine()
	{
		if (electronicsItems.bHasQueried)
		{
			reset_electronics_structures();
			ImGuiMesa::SetElectronicsRamInformationToNull();
			electronicsItems.ramSoftSlots.clear();
			electronicsItems.bHasQueried = false;
		}
	}

	//-----------------------------------------------------------------------------
	// [SECTION] A variety of Dear ImGui mesas
	//-----------------------------------------------------------------------------

	// About mesa
	void ImGuiMesa::ShowAboutKarmaMesa(bool* pbOpen)
	{
		if (!ImGui::Begin("Karma Engine", pbOpen, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::End();
			return;
		}

		// Gather relevant data
		QueryForTuringMachineElectronics();

		//-----------------------------------------------------------------------------------------------------------//

		// Precomputation based on text, for gauging the image dimensions
		const char* tagLine = "Nothing is impossible once you have the Source Code (and know how to use it)!";
		ImVec2 tagLineDimensions = ImGui::CalcTextSize(tagLine, nullptr, false, 0.0f);

		const char* authorName = "The_Cowboy";
		ImVec2 authorNameDimensions = ImGui::CalcTextSize(authorName, nullptr, false, 0.0f);

		//-----------------------------------------------------------------------------------------------------------//

		// Vulkan experiment
		ImGuiIO& io = ImGui::GetIO();
		ImGui_KarmaImplVulkan_Data* backendData = ImGui::GetCurrentContext() ? (ImGui_KarmaImplVulkan_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;

		ImGui_KarmaImplVulkan_Image_TextureData* mesaDecalElement = backendData->mesaDecalDataList.at(0);

		ImTextureID aboutImageTextureID = (ImTextureID)mesaDecalElement->TextureDescriptorSet;
		uint32_t width = mesaDecalElement->width;
		uint32_t height = mesaDecalElement->height;

		{
			ImVec2 position = ImGui::GetCursorScreenPos();

			ImVec2 uvMin = ImVec2(0.0f, 0.0f);                 // Top-left
			ImVec2 uvMax = ImVec2(1.0f, 1.0f);                 // Lower-right
			ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
			ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
			ImGui::Image(aboutImageTextureID, ImVec2(width, height), uvMin, uvMax, tint_col, border_col);
		}

		//-----------------------------------------------------------------------------------------------------------//

		ImGui::Text("%s", tagLine);
		ImGui::Text(" "); ImGui::SameLine(tagLineDimensions.x - authorNameDimensions.x); ImGui::Text("%s", authorName);

		ImGui::Separator();

		//-----------------------------------------------------------------------------------------------------------//

		const char* licenseLine_1 = "Karma Engine is a copyleft software and distributed under the umbrella of";
		const char* licenseLine_2 = "GNU GPL v3 https://github.com/ravimohan1991/KarmaEngine/blob/main/LICENSE";
		const char* licenseLine_3 = "Although careful and mindful tending of the \"web of companion libraries\"";
		const char* licenseLine_4 = "lincences has been undertaken, it is a work in progress with the hope of";
		const char* licenseLine_5 = "eliminating the concept of software licensing itself.";

		ImGui::Text("%s", licenseLine_1);
		ImGui::Text("%s", licenseLine_2);
		ImGui::Text("%s", licenseLine_3);
		ImGui::Text("%s", licenseLine_4);
		ImGui::Text("%s", licenseLine_5);

		ImGui::Separator();

		//-----------------------------------------------------------------------------------------------------------//

		static bool showPhysicalRigInformation = false;
		ImGui::Checkbox("Config/Build Information", &showPhysicalRigInformation);
		if (showPhysicalRigInformation)
		{
			ImGuiIO& io = ImGui::GetIO();
			ImGuiStyle& style = ImGui::GetStyle();

			bool copy_to_clipboard = ImGui::Button("Copy to clipboard");
		}

		ImGui::Separator();

		//-----------------------------------------------------------------------------------------------------------//

		ImGui::Text("Credits und Acknowledgements");
		ImGui::Text("Cherno");
		ImGui::Text("Travis V Roman");
		ImGui::Text("GitHub");
		ImGui::Text("Companion Libraries Authors");
		ImGui::Text("Sumo India");

		ImGui::Separator();

		//-----------------------------------------------------------------------------------------------------------//

		// Finally the Turing Machine's electronics information presentation!
		static bool bShowTuringElectronics = false;
		ImGui::Checkbox("Turing Machine Information", &bShowTuringElectronics);
		if (bShowTuringElectronics)
		{
			ImGuiIO& io = ImGui::GetIO();
			ImGuiStyle& style = ImGui::GetStyle();

			bool bCopyToClipboard = ImGui::Button("Copy to clipboard");
			ImVec2 childSize = ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 18);
			ImGui::BeginChildFrame(ImGui::GetID("cfg_infos"), childSize, ImGuiWindowFlags_NoMove);

			if (bCopyToClipboard)
			{
				ImGui::LogToClipboard();
				ImGui::LogText("```\n"); // Back quotes will make text appears without formatting when pasting on GitHub
			}

			ImGui::Text("Machine BIOS (v%s)", electronicsItems.biosVersion.c_str());
			ImGui::Separator();

			ImGui::Text("Vendor: %s", electronicsItems.biosVendorName.c_str());
			ImGui::Text("Supplied On: %s", electronicsItems.biosReleaseDate.c_str());
			ImGui::Text("ROM Size: %s", electronicsItems.biosROMSize.c_str());
			ImGui::Text("Current Language: %s", electronicsItems.biosCurrentSetLanguage.c_str());
			ImGui::Text("Supported Languages:");
			ImGui::Indent();
			ImGui::Text("%s", electronicsItems.biosRestOfTheSupportedLanguages.c_str());
			ImGui::Unindent();
			ImGui::Text("BIOS Characteristics:");
			ImGui::Indent();
			ImGui::Text("%s", electronicsItems.biosCharacteristics.c_str());
			ImGui::Unindent();
			ImGui::Separator();

			ImGui::Text("Machine System Memory (RAM and all that)");
			ImGui::Separator();

			ImGui::Text("Supporting Area: %s", electronicsItems.supportingArea.c_str());
			ImGui::Text("Estimated Capacity: %s", electronicsItems.estimatedCapacity.c_str());
			ImGui::Text("Total such devices (est): %d", electronicsItems.numberOfMemoryDevices);
			ImGui::Text("Physical devices present:");

			for (uint32_t counter = 0; counter < electronicsItems.ramSoftSlots.size(); counter++)
			{
				ImGui::Text("RAM %d", counter + 1);
				ImGui::Text("Manufacturer: %s", electronicsItems.ramInformation[counter].manufacturer.c_str());

				ImGui::Text("Identification Parameters");

				ImGui::Indent();
				ImGui::Text("Ram Type: %s", electronicsItems.ramInformation[counter].ramType.c_str());
				ImGui::Text("Part Number: %s", electronicsItems.ramInformation[counter].partNumber.c_str());
				ImGui::Text("Serial Number: %s", electronicsItems.ramInformation[counter].serialNumber.c_str());
				ImGui::Text("(Bank | Device) Locator: %s | %s", electronicsItems.ramInformation[counter].bankLocator.c_str(),
					electronicsItems.ramInformation[counter].locator.c_str());
				ImGui::Text("Asset Tag: %s", electronicsItems.ramInformation[counter].assetTag.c_str());
				ImGui::Unindent();

				ImGui::Text("Ram Conditions");
				ImGui::Indent();
				ImGui::Text("Size: %s", electronicsItems.ramInformation[counter].ramSize.c_str());
				ImGui::Text("Operating Voltage: %s", electronicsItems.ramInformation[counter].operatingVoltage.c_str());
				ImGui::Text("Speed (Current | Maximum): %s | %s", electronicsItems.ramInformation[counter].configuredMemorySpeed.c_str(),
					electronicsItems.ramInformation[counter].memorySpeed.c_str());
				ImGui::Text("Form Factor: %s", electronicsItems.ramInformation[counter].formFactor.c_str());
				ImGui::Unindent();
			}

			ImGui::Text("RAM Logistics");
			ImGui::Indent();
			ImGui::Text("Total Ram Size: %d %s", electronicsItems.totalRamSize, electronicsItems.ramSizeDimensions.c_str());
			ImGui::Unindent();

			ImGui::Separator();

			ImGui::Text("Central Processor Unit");
			ImGui::Separator();

			ImGui::Text("Manufacturer: %s", electronicsItems.cpuManufacturer.c_str());
			ImGui::Text("Processor Family: %s", electronicsItems.cpuProcessingfamily.c_str());
			ImGui::Text("Version: %s", electronicsItems.cpuVersion.c_str());
			ImGui::Text("CPU Conditions");
			ImGui::Indent();
			ImGui::Text("Speed (Current | Maximum): %s | %s", electronicsItems.cpuCurrentSpeed.c_str(), electronicsItems.cpuMaximumSpeed.c_str());
			ImGui::Text("External Clock: %s", electronicsItems.cpuExternalClock.c_str());
			ImGui::Text("Cores (Current | Maximum): %s | %s", electronicsItems.cpuEnabledCoresCount.c_str(), electronicsItems.cpuCorescount.c_str());
			ImGui::Text("Threads Count: %s", electronicsItems.cpuThreadCount.c_str());
			ImGui::Text("Operating Voltage: %s", electronicsItems.cpuOperatingVoltage.c_str());
			ImGui::Unindent();
			ImGui::Text("CPU Tags or Numbers");
			ImGui::Indent();
			ImGui::Text("Signature: %s", electronicsItems.cpuSignature.c_str());
			ImGui::Text("ID: %s", electronicsItems.cpuid.c_str());
			ImGui::Text("Part Number: %s", electronicsItems.cpuPartNumber.c_str());
			ImGui::Text("Serial Number: %s", electronicsItems.cpuSerialNumber.c_str());
			ImGui::Text("Asset Tag: %s", electronicsItems.cpuAssettag.c_str());
			ImGui::Unindent();
			ImGui::Text("CPU Characteristics");
			ImGui::Indent();
			ImGui::Text("%s", electronicsItems.cpuTheCharacterstics.c_str());
			ImGui::Unindent();
			ImGui::Text("Flags:");
			ImGui::Indent();
			ImGui::Text("%s", electronicsItems.cpuFlags.c_str());
			ImGui::Unindent();

			ImGui::Separator();

			ImGui::Text("Graphics Processing Unit");
			ImGui::Separator();

			ImGui::Text("Manufacturer: %s", electronicsItems.gpuVendor.c_str());
			ImGui::Text("Model: %s", electronicsItems.gpuModelIdentification.c_str());
			ImGui::Text("VRam: %s", electronicsItems.gpuVMemory.c_str());

			ImGui::Separator();

			if (bCopyToClipboard)
			{
				ImGui::LogText("\n```\n");
				ImGui::LogFinish();
			}
			ImGui::EndChildFrame();
		}

		ImGui::End();
	}

	// Strings are copied in this not-so-cheap function. Hence the check!!
	// First copying is done within BiosReader for apporpriate seperation into structures. Prevents multiple queries
	// at the cost of bulk (pun intended!).
	// Next copying is done here, in the routine.
	void ImGuiMesa::QueryForTuringMachineElectronics()
	{
		if (electronicsItems.bHasQueried)
		{
			return;
		}

		// Catcher rhymes with Hatcher, the Topologist, just for information!
		void* catcher = electronics_spit(ss_bios);

		if (bios_information* bInfo = static_cast<bios_information*>(catcher))
		{
			electronicsItems.biosVendorName = bInfo->vendor != nullptr ? bInfo->vendor : notAvailableText;
			electronicsItems.biosVersion = bInfo->version != nullptr ? bInfo->version : notAvailableText;
			electronicsItems.biosReleaseDate = bInfo->biosreleasedate != nullptr ? bInfo->biosreleasedate : notAvailableText;
			electronicsItems.biosROMSize = bInfo->biosromsize != nullptr ? bInfo->biosromsize : notAvailableText;
			electronicsItems.biosCharacteristics = bInfo->bioscharacteristics[0] != '\0' ? bInfo->bioscharacteristics : notAvailableText;
		}
		else
		{
			KR_CORE_WARN("BiosReader isn't behaving normally.");
		}

		catcher = electronics_spit(pi_bioslanguages);

		if (mb_language_modules* mbLangModules = static_cast<mb_language_modules*>(catcher))
		{
			electronicsItems.biosCurrentSetLanguage = mbLangModules->currentactivemodule != nullptr ? mbLangModules->currentactivemodule : notAvailableText;
			electronicsItems.biosRestOfTheSupportedLanguages = mbLangModules->supportedlanguagemodules != nullptr ? mbLangModules->supportedlanguagemodules : notAvailableText;
		}
		else
		{
			KR_CORE_WARN("BiosReader isn't behaving normally.");
		}

		catcher = electronics_spit(pi_systemmemory);
		if (turing_machine_system_memory* tInfo = static_cast<turing_machine_system_memory*>(catcher))
		{
			electronicsItems.estimatedCapacity = tInfo->total_grand_capacity != nullptr ? tInfo->total_grand_capacity : notAvailableText;
			electronicsItems.numberOfMemoryDevices = tInfo->number_of_ram_or_system_memory_devices != 0 ? tInfo->number_of_ram_or_system_memory_devices : 0;
			electronicsItems.supportingArea = tInfo->mounting_location != nullptr ? tInfo->mounting_location : notAvailableText;
		}
		else
		{
			KR_CORE_WARN("BiosReader isn't behaving normally.");
		}

		catcher = electronics_spit(ps_systemmemory);

		// Now since there may be more than one Ram type of electronics, and given that BIOS lies, we need a mechanism
		// to gauge the true amount of every estimation we obtained earlier
		// Please see https://github.com/ravimohan1991/BiosReader/blob/37e1179f876b940b3f483a398091f44a479692ea/src/private/dmidecode.c#L4980
		if (random_access_memory* rInfo = static_cast<random_access_memory*>(catcher))
		{
			KarmaTuringMachineElectronics::GaugeSystemMemoryDevices(rInfo);

			if (electronicsItems.ramInformation == nullptr)
			{
				electronicsItems.ramInformation = new KarmaTuringMachineElectronics::SystemRAM[electronicsItems.ramSoftSlots.size()];
			}
			else
			{
				KR_CORE_WARN("ramInformation is already allocated which should have been cleared in the first place.");
			}

			uint32_t counter = 0;

			for (auto& elem : electronicsItems.ramSoftSlots)
			{
				KarmaTuringMachineElectronics::FillTheSystemRamStructure(electronicsItems.ramInformation[counter++], *fetch_access_memory_members(elem));
			}

			KarmaTuringMachineElectronics::FindRealCapacityOfRam();
		}
		else
		{
			KR_CORE_WARN("BiosReader isn't behaving normally.");
		}

		catcher = electronics_spit(ps_processor);

		if (central_processing_unit* pInfo = static_cast<central_processing_unit*>(catcher))
		{
			//electronicsItems.cpuDesignation = pInfo->designation; <----- Please refer to central_processing_unit struct
			electronicsItems.cpuFlags = pInfo->cpuflags != nullptr ? pInfo->cpuflags : notAvailableText;
			electronicsItems.cpuid = pInfo->cpuid != nullptr ? pInfo->cpuid : notAvailableText;
			electronicsItems.cpuManufacturer = pInfo->manufacturer ? pInfo->manufacturer : notAvailableText;
			electronicsItems.cpuProcessingfamily = pInfo->processingfamily ? pInfo->processingfamily : notAvailableText;
			electronicsItems.cpuVersion = pInfo->version ? pInfo->version : notAvailableText;
			electronicsItems.cpuPartNumber = pInfo->partnumber ? pInfo->partnumber : notAvailableText;
			electronicsItems.cpuSerialNumber = pInfo->serialnumber ? pInfo->serialnumber : notAvailableText;
			electronicsItems.cpuAssettag = pInfo->assettag ? pInfo->assettag : notAvailableText;
			electronicsItems.cpuOperatingVoltage = pInfo->operatingvoltage ? pInfo->operatingvoltage : notAvailableText;
			electronicsItems.cpuCurrentSpeed = pInfo->currentspeed ? pInfo->currentspeed : notAvailableText;
			electronicsItems.cpuMaximumSpeed = pInfo->maximumspeed ? pInfo->maximumspeed : notAvailableText;
			electronicsItems.cpuExternalClock = pInfo->externalclock ? pInfo->externalclock : notAvailableText;
			electronicsItems.cpuCorescount = pInfo->corescount ? pInfo->corescount : notAvailableText;
			electronicsItems.cpuThreadCount = pInfo->threadcount ? pInfo->threadcount : notAvailableText;
			electronicsItems.cpuEnabledCoresCount = pInfo->enabledcorescount ? pInfo->enabledcorescount : notAvailableText;
			electronicsItems.cpuTheCharacterstics = pInfo->characterstics ? pInfo->characterstics : notAvailableText;
			electronicsItems.cpuSignature = pInfo->signature ? pInfo->signature : notAvailableText;
		}
		else
		{
			KR_CORE_WARN("BiosReader isn't behaving normally");
		}

		catcher = electronics_spit(ps_graphicscard);

		if (graphics_processing_unit* gInfo = static_cast<graphics_processing_unit*>(catcher))
		{
			electronicsItems.gpuModelIdentification = gInfo->gpuModel != nullptr ? gInfo->gpuModel : notAvailableText;
			electronicsItems.gpuVendor = gInfo->vendor ? gInfo->vendor : notAvailableText;
			electronicsItems.gpuVMemory = gInfo->grandtotalvideomemory ? gInfo->grandtotalvideomemory : notAvailableText;
		}
		else
		{
			KR_CORE_WARN("BiosReader isn't behaving normally");
		}

		electronicsItems.bHasQueried = true;
	}

	//-----------------------------------------------------------------------------
	// [SECTION] MISC HELPERS/UTILITIES (String, Format, Hash functions)
	//-----------------------------------------------------------------------------

	int ImGuiMesa::ImStrlenW(const ImWchar* str)
	{
		//return (int)wcslen((const wchar_t*)str);  // FIXME-OPT: Could use this when wchar_t are 16-bit
		int n = 0;
		while (*str++) n++;
		return n;
	}

	uint32_t ImGuiMesa::ChernUint32FromString(const std::string& ramString)
	{
		std::string digitString;
		//bool ctype = std::isdigit(ramString[0]);

		for (char c : ramString)
		{
			if (std::isdigit(c))
			{
				digitString.push_back(c);
			}
		}

		// object from the class stringstream
		std::stringstream ramSize(digitString);

		uint32_t value = 0;
		ramSize >> value;

		return value;
	}

	std::string ImGuiMesa::ChernDimensionsFromString(const std::string& ramString)
	{
		std::string dimensionString;

		for (char c : ramString)
		{
			if (!std::isdigit(c) && !std::isspace(c))
			{
				dimensionString.push_back(c);
			}
		}

		return dimensionString;
	}

	void ImGuiMesa::SetElectronicsRamInformationToNull()
	{
		if (electronicsItems.ramInformation != nullptr)
		{
			delete[] electronicsItems.ramInformation;
			electronicsItems.ramInformation = nullptr;
		}
	}

	void KarmaTuringMachineElectronics::FindRealCapacityOfRam()
	{
		uint32_t ramSizeFound = 0;

		// Assumption dimension of memory is GB only
		for (uint32_t counter = 0; counter < ImGuiMesa::GetGatheredElectronicsInformation().ramSoftSlots.size(); counter++)
		{
			ramSizeFound += ImGuiMesa::ChernUint32FromString(ImGuiMesa::GetGatheredElectronicsInformation().ramInformation[counter].ramSize);
		}

		ImGuiMesa::GetGatheredElectronicsInformationForModification().totalRamSize = ramSizeFound;

		// Hoping for GB only dimension
		ImGuiMesa::GetGatheredElectronicsInformationForModification().ramSizeDimensions = ImGuiMesa::ChernDimensionsFromString(ImGuiMesa::GetGatheredElectronicsInformation().ramInformation[0].ramSize);
	}

	void KarmaTuringMachineElectronics::GaugeSystemMemoryDevices(random_access_memory* ramCluster)
	{
		if (ramCluster == nullptr)
		{
			KR_CORE_WARN("Memory devices pointer is null. No Ram(s) shall be detected and reported");
			return;
		}

		KarmaTuringMachineElectronics selfRefrentialVariable = ImGuiMesa::GetGatheredElectronicsInformationForModification();

		uint32_t biosReportedNumber = selfRefrentialVariable.numberOfMemoryDevices;

		for (uint32_t counter = 0; counter < biosReportedNumber; counter++)
		{
			random_access_memory* aMemoryBeingScanned = fetch_access_memory_members(counter);

			if (aMemoryBeingScanned != nullptr && IsPhysicalRamPresent(*aMemoryBeingScanned))
			{
				ImGuiMesa::GetGatheredElectronicsInformationForModification().ramSoftSlots.push_back(counter);
			}
		}
	}

	void KarmaTuringMachineElectronics::FillTheSystemRamStructure(SystemRAM& destinationStructure, random_access_memory& sourceStructure)
	{
		destinationStructure.assetTag = sourceStructure.assettag != nullptr ? sourceStructure.assettag : ImGuiMesa::notAvailableText;
		destinationStructure.bankLocator = sourceStructure.banklocator != nullptr ? sourceStructure.banklocator : ImGuiMesa::notAvailableText;
		destinationStructure.configuredMemorySpeed = sourceStructure.configuredmemoryspeed != nullptr ? sourceStructure.configuredmemoryspeed : ImGuiMesa::notAvailableText;
		destinationStructure.memorySpeed = sourceStructure.memoryspeed != nullptr ? sourceStructure.memoryspeed : ImGuiMesa::notAvailableText;
		destinationStructure.formFactor = sourceStructure.formfactor != nullptr ? sourceStructure.formfactor : ImGuiMesa::notAvailableText;
		destinationStructure.locator = sourceStructure.locator != nullptr ? sourceStructure.locator : ImGuiMesa::notAvailableText;
		destinationStructure.manufacturer = sourceStructure.manufacturer != nullptr ? sourceStructure.manufacturer : ImGuiMesa::notAvailableText;
		destinationStructure.operatingVoltage = sourceStructure.operatingvoltage != nullptr ? sourceStructure.operatingvoltage : ImGuiMesa::notAvailableText;
		destinationStructure.partNumber = sourceStructure.partnumber != nullptr ? sourceStructure.partnumber : ImGuiMesa::notAvailableText;
		destinationStructure.ramSize = sourceStructure.ramsize != nullptr ? sourceStructure.ramsize : ImGuiMesa::notAvailableText;
		destinationStructure.ramType = sourceStructure.ramtype != nullptr ? sourceStructure.ramtype : ImGuiMesa::notAvailableText;
		//destinationStructure.rank = sourceStructure.rank; Not a big fan of rank, reminds me of my JEE AIR 4729
		destinationStructure.serialNumber = sourceStructure.serialnumber != nullptr ? sourceStructure.serialnumber : ImGuiMesa::notAvailableText;
	}

	bool KarmaTuringMachineElectronics::IsPhysicalRamPresent(const random_access_memory& ramScam)
	{
		if (ramScam.memoryspeed == nullptr || ramScam.configuredmemoryspeed == nullptr || ramScam.banklocator == nullptr
			|| ramScam.formfactor == nullptr || ramScam.operatingvoltage == nullptr)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	KarmaTuringMachineElectronics::~KarmaTuringMachineElectronics()
	{
		//ImGuiMesa::SetElectronicsRamInformationToNull();
	}
}
