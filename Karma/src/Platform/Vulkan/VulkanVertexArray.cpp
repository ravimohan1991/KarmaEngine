#include "VulkanVertexArray.h"
#include "Platform/Vulkan/VulkanHolder.h"
#include <fstream>

namespace Karma
{	
	VulkanVertexArray::VulkanVertexArray()
	{
		m_device = VulkanHolder::GetVulkanContext()->GetLogicalDevice();
	}

	VulkanVertexArray::~VulkanVertexArray()
	{
		vkDeviceWaitIdle(m_device);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
		}

		vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	}

	void VulkanVertexArray::Bind() const
	{
	}

	void VulkanVertexArray::RecreateSwapChainAndPipeline()
	{
		vkDeviceWaitIdle(m_device);

		CleanupPipelineandCommandBuffers();

		VulkanHolder::GetVulkanContext()->RecreateSwapChain();
		CreateGraphicsPipeline();
		CreateCommandBuffers();
	}

	void VulkanVertexArray::CleanupPipelineandCommandBuffers()
	{
		vkFreeCommandBuffers(m_device, VulkanHolder::GetVulkanContext()->GetCommandPool(), static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
		vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	}

	void VulkanVertexArray::CreateSemaphores()
	{
		m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		m_imagesInFlight.resize(VulkanHolder::GetVulkanContext()->GetSwapChainImages().size(), VK_NULL_HANDLE);
		
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkResult resulti = vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]);
			KR_CORE_ASSERT(resulti == VK_SUCCESS, "Failed to create imageAvailableSemaphore");

			VkResult resultr = vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]);
			KR_CORE_ASSERT(resultr == VK_SUCCESS, "Failed to create renderFinishedSemaphore");

			VkResult resultf = vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]);
			KR_CORE_ASSERT(resultf == VK_SUCCESS, "Failed to create inFlightFence");
		}
	}

	void VulkanVertexArray::CreateCommandBuffers()
	{
		m_commandBuffers.resize(VulkanHolder::GetVulkanContext()->GetSwapChainFrameBuffer().size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = VulkanHolder::GetVulkanContext()->GetCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

		VkResult result = vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data());

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create command buffers!");

		for (size_t i = 0; i < m_commandBuffers.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = nullptr;

			VkResult result = vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo);

			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to begin recording command buffer");

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = VulkanHolder::GetVulkanContext()->GetRenderPass();
			renderPassInfo.framebuffer = VulkanHolder::GetVulkanContext()->GetSwapChainFrameBuffer()[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = VulkanHolder::GetVulkanContext()->GetSwapChainExtent();

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

			// Bind vertex/index buffers
			VkBuffer vertexBuffers[] = {m_VertexBuffer->GetVertexBuffer()};
			VkDeviceSize offsets[] = { 0 };
			
			vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(m_commandBuffers[i], m_IndexBuffer->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(m_commandBuffers[i], m_IndexBuffer->GetCount(), 1, 0, 0, 0);

			vkCmdEndRenderPass(m_commandBuffers[i]);

			VkResult resultCB = vkEndCommandBuffer(m_commandBuffers[i]);

			KR_CORE_ASSERT(resultCB == VK_SUCCESS, "Failed to record command buffer");
		}

	}

	void VulkanVertexArray::SetShader(std::shared_ptr<Shader> shader)
	{
		m_Shader = std::static_pointer_cast<VulkanShader>(shader);
		GenerateVulkanVA();
	}

	void VulkanVertexArray::CreateGraphicsPipeline()
	{
		//auto vertShaderCode = ReadFile("../Resources/Shaders/vert.spv");
		//auto fragShaderCode = ReadFile("../Resources/Shaders/frag.spv");

		VkShaderModule vertShaderModule = CreateShaderModule(m_Shader->GetVertSpirV());
		VkShaderModule fragShaderModule = CreateShaderModule(m_Shader->GetFragSpirV());

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &m_bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = m_attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)VulkanHolder::GetVulkanContext()->GetSwapChainExtent().width;
		viewport.height = (float)VulkanHolder::GetVulkanContext()->GetSwapChainExtent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = VulkanHolder::GetVulkanContext()->GetSwapChainExtent();

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
			| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		VkResult result = vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr,
			&m_pipelineLayout);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create pipeline layout!");

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = m_pipelineLayout;
		pipelineInfo.renderPass = VulkanHolder::GetVulkanContext()->GetRenderPass();
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		VkResult resultGP = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE,
			1, &pipelineInfo, nullptr, &m_graphicsPipeline);

		KR_CORE_ASSERT(resultGP == VK_SUCCESS, "Failed to create graphics pipeline!");

		vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
		vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
	}
	
	std::vector<char> VulkanVertexArray::ReadFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			KR_CORE_ASSERT(false, "Failed to open file: " + filename);
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}

	VkShaderModule VulkanVertexArray::CreateShaderModule(const std::vector<uint32_t>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size() * sizeof(uint32_t);
		createInfo.pCode = code.data();

		VkShaderModule shaderModule;
		VkResult result = vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create shader module!");

		return shaderModule;
	}

	static VkFormat ShaderDataTypeToVulkanType(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:
			return VK_FORMAT_R32_SFLOAT;
		case ShaderDataType::Float2:
			return VK_FORMAT_R32G32_SFLOAT;
		case ShaderDataType::Float3:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case ShaderDataType::Float4:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		}

		KR_CORE_ASSERT(false, "Vulkan doesn't support this ShaderDatatype");
		return VK_FORMAT_UNDEFINED;
	}

	void VulkanVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
	{
		KR_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "VertexBufferLayout empty.");

		vertexBuffer->Bind();// What to do here?

		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();

		m_bindingDescription.binding = 0;
		m_bindingDescription.stride = layout.GetStride();
		m_bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		for (const auto& element : layout)
		{
			VkVertexInputAttributeDescription elementAttributeDescription{};
			elementAttributeDescription.binding = 0;
			elementAttributeDescription.location = index;
			elementAttributeDescription.format = ShaderDataTypeToVulkanType(element.Type);
			elementAttributeDescription.offset = static_cast<uint32_t>(element.Offset);

			m_attributeDescriptions.push_back(elementAttributeDescription);
			index++;
		}

		m_VertexBuffer = std::static_pointer_cast<VulkanVertexBuffer>(vertexBuffer);
	}

	void VulkanVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
	{
		indexBuffer->Bind();// What to do here?

		m_IndexBuffer = std::static_pointer_cast<VulkanIndexBuffer>(indexBuffer);

		//GenerateVulkanVA();
	}

	void VulkanVertexArray::GenerateVulkanVA()
	{
		CreateGraphicsPipeline();
		CreateCommandBuffers();
		CreateSemaphores();
	}
}