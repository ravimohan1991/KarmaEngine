#include "VulkanVertexArray.h"
#include "Platform/Vulkan/VulkanHolder.h"
#include "Platform/Vulkan/VulkanTexutre.h"
#include "Karma/Renderer/RenderCommand.h"

namespace Karma
{
	VulkanVertexArray::VulkanVertexArray() : m_SupportedDeviceFeatures(VulkanHolder::GetVulkanContext()->GetSupportedDeviceFeatures()),
		m_device(VulkanHolder::GetVulkanContext()->GetLogicalDevice())
	{
	}

	VulkanVertexArray::~VulkanVertexArray()
	{
		vkDeviceWaitIdle(m_device);
		CleanupPipeline();
	}

	void VulkanVertexArray::Bind() const
	{
	}

	void VulkanVertexArray::RecreateVulkanVA()
	{
		CreateDescriptorSetLayout();
		CreatePipelineLayout();
		CreateGraphicsPipeline();
		CreateDescriptorPool();
		CreateDescriptorSets();
	}

	void VulkanVertexArray::CleanupPipeline()
	{
		vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
		vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);// Descriptorsets get automatically get freed
	}

	void VulkanVertexArray::SetShader(std::shared_ptr<Shader> shader)
	{
		m_Shader = std::static_pointer_cast<VulkanShader>(shader);
		VulkanHolder::GetVulkanContext()->RegisterUBO(m_Shader->GetUniformBufferObject());
		GenerateVulkanVA();
	}

	void VulkanVertexArray::CreateGraphicsPipeline()
	{
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
		rasterizer.cullMode = VK_CULL_MODE_NONE;//VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		// Antialiasing
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkBool32 bLogicalOperationsAllowed = m_SupportedDeviceFeatures.logicOp;

		// Mix the old and new value to produce a final color
		// finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor;
		// finalColor.a = newAlpha.a;
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
			| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		if (!bLogicalOperationsAllowed)
		{
			colorBlendAttachment.blendEnable = VK_TRUE;
		}
		else
		{
			colorBlendAttachment.blendEnable = VK_FALSE;
		}
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		// Combine the old and new value using a bitwise operation
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		if (bLogicalOperationsAllowed)
		{
			colorBlending.logicOpEnable = VK_TRUE;
		}
		else
		{
			colorBlending.logicOpEnable = VK_FALSE;
		}
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

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
		pipelineInfo.pDepthStencilState = &depthStencil;

		VkResult resultGP = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE,
			1, &pipelineInfo, nullptr, &m_graphicsPipeline);

		KR_CORE_ASSERT(resultGP == VK_SUCCESS, "Failed to create graphics pipeline!");

		vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
		vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
	}

	void VulkanVertexArray::SetMesh(std::shared_ptr<Mesh> mesh)
	{
		KR_CORE_ASSERT(mesh->GetVertexBuffer()->GetLayout().GetElements().size(), "VertexBufferLayout empty.");

		//mesh->GetVertexBuffer()->Bind();
		AddVertexBuffer(mesh->GetVertexBuffer());

		// We are seperating VertexBuffers from Mesh.  Hopefully useful for batch rendering!
		m_VertexBuffers.push_back(mesh->GetVertexBuffer());

		// May need modificaitons for batch rendering later.
		m_IndexBuffer = std::static_pointer_cast<VulkanIndexBuffer>(mesh->GetIndexBuffer());
	}

	void VulkanVertexArray::SetMaterial(std::shared_ptr<Material> material)
	{
		m_Materials.push_back(material);
		m_Shader = std::static_pointer_cast<VulkanShader>(material->GetShader(0));

		VulkanHolder::GetVulkanContext()->RegisterUBO(m_Shader->GetUniformBufferObject());
		GenerateVulkanVA();
	}

	void VulkanVertexArray::UpdateProcessAndSetReadyForSubmission() const
	{
		// May need entry point for Object's world transform
		// also may need to shift a level up
		m_Materials.at(0)->OnUpdate();
		m_Materials.at(0)->ProcessForSubmission();
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
		case ShaderDataType::None:
		case ShaderDataType::Mat3:
		case ShaderDataType::Mat4:
		case ShaderDataType::Int:
		case ShaderDataType::Int2:
		case ShaderDataType::Int3:
		case ShaderDataType::Int4:
		case ShaderDataType::Bool:
			// Refer Mesh::GaugeVertexDataLayout for usual datatype
			// to be used in the context of vertex buffer
			KR_CORE_ASSERT(false, "Weird ShaderDataType is being used")
				return VK_FORMAT_UNDEFINED;
			break;
		}

		KR_CORE_ASSERT(false, "Vulkan doesn't support this ShaderDatatype");
		return VK_FORMAT_UNDEFINED;
	}

	void VulkanVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
	{
		KR_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "VertexBufferLayout empty.");

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
		m_IndexBuffer = std::static_pointer_cast<VulkanIndexBuffer>(indexBuffer);
	}

	void VulkanVertexArray::GenerateVulkanVA()
	{
		CreateDescriptorSetLayout();
		CreatePipelineLayout();
		CreateGraphicsPipeline();
		CreateDescriptorPool();
		CreateDescriptorSets();
	}

	void VulkanVertexArray::CreatePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;

		VkResult result = vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr,
			&m_pipelineLayout);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create pipeline layout!");
	}

	void VulkanVertexArray::CreateDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(VulkanHolder::GetVulkanContext()->GetSwapChainImages().size());
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(VulkanHolder::GetVulkanContext()->GetSwapChainImages().size());

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(VulkanHolder::GetVulkanContext()->GetSwapChainImages().size());

		VkResult result = vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create descriptor pool!");
	}

	void VulkanVertexArray::CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = m_Shader->GetUniformBufferObject()->GetBindingPointIndex();
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		VkResult result = vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create descriptor set layout!");
	}

	void VulkanVertexArray::CreateDescriptorSets()
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

		int maxFramesInFlight = vulkanAPI->GetMaxFramesInFlight();

		std::vector<VkDescriptorSetLayout> layouts(maxFramesInFlight, m_descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(maxFramesInFlight);
		allocInfo.pSetLayouts = layouts.data();

		m_descriptorSets.resize(maxFramesInFlight);
		VkResult result = vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data());

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to allocate descriptor sets!");

		for (size_t i = 0; i < maxFramesInFlight; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_Shader->GetUniformBufferObject()->GetUniformBuffers()[i];
			bufferInfo.offset = 0;
			bufferInfo.range = m_Shader->GetUniformBufferObject()->GetBufferSize();

			// Fetch right texture pointer first whose image is to be considered.
			// Caution: GetTexture index is with temporary assumption that needs addressing.
			std::shared_ptr<VulkanTexture> vTexture = m_Materials[0]->GetTexture(0)->GetVulkanTexture();

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = vTexture->GetImageView();
			imageInfo.sampler = vTexture->GetImageSampler();

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = m_descriptorSets[i];
			descriptorWrites[0].dstBinding = m_Shader->GetUniformBufferObject()->GetBindingPointIndex();
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = m_descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}
}