#include "VulkanDevice.h"
#include "VulkanDynamicRHI.h"
#include "Vulkan/VulkanTexture.h"
#include "VulkanRHI/VulkanDescriptorSets.h"

namespace Karma
{
	FVulkanDevice::FVulkanDevice(FVulkanDynamicRHI* InRHI, VkPhysicalDevice InGpu) : m_VulkanDynamicRHI(InRHI),
		m_GPU(InGpu), m_FenceManager(*this), m_CommandPool(VK_NULL_HANDLE), m_DefaultTexture(nullptr), m_DefaultDescriptorSetLayout(nullptr),
		m_LogicalDevice(VK_NULL_HANDLE), m_GraphicsQueue(VK_NULL_HANDLE), m_PresentQueue(VK_NULL_HANDLE)
	{
		// Implementation for creating a Vulkan logical device
	}

	FVulkanDevice::~FVulkanDevice()
	{
		// Implementation for cleaning up Vulkan device resources
	}

	void FVulkanDevice::Destroy()
	{
		// vkdestroy default buffers etc
		for (uint32_t counter = 0; counter < m_MaxFramesInFlight; counter++)
		{
			delete m_DefaultDescriptorSets[counter];
			delete m_DescriptorPool[counter];
		}

		delete m_DefaultTexture;
		delete m_DefaultDescriptorSetLayout;
		vkDestroyCommandPool(m_LogicalDevice, m_CommandPool, nullptr);
		vkDestroyDevice(m_LogicalDevice, nullptr);
	}

	void FVulkanDevice::InitGPU()
	{
		QueueFamilyIndices indices = m_VulkanDynamicRHI->FindQueueFamilies(m_GPU);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(),
		indices.presentFamily.value() };

		if (m_VulkanDynamicRHI->GetValidationLayersSetting())
		{
			KR_CORE_INFO("+-------------------------------------------------");
			KR_CORE_INFO("| Available Unique Queue Family Indices (GPU):");
			uint32_t index = 1;
			for (uint32_t queueFamily : uniqueQueueFamilies)
			{
				KR_CORE_INFO("| {0}. {1}", index++, queueFamily);
			}
			KR_CORE_INFO("+-------------------------------------------------");
		}

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		if (m_VulkanDynamicRHI->GetGpuDeviceFeatures().logicOp)
		{
			deviceFeatures.logicOp = VK_TRUE;
		}

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(FVulkanDynamicRHI::deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = FVulkanDynamicRHI::deviceExtensions.data();

		if (m_VulkanDynamicRHI->GetValidationLayersSetting())
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(FVulkanDynamicRHI::validationLayers.size());
			createInfo.ppEnabledLayerNames = FVulkanDynamicRHI::validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		VkResult result = vkCreateDevice(m_GPU, &createInfo, nullptr, &m_LogicalDevice);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create logical device!");
		KR_CORE_INFO("Successfully created a Vulkan logical device");

		vkGetDeviceQueue(m_LogicalDevice, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_LogicalDevice, indices.presentFamily.value(), 0, &m_PresentQueue);

		// CommandPool (UE has FVulkanDescriptorPoolsManager class for managing commandpools)
		QueueFamilyIndices queueFamilyIndices = m_VulkanDynamicRHI->FindQueueFamilies(m_GPU);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		result = vkCreateCommandPool(m_LogicalDevice, &poolInfo, nullptr, &m_CommandPool);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create command pool!");

		KR_CORE_INFO("Created Vulkan commandpool");

		// Default descriptorsets
		m_DefaultDescriptorSetLayout = new FVulkanDescriptorSetsLayout(this);
		
		PopulateWithDescriptorSetsLayout(*m_DefaultDescriptorSetLayout);
		m_DefaultDescriptorSetLayout->Compile();
	

		// Default texture (unreal grid)
		m_DefaultTexture = new VulkanTexture(this, "../Resources/Textures/UnrealGrid.png");
	}

	void FVulkanDevice::InitializeDefaultDescriptorSets(uint32_t MaxFramesInFlight)
	{
		m_DescriptorPool.Resize(MaxFramesInFlight);
		m_DefaultDescriptorSets.Resize(MaxFramesInFlight);

		for (uint32_t counter = 0; counter < MaxFramesInFlight; counter++)
		{
			m_DescriptorPool[counter] = new FVulkanDescriptorPool(this, *m_DefaultDescriptorSetLayout);
			m_DefaultDescriptorSets[counter] = new FVulkanDescriptorSets(this, *m_DefaultDescriptorSetLayout);

			m_DescriptorPool[counter]->AllocateDescriptorSets(*m_DefaultDescriptorSetLayout, *m_DefaultDescriptorSets[counter]);
		}

		m_MaxFramesInFlight = MaxFramesInFlight;
	}

	void FVulkanDevice::PopulateWithDescriptorSetsLayout(FVulkanDescriptorSetsLayout& InLayout)
	{
		// Set 0: Global descriptors (camera UBO, texture sampler)
		FVulkanDescriptorSetsLayoutInfo::FSetLayout setLayout;

		// Camera UBO at binding 0 (vertex shader)
		setLayout.m_LayoutBindings.Add({
			0, // binding
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1, // descriptorCount
			VK_SHADER_STAGE_VERTEX_BIT,
			nullptr
			});
		// Texture sampler at binding 1 (fragment shader) with default sampler and image view
		setLayout.m_LayoutBindings.Add({
			1, // binding
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1, // descriptorCount
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
			});

		setLayout.m_NumberOfDescriptorSets = 1;

		InLayout.AddDescriptorSet(setLayout);

		// Set 1: Per-mesh descriptors (per-mesh UBO)
		FVulkanDescriptorSetsLayoutInfo::FSetLayout meshSetLayout;
		// Per mesh UBO at binding 0 (vertex shader)
		meshSetLayout.m_LayoutBindings.Add({
			0, // binding
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1, // descriptorCount
			VK_SHADER_STAGE_VERTEX_BIT,
			});
		meshSetLayout.m_NumberOfDescriptorSets = 16;

		InLayout.AddDescriptorSet(meshSetLayout);
	}

	void FVulkanDevice::WaitUntilIdle()
	{
		VkResult result = vkDeviceWaitIdle(m_LogicalDevice);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to wait");
	}

	void FVulkanDevice::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_LogicalDevice, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && HasStencilComponent(format))
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		else
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = 0; // TODO
		barrier.dstAccessMask = 0; // TODO

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else
		{
			KR_CORE_ASSERT(false, "Unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_GraphicsQueue);

		vkFreeCommandBuffers(m_LogicalDevice, m_CommandPool, 1, &commandBuffer);
	}

	void FVulkanDevice::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_LogicalDevice, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_GraphicsQueue);

		vkFreeCommandBuffers(m_LogicalDevice, m_CommandPool, 1, &commandBuffer);
	}

	bool FVulkanDevice::HasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}
}
