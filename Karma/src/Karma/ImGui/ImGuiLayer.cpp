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
			// Cache the VkDevice and rest of the information from context already created
			m_Device = VulkanHolder::GetVulkanContext()->GetLogicalDevice();
			m_Instance = VulkanHolder::GetVulkanContext()->GetInstance();

			// Create descriptor pool for IMGUI (code taken from from imgui demo)
			// Compare with the VulkanContext pool
			CreateDescriptorPool();

			// Need this else get confronted by assertion in imgui_impl_vulkan.cpp
			// "Need to call ImGui_ImplVulkan_LoadFunctions() if IMGUI_IMPL_VULKAN_NO_PROTOTYPES or VK_NO_PROTOTYPES are set!"
			ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void*) { return vkGetInstanceProcAddr(VulkanHolder::GetVulkanContext()->GetInstance(), function_name); });

			// Curate the data associated with Window context and VulkanAPI
			int width, height;
			GLFWwindow* window = static_cast<GLFWwindow*>(m_AssociatedWindow->GetNativeWindow());

			glfwGetFramebufferSize(window, &width, &height);
			GatherVulkanWindowData(&m_VulkanWindowData, width, height);
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
		pool_info.poolSizeCount = uint32_t (std::size(pool_sizes));
		pool_info.pPoolSizes = pool_sizes;

		VkResult result = vkCreateDescriptorPool(m_Device, &pool_info, nullptr, &m_ImGuiDescriptorPool);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create descriptor pool for ImGui");
	}

	// Here we fill up the Vulkan relevant fields associated with GLFW window and Context created earlier
	void ImGuiLayer::GatherVulkanWindowData(ImGui_ImplVulkanH_Window* vulkanWindowData, int width, int height)
	{
		vulkanWindowData->Surface = VulkanHolder::GetVulkanContext()->GetSurface();

		// Fetch the image count, format, and mode from VulkanContext
		vulkanWindowData->ImageCount = VulkanHolder::GetVulkanContext()->GetSwapChainImages().size();
		vulkanWindowData->SurfaceFormat = VulkanHolder::GetVulkanContext()->GetSurfaceFormat();
		vulkanWindowData->PresentMode = VulkanHolder::GetVulkanContext()->GetPresentMode();

		// Start with nullptr for these
		vulkanWindowData->Frames = nullptr;
		vulkanWindowData->FramesOnFlight = nullptr;

		// https://computergraphics.stackexchange.com/a/8910
		// Author's note: I shouldn't be using the functions ImGui_ImplVUlkanH_*(). Should be useful when attempting to bring ImGui Layer and ExampleLayer (with Cylinder mesh + material)
		// vulkanWindowData->Swapchain = VulkanHolder::GetVulkanContext()->GetSwapChain();

		// Share SwapChain, RenderPass, Framebuffer, CommandPool, and Semaphores & Fence etc.
		ShareVulkanContextOfMainWindow(vulkanWindowData, true);
	}

	ImGuiLayer::~ImGuiLayer()
	{
	}

	// ImGui (ImGui_ImplVulkanH_CreateOrResizeWindow) equivalent
	void ImGuiLayer::ShareVulkanContextOfMainWindow(ImGui_ImplVulkanH_Window* windowData, bool bCreateSyncronicity)
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

		// So in order to do that, we need to do some reinterpretation of the data structure ImGui_ImplVulkanH_Frame so that ImGui's unofficial support may
		// conform to Vulkan's official instructions.

		// We instantiate the Frames with same number as SwapChainImages number and give an interpretational label ImageFrame
		KR_CORE_ASSERT(windowData->Frames == nullptr, "Somehow frames are still occupied. Please clear them.");
		windowData->Frames = new ImGui_ImplVulkanH_Frame[windowData->ImageCount];

		for(uint32_t counter = 0; counter < windowData->ImageCount; counter++)
		{
			ImGui_ImplVulkanH_Frame* frameData = &windowData->Frames[counter];

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

	void ImGuiLayer::ClearVulkanWindowData(ImGui_ImplVulkanH_Window* vulkanWindowData, bool bDestroySyncronicity)
	{
		// We are assuming that VulkanRendererAPI::RecreateCommandBuffersPipelineSwapchain()
		// has been called, thereby rendering the Swapchain handle of vulkanWindowData redundant.
		vulkanWindowData->Swapchain = VK_NULL_HANDLE;

		// We won't be needing to wait, because VulkanRendererAPI should take care of the waiting
		//vkDeviceWaitIdle(m_Device);

		for (uint32_t i = 0; i < vulkanWindowData->ImageCount; i++)
		{
			if(vulkanWindowData->Frames != nullptr)
			{
				DestroyWindowDataFrame(&vulkanWindowData->Frames[i]);
			}
			if(vulkanWindowData->FramesOnFlight != nullptr && bDestroySyncronicity)
			{
				// Remove syncronicity resources using Vulkan API
				DestroyFramesOnFlightData(&vulkanWindowData->FramesOnFlight[i]);
			}
		}

		if(vulkanWindowData->Frames != nullptr)
		{
			delete[] vulkanWindowData->Frames;
			vulkanWindowData->Frames = nullptr;
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

	void ImGuiLayer::DestroyWindowDataFrame(ImGui_ImplVulkanH_Frame* frame)
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
			init_info.Instance = m_Instance;
			init_info.PhysicalDevice = VulkanHolder::GetVulkanContext()->GetPhysicalDevice();
			init_info.Device = m_Device;
			init_info.QueueFamily = VulkanHolder::GetVulkanContext()->FindQueueFamilies(init_info.PhysicalDevice).graphicsFamily.value();
			init_info.Queue = VulkanHolder::GetVulkanContext()->GetGraphicsQueue();// An inter-class communication
			init_info.DescriptorPool = m_ImGuiDescriptorPool;
			init_info.MinImageCount = VulkanHolder::GetVulkanContext()->GetMinImageCount();
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
				//ImGui_ImplVulkan_SetMinImageCount(m_MinImageCount);
				ShareVulkanContextOfMainWindow(&m_VulkanWindowData);
				//m_VulkanWindowData.FrameIndex = 0;
				//m_SwapChainRebuild = false;
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

		vkDeviceWaitIdle(m_Device);
		for (size_t i = 0; i < m_VulkanWindowData.ImageCount; i++)
		{
			vkResetCommandBuffer(m_VulkanWindowData.Frames[i].CommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		}
	}

	// Helper taken from https://github.com/TheCherno/Walnut/blob/cc26ee1cc875db50884fe244e0a3195dd730a1ef/Walnut/src/Walnut/Application.cpp#L270 who probably took help from official example https://github.com/ravimohan1991/imgui/blob/cf070488c71be01a04498e8eb50d66b982c7af9b/examples/example_glfw_vulkan/main.cpp#L261, with chiefly naming modifications.
	void ImGuiLayer::FrameRender(ImGui_ImplVulkanH_Window* windowData, ImDrawData* draw_data)
	{
		VkResult result;

		{
			result = vkWaitForFences(m_Device, 1, &windowData->FramesOnFlight[m_CurrentFrame].Fence, VK_TRUE, UINT64_MAX);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to wait");
		}

		VkSemaphore image_acquired_semaphore = windowData->FramesOnFlight[m_CurrentFrame].ImageAcquiredSemaphore;
		VkSemaphore render_complete_semaphore = windowData->FramesOnFlight[m_CurrentFrame].RenderCompleteSemaphore;

		result = vkAcquireNextImageKHR(m_Device, windowData->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			m_SwapChainRebuild = true;
			return;
		}

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to acquire next image from swapchain");

		// Record begins:
		for(uint32_t imageCounter = 0; imageCounter < windowData->ImageCount; imageCounter++)
		{
			ImGui_ImplVulkanH_Frame* frameData = &windowData->Frames[imageCounter];
			
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
					ImGui_ImplVulkan_RenderDrawData(draw_data, frameData->CommandBuffer);
				
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
			info.pWaitSemaphores = &image_acquired_semaphore;
			info.pWaitDstStageMask = &wait_stage;
			info.commandBufferCount = 1;
			info.pCommandBuffers = &(windowData->Frames[imageIndex].CommandBuffer);
			info.signalSemaphoreCount = 1;
			info.pSignalSemaphores = &render_complete_semaphore;

			result = vkResetFences(m_Device, 1, &windowData->FramesOnFlight[m_CurrentFrame].Fence);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to reset fence");
			
			result = vkQueueSubmit(VulkanHolder::GetVulkanContext()->GetGraphicsQueue(), 1, &info, windowData->FramesOnFlight[m_CurrentFrame].Fence);

			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to submit queue");
		}
	}

	void ImGuiLayer::FramePresent(ImGui_ImplVulkanH_Window* windowData)
	{
		VkSemaphore render_complete_semaphore = windowData->FramesOnFlight[m_CurrentFrame].RenderCompleteSemaphore;

		VkPresentInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &render_complete_semaphore;
		info.swapchainCount = 1;
		info.pSwapchains = &windowData->Swapchain;
		info.pImageIndices = &imageIndex;
		VkResult result = vkQueuePresentKHR(VulkanHolder::GetVulkanContext()->GetGraphicsQueue(), &info);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			m_SwapChainRebuild = true;
			return;
		}

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to submit queue");

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT; // Now we can use the next set of semaphores
	}

	void ImGuiLayer::GracefulVulkanShutDown()
	{
		VkResult result = vkDeviceWaitIdle(m_Device);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to wait for the completion of command buffers");

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		CleanUpVulkanAndWindowData();
	}

	void ImGuiLayer::CleanUpVulkanAndWindowData()
	{
		// Clean up Window
		ClearVulkanWindowData(&m_VulkanWindowData, true);
		//ImGui_ImplVulkanH_DestroyWindow(m_Instance, m_Device, &m_VulkanWindowData, VK_NULL_HANDLE);

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
		io.DisplaySize = ImVec2(float (e.GetWidth()), float (e.GetHeight()));
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

		return false;
	}
}
