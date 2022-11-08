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
	//std::vector<uint32_t> KarmaTuringMachineElectronics::ramSoftSlots;

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

		ImGui::Text(tagLine);
		ImGui::Text(" "); ImGui::SameLine(tagLineDimensions.x - authorNameDimensions.x); ImGui::Text(authorName);

		ImGui::Separator();

		//-----------------------------------------------------------------------------------------------------------//

		const char* licenseLine_1 = "Karma Engine is a copyleft software and distributed under the umbrella of";
		const char* licenseLine_2 = "GNU GPL v3 https://github.com/ravimohan1991/KarmaEngine/blob/main/LICENSE";
		const char* licenseLine_3 = "Although careful and mindful tending of the \"web of companion libraries\"";
		const char* licenseLine_4 = "lincences has been undertaken, it is a work in progress with the hope of";
		const char* licenseLine_5 = "eliminating the concept of software licensing itself.";

		ImGui::Text(licenseLine_1);
		ImGui::Text(licenseLine_2);
		ImGui::Text(licenseLine_3);
		ImGui::Text(licenseLine_4);
		ImGui::Text(licenseLine_5);

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
				ImGui::Text("Bank Locator / Locator: %s / %s", electronicsItems.ramInformation[counter].bankLocator.c_str(),
					electronicsItems.ramInformation[counter].locator.c_str());
				ImGui::Text("Asset Tag: %s", electronicsItems.ramInformation[counter].assetTag.c_str());
				ImGui::Unindent();

				ImGui::Text("Ram Conditions");
				ImGui::Indent();
				ImGui::Text("Size: %s", electronicsItems.ramInformation[counter].ramSize.c_str());
				ImGui::Text("Operating Voltage: %s", electronicsItems.ramInformation[counter].operatingVoltage.c_str());
				ImGui::Text("Memory / Configured Memory Speed: %s / %s", electronicsItems.ramInformation[counter].memorySpeed.c_str(),
					electronicsItems.ramInformation[counter].configuredMemorySpeed.c_str());
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
			ImGui::Text("Speed (Current / Maximum): %s / %s", electronicsItems.cpuCurrentSpeed.c_str(), electronicsItems.cpuMaximumSpeed.c_str());
			ImGui::Text("External Clock: %s", electronicsItems.cpuExternalClock.c_str());
			ImGui::Text("Cores Enabled / Threads Count: %s / %s", electronicsItems.cpuEnabledCoresCount.c_str(), electronicsItems.cpuThreadCount.c_str());
			ImGui::Text("Cores Count: %s", electronicsItems.cpuCorescount.c_str());
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

			ImGui::Text("sizeof(size_t): %d, sizeof(ImDrawIdx): %d, sizeof(ImDrawVert): %d", (int)sizeof(size_t), (int)sizeof(ImDrawIdx), (int)sizeof(ImDrawVert));
			ImGui::Text("define: __cplusplus=%d", (int)__cplusplus);
			ImGui::Separator();
			ImGui::Text("io.BackendPlatformName: %s", io.BackendPlatformName ? io.BackendPlatformName : "NULL");
			ImGui::Text("io.BackendRendererName: %s", io.BackendRendererName ? io.BackendRendererName : "NULL");
			ImGui::Text("io.ConfigFlags: 0x%08X", io.ConfigFlags);
			if (io.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard)        ImGui::Text(" NavEnableKeyboard");
			if (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad)         ImGui::Text(" NavEnableGamepad");
			if (io.ConfigFlags & ImGuiConfigFlags_NavEnableSetMousePos)     ImGui::Text(" NavEnableSetMousePos");
			if (io.ConfigFlags & ImGuiConfigFlags_NavNoCaptureKeyboard)     ImGui::Text(" NavNoCaptureKeyboard");
			if (io.ConfigFlags & ImGuiConfigFlags_NoMouse)                  ImGui::Text(" NoMouse");
			if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)      ImGui::Text(" NoMouseCursorChange");
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)            ImGui::Text(" DockingEnable");
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)          ImGui::Text(" ViewportsEnable");
			if (io.ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)  ImGui::Text(" DpiEnableScaleViewports");
			if (io.ConfigFlags & ImGuiConfigFlags_DpiEnableScaleFonts)      ImGui::Text(" DpiEnableScaleFonts");
			if (io.MouseDrawCursor)                                         ImGui::Text("io.MouseDrawCursor");
			if (io.ConfigViewportsNoAutoMerge)                              ImGui::Text("io.ConfigViewportsNoAutoMerge");
			if (io.ConfigViewportsNoTaskBarIcon)                            ImGui::Text("io.ConfigViewportsNoTaskBarIcon");
			if (io.ConfigViewportsNoDecoration)                             ImGui::Text("io.ConfigViewportsNoDecoration");
			if (io.ConfigViewportsNoDefaultParent)                          ImGui::Text("io.ConfigViewportsNoDefaultParent");
			if (io.ConfigDockingNoSplit)                                    ImGui::Text("io.ConfigDockingNoSplit");
			if (io.ConfigDockingWithShift)                                  ImGui::Text("io.ConfigDockingWithShift");
			if (io.ConfigDockingAlwaysTabBar)                               ImGui::Text("io.ConfigDockingAlwaysTabBar");
			if (io.ConfigDockingTransparentPayload)                         ImGui::Text("io.ConfigDockingTransparentPayload");
			if (io.ConfigMacOSXBehaviors)                                   ImGui::Text("io.ConfigMacOSXBehaviors");
			if (io.ConfigInputTextCursorBlink)                              ImGui::Text("io.ConfigInputTextCursorBlink");
			if (io.ConfigWindowsResizeFromEdges)                            ImGui::Text("io.ConfigWindowsResizeFromEdges");
			if (io.ConfigWindowsMoveFromTitleBarOnly)                       ImGui::Text("io.ConfigWindowsMoveFromTitleBarOnly");
			if (io.ConfigMemoryCompactTimer >= 0.0f)                        ImGui::Text("io.ConfigMemoryCompactTimer = %.1f", io.ConfigMemoryCompactTimer);
			ImGui::Text("io.BackendFlags: 0x%08X", io.BackendFlags);
			if (io.BackendFlags & ImGuiBackendFlags_HasGamepad)             ImGui::Text(" HasGamepad");
			if (io.BackendFlags & ImGuiBackendFlags_HasMouseCursors)        ImGui::Text(" HasMouseCursors");
			if (io.BackendFlags & ImGuiBackendFlags_HasSetMousePos)         ImGui::Text(" HasSetMousePos");
			if (io.BackendFlags & ImGuiBackendFlags_PlatformHasViewports)   ImGui::Text(" PlatformHasViewports");
			if (io.BackendFlags & ImGuiBackendFlags_HasMouseHoveredViewport)ImGui::Text(" HasMouseHoveredViewport");
			if (io.BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset)   ImGui::Text(" RendererHasVtxOffset");
			if (io.BackendFlags & ImGuiBackendFlags_RendererHasViewports)   ImGui::Text(" RendererHasViewports");
			ImGui::Separator();
			ImGui::Text("io.Fonts: %d fonts, Flags: 0x%08X, TexSize: %d,%d", io.Fonts->Fonts.Size, io.Fonts->Flags, io.Fonts->TexWidth, io.Fonts->TexHeight);
			ImGui::Text("io.DisplaySize: %.2f,%.2f", io.DisplaySize.x, io.DisplaySize.y);
			ImGui::Text("io.DisplayFramebufferScale: %.2f,%.2f", io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
			ImGui::Separator();
			ImGui::Text("style.WindowPadding: %.2f,%.2f", style.WindowPadding.x, style.WindowPadding.y);
			ImGui::Text("style.WindowBorderSize: %.2f", style.WindowBorderSize);
			ImGui::Text("style.FramePadding: %.2f,%.2f", style.FramePadding.x, style.FramePadding.y);
			ImGui::Text("style.FrameRounding: %.2f", style.FrameRounding);
			ImGui::Text("style.FrameBorderSize: %.2f", style.FrameBorderSize);
			ImGui::Text("style.ItemSpacing: %.2f,%.2f", style.ItemSpacing.x, style.ItemSpacing.y);
			ImGui::Text("style.ItemInnerSpacing: %.2f,%.2f", style.ItemInnerSpacing.x, style.ItemInnerSpacing.y);

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
			electronicsItems.biosVendorName = bInfo->vendor != nullptr ? bInfo->vendor : "Kasturi Trishna (The MuskThirst)";
			electronicsItems.biosVersion = bInfo->version != nullptr ? bInfo->version : "Kasturi Trishna (The MuskThirst)";
			electronicsItems.biosReleaseDate = bInfo->biosreleasedate != nullptr ? bInfo->biosreleasedate : "Kasturi Trishna (The MuskThirst)";
			electronicsItems.biosROMSize = bInfo->biosromsize ? bInfo->biosromsize : "Kasturi Trishna (The MuskThirst)";
			electronicsItems.biosCharacteristics = bInfo->bioscharacteristics ? bInfo->bioscharacteristics : "Kasturi Trishna (The MuskThirst)";
		}
		else
		{
			KR_CORE_WARN("BiosReader isn't behaving normally.");
		}

		catcher = electronics_spit(pi_systemmemory);
		if (turing_machine_system_memory* tInfo = static_cast<turing_machine_system_memory*>(catcher))
		{
			electronicsItems.estimatedCapacity = tInfo->total_grand_capacity;
			electronicsItems.numberOfMemoryDevices = tInfo->number_of_ram_or_system_memory_devices;
			electronicsItems.supportingArea = tInfo->mounting_location;
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
			electronicsItems.cpuFlags = pInfo->cpuflags;
			electronicsItems.cpuid = pInfo->cpuid;
			electronicsItems.cpuManufacturer = pInfo->manufacturer;
			electronicsItems.cpuProcessingfamily = pInfo->processingfamily;
			electronicsItems.cpuVersion = pInfo->version;
			electronicsItems.cpuPartNumber = pInfo->partnumber;
			electronicsItems.cpuSerialNumber = pInfo->serialnumber;
			electronicsItems.cpuAssettag = pInfo->assettag;
			electronicsItems.cpuOperatingVoltage = pInfo->operatingvoltage;
			electronicsItems.cpuCurrentSpeed = pInfo->currentspeed;
			electronicsItems.cpuMaximumSpeed = pInfo->maximumspeed;
			electronicsItems.cpuExternalClock = pInfo->externalclock;
			electronicsItems.cpuCorescount = pInfo->corescount;
			electronicsItems.cpuThreadCount = pInfo->threadcount;
			electronicsItems.cpuEnabledCoresCount = pInfo->enabledcorescount;
			electronicsItems.cpuTheCharacterstics = pInfo->characterstics;
			electronicsItems.cpuSignature = pInfo->signature;
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
		destinationStructure.assetTag = sourceStructure.assettag;
		destinationStructure.bankLocator = sourceStructure.banklocator;
		destinationStructure.configuredMemorySpeed = sourceStructure.configuredmemoryspeed;
		destinationStructure.memorySpeed = sourceStructure.memoryspeed;
		destinationStructure.formFactor = sourceStructure.formfactor;
		destinationStructure.locator = sourceStructure.locator;
		destinationStructure.manufacturer = sourceStructure.manufacturer;
		destinationStructure.operatingVoltage = sourceStructure.operatingvoltage;
		destinationStructure.partNumber = sourceStructure.partnumber;
		destinationStructure.ramSize = sourceStructure.ramsize;
		destinationStructure.ramType = sourceStructure.ramtype;
		//destinationStructure.rank = sourceStructure.rank; Not a big fan of rank, reminds me of my JEE AIR 4729
		destinationStructure.serialNumber = sourceStructure.serialnumber;
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