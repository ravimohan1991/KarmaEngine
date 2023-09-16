/*
 A mesa is an isolated, flat-topped elevation, ridge or hill, which is bounded from all sides by steep escarpments and stands distinctly above a surrounding plain.
 Courtsey https://en.wikipedia.org/wiki/Mesa
 */

 /*
  Following code snippets are heavily influenced from the official Dear ImGUI windows (isn't that always the scenario?). Please see https://github.com/ravimohan1991/imgui/blob/9068fd1afbaba56247aacd452bbadebb286800c7/imgui_demo.cpp for the historical and legacy purposes.

  Also see https://en.wikipedia.org/wiki/Indian_Mesa, since I am Indian! And I still love my country!!
  */

#include "KarmaGuiMesa.h"
#include "Karma/Application.h"
#include "Karma/Renderer/RendererAPI.h"
#include "spdlog/sinks/callback_sink.h"

namespace Karma
{
	KarmaTuringMachineElectronics KarmaGuiMesa::electronicsItems;
	std::string KarmaGuiMesa::notAvailableText = "Kasturi Trishna (The MuskThirst)";
	bool KarmaGuiMesa::m_ViewportFocused = false;
	bool KarmaGuiMesa::m_ViewportHovered = false;
	KarmaLogMesa KarmaGuiMesa::m_KarmaLog;
	KarmaGuiTextBuffer     KarmaLogMesa::TextBuffer;
	KarmaGuiTextFilter     KarmaLogMesa::TextFilter;
	KGVector<int>       KarmaLogMesa::LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
	bool                KarmaLogMesa::AutoScroll;  // Keep scrolling if already at the bottom.
	std::shared_ptr<spdlog::logger> s_MesaCoreLogger = nullptr;
	std::shared_ptr<spdlog::logger> s_MesaClientLogger = nullptr;
	std::shared_ptr<spdlog::pattern_formatter> s_MesaLogFormatter = nullptr;
	bool KarmaGuiMesa::m_EditorInitialized = false;
	bool KarmaGuiMesa::m_RefreshRenderingResources = false;

	WindowManipulationGaugeData KarmaGuiMesa::m_3DExhibitor;

	KarmaGuiDockPreviewData::KarmaGuiDockPreviewData() : FutureNode(0)
	{
		IsDropAllowed = IsCenterAvailable = IsSidesAvailable = IsSplitDirExplicit = false; 
		SplitNode = NULL; SplitDir = KGGuiDir_None; SplitRatio = 0.f; 
		
		for (int n = 0; n < KG_ARRAYSIZE(DropRectsDraw); n++)
		{
			DropRectsDraw[n] = KGRect(+FLT_MAX, +FLT_MAX, -FLT_MAX, -FLT_MAX);
		}
	}

	void KarmaGuiMesa::RevealMainFrame(KGGuiID mainMesaDockID, std::shared_ptr<Scene> scene, const CallbacksFromEditor& editorCallbacks)
	{
		// The MM (Main Menu) menu bar
		DrawKarmaMainMenuBarMesa();

		// 2. Show a simple sampling and experiment window
		{
			static bool show = true;
			static float fValue = 0.0f;
			static int counter = 0;

			KarmaGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and appeninto it

			KarmaGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			KarmaGui::Checkbox("Demo Window", &show);                  // Edit bools storing our window open/close state
			KarmaGui::Checkbox("Another Window", &show);

			KarmaGui::SliderFloat("float", &fValue, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a colo

			if (KarmaGui::Button("Button"))                            // Buttons return true when clicked (most widgets returtrue when edited/activated)
				counter++;
			KarmaGui::SameLine();

			KarmaGui::Text("counter = %d", counter);
			KarmaGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / KarmaGui::GetIO().Framerate, KarmaGui::GetIO().Framerate);

			KGGuiDockNode* node = nullptr;
			KGGuiWindow* window = nullptr;//ImGui::FindDockSpaceByID(mainMesaDockID);
			KGGuiWindow* payloadWindow = nullptr;//ImGui::FindWindowByName("Karma: Log");

			int boxNumber = -5;
			node = nullptr;//ImGui::FindAppropriateNode(window, payloadWindow, boxNumber);

			KarmaGui::Text("Node ID = %d at position x = %f, y = %f on docking box %d", node != nullptr ? node->ID : 0, KarmaGui::GetMousePos().x, KarmaGui::GetMousePos().y, boxNumber);

			if (payloadWindow)
				KarmaGui::Text("Karma: Log window is of dimension width = %f und height = %f", payloadWindow->Size.x, payloadWindow->Size.y);
			KarmaGui::End();
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

		// 5. A window for 3D rendering part
		{
			Draw3DModelExhibitorMesa(scene);
		}

		// 6. The content browser
		{
			DrawContentBrowser(editorCallbacks.openSceneCallback);
		}

		// 7. Memory-usage exhibhitor
		{
			DrawMemoryExhibitor();
		}

		// Display ready log message and do one time initialization stuff
		{
			if(!m_EditorInitialized)
			{
				KR_INFO("Pranjal is prepared for work.");
				m_EditorInitialized = true;
			}
		}
	}

	KGGuiDockNode* KarmaGuiMesa::DockNodeTreeFindFallbackLeafNode(KGGuiDockNode* node)
	{
		if (node->IsLeafNode())
			return node;
		if (KGGuiDockNode* leaf_node = DockNodeTreeFindFallbackLeafNode(node->ChildNodes[0]))
			return leaf_node;
		if (KGGuiDockNode* leaf_node = DockNodeTreeFindFallbackLeafNode(node->ChildNodes[1]))
			return leaf_node;
		return NULL;
	}

	//-----------------------------------------------------------------------------
	// [SECTION] A variety of KarmaGui  mesas
	//-----------------------------------------------------------------------------

	// Once we have projects, change this
	extern const std::filesystem::path g_AssetPath = "assets";
	std::filesystem::path KarmaGuiMesa::m_CurrentDirectory = std::filesystem::current_path();
	uint32_t KarmaGuiMesa::m_DirectoryIcon = 3;
	uint32_t KarmaGuiMesa::m_FileIcon = 2;

	void KarmaGuiMesa::DrawMemoryExhibitor()
	{
		KarmaGuiWindowFlags windowFlags =  KGGuiWindowFlags_HorizontalScrollbar;

		// fiddle this parameter on increasing / decreasing memoryBlockWidth
		KarmaGui::SetNextWindowContentSize(KGVec2(1450, 500));
		KarmaGui::Begin("Memory Exhibitor", nullptr, windowFlags);

		KGDrawList* drawList = KarmaGui::GetWindowDrawList();

		static float x, y;
		static float bareToFrameX, bareToFrameY;
		static float bareXBL, bareYBL;
		static float bareXTR, bareYTR;
		static float memoryBlockWidth = 1250;
		static float memoryBlockHeight = 150;
		static KGVec4 legendTextColor = KGVec4(0.0f, 1.0f, 0.0f, 1.0f);
		static KGU32 occupiedMemoryColor = KG_COL32(128, 128, 128, 100);
		static KGU32 arrowColor = KG_COL32(255, 215, 0, 255);
		static float occupiedMemoryPercent = 25.0f;

		KGGuiWindow* currentWindow = KarmaGuiInternal::GetCurrentWindow();

		bareXBL = 45;
		bareYBL = 100 + memoryBlockHeight;
		bareToFrameX = currentWindow->Pos.x - KarmaGui::GetScrollX();
		bareToFrameY = currentWindow->Pos.y - KarmaGui::GetScrollY();

		x = bareXBL + bareToFrameX;
		y = bareYBL + bareToFrameY;
		KGVec2 bottomLeftCoordinates = KGVec2(x, y);
		x = x + memoryBlockWidth;
		y = y - memoryBlockHeight;
		KGVec2 topRightCoordinates = KGVec2(x, y);

		KGVec2 fillerTopRightCoordinates = KGVec2(topRightCoordinates.x - (1 - occupiedMemoryPercent / 100) * memoryBlockWidth, topRightCoordinates.y);

		KarmaGui::SliderFloat("Memory Occupied", &occupiedMemoryPercent, 0.0f, 100.0f);

		// Draw total memory block and occupied memory
		drawList->AddRectFilled(bottomLeftCoordinates, topRightCoordinates, KG_COL32_WHITE);
		drawList->AddRectFilled(bottomLeftCoordinates, fillerTopRightCoordinates, occupiedMemoryColor);

		static std::string addressText = "0x001BB";
		static KGVec2 addressTextSize = KarmaGui::CalcTextSize(addressText.c_str());

		KGVec2 pointerRectangleCoordinatesMin, pointerRectangleCoordinatesMax;
		KGVec2 cursorPosition;

		// Draw appropriate lables for display of addresses explicitly
		{
			KarmaGuiInternal::RenderArrowPointingAt(drawList, bottomLeftCoordinates, KGVec2(5, 16), KGGuiDir_Up, arrowColor);
			pointerRectangleCoordinatesMin = KGVec2(bottomLeftCoordinates.x - 2.5, bottomLeftCoordinates.y + 16 + addressTextSize.y);
			pointerRectangleCoordinatesMax = KGVec2(bottomLeftCoordinates.x - 2.5 + addressTextSize.x, bottomLeftCoordinates.y + 16);
			drawList->AddRect(pointerRectangleCoordinatesMin, pointerRectangleCoordinatesMax, KG_COL32_BLACK);
			cursorPosition = KGVec2(pointerRectangleCoordinatesMin.x - bareToFrameX, pointerRectangleCoordinatesMin.y - bareToFrameY - addressTextSize.y);
			KarmaGui::SetCursorPos(cursorPosition);
			KarmaGui::TextColored(legendTextColor, "%s", addressText.c_str());
		}

		{
			KarmaGuiInternal::RenderArrowPointingAt(drawList, fillerTopRightCoordinates, KGVec2(5, 16), KGGuiDir_Down, arrowColor);
			pointerRectangleCoordinatesMin = KGVec2(fillerTopRightCoordinates.x - addressTextSize.x / 2, fillerTopRightCoordinates.y - 16);
			pointerRectangleCoordinatesMax = KGVec2(fillerTopRightCoordinates.x + addressTextSize.x / 2, fillerTopRightCoordinates.y - 16 - addressTextSize.y);
			drawList->AddRect(pointerRectangleCoordinatesMin, pointerRectangleCoordinatesMax, KG_COL32_BLACK);
			cursorPosition = KGVec2(pointerRectangleCoordinatesMin.x - bareToFrameX, pointerRectangleCoordinatesMin.y - bareToFrameY - addressTextSize.y);
			KarmaGui::SetCursorPos(cursorPosition);
			KarmaGui::TextColored(legendTextColor, "%s", addressText.c_str());
		}

		static KGVec2 textSize = KarmaGui::CalcTextSize("Memory Quota for UObjects");

		KarmaGui::SetCursorPos(KGVec2((bareXBL + memoryBlockWidth) / 2 - textSize.x / 2, bareYBL));// local coordinates, scrolling included
		KarmaGui::TextColored(legendTextColor, "Memory Quota for UObjects");

		static KGFont* verticalTextFont = KarmaGui::GetFont();
		verticalTextFont->Scale = 0.6f;

		KarmaGui::PushFont(verticalTextFont);
		static KGVec2 textSize2 = KarmaGui::CalcTextSize("Vertical Text");
		KarmaGui::AddTextVertical(drawList, "Vertical Text", KGVec2(topRightCoordinates.x - memoryBlockWidth / 2, bottomLeftCoordinates.y - memoryBlockHeight / 2 + textSize2.x / 2), KG_COL32_BLACK);
		//KarmaGui::TextColored(legendTextColor, "Memory Quota for UObjects");
		KarmaGui::PopFont();
		verticalTextFont->Scale = 1.0f;

		KarmaGui::End();
	}

	void KarmaGuiMesa::DrawContentBrowser(const std::function< void(std::string) >& openSceneCallback)
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		KarmaGuiBackendRendererUserData* backendData = KarmaGuiRenderer::GetBackendRendererUserData();

		KarmaGui::Begin("Content Browser");

		if (m_CurrentDirectory != std::filesystem::path(g_AssetPath))
		{
			//static uint32_t buttonPositionY = ImGui::GetCurrentWindow()->DC.CursorPos.y;
			KarmaGui::PushStyleColor(KGGuiCol_Button, KGVec4(0, 0, 0, 0));
			if (KarmaGui::ImageButton("Up Button", backendData->GetTextureIDAtIndex(5), {20.0f, 20.0f}))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
			KarmaGui::PopStyleColor();

			KarmaGui::SameLine(0.0f, 5.0f);
			KarmaGui::Text("%s", m_CurrentDirectory.string().c_str());
		}

		// Vertical padding
		// Hmm, using KarmaGuiInternal is ok?
		KarmaGuiInternal::GetCurrentWindow()->DC.CursorPos.y += 20;

		static float padding = 16.0f;
		static float thumbnailSize = 80.0f;

		float cellSize = thumbnailSize + padding;
		float panelWidth = KarmaGui::GetContentRegionAvail().x;

		int columnCount = (int)(panelWidth / cellSize);

		if (columnCount < 1)
		{
			columnCount = 1;
		}

		KarmaGui::Columns(columnCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			std::string filenameString = path.filename().string();

			KarmaGui::PushID(filenameString.c_str());
			uint32_t iconNumber = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;

			KarmaGui::PushStyleColor(KGGuiCol_Button, KGVec4(0, 0, 0, 0));

			KarmaGui::ImageButton("Content Browser", backendData->GetTextureIDAtIndex(iconNumber), {thumbnailSize, thumbnailSize}, {0, 1}, {1, 0});

			KarmaGui::PopStyleColor();

			if (KarmaGui::IsItemHovered() && KarmaGui::IsMouseDoubleClicked(KGGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory())
				{
					m_CurrentDirectory /= path.filename();
				}
				else if(directoryEntry.is_regular_file() && (path.filename().extension() == ".obj"))
				{
					KR_INFO("Opening a scene from {0}", path.string().c_str());
					openSceneCallback(path.string());

					m_RefreshRenderingResources = true;
				}
			}

			KarmaGui::TextWrapped("%s", filenameString.c_str());
			KarmaGui::NextColumn();

			KarmaGui::PopID();
		}

		KarmaGui::Columns(1);

		KarmaGui::End();
	}

	void KarmaGuiMesa::Draw3DModelExhibitorMesa(std::shared_ptr<Scene> scene)
	{
		KarmaGuiWindowFlags windowFlags = KGGuiWindowFlags_NoScrollWithMouse | KGGuiWindowFlags_NoScrollbar;
		KarmaGui::Begin("3D Exhibitor", nullptr, windowFlags);
		KarmaGui::SetNextWindowSize(KGVec2(400, 400), KGGuiCond_FirstUseEver);

		KGVec4 bgColor;
		bgColor.x = 1.0f;
		bgColor.y = 1.0f;
		bgColor.z = 1.0f;
		bgColor.w = 1.0f;

		KarmaGui::PushStyleColor(KGGuiCol_WindowBg, KarmaGui::GetColorU32(bgColor));

		KGGuiWindow* window = KarmaGuiInternal::GetCurrentWindow();

		m_ViewportFocused = KarmaGui::IsWindowFocused();
		m_ViewportHovered = KarmaGui::IsWindowHovered() && !((window->Pos.y + window->TitleBarHeight()) * KarmaGui::GetIO().DisplayFramebufferScale.y > KarmaGui::GetMousePos().y);

		KarmaGuiIO& io = KarmaGui::GetIO();

		KGTextureID backgroundImageTextureID = 0;

		uint32_t width = 0;
		uint32_t height = 0;

		KarmaGuiBackendRendererUserData* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		backgroundImageTextureID = backendData->GetTextureIDAtIndex(1);
		width = backendData->GetTextureWidthAtIndex(1);
		height = backendData->GetTextureHeightAtIndex(1);


		//ImGui::GetCurrentWindow()->DrawList->SetWindowBackgroundColor(bgColor);

		KGDrawCallback sceneCallBack = [](const KGDrawList* parentList, const KGDrawCmd* drawCommand)
		{
			//KR_CORE_INFO("Scene Callback");
		};

		//KGGuiWindow* theWindow = KarmaGuiInternal::GetCurrentWindow();
		scene->SetRenderWindow(window);

		{
			KGVec2 uvMin = KGVec2(0.0f, 0.0f);                 // Top-left
			KGVec2 uvMax = KGVec2(1.0f, 1.0f);                 // Lower-right
			KGVec4 tint_col = KGVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
			KGVec4 border_col = KGVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
			KarmaGui::Image(backgroundImageTextureID, KGVec2(window->Size.x, window->Size.y), uvMin, uvMax, tint_col, border_col);
		}

		if(window->Size.x != m_3DExhibitor.widthCache || window->Size.y != m_3DExhibitor.heightCache)
		{
			scene->SetWindowToRenderWithinResize(true);

			m_3DExhibitor.widthCache = window->Size.x;
			m_3DExhibitor.heightCache = window->Size.y;
		}
		else if (window->Pos.x != m_3DExhibitor.startXCache || window->Pos.y != m_3DExhibitor.startYCache)
		{
			scene->SetWindowToRenderWithinResize(true);

			m_3DExhibitor.startYCache = window->Pos.y;
			m_3DExhibitor.startXCache = window->Pos.x;
		}
		else if(io.DisplaySize.x != m_3DExhibitor.ioDisplayXCache || io.DisplaySize.y != m_3DExhibitor.ioDisplayYCache)
		{
			scene->SetWindowToRenderWithinResize(true);

			m_3DExhibitor.ioDisplayXCache = io.DisplaySize.x;
			m_3DExhibitor.ioDisplayYCache = io.DisplaySize.y;
		}
		else
		{
			scene->SetWindowToRenderWithinResize(false);
		}

		KarmaGuiInternal::GetCurrentWindow()->DrawList->AddCallback(sceneCallBack, (void*)scene.get());
		
		if (m_RefreshRenderingResources)
		{
			scene->SetWindowToRenderWithinResize(true);
			m_RefreshRenderingResources = false;
		}

		KarmaGui::PopStyleColor();
		KarmaGui::End();
	}

	void KarmaGuiMesa::DrawKarmaSceneHierarchyPanelMesa()
	{
		KarmaGui::SetNextWindowSize(KGVec2(500, 400), KGGuiCond_FirstUseEver);
		
		KarmaGui::Begin("Scene Hierarchy");
		KarmaGui::Text("Some Stuff 1");
		KarmaGui::Text("Some stuff 2");
		KarmaGui::Text("lumbdaa");
		KarmaGui::End();
	}

	// MM bar mesa
	void KarmaGuiMesa::DrawKarmaMainMenuBarMesa()
	{
		static bool showKarmaAbout = false;

		if (KarmaGui::BeginMainMenuBar())
		{
			if (KarmaGui::BeginMenu("File"))
			{
				DrawMainMenuFileListMesa();
				KarmaGui::EndMenu();
			}
			if (KarmaGui::BeginMenu("Details"))
			{
				if (KarmaGui::MenuItem("About", nullptr, &showKarmaAbout));
				KarmaGui::EndMenu();
			}
			KarmaGui::EndMainMenuBar();
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
				KarmaGuiMesa::SetElectronicsRamInformationToNull();
				electronicsItems.ramSoftSlots.clear();
				electronicsItems.bHasQueried = false;
			}
		}
	}

	// Log mesa
	// The lougging window with basic filtering.
	void KarmaGuiMesa::DrawKarmaLogMesa(KGGuiID mainMesaDockID)
	{
		KGVec2 windowSize = KGVec2(680, 420);

		KarmaGuiCond conditions = KGGuiCond_Once;
		KarmaGui::SetNextWindowSize(windowSize, conditions);

		// Disable user resize,
		KarmaGuiWindowFlags windowFlags = KGGuiWindowFlags_NoResize;

		KarmaGui::Begin("Karma: Log", nullptr, windowFlags);

		if(s_MesaLogFormatter == nullptr)
		{
			s_MesaLogFormatter.reset(new spdlog::pattern_formatter());
		}

		if(s_MesaCoreLogger == nullptr)
		{
			s_MesaCoreLogger = Log::GetCoreLogger();
			auto callbackSink = std::make_shared<spdlog::sinks::callback_sink_mt>([](const spdlog::details::log_msg &msg)
			{
				spdlog::memory_buf_t logToDisplay;
				s_MesaLogFormatter->format(msg, logToDisplay);

				KarmaGuiMesa::m_KarmaLog.AddLog(fmt::to_string(logToDisplay).c_str());
			});

			callbackSink->set_level(spdlog::level::trace);

			if (s_MesaCoreLogger)
			{
				s_MesaCoreLogger->add_sink(callbackSink);
			}
		}

		if(s_MesaClientLogger == nullptr)
		{
			s_MesaClientLogger = Log::GetClientLogger();
			auto callbackSink = std::make_shared<spdlog::sinks::callback_sink_mt>([](const spdlog::details::log_msg &msg)
			{
				spdlog::memory_buf_t logToDisplay;
				s_MesaLogFormatter->format(msg, logToDisplay);

				KarmaGuiMesa::m_KarmaLog.AddLog(fmt::to_string(logToDisplay).c_str());
			});

			callbackSink->set_level(spdlog::level::trace);
			
			if (s_MesaClientLogger)
			{
				s_MesaClientLogger->add_sink(callbackSink);
			}
		}

		KarmaGui::End();

		// Actually call in the regular Log helper (which will Begin() into the same window as we just did)
		m_KarmaLog.Draw("Karma: Log");
	}

	// Menu mesa
	void KarmaGuiMesa::DrawMainMenuFileListMesa()
	{
		if (KarmaGui::MenuItem("Open", "Ctrl+O")) {}
		if (KarmaGui::BeginMenu("Open Recent"))
		{
			// write code with some memorized history
			KarmaGui::EndMenu();
		}

		KarmaGui::Separator();

		if (KarmaGui::MenuItem("Quit", "Alt+F4"))
		{
			Application::Get().CloseApplication();
		}
	}

	// About mesa
	void KarmaGuiMesa::ShowAboutKarmaMesa(bool* pbOpen)
	{
		if (!KarmaGui::Begin("Karma Engine", pbOpen, KGGuiWindowFlags_AlwaysAutoResize))
		{
			KarmaGui::End();
			return;
		}

		// Gather relevant data
		QueryForTuringMachineElectronics();

		//-----------------------------------------------------------------------------------------------------------//

		// Precomputation based on text, for gauging the image dimensions
		const char* tagLine = "Nothing is impossible once you have the Source Code (and know how to use it)!";
		KGVec2 tagLineDimensions = KarmaGui::CalcTextSize(tagLine, nullptr, false, 0.0f);

		const char* authorName = "The_Cowboy";
		KGVec2 authorNameDimensions = KarmaGui::CalcTextSize(authorName, nullptr, false, 0.0f);

		//-----------------------------------------------------------------------------------------------------------//

		KarmaGuiIO& io = KarmaGui::GetIO();

		KGTextureID aboutImageTextureID = 0;

		uint32_t width = 0;
		uint32_t height = 0;

		KarmaGuiBackendRendererUserData* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		aboutImageTextureID = backendData->GetTextureIDAtIndex(0);
		width = backendData->GetTextureWidthAtIndex(0);
		height = backendData->GetTextureHeightAtIndex(0);

		{
			KGVec2 position = KarmaGui::GetCursorScreenPos();

			KGVec2 uvMin = KGVec2(0.0f, 0.0f);                 // Top-left
			KGVec2 uvMax = KGVec2(1.0f, 1.0f);                 // Lower-right
			KGVec4 tint_col = KGVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
			KGVec4 border_col = KGVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
			KarmaGui::Image(aboutImageTextureID, KGVec2(width, height), uvMin, uvMax, tint_col, border_col);
		}

		//-----------------------------------------------------------------------------------------------------------//

		KarmaGui::Text("%s", tagLine);
		KarmaGui::Text(" "); KarmaGui::SameLine(tagLineDimensions.x - authorNameDimensions.x); KarmaGui::Text("%s", authorName);

		KarmaGui::Separator();

		//-----------------------------------------------------------------------------------------------------------//

		const char* licenseLine_1 = "Karma Engine is a copyleft software and distributed under the umbrella of";
		const char* licenseLine_2 = "GNU GPL v3 https://github.com/ravimohan1991/KarmaEngine/blob/main/LICENSE";
		const char* licenseLine_3 = "Although careful and mindful tending of the \"web of companion libraries\"";
		const char* licenseLine_4 = "lincences has been undertaken, it is a work in progress with the hope of";
		const char* licenseLine_5 = "eliminating the concept of software licensing itself.";

		KarmaGui::Text("%s", licenseLine_1);
		KarmaGui::Text("%s", licenseLine_2);
		KarmaGui::Text("%s", licenseLine_3);
		KarmaGui::Text("%s", licenseLine_4);
		KarmaGui::Text("%s", licenseLine_5);

		KarmaGui::Separator();

		//-----------------------------------------------------------------------------------------------------------//

		static bool showPhysicalRigInformation = false;
		KarmaGui::Checkbox("Config/Build Information", &showPhysicalRigInformation);
		if (showPhysicalRigInformation)
		{
			KarmaGuiIO& io = KarmaGui::GetIO();
			KarmaGuiStyle& style = KarmaGui::GetStyle();

			bool copy_to_clipboard = KarmaGui::Button("Copy to clipboard");
		}

		KarmaGui::Separator();

		//-----------------------------------------------------------------------------------------------------------//

		KarmaGui::Text("Credits und Acknowledgements");
		KarmaGui::Text("Cherno");
		KarmaGui::Text("Travis V Roman");
		KarmaGui::Text("GitHub");
		KarmaGui::Text("Companion Libraries Authors");
		KarmaGui::Text("Sumo India");

		KarmaGui::Separator();

		//-----------------------------------------------------------------------------------------------------------//

		// Finally the Turing Machine's electronics information presentation!
		static bool bShowTuringElectronics = false;
		KarmaGui::Checkbox("Turing Machine Information", &bShowTuringElectronics);
		if (bShowTuringElectronics)
		{
			KarmaGuiIO& io = KarmaGui::GetIO();
			KarmaGuiStyle& style = KarmaGui::GetStyle();

			bool bCopyToClipboard = KarmaGui::Button("Copy to clipboard");
			KGVec2 childSize = KGVec2(0, KarmaGui::GetTextLineHeightWithSpacing() * 18);
			KarmaGui::BeginChildFrame(KarmaGui::GetID("cfg_infos"), childSize, KGGuiWindowFlags_NoMove);

			if (bCopyToClipboard)
			{
				KarmaGui::LogToClipboard();
				KarmaGui::LogText("```\n"); // Back quotes will make text appears without formatting when pasting on GitHub
			}

			KarmaGui::Text("Machine BIOS (v%s)", electronicsItems.biosVersion.c_str());
			KarmaGui::Separator();

			KarmaGui::Text("Vendor: %s", electronicsItems.biosVendorName.c_str());
			KarmaGui::Text("Supplied On: %s", electronicsItems.biosReleaseDate.c_str());
			KarmaGui::Text("ROM Size: %s", electronicsItems.biosROMSize.c_str());
			KarmaGui::Text("Current Language: %s", electronicsItems.biosCurrentSetLanguage.c_str());
			KarmaGui::Text("Supported Languages:");
			KarmaGui::Indent();
			KarmaGui::Text("%s", electronicsItems.biosRestOfTheSupportedLanguages.c_str());
			KarmaGui::Unindent();
			KarmaGui::Text("BIOS Characteristics:");
			KarmaGui::Indent();
			KarmaGui::Text("%s", electronicsItems.biosCharacteristics.c_str());
			KarmaGui::Unindent();
			KarmaGui::Separator();

			KarmaGui::Text("Machine System Memory (RAM and all that)");
			KarmaGui::Separator();

			KarmaGui::Text("Supporting Area: %s", electronicsItems.supportingArea.c_str());
			KarmaGui::Text("Estimated Capacity: %s", electronicsItems.estimatedCapacity.c_str());
			KarmaGui::Text("Total such devices (est): %d", electronicsItems.numberOfMemoryDevices);
			KarmaGui::Text("Physical devices present:");

			for (uint32_t counter = 0; counter < electronicsItems.ramSoftSlots.size(); counter++)
			{
				KarmaGui::Text("RAM %d", counter + 1);
				KarmaGui::Text("Manufacturer: %s", electronicsItems.ramInformation[counter].manufacturer.c_str());

				KarmaGui::Text("Identification Parameters");

				KarmaGui::Indent();
				KarmaGui::Text("Ram Type: %s", electronicsItems.ramInformation[counter].ramType.c_str());
				KarmaGui::Text("Part Number: %s", electronicsItems.ramInformation[counter].partNumber.c_str());
				KarmaGui::Text("Serial Number: %s", electronicsItems.ramInformation[counter].serialNumber.c_str());
				KarmaGui::Text("(Bank | Device) Locator: %s | %s", electronicsItems.ramInformation[counter].bankLocator.c_str(),
					electronicsItems.ramInformation[counter].locator.c_str());
				KarmaGui::Text("Asset Tag: %s", electronicsItems.ramInformation[counter].assetTag.c_str());
				KarmaGui::Unindent();

				KarmaGui::Text("Ram Conditions");
				KarmaGui::Indent();
				KarmaGui::Text("Size: %s", electronicsItems.ramInformation[counter].ramSize.c_str());
				KarmaGui::Text("Operating Voltage: %s", electronicsItems.ramInformation[counter].operatingVoltage.c_str());
				KarmaGui::Text("Speed (Current | Maximum): %s | %s", electronicsItems.ramInformation[counter].configuredMemorySpeed.c_str(),
					electronicsItems.ramInformation[counter].memorySpeed.c_str());
				KarmaGui::Text("Form Factor: %s", electronicsItems.ramInformation[counter].formFactor.c_str());
				KarmaGui::Unindent();
			}

			KarmaGui::Text("RAM Logistics");
			KarmaGui::Indent();
			KarmaGui::Text("Total Ram Size: %d %s", electronicsItems.totalRamSize, electronicsItems.ramSizeDimensions.c_str());
			KarmaGui::Unindent();

			KarmaGui::Separator();

			KarmaGui::Text("Central Processor Unit");
			KarmaGui::Separator();

			KarmaGui::Text("Manufacturer: %s", electronicsItems.cpuManufacturer.c_str());
			KarmaGui::Text("Processor Family: %s", electronicsItems.cpuProcessingfamily.c_str());
			KarmaGui::Text("Version: %s", electronicsItems.cpuVersion.c_str());
			KarmaGui::Text("CPU Conditions");
			KarmaGui::Indent();
			KarmaGui::Text("Speed (Current | Maximum): %s | %s", electronicsItems.cpuCurrentSpeed.c_str(), electronicsItems.cpuMaximumSpeed.c_str());
			KarmaGui::Text("External Clock: %s", electronicsItems.cpuExternalClock.c_str());
			KarmaGui::Text("Cores (Current | Maximum): %s | %s", electronicsItems.cpuEnabledCoresCount.c_str(), electronicsItems.cpuCorescount.c_str());
			KarmaGui::Text("Threads Count: %s", electronicsItems.cpuThreadCount.c_str());
			KarmaGui::Text("Operating Voltage: %s", electronicsItems.cpuOperatingVoltage.c_str());
			KarmaGui::Unindent();
			KarmaGui::Text("CPU Tags or Numbers");
			KarmaGui::Indent();
			KarmaGui::Text("Signature: %s", electronicsItems.cpuSignature.c_str());
			KarmaGui::Text("ID: %s", electronicsItems.cpuid.c_str());
			KarmaGui::Text("Part Number: %s", electronicsItems.cpuPartNumber.c_str());
			KarmaGui::Text("Serial Number: %s", electronicsItems.cpuSerialNumber.c_str());
			KarmaGui::Text("Asset Tag: %s", electronicsItems.cpuAssettag.c_str());
			KarmaGui::Unindent();
			KarmaGui::Text("CPU Characteristics");
			KarmaGui::Indent();
			KarmaGui::Text("%s", electronicsItems.cpuTheCharacterstics.c_str());
			KarmaGui::Unindent();
			KarmaGui::Text("Flags:");
			KarmaGui::Indent();
			KarmaGui::Text("%s", electronicsItems.cpuFlags.c_str());
			KarmaGui::Unindent();

			KarmaGui::Separator();

			KarmaGui::Text("Graphics Processing Unit");
			KarmaGui::Separator();

			KarmaGui::Text("Manufacturer: %s", electronicsItems.gpuVendor.c_str());
			KarmaGui::Text("Model: %s", electronicsItems.gpuModelIdentification.c_str());
			KarmaGui::Text("VRam: %s", electronicsItems.gpuVMemory.c_str());

			KarmaGui::Separator();

			if (bCopyToClipboard)
			{
				KarmaGui::LogText("\n```\n");
				KarmaGui::LogFinish();
			}
			KarmaGui::EndChildFrame();
		}

		KarmaGui::End();
	}

	// Strings are copied in this not-so-cheap function. Hence the check!!
	// First copying is done within BiosReader for apporpriate seperation into structures. Prevents multiple queries
	// at the cost of bulk (pun intended!).
	// Next copying is done here, in the routine.
	void KarmaGuiMesa::QueryForTuringMachineElectronics()
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

	int KarmaGuiMesa::ImStrlenW(const KGWchar* str)
	{
		//return (int)wcslen((const wchar_t*)str);  // FIXME-OPT: Could use this when wchar_t are 16-bit
		int n = 0;
		while (*str++) n++;
		return n;
	}

	uint32_t KarmaGuiMesa::ChernUint32FromString(const std::string& ramString)
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

	std::string KarmaGuiMesa::ChernDimensionsFromString(const std::string& ramString)
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

	void KarmaGuiMesa::SetElectronicsRamInformationToNull()
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
		for (uint32_t counter = 0; counter < KarmaGuiMesa::GetGatheredElectronicsInformation().ramSoftSlots.size(); counter++)
		{
			ramSizeFound += KarmaGuiMesa::ChernUint32FromString(KarmaGuiMesa::GetGatheredElectronicsInformation().ramInformation[counter].ramSize);
		}

		KarmaGuiMesa::GetGatheredElectronicsInformationForModification().totalRamSize = ramSizeFound;

		// Hoping for GB only dimension
		KarmaGuiMesa::GetGatheredElectronicsInformationForModification().ramSizeDimensions = KarmaGuiMesa::ChernDimensionsFromString(KarmaGuiMesa::GetGatheredElectronicsInformation().ramInformation[0].ramSize);
	}

	void KarmaTuringMachineElectronics::GaugeSystemMemoryDevices(random_access_memory* ramCluster)
	{
		if (ramCluster == nullptr)
		{
			KR_CORE_WARN("Memory devices pointer is null. No Ram(s) shall be detected and reported");
			return;
		}

		KarmaTuringMachineElectronics selfRefrentialVariable = KarmaGuiMesa::GetGatheredElectronicsInformationForModification();

		uint32_t biosReportedNumber = selfRefrentialVariable.numberOfMemoryDevices;

		for (uint32_t counter = 0; counter < biosReportedNumber; counter++)
		{
			random_access_memory* aMemoryBeingScanned = fetch_access_memory_members(counter);

			if (aMemoryBeingScanned != nullptr && IsPhysicalRamPresent(*aMemoryBeingScanned))
			{
				KarmaGuiMesa::GetGatheredElectronicsInformationForModification().ramSoftSlots.push_back(counter);
			}
		}
	}

	void KarmaTuringMachineElectronics::FillTheSystemRamStructure(SystemRAM& destinationStructure, random_access_memory& sourceStructure)
	{
		destinationStructure.assetTag = sourceStructure.assettag != nullptr ? sourceStructure.assettag : KarmaGuiMesa::notAvailableText;
		destinationStructure.bankLocator = sourceStructure.banklocator != nullptr ? sourceStructure.banklocator : KarmaGuiMesa::notAvailableText;
		destinationStructure.configuredMemorySpeed = sourceStructure.configuredmemoryspeed != nullptr ? sourceStructure.configuredmemoryspeed : KarmaGuiMesa::notAvailableText;
		destinationStructure.memorySpeed = sourceStructure.memoryspeed != nullptr ? sourceStructure.memoryspeed : KarmaGuiMesa::notAvailableText;
		destinationStructure.formFactor = sourceStructure.formfactor != nullptr ? sourceStructure.formfactor : KarmaGuiMesa::notAvailableText;
		destinationStructure.locator = sourceStructure.locator != nullptr ? sourceStructure.locator : KarmaGuiMesa::notAvailableText;
		destinationStructure.manufacturer = sourceStructure.manufacturer != nullptr ? sourceStructure.manufacturer : KarmaGuiMesa::notAvailableText;
		destinationStructure.operatingVoltage = sourceStructure.operatingvoltage != nullptr ? sourceStructure.operatingvoltage : KarmaGuiMesa::notAvailableText;
		destinationStructure.partNumber = sourceStructure.partnumber != nullptr ? sourceStructure.partnumber : KarmaGuiMesa::notAvailableText;
		destinationStructure.ramSize = sourceStructure.ramsize != nullptr ? sourceStructure.ramsize : KarmaGuiMesa::notAvailableText;
		destinationStructure.ramType = sourceStructure.ramtype != nullptr ? sourceStructure.ramtype : KarmaGuiMesa::notAvailableText;
		//destinationStructure.rank = sourceStructure.rank; Not a big fan of rank, reminds me of my JEE AIR 4729
		destinationStructure.serialNumber = sourceStructure.serialnumber != nullptr ? sourceStructure.serialnumber : KarmaGuiMesa::notAvailableText;
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

	//-----------------------------------------------------------------------------
	// [SECTION] Omega stuff!
	//-----------------------------------------------------------------------------

	void KarmaGuiMesa::MesaShutDownRoutine()
	{
		if (electronicsItems.bHasQueried)
		{
			reset_electronics_structures();
			KarmaGuiMesa::SetElectronicsRamInformationToNull();
			electronicsItems.ramSoftSlots.clear();
			electronicsItems.bHasQueried = false;
		}
	}

	KarmaTuringMachineElectronics::~KarmaTuringMachineElectronics()
	{
		//ImGuiMesa::SetElectronicsRamInformationToNull();
	}
}
