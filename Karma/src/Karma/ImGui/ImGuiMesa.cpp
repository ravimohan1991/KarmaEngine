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

		//-----------------------------------------------------------------------------------------------------------//

		ImGui::End();
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
}