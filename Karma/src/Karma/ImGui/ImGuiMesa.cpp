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

			if(payloadWindow)
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
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				DrawMainMenuFileListMesa();
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Details"))
			{
				if(ImGui::MenuItem("About")) {}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
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
}
