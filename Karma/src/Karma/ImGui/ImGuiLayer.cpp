#include "ImGuiLayer.h"
#include "glad/glad.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "Karma/Application.h"
#include "backends/imgui_impl_glfw.h"
#include "Vulkan/VulkanHolder.h"
#include "Renderer/RendererAPI.h"
#include "Renderer/RenderCommand.h"
#include "glm/glm.hpp"
#include "ImGui/ImGuiMesa.h"
#include "Renderer/Renderer.h"

// Emedded font
#include "Karma/ImGui/Roboto-Regular.h"

namespace Karma
{
	std::shared_ptr<Karma::Scene> ImGuiLayer::m_Scene = nullptr;

	ImGuiLayer::ImGuiLayer(Window* relevantWindow)
		: Layer("ImGuiLayer"), m_AssociatedWindow(relevantWindow)
	{
		if (RendererAPI::GetAPI() == RendererAPI::API::Vulkan)
		{
			// Curate the data associated with Window context and VulkanAPI
			int width, height;
			GLFWwindow* window = static_cast<GLFWwindow*>(m_AssociatedWindow->GetNativeWindow());

			glfwGetFramebufferSize(window, &width, &height);
		}
	}

	void ImGuiLayer::CreateDescriptorPool()
	{
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 1000;
		poolInfo.poolSizeCount = uint32_t(std::size(pool_sizes));
		poolInfo.pPoolSizes = pool_sizes;

		VkResult result = vkCreateDescriptorPool(VulkanHolder::GetVulkanContext()->GetLogicalDevice(), &poolInfo, nullptr, &m_ImGuiDescriptorPool);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create descriptor pool for ImGui");
	}
	ImGuiLayer::~ImGuiLayer()
	{
		ImGuiMesa::MesaShutDownRoutine();
	}

	void ImGuiLayer::OnAttach()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		(void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	// Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;		// Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;		// Enable Multi-Viewport / Platform Windows

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setting Dear ImGui ini file
		io.IniFilename = "../Resources/Misc/DearImGuiEditor.ini";//"yeehaw!";

		GLFWwindow* window = static_cast<GLFWwindow*>(m_AssociatedWindow->GetNativeWindow());

		// Setup Platform/Renderer bindings
		if (RendererAPI::GetAPI() == RendererAPI::API::Vulkan)
		{
			// Exposing Karma's Vulkan components to Dear ImGui
			ImGui_ImplGlfw_InitForVulkan(window, true);

			ImGui_KarmaImplVulkan_InitInfo initInfo = {};
			// An inter-class communication
			initInfo.Instance = VulkanHolder::GetVulkanContext()->GetInstance();
			initInfo.PhysicalDevice = VulkanHolder::GetVulkanContext()->GetPhysicalDevice();
			initInfo.Device = VulkanHolder::GetVulkanContext()->GetLogicalDevice();
			initInfo.QueueFamily = VulkanHolder::GetVulkanContext()->FindQueueFamilies(initInfo.PhysicalDevice).graphicsFamily.value();
			initInfo.Queue = VulkanHolder::GetVulkanContext()->GetGraphicsQueue();
			initInfo.MinImageCount = VulkanHolder::GetVulkanContext()->GetMinImageCount();
			initInfo.ImageCount = VulkanHolder::GetVulkanContext()->GetImageCount();
			initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

			// Stuff created and dedicated to Dear ImGUI
			CreateDescriptorPool();
			initInfo.DescriptorPool = m_ImGuiDescriptorPool;
			initInfo.RenderPass = VulkanHolder::GetVulkanContext()->GetRenderPass();

			ImGuiVulkanHandler::ImGui_KarmaImplVulkan_Init(&initInfo);

			// Fresh start with newly instantiated Vulkan data
			// Since VulkanContext has already instantiated fresh swapchain and commandbuffers, we send that false
			ImGuiVulkanHandler::ShareVulkanContextResourcesOfMainWindow(&m_VulkanWindowData, true);

			// Load default font
			ImFontConfig fontConfig;
			fontConfig.FontDataOwnedByAtlas = false;
			ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular, sizeof(g_RobotoRegular), 20.0f, &fontConfig);
			io.FontDefault = robotoFont;

			// Upload Fonts
			{
				// Use any command queue
				VkCommandPool commandPool = m_VulkanWindowData.CommandPool;
				VkCommandBuffer commandBuffer = m_VulkanWindowData.FramesOnFlight[m_VulkanWindowData.SemaphoreIndex].CommandBuffer;

				// Hehe, out-of-trendy way!!!
				VkResult result = vkResetCommandPool(initInfo.Device, commandPool, 0);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to reset command pool!");

				VkCommandBufferBeginInfo beginInfo = {};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to begin recording(?) command buffer!");

				// Load Fonts
				ImGuiVulkanHandler::ImGui_KarmaImplVulkan_CreateFontsTexture(commandBuffer);

				// Load Images

				//	1. The wall
				ImGuiVulkanHandler::ImGui_KarmaImplVulkan_CreateTexture(commandBuffer, "../Resources/Textures/The_Source_Wall.jpg");
				ImGuiVulkanHandler::ImGui_KarmaImplVulkan_CreateTexture(commandBuffer, "../Resources/Textures/Measures.png");

				VkSubmitInfo endInfo = {};
				endInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				endInfo.commandBufferCount = 1;
				endInfo.pCommandBuffers = &commandBuffer;
				result = vkEndCommandBuffer(commandBuffer);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to end recording(?) command buffer!");

				result = vkQueueSubmit(initInfo.Queue, 1, &endInfo, VK_NULL_HANDLE);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to submit command buffer!");

				result = vkDeviceWaitIdle(initInfo.Device);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to wait!");

				ImGuiVulkanHandler::ImGui_KarmaImplVulkan_DestroyFontUploadObjects();
			}
		}
		else if (RendererAPI::GetAPI() == RendererAPI::API::OpenGL)
		{
			ImGui_ImplGlfw_InitForOpenGL(window, true);
			ImGuiOpenGLHandler::ImGui_ImplOpenGL3_Init("#version 410");

			// Load default font
			ImFontConfig fontConfig;
			fontConfig.FontDataOwnedByAtlas = false;
			ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular, sizeof(g_RobotoRegular), 20.0f, &fontConfig);
			io.FontDefault = robotoFont;

			// Load images
			unsigned int aboutTexture;
			glGenTextures(1, &aboutTexture);
			glBindTexture(GL_TEXTURE_2D, aboutTexture);
			// set the texture wrapping parameters
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			// set texture filtering parameters
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// get filename and create opengl texture
			int width, height, nrChannels;
			// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
			unsigned char* data = stbi_load("../Resources/Textures/The_Source_Wall.jpg", &width, &height, &nrChannels, 0);
			if (data)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			}
			else
			{
				KR_CORE_ASSERT(data, "Failed to load textures image!");
			}
			stbi_image_free(data);

			ImGui_ImplOpenGL3_Data* bd = ImGuiOpenGLHandler::ImGui_ImplOpenGL3_GetBackendData();

			MesaDecalData mDData;
			mDData.height = height;
			mDData.width = width;
			mDData.DecalRef = aboutTexture;

			bd->mesaDecalDataList.push_back(mDData);
		}
	}

	void ImGuiLayer::OnDetach()
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan:
			GracefulVulkanShutDown();
			break;
		case RendererAPI::API::OpenGL:
			ImGuiOpenGLHandler::ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
			break;
		case RendererAPI::API::None:
			KR_CORE_ASSERT(false, "RendererAPI::None is not supported");
			break;
		default:
			KR_CORE_ASSERT(false, "Unknown RendererAPI {0} is in play.")
				break;
		}

		KR_CORE_INFO("Shutting down ImGuiLayer");
	}

	void ImGuiLayer::GiveLoopBeginControlToVulkan()
	{
		// Resize swap chain?
		if (m_SwapChainRebuild)
		{
			int width, height;
			GLFWwindow* window = static_cast<GLFWwindow*>(m_AssociatedWindow->GetNativeWindow());
			glfwGetFramebufferSize(window, &width, &height);

			if (width > 0 && height > 0)
			{
				RendererAPI* rAPI = RenderCommand::GetRendererAPI();
				VulkanRendererAPI* vulkanAPI = nullptr;

				if (rAPI->GetAPI() == RendererAPI::API::Vulkan)
				{
					vulkanAPI = static_cast<VulkanRendererAPI*>(rAPI);
				}
				else
				{
					KR_CORE_ASSERT(false, "How is this even possible?");
				}

				KR_CORE_ASSERT(vulkanAPI != nullptr, "Casting to VulkanAPI failed");

				vulkanAPI->RecreateCommandBuffersAndSwapChain();

				ImGuiVulkanHandler::ShareVulkanContextResourcesOfMainWindow(&m_VulkanWindowData, false);
				m_SwapChainRebuild = false;
			}
		}

		ImGuiVulkanHandler::ImGui_KarmaImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
	}

	// The ImGuiLayer sequence begins
	void ImGuiLayer::Begin()
	{
		m_Scene = Renderer::GetScene();

		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan:
			GiveLoopBeginControlToVulkan();
			break;
		case RendererAPI::API::OpenGL:
			ImGuiOpenGLHandler::ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			break;
		case RendererAPI::API::None:
			KR_CORE_ASSERT(false, "RendererAPI::None is not supported");
			break;
		default:
			KR_CORE_ASSERT(false, "Unknown RendererAPI {0} is in play.")
				break;
		}
		ImGui::NewFrame();
	}

	void ImGuiLayer::ImGuiRender(float deltaTime)
	{
		ImGuiID dockspaceID;

		// 1. Show the big demo window. For debug purpose!!
		static bool show = true;
		ImGui::ShowDemoWindow(&show);

		// 2. A UI canvas, if I may, for the main window!!
		{
			static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
			ImGuiWindowFlags windowFlags = ImGuiDockNodeFlags_None;

			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);

			// No clue
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

			windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

			windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
			// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
			// all active windows docked into it will lose their parent and become undocked.
			// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
			// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
			// hmm
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			ImGui::Begin("KarmaSafetyDockSpace", nullptr, windowFlags);
			ImGui::PopStyleVar();
			ImGui::PopStyleVar(2);

			dockspaceID = ImGui::GetID("KarmaSafetyDockSpace");
			ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);

			ImGui::End();
		}

		// The complete UI Karma shall (ever?) need. Not counting meta morpho analytic and service toolset
		{
			ImGuiMesa::RevealMainFrame(dockspaceID, m_Scene);
		}
	}

	void ImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
		GLFWwindow* window = static_cast<GLFWwindow*>(m_AssociatedWindow->GetNativeWindow());

		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan:
			GiveLoopEndControlToVulkan();
			break;
		case RendererAPI::API::OpenGL:
		{
			int displayWidth, displayHeight;
			glfwGetFramebufferSize(window, &displayWidth, &displayHeight);
			glViewport(0, 0, displayWidth, displayHeight);
			glm::vec4 clearColor = RenderCommand::GetClearColor();
			glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGuiOpenGLHandler::ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				GLFWwindow* backup_current_context = glfwGetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				glfwMakeContextCurrent(backup_current_context);
			}
		}
		break;
		case RendererAPI::API::None:
			KR_CORE_ASSERT(false, "RendererAPI::None is not supported");
			break;
		default:
			KR_CORE_ASSERT(false, "Unknown RendererAPI {0} is in play.");
			break;
		}
	}
	// The ImGuiLayer sequence ends

	void ImGuiLayer::GiveLoopEndControlToVulkan()
	{
		// Rendering
		ImGui::Render();
		ImDrawData* mainDrawData = ImGui::GetDrawData();
		const bool mainIsMinimized = (mainDrawData->DisplaySize.x <= 0.0f || mainDrawData->DisplaySize.y <= 0.0f);

		glm::vec4 clearColor = RenderCommand::GetClearColor();

		m_VulkanWindowData.ClearValue.color.float32[0] = clearColor.x * clearColor.w;
		m_VulkanWindowData.ClearValue.color.float32[1] = clearColor.y * clearColor.w;
		m_VulkanWindowData.ClearValue.color.float32[2] = clearColor.z * clearColor.w;
		m_VulkanWindowData.ClearValue.color.float32[3] = clearColor.w;

		if (!mainIsMinimized)
			FrameRender(&m_VulkanWindowData, mainDrawData);

		// Update and Render additional Platform Windows
		// Outside MainWindow context
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		// Present Main Platform Window
		if (!mainIsMinimized)
			FramePresent(&m_VulkanWindowData);

		ImGui_KarmaImplVulkan_Data* backendData = ImGuiVulkanHandler::ImGui_KarmaImplVulkan_GetBackendData();
		ImGui_KarmaImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		vkDeviceWaitIdle(vulkanInfo->Device);

		for (size_t i = 0; i < m_VulkanWindowData.MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkResetCommandBuffer(m_VulkanWindowData.FramesOnFlight[i].CommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		}
	}

	// Helper taken from https://github.com/TheCherno/Walnut/blob/cc26ee1cc875db50884fe244e0a3195dd730a1ef/Walnut/src/Walnut/Application.cpp#L270 who probably took help from official example https://github.com/ravimohan1991/imgui/blob/cf070488c71be01a04498e8eb50d66b982c7af9b/examples/example_glfw_vulkan/main.cpp#L261, with chiefly naming modifications and entire restructuring of ImGuiVulkanHandler::ImGui_KarmaImplVulkan_RenderDrawData.
	void ImGuiLayer::FrameRender(ImGui_KarmaImplVulkanH_Window* windowData, ImDrawData* drawData)
	{
		ImGui_KarmaImplVulkan_Data* backendData = ImGuiVulkanHandler::ImGui_KarmaImplVulkan_GetBackendData();
		ImGui_KarmaImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		// Pointer to the per frame data for instance fence, semaphores, and commandbuffer
		// Remember windowData->SemaphoreIndex is m_CurrentFrame equivalent of VulkanRendererAPI
		ImGui_Vulkan_Frame_On_Flight* frameOnFlightData = &windowData->FramesOnFlight[windowData->SemaphoreIndex];
		VkResult result;

		result = vkWaitForFences(vulkanInfo->Device, 1, &frameOnFlightData->Fence, VK_TRUE, UINT64_MAX);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to wait");

		// ImageAcquiredSemaphore is m_ImageAvailableSemaphores equivalent
		VkSemaphore imageAcquiredSemaphore = frameOnFlightData->ImageAcquiredSemaphore;
		VkSemaphore renderCompleteSemaphore = frameOnFlightData->RenderCompleteSemaphore;

		result = vkAcquireNextImageKHR(vulkanInfo->Device, windowData->Swapchain, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE, &windowData->ImageFrameIndex);

		// Pointer to the container of CommandPool, swapchainImages und views
		ImGui_KarmaImplVulkanH_ImageFrame* frameData = &windowData->ImageFrames[windowData->ImageFrameIndex];

		// May be try to free resources here
		//ImGuiVulkanHandler::ImGui_KarmaImplVulkan_ClearUndFreeResources(drawData, windowData->ImageFrameIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			m_SwapChainRebuild = true;
			return;
		}

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to acquire next image from swapchain");

		//result = vkResetCommandPool(m_Device, frameData->CommandPool, 0);
		//KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to reset command pool");

		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		// Recording begins:
		result = vkBeginCommandBuffer(frameOnFlightData->CommandBuffer, &info);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to begin command buffer");

		// Render Pass
		// Ponder over here for UI and 3D model depth stuff
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

		renderPassInfo.renderPass = windowData->RenderPass;
		renderPassInfo.framebuffer = frameData->Framebuffer;
		renderPassInfo.renderArea.extent = windowData->RenderArea.extent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0] = { windowData->ClearValue.color.float32[0], windowData->ClearValue.color.float32[1], 		windowData->ClearValue.color.float32[2], windowData->ClearValue.color.float32[3] };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(frameOnFlightData->CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		{
			// Record dear imgui primitives into command buffer
			ImGuiVulkanHandler::ImGui_KarmaImplVulkan_RenderDrawData(drawData, frameOnFlightData->CommandBuffer, VK_NULL_HANDLE, windowData->SemaphoreIndex);
		}

		vkCmdEndRenderPass(frameOnFlightData->CommandBuffer);

		result = vkEndCommandBuffer(frameOnFlightData->CommandBuffer);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to end command buffer");
		// Recording ends:

		// Submit command buffer
		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &imageAcquiredSemaphore;
		submitInfo.pWaitDstStageMask = &waitStage;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &(frameOnFlightData->CommandBuffer);
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &renderCompleteSemaphore;

		result = vkResetFences(vulkanInfo->Device, 1, &frameOnFlightData->Fence);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to reset fence");

		result = vkQueueSubmit(vulkanInfo->Queue, 1, &submitInfo, frameOnFlightData->Fence);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to submit queue");
	}

	void ImGuiLayer::FramePresent(ImGui_KarmaImplVulkanH_Window* windowData)
	{
		if (m_SwapChainRebuild)
		{
			return;
		}

		VkSemaphore renderCompleteSemaphore = windowData->FramesOnFlight[windowData->SemaphoreIndex].RenderCompleteSemaphore;

		VkPresentInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &renderCompleteSemaphore;
		info.swapchainCount = 1;
		info.pSwapchains = &windowData->Swapchain;
		info.pImageIndices = &windowData->ImageFrameIndex;

		ImGui_KarmaImplVulkan_Data* backendData = ImGuiVulkanHandler::ImGui_KarmaImplVulkan_GetBackendData();
		ImGui_KarmaImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		VkResult result = vkQueuePresentKHR(vulkanInfo->Queue, &info);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			m_SwapChainRebuild = true;
			return;
		}

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to submit queue");

		windowData->SemaphoreIndex = (windowData->SemaphoreIndex + 1) % windowData->MAX_FRAMES_IN_FLIGHT; // Now we can use the next set of semaphores
	}

	void ImGuiLayer::GracefulVulkanShutDown()
	{
		ImGui_KarmaImplVulkan_Data* backendData = ImGuiVulkanHandler::ImGui_KarmaImplVulkan_GetBackendData();
		ImGui_KarmaImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		VkResult result = vkDeviceWaitIdle(vulkanInfo->Device);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to wait for the completion of command buffers");

		CleanUpVulkanAndWindowData();

		ImGuiVulkanHandler::ImGui_KarmaImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::CleanUpVulkanAndWindowData()
	{
		// Clean up Window
		//ImGuiVulkanHandler::ClearVulkanWindowData(&m_VulkanWindowData, true);
		ImGuiVulkanHandler::ImGui_KarmaImplVulkan_DestroyWindow(&m_VulkanWindowData);

		ImGui_KarmaImplVulkan_Data* backendData = ImGuiVulkanHandler::ImGui_KarmaImplVulkan_GetBackendData();
		ImGui_KarmaImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		// Clean up Vulkan's pool component instantiated earlier here
		vkDestroyDescriptorPool(vulkanInfo->Device, m_ImGuiDescriptorPool, VK_NULL_HANDLE);
	}

	void ImGuiLayer::OnUpdate(float deltaTime)
	{
		// Nothing to do
	}

	void ImGuiLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonPressedEvent>(KR_BIND_EVENT_FN(ImGuiLayer::OnMouseButtonPressedEvent));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(KR_BIND_EVENT_FN(ImGuiLayer::OnMouseButtonReleasedEvent));
		dispatcher.Dispatch<MouseMovedEvent>(KR_BIND_EVENT_FN(ImGuiLayer::OnMouseMovedEvent));
		dispatcher.Dispatch<MouseScrolledEvent>(KR_BIND_EVENT_FN(ImGuiLayer::OnMouseScrollEvent));
		dispatcher.Dispatch<KeyPressedEvent>(KR_BIND_EVENT_FN(ImGuiLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<KeyReleasedEvent>(KR_BIND_EVENT_FN(ImGuiLayer::OnKeyReleasedEvent));
		dispatcher.Dispatch<KeyTypedEvent>(KR_BIND_EVENT_FN(ImGuiLayer::OnKeyTypedEvent));
		dispatcher.Dispatch<WindowResizeEvent>(KR_BIND_EVENT_FN(ImGuiLayer::OnWindowResizeEvent));
	}

	bool ImGuiLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[e.GetMouseButton()] = true;

		return false;
	}

	bool ImGuiLayer::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[e.GetMouseButton()] = false;

		return false;
	}

	bool ImGuiLayer::OnMouseMovedEvent(MouseMovedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(e.GetX(), e.GetY());

		return false;
	}

	bool ImGuiLayer::OnMouseScrollEvent(MouseScrolledEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheelH += e.GetXOffset();
		io.MouseWheel += e.GetYOffset();

		return false;
	}

	bool ImGuiLayer::OnKeyPressedEvent(KeyPressedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[e.GetKeyCode()] = true;

		io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
		io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
		io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
		io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

		return false;
	}

	bool ImGuiLayer::OnKeyReleasedEvent(KeyReleasedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[e.GetKeyCode()] = false;

		return false;
	}

	bool ImGuiLayer::OnKeyTypedEvent(KeyTypedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		int keycode = e.GetKeyCode();
		if (keycode > 0 && keycode < 0x10000)
		{
			io.AddInputCharacter((unsigned short)keycode);
		}

		return false;
	}

	bool ImGuiLayer::OnWindowResizeEvent(WindowResizeEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(float(e.GetWidth()), float(e.GetHeight()));
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

		return false;
	}
}