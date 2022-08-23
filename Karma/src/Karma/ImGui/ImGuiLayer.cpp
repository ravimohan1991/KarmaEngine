// Got help from https://frguthmann.github.io/posts/vulkan_imgui/
// and https://github.com/TheCherno/Walnut
// for Vulkan specific part.

#include "ImGuiLayer.h"
#include "GLFW/glfw3.h"
#include "Karma/Application.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Vulkan/VulkanHolder.h"
#include "Karma/Renderer/RendererAPI.h"
#include "Karma/Renderer/RenderCommand.h"
#include "glm/glm.hpp"

// Emedded font
#include "Karma/ImGui/Roboto-Regular.h"//

namespace Karma
{
	ImGuiLayer::ImGuiLayer(Window* relevantWindow)
		: Layer("ImGuiLayer"), m_AssociatedWindow(relevantWindow)
	{
		if(RendererAPI::GetAPI() == RendererAPI::API::Vulkan)
		{
			// Cache the VkDevice from context already created
			m_Device = VulkanHolder::GetVulkanContext()->GetLogicalDevice();
			
			// Create descriptor pool for IMGUI (code taken from from imgui demo)
			CreateDescriptorPool();
			
			// Need this else get confronted by assertion in imgui_impl_vulkan.cpp
			// "Need to call ImGui_ImplVulkan_LoadFunctions() if IMGUI_IMPL_VULKAN_NO_PROTOTYPES or VK_NO_PROTOTYPES are set!"
			ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void*) { return vkGetInstanceProcAddr(VulkanHolder::GetVulkanContext()->GetInstance(), function_name); });
			
			// Create Framebuffers, gather Window data
			int width, height;
			GLFWwindow* window = static_cast<GLFWwindow*>(m_AssociatedWindow->GetNativeWindow());
			
			glfwGetFramebufferSize(window, &width, &height);
			ImGui_ImplVulkanH_Window* windowData = &m_VulkanWindowData;
			GatherVulkanWindowData(windowData, VulkanHolder::GetVulkanContext()->GetSurface(), width, height);
			
			// Let me see what data we have gathered so far
			/*
			KR_CORE_INFO("+-----------------------------------------");
			KR_CORE_INFO("| Window Width: {0}", vulkanWindowData.Width);
			KR_CORE_INFO("| Window Height: {0}", vulkanWindowData.Height);
			KR_CORE_INFO("| PresentMode: {0}", vulkanWindowData.PresentMode);
			KR_CORE_INFO("| ImageCount: {0}", vulkanWindowData.ImageCount);
			KR_CORE_INFO("+-----------------------------------------");
			KR_CORE_ASSERT(false, "That is it folks!")
			*/
			
			// Allocate ImGui resources for frames_in_flight, accordingly
			m_AllocatedCommandBuffers.resize(m_VulkanWindowData.ImageCount);
			m_ResourceFreeQueue.resize(m_VulkanWindowData.ImageCount);	
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

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		VkResult result = vkCreateDescriptorPool(m_Device, &pool_info, nullptr, &m_ImGuiDescriptorPool);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create descriptor pool for ImGui");
	}
	
	VkCommandBuffer ImGuiLayer::AllocateImGuiCommandBuffers(bool bBegin)
	{
		ImGui_ImplVulkanH_Window* windowData = &m_VulkanWindowData;

		// Use any command queue
		// We will use the one created by ImGui like so
		// https://github.com/ravimohan1991/imgui/blob/83c4d0108c730531a916ace1d6b5bf5fc7d2f1ee/backends/imgui_impl_vulkan.cpp#L1073
		VkCommandPool command_pool = windowData->Frames[windowData->FrameIndex].CommandPool;
		
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = command_pool;//VulkanHolder::GetVulkanContext()->GetCommandPool(); <--- The less inter-class dependence, the better!
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;// Ponder why 1
		
		VkCommandBuffer& command_buffer = m_AllocatedCommandBuffers[windowData->FrameIndex].emplace_back();
		VkResult result = vkAllocateCommandBuffers(m_Device, &allocInfo, &command_buffer);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create command buffers!");
		
		if(bBegin)
		{
			VkCommandBufferBeginInfo begin_info = {};
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			result = vkBeginCommandBuffer(command_buffer, &begin_info);
			
			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to begin command buffers!");
		}

		return command_buffer;
	}

	// Here we fill up the Vulkan relevant fields associated with GLFW window (?)
	void ImGuiLayer::GatherVulkanWindowData(ImGui_ImplVulkanH_Window* vulkanWindowData, VkSurfaceKHR surface, int width, int height)
	{
		VkPhysicalDevice physicalDevice = VulkanHolder::GetVulkanContext()->GetPhysicalDevice();
		
		vulkanWindowData->Surface = surface;

		// Check for WSI support
		VkBool32 result;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, VulkanHolder::GetVulkanContext()->FindQueueFamilies(physicalDevice).graphicsFamily.value(), vulkanWindowData->Surface, &result);
		KR_CORE_ASSERT(result == VK_TRUE, "No WSI support found on physical device");

		// Select Surface Format
		const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM, VK_FORMAT_B8G8R8A8_SRGB };
		const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		vulkanWindowData->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(physicalDevice, vulkanWindowData->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

		// Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
		VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
		VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
		vulkanWindowData->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(physicalDevice, vulkanWindowData->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));

		
		// A hacky way to let Dear ImGui deal with swapchain and all that
		// https://computergraphics.stackexchange.com/a/8910

		// Also Author's note: I shouldn't be using the functions ImGui_ImplVUlkanH_*(). Should be useful when attempting to bring ImGui Layer and ExampleLayer (with Cylinder mesh + material) 
		vulkanWindowData->Swapchain = VulkanHolder::GetVulkanContext()->GetSwapChain();
		
		// Create SwapChain, RenderPass, Framebuffer, CommandPool etc.
		ImGui_ImplVulkanH_CreateOrResizeWindow(VulkanHolder::GetVulkanContext()->GetInstance(), physicalDevice, m_Device, vulkanWindowData, VulkanHolder::GetVulkanContext()->FindQueueFamilies(physicalDevice).graphicsFamily.value(), VK_NULL_HANDLE, width, height, m_MinImageCount);
	}

	ImGuiLayer::~ImGuiLayer()
	{
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

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(m_AssociatedWindow->GetNativeWindow());//static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
		
		// Setup Platform/Renderer bindings
		if(RendererAPI::GetAPI() == RendererAPI::API::Vulkan)
		{
			// Exposing Karma's Vulkan components to Dear ImGui
			ImGui_ImplGlfw_InitForVulkan(window, true);
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = VulkanHolder::GetVulkanContext()->GetInstance();
			init_info.PhysicalDevice = VulkanHolder::GetVulkanContext()->GetPhysicalDevice();
			init_info.Device = m_Device;
			init_info.QueueFamily = VulkanHolder::GetVulkanContext()->FindQueueFamilies(init_info.PhysicalDevice).graphicsFamily.value();
			init_info.Queue = VulkanHolder::GetVulkanContext()->GetGraphicsQueue();// An inter-class communication
			init_info.DescriptorPool = m_ImGuiDescriptorPool;
			init_info.MinImageCount = m_MinImageCount;
			init_info.ImageCount = VulkanHolder::GetVulkanContext()->GetImageCount();
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			
			ImGui_ImplVulkan_Init(&init_info, m_VulkanWindowData.RenderPass);
			
			// Load default font
			ImFontConfig fontConfig;
			fontConfig.FontDataOwnedByAtlas = false;
			ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular, sizeof(g_RobotoRegular), 20.0f, &fontConfig);
			io.FontDefault = robotoFont;

			// Upload Fonts
			{
				// Use any command queue
				VkCommandPool command_pool = m_VulkanWindowData.Frames[m_VulkanWindowData.FrameIndex].CommandPool;
				VkCommandBuffer command_buffer = m_VulkanWindowData.Frames[m_VulkanWindowData.FrameIndex].CommandBuffer;

				VkResult result = vkResetCommandPool(m_Device, command_pool, 0);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to reset command pool!");
				
				VkCommandBufferBeginInfo begin_info = {};
				begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				result = vkBeginCommandBuffer(command_buffer, &begin_info);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to begin recording(?) command buffer!");

				ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
				
				VkSubmitInfo end_info = {};
				end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				end_info.commandBufferCount = 1;
				end_info.pCommandBuffers = &command_buffer;
				result = vkEndCommandBuffer(command_buffer);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to end recording(?) command buffer!");
				
				result = vkQueueSubmit(VulkanHolder::GetVulkanContext()->GetGraphicsQueue(), 1, &end_info, VK_NULL_HANDLE);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to submit command buffer!");
				
				result = vkDeviceWaitIdle(m_Device);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to wait!");
				
				ImGui_ImplVulkan_DestroyFontUploadObjects();
			}
		}
		else if (RendererAPI::GetAPI() == RendererAPI::API::OpenGL)
		{
			ImGui_ImplGlfw_InitForOpenGL(window, true);
			ImGui_ImplOpenGL3_Init("#version 410");
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
				ImGui_ImplOpenGL3_Shutdown();
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

	void ImGuiLayer::Begin()
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::Vulkan:
				GiveLoopBeginControlToVulkan();
				break;
			case RendererAPI::API::OpenGL:
				ImGui_ImplOpenGL3_NewFrame();
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

	void ImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());
		
		// Rendering
		ImGui::Render();
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::Vulkan:
				GiveLoopEndControlToVulkan();
				break;
			case RendererAPI::API::OpenGL:
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
				break;
			case RendererAPI::API::None:
				KR_CORE_ASSERT(false, "RendererAPI::None is not supported");
				break;
			default:
				KR_CORE_ASSERT(false, "Unknown RendererAPI {0} is in play.")
				break;
		}
		
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
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
				ImGui_ImplVulkan_SetMinImageCount(m_MinImageCount);
				VkPhysicalDevice physicalDevice = VulkanHolder::GetVulkanContext()->GetPhysicalDevice();
				ImGui_ImplVulkanH_CreateOrResizeWindow(VulkanHolder::GetVulkanContext()->GetInstance(), physicalDevice, m_Device, &m_VulkanWindowData, VulkanHolder::GetVulkanContext()->FindQueueFamilies(physicalDevice).graphicsFamily.value(), VK_NULL_HANDLE, width, height, m_MinImageCount);
				m_VulkanWindowData.FrameIndex = 0;
				m_SwapChainRebuild = false;
			}
		}
		
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
	}

	void ImGuiLayer::OnImGuiRender()
	{
		// 1. Show the big demo window. For debug purpose!!
		static bool show = true;
		ImGui::ShowDemoWindow(&show);
		
		// 2. Something that I don't fully understand, but relevant to demo window docking mechanism maybe
		{
			static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

			// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
			// because it would be confusing to have two docking targets within each others.
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
			if (m_MenubarCallback)
				window_flags |= ImGuiWindowFlags_MenuBar;

			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
			// and handle the pass-thru hole, so we ask Begin() to not render a background.
			if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
				window_flags |= ImGuiWindowFlags_NoBackground;

			// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
			// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
			// all active windows docked into it will lose their parent and become undocked.
			// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
			// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("DockSpace Demo", nullptr, window_flags);
			ImGui::PopStyleVar();

			ImGui::PopStyleVar(2);

			// Submit the DockSpace
			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				ImGuiID dockspace_id = ImGui::GetID("VulkanAppDockspace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			}

			if (m_MenubarCallback)
			{
				if (ImGui::BeginMenuBar())
				{
					m_MenubarCallback();
					ImGui::EndMenuBar();
				}
			}

			ImGui::End();
		}
	}

	void ImGuiLayer::GiveLoopEndControlToVulkan()
	{
		// Rendering
		ImGui::Render();
		ImDrawData* main_draw_data = ImGui::GetDrawData();
		const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
		
		glm::vec4 clear_color = RenderCommand::GetClearColor();
		
		m_VulkanWindowData.ClearValue.color.float32[0] = clear_color.x * clear_color.w;
		m_VulkanWindowData.ClearValue.color.float32[1] = clear_color.y * clear_color.w;
		m_VulkanWindowData.ClearValue.color.float32[2] = clear_color.z * clear_color.w;
		m_VulkanWindowData.ClearValue.color.float32[3] = clear_color.w;
		
		if (!main_is_minimized)
			FrameRender(&m_VulkanWindowData, main_draw_data);
		
		// Update and Render additional Platform Windows
		/*
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
		*/
		// Present Main Platform Window
		if (!main_is_minimized)
			FramePresent(&m_VulkanWindowData);
	}

	// Helper taken from https://github.com/TheCherno/Walnut/blob/cc26ee1cc875db50884fe244e0a3195dd730a1ef/Walnut/src/Walnut/Application.cpp#L270 who probably took help from official example https://github.com/ravimohan1991/imgui/blob/cf070488c71be01a04498e8eb50d66b982c7af9b/examples/example_glfw_vulkan/main.cpp#L261, with chiefly naming modifications.
	void ImGuiLayer::FrameRender(ImGui_ImplVulkanH_Window* windowData, ImDrawData* draw_data)
	{
		VkResult result;

		VkSemaphore image_acquired_semaphore = windowData->FrameSemaphores[windowData->SemaphoreIndex].ImageAcquiredSemaphore;
		VkSemaphore render_complete_semaphore = windowData->FrameSemaphores[windowData->SemaphoreIndex].RenderCompleteSemaphore;
		result = vkAcquireNextImageKHR(m_Device, windowData->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &windowData->FrameIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			m_SwapChainRebuild = true;
			return;
		}
		
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to acquire next image from swapchain");

		m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_VulkanWindowData.ImageCount;

		ImGui_ImplVulkanH_Frame* frameData = &windowData->Frames[windowData->FrameIndex];
		{
			result = vkWaitForFences(m_Device, 1, &frameData->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
			
			// Little strange to check for vkWaitForFences something we didn't do in Vulkan renderer
			// https://github.com/ravimohan1991/KarmaEngine/blob/d718f6ede15770890de5d00a45cc07fef39652fd/Karma/src/Platform/Vulkan/VulkanRendererAPI.cpp#L161
			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to wait");

			result = vkResetFences(m_Device, 1, &frameData->Fence);
			
			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to reset fence");
		}

		{
			// Free resources in queue
			for (auto& func : m_ResourceFreeQueue[m_CurrentFrameIndex])
			func();
			m_ResourceFreeQueue[m_CurrentFrameIndex].clear();
		}
		
		{
			// Free command buffers allocated by ImGuiLayer::AllocateImGuiCommandBuffers
			// These use m_VulkanWindowData.FrameIndex and not m_CurrentFrameIndex because they're tied to the swapchain image index. Ponder ...
			auto& allocatedCommandBuffers = m_AllocatedCommandBuffers[windowData->FrameIndex];
			if (allocatedCommandBuffers.size() > 0)
			{
				vkFreeCommandBuffers(m_Device, frameData->CommandPool, (uint32_t)allocatedCommandBuffers.size(), allocatedCommandBuffers.data());
				allocatedCommandBuffers.clear();
			}

			result = vkResetCommandPool(m_Device, frameData->CommandPool, 0);
			
			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to reset command pool");
			
			VkCommandBufferBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			result = vkBeginCommandBuffer(frameData->CommandBuffer, &info);
			
			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to begin command buffer");
		}
		
		{
			VkRenderPassBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.renderPass = windowData->RenderPass;
			info.framebuffer = frameData->Framebuffer;
			info.renderArea.extent.width = windowData->Width;
			info.renderArea.extent.height = windowData->Height;
			info.clearValueCount = 1;
			info.pClearValues = &windowData->ClearValue;
			vkCmdBeginRenderPass(frameData->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
		}

		// Record dear imgui primitives into command buffer
		ImGui_ImplVulkan_RenderDrawData(draw_data, frameData->CommandBuffer);

		// Submit command buffer
		vkCmdEndRenderPass(frameData->CommandBuffer);
		{
			VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			VkSubmitInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			info.waitSemaphoreCount = 1;
			info.pWaitSemaphores = &image_acquired_semaphore;
			info.pWaitDstStageMask = &wait_stage;
			info.commandBufferCount = 1;
			info.pCommandBuffers = &frameData->CommandBuffer;
			info.signalSemaphoreCount = 1;
			info.pSignalSemaphores = &render_complete_semaphore;
			result = vkEndCommandBuffer(frameData->CommandBuffer);
			
			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to end command buffer");
			
			result = vkQueueSubmit(VulkanHolder::GetVulkanContext()->GetGraphicsQueue(), 1, &info, frameData->Fence);
			
			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to submit queue");
		}
	}

	void ImGuiLayer::FramePresent(ImGui_ImplVulkanH_Window* windowData)
	{
		if (m_SwapChainRebuild)
			return;
			
		VkSemaphore render_complete_semaphore = windowData->FrameSemaphores[windowData->SemaphoreIndex].RenderCompleteSemaphore;
		VkPresentInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &render_complete_semaphore;
		info.swapchainCount = 1;
		info.pSwapchains = &windowData->Swapchain;
		info.pImageIndices = &windowData->FrameIndex;
		VkResult result = vkQueuePresentKHR(VulkanHolder::GetVulkanContext()->GetGraphicsQueue(), &info);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			m_SwapChainRebuild = true;
			return;
		}
		
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to submit queue");
		
		windowData->SemaphoreIndex = (windowData->SemaphoreIndex + 1) % windowData->ImageCount; // Now we can use the next set of semaphores
	}

	void ImGuiLayer::GracefulVulkanShutDown()
	{
		VkResult result = vkDeviceWaitIdle(m_Device);
		
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to wait for the completion of command buffers");

		// Free resources in queue
		for (auto& queue : m_ResourceFreeQueue)
		{
			for (auto& func : queue)
				func();
		}
		m_ResourceFreeQueue.clear();
		
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		
		CleanUpVulkanAndWindowData();
	}

	void ImGuiLayer::CleanUpVulkanAndWindowData()
	{
		// Clean up Window
		ImGui_ImplVulkanH_DestroyWindow(VulkanHolder::GetVulkanContext()->GetInstance(), m_Device, &m_VulkanWindowData, VK_NULL_HANDLE);
		
		// Clean up Vulkan's pool component instantiated earlier here
		vkDestroyDescriptorPool(m_Device, m_ImGuiDescriptorPool, VK_NULL_HANDLE);
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
		io.DisplaySize = ImVec2(e.GetWidth(), e.GetHeight());
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
		
		return false;
	}
}
