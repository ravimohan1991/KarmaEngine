#include "KarmaGuiRenderer.h"
#include "Renderer/RendererAPI.h"
#include "Vulkan/VulkanHolder.h"
#include "Renderer/RenderCommand.h"
#include "StaticMeshActor.h"
#include "Platform/Vulkan/VulkanVertexArray.h"
#include "VulkanRHI/VulkanDynamicRHI.h"
#include "VulkanRHI/VulkanSwapChain.h"
#include "VulkanRHI/VulkanSynchronization.h"
#include "KarmaRHI/DynamicRHI.h"
#include "VulkanRHI/VulkanRenderPass.h"
#include "VulkanRHI/VulkanDescriptorSets.h"
#include "StaticMeshActor.h"

// Emedded font
#include "Karma/KarmaGui/Roboto-Regular.h"

namespace Karma
{
	VkDescriptorPool KarmaGuiRenderer::m_KarmaGuiDescriptorPool;
	uint32_t KarmaGuiRenderer::m_SMCounter = 0;
	KarmaGui_ImplVulkanH_Window KarmaGuiRenderer::m_VulkanWindowData;
	bool KarmaGuiRenderer::m_SwapChainRebuild;
	GLFWwindow* KarmaGuiRenderer::m_GLFWwindow = nullptr;

	void KarmaGuiRenderer::SetUpKarmaGuiRenderer(GLFWwindow* window)
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		(void)io;

		m_GLFWwindow = window;

		if (GRHIInterfaceType == ERHIInterfaceType::Vulkan)
		{
			KarmaGui_ImplGlfw_InitForVulkan(window, true);

			KarmaGui_ImplVulkan_InitInfo initInfo = {};
			initInfo.Instance = FVulkanDynamicRHI::Get().GetInstance();
			initInfo.PhysicalDevice = FVulkanDynamicRHI::Get().GetDevice()->GetGPU();
			initInfo.Device = FVulkanDynamicRHI::Get().GetDevice()->GetLogicalDevice();
			initInfo.QueueFamily = FVulkanDynamicRHI::Get().FindQueueFamilies(initInfo.PhysicalDevice).graphicsFamily.value();
			initInfo.Queue = FVulkanDynamicRHI::Get().GetDevice()->GetGraphicsQueue();
			initInfo.PipelineCache = VK_NULL_HANDLE;
			initInfo.MinImageCount = FVulkanDynamicRHI::Get().GetGpuSwapChainSupportDetails().capabilities.minImageCount;
			initInfo.ImageCount = FVulkanDynamicRHI::Get().SwapChainImageCount();
			initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

			// Stuff created and dedicated to KarmaGui
			CreateDescriptorPool(initInfo.Device);
			initInfo.DescriptorPool = m_KarmaGuiDescriptorPool;

			// Settingup backend in KarmaGui
			// KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_Init(&initInfo);
			KarmaGui_ImplVulkan_Init(&initInfo);

			// Fresh start with newly instantiated Vulkan data
			KarmaGuiVulkanHandler::FillWindowData(&m_VulkanWindowData, true);

			// See if all the appropriate Vulkan resources have been instantiated
			KarmaGuiVulkanHandler::CheckInitialization();

			// Font, descriptor, and pipeline
			KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateDeviceObjects();
			FVulkanDynamicRHI::Get().GetDevice()->InitializeDefaultDescriptorSets(m_VulkanWindowData.RHIResources->VulkanSwapChain->GetMaxFramesInFlight());

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
				// Images are loaded via KarmaGuiRenderer::AddImageTexture

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
			// No images to load yet
		}
	}

	void KarmaGuiRenderer::OnAdditionOfStaticMesh(AStaticMeshActor* smActor)
	{
		if(GRHIInterfaceType == ERHIInterfaceType::Vulkan)
		{
			uint32_t maxFramesInFlight = GetWindowData().RHIResources->VulkanSwapChain->GetMaxFramesInFlight();
			
			for (uint32_t counter = 0; counter < maxFramesInFlight; counter++)
			{
				FVulkanDynamicRHI::Get().GetDevice()->GetDefaultDescriptorSets()[counter]->UpdateUniformBufferDescriptorSet(static_cast<VulkanUniformBuffer*>(smActor->GetMeshTransformUniform().get()), 1, m_SMCounter, counter);
			}
			
			m_SMCounter++;
		}
	}

	void KarmaGuiRenderer::AddImageTexture(char const* fileName, const std::string& label)
	{
		switch(GRHIInterfaceType)
		{
			case ERHIInterfaceType::Vulkan:
			{
				KarmaGuiIO& io = KarmaGui::GetIO();
				KarmaGuiBackendRendererUserData* backendData = (KarmaGuiBackendRendererUserData*) io.BackendRendererUserData;

				// Use any command queue
				VkCommandPool commandPool = m_VulkanWindowData.CommandPool;
				VkCommandBuffer commandBuffer = m_VulkanWindowData.FramesOnFlight[m_VulkanWindowData.SemaphoreIndex].CommandBuffer;

				VkResult result = vkResetCommandPool(backendData->VulkanInitInfo.Device, commandPool, 0);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to reset command pool!");

				VkCommandBufferBeginInfo beginInfo = {};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to begin recording(?) command buffer!");

				KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateTexture(commandBuffer, fileName, label);

				VkSubmitInfo endInfo = {};
				endInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				endInfo.commandBufferCount = 1;
				endInfo.pCommandBuffers = &commandBuffer;
				result = vkEndCommandBuffer(commandBuffer);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to end recording(?) command buffer!");

				result = vkQueueSubmit(backendData->VulkanInitInfo.Queue, 1, &endInfo, VK_NULL_HANDLE);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to submit command buffer!");

				result = vkDeviceWaitIdle(backendData->VulkanInitInfo.Device);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to wait!");
			}
			break;
			case ERHIInterfaceType::OpenGL:
			{
				KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_CreateTexture(fileName, label);
			}
			break;
			case ERHIInterfaceType::Null:
					KR_CORE_ASSERT(false, "RendererAPI::None is not supported");
				break;
			default:
					KR_CORE_ASSERT(false, "Unknown RendererAPI {0} is in play.")
				break;
		}
	}

	void KarmaGuiRenderer::OnKarmaGuiLayerBegin()
	{
		switch (GRHIInterfaceType)
		{
		case ERHIInterfaceType::Vulkan:
				GiveLoopBeginControlToVulkan();
			break;
		case ERHIInterfaceType::OpenGL:
				KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_NewFrame();
				KarmaGui_ImplGlfw_NewFrame();
			break;
		case ERHIInterfaceType::Null:
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

	void KarmaGuiRenderer::KarmaGui_ImplVulkan_Init(KarmaGui_ImplVulkan_InitInfo* info)
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		KR_CORE_ASSERT(io.BackendRendererUserData == nullptr, "Already initialized a renderer backend!");

		// Setup backend capabilities flags
		KarmaGuiBackendRendererUserData* backendData = new KarmaGuiBackendRendererUserData();

		// Since it seems like initialized struct, in MSVC, leads to problems in pushing back, we are doing this.
		backendData->BufferMemoryAlignment = 256;

		io.BackendRendererUserData = (void*)backendData;
		io.BackendRendererName = "Vulkan_Got_Back";
		io.BackendFlags |= KGGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.

		// Maybe chore for toofani mood!
		// io.BackendFlags |= KarmaGuiBackendFlags_RendererHasViewports;  // We can create multi-viewports on the Renderer side (optional)

		backendData->VulkanInitInfo = *info;
		//backendData->VulkanInitInfo.Device = info->Device;

		//backendData->RenderPass = info->RenderPass;
		//backendData->Subpass = info->Subpass;

		// Font, descriptor, and pipeline (moved to the KarmaGuiRenderer::SetUpKarmaGuiRenderer, after KarmaGuiVulkanHandler::FillWindowData
		// because RenderPass is required for pipeline creation)
		// KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateDeviceObjects();

		// Our render function expect RendererUserData to be storing the window render buffer we need (for the main viewport we won't use ->Window)
		KarmaGuiViewport* mainViewport = KarmaGui::GetMainViewport();

		mainViewport->RendererUserData = new KarmaGui_ImplVulkan_ViewportData();

		if (io.ConfigFlags & KGGuiConfigFlags_ViewportsEnable)
		{
			// Setting up KarmaGui's window operations (create, resize, and all that)
			KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_InitPlatformInterface();
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
                //KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateOrResizeWindow(&m_VulkanWindowData, true, true);
                KarmaGuiVulkanHandler::ShivaSwapChainForRebuild(&m_VulkanWindowData);
                KarmaGuiVulkanHandler::FillWindowData(&m_VulkanWindowData, false);

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

	KarmaGuiBackendRendererUserData* KarmaGuiRenderer::GetBackendRendererUserData()
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		return (KarmaGuiBackendRendererUserData*)io.BackendRendererUserData;
	}

	void KarmaGuiRenderer::GracefulVulkanShutDown()
	{
		KarmaGuiBackendRendererUserData* backendData = GetBackendRendererUserData();
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
		KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_DestroyWindow(&m_VulkanWindowData);

		KarmaGuiBackendRendererUserData* backendData = GetBackendRendererUserData();
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		// Clean up Vulkan's pool component instantiated earlier here
		vkDestroyDescriptorPool(vulkanInfo->Device, m_KarmaGuiDescriptorPool, VK_NULL_HANDLE);
	}

	void KarmaGuiRenderer::CreateDescriptorPool(VkDevice VulkanDevice)
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

		VkResult result = vkCreateDescriptorPool(VulkanDevice, &poolInfo, nullptr, &m_KarmaGuiDescriptorPool);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create descriptor pool for KarmaGui");
	}

	// Helper taken from https://github.com/TheCherno/Walnut/blob/cc26ee1cc875db50884fe244e0a3195dd730a1ef/Walnut/src/Walnut/Application.cpp#L270 who probably took help from official example https://github.com/ravimohan1991/imgui/blob/cf070488c71be01a04498e8eb50d66b982c7af9b/examples/example_glfw_vulkan/main.cpp#L261, with chiefly naming modifications and drawing of 3D scene on 2D render target
	void KarmaGuiRenderer::FrameRender(KarmaGui_ImplVulkanH_Window* windowData, KGDrawData* drawData)
	{
		KarmaGuiBackendRendererUserData* backendData = GetBackendRendererUserData();
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		// Pointer to the per frame data for instance fence, semaphores, and commandbuffer
		KarmaGui_Vulkan_Frame_On_Flight* frameOnFlightData = &windowData->FramesOnFlight[windowData->SemaphoreIndex];
		VkResult result;

		// Fence needs to be signaled to pass the vkWaitForFences.
		FVulkanDynamicRHI::Get().GetDevice()->GetFenceManager().WaitForFence(frameOnFlightData->Fence);

		VkSemaphore imageAcquiredSemaphore = frameOnFlightData->ImageAcquiredSemaphore;
		VkSemaphore renderCompleteSemaphore = frameOnFlightData->RenderCompleteSemaphore;

		result = vkAcquireNextImageKHR(vulkanInfo->Device, windowData->Swapchain, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE, &windowData->ImageFrameIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			m_SwapChainRebuild = true;
			vkDeviceWaitIdle(FVulkanDynamicRHI::Get().GetDevice()->GetLogicalDevice());

			return;
		}
		else
		{
			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to acquire next image");
		}

		// Pointer to the container of framebuffers (based on number of swapchain images)
		KarmaGui_ImplVulkanH_ImageFrame* frameData = &windowData->ImageFrames[windowData->ImageFrameIndex];

		vkResetCommandBuffer(frameOnFlightData->CommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		// recording begins:
		{
			VkCommandBufferBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			
			result = vkBeginCommandBuffer(frameOnFlightData->CommandBuffer, &info);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't begin commandbuffer recording");
		}

		FVulkanDynamicRHI::Get().UploadUniformBufferObjects(windowData->SemaphoreIndex);
		
		for (auto it = backendData->Elements3DTo2D.begin(); it != backendData->Elements3DTo2D.end(); ++it)
		{
			std::shared_ptr<Scene> scene3D = it->Scene3D;

			FVulkanDescriptorSets* descriptorSets = FVulkanDynamicRHI::Get().GetDevice()->GetDefaultDescriptorSets()[windowData->SemaphoreIndex];

			{
				VkRenderPassBeginInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = backendData->OffScreenRR.RenderPass->GetHandle();
				renderPassInfo.framebuffer = it->FrameBuffer;
				renderPassInfo.renderArea.offset = {0, 0};
				renderPassInfo.renderArea.extent.width = it->Size.x;
				renderPassInfo.renderArea.extent.height = it->Size.y;
				
				// Define clear values for the color and depth attachments
				std::vector<VkClearValue> clearValues(2);
				clearValues[0].color = {{0.0f, 0.0f, 0.0f, 0.0f}}; // Clear color to transparent
				clearValues[1].depthStencil = {1.0f, 0};           // Clear depth to 1.0 (farthest)
				
				renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
				renderPassInfo.pClearValues = clearValues.data();
				
				// The pass starts here and all commands until vkCmdEndRenderPass are recorded into it
				vkCmdBeginRenderPass(frameOnFlightData->CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

				// ---- Bind Graphics Pipeline ----
				vkCmdBindPipeline(frameOnFlightData->CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, backendData->OffScreenRR.OffscreenGraphicsPipeline);

				// === BIND GLOBAL DESCRIPTOR SETS (once per frame) ===
				// Set 0, Binding 0 :Camera UBO
				vkCmdBindDescriptorSets(frameOnFlightData->CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, backendData->OffScreenRR.OffscreenPipelineLayout, 0, 1, &descriptorSets->m_DescriptorSets[0][0], 0, nullptr);
				// Set 0, Binding 1: Texture
				vkCmdBindDescriptorSets(frameOnFlightData->CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, backendData->OffScreenRR.OffscreenPipelineLayout, 0, 1, &descriptorSets->m_DescriptorSets[0][0], 0, nullptr);

				uint32_t objectIndex = 0;
				
				// ---- Bind 3D Vertex And Index Buffers ----
				for (const auto& smActor : scene3D->GetSMActors())
				{
					std::shared_ptr<Mesh> mesh = smActor->GetStaticMeshComponent()->GetStaticMesh();

					VkBuffer vertexBuffers[1] = { std::static_pointer_cast<VulkanVertexBuffer>(mesh->GetVertexBuffer())->GetVertexBuffer() };
					VkDeviceSize vertexOffset[1] = { 0 };
					vkCmdBindVertexBuffers(frameOnFlightData->CommandBuffer, 0, 1, vertexBuffers, vertexOffset);
					vkCmdBindIndexBuffer(frameOnFlightData->CommandBuffer, std::static_pointer_cast<VulkanIndexBuffer>(mesh->GetIndexBuffer())->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

					// Set 2: Object UBO
					vkCmdBindDescriptorSets(frameOnFlightData->CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, backendData->OffScreenRR.OffscreenPipelineLayout, 1, 1, &descriptorSets->m_DescriptorSets[1][objectIndex], 0, nullptr);

					// ----Issue Draw Commands----
					// Draw 3D scene geometry on 2D rendertarget (it->FrameBuffers)
					vkCmdDrawIndexed(frameOnFlightData->CommandBuffer, std::static_pointer_cast<VulkanIndexBuffer>(mesh->GetIndexBuffer())->GetCount(), 1, 0, 0, 0);

					objectIndex++;
				}
				
				// ---- End the Offscreen Render Pass ----
				vkCmdEndRenderPass(frameOnFlightData->CommandBuffer);
			}
		}

		// Render Pass
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

		renderPassInfo.renderPass = windowData->RenderPass;
		renderPassInfo.framebuffer = frameData->Framebuffer;
		renderPassInfo.renderArea.extent = windowData->RenderArea.extent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0] = { windowData->ClearValue.color.float32[0], windowData->ClearValue.color.float32[1], windowData->ClearValue.color.float32[2], windowData->ClearValue.color.float32[3] };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(frameOnFlightData->CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		{
			// Record KarmaGui primitives into command buffer
			KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_RenderDrawData(drawData, frameOnFlightData->CommandBuffer, VK_NULL_HANDLE, windowData->SemaphoreIndex);
		}

		vkCmdEndRenderPass(frameOnFlightData->CommandBuffer);

		result = vkEndCommandBuffer(frameOnFlightData->CommandBuffer);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to end command buffer");
		// Recording ends:
		// command buffer can be submitted to the queue for execution

		// Submit command buffer
		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &imageAcquiredSemaphore;
		submitInfo.pWaitDstStageMask = &waitStage;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &frameOnFlightData->CommandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &renderCompleteSemaphore;

		// vkResetFences unsignals the Fence
		// Fixing a deadlock: https://vulkan-tutorial.com/Drawing_a_triangle/Swap_chain_recreation#page_Fixing-a-deadlock
		FVulkanDynamicRHI::Get().GetDevice()->GetFenceManager().ResetFence(frameOnFlightData->Fence);

		VkFence fence = frameOnFlightData->Fence->GetHandle();
		// vkQueueSubmit signals the Fence once commandbuffers finish execution
		result = vkQueueSubmit(vulkanInfo->Queue, 1, &submitInfo, fence);
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

		KarmaGuiBackendRendererUserData* backendData = GetBackendRendererUserData();
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		VkResult result = vkQueuePresentKHR(vulkanInfo->Queue, &info);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			m_SwapChainRebuild = true;
		}
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			return;
		}

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to submit queue");

		windowData->SemaphoreIndex = (windowData->SemaphoreIndex + 1) % windowData->MAX_FRAMES_IN_FLIGHT; // Now we can use the next set of semaphores
	}

	KGTextureID KarmaGuiRenderer::Add3DSceneFor2DRendering(std::shared_ptr<Scene> scene, KGVec2 dimensions)
	{
		if(GRHIInterfaceType == ERHIInterfaceType::Vulkan)
		{
			KarmaGui_ImplVulkan_Data* backendData = GetBackendRendererUserData();
			KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;
			
			for(auto it = backendData->Elements3DTo2D.begin(); it != backendData->Elements3DTo2D.end(); ++it)
			{
				if(it->Scene3D == scene)
				{
					return it->KarmaGui_Texture;
				}
			}
			
			KarmaGui_3DScene_To_2DTexture_Data SceneToTexture;
			
			SceneToTexture.Size = dimensions;
			SceneToTexture.Scene3D = scene;
			
			VkResult result;
			
			// bAllocationDoneOnce is set in KarmaGuiVulkanHandler::CreateOffScreenTextureResources()
			if(!backendData->OffScreenRR.bAllocationDoneOnce)
			{
				VkSamplerCreateInfo samplerInfo{};
				samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				samplerInfo.magFilter = VK_FILTER_LINEAR;
				samplerInfo.minFilter = VK_FILTER_LINEAR;
				samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				samplerInfo.anisotropyEnable = VK_FALSE;
				samplerInfo.maxAnisotropy = 1.0f;
				samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
				samplerInfo.unnormalizedCoordinates = VK_FALSE;
				samplerInfo.compareEnable = VK_FALSE;
				samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
				samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				samplerInfo.mipLodBias = 0.0f;
				samplerInfo.minLod = 0.0f;
				samplerInfo.maxLod = 0.0f;
				
				result = vkCreateSampler(vulkanInfo->Device, &samplerInfo, vulkanInfo->Allocator, &backendData->OffScreenRR.Sampler);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't create sampler");
			}

			// Create images (along with appropriate memory)
			{
				VkImageCreateInfo imageCreateCI{};
				imageCreateCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				imageCreateCI.imageType = VK_IMAGE_TYPE_2D;
				imageCreateCI.format = VK_FORMAT_R8G8B8A8_UNORM;
				imageCreateCI.extent.width = dimensions.x;
				imageCreateCI.extent.height = dimensions.y;
				imageCreateCI.extent.depth = 1;
				imageCreateCI.arrayLayers = 1;
				imageCreateCI.mipLevels = 1;
				imageCreateCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageCreateCI.samples = VK_SAMPLE_COUNT_1_BIT;
				imageCreateCI.tiling = VK_IMAGE_TILING_OPTIMAL;
				imageCreateCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
				imageCreateCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				result = vkCreateImage(vulkanInfo->Device, &imageCreateCI, vulkanInfo->Allocator, &SceneToTexture.Image);
					KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't create a image");

				VkMemoryRequirements req;
				vkGetImageMemoryRequirements(vulkanInfo->Device, SceneToTexture.Image, &req);
				VkMemoryAllocateInfo allocInfo = {};
				allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				allocInfo.allocationSize = req.size;
				allocInfo.memoryTypeIndex = KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_MemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, req.memoryTypeBits);

				result = vkAllocateMemory(vulkanInfo->Device, &allocInfo, vulkanInfo->Allocator, &SceneToTexture.DeviceMemory);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't allocate memory");

				result = vkBindImageMemory(vulkanInfo->Device, SceneToTexture.Image, SceneToTexture.DeviceMemory, 0);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't bind image memory");
			}

			// insert image memory barrier for VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL?

			// Create image view:
			{
				VkImageViewCreateInfo viewInfo{};
				viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewInfo.image = SceneToTexture.Image;
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
				viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				viewInfo.subresourceRange.baseMipLevel = 0;
				viewInfo.subresourceRange.levelCount = 1;
				viewInfo.subresourceRange.baseArrayLayer = 0;
				viewInfo.subresourceRange.layerCount = 1;

				result = vkCreateImageView(vulkanInfo->Device, &viewInfo, vulkanInfo->Allocator, &SceneToTexture.Image_View);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't create image view");
			}

			SceneToTexture.KarmaGui_Texture = KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_AddTexture(backendData->OffScreenRR.Sampler, SceneToTexture.Image_View, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			
						
			backendData->Elements3DTo2D.push_back(SceneToTexture);
			
			// OffScreen texture (3D scene to 2D texture) resources like renderpass, framebuffers, and depth resources
			// for the SceneToTexture element. We also create different graphics pipeline specifically for KarmaGui window display of the 3D scene. We may be creating some resources again when not required, when new element is added
			// resources for previous elements are also created. So take care of that.
			KarmaGuiVulkanHandler::CreateOffScreenTextureResources();
			
			return SceneToTexture.KarmaGui_Texture;
		}

		return nullptr;
	}

	KGTextureID KarmaGuiBackendRendererUserData::GetTextureIDAtIndex(uint32_t index)
	{
		if (RendererAPI::GetAPI() == RendererAPI::API::Vulkan)
		{
			return vulkanMesaDecalDataList.at(index)->TextureDescriptorSet;
		}
		else if(RendererAPI::GetAPI() == RendererAPI::API::OpenGL)
		{
			return openglMesaDecalDataList.at(index).DecalID;
		}

		return nullptr;
	}

	uint32_t KarmaGuiBackendRendererUserData::GetTextureWidthAtIndex(uint32_t index)
	{
		if (RendererAPI::GetAPI() == RendererAPI::API::Vulkan)
		{
			return vulkanMesaDecalDataList.at(index)->width;
		}
		else if(RendererAPI::GetAPI() == RendererAPI::API::OpenGL)
		{
			return openglMesaDecalDataList.at(index).width;
		}

		return 0;
	}

	uint32_t KarmaGuiBackendRendererUserData::GetTextureHeightAtIndex(uint32_t index)
	{
		if (RendererAPI::GetAPI() == RendererAPI::API::Vulkan)
		{
			return vulkanMesaDecalDataList.at(index)->height;
		}
		else if(RendererAPI::GetAPI() == RendererAPI::API::OpenGL)
		{
			return openglMesaDecalDataList.at(index).height;
		}

		return 0;
	}
}
