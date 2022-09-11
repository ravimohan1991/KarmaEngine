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
			CreateDescriptorPool();

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
		poolInfo.poolSizeCount = uint32_t (std::size(pool_sizes));
		poolInfo.pPoolSizes = pool_sizes;

		VkResult result = vkCreateDescriptorPool(VulkanHolder::GetVulkanContext()->GetLogicalDevice(), &poolInfo, nullptr, &m_ImGuiDescriptorPool);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create descriptor pool for ImGui");
	}

	// Here we fill up the Vulkan relevant fields associated with GLFW window and Context created earlier
	/*void ImGuiLayer::GatherVulkanWindowData(ImGui_KarmaImplVulkanH_Window* vulkanWindowData, int width, int height)
	{
		vulkanWindowData->Surface = VulkanHolder::GetVulkanContext()->GetSurface();

		// Fetch the image count, format, and mode from VulkanContext
		vulkanWindowData->ImageCount = VulkanHolder::GetVulkanContext()->GetSwapChainImages().size();
		vulkanWindowData->SurfaceFormat = VulkanHolder::GetVulkanContext()->GetSurfaceFormat();
		vulkanWindowData->PresentMode = VulkanHolder::GetVulkanContext()->GetPresentMode();

		// Start with nullptr for these
		vulkanWindowData->ImageFrames = nullptr;
		vulkanWindowData->FramesOnFlight = nullptr;

		// https://computergraphics.stackexchange.com/a/8910
		// Author's note: I shouldn't be using the functions ImGui_KarmaImplVulkanH_*(). Should be useful when attempting to bring ImGui Layer and ExampleLayer (with Cylinder mesh + material)
		// vulkanWindowData->Swapchain = VulkanHolder::GetVulkanContext()->GetSwapChain();

		// Share SwapChain, RenderPass, Framebuffer, CommandPool, and Semaphores & Fence etc.
		ShareVulkanContextOfMainWindow(vulkanWindowData, true);
	}*/

	ImGuiLayer::~ImGuiLayer()
	{
	}

	// ImGui (ImGui_KarmaImplVulkanH_CreateOrResizeWindow) equivalent
	/*void ImGuiLayer::ShareVulkanContextOfMainWindow(ImGui_KarmaImplVulkanH_Window* windowData, bool bCreateSyncronicity)
	{
		// Clear the structure with now redundant information
		ClearVulkanWindowData(windowData, bCreateSyncronicity);

		// Fill relevant information
		// Assuming new swapchain and all that has been created
		windowData->Swapchain = VulkanHolder::GetVulkanContext()->GetSwapChain();
		windowData->ImageCount = VulkanHolder::GetVulkanContext()->GetSwapChainImages().size();
		windowData->RenderArea.extent = VulkanHolder::GetVulkanContext()->GetSwapChainExtent();
		windowData->RenderArea.offset = { 0, 0 };

		// For the render pass
		// We may need to consider the the notion of depthAttachment that we wrote in the VulkanContext which is not present in ImGui's take
		{
			VkAttachmentDescription attachment = {};
			attachment.format = windowData->SurfaceFormat.format;
			attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment.loadOp = windowData->ClearEnable ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			VkAttachmentReference color_attachment = {};
			color_attachment.attachment = 0;
			color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &color_attachment;
			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			VkRenderPassCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			info.attachmentCount = 1;
			info.pAttachments = &attachment;
			info.subpassCount = 1;
			info.pSubpasses = &subpass;
			info.dependencyCount = 1;
			info.pDependencies = &dependency;
			//VkResult result = vkCreateRenderPass(m_Device, &info, VK_NULL_HANDLE, &windowData->RenderPass);
			//KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create renderpass for ImGui");
		}
		windowData->RenderPass = VulkanHolder::GetVulkanContext()->GetRenderPass();

		RendererAPI* rAPI = RenderCommand::GetRendererAPI();
		VulkanRendererAPI* vulkanAPI = nullptr;

		if(rAPI->GetAPI() == RendererAPI::API::Vulkan)
		{
			vulkanAPI = static_cast<VulkanRendererAPI*>(rAPI);
		}
		else
		{
			KR_CORE_ASSERT(false, "How is this even possible?");
		}

		KR_CORE_ASSERT(vulkanAPI != nullptr, "Casting to VulkanAPI failed");

		MAX_FRAMES_IN_FLIGHT = vulkanAPI->GetMaxFramesInFlight();

		// Recreate the structure with right amount of number
		// Cowboy's Note: Seems like ImGui guys have confused notion of ImageCount (deciding number of SwapChainImages, framebuffer & commandbuffer size, and so on)
		// (https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Retrieving-the-swap-chain-images)
		// and MAX_FRAMES_IN_FLIGHT, which is representative of (linearly proportional to or indicative of) number of commandbuffer recordings on CPU that may happen
		// whilst the rendering is being done on GPU. That should determine the semaphore and fence size.
		// https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Frames_in_flight
		// The argument is elicited by the comment line https://github.com/ravimohan1991/imgui/blob/e4967701b67edd491e884632f239ab1f38867d86/backends/imgui_impl_vulkan.h#L144
		// I shall continue with the official tutorial and leave the manipulation of numbers to, well, my later self.

		// So in order to do that, we need to do some reinterpretation of the data structure ImGui_KarmaImplVulkanH_ImageFrame so that ImGui's unofficial support may
		// conform to Vulkan's official instructions.

		// We instantiate the Frames with same number as SwapChainImages number and give an interpretational label ImageFrame
		KR_CORE_ASSERT(windowData->ImageFrames == nullptr, "Somehow frames are still occupied. Please clear them.");
		windowData->ImageFrames = new ImGui_KarmaImplVulkanH_ImageFrame[windowData->ImageCount];

		for(uint32_t counter = 0; counter < windowData->ImageCount; counter++)
		{
			ImGui_KarmaImplVulkanH_ImageFrame* frameData = &windowData->ImageFrames[counter];

			// VulkanContext ImageView equivalent
			frameData->BackbufferView = VulkanHolder::GetVulkanContext()->GetSwapChainImageViews()[counter];

			// Framebuffer
			frameData->Framebuffer = VulkanHolder::GetVulkanContext()->GetSwapChainFrameBuffer()[counter];

			// Backbuffers could be VulkanContext m_swapChainImages equivalent
			frameData->Backbuffer = VulkanHolder::GetVulkanContext()->GetSwapChainImages()[counter];

			// Commandpool
			frameData->CommandPool = VulkanHolder::GetVulkanContext()->GetCommandPool();

			// Allotted in my implementation of VulkanAPI
			if(m_SwapChainRebuild)
			{
				vulkanAPI->AllocateCommandBuffers();
			}
			frameData->CommandBuffer = vulkanAPI->GetCommandBuffers()[counter];
		}

		// We create seperate syncronicity resources for Dear ImGui
		if(bCreateSyncronicity)
		{
			// For syncronicity, we instantiate FramesOnFlight with MAX_FRAMES_IN_FLIGHT number and label them RealFrameInFlight
			KR_CORE_ASSERT(windowData->FramesOnFlight == nullptr, "Somehow frames-on-flight are still occupied. Please clear them.");
			windowData->FramesOnFlight = new ImGui_Vulkan_Frame_On_Flight[MAX_FRAMES_IN_FLIGHT];

			for(uint32_t counter = 0; counter < MAX_FRAMES_IN_FLIGHT; counter++)
			{
				ImGui_Vulkan_Frame_On_Flight* frameOnFlight = &windowData->FramesOnFlight[counter];

				VkFenceCreateInfo fenceInfo = {};
				fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
				VkResult result = vkCreateFence(m_Device, &fenceInfo, VK_NULL_HANDLE, &frameOnFlight->Fence);

				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create fence");

				VkSemaphoreCreateInfo semaphoreInfo = {};
				semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
				
				result = vkCreateSemaphore(m_Device, &semaphoreInfo, VK_NULL_HANDLE, &frameOnFlight->ImageAcquiredSemaphore);

				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create ImageAcquiredSemaphore");

				result = vkCreateSemaphore(m_Device, &semaphoreInfo, VK_NULL_HANDLE, &frameOnFlight->RenderCompleteSemaphore);

				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create RenderCompleteSemaphore");
			}
		}
	}

	void ImGuiLayer::ClearVulkanWindowData(ImGui_KarmaImplVulkanH_Window* vulkanWindowData, bool bDestroySyncronicity)
	{
		// We are assuming that VulkanRendererAPI::RecreateCommandBuffersPipelineSwapchain()
		// has been called, thereby rendering the Swapchain handle of vulkanWindowData redundant.
		vulkanWindowData->Swapchain = VK_NULL_HANDLE;

		// We won't be needing to wait, because VulkanRendererAPI should take care of the waiting
		//vkDeviceWaitIdle(m_Device);

		for (uint32_t i = 0; i < vulkanWindowData->ImageCount; i++)
		{
			if(vulkanWindowData->ImageFrames != nullptr)
			{
				DestroyWindowDataFrame(&vulkanWindowData->ImageFrames[i]);
			}
			if(vulkanWindowData->FramesOnFlight != nullptr && bDestroySyncronicity)
			{
				// Remove syncronicity resources using Vulkan API
				DestroyFramesOnFlightData(&vulkanWindowData->FramesOnFlight[i]);
			}
		}

		if(vulkanWindowData->ImageFrames != nullptr)
		{
			delete[] vulkanWindowData->ImageFrames;
			vulkanWindowData->ImageFrames = nullptr;
		}

		if(bDestroySyncronicity && vulkanWindowData->FramesOnFlight != nullptr)
		{
			delete[] vulkanWindowData->FramesOnFlight;
			vulkanWindowData->FramesOnFlight = nullptr;
		}
		
		vulkanWindowData->ImageCount = 0;
		
		if(vulkanWindowData->RenderPass)
		{
			vulkanWindowData->RenderPass = VK_NULL_HANDLE;
		}
	}

	void ImGuiLayer::DestroyWindowDataFrame(ImGui_KarmaImplVulkanH_ImageFrame* frame)
	{
		if(frame == nullptr)
		{
			return;
		}

		frame->CommandBuffer = VK_NULL_HANDLE;
		frame->CommandPool = VK_NULL_HANDLE;

		frame->Backbuffer = VK_NULL_HANDLE;
		frame->Framebuffer = VK_NULL_HANDLE;
	}

	void ImGuiLayer::DestroyFramesOnFlightData(ImGui_Vulkan_Frame_On_Flight* frameSyncronicityData)
	{
		if(frameSyncronicityData == nullptr)
		{
			return;
		}

		vkDeviceWaitIdle(m_Device);
		
		vkDestroyFence(m_Device, frameSyncronicityData->Fence, VK_NULL_HANDLE);
		frameSyncronicityData->Fence = VK_NULL_HANDLE;

		vkDestroySemaphore(m_Device, frameSyncronicityData->ImageAcquiredSemaphore, VK_NULL_HANDLE);
		vkDestroySemaphore(m_Device, frameSyncronicityData->RenderCompleteSemaphore, VK_NULL_HANDLE);
		frameSyncronicityData->ImageAcquiredSemaphore = frameSyncronicityData->RenderCompleteSemaphore = VK_NULL_HANDLE;
	}*/

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
			ImGuiVulkanHandler::ShareVulkanContextResourcesOfMainWindow(&m_VulkanWindowData, true, false);

			// Load default font
			ImFontConfig fontConfig;
			fontConfig.FontDataOwnedByAtlas = false;
			ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular, sizeof(g_RobotoRegular), 20.0f, &fontConfig);
			io.FontDefault = robotoFont;

			// Upload Fonts
			{
				// Use any command queue
				VkCommandPool commandPool = m_VulkanWindowData.ImageFrames[m_VulkanWindowData.ImageFrameIndex].CommandPool;
				VkCommandBuffer commandBuffer = m_VulkanWindowData.ImageFrames[m_VulkanWindowData.ImageFrameIndex].CommandBuffer;

				// Hehe, out-of-trendy way!!!
				VkResult result = vkResetCommandPool(initInfo.Device, commandPool, 0);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to reset command pool!");

				VkCommandBufferBeginInfo beginInfo = {};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to begin recording(?) command buffer!");

				ImGuiVulkanHandler::ImGui_KarmaImplVulkan_CreateFontsTexture(commandBuffer);

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
				ImGuiVulkanHandler::ShareVulkanContextResourcesOfMainWindow(&m_VulkanWindowData, false, true);
				m_SwapChainRebuild = false;
			}
		}

		ImGuiVulkanHandler::ImGui_KarmaImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
	}

	void ImGuiLayer::OnImGuiRender()
	{
		// 1. Show the big demo window. For debug purpose!!
		static bool show = true;
		ImGui::ShowDemoWindow(&show);

		// 2. Something that I don't fully understand, but relevant to demo window docking mechanism maybe
		/*
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
		}*/
		
		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;
			
			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and appeninto it
			
			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show);                  // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show);
			
			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a colo
			
			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets returtrue when edited/activated)
				counter++;
			ImGui::SameLine();
			
			ImGui::Text("counter = %d", counter);
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
	}

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
		/*
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
		*/
		// Present Main Platform Window
		if (!mainIsMinimized)
			FramePresent(&m_VulkanWindowData);

		ImGui_KarmaImplVulkan_Data* backendData = ImGuiVulkanHandler::ImGui_KarmaImplVulkan_GetBackendData();
		ImGui_KarmaImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		vkDeviceWaitIdle(vulkanInfo->Device);

		for (size_t i = 0; i < m_VulkanWindowData.TotalImageCount; i++)
		{
			vkResetCommandBuffer(m_VulkanWindowData.ImageFrames[i].CommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		}
	}

	// Helper taken from https://github.com/TheCherno/Walnut/blob/cc26ee1cc875db50884fe244e0a3195dd730a1ef/Walnut/src/Walnut/Application.cpp#L270 who probably took help from official example https://github.com/ravimohan1991/imgui/blob/cf070488c71be01a04498e8eb50d66b982c7af9b/examples/example_glfw_vulkan/main.cpp#L261, with chiefly naming modifications.
	void ImGuiLayer::FrameRender(ImGui_KarmaImplVulkanH_Window* windowData, ImDrawData* drawData)
	{
		ImGui_KarmaImplVulkan_Data* backendData = ImGuiVulkanHandler::ImGui_KarmaImplVulkan_GetBackendData();
		ImGui_KarmaImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;
		VkResult result;

		{
			result = vkWaitForFences(vulkanInfo->Device, 1, &windowData->FramesOnFlight[windowData->SemaphoreIndex].Fence, VK_TRUE, UINT64_MAX);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to wait");
			//ImGuiVulkanHandler::ImGui_KarmaImplVulkan_ClearUndFreeResources(drawData);
		}

		VkSemaphore imageAcquiredSemaphore = windowData->FramesOnFlight[windowData->SemaphoreIndex].ImageAcquiredSemaphore;
		VkSemaphore renderCompleteSemaphore = windowData->FramesOnFlight[windowData->SemaphoreIndex].RenderCompleteSemaphore;

		result = vkAcquireNextImageKHR(vulkanInfo->Device, windowData->Swapchain, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE, &windowData->ImageFrameIndex);
		
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			m_SwapChainRebuild = true;
			return;
		}

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to acquire next image from swapchain");

		// Record begins:
		for(uint32_t imageCounter = 0; imageCounter < windowData->TotalImageCount; imageCounter++)
		{
			ImGui_KarmaImplVulkanH_ImageFrame* frameData = &windowData->ImageFrames[imageCounter];
			
			//result = vkResetCommandPool(m_Device, frameData->CommandPool, 0);
	
			//KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to reset command pool");
	
			VkCommandBufferBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
				
			result = vkBeginCommandBuffer(frameData->CommandBuffer, &info);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to begin command buffer");
			
				// Render Pass
				
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
	
				vkCmdBeginRenderPass(frameData->CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
				
				{
					// Record dear imgui primitives into command buffer
					ImGuiVulkanHandler::ImGui_KarmaImplVulkan_RenderDrawData(drawData, frameData->CommandBuffer, VK_NULL_HANDLE, windowData->ImageFrameIndex);
				}
				vkCmdEndRenderPass(frameData->CommandBuffer);
			
			result = vkEndCommandBuffer(frameData->CommandBuffer);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to end command buffer");
		}
		// Recording ends:
		
		// Submit command buffer
		{
			VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			VkSubmitInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			info.waitSemaphoreCount = 1;
			info.pWaitSemaphores = &imageAcquiredSemaphore;
			info.pWaitDstStageMask = &wait_stage;
			info.commandBufferCount = 1;
			info.pCommandBuffers = &(windowData->ImageFrames[windowData->ImageFrameIndex].CommandBuffer);
			info.signalSemaphoreCount = 1;
			info.pSignalSemaphores = &renderCompleteSemaphore;

			result = vkResetFences(vulkanInfo->Device, 1, &windowData->FramesOnFlight[windowData->SemaphoreIndex].Fence);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to reset fence");
			
			result = vkQueueSubmit(vulkanInfo->Queue, 1, &info, windowData->FramesOnFlight[windowData->SemaphoreIndex].Fence);

			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to submit queue");
		}
	}

	void ImGuiLayer::FramePresent(ImGui_KarmaImplVulkanH_Window* windowData)
	{
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

		ImGuiVulkanHandler::ImGui_KarmaImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		CleanUpVulkanAndWindowData();
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
		io.DisplaySize = ImVec2(float (e.GetWidth()), float (e.GetHeight()));
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

		return false;
	}
}
