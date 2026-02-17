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
#include "hwinfo/hwinfo.h"
#include "hwinfo/utils/unit.h"
#include "spdlog/sinks/callback_sink.h"
#include "KarmaGui/KarmaGuizmo.h"
#include "StaticMeshActor.h"

// Experimental
//#include "Karma/Core/UObjectAllocator.h"
#include "Karma/Ganit/KarmaMath.h"

namespace Karma
{
	KarmaTuringMachineElectronics KarmaGuiMesa::electronicsItems;
	std::string KarmaGuiMesa::notAvailableText = "Kasturi Trishna (The MuskThirst)";
	bool KarmaGuiMesa::m_ViewportFocused = false;
	bool KarmaGuiMesa::m_ViewportHovered = false;
	KarmaVector<UObjectsStatistics> KarmaGuiMesa::m_UObjectStatistics;
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
	AStaticMeshActor* KarmaGuiMesa::m_SelectedSMActor = nullptr;
	TransformCache KarmaGuiMesa::m_SelectedSMActorTransformCache;

	WindowManipulationGaugeData KarmaGuiMesa::m_3DExhibitor;
	WindowManipulationGaugeData	KarmaGuiMesa::m_MemoryExhibitor;

	KarmaGuiDockPreviewData::KarmaGuiDockPreviewData() : FutureNode(0)
	{
		IsDropAllowed = IsCenterAvailable = IsSidesAvailable = IsSplitDirExplicit = false;
		SplitNode = NULL; SplitDir = KGGuiDir_None; SplitRatio = 0.f;

		for (int n = 0; n < KG_ARRAYSIZE(DropRectsDraw); n++)
		{
			DropRectsDraw[n] = KGRect(+FLT_MAX, +FLT_MAX, -FLT_MAX, -FLT_MAX);
		}
	}
	
	void KarmaGuiMesa::EditTransform(std::shared_ptr<Scene> scene)
	{
		if(m_SelectedSMActor == nullptr)
		{
			return;
		}
		
		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		FTransform transform = m_SelectedSMActor->GetTransform();
		
		matrixTranslation[0] = m_SelectedSMActorTransformCache.translation[0] = transform.GetTranslation().x;
		matrixTranslation[1] = m_SelectedSMActorTransformCache.translation[1] = transform.GetTranslation().y;
		matrixTranslation[2] = m_SelectedSMActorTransformCache.translation[2] = transform.GetTranslation().z;
		
		matrixRotation[0] = m_SelectedSMActorTransformCache.rotation[0] = transform.GetRotation().m_Roll;
		matrixRotation[1] = m_SelectedSMActorTransformCache.rotation[1] = transform.GetRotation().m_Pitch;
		matrixRotation[2] = m_SelectedSMActorTransformCache.rotation[2] = transform.GetRotation().m_Yaw;
		
		matrixScale[0] = m_SelectedSMActorTransformCache.scale[0] = transform.GetScale3D().x;
		matrixScale[1] = m_SelectedSMActorTransformCache.scale[1] = transform.GetScale3D().y;
		matrixScale[2] = m_SelectedSMActorTransformCache.scale[2] = transform.GetScale3D().z;
		
		KarmaGui::InputFloat3("Tr", matrixTranslation, "%.2f");
		KarmaGui::InputFloat3("Rt", matrixRotation, "%.2f");
		
		KarmaGui::InputFloat3("Sc", matrixScale, "%.2f");

		if (matrixTranslation[0] != m_SelectedSMActorTransformCache.translation[0] || matrixTranslation[1] != m_SelectedSMActorTransformCache.translation[1] || matrixTranslation[2] != m_SelectedSMActorTransformCache.translation[2] ||
			matrixRotation[0] != m_SelectedSMActorTransformCache.rotation[0] || matrixRotation[1] != m_SelectedSMActorTransformCache.rotation[1] || matrixRotation[2] != m_SelectedSMActorTransformCache.rotation[2] ||
			matrixScale[0] != m_SelectedSMActorTransformCache.scale[0] || matrixScale[1] != m_SelectedSMActorTransformCache.scale[1] || matrixScale[2] != m_SelectedSMActorTransformCache.scale[2])
		{
			m_SelectedSMActorTransformCache.translation[0] = matrixTranslation[0];
			m_SelectedSMActorTransformCache.translation[1] = matrixTranslation[1];
			m_SelectedSMActorTransformCache.translation[2] = matrixTranslation[2];
			
			m_SelectedSMActorTransformCache.rotation[0] = matrixRotation[0];
			m_SelectedSMActorTransformCache.rotation[1] = matrixRotation[1];
			m_SelectedSMActorTransformCache.rotation[2] = matrixRotation[2];
			
			m_SelectedSMActorTransformCache.scale[0] = matrixScale[0];
			m_SelectedSMActorTransformCache.scale[1] = matrixScale[1];
			m_SelectedSMActorTransformCache.scale[2] = matrixScale[2];

			m_SelectedSMActorTransformCache.bIsDirty = true;
		}
		
		if (m_SelectedSMActorTransformCache.bIsDirty)
		{
			glm::vec3 translation(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2]);
			transform.SetTranslation(translation);

			glm::vec3 euler(matrixRotation[0], matrixRotation[1], matrixRotation[2]);
			TRotator rotation(euler);

			transform.SetRotation(rotation);

			glm::vec3 scale(matrixScale[0], matrixScale[1], matrixScale[2]);
			transform.SetScale3D(scale);

			m_SelectedSMActor->SetActorTransform(transform);
			m_SelectedSMActorTransformCache.bIsDirty = false;
		}
	}

	void KarmaGuiMesa::RevealMainFrame(KGGuiID mainMesaDockID, std::shared_ptr<Scene> scene, const CallbacksFromEditor& editorCallbacks)
	{
		// The MM (Main Menu) menu bar
		DrawKarmaMainMenuBarMesa();

		// 2. Show a simple sampling and experiment window
		{
			static bool show1 = true;
			static bool show2 = true;
			static float fValue = 0.0f;
			static int counter = 0;

			KarmaGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and appeninto it

			KarmaGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			KarmaGui::Checkbox("Demo Window", &show1);                  // Edit bools storing our window open/close state
			KarmaGui::Checkbox("Another Window", &show2);

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
			
			EditTransform(scene);

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
			DrawKarmaSceneHierarchyPanelMesa(scene);
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
		static KGU32 usageColor = KG_COL32(128, 0, 128, 255);
		static KGU32 partitionColor = KG_COL32(50, 50, 200, 255);
		double occupiedMemoryFraction = 0.0f;

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

		// Some addresses computations for memory block
		std::string memoryBegin;
		std::string memoryCurrent;
		std::string memoryEnd;
		double memoryBeginui;
		double memoryCurrentui;
		double memoryEndui;

		{
			std::ostringstream oss;
			oss << (void*)GUObjectAllocator.GetPermanentObjectPool();

			memoryBegin = oss.str();
			memoryBeginui = HexStringToDecimal(memoryBegin);
		}

		{
			std::ostringstream oss;
			oss << (void*)GUObjectAllocator.GetPermanentObjectPoolEnd();

			memoryEnd = oss.str();
			memoryEndui = HexStringToDecimal(memoryEnd);
		}

		{
			std::ostringstream oss;
			oss << (void*)GUObjectAllocator.GetPermanentObjectPoolTail();

			memoryCurrent = oss.str();
			memoryCurrentui = HexStringToDecimal(memoryCurrent);
		}

		// Compute how much of memory is filled with UObjects
		occupiedMemoryFraction = (float) (memoryCurrentui - memoryBeginui) / (memoryEndui - memoryBeginui);

		KGVec2 fillerTopRightCoordinates = KGVec2(topRightCoordinates.x - (1 - (float)occupiedMemoryFraction) * memoryBlockWidth, topRightCoordinates.y);

		//KarmaGui::SliderFloat("Memory Occupied", &occupiedMemoryPercent, 0.0f, 100.0f);

		// Draw total memory block and occupied memory
		drawList->AddRectFilled(bottomLeftCoordinates, topRightCoordinates, KG_COL32_WHITE);
		drawList->AddRectFilled(bottomLeftCoordinates, fillerTopRightCoordinates, occupiedMemoryColor);

		double uobjectPlacement = 0;
		double placementFraction = 0;
		KGVec2 uobjectTopRightCoordinates;
		bool bHandleDynamicPartitioning = false;

		if(currentWindow->Size.x != m_MemoryExhibitor.widthCache || currentWindow->Size.y != m_MemoryExhibitor.heightCache)
		{
			bHandleDynamicPartitioning = true;

			m_MemoryExhibitor.widthCache = currentWindow->Size.x;
			m_MemoryExhibitor.heightCache = currentWindow->Size.y;
		}
		else if (currentWindow->Pos.x != m_MemoryExhibitor.startXCache || currentWindow->Pos.y != m_MemoryExhibitor.startYCache)
		{
			bHandleDynamicPartitioning = true;

			m_MemoryExhibitor.startYCache = currentWindow->Pos.y;
			m_MemoryExhibitor.startXCache = currentWindow->Pos.x;
		}
		else if(KarmaGui::GetScrollX() != m_MemoryExhibitor.scrollX || KarmaGui::GetScrollY() != m_MemoryExhibitor.scrollY)
		{
			bHandleDynamicPartitioning = true;

			m_MemoryExhibitor.scrollX = KarmaGui::GetScrollX();
			m_MemoryExhibitor.scrollY = KarmaGui::GetScrollY();
		}
		else
		{
			bHandleDynamicPartitioning = false;
		}

		uint32_t index = 0;
		int32_t hoverIndex = -1;// default

		// Draw partitions and see which column mouse is hovering upon, along sides
		// may need mild modifications upon removal of UObjects
		for(auto& element : m_UObjectStatistics)
		{
			if(element.placementCoordi.x == 0 || bHandleDynamicPartitioning)
			{
				uobjectPlacement = (double)std::stoll(element.beginAddress, 0 , 16);
				placementFraction = (float) (memoryEndui - uobjectPlacement) / (memoryEndui - memoryBeginui);

				uobjectTopRightCoordinates = KGVec2((float)(topRightCoordinates.x - (placementFraction) * memoryBlockWidth), float(topRightCoordinates.y));
				drawList->AddRectFilled(uobjectTopRightCoordinates + KGVec2(-1, memoryBlockHeight), uobjectTopRightCoordinates, partitionColor);

				element.placementCoordi = uobjectTopRightCoordinates;

				double offset = (double)element.size / (double)GUObjectAllocator.GetPermanentPoolSize() * (topRightCoordinates.x - bottomLeftCoordinates.x);

				if(KarmaGui::IsMouseHoveringRect(element.placementCoordi, element.placementCoordi + KGVec2((float)offset, memoryBlockHeight)))
				{
					hoverIndex = index;
				}
			}
			else
			{
				KGVec2 coordinates = element.placementCoordi;
				drawList->AddRectFilled(coordinates + KGVec2(-1, memoryBlockHeight), coordinates, partitionColor);

				double offset = (double)element.size / (double)GUObjectAllocator.GetPermanentPoolSize() * (topRightCoordinates.x - bottomLeftCoordinates.x);

				if(KarmaGui::IsMouseHoveringRect(element.placementCoordi, element.placementCoordi + KGVec2((float)offset, memoryBlockHeight)))
				{
					hoverIndex = index;
				}
			}
			index++;
		}

		// well done ocornut for nutting up the rectangle coordinates convention
		bool bIsHoveringFilledSlot = KarmaGui::IsMouseHoveringRect(bottomLeftCoordinates - KGVec2(0, memoryBlockHeight), fillerTopRightCoordinates + KGVec2(0, memoryBlockHeight));

		if(bIsHoveringFilledSlot && !currentWindow->Hidden)
		{
			KarmaGuiInternal::BeginTooltipEx(KGGuiTooltipFlags_OverridePreviousTooltip, KGGuiWindowFlags_None);
			//KarmaGui::Text("Current Memory: 0x34245421AC");
			KarmaGui::Text("UObject Details");
			KarmaGui::Text("Number: %d", hoverIndex);
			KarmaGui::Text("Name: %s", m_UObjectStatistics.IndexToObject(hoverIndex).uobjectName.c_str());
			KarmaGui::Text("Begin Address: %s", m_UObjectStatistics.IndexToObject(hoverIndex).beginAddress.c_str());
			KarmaGui::Text("End Address: %s", m_UObjectStatistics.IndexToObject(hoverIndex).endAddress.c_str());
			KarmaGui::Text("Size : %zu (bytes)", m_UObjectStatistics.IndexToObject(hoverIndex).size);
			KarmaGui::Text("Size in pool : %zu (bytes)", m_UObjectStatistics.IndexToObject(hoverIndex).sizeInPool);
			KarmaGui::Text("Alignment: %u", m_UObjectStatistics.IndexToObject(hoverIndex).alignment);
			if(m_UObjectStatistics.IndexToObject(hoverIndex).classObject != nullptr && m_UObjectStatistics.IndexToObject(hoverIndex).classObject->GetName() != "")
			{
				KarmaGui::Text("Class Name: %s", m_UObjectStatistics.IndexToObject(hoverIndex).classObject->GetName().c_str());
			}
			KarmaGui::EndTooltip();
		}

		//float someCol = 11316;
		//KarmaGui::ColorEdit4("acolor", &someCol);

		std::string addressText;
		static KGVec2 addressTextSize;

		KGVec2 pointerRectangleCoordinatesMin, pointerRectangleCoordinatesMax;
		KGVec2 cursorPosition;

		// Draw appropriate lables for display of addresses explicitly

		// 1. Draw arrow alpha, starting of reserved memory block
		{
			addressText = memoryBegin;
			addressTextSize = KarmaGui::CalcTextSize(addressText.c_str());

			KarmaGuiInternal::RenderArrowPointingAt(drawList, bottomLeftCoordinates, KGVec2(5, 16), KGGuiDir_Up, arrowColor);
			pointerRectangleCoordinatesMin = KGVec2(bottomLeftCoordinates.x - 2.5f, bottomLeftCoordinates.y + 16 + addressTextSize.y);
			pointerRectangleCoordinatesMax = KGVec2(bottomLeftCoordinates.x - 2.5f + addressTextSize.x, bottomLeftCoordinates.y + 16);
			drawList->AddRect(pointerRectangleCoordinatesMin, pointerRectangleCoordinatesMax, KG_COL32_BLACK);
			cursorPosition = KGVec2(pointerRectangleCoordinatesMin.x - bareToFrameX, pointerRectangleCoordinatesMin.y - bareToFrameY - addressTextSize.y);
			KarmaGui::SetCursorPos(cursorPosition);
			KarmaGui::TextColored(legendTextColor, "%s", addressText.c_str());
		}

		// 2. Draw the current available memory pointer arrow
		{
			addressText = memoryCurrent;
			addressTextSize = KarmaGui::CalcTextSize(addressText.c_str());

			KarmaGuiInternal::RenderArrowPointingAt(drawList, fillerTopRightCoordinates, KGVec2(5, 16), KGGuiDir_Down, arrowColor);
			pointerRectangleCoordinatesMin = KGVec2(fillerTopRightCoordinates.x - addressTextSize.x / 2, fillerTopRightCoordinates.y - 16);
			pointerRectangleCoordinatesMax = KGVec2(fillerTopRightCoordinates.x + addressTextSize.x / 2, fillerTopRightCoordinates.y - 16 - addressTextSize.y);
			drawList->AddRect(pointerRectangleCoordinatesMin, pointerRectangleCoordinatesMax, KG_COL32_BLACK);
			cursorPosition = KGVec2(pointerRectangleCoordinatesMin.x - bareToFrameX, pointerRectangleCoordinatesMin.y - bareToFrameY - addressTextSize.y);
			KarmaGui::SetCursorPos(cursorPosition);
			KarmaGui::TextColored(legendTextColor, "%s", addressText.c_str());
		}

		// 3. Draw arrow omega, the pointer at the end of memory block
		{
			addressText = memoryEnd;
			addressTextSize = KarmaGui::CalcTextSize(addressText.c_str());

			KarmaGuiInternal::RenderArrowPointingAt(drawList, KGVec2(topRightCoordinates.x, topRightCoordinates.y + memoryBlockHeight), KGVec2(5, 16), KGGuiDir_Up, arrowColor);
			pointerRectangleCoordinatesMin = KGVec2(topRightCoordinates.x - addressTextSize.x / 2, topRightCoordinates.y + memoryBlockHeight + addressTextSize.y + 16);
			pointerRectangleCoordinatesMax = KGVec2(topRightCoordinates.x + addressTextSize.x / 2, topRightCoordinates.y + 16 + memoryBlockHeight);
			drawList->AddRect(pointerRectangleCoordinatesMin, pointerRectangleCoordinatesMax, KG_COL32_BLACK);
			cursorPosition = KGVec2(pointerRectangleCoordinatesMin.x - bareToFrameX, pointerRectangleCoordinatesMin.y - bareToFrameY - addressTextSize.y);
			KarmaGui::SetCursorPos(cursorPosition);
			KarmaGui::TextColored(legendTextColor, "%s", addressText.c_str());
		}

		// 4. Render statistics for UObjects
		{
			KarmaGui::SetCursorPos(KGVec2(45, bareYBL + 50));
			KarmaGui::Text("UObjects Statistics:");
			KarmaGui::Text("Bare UObjects: %u bytes", GUObjectAllocator.GetBareUObjectSize());
			KarmaGui::Text("Aligned UObjects: %u bytes", GUObjectAllocator.GetAlignedUObjectSize());
			KarmaGui::Text("Total reservation: %u bytes", GUObjectAllocator.GetPermanentPoolSize());
			KarmaGui::Text("Usable memory: %u bytes", GUObjectAllocator.GetPermanentPoolSize() - GUObjectAllocator.GetAlignedUObjectSize());
			KarmaGui::Text("Number of UObjects: %u", GUObjectAllocator.GetNumberOfUObjects());
		}

		static KGVec2 textSize = KarmaGui::CalcTextSize("Memory Quota for UObjects");

		KarmaGui::SetCursorPos(KGVec2((bareXBL + memoryBlockWidth) / 2 - textSize.x / 2, bareYBL));// local coordinates, scrolling included
		KarmaGui::TextColored(legendTextColor, "Memory Quota for UObjects");

		static KGFont* verticalTextFont = KarmaGui::GetFont();
		verticalTextFont->Scale = 0.6f;

		KarmaGui::PushFont(verticalTextFont);
		std::string usageText = "Memory Usage: " + std::to_string(int(occupiedMemoryFraction * 100.f)) + "%";
		static KGVec2 textSize2 = KarmaGui::CalcTextSize(usageText.c_str());
		KarmaGui::AddTextVertical(drawList, usageText.c_str(), KGVec2(fillerTopRightCoordinates.x, bottomLeftCoordinates.y - memoryBlockHeight / 2 + textSize2.x / 2), usageColor);
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

		static bool bShouldRefresh = false;

		// refresh once when clicked and rendered
		if(!window->Hidden && bShouldRefresh)
		{
			m_RefreshRenderingResources = true;
			bShouldRefresh = false;
		}
		else
		{
			bShouldRefresh = true;
		}

		if (!KarmaGuizmo::IsUsing())
		{
			m_ViewportFocused = KarmaGui::IsWindowFocused();
			m_ViewportHovered = KarmaGui::IsWindowHovered() && !((window->Pos.y + window->TitleBarHeight()) * KarmaGui::GetIO().DisplayFramebufferScale.y > KarmaGui::GetMousePos().y);
		}
		else
		{
			m_ViewportFocused = false;
			m_ViewportHovered = false;
		}

		KarmaGuiIO& io = KarmaGui::GetIO();

		KGTextureID backgroundImageTextureID = 0;

		uint32_t width = 0;
		uint32_t height = 0;

		KarmaGuiBackendRendererUserData* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		backgroundImageTextureID = backendData->GetTextureIDAtIndex(1);
		
		KGTextureID textureID3D = nullptr;
		if (scene->GetSMActors().size() > 0)
		{
			textureID3D = KarmaGuiRenderer::Add3DSceneFor2DRendering(scene, KGVec2(window->Size.x, window->Size.y));
			KarmaGui::SetItemAllowOverlap();
		}

		KGVec2 pos = KarmaGui::GetWindowPos();
		KGDrawList* drawList = KarmaGui::GetWindowDrawList();
		
		drawList->AddImage((void*)backgroundImageTextureID, pos, KGVec2(pos.x + window->Size.x, pos.y + window->Size.y));
		if (scene->GetSMActors().size() > 0 && textureID3D != nullptr)
		{
			drawList->AddImage((void*)textureID3D, pos, KGVec2(pos.x + window->Size.x, pos.y + window->Size.y));
		}
		
		KarmaGuizmo::SetDrawlist();
		KarmaGuizmo::SetRect(pos.x, pos.y + window->TitleBarHeight(), window->Size.x, window->Size.y - window->TitleBarHeight());
		
        if(scene->GetSMActors().size() > 0 && m_SelectedSMActor != nullptr)
		{
			std::shared_ptr<Camera> sceneCamera =  scene->GetSceneCamera();
			glm::mat4 viewMatrix = sceneCamera->GetViewMatirx();
			
			glm::mat4 projectionMatrix = sceneCamera->GetProjectionMatrix();
			
			float* projectionPtr = glm::value_ptr(projectionMatrix);
			projectionPtr[5] *= -1.f;
			
			FTransform operationalTransform = m_SelectedSMActor->GetTransform();

			glm::mat4 objectMatrix = operationalTransform.ToMatrixWithScale();
            bool bManipulate = KarmaGuizmo::Manipulate(glm::value_ptr(viewMatrix), projectionPtr, KarmaGuizmo::UNIVERSAL, KarmaGuizmo::LOCAL, glm::value_ptr(objectMatrix));
			
			if(KarmaGuizmo::IsUsing() && bManipulate)
			{
				FTransform transform;
				transform.ToTransform(objectMatrix);
				
				m_SelectedSMActor->SetActorTransform(transform);
			}
        }
		
		scene->SetRenderWindow(window);

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

		if (m_RefreshRenderingResources)
		{
			scene->SetWindowToRenderWithinResize(true);
			m_RefreshRenderingResources = false;
		}

		KarmaGui::PopStyleColor();
		KarmaGui::End();
	}

	void KarmaGuiMesa::DrawKarmaSceneHierarchyPanelMesa(std::shared_ptr<Scene> scene)
	{
		KarmaGui::SetNextWindowSize(KGVec2(500, 400), KGGuiCond_FirstUseEver);

		KarmaGui::Begin("Scene Hierarchy");
		
		for(const auto& smActor : scene->GetSMActors())
		{
			const char* sceneElement = smActor->GetName().c_str();
			
			KGGuiTreeNodeFlags_ flags = (smActor == m_SelectedSMActor) ? KGGuiTreeNodeFlags_Selected : KGGuiTreeNodeFlags_Bullet;
			bool opened = KarmaGui::TreeNodeEx(sceneElement, flags);
			
			if(KarmaGui::IsItemClicked())
			{
				m_SelectedSMActor = smActor;
			}
			
			if(opened)
			{
				KarmaGui::TreePop();
			}
		}
		
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
				if (KarmaGui::MenuItem("About", nullptr, &showKarmaAbout)) {}
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
				KarmaGuiMesa::SetElectronicsRamInformationToNull();
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
			KGVec2 uvMin = KGVec2(0.0f, 0.0f);                 // Top-left
			KGVec2 uvMax = KGVec2(1.0f, 1.0f);                 // Lower-right
			KGVec4 tint_col = KGVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
			KGVec4 border_col = KGVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
			KarmaGui::Image(aboutImageTextureID, KGVec2((float)width, (float)height), uvMin, uvMax, tint_col, border_col);
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

			/*bool copy_to_clipboard =*/ KarmaGui::Button("Copy to clipboard");
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
			KarmaGui::Text("MotherBoard: %s", electronicsItems.biosBoardName.c_str());
			KarmaGui::Text("MotherBoard Serial: %s", electronicsItems.biosSerialNumber.c_str());
			KarmaGui::Text("BIOS Version: %s", electronicsItems.biosVersion.c_str());
			KarmaGui::Separator();

			KarmaGui::Text("Machine System Memory (RAM)");
			KarmaGui::Separator();

			KarmaGui::Text("Total Capacity (MiB): %s", electronicsItems.memoryCapacity.c_str());
			KarmaGui::Text("Free Memory (MiB): %s", electronicsItems.freeMemory.c_str());

			KarmaGui::Text("Total modules: %d", electronicsItems.numberOfMemoryDevices);
			
			for(uint32_t counter = 0; counter < electronicsItems.numberOfMemoryDevices; counter++)
			{
				KarmaGui::Text("RAM %d", counter + 1);
				KarmaGui::Indent();
				KarmaGui::Text("Vendor: %s", electronicsItems.ramInformation[counter].vendor.c_str());
				KarmaGui::Text("Name: %s", electronicsItems.ramInformation[counter].name.c_str());
				KarmaGui::Text("Model: %s", electronicsItems.ramInformation[counter].model.c_str());
				KarmaGui::Text("Serial Number: %s", electronicsItems.ramInformation[counter].serialNumber.c_str());
				KarmaGui::Text("Frequency (MHz): %s", electronicsItems.ramInformation[counter].frequency.c_str());
				KarmaGui::Text("Capacity: %s", electronicsItems.ramInformation[counter].capacity.c_str());
				KarmaGui::Unindent();
			}
			
			KarmaGui::Separator();

			KarmaGui::Text("Central Processor Unit");
			KarmaGui::Separator();

			KarmaGui::Text("Manufacturer: %s", electronicsItems.cpuInformation.cpuVendor.c_str());
			KarmaGui::Text("Processor Family: %s", electronicsItems.cpuInformation.cpuModel.c_str());
			KarmaGui::Text("CPU Conditions");
			KarmaGui::Indent();
			KarmaGui::Text("Speed : %s (MHz) | %s (MHz)", electronicsItems.cpuInformation.cpuCurrentFrequency.c_str(), electronicsItems.cpuInformation.cpuMaximumFrequency.c_str());
			KarmaGui::Text("Cores (Logical | Physical): %s | %s", electronicsItems.cpuInformation.cpuLogicalCores.c_str(), electronicsItems.cpuInformation.cpuPhysicalCores.c_str());
			KarmaGui::Unindent();
			KarmaGui::Text("CPU Tags or Numbers");
			KarmaGui::Indent();
			KarmaGui::Text("Caches (MegaBytes)");
			KarmaGui::Text("L1 Cache : %s", electronicsItems.cpuInformation.cpuCacheSizeL1.c_str());
			KarmaGui::Text("L2 Cache: %s", electronicsItems.cpuInformation.cpuCacheSizeL2.c_str());
			KarmaGui::Text("L3 Cache: %s", electronicsItems.cpuInformation.cpuCacheSizeL3.c_str());
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
	void KarmaGuiMesa::QueryForTuringMachineElectronics()
	{
		if (electronicsItems.bHasQueried)
		{
			return;
		}
		
		hwinfo::MainBoard mainboard;
		
		electronicsItems.biosVendorName = mainboard.vendor() != "" ? mainboard.vendor() : notAvailableText;
		electronicsItems.biosBoardName = mainboard.name() != "" ? mainboard.name() : notAvailableText;
		electronicsItems.biosVersion = mainboard.version() != "" ? mainboard.version() : notAvailableText;
		electronicsItems.biosSerialNumber = mainboard.serialNumber() != "" ? mainboard.serialNumber() : notAvailableText;
		
		hwinfo::Memory memory;
		
		electronicsItems.memoryCapacity = std::to_string(hwinfo::unit::bytes_to_MiB(memory.total_Bytes()));
		electronicsItems.freeMemory = std::to_string(hwinfo::unit::bytes_to_MiB(memory.free_Bytes() > 0 ? memory.free_Bytes() : 0));
		
		electronicsItems.numberOfMemoryDevices = static_cast<uint32_t>(memory.modules().size());
		
		for(const auto& module : memory.modules())
		{
			KR_INFO("ID: {0}", module.id);
			KR_INFO("Frequencey (Hz): {0}", module.frequency_Hz == -1 ? -1 : static_cast<double>(module.frequency_Hz) / 1e6);
			KR_INFO("Name: {0}", module.name);
			KR_INFO("Serial Number: {0}", module.serial_number);
			KR_INFO("Vendor: {0}", module.vendor);
			KR_INFO("Model: {0}", module.model);
			KR_INFO("Capacity: {0}", hwinfo::unit::bytes_to_MiB(module.total_Bytes));
		}
		
		electronicsItems.ramInformation = new KarmaTuringMachineElectronics::SystemRAM[memory.modules().size()];
		uint32_t counter = 0;
		
		for(const auto& mod : memory.modules())
		{
			electronicsItems.ramInformation[counter].name = mod.name;
			electronicsItems.ramInformation[counter].vendor = mod.vendor;
			electronicsItems.ramInformation[counter].model = mod.model;
			electronicsItems.ramInformation[counter].serialNumber = mod.serial_number;
			electronicsItems.ramInformation[counter].frequency = std::to_string(mod.frequency_Hz == -1 ? -1 : static_cast<double>(mod.frequency_Hz) / 1e6);
			electronicsItems.ramInformation[counter].id = mod.id;
			electronicsItems.ramInformation[counter].capacity = std::to_string(hwinfo::unit::bytes_to_MiB(mod.total_Bytes));
			counter++;
		}

		const auto cpus = hwinfo::getAllCPUs();

		for(const auto& cpu : cpus)
		{
			electronicsItems.cpuInformation.cpuVendor = cpu.vendor();
			electronicsItems.cpuInformation.cpuModel = cpu.modelName();
			electronicsItems.cpuInformation.cpuLogicalCores = std::to_string(cpu.numLogicalCores());
			electronicsItems.cpuInformation.cpuPhysicalCores = std::to_string(cpu.numPhysicalCores());
			
			int64_t averageFrequency = 0;
			for(uint32_t counter = 0; counter < cpu.numLogicalCores(); counter++)
			{
				averageFrequency += cpu.currentClockSpeed_MHz()[counter];
			}
			
			averageFrequency = averageFrequency / cpu.numLogicalCores();
			
			electronicsItems.cpuInformation.cpuCurrentFrequency = std::to_string(averageFrequency);
			electronicsItems.cpuInformation.cpuMaximumFrequency = std::to_string(cpu.maxClockSpeed_MHz());
			KR_INFO("CPU frequency: {0}", cpu.maxClockSpeed_MHz());
			electronicsItems.cpuInformation.cpuCacheSizeL1 = std::to_string(hwinfo::unit::bytes_to_MiB(cpu.L1CacheSize_Bytes() > 0 ? cpu.L1CacheSize_Bytes() : 0));
			electronicsItems.cpuInformation.cpuCacheSizeL2 = std::to_string(hwinfo::unit::bytes_to_MiB(cpu.L2CacheSize_Bytes() > 0 ? cpu.L2CacheSize_Bytes() : 0));
			electronicsItems.cpuInformation.cpuCacheSizeL3 = std::to_string(hwinfo::unit::bytes_to_MiB(cpu.L3CacheSize_Bytes() > 0 ? cpu.L3CacheSize_Bytes() : 0));

			break;
		}

		auto gpus = hwinfo::getAllGPUs();
		
		if(gpus.size() == 0)
		{
			electronicsItems.gpuModelIdentification = notAvailableText;
			electronicsItems.gpuVendor = notAvailableText;
			
			electronicsItems.gpuVMemory = notAvailableText;
		}

		for(const auto& gpu : gpus)
		{
			electronicsItems.gpuModelIdentification = gpu.name() != "" ? gpu.name() : notAvailableText;
			electronicsItems.gpuVendor = gpu.vendor() != "" ? gpu.vendor() : notAvailableText;
			
			electronicsItems.gpuVMemory = gpu.memory_Bytes() != 0 ? std::to_string(hwinfo::unit::bytes_to_MiB(gpu.memory_Bytes())) : notAvailableText;
			break;
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

	uint32_t KarmaGuiMesa::ChurnUint32FromString(const std::string& ramString)
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

	std::string KarmaGuiMesa::ChurnDimensionsFromString(const std::string& ramString)
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

	double KarmaGuiMesa::HexStringToDecimal(const std::string& hexString)
	{
		return (double)std::stoll(hexString, 0, 16);
	}

	void KarmaGuiMesa::DumpUObjectStatistics(void* InObject, const std::string& InName, size_t InSize, size_t InAlignment, UClass* InClass)
	{
		std::ostringstream oss;
		oss << InObject;

		std::string pointerAddress = oss.str();
		//KR_INFO("[UObjectDump] {0}, {1}, {2}, {3}", pointerAddress, InName, InSize, FMath::Max<size_t>(16, InAlignment));

		UObjectsStatistics anElement;
		anElement.uobjectName = InName;
		anElement.objectPointer = InObject;
		anElement.beginAddress = pointerAddress;
		anElement.size = InSize;
		anElement.alignment = (uint32_t)FMath::Max<size_t>(16, InAlignment);

		std::ostringstream osb;
		osb << (void*)((uint8_t*)InObject + InSize);

		anElement.endAddress = osb.str();

		uint32_t vectorLength = m_UObjectStatistics.Num();

		if(vectorLength > 0)
		{
			long sizeInPool = long((uint8_t*)InObject - (uint8_t*)m_UObjectStatistics.GetElements()[vectorLength - 1].objectPointer);
			m_UObjectStatistics.ModifyElements()[vectorLength - 1].sizeInPool = sizeInPool;
		}

		long sizeInPool = long(Align((uint8_t*)GUObjectAllocator.GetPermanentObjectPoolTail(), FMath::Max<size_t>(16, InAlignment)) - (uint8_t*)InObject);
		anElement.sizeInPool = sizeInPool;
		anElement.classObject = InClass;

		m_UObjectStatistics.Add(anElement);
	}

	//-----------------------------------------------------------------------------
	// [SECTION] Omega stuff!
	//-----------------------------------------------------------------------------

	void KarmaGuiMesa::MesaShutDownRoutine()
	{
		if (electronicsItems.bHasQueried)
		{
			KarmaGuiMesa::SetElectronicsRamInformationToNull();
			electronicsItems.bHasQueried = false;
		}
	}

	KarmaTuringMachineElectronics::~KarmaTuringMachineElectronics()
	{
		//ImGuiMesa::SetElectronicsRamInformationToNull();
	}
}
