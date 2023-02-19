#include "KarmaGuiRenderer.h"
#include "Renderer/RendererAPI.h"
#include "Vulkan/VulkanHolder.h"
#include "Renderer/RenderCommand.h"

// Emedded font
#include "Karma/KarmaGui/Roboto-Regular.h"

namespace Karma
{
	VkDescriptorPool KarmaGuiRenderer::m_KarmaGuiDescriptorPool;
	KarmaGui_ImplVulkanH_Window KarmaGuiRenderer::m_VulkanWindowData;
	bool KarmaGuiRenderer::m_SwapChainRebuild;
	GLFWwindow* KarmaGuiRenderer::m_GLFWwindow = nullptr;

	void KarmaGuiRenderer::SetUpKarmaGuiRenderer(GLFWwindow* window)
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		(void)io;

		m_GLFWwindow = window;

		if (RendererAPI::GetAPI() == RendererAPI::API::Vulkan)
		{
			// Exposing Karma's Vulkan components to Dear ImGui
			KarmaGui_ImplGlfw_InitForVulkan(window, true);

			KarmaGui_ImplVulkan_InitInfo initInfo = {};
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
			initInfo.DescriptorPool = m_KarmaGuiDescriptorPool;
			initInfo.RenderPass = VulkanHolder::GetVulkanContext()->GetRenderPass();

			// Settingup backend in KarmaGui
			KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_Init(&initInfo);

			// Fresh start with newly instantiated Vulkan data
			// Since VulkanContext has already instantiated fresh swapchain and commandbuffers, we send that false
			KarmaGuiVulkanHandler::ShareVulkanContextResourcesOfMainWindow(&m_VulkanWindowData, true);

			// Load default font
			KGFontConfig fontConfig;
			fontConfig.FontDataOwnedByAtlas = false;
			KGFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular, sizeof(g_RobotoRegular), 20.0f, &fontConfig);
			io.FontDefault = robotoFont;

			// Upload Fonts and Images
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
				KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateFontsTexture(commandBuffer);

				// Load Images

				//	1. The wall
				KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateTexture(commandBuffer, "../Resources/Textures/The_Source_Wall.jpg", "The Source");

				// 2. 3D Exhibitor large image (primitive theme)
				KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateTexture(commandBuffer, "../Resources/Textures/Measures.png", "Primitive Background");

				// 3. Icons Packa

				KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateTexture(commandBuffer, "../Resources/Textures/EditorIcons/File.png", "File icon");
				KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateTexture(commandBuffer, "../Resources/Textures/EditorIcons/Folder.png", "Folder icon");
				KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateTexture(commandBuffer, "../Resources/Textures/EditorIcons/OpenFolder.png", "Opened Folder incon");
				KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateTexture(commandBuffer, "../Resources/Textures/EditorIcons/LeftArrow.png", "Left Arrow icon");
				KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateTexture(commandBuffer, "../Resources/Textures/EditorIcons/RightArrow.png");

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

				KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_DestroyFontUploadObjects();
			}
		}
		else if (RendererAPI::GetAPI() == RendererAPI::API::OpenGL)
		{
			KarmaGui_ImplGlfw_InitForOpenGL(window, true);

			// Sets backend in KarmaGui
			KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_Init("#version 410");

			// Load default font
			KGFontConfig fontConfig;
			fontConfig.FontDataOwnedByAtlas = false;
			KGFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular, sizeof(g_RobotoRegular), 20.0f, &fontConfig);
			io.FontDefault = robotoFont;

			// Load images

			//	1. The wall
			KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_CreateTexture("../Resources/Textures/The_Source_Wall.jpg", "The Source");

			// 2. 3D Exhibitor large image (primitive theme)
			KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_CreateTexture("../Resources/Textures/Measures.png", "Primitive Background");

			// 3. Icons Packa
			KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_CreateTexture("../Resources/Textures/EditorIcons/File.png", "File icon");
			KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_CreateTexture("../Resources/Textures/EditorIcons/Folder.png", "Folder icon");
			KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_CreateTexture("../Resources/Textures/EditorIcons/OpenFolder.png", "Opened Folder incon");
			KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_CreateTexture("../Resources/Textures/EditorIcons/LeftArrow.png", "Left Arrow icon");
			KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_CreateTexture("../Resources/Textures/EditorIcons/RightArrow.png");
		}
	}

	void KarmaGuiRenderer::OnKarmaGuiLayerBegin()
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan:
			GiveLoopBeginControlToVulkan();
			break;
		case RendererAPI::API::OpenGL:
			KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_NewFrame();
			KarmaGui_ImplGlfw_NewFrame();
			break;
		case RendererAPI::API::None:
			KR_CORE_ASSERT(false, "RendererAPI::None is not supported");
			break;
		default:
			KR_CORE_ASSERT(false, "Unknown RendererAPI {0} is in play.")
				break;
		}
	}

	void KarmaGuiRenderer::OnKarmaGuiLayerEnd()
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		(void)io;

		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan:
			GiveLoopEndControlToVulkan();
			break;
		case RendererAPI::API::OpenGL:
		{
			int displayWidth, displayHeight;
			glfwGetFramebufferSize(m_GLFWwindow, &displayWidth, &displayHeight);
			glViewport(0, 0, displayWidth, displayHeight);
			glm::vec4 clearColor = RenderCommand::GetClearColor();
			glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
			glClear(GL_COLOR_BUFFER_BIT);
			KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_RenderDrawData(KarmaGui::GetDrawData());
			if (io.ConfigFlags & KGGuiConfigFlags_ViewportsEnable)
			{
				GLFWwindow* backup_current_context = glfwGetCurrentContext();
				KarmaGui::UpdatePlatformWindows();
				KarmaGui::RenderPlatformWindowsDefault();
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

	void KarmaGuiRenderer::GiveLoopBeginControlToVulkan()
	{
		// Resize swap chain?
		if (m_SwapChainRebuild)
		{
			int width, height;
			//GLFWwindow* window = static_cast<GLFWwindow*>(m_AssociatedWindow->GetNativeWindow());
			glfwGetFramebufferSize(m_GLFWwindow, &width, &height);

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

				KarmaGuiVulkanHandler::ShareVulkanContextResourcesOfMainWindow(&m_VulkanWindowData, false);
				m_SwapChainRebuild = false;
			}
		}

		KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_NewFrame();
		KarmaGui_ImplGlfw_NewFrame();
	}

	void KarmaGuiRenderer::GiveLoopEndControlToVulkan()
	{
		// Rendering
		KarmaGui::Render();
		KGDrawData* mainDrawData = KarmaGui::GetDrawData();
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
		KarmaGuiIO& io = KarmaGui::GetIO();
		if (io.ConfigFlags & KGGuiConfigFlags_ViewportsEnable)
		{
			KarmaGui::UpdatePlatformWindows();
			KarmaGui::RenderPlatformWindowsDefault();
		}

		// Present Main Platform Window
		if (!mainIsMinimized)
			FramePresent(&m_VulkanWindowData);

		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_GetBackendData();
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		vkDeviceWaitIdle(vulkanInfo->Device);

		for (size_t i = 0; i < m_VulkanWindowData.MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkResetCommandBuffer(m_VulkanWindowData.FramesOnFlight[i].CommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		}
	}

	void KarmaGuiRenderer::OnKarmaGuiLayerDetach()
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan:
			GracefulVulkanShutDown();
			break;
		case RendererAPI::API::OpenGL:
			KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_Shutdown();
			KarmaGui_ImplGlfw_Shutdown();
			KarmaGui::DestroyContext();
			break;
		case RendererAPI::API::None:
			KR_CORE_ASSERT(false, "RendererAPI::None is not supported");
			break;
		default:
			KR_CORE_ASSERT(false, "Unknown RendererAPI {0} is in play.")
				break;
		}
	}

	void* KarmaGuiRenderer::GetBackendRendererUserData()
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		return io.BackendRendererUserData;
	}

	void KarmaGuiRenderer::GracefulVulkanShutDown()
	{
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_GetBackendData();
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		VkResult result = vkDeviceWaitIdle(vulkanInfo->Device);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to wait for the completion of command buffers");

		CleanUpVulkanAndWindowData();

		KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_Shutdown();
		KarmaGui_ImplGlfw_Shutdown();
		KarmaGui::DestroyContext();
	}

	void KarmaGuiRenderer::CleanUpVulkanAndWindowData()
	{
		// Clean up Window
		//ImGuiVulkanHandler::ClearVulkanWindowData(&m_VulkanWindowData, true);
		KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_DestroyWindow(&m_VulkanWindowData);

		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_GetBackendData();
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		// Clean up Vulkan's pool component instantiated earlier here
		vkDestroyDescriptorPool(vulkanInfo->Device, m_KarmaGuiDescriptorPool, VK_NULL_HANDLE);
	}

	void KarmaGuiRenderer::CreateDescriptorPool()
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

		VkResult result = vkCreateDescriptorPool(VulkanHolder::GetVulkanContext()->GetLogicalDevice(), &poolInfo, nullptr, &m_KarmaGuiDescriptorPool);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create descriptor pool for ImGui");
	}

	// Helper taken from https://github.com/TheCherno/Walnut/blob/cc26ee1cc875db50884fe244e0a3195dd730a1ef/Walnut/src/Walnut/Application.cpp#L270 who probably took help from official example https://github.com/ravimohan1991/imgui/blob/cf070488c71be01a04498e8eb50d66b982c7af9b/examples/example_glfw_vulkan/main.cpp#L261, with chiefly naming modifications and entire restructuring of KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_RenderDrawData.
	void KarmaGuiRenderer::FrameRender(KarmaGui_ImplVulkanH_Window* windowData, KGDrawData* drawData)
	{
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_GetBackendData();
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		// Pointer to the per frame data for instance fence, semaphores, and commandbuffer
		// Remember windowData->SemaphoreIndex is m_CurrentFrame equivalent of VulkanRendererAPI
		KarmaGui_Vulkan_Frame_On_Flight* frameOnFlightData = &windowData->FramesOnFlight[windowData->SemaphoreIndex];
		VkResult result;

		result = vkWaitForFences(vulkanInfo->Device, 1, &frameOnFlightData->Fence, VK_TRUE, UINT64_MAX);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to wait");

		// ImageAcquiredSemaphore is m_ImageAvailableSemaphores equivalent
		VkSemaphore imageAcquiredSemaphore = frameOnFlightData->ImageAcquiredSemaphore;
		VkSemaphore renderCompleteSemaphore = frameOnFlightData->RenderCompleteSemaphore;

		result = vkAcquireNextImageKHR(vulkanInfo->Device, windowData->Swapchain, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE, &windowData->ImageFrameIndex);

		// Pointer to the container of CommandPool, swapchainImages und views
		KarmaGui_ImplVulkanH_ImageFrame* frameData = &windowData->ImageFrames[windowData->ImageFrameIndex];

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
			KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_RenderDrawData(drawData, frameOnFlightData->CommandBuffer, VK_NULL_HANDLE, windowData->SemaphoreIndex);
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

	void KarmaGuiRenderer::FramePresent(KarmaGui_ImplVulkanH_Window* windowData)
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

		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_GetBackendData();
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		VkResult result = vkQueuePresentKHR(vulkanInfo->Queue, &info);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			m_SwapChainRebuild = true;
			return;
		}

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to submit queue");

		windowData->SemaphoreIndex = (windowData->SemaphoreIndex + 1) % windowData->MAX_FRAMES_IN_FLIGHT; // Now we can use the next set of semaphores
	}
}