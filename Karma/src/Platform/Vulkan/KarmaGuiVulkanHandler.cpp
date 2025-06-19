#include "KarmaGuiVulkanHandler.h"
#include "Vulkan/VulkanHolder.h"
#include "Renderer/RenderCommand.h"
#include "KarmaUtilities.h"
#include "Platform/Vulkan/VulkanVertexArray.h"
#include "Karma/KarmaGui/KarmaGuiRenderer.h"

// Visual Studio warnings
/*#ifdef _MSC_VER
#pragma warning (disable: 4127) // condition expression is constant
#endif
*/
namespace Karma
{
	//-----------------------------------------------------------------------------
	// SHADERS
	//-----------------------------------------------------------------------------
	static uint32_t __glsl_shader_vert_spv[] =
	{
		0x07230203,0x00010000,0x00080001,0x0000002e,0x00000000,0x00020011,0x00000001,0x0006000b,
		0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
		0x000a000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000b,0x0000000f,0x00000015,
		0x0000001b,0x0000001c,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
		0x00000000,0x00030005,0x00000009,0x00000000,0x00050006,0x00000009,0x00000000,0x6f6c6f43,
		0x00000072,0x00040006,0x00000009,0x00000001,0x00005655,0x00030005,0x0000000b,0x0074754f,
		0x00040005,0x0000000f,0x6c6f4361,0x0000726f,0x00030005,0x00000015,0x00565561,0x00060005,
		0x00000019,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x00000019,0x00000000,
		0x505f6c67,0x7469736f,0x006e6f69,0x00030005,0x0000001b,0x00000000,0x00040005,0x0000001c,
		0x736f5061,0x00000000,0x00060005,0x0000001e,0x73755075,0x6e6f4368,0x6e617473,0x00000074,
		0x00050006,0x0000001e,0x00000000,0x61635375,0x0000656c,0x00060006,0x0000001e,0x00000001,
		0x61725475,0x616c736e,0x00006574,0x00030005,0x00000020,0x00006370,0x00040047,0x0000000b,
		0x0000001e,0x00000000,0x00040047,0x0000000f,0x0000001e,0x00000002,0x00040047,0x00000015,
		0x0000001e,0x00000001,0x00050048,0x00000019,0x00000000,0x0000000b,0x00000000,0x00030047,
		0x00000019,0x00000002,0x00040047,0x0000001c,0x0000001e,0x00000000,0x00050048,0x0000001e,
		0x00000000,0x00000023,0x00000000,0x00050048,0x0000001e,0x00000001,0x00000023,0x00000008,
		0x00030047,0x0000001e,0x00000002,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,
		0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040017,
		0x00000008,0x00000006,0x00000002,0x0004001e,0x00000009,0x00000007,0x00000008,0x00040020,
		0x0000000a,0x00000003,0x00000009,0x0004003b,0x0000000a,0x0000000b,0x00000003,0x00040015,
		0x0000000c,0x00000020,0x00000001,0x0004002b,0x0000000c,0x0000000d,0x00000000,0x00040020,
		0x0000000e,0x00000001,0x00000007,0x0004003b,0x0000000e,0x0000000f,0x00000001,0x00040020,
		0x00000011,0x00000003,0x00000007,0x0004002b,0x0000000c,0x00000013,0x00000001,0x00040020,
		0x00000014,0x00000001,0x00000008,0x0004003b,0x00000014,0x00000015,0x00000001,0x00040020,
		0x00000017,0x00000003,0x00000008,0x0003001e,0x00000019,0x00000007,0x00040020,0x0000001a,
		0x00000003,0x00000019,0x0004003b,0x0000001a,0x0000001b,0x00000003,0x0004003b,0x00000014,
		0x0000001c,0x00000001,0x0004001e,0x0000001e,0x00000008,0x00000008,0x00040020,0x0000001f,
		0x00000009,0x0000001e,0x0004003b,0x0000001f,0x00000020,0x00000009,0x00040020,0x00000021,
		0x00000009,0x00000008,0x0004002b,0x00000006,0x00000028,0x00000000,0x0004002b,0x00000006,
		0x00000029,0x3f800000,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,
		0x00000005,0x0004003d,0x00000007,0x00000010,0x0000000f,0x00050041,0x00000011,0x00000012,
		0x0000000b,0x0000000d,0x0003003e,0x00000012,0x00000010,0x0004003d,0x00000008,0x00000016,
		0x00000015,0x00050041,0x00000017,0x00000018,0x0000000b,0x00000013,0x0003003e,0x00000018,
		0x00000016,0x0004003d,0x00000008,0x0000001d,0x0000001c,0x00050041,0x00000021,0x00000022,
		0x00000020,0x0000000d,0x0004003d,0x00000008,0x00000023,0x00000022,0x00050085,0x00000008,
		0x00000024,0x0000001d,0x00000023,0x00050041,0x00000021,0x00000025,0x00000020,0x00000013,
		0x0004003d,0x00000008,0x00000026,0x00000025,0x00050081,0x00000008,0x00000027,0x00000024,
		0x00000026,0x00050051,0x00000006,0x0000002a,0x00000027,0x00000000,0x00050051,0x00000006,
		0x0000002b,0x00000027,0x00000001,0x00070050,0x00000007,0x0000002c,0x0000002a,0x0000002b,
		0x00000028,0x00000029,0x00050041,0x00000011,0x0000002d,0x0000001b,0x0000000d,0x0003003e,
		0x0000002d,0x0000002c,0x000100fd,0x00010038
	};

	static uint32_t __glsl_shader_frag_spv[] =
	{
		0x07230203,0x00010000,0x00080001,0x0000001e,0x00000000,0x00020011,0x00000001,0x0006000b,
		0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
		0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000d,0x00030010,
		0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
		0x00000000,0x00040005,0x00000009,0x6c6f4366,0x0000726f,0x00030005,0x0000000b,0x00000000,
		0x00050006,0x0000000b,0x00000000,0x6f6c6f43,0x00000072,0x00040006,0x0000000b,0x00000001,
		0x00005655,0x00030005,0x0000000d,0x00006e49,0x00050005,0x00000016,0x78655473,0x65727574,
		0x00000000,0x00040047,0x00000009,0x0000001e,0x00000000,0x00040047,0x0000000d,0x0000001e,
		0x00000000,0x00040047,0x00000016,0x00000022,0x00000000,0x00040047,0x00000016,0x00000021,
		0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,
		0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,
		0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040017,0x0000000a,0x00000006,
		0x00000002,0x0004001e,0x0000000b,0x00000007,0x0000000a,0x00040020,0x0000000c,0x00000001,
		0x0000000b,0x0004003b,0x0000000c,0x0000000d,0x00000001,0x00040015,0x0000000e,0x00000020,
		0x00000001,0x0004002b,0x0000000e,0x0000000f,0x00000000,0x00040020,0x00000010,0x00000001,
		0x00000007,0x00090019,0x00000013,0x00000006,0x00000001,0x00000000,0x00000000,0x00000000,
		0x00000001,0x00000000,0x0003001b,0x00000014,0x00000013,0x00040020,0x00000015,0x00000000,
		0x00000014,0x0004003b,0x00000015,0x00000016,0x00000000,0x0004002b,0x0000000e,0x00000018,
		0x00000001,0x00040020,0x00000019,0x00000001,0x0000000a,0x00050036,0x00000002,0x00000004,
		0x00000000,0x00000003,0x000200f8,0x00000005,0x00050041,0x00000010,0x00000011,0x0000000d,
		0x0000000f,0x0004003d,0x00000007,0x00000012,0x00000011,0x0004003d,0x00000014,0x00000017,
		0x00000016,0x00050041,0x00000019,0x0000001a,0x0000000d,0x00000018,0x0004003d,0x0000000a,
		0x0000001b,0x0000001a,0x00050057,0x00000007,0x0000001c,0x00000017,0x0000001b,0x00050085,
		0x00000007,0x0000001d,0x00000012,0x0000001c,0x0003003e,0x00000009,0x0000001d,0x000100fd,
		0x00010038
	};

	uint32_t KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_MemoryType(VkMemoryPropertyFlags properties, uint32_t type_bits)
	{
		KarmaGuiBackendRendererUserData* bd = KarmaGuiRenderer::GetBackendRendererUserData();
		KarmaGui_ImplVulkan_InitInfo* v = &bd->VulkanInitInfo;

		VkPhysicalDeviceMemoryProperties prop;
		vkGetPhysicalDeviceMemoryProperties(v->PhysicalDevice, &prop);

		for (uint32_t i = 0; i < prop.memoryTypeCount; i++)// prop.memoryTypeCount is the number of valid elements in the memoryTypes (VkMemoryType) array.
			if ((prop.memoryTypes[i].propertyFlags & properties) == properties && type_bits & (1 << i))
				return i;
		
		return 0xFFFFFFFF; // Unable to find memoryType
	}

	// Same as KR_MEMALIGN(). 'alignment' must be a power of two.
	static inline VkDeviceSize AlignBufferSize(VkDeviceSize size, VkDeviceSize alignment)
	{
		return (size + alignment - 1) & ~(alignment - 1);
	}

	void KarmaGuiVulkanHandler::CreateOrResizeBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDeviceSize& bufferSize, size_t newSize, VkBufferUsageFlagBits usage)
	{
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		KarmaGui_ImplVulkan_InitInfo* vulkanInitInfo = &backendData->VulkanInitInfo;

		if (buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(vulkanInitInfo->Device, buffer, vulkanInitInfo->Allocator);
		}
		if (bufferMemory != VK_NULL_HANDLE)
		{
			vkFreeMemory(vulkanInitInfo->Device, bufferMemory, vulkanInitInfo->Allocator);
		}

		VkResult result;

		VkDeviceSize vertexBufferSizeAligned = AlignBufferSize(KR_MAX(1024*1024, newSize), backendData->BufferMemoryAlignment);
		
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = vertexBufferSizeAligned;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		result = vkCreateBuffer(vulkanInitInfo->Device, &bufferInfo, vulkanInitInfo->Allocator, &buffer);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't create the buffer");

		VkMemoryRequirements requirements;
		vkGetBufferMemoryRequirements(vulkanInitInfo->Device, buffer, &requirements);
		backendData->BufferMemoryAlignment = (backendData->BufferMemoryAlignment > requirements.alignment) ? backendData->BufferMemoryAlignment : requirements.alignment;

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = requirements.size;
		allocInfo.memoryTypeIndex = KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_MemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, requirements.memoryTypeBits);

		result = vkAllocateMemory(vulkanInitInfo->Device, &allocInfo, vulkanInitInfo->Allocator, &bufferMemory);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't allocate memory");

		result = vkBindBufferMemory(vulkanInitInfo->Device, buffer, bufferMemory, 0);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't bind memory. Oh crappp");

		bufferSize = requirements.size;
	}

	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_SetupRenderStateFor3DRendering(Scene* sceneToDraw, VkCommandBuffer commandBuffer, KGDrawData* drawData)
	{
		std::shared_ptr<VulkanVertexArray> vulkanVA = static_pointer_cast<VulkanVertexArray>(sceneToDraw->GetRenderableVertexArray());

		// Bind 3D Vertex And Index Buffers:
		{
			VkBuffer vertexBuffers[1] = { vulkanVA->GetVertexBuffer()->GetVertexBuffer() };
			VkDeviceSize vertexOffset[1] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, vertexOffset);
			vkCmdBindIndexBuffer(commandBuffer, vulkanVA->GetIndexBuffer()->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}

		// Setup viewport:
		{
			if(sceneToDraw->GetWindowToRenderWithinResizeStatus())
			{
				KGGuiWindow* windowToRenderWithin = static_cast<KGGuiWindow*>(sceneToDraw->GetRenderingWindow());

				vulkanVA->CreateExternalViewPort(windowToRenderWithin->Pos.x * drawData->FramebufferScale.x, (windowToRenderWithin->Pos.y + windowToRenderWithin->TitleBarHeight()) * drawData->FramebufferScale.y , windowToRenderWithin->Size.x * drawData->FramebufferScale.x , (windowToRenderWithin->Size.y - windowToRenderWithin->TitleBarHeight()) * drawData->FramebufferScale.y);
				vulkanVA->CleanupPipeline();
				vulkanVA->RecreateVulkanVA();
				sceneToDraw->SetWindowToRenderWithinResize(false);
			}
		}

		// Bind pipeline:
		{
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, static_pointer_cast<VulkanVertexArray>(sceneToDraw->GetRenderableVertexArray())->GetGraphicsPipeline());
		}

	}

	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_SetupRenderState(KGDrawData* drawData, VkPipeline pipeline, VkCommandBuffer commandBuffer, KarmaGui_ImplVulkanH_ImageFrameRenderBuffers* remderingBufferData, int width, int height)
	{
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();

		// Bind pipeline:
		{
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		}

		// Bind Vertex And Index Buffer:
		if (drawData->TotalVtxCount > 0)
		{
			VkBuffer vertexBuffers[1] = { remderingBufferData->VertexBuffer };
			VkDeviceSize vertexOffset[1] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, vertexOffset);
			vkCmdBindIndexBuffer(commandBuffer, remderingBufferData->IndexBuffer, 0, sizeof(KGDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
		}

		// Setup viewport:
		{
			VkViewport viewport;
			viewport.x = 0;
			viewport.y = 0;
			viewport.width = (float)width;
			viewport.height = (float)height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		}

		// Cowboy's Note: May need to use uniform buffer objects.
		// Setup scale and translation:
		// Our visible KarmaGui space lies from drawData->DisplayPps (top left) to drawData->DisplayPos + darawData->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
		{
			float scale[2];
			scale[0] = 2.0f / drawData->DisplaySize.x;
			scale[1] = 2.0f / drawData->DisplaySize.y;
			float translate[2];
			translate[0] = -1.0f - drawData->DisplayPos.x * scale[0];
			translate[1] = -1.0f - drawData->DisplayPos.y * scale[1];
			vkCmdPushConstants(commandBuffer, backendData->PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 0, sizeof(float) * 2, scale);
			vkCmdPushConstants(commandBuffer, backendData->PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 2, sizeof(float) * 2, translate);
		}
	}

	// Render function
	// As a side project, ponder over this rendering and Vulkan rendering logic of Karma. Maybe there is a scheme
	// to bring them together!!
	// For rendering windows of different viewport too
	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_RenderDrawData(KGDrawData* drawData, VkCommandBuffer commandBuffer, VkPipeline pipeline, uint32_t frameIndex)
	{
		// frameIndex parameter based algorithm, completely independent of windowData->ImageFrameIndex

		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		// Interesting insight, says the Cowboy! I think official tutorial mentions something too.
		int width = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
		int height = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
		if (width <= 0 || height <= 0)
		{
			return;
		}

		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		if (pipeline == VK_NULL_HANDLE)
		{
			pipeline = backendData->Pipeline;
		}

		// Allocate array to store enough vertex/index buffers. Each unique viewport gets its own storage.
		KarmaGui_ImplVulkan_ViewportData* viewportRendererData = (KarmaGui_ImplVulkan_ViewportData*)drawData->OwnerViewport->RendererUserData;
		KR_CORE_ASSERT(viewportRendererData != nullptr, "Couldn't unearth viewport data!");

		KarmaGui_ImplVulkanH_WindowRenderBuffers* windowRenderBuffers = &viewportRendererData->RenderBuffers;

		// Let me bind buffer and memory resources to frameIndex
		windowRenderBuffers->Index = frameIndex;

		if (windowRenderBuffers->FrameRenderBuffers == nullptr)
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

			windowRenderBuffers->Count = vulkanAPI->GetMaxFramesInFlight();

			// Caution: Need to think about the object instantiation and resource management
			windowRenderBuffers->FrameRenderBuffers = new KarmaGui_ImplVulkanH_ImageFrameRenderBuffers[windowRenderBuffers->Count];
		}

		KarmaGui_ImplVulkanH_ImageFrameRenderBuffers* renderBuffer = &windowRenderBuffers->FrameRenderBuffers[windowRenderBuffers->Index];

		if (drawData->TotalVtxCount > 0)
		{
			// Create or resize the vertex/index buffers
			size_t vertexSize = drawData->TotalVtxCount * sizeof(KGDrawVert);
			size_t indexSize = drawData->TotalIdxCount * sizeof(KGDrawIdx);
			if (renderBuffer->VertexBuffer == VK_NULL_HANDLE || renderBuffer->VertexBufferSize < vertexSize)
			{
				CreateOrResizeBuffer(renderBuffer->VertexBuffer, renderBuffer->VertexBufferMemory, renderBuffer->VertexBufferSize, vertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
			}
			if (renderBuffer->IndexBuffer == VK_NULL_HANDLE || renderBuffer->IndexBufferSize < indexSize)
			{
				CreateOrResizeBuffer(renderBuffer->IndexBuffer, renderBuffer->IndexBufferMemory, renderBuffer->IndexBufferSize, indexSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
			}

			// Upload vertex/index data into a single contiguous GPU buffer
			KGDrawVert* vertexData = nullptr;
			KGDrawIdx* indexData = nullptr;

			VkResult result = vkMapMemory(vulkanInfo->Device, renderBuffer->VertexBufferMemory, 0, renderBuffer->VertexBufferSize, 0, (void**)(&vertexData));
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't map the Vertex memory");

			result = vkMapMemory(vulkanInfo->Device, renderBuffer->IndexBufferMemory, 0, renderBuffer->IndexBufferSize, 0, (void**)(&indexData));
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't map the Index memory");

			for (int n = 0; n < drawData->CmdListsCount; n++)
			{
				const KGDrawList* cmdList = drawData->CmdLists[n];
				memcpy(vertexData, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(KGDrawVert));
				memcpy(indexData, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(KGDrawIdx));
				vertexData += cmdList->VtxBuffer.Size;
				indexData += cmdList->IdxBuffer.Size;
			}

			VkMappedMemoryRange range[2] = {};
			range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			range[0].memory = renderBuffer->VertexBufferMemory;
			range[0].size = VK_WHOLE_SIZE;
			range[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			range[1].memory = renderBuffer->IndexBufferMemory;
			range[1].size = VK_WHOLE_SIZE;

			result = vkFlushMappedMemoryRanges(vulkanInfo->Device, 2, range);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't flush the decohered memory range");

			vkUnmapMemory(vulkanInfo->Device, renderBuffer->VertexBufferMemory);
			vkUnmapMemory(vulkanInfo->Device, renderBuffer->IndexBufferMemory);
		}

		// Will project scissor/clipping rectangles into framebuffer space
		KGVec2 clipOff = drawData->DisplayPos;         // (0,0) unless using multi-viewports
		KGVec2 clipScale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

		bool bDoneSettingRenderState = false;

		// Render command lists
		// (Because we merged all buffers into a single one, we maintain our own offset into them)
		// Aah the interleaving, I was wondering when this'd show up...
		int globalVertexOffset = 0;
		int globalIndexOffset = 0;

		Scene* sceneToDraw = nullptr;

		for (int n = 0; n < drawData->CmdListsCount; n++)
		{
			// Pointer to primitive drawing resources
			const KGDrawList* commandList = drawData->CmdLists[n];
			for (int commandCounter = 0; commandCounter < commandList->CmdBuffer.Size; commandCounter++)
			{
				const KGDrawCmd* drawCommand = &commandList->CmdBuffer[commandCounter];
				if (drawCommand->UserCallback != nullptr)
				{
					// User callback, registered via KarmaDrawList::AddCallback()
					// (KGDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset 	render state.)
					if (drawCommand->UserCallback == KGDrawCallback_ResetRenderState)
					{
						//sceneToDraw->SetWindowToRenderWithinResize(true);
					}
					else
					{
						drawCommand->UserCallback(commandList, drawCommand);
						sceneToDraw = static_cast<Scene*>(drawCommand->UserCallbackData);
						if (sceneToDraw)
						{
							// Assuming only one such callback
							KarmaGui_ImplVulkan_SetupRenderStateFor3DRendering(sceneToDraw, commandBuffer, drawData);
							bDoneSettingRenderState = false;

							std::shared_ptr<VulkanVertexArray> vulkanVA = static_pointer_cast<VulkanVertexArray>(sceneToDraw->GetRenderableVertexArray());

							// Hmm
							vulkanVA->UpdateProcessAndSetReadyForSubmission();
							vulkanVA->Bind();

							VulkanHolder::GetVulkanContext()->UploadUBO(frameIndex);

							vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanVA->GetGraphicsPipelineLayout(), 0, 1, &vulkanVA->GetDescriptorSets()[frameIndex], 0, nullptr);
							vkCmdDrawIndexed(commandBuffer, vulkanVA->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
						}
					}
				}
				else
				{
					// Setup desired Vulkan state
					// vkCmdBindPipeline, vkCmdBindVertexBuffers, vkCmdBindIndexBuffer, setup display viewport, and upload pushconstants or UBOs
					// specific to KarmaGui's not so 3d rendering types (windows etc)
					if (!bDoneSettingRenderState)
					{
						KarmaGui_ImplVulkan_SetupRenderState(drawData, pipeline, commandBuffer, renderBuffer, width, height);
						bDoneSettingRenderState = true;
					}

					// Project scissor/clipping rectangles into framebuffer space
					KGVec2 clipMin((drawCommand->ClipRect.x - clipOff.x) * clipScale.x, (drawCommand->ClipRect.y - clipOff.y) * clipScale.y);
					KGVec2 clipMax((drawCommand->ClipRect.z - clipOff.x) * clipScale.x, (drawCommand->ClipRect.w - clipOff.y) * clipScale.y);

					// Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
					if (clipMin.x < 0.0f) { clipMin.x = 0.0f; }
					if (clipMin.y < 0.0f) { clipMin.y = 0.0f; }
					if (clipMax.x > width) { clipMax.x = (float)width; }
					if (clipMax.y > height) { clipMax.y = (float)height; }
					if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
					{
						continue;
					}

					// Apply scissor/clipping rectangle
					VkRect2D scissor;
					scissor.offset.x = (int32_t)(clipMin.x);
					scissor.offset.y = (int32_t)(clipMin.y);
					scissor.extent.width = (uint32_t)(clipMax.x - clipMin.x);
					scissor.extent.height = (uint32_t)(clipMax.y - clipMin.y);
					vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

					// Bind DescriptorSet with font or user texture
					VkDescriptorSet descSet[1] = { (VkDescriptorSet)drawCommand->TextureId };
					if (sizeof(KGTextureID) < sizeof(KGU64))
					{
						// We don't support texture switches if ImTextureID hasn't been redefined to be 64-bit. Do a flaky check that other textures haven't been used.
						KR_CORE_ASSERT(drawCommand->TextureId == (KGTextureID)backendData->FontDescriptorSet, "ID mismatch");
						descSet[0] = backendData->FontDescriptorSet;
					}
					vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, backendData->PipelineLayout, 0, 1, descSet, 0, nullptr);

					// Draw
					vkCmdDrawIndexed(commandBuffer, drawCommand->ElemCount, 1, drawCommand->IdxOffset + globalIndexOffset, drawCommand->VtxOffset + globalVertexOffset, 0);
				}
			}
			globalIndexOffset += commandList->IdxBuffer.Size;
			globalVertexOffset += commandList->VtxBuffer.Size;
		}

		// Note: at this point both vkCmdSetViewport() and vkCmdSetScissor() have been called.
		// Our last values will leak into user/application rendering IF:
		// - Your application uses a pipeline with VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR dynamic state
		// - And you forgot to call vkCmdSetViewport() and vkCmdSetScissor() yourself to explicitely set that state.
		// If you use VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR you are responsible for setting the values before rendering.
		// In theory we should aim to backup/restore those values but I am not sure this is possible.
		// We perform a call to vkCmdSetScissor() to set back a full viewport which is likely to fix things for 99% users but technically this is not perfect. (See github #4644)
		VkRect2D scissor = { { 0, 0 }, { (uint32_t)width, (uint32_t)height } };
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	bool KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateTexture(VkCommandBuffer commandBuffer, char const* fileName, const std::string& lable)
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		KarmaGui_ImplVulkan_Image_TextureData* imageData = new KarmaGui_ImplVulkan_Image_TextureData();
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		imageData->TextureLable = lable;

		int width, height, channels;
		unsigned char* imagePixelData;

		imagePixelData = KarmaUtilities::GetImagePixelData(fileName, &width, &height, &channels, STBI_rgb_alpha);

		size_t uploadSize = width * height * 4 * sizeof(char);

		VkResult result;

		// Create the Image:
		{
			VkImageCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			info.imageType = VK_IMAGE_TYPE_2D;
			info.format = VK_FORMAT_R8G8B8A8_UNORM;
			info.extent.width = width;
			info.extent.height = height;
			info.extent.depth = 1;
			info.mipLevels = 1;
			info.arrayLayers = 1;
			info.samples = VK_SAMPLE_COUNT_1_BIT;
			info.tiling = VK_IMAGE_TILING_OPTIMAL;
			info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			result = vkCreateImage(vulkanInfo->Device, &info, vulkanInfo->Allocator, &imageData->TextureImage);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't create a image");

			VkMemoryRequirements req;
			vkGetImageMemoryRequirements(vulkanInfo->Device, imageData->TextureImage, &req);
			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = req.size;
			allocInfo.memoryTypeIndex = KarmaGui_ImplVulkan_MemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, req.memoryTypeBits);

			result = vkAllocateMemory(vulkanInfo->Device, &allocInfo, vulkanInfo->Allocator, &imageData->TextureMemory);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't allocate memory");

			result = vkBindImageMemory(vulkanInfo->Device, imageData->TextureImage, imageData->TextureMemory, 0);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't bind image memory");
		}

		// Create the Image View:
		{
			VkImageViewCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.image = imageData->TextureImage;
			info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			info.format = VK_FORMAT_R8G8B8A8_UNORM;
			info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			info.subresourceRange.levelCount = 1;
			info.subresourceRange.layerCount = 1;

			result = vkCreateImageView(vulkanInfo->Device, &info, vulkanInfo->Allocator, &imageData->TextureView);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Coudln't create image view");
		}

		// Create the Descriptor Set:
		imageData->TextureDescriptorSet = (VkDescriptorSet)KarmaGui_ImplVulkan_AddTexture(imageData->TextureSampler, imageData->TextureView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		// Create the Upload Buffer:
		{
			VkBufferCreateInfo bufferInfo = {};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = uploadSize;
			bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			result = vkCreateBuffer(vulkanInfo->Device, &bufferInfo, vulkanInfo->Allocator, &imageData->UploadBuffer);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't create buffer");

			VkMemoryRequirements requirements;
			vkGetBufferMemoryRequirements(vulkanInfo->Device, imageData->UploadBuffer, &requirements);
			backendData->BufferMemoryAlignment = (backendData->BufferMemoryAlignment > requirements.alignment) ? backendData->BufferMemoryAlignment : requirements.alignment;
			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = requirements.size;
			allocInfo.memoryTypeIndex = KarmaGui_ImplVulkan_MemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, requirements.memoryTypeBits);

			result = vkAllocateMemory(vulkanInfo->Device, &allocInfo, vulkanInfo->Allocator, &imageData->UploadBufferMemory);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't allocate memory");

			result = vkBindBufferMemory(vulkanInfo->Device, imageData->UploadBuffer, imageData->UploadBufferMemory, 0);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't bind buffer memory");
		}

		// Upload to Buffer:
		{
			char* map = nullptr;
			result = vkMapMemory(vulkanInfo->Device, imageData->UploadBufferMemory, 0, uploadSize, 0, (void**)(&map));
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't map memory");

			memcpy(map, imagePixelData, uploadSize);
			VkMappedMemoryRange range[1] = {};
			range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			range[0].memory = imageData->UploadBufferMemory;
			range[0].size = uploadSize;

			result = vkFlushMappedMemoryRanges(vulkanInfo->Device, 1, range);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't flush memory range");

			vkUnmapMemory(vulkanInfo->Device, imageData->UploadBufferMemory);
		}

		// Copy to Image:
		{
			VkImageMemoryBarrier copyBarrier[1] = {};
			copyBarrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			copyBarrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			copyBarrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			copyBarrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			copyBarrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			copyBarrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			copyBarrier[0].image = imageData->TextureImage;
			copyBarrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyBarrier[0].subresourceRange.levelCount = 1;
			copyBarrier[0].subresourceRange.layerCount = 1;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, copyBarrier);

			VkBufferImageCopy region = {};
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.layerCount = 1;
			region.imageExtent.width = width;
			region.imageExtent.height = height;
			region.imageExtent.depth = 1;
			vkCmdCopyBufferToImage(commandBuffer, imageData->UploadBuffer, imageData->TextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			VkImageMemoryBarrier useBarrier[1] = {};
			useBarrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			useBarrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			useBarrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			useBarrier[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			useBarrier[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			useBarrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			useBarrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			useBarrier[0].image = imageData->TextureImage;
			useBarrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			useBarrier[0].subresourceRange.levelCount = 1;
			useBarrier[0].subresourceRange.layerCount = 1;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, useBarrier);
		}

		imageData->height = (uint32_t)height;
		imageData->width = (uint32_t)width;
		imageData->channels = (uint32_t)channels;
		imageData->size = (uint32_t)uploadSize;

		backendData->vulkanMesaDecalDataList.push_back(imageData);

		return true;
	}

	bool KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateFontsTexture(VkCommandBuffer commandBuffer)
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		size_t uploadSize = width * height * 4 * sizeof(char);

		VkResult result;

		// Create the Image:
		{
			VkImageCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			info.imageType = VK_IMAGE_TYPE_2D;
			info.format = VK_FORMAT_R8G8B8A8_UNORM;
			info.extent.width = width;
			info.extent.height = height;
			info.extent.depth = 1;
			info.mipLevels = 1;
			info.arrayLayers = 1;
			info.samples = VK_SAMPLE_COUNT_1_BIT;
			info.tiling = VK_IMAGE_TILING_OPTIMAL;
			info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			result = vkCreateImage(vulkanInfo->Device, &info, vulkanInfo->Allocator, &backendData->FontImage);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't create a image");

			VkMemoryRequirements req;
			vkGetImageMemoryRequirements(vulkanInfo->Device, backendData->FontImage, &req);
			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = req.size;
			allocInfo.memoryTypeIndex = KarmaGui_ImplVulkan_MemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, req.memoryTypeBits);

			result = vkAllocateMemory(vulkanInfo->Device, &allocInfo, vulkanInfo->Allocator, &backendData->FontMemory);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't allocate memory");

			result = vkBindImageMemory(vulkanInfo->Device, backendData->FontImage, backendData->FontMemory, 0);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't bind image memory");
		}

		// Create the Image View:
		{
			VkImageViewCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.image = backendData->FontImage;
			info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			info.format = VK_FORMAT_R8G8B8A8_UNORM;
			info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			info.subresourceRange.levelCount = 1;
			info.subresourceRange.layerCount = 1;

			result = vkCreateImageView(vulkanInfo->Device, &info, vulkanInfo->Allocator, &backendData->FontView);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Coudln't create image view");
		}

		// Create the Descriptor Set:
		backendData->FontDescriptorSet = (VkDescriptorSet)KarmaGui_ImplVulkan_AddTexture(backendData->FontSampler, backendData->FontView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		// Create the Upload Buffer:
		{
			VkBufferCreateInfo bufferInfo = {};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = uploadSize;
			bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			result = vkCreateBuffer(vulkanInfo->Device, &bufferInfo, vulkanInfo->Allocator, &backendData->UploadBuffer);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't create buffer");

			VkMemoryRequirements requirements;
			vkGetBufferMemoryRequirements(vulkanInfo->Device, backendData->UploadBuffer, &requirements);
			backendData->BufferMemoryAlignment = (backendData->BufferMemoryAlignment > requirements.alignment) ? backendData->BufferMemoryAlignment : requirements.alignment;
			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = requirements.size;
			allocInfo.memoryTypeIndex = KarmaGui_ImplVulkan_MemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, requirements.memoryTypeBits);

			result = vkAllocateMemory(vulkanInfo->Device, &allocInfo, vulkanInfo->Allocator, &backendData->UploadBufferMemory);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't allocate memory");

			result = vkBindBufferMemory(vulkanInfo->Device, backendData->UploadBuffer, backendData->UploadBufferMemory, 0);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't bind buffer memory");
		}

		// Upload to Buffer:
		{
			char* map = nullptr;
			result = vkMapMemory(vulkanInfo->Device, backendData->UploadBufferMemory, 0, uploadSize, 0, (void**)(&map));
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't map memory");

			memcpy(map, pixels, uploadSize);
			VkMappedMemoryRange range[1] = {};
			range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			range[0].memory = backendData->UploadBufferMemory;
			range[0].size = uploadSize;

			result = vkFlushMappedMemoryRanges(vulkanInfo->Device, 1, range);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't flush memory range");

			vkUnmapMemory(vulkanInfo->Device, backendData->UploadBufferMemory);
		}

		// Copy to Image:
		{
			VkImageMemoryBarrier copyBarrier[1] = {};
			copyBarrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			copyBarrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			copyBarrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			copyBarrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			copyBarrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			copyBarrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			copyBarrier[0].image = backendData->FontImage;
			copyBarrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyBarrier[0].subresourceRange.levelCount = 1;
			copyBarrier[0].subresourceRange.layerCount = 1;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, copyBarrier);

			VkBufferImageCopy region = {};
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.layerCount = 1;
			region.imageExtent.width = width;
			region.imageExtent.height = height;
			region.imageExtent.depth = 1;
			vkCmdCopyBufferToImage(commandBuffer, backendData->UploadBuffer, backendData->FontImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			VkImageMemoryBarrier useBarrier[1] = {};
			useBarrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			useBarrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			useBarrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			useBarrier[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			useBarrier[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			useBarrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			useBarrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			useBarrier[0].image = backendData->FontImage;
			useBarrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			useBarrier[0].subresourceRange.levelCount = 1;
			useBarrier[0].subresourceRange.layerCount = 1;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, useBarrier);
		}

		// Store our identifier
		io.Fonts->SetTexID((KGTextureID)backendData->FontDescriptorSet);

		return true;
	}

	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateShaderModules(VkDevice device, const VkAllocationCallbacks* allocator)
	{
		// Create the shader modules
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();;
		if (backendData->ShaderModuleVert == VK_NULL_HANDLE)
		{
			VkShaderModuleCreateInfo vertexInfo = {};
			vertexInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			vertexInfo.codeSize = sizeof(__glsl_shader_vert_spv);
			vertexInfo.pCode = (uint32_t*)__glsl_shader_vert_spv;
			VkResult result = vkCreateShaderModule(device, &vertexInfo, allocator, &backendData->ShaderModuleVert);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't create shader module");
		}

		if (backendData->ShaderModuleFrag == VK_NULL_HANDLE)
		{
			VkShaderModuleCreateInfo fragmentInfo = {};
			fragmentInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			fragmentInfo.codeSize = sizeof(__glsl_shader_frag_spv);
			fragmentInfo.pCode = (uint32_t*)__glsl_shader_frag_spv;

			VkResult result = vkCreateShaderModule(device, &fragmentInfo, allocator, &backendData->ShaderModuleFrag);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Create shader module");
		}
	}

	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateFontSampler(VkDevice device, const VkAllocationCallbacks* allocator)
	{
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		if (backendData->FontSampler)
		{
			return;
		}

		// Bilinear sampling is required by default. Set 'io.Fonts->Flags |= KGFontAtlasFlags_NoBakedLines' or 	'style.AntiAliasedLinesUseTex = false' to allow point/nearest sampling.
		VkSamplerCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		info.magFilter = VK_FILTER_LINEAR;
		info.minFilter = VK_FILTER_LINEAR;
		info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		info.minLod = -1000;
		info.maxLod = 1000;
		info.maxAnisotropy = 1.0f;

		VkResult result = vkCreateSampler(device, &info, allocator, &backendData->FontSampler);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't create sampler");
	}

	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateDescriptorSetLayout(VkDevice device, const VkAllocationCallbacks* allocator)
	{
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		if (backendData->DescriptorSetLayout)
		{
			return;
		}

		KarmaGui_ImplVulkan_CreateFontSampler(device, allocator);
		VkSampler sampler[1] = { backendData->FontSampler };
		VkDescriptorSetLayoutBinding binding[1] = {};
		binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding[0].descriptorCount = 1;
		binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		binding[0].pImmutableSamplers = sampler;
		VkDescriptorSetLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = 1;
		info.pBindings = binding;

		VkResult result = vkCreateDescriptorSetLayout(device, &info, allocator, &backendData->DescriptorSetLayout);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't create descriptor set layout");
	}

	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreatePipelineLayout(VkDevice device, const VkAllocationCallbacks* allocator)
	{
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		if (backendData->PipelineLayout)
		{
			return;
		}

		// Constants: we are using 'vec2 offset' and 'vec2 scale' instead of a full 3d projection matrix
		KarmaGui_ImplVulkan_CreateDescriptorSetLayout(device, allocator);
		VkPushConstantRange pushConstants[1] = {};
		pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstants[0].offset = sizeof(float) * 0;
		pushConstants[0].size = sizeof(float) * 4;
		VkDescriptorSetLayout setLayout[1] = { backendData->DescriptorSetLayout };
		VkPipelineLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = 1;
		layoutInfo.pSetLayouts = setLayout;
		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = pushConstants;

		VkResult result = vkCreatePipelineLayout(device, &layoutInfo, allocator, &backendData->PipelineLayout);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't create pipeline layout");
	}

	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreatePipeline(VkDevice device, const VkAllocationCallbacks* allocator, VkPipelineCache 	pipelineCache, VkRenderPass renderPass, VkSampleCountFlagBits MSAASamples, VkPipeline* pipeline, uint32_t subpass)
	{
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		KarmaGui_ImplVulkan_CreateShaderModules(device, allocator);

		VkPipelineShaderStageCreateInfo stage[2] = {};
		stage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		stage[0].module = backendData->ShaderModuleVert;
		stage[0].pName = "main";
		stage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		stage[1].module = backendData->ShaderModuleFrag;
		stage[1].pName = "main";

		VkVertexInputBindingDescription bindingDesc[1] = {};
		bindingDesc[0].stride = sizeof(KGDrawVert);
		bindingDesc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription attributeDesc[3] = {};
		attributeDesc[0].location = 0;
		attributeDesc[0].binding = bindingDesc[0].binding;
		attributeDesc[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDesc[0].offset = KG_OFFSETOF(KGDrawVert, pos);
		attributeDesc[1].location = 1;
		attributeDesc[1].binding = bindingDesc[0].binding;
		attributeDesc[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDesc[1].offset = KG_OFFSETOF(KGDrawVert, uv);
		attributeDesc[2].location = 2;
		attributeDesc[2].binding = bindingDesc[0].binding;
		attributeDesc[2].format = VK_FORMAT_R8G8B8A8_UNORM;
		attributeDesc[2].offset = KG_OFFSETOF(KGDrawVert, col);

		VkPipelineVertexInputStateCreateInfo vertexInfo = {};
		vertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInfo.vertexBindingDescriptionCount = 1;
		vertexInfo.pVertexBindingDescriptions = bindingDesc;
		vertexInfo.vertexAttributeDescriptionCount = 3;
		vertexInfo.pVertexAttributeDescriptions = attributeDesc;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
		inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineViewportStateCreateInfo viewportInfo = {};
		viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportInfo.viewportCount = 1;
		viewportInfo.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterInfo = {};
		rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterInfo.cullMode = VK_CULL_MODE_NONE;
		rasterInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterInfo.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multiSampleInfo = {};
		multiSampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multiSampleInfo.rasterizationSamples = (MSAASamples != 0) ? MSAASamples : VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorAttachment[1] = {};
		colorAttachment[0].blendEnable = VK_TRUE;
		colorAttachment[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorAttachment[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorAttachment[0].colorBlendOp = VK_BLEND_OP_ADD;
		colorAttachment[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorAttachment[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorAttachment[0].alphaBlendOp = VK_BLEND_OP_ADD;
		colorAttachment[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineDepthStencilStateCreateInfo depthInfo = {};
		depthInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

		VkPipelineColorBlendStateCreateInfo blendInfo = {};
		blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blendInfo.attachmentCount = 1;
		blendInfo.pAttachments = colorAttachment;

		VkDynamicState dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = (uint32_t)KG_ARRAYSIZE(dynamicStates);
		dynamicState.pDynamicStates = dynamicStates;

		KarmaGui_ImplVulkan_CreatePipelineLayout(device, allocator);

		VkGraphicsPipelineCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		info.flags = backendData->PipelineCreateFlags;
		info.stageCount = 2;
		info.pStages = stage;
		info.pVertexInputState = &vertexInfo;
		info.pInputAssemblyState = &inputAssemblyInfo;
		info.pViewportState = &viewportInfo;
		info.pRasterizationState = &rasterInfo;
		info.pMultisampleState = &multiSampleInfo;
		info.pDepthStencilState = &depthInfo;
		info.pColorBlendState = &blendInfo;
		info.pDynamicState = &dynamicState;
		info.layout = backendData->PipelineLayout;
		info.renderPass = renderPass;
		info.subpass = subpass;

		VkResult result = vkCreateGraphicsPipelines(device, pipelineCache, 1, &info, allocator, pipeline);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Coudln't create graphics pipelines");
	}

	bool KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateDeviceObjects()
	{
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;
		VkResult result;

		if (!backendData->FontSampler)
		{
			VkSamplerCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.magFilter = VK_FILTER_LINEAR;
			info.minFilter = VK_FILTER_LINEAR;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.minLod = -1000;
			info.maxLod = 1000;
			info.maxAnisotropy = 1.0f;
			result = vkCreateSampler(vulkanInfo->Device, &info, vulkanInfo->Allocator, &backendData->FontSampler);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Coudln't create sampler");
		}

		if (!backendData->DescriptorSetLayout)
		{
			VkSampler sampler[1] = { backendData->FontSampler };
			VkDescriptorSetLayoutBinding binding[1] = {};
			binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			binding[0].descriptorCount = 1;
			binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			binding[0].pImmutableSamplers = sampler;
			VkDescriptorSetLayoutCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.bindingCount = 1;
			info.pBindings = binding;

			result = vkCreateDescriptorSetLayout(vulkanInfo->Device, &info, vulkanInfo->Allocator, &backendData->DescriptorSetLayout);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't create descriptor set layout");
		}

		if (!backendData->PipelineLayout)
		{
			// Constants: we are using 'vec2 offset' and 'vec2 scale' instead of a full 3d projection matrix
			VkPushConstantRange pushConstants[1] = {};
			pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			pushConstants[0].offset = sizeof(float) * 0;
			pushConstants[0].size = sizeof(float) * 4;
			VkDescriptorSetLayout set_layout[1] = { backendData->DescriptorSetLayout };
			VkPipelineLayoutCreateInfo layoutInfo = {};
			layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			layoutInfo.setLayoutCount = 1;
			layoutInfo.pSetLayouts = set_layout;
			layoutInfo.pushConstantRangeCount = 1;
			layoutInfo.pPushConstantRanges = pushConstants;
			result = vkCreatePipelineLayout(vulkanInfo->Device, &layoutInfo, vulkanInfo->Allocator, &backendData->PipelineLayout);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't create pipeline layout");
		}

		KarmaGui_ImplVulkan_CreatePipeline(vulkanInfo->Device, vulkanInfo->Allocator, vulkanInfo->PipelineCache, backendData->RenderPass, vulkanInfo->MSAASamples, &backendData->Pipeline, backendData->Subpass);

		return true;
	}

	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_DestroyFontUploadObjects()
	{
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;
		if (backendData->UploadBuffer)
		{
			vkDestroyBuffer(vulkanInfo->Device, backendData->UploadBuffer, vulkanInfo->Allocator);
			backendData->UploadBuffer = VK_NULL_HANDLE;
		}
		if (backendData->UploadBufferMemory)
		{
			vkFreeMemory(vulkanInfo->Device, backendData->UploadBufferMemory, vulkanInfo->Allocator);
			backendData->UploadBufferMemory = VK_NULL_HANDLE;
		}
	}

	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_DestroyDeviceObjects()
	{
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_DestroyAllViewportsRenderBuffers(vulkanInfo->Device, vulkanInfo->Allocator);
		KarmaGui_ImplVulkan_DestroyFontUploadObjects();

		if (backendData->ShaderModuleVert)
		{
			vkDestroyShaderModule(vulkanInfo->Device, backendData->ShaderModuleVert, vulkanInfo->Allocator);
			backendData->ShaderModuleVert = VK_NULL_HANDLE;
		}
		if (backendData->ShaderModuleFrag)
		{
			vkDestroyShaderModule(vulkanInfo->Device, backendData->ShaderModuleFrag, vulkanInfo->Allocator);
			backendData->ShaderModuleFrag = VK_NULL_HANDLE;
		}
		if (backendData->FontView)
		{
			vkDestroyImageView(vulkanInfo->Device, backendData->FontView, vulkanInfo->Allocator);
			backendData->FontView = VK_NULL_HANDLE;
		}
		if (backendData->FontImage)
		{
			vkDestroyImage(vulkanInfo->Device, backendData->FontImage, vulkanInfo->Allocator);
			backendData->FontImage = VK_NULL_HANDLE;
		}
		if (backendData->FontMemory)
		{
			vkFreeMemory(vulkanInfo->Device, backendData->FontMemory, vulkanInfo->Allocator);
			backendData->FontMemory = VK_NULL_HANDLE;
		}
		if (backendData->FontSampler)
		{
			vkDestroySampler(vulkanInfo->Device, backendData->FontSampler, vulkanInfo->Allocator);
			backendData->FontSampler = VK_NULL_HANDLE;
		}
		if (backendData->DescriptorSetLayout)
		{
			vkDestroyDescriptorSetLayout(vulkanInfo->Device, backendData->DescriptorSetLayout, vulkanInfo->Allocator);
			backendData->DescriptorSetLayout = VK_NULL_HANDLE;
		}
		if (backendData->PipelineLayout)
		{
			vkDestroyPipelineLayout(vulkanInfo->Device, backendData->PipelineLayout, vulkanInfo->Allocator);
			backendData->PipelineLayout = VK_NULL_HANDLE;
		}
		if (backendData->Pipeline)
		{
			vkDestroyPipeline(vulkanInfo->Device, backendData->Pipeline, vulkanInfo->Allocator);
			backendData->Pipeline = VK_NULL_HANDLE;
		}

		for (auto& elem : backendData->vulkanMesaDecalDataList)
		{
			if (elem->TextureView)
			{
				vkDestroyImageView(vulkanInfo->Device, elem->TextureView, vulkanInfo->Allocator);
				elem->TextureView = VK_NULL_HANDLE;
			}
			if (elem->TextureImage)
			{
				vkDestroyImage(vulkanInfo->Device, elem->TextureImage, vulkanInfo->Allocator);
				elem->TextureImage = VK_NULL_HANDLE;
			}
			if (elem->TextureMemory)
			{
				vkFreeMemory(vulkanInfo->Device, elem->TextureMemory, vulkanInfo->Allocator);
				elem->TextureMemory = VK_NULL_HANDLE;
			}
			if (elem->TextureSampler)
			{
				vkDestroySampler(vulkanInfo->Device, elem->TextureSampler, vulkanInfo->Allocator);
				elem->TextureSampler = VK_NULL_HANDLE;
			}
			if (elem->TextureDescriptorSet)
			{
				elem->TextureDescriptorSet = VK_NULL_HANDLE;
			}
			if (elem->UploadBuffer)
			{
				vkDestroyBuffer(vulkanInfo->Device, elem->UploadBuffer, vulkanInfo->Allocator);
				elem->UploadBuffer = VK_NULL_HANDLE;
			}
			if (elem->UploadBufferMemory)
			{
				vkFreeMemory(vulkanInfo->Device, elem->UploadBufferMemory, vulkanInfo->Allocator);
				elem->UploadBufferMemory = VK_NULL_HANDLE;
			}

			delete elem;
			elem = nullptr;
		}
	}

	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_DestroyAllViewportsRenderBuffers(VkDevice device, const VkAllocationCallbacks* allocator)
	{
		KarmaGuiPlatformIO& platformIO = KarmaGui::GetPlatformIO();

		// Starts from 0, ponder over since mainviewport is handeled seperately?
		for (int counter = 0; counter < platformIO.Viewports.Size; counter++)
		{
			if (KarmaGui_ImplVulkan_ViewportData* viewportData = (KarmaGui_ImplVulkan_ViewportData*)platformIO.Viewports[counter]->RendererUserData)
			{
				KarmaGui_ImplVulkan_ShivaWindowRenderBuffers(device, &viewportData->RenderBuffers, allocator);
			}
		}
	}

	// Shiva means Destroy here!
	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_ShivaWindowRenderBuffers(VkDevice device, KarmaGui_ImplVulkanH_WindowRenderBuffers* buffers, const VkAllocationCallbacks* allocator)
	{
		for (uint32_t counter = 0; counter < buffers->Count; counter++)
		{
			KarmaGui_ImplVulkan_ShivaFrameRenderBuffers(device, &buffers->FrameRenderBuffers[counter], allocator);
		}

		delete[] buffers->FrameRenderBuffers;
		buffers->FrameRenderBuffers = nullptr;
		buffers->Index = 0;
		buffers->Count = 0;
	}

	// We should transition to Karma's way of vertex/index buffer resource allocation sometime in future.
	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_ShivaFrameRenderBuffers(VkDevice device, KarmaGui_ImplVulkanH_ImageFrameRenderBuffers* buffers, const VkAllocationCallbacks* allocator)
	{
		if (buffers->VertexBuffer)
		{
			vkDestroyBuffer(device, buffers->VertexBuffer, allocator);
			buffers->VertexBuffer = VK_NULL_HANDLE;
		}
		if (buffers->VertexBufferMemory)
		{
			vkFreeMemory(device, buffers->VertexBufferMemory, allocator);
			buffers->VertexBufferMemory = VK_NULL_HANDLE;
		}
		if (buffers->IndexBuffer)
		{
			vkDestroyBuffer(device, buffers->IndexBuffer, allocator);
			buffers->IndexBuffer = VK_NULL_HANDLE;
		}
		if (buffers->IndexBufferMemory)
		{
			vkFreeMemory(device, buffers->IndexBufferMemory, allocator);
			buffers->IndexBufferMemory = VK_NULL_HANDLE;
		}
		buffers->VertexBufferSize = 0;
		buffers->IndexBufferSize = 0;
	}

	// Seems like used for initializing the Vulkan relevant variables
	/*bool KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_Init(KarmaGui_ImplVulkan_InitInfo* info)
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		KR_CORE_ASSERT(io.BackendRendererUserData == nullptr, "Already initialized a renderer backend!");

		// Setup backend capabilities flags
		KarmaGui_ImplVulkan_Data* backendData = new KarmaGui_ImplVulkan_Data();

		// Since it seems like initialized struct, in MSVC, leads to problems in pushing back, we are doing this.
		backendData->BufferMemoryAlignment = 256;

		io.BackendRendererUserData = (void*)backendData;
		io.BackendRendererName = "Vulkan_Got_Back";
		io.BackendFlags |= KGGuiBackendFlags_RendererHasVtxOffset;  // We can honor the KGDrawCmd::VtxOffset field, allowing for large meshes.

		// Maybe chore for toofani mood!
		// io.BackendFlags |= KarmaGuiBackendFlags_RendererHasViewports;  // We can create multi-viewports on the Renderer side (optional)

		KR_CORE_ASSERT(info->Instance != VK_NULL_HANDLE, "No instance found");
		KR_CORE_ASSERT(info->PhysicalDevice != VK_NULL_HANDLE, "No physical device found");
		KR_CORE_ASSERT(info->Device != VK_NULL_HANDLE, "No device found");
		KR_CORE_ASSERT(info->Queue != VK_NULL_HANDLE, "No queue assigned");
		KR_CORE_ASSERT(info->DescriptorPool != VK_NULL_HANDLE, "No descriptor pool found");
		KR_CORE_ASSERT(info->MinImageCount >= 2, "Minimum image count exceeding limit");
		KR_CORE_ASSERT(info->ImageCount >= info->MinImageCount, "Not enough pitch for ImageCount");
		KR_CORE_ASSERT(info->RenderPass != VK_NULL_HANDLE, "No renderpass assigned");

		backendData->VulkanInitInfo = *info;
		backendData->RenderPass = info->RenderPass;
		backendData->Subpass = info->Subpass;

		// Font, descriptor, and pipeline
		KarmaGui_ImplVulkan_CreateDeviceObjects();

		// Our render function expect RendererUserData to be storing the window render buffer we need (for the main viewport we won't use ->Window)
		KarmaGuiViewport* mainViewport = KarmaGui::GetMainViewport();

		mainViewport->RendererUserData = new KarmaGui_ImplVulkan_ViewportData();

		if (io.ConfigFlags & KGGuiConfigFlags_ViewportsEnable)
		{
			// Setting up Dear ImGUI's window operations (create, resize, and all that)
			KarmaGui_ImplVulkan_InitPlatformInterface();
		}

		return true;
	}*/

	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_Shutdown()
	{
		KarmaGuiBackendRendererUserData* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		KR_CORE_ASSERT(backendData != nullptr, "No renderer backend to shutdown, or already shutdown?");

		KarmaGuiIO& io = KarmaGui::GetIO();

		// First destroy objects in all viewports
		KarmaGui_ImplVulkan_DestroyDeviceObjects();

		// Manually delete main viewport render data in-case we haven't initialized for viewports
		KarmaGuiViewport* mainViewport = KarmaGui::GetMainViewport();
		if (KarmaGui_ImplVulkan_ViewportData* viewportData = (KarmaGui_ImplVulkan_ViewportData*)mainViewport->RendererUserData)
		{
			delete viewportData;
			viewportData = nullptr;
		}
		mainViewport->RendererUserData = nullptr;

		// Clean up windows
		KarmaGui_ImplVulkan_ShutdownPlatformInterface();

		io.BackendRendererName = nullptr;
		io.BackendRendererUserData = nullptr;
		delete backendData;
	}

	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_NewFrame()
	{
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();

		KR_CORE_ASSERT(backendData != nullptr, "Did you call KarmaGui_ImplVulkan_Init()?");

		//IM_UNUSED(backendData);
	}

	// Register a texture
	// FIXME: This is experimental in the sense that we are unsure how to best design/tackle this problem, please post to 	https://github.com/ocornut/imgui/pull/914 if you have suggestions.
	VkDescriptorSet KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_AddTexture(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout)
	{
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		// Create Descriptor Set:
		VkDescriptorSet descriptorSet;
		{
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = vulkanInfo->DescriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &backendData->DescriptorSetLayout;
			VkResult result = vkAllocateDescriptorSets(vulkanInfo->Device, &allocInfo, &descriptorSet);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't allocate descriptor sets");
		}

		// Update the Descriptor Set:
		{
			VkDescriptorImageInfo descImage[1] = {};
			descImage[0].sampler = sampler;
			descImage[0].imageView = imageView;
			descImage[0].imageLayout = imageLayout;
			VkWriteDescriptorSet writeDesc[1] = {};
			writeDesc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDesc[0].dstSet = descriptorSet;
			writeDesc[0].descriptorCount = 1;
			writeDesc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDesc[0].pImageInfo = descImage;
			vkUpdateDescriptorSets(vulkanInfo->Device, 1, writeDesc, 0, NULL);
		}

		return descriptorSet;
	}

	void KarmaGuiVulkanHandler::ShareVulkanContextResourcesOfMainWindow(KarmaGui_ImplVulkanH_Window* windowData, bool bCreateSyncronicity)
	{
		// Still need to address bRecreateSwapChainAndCommandBuffers?

		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		// Clear the structure with now redundant information
		ClearVulkanWindowData(windowData, bCreateSyncronicity);

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

		// Fill relevant information gathered from VulkanContext
		// Assuming new swapchain and all that has been created in the KarmaGuiLayer::GiveLoopBeginControlToVulkan
		windowData->Swapchain = VulkanHolder::GetVulkanContext()->GetSwapChain();
		windowData->TotalImageCount = (uint32_t)VulkanHolder::GetVulkanContext()->GetSwapChainImages().size();
		windowData->RenderArea.extent = VulkanHolder::GetVulkanContext()->GetSwapChainExtent();
		windowData->RenderArea.offset = { 0, 0 };
		windowData->RenderPass = VulkanHolder::GetVulkanContext()->GetRenderPass();
		windowData->MAX_FRAMES_IN_FLIGHT = vulkanAPI->GetMaxFramesInFlight();
		windowData->CommandPool = VulkanHolder::GetVulkanContext()->GetCommandPool();
		windowData->ImageFrameIndex = 0;

		KR_CORE_ASSERT(windowData->ImageFrames == nullptr, "Somehow frames are still occupied. Please clear them.");
		windowData->ImageFrames = new KarmaGui_ImplVulkanH_ImageFrame[windowData->TotalImageCount];

		for (uint32_t counter = 0; counter < windowData->TotalImageCount; counter++)
		{
			KarmaGui_ImplVulkanH_ImageFrame* frameData = &windowData->ImageFrames[counter];

			// VulkanContext ImageView equivalent
			frameData->BackbufferView = VulkanHolder::GetVulkanContext()->GetSwapChainImageViews()[counter];

			// Framebuffer
			frameData->Framebuffer = VulkanHolder::GetVulkanContext()->GetSwapChainFrameBuffer()[counter];

			// Backbuffers could be VulkanContext m_swapChainImages equivalent
			frameData->Backbuffer = VulkanHolder::GetVulkanContext()->GetSwapChainImages()[counter];
		}

		// We create seperate syncronicity resources for Dear KarmaGui
		if (bCreateSyncronicity)
		{
			// For syncronicity, we instantiate FramesOnFlight with MAX_FRAMES_IN_FLIGHT number and label them the Real-Frames-On-Flight in our mind
			KR_CORE_ASSERT(windowData->FramesOnFlight == nullptr, "Somehow frames-on-flight are still occupied. Please clear them.");

			windowData->FramesOnFlight = new KarmaGui_Vulkan_Frame_On_Flight[windowData->MAX_FRAMES_IN_FLIGHT];

			for (uint32_t counter = 0; counter < windowData->MAX_FRAMES_IN_FLIGHT; counter++)
			{
				KarmaGui_Vulkan_Frame_On_Flight* frameOnFlight = &windowData->FramesOnFlight[counter];

				VkFenceCreateInfo fenceInfo = {};
				fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
				VkResult result = vkCreateFence(vulkanInfo->Device, &fenceInfo, VK_NULL_HANDLE, &frameOnFlight->Fence);

				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create fence");

				VkSemaphoreCreateInfo semaphoreInfo = {};
				semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

				result = vkCreateSemaphore(vulkanInfo->Device, &semaphoreInfo, VK_NULL_HANDLE, &frameOnFlight->ImageAcquiredSemaphore);

				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create ImageAcquiredSemaphore");

				result = vkCreateSemaphore(vulkanInfo->Device, &semaphoreInfo, VK_NULL_HANDLE, &frameOnFlight->RenderCompleteSemaphore);

				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create RenderCompleteSemaphore");
			}

			windowData->SemaphoreIndex = 0;
		}

		KR_CORE_ASSERT(windowData->FramesOnFlight != nullptr, "Commandbuffers are being assigned without enough resources");
		vulkanAPI->AllocateCommandBuffers();

		for (uint32_t counter = 0; counter < windowData->MAX_FRAMES_IN_FLIGHT; counter++)
		{
			KarmaGui_Vulkan_Frame_On_Flight* frameOnFlight = &windowData->FramesOnFlight[counter];
			frameOnFlight->CommandBuffer = vulkanAPI->GetCommandBuffers()[counter];
		}
	}

	void KarmaGuiVulkanHandler::ClearVulkanWindowData(KarmaGui_ImplVulkanH_Window* vulkanWindowData, bool bDestroySyncronicity)
	{
		// We are assuming that VulkanRendererAPI::RecreateCommandBuffersAndSwapChain()
		// has been called, thereby rendering the Swapchain and all those handles of vulkanWindowData, redundant.
		vulkanWindowData->Swapchain = VK_NULL_HANDLE;
		vulkanWindowData->RenderPass = VK_NULL_HANDLE;

		for (uint32_t counter = 0; counter < vulkanWindowData->TotalImageCount; counter++)
		{
			if (vulkanWindowData->ImageFrames != nullptr)
			{
				DestroyWindowDataFrame(&vulkanWindowData->ImageFrames[counter]);
			}
		}

		if (bDestroySyncronicity)
		{
			for (uint32_t counter = 0; counter < vulkanWindowData->MAX_FRAMES_IN_FLIGHT; counter++)
			{
				if (vulkanWindowData->FramesOnFlight != nullptr)
				{
					// Remove syncronicity resources using Vulkan API
					DestroyFramesOnFlightData(&vulkanWindowData->FramesOnFlight[counter]);
				}
			}
		}

		if (vulkanWindowData->ImageFrames != nullptr)
		{
			delete[] vulkanWindowData->ImageFrames;
			vulkanWindowData->ImageFrames = nullptr;
		}

		if (bDestroySyncronicity && vulkanWindowData->FramesOnFlight != nullptr)
		{
			delete[] vulkanWindowData->FramesOnFlight;
			vulkanWindowData->FramesOnFlight = nullptr;
		}

		vulkanWindowData->TotalImageCount = 0;
		vulkanWindowData->SemaphoreIndex = 0;
		vulkanWindowData->CommandPool = VK_NULL_HANDLE;
	}

	void KarmaGuiVulkanHandler::DestroyWindowDataFrame(KarmaGui_ImplVulkanH_ImageFrame* frame)
	{
		if (frame == nullptr)
		{
			return;
		}

		frame->Backbuffer = VK_NULL_HANDLE;
		frame->Framebuffer = VK_NULL_HANDLE;
	}

	void KarmaGuiVulkanHandler::DestroyFramesOnFlightData(KarmaGui_Vulkan_Frame_On_Flight* frameSyncronicityData)
	{
		if (frameSyncronicityData == nullptr)
		{
			return;
		}

		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		vkDeviceWaitIdle(vulkanInfo->Device);

		vkDestroyFence(vulkanInfo->Device, frameSyncronicityData->Fence, VK_NULL_HANDLE);
		frameSyncronicityData->Fence = VK_NULL_HANDLE;

		vkDestroySemaphore(vulkanInfo->Device, frameSyncronicityData->ImageAcquiredSemaphore, VK_NULL_HANDLE);
		vkDestroySemaphore(vulkanInfo->Device, frameSyncronicityData->RenderCompleteSemaphore, VK_NULL_HANDLE);
		frameSyncronicityData->ImageAcquiredSemaphore = frameSyncronicityData->RenderCompleteSemaphore = VK_NULL_HANDLE;

		// Seems bit odd, should make smooth in subsequent iterations
		frameSyncronicityData->CommandBuffer = VK_NULL_HANDLE;
	}

	// Create or resize window
	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateOrResizeWindow(KarmaGui_ImplVulkanH_Window* windowData, bool bCreateSyncronicity, bool bRecreateSwapChainAndCommandBuffers)
	{
		if (bRecreateSwapChainAndCommandBuffers)
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
		}

		KarmaGuiVulkanHandler::ShareVulkanContextResourcesOfMainWindow(windowData, bCreateSyncronicity);
	}

	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_DestroyWindow(KarmaGui_ImplVulkanH_Window* windowData)
	{
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		//vkDeviceWaitIdle(vulkanInfo->Device); // FIXME: We could wait on the Queue if we had the queue in windowData-> (otherwise VulkanH functions can't use globals)

		ClearVulkanWindowData(windowData, true);

		// Seems like each window has seperate instantiation of the resources below
		// Ponder over them
		/*
		vkDestroyPipeline(device, wd->Pipeline, allocator);
		vkDestroyRenderPass(device, wd->RenderPass, allocator);
		vkDestroySwapchainKHR(device, wd->Swapchain, allocator);
		vkDestroySurfaceKHR(instance, wd->Surface, allocator);
		*/
	}

	//--------------------------------------------------------------------------------------------------------
	// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
	// This is an _advanced_ and _optional_ feature, allowing the backend to create and handle multiple viewports simultaneously.
	// If you are new to dear imgui or creating a new binding for dear imgui, it is recommended that you completely ignore this section first..
	//--------------------------------------------------------------------------------------------------------

	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateWindow(KarmaGuiViewport* viewport)
	{
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		KarmaGui_ImplVulkan_ViewportData* viewportData = new KarmaGui_ImplVulkan_ViewportData();
		viewport->RendererUserData = viewportData;
		KarmaGui_ImplVulkanH_Window* windowData = &viewportData->Window;
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		VkSurfaceKHR vulkanSurface = VulkanHolder::GetVulkanContext()->GetSurface();

		// Check for Window System Integration (WSI) support
		VkBool32 bResult;
		vkGetPhysicalDeviceSurfaceSupportKHR(vulkanInfo->PhysicalDevice, vulkanInfo->QueueFamily, vulkanSurface, &bResult);
		if (bResult != VK_TRUE)
		{
			KR_CORE_ASSERT(false, "No WSI support found"); // Error: no WSI support on physical device
			return;
		}
		windowData->SurfaceFormat = VulkanHolder::GetVulkanContext()->GetSurfaceFormat();
		windowData->PresentMode = VulkanHolder::GetVulkanContext()->GetPresentMode();

		// No clue about the utility
		windowData->ClearEnable = (viewport->Flags & KGGuiViewportFlags_NoRendererClear) ? false : true;

		// Create SwapChain, RenderPass, Framebuffer, etc.
		KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateOrResizeWindow(windowData, true, false);
		viewportData->WindowOwned = true;
	}

	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_DestroyWindow(KarmaGuiViewport* viewport)
	{
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		if (KarmaGui_ImplVulkan_ViewportData* viewportData = (KarmaGui_ImplVulkan_ViewportData*)viewport->RendererUserData)
		{
			KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;
			if (viewportData->WindowOwned)
			{
				KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_DestroyWindow(&viewportData->Window);
			}
			KarmaGui_ImplVulkan_ShivaWindowRenderBuffers(vulkanInfo->Device, &viewportData->RenderBuffers, vulkanInfo->Allocator);
			delete viewportData;
			viewportData = nullptr;
		}
		viewport->RendererUserData = nullptr;
	}

	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_SetWindowSize(KarmaGuiViewport* viewport, KGVec2 size)
	{
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		KarmaGui_ImplVulkan_ViewportData* viewportData = (KarmaGui_ImplVulkan_ViewportData*)viewport->RendererUserData;
		if (viewportData == nullptr) // This is NULL for the main viewport (which is left to the user/app to handle)
		{
			return;
		}
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;
		viewportData->Window.ClearEnable = (viewport->Flags & KGGuiViewportFlags_NoRendererClear) ? false : true;
		KarmaGui_ImplVulkan_CreateOrResizeWindow(&viewportData->Window, false, true);
	}

	// May need extra scrutiny especially when we decoupled imageIndex and FrameOnFlightIndex.
	// In the current state of Engine, this function is not called because multiport rendering is not supported.
	void KarmaGuiVulkanHandler::KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_RenderWindow(KarmaGuiViewport* viewport, void*)
	{
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		KarmaGui_ImplVulkan_ViewportData* viewportData = (KarmaGui_ImplVulkan_ViewportData*)viewport->RendererUserData;
		KarmaGui_ImplVulkanH_Window* windowData = &viewportData->Window;
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		VkResult result;
		//windowData->SemaphoreIndex = 0;

		KarmaGui_ImplVulkanH_ImageFrame* frameData = nullptr;
		KarmaGui_Vulkan_Frame_On_Flight* frameFlightData = &windowData->FramesOnFlight[windowData->SemaphoreIndex];
		{
			{
				result = vkAcquireNextImageKHR(vulkanInfo->Device, windowData->Swapchain, UINT64_MAX, frameFlightData->ImageAcquiredSemaphore, VK_NULL_HANDLE, &windowData->ImageFrameIndex);
				frameData = &windowData->ImageFrames[windowData->ImageFrameIndex];
			}
			for (;;)
			{
				result = vkWaitForFences(vulkanInfo->Device, 1, &frameFlightData->Fence, VK_TRUE, 100);
				if (result == VK_SUCCESS) break;
				if (result == VK_TIMEOUT) continue;

				KR_CORE_ASSERT((result == VK_SUCCESS || VK_TIMEOUT), "Something not right");
			}
			{
				//result = vkResetCommandPool(vulkanInfo->Device, windowData->CommandPool, 0);
				//KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't resent the commandpool");

				VkCommandBufferBeginInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

				result = vkBeginCommandBuffer(frameFlightData->CommandBuffer, &info);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't begin commandbuffer recording");
			}
			{
				KGVec4 clearColor = KGVec4(0.0f, 0.0f, 0.0f, 1.0f);
				memcpy(&windowData->ClearValue.color.float32[0], &clearColor, 4 * sizeof(float));

				std::array<VkClearValue, 2> clearValues{};
				clearValues[0] = { windowData->ClearValue.color.float32[0], windowData->ClearValue.color.float32[1], 		windowData->ClearValue.color.float32[2], windowData->ClearValue.color.float32[3] };
				clearValues[1].depthStencil = { 1.0f, 0 };

				VkRenderPassBeginInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				info.renderPass = windowData->RenderPass;
				info.framebuffer = frameData->Framebuffer;
				info.renderArea.extent.width = windowData->Width;
				info.renderArea.extent.height = windowData->Height;
				info.clearValueCount = static_cast<uint32_t>(clearValues.size());
				info.pClearValues = clearValues.data();

				vkCmdBeginRenderPass(frameFlightData->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
			}
		}

		KarmaGui_ImplVulkan_RenderDrawData(viewport->DrawData, frameFlightData->CommandBuffer, windowData->Pipeline, windowData->SemaphoreIndex);

		{
			vkCmdEndRenderPass(frameFlightData->CommandBuffer);
			{
				VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				VkSubmitInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				info.waitSemaphoreCount = 1;
				info.pWaitSemaphores = &frameFlightData->ImageAcquiredSemaphore;
				info.pWaitDstStageMask = &waitStage;
				info.commandBufferCount = 1;
				info.pCommandBuffers = &frameFlightData->CommandBuffer;
				info.signalSemaphoreCount = 1;
				info.pSignalSemaphores = &frameFlightData->RenderCompleteSemaphore;

				result = vkEndCommandBuffer(frameFlightData->CommandBuffer);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Coudn't finish the commandbuffer recording")

					result = vkResetFences(vulkanInfo->Device, 1, &frameFlightData->Fence);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't reset the fences");

				result = vkQueueSubmit(vulkanInfo->Queue, 1, &info, frameFlightData->Fence);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't submit the queue");
			}
		}
	}

	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_SwapBuffers(KarmaGuiViewport* viewport, void*)
	{
		KarmaGui_ImplVulkan_Data* backendData = KarmaGuiRenderer::GetBackendRendererUserData();
		KarmaGui_ImplVulkan_ViewportData* viewportData = (KarmaGui_ImplVulkan_ViewportData*)viewport->RendererUserData;
		KarmaGui_ImplVulkanH_Window* windowData = &viewportData->Window;
		KarmaGui_ImplVulkan_InitInfo* vulkanInfo = &backendData->VulkanInitInfo;

		KarmaGuiViewport* mainViewPort = KarmaGui::GetMainViewport();
		KarmaGui_ImplVulkan_ViewportData* mainViewPortData = static_cast<KarmaGui_ImplVulkan_ViewportData*>(mainViewPort->RendererUserData);

		VkResult result;
		uint32_t presentIndex = windowData->ImageFrameIndex;

		KarmaGui_Vulkan_Frame_On_Flight* frameFlightData = &windowData->FramesOnFlight[windowData->SemaphoreIndex];
		VkPresentInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &frameFlightData->RenderCompleteSemaphore;
		info.swapchainCount = 1;
		info.pSwapchains = &windowData->Swapchain;
		info.pImageIndices = &presentIndex;

		result = vkQueuePresentKHR(vulkanInfo->Queue, &info);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			KarmaGui_ImplVulkan_CreateOrResizeWindow(&viewportData->Window, false, true);
		}
		else
		{
			KR_CORE_ASSERT(result == VK_SUCCESS, "Couldn't queue the image for presentation");
		}

		windowData->SemaphoreIndex = (windowData->SemaphoreIndex + 1) % windowData->MAX_FRAMES_IN_FLIGHT; // Now we can use the next set of semaphores
	}

	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_InitPlatformInterface()
	{
		KarmaGuiPlatformIO& platformIO = KarmaGui::GetPlatformIO();
		if (KarmaGui::GetIO().ConfigFlags & KGGuiConfigFlags_ViewportsEnable)
		{
			KR_CORE_ASSERT(platformIO.Platform_CreateVkSurface != nullptr, "Platform needs to setup the CreateVkSurface handler.");
		}
		platformIO.Renderer_CreateWindow = KarmaGui_ImplVulkan_CreateWindow;
		platformIO.Renderer_DestroyWindow = KarmaGui_ImplVulkan_DestroyWindow;
		platformIO.Renderer_SetWindowSize = KarmaGui_ImplVulkan_SetWindowSize;
		platformIO.Renderer_RenderWindow = KarmaGui_ImplVulkan_RenderWindow;
		platformIO.Renderer_SwapBuffers = KarmaGui_ImplVulkan_SwapBuffers;
	}

	void KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_ShutdownPlatformInterface()
	{
		KarmaGui::DestroyPlatformWindows();
	}
}
