#include "VulkanContext.h"
#include "GLFW/glfw3.h"
#include "Platform/Vulkan/VulkanHolder.h"
#include "glslang/Public/ShaderLang.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"
#include "Karma/Renderer/RenderCommand.h"
#include "Platform/Vulkan/VulkanVertexArray.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Scene.h"
#include "Engine/StaticMeshActor.h"

namespace Karma
{
	const std::vector<const char*> ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
	// Subject to change based on available hardware scrutiny
	std::vector<const char*> DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifdef KR_DEBUG
	bool VulkanContext::bEnableValidationLayers = true;
#else
	bool VulkanContext::bEnableValidationLayers = false;
#endif

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

	VulkanContext::VulkanContext(GLFWwindow* windowHandle)
		: m_windowHandle(windowHandle)
	{
		KR_CORE_ASSERT(windowHandle, "windowHandle is null");
		m_vulkanRendererAPI = static_cast<VulkanRendererAPI*> (RenderCommand::GetRendererAPI());
	}

	VulkanContext::~VulkanContext()
	{
		vkDestroyDescriptorSetLayout(m_device, m_ViewLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_device, m_TextureLayout, nullptr);

		vkDestroyDescriptorSetLayout(m_device, m_ObjectLayout, nullptr);
		vkDestroyPipelineLayout(m_device, m_KarmaGuiGeneralPipelineLayout, nullptr);

		vkDestroyDescriptorPool(m_device, m_GeneralDescriptorPool, nullptr);

		// Tentative
		static_pointer_cast<VulkanTexture>(m_GeneralTexture)->~VulkanTexture();

		m_vulkanRendererAPI->ClearVulkanRendererAPI();
		
		// Tentative for the moment
		// ClearUBO();

		for (auto framebuffer : m_swapChainFrameBuffers)
		{
			vkDestroyFramebuffer(m_device, framebuffer, nullptr);
		}

		vkDestroyImageView(m_device, m_DepthImageView, nullptr);
		vkDestroyImage(m_device, m_DepthImage, nullptr);
		vkFreeMemory(m_device, m_DepthImageMemory, nullptr);

		vkDestroyCommandPool(m_device, m_commandPool, nullptr);
		vkDestroyRenderPass(m_device, m_renderPass, nullptr);
		for (auto imageView : m_swapChainImageViews)
		{
			vkDestroyImageView(m_device, imageView, nullptr);
		}
		vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
		vkDestroyDevice(m_device, nullptr);
		if (bEnableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(m_Instance, debugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(m_Instance, m_surface, nullptr);

		vkDestroyInstance(m_Instance, nullptr);

		glslang::FinalizeProcess();
	}

	void VulkanContext::CleanUpKarmaGuiGeneralGraphicsPipeline()
	{
		vkDestroyPipeline(m_device, m_KarmaGuiGeneralGraphicsPipeline, nullptr);
	}

	void VulkanContext::RegisterUBO(VulkanUniformBuffer* ubo)
	{
		m_VulkanUBO.insert(ubo);
	}

	void VulkanContext::UploadUBO(size_t frameIndex)
	{
		for (auto ubo : m_VulkanUBO)
		{
			ubo->UploadUniformBuffer(frameIndex);
		}
	}

	void VulkanContext::ClearUBO()
	{
		for (auto ubo : m_VulkanUBO)
		{
			ubo->ClearBuffer();
		}
	}

	void VulkanContext::RecreateUBO()
	{
		for (auto ubo : m_VulkanUBO)
		{
			ubo->BufferCreation();
		}
	}

	void VulkanContext::Init()
	{
		CreateInstance();
		SetupDebugMessenger();
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateCommandPool();
		CreateDepthResources();
		CreateFrameBuffers();

		VulkanHolder::SetVulkanContext(this);

		m_vulkanRendererAPI->CreateSynchronicity();

		// For glslang
		Initializeglslang();

		CreateGeneralShader();
		CreateGeneralTexture();
		
		CreateGeneralDescriptorSetLayouts();
	}

	void VulkanContext::CreateVulkanResourcesForScene(std::shared_ptr<Scene> scene3D)
	{
		uint32_t smElementsNumber = 0;

		for (const auto element : scene3D->GetSMActors())
		{
			smElementsNumber++;
		}

		RendererAPI* rendererAPI = RenderCommand::GetRendererAPI();
		VulkanRendererAPI* vulkanRendererAPI = static_cast<VulkanRendererAPI*>(rendererAPI);

		if (!vulkanRendererAPI)
		{
			KR_CORE_ASSERT(false, "VulkanRendererAPI is null!");
			return;
		}

		uint32_t maxFramesInFlight = static_cast<uint32_t>(vulkanRendererAPI->GetMaxFramesInFlight());

		CreateGeneralDescriptorPool(smElementsNumber);

		CreateGeneralDescriptorSets(scene3D, smElementsNumber, maxFramesInFlight);
		
		for (uint32_t frameIndex = 0; frameIndex < maxFramesInFlight; frameIndex++)
		{
			UpdateGeneralDescriptorSets(scene3D, frameIndex);
		}
	}

	void VulkanContext::CreateGeneralShader()
	{
		// We are creating general shader here for the static material (material used as default for meshes)
		// Ponder how this would look like in OpenGL
		m_GeneralShader.reset(new VulkanShader("../Resources/Shaders/shader.vert", "../Resources/Shaders/shader.frag"));
	}

	void VulkanContext::CreateGeneralTexture()
	{
		m_GeneralTexture.reset(new Texture(Karma::TextureType::Image, "../Resources/Textures/UnrealGrid.png", "VikingTex", "texSampler"));
	}

	VkShaderModule VulkanContext::CreateShaderModule(const std::vector<uint32_t>& code)
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

	void VulkanContext::CreateKarmaGuiGeneralGraphicsPipeline(VkRenderPass renderPassKG, float windowKGWidth, float windowKGHeight)
	{
		std::array<VkDescriptorSetLayout, 3> setLayouts = {
									m_ViewLayout,    // set = 0
									m_TextureLayout, // set = 1
									m_ObjectLayout   // set = 2
			};

		VkPipelineLayoutCreateInfo plInfo{};
		plInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		plInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
		plInfo.pSetLayouts = setLayouts.data();
		plInfo.pushConstantRangeCount = 0;
		plInfo.pPushConstantRanges = nullptr;

		VkResult result = vkCreatePipelineLayout(m_device, &plInfo, nullptr, &m_KarmaGuiGeneralPipelineLayout);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create pipeline layout!");

		VkShaderModule vertShaderModule = CreateShaderModule(m_GeneralShader->GetVertSpirV());
		VkShaderModule fragShaderModule = CreateShaderModule(m_GeneralShader->GetFragSpirV());

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

		// Telling vulkan what to expect from vertex data in terms of attributes and their rate of loading
		uint32_t index = 0;

		// See Mesh::GaugeVertexDataLayout to understand the layout of vertex data we are using
		BufferLayout layout;
		layout.PushElement(BufferElement(ShaderDataType::Float3, "v_Position"));
		layout.PushElement(BufferElement(ShaderDataType::Float2, "v_UV"));
		
		layout.PushElement(BufferElement(ShaderDataType::Float3, "v_Normal"));
		/*layout.PushElement(BufferElement(ShaderDataType::Float4, "v_Color"));
		layout.PushElement(BufferElement(ShaderDataType::Float3, "v_Normal"));
		layout.PushElement(BufferElement(ShaderDataType::Float3, "v_Tangent"));*/

		VkVertexInputBindingDescription bindingDescription = {};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

		bindingDescription.binding = 0;
		bindingDescription.stride = layout.GetStride();
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		for (const auto& element : layout)
		{
			VkVertexInputAttributeDescription elementAttributeDescription{};
			elementAttributeDescription.binding = 0;
			elementAttributeDescription.location = index;
			elementAttributeDescription.format = ShaderDataTypeToVulkanType(element.Type);
			elementAttributeDescription.offset = static_cast<uint32_t>(element.Offset);

			attributeDescriptions.push_back(elementAttributeDescription);
			index++;
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = windowKGWidth;
		viewport.height = windowKGHeight;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent.width = windowKGWidth;
		scissor.extent.height = windowKGHeight;

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
		rasterizer.cullMode = VK_CULL_MODE_NONE;
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
		pipelineInfo.layout = m_KarmaGuiGeneralPipelineLayout;
		pipelineInfo.renderPass = renderPassKG;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.pDepthStencilState = &depthStencil;

		VkResult resultGP = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE,
			1, &pipelineInfo, nullptr, &m_KarmaGuiGeneralGraphicsPipeline);

		KR_CORE_ASSERT(resultGP == VK_SUCCESS, "Failed to create graphics pipeline!");


		vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
		vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
	}

	void VulkanContext::CreateGeneralDescriptorPool(uint32_t smElementsNumber)
	{
		RendererAPI* rendererAPI = RenderCommand::GetRendererAPI();
		VulkanRendererAPI* vulkanRendererAPI = static_cast<VulkanRendererAPI*>(rendererAPI);

		if (!vulkanRendererAPI)
		{
			KR_CORE_ASSERT(false, "VulkanRendererAPI is null!");
			return;
		}

		uint32_t maxFramesInFlight = static_cast<uint32_t>(vulkanRendererAPI->GetMaxFramesInFlight());
		
		std::array<VkDescriptorPoolSize, 2> poolSizes{};

		// Uniform Buffers : Camera UBO + per object UBOs
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = (1 + smElementsNumber) * maxFramesInFlight; // 1 for camera UBO + 1 for object UBO

		// Combined Image Samplers : Texture
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = smElementsNumber * maxFramesInFlight; // 1 for each object texture

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 3 * maxFramesInFlight; // 3 sets per frame

		VkResult result = vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_GeneralDescriptorPool);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create general descriptor pool!");
	}

	void VulkanContext::CreateGeneralDescriptorSetLayouts()
	{
		// ===== Set 0: Camera UBO =====
		VkDescriptorSetLayoutBinding viewBinding{};
		viewBinding.binding = 0;
		viewBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		viewBinding.descriptorCount = 1;
		viewBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		viewBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo viewLayoutInfo{};
		viewLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		viewLayoutInfo.bindingCount = 1;
		viewLayoutInfo.pBindings = &viewBinding;
		VkResult result = vkCreateDescriptorSetLayout(m_device, &viewLayoutInfo, nullptr, &m_ViewLayout);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create view descriptor set layout!");

		// === SET 1: MATERIAL UBO + 1 TEXTURE ===
		/*std::array<VkDescriptorSetLayoutBinding, 2> materialBindings{};

		// Binding 0: Material UBO
		materialBindings[0].binding = 0;
		materialBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		materialBindings[0].descriptorCount = 1;
		materialBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;

		// Binding 1: SINGLE Texture
		materialBindings[1].binding = 1;
		materialBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		materialBindings[1].descriptorCount = 1;
		materialBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo materialLayoutInfo{};
		materialLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		materialLayoutInfo.bindingCount = 2;  // Only 2 bindings now
		materialLayoutInfo.pBindings = materialBindings.data();
		vkCreateDescriptorSetLayout(device, &materialLayoutInfo, nullptr, &materialLayout);

		GLSLANG USAGE EXAMPLE:
		// Set 1: Material UBO + 1 Texture
		layout(set = 1, binding = 0) uniform MaterialUBO {
					 vec4 baseColor;
					float metallic;
					float roughness;
				} material;

		layout(set = 1, binding = 1) uniform sampler2D albedoTexture;  // Single texture!
		*/

		// ===== Set 1: Single texture (combined image + sampler) =====
		VkDescriptorSetLayoutBinding texBinding{};
		texBinding.binding = 0;
		texBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		texBinding.descriptorCount = 1;
		texBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		texBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo texInfo{};
		texInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		texInfo.bindingCount = 1;
		texInfo.pBindings = &texBinding;

		result = vkCreateDescriptorSetLayout(m_device, &texInfo, nullptr, &m_TextureLayout);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create texture descriptor set layout!");

		// ===== Set 2: Per-mesh transform UBO  =====
		VkDescriptorSetLayoutBinding objectBinding{};
		objectBinding.binding = 0;
		objectBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		objectBinding.descriptorCount = 1;
		objectBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		objectBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo objInfo{};
		objInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		objInfo.bindingCount = 1;
		objInfo.pBindings = &objectBinding;

		result = vkCreateDescriptorSetLayout(m_device, &objInfo, nullptr, &m_ObjectLayout);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create object descriptor set layout!");
	}

	void VulkanContext::CreateGeneralDescriptorSets(std::shared_ptr<Scene> scene3D, uint32_t smElementsNumber, uint32_t maxFramesInFlight)
	{	
		m_GeneralDescriptorSets.resize(maxFramesInFlight);

		for (uint32_t frameIndex = 0; frameIndex < maxFramesInFlight; frameIndex++)
		{
			std::array<VkDescriptorSet, 3> frameSets;

			std::array<VkDescriptorSetLayout, 3> frameLayouts = {
											m_ViewLayout,     // Set 0
											m_TextureLayout,  // Set 1  
											m_ObjectLayout    // Set 2
			};

			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = m_GeneralDescriptorPool;
			allocInfo.descriptorSetCount = 3;
			allocInfo.pSetLayouts = frameLayouts.data();

			vkAllocateDescriptorSets(m_device, &allocInfo, frameSets.data());

			// Copy to global struct
			m_GeneralDescriptorSets[frameIndex].viewSet = frameSets[0];
			//m_GeneralDescriptorSets[frameIndex].textureSet = frameSets[1];
			//m_GeneralDescriptorSets[frameIndex].objectSet = frameSets[2];

			m_GeneralDescriptorSets[frameIndex].textureSet.resize(smElementsNumber);
			m_GeneralDescriptorSets[frameIndex].objectsSet.resize(smElementsNumber);

			uint32_t meshIndex = 0;
			for (const auto element : scene3D->GetSMActors())
			{
				m_GeneralDescriptorSets[frameIndex].textureSet[/*element->GetSMID()*/meshIndex] = frameSets[1];
				m_GeneralDescriptorSets[frameIndex].objectsSet[/*element->GetSMID()*/meshIndex] = frameSets[2];
			}
		}
	}

	void VulkanContext::UpdateGeneralDescriptorSets(std::shared_ptr<Scene> scene3D, uint32_t frameIndex)
	{
		FrameDescriptorSets& frameDescriptorSets = m_GeneralDescriptorSets[frameIndex];

		// Update Set 0: Camera UBO
		{
			std::shared_ptr<VulkanUniformBuffer> vUBO = static_pointer_cast<VulkanUniformBuffer>(scene3D->GetSceneCamera()->GetViewProjectionUBO());

			VkDescriptorBufferInfo viewInfo{};
			viewInfo.buffer = vUBO->GetUniformBuffers()[frameIndex];
			viewInfo.offset = 0;
			viewInfo.range = vUBO->GetBufferSize();

			VkWriteDescriptorSet viewWrite{};
			viewWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			viewWrite.dstSet = frameDescriptorSets.viewSet;
			viewWrite.dstBinding = 0;
			viewWrite.dstArrayElement = 0;
			viewWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			viewWrite.descriptorCount = 1;
			viewWrite.pBufferInfo = &viewInfo;

			vkUpdateDescriptorSets(m_device, 1, &viewWrite, 0, nullptr);
		}

		uint32_t smIndex = 0;
		for (const auto smElement : scene3D->GetSMActors())
		{
			// Update set 1: Texture
			{
				std::shared_ptr<VulkanTexture> vTexture = m_GeneralTexture->GetVulkanTexture();

				VkDescriptorImageInfo texInfo{};
				texInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				texInfo.sampler = vTexture->GetImageSampler();
				texInfo.imageView = vTexture->GetImageView();

				VkWriteDescriptorSet texWrite{};
				texWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				texWrite.dstSet = frameDescriptorSets.textureSet[smIndex];
				texWrite.dstBinding = 0;
				texWrite.dstArrayElement = 0;
				texWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				texWrite.pImageInfo = &texInfo;
				texWrite.descriptorCount = 1;

				vkUpdateDescriptorSets(m_device, 1, &texWrite, 0, nullptr);
			}

			// Update set 2: Per-mesh UBO will be updated during mesh rendering
			{
				std::shared_ptr<VulkanUniformBuffer> objectUBO = static_pointer_cast<VulkanUniformBuffer>(smElement->GetMeshTransformUniform());

				VkDescriptorBufferInfo objectInfo{};
				objectInfo.buffer = objectUBO->GetUniformBuffers()[frameIndex];
				objectInfo.offset = 0;
				objectInfo.range = objectUBO->GetBufferSize();

				VkWriteDescriptorSet objectWrite{};
				objectWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				objectWrite.dstSet = frameDescriptorSets.objectsSet[smIndex];
				objectWrite.dstBinding = 0;
				objectWrite.dstArrayElement = 0;
				objectWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				objectWrite.pBufferInfo = &objectInfo;
				objectWrite.descriptorCount = 1;

				vkUpdateDescriptorSets(m_device, 1, &objectWrite, 0, nullptr);
			}

			smIndex++;
		}
	}

	void VulkanContext::Initializeglslang()
	{
		bool result = glslang::InitializeProcess();
		KR_CORE_INFO("glslang status = {0}", result ? "true" : "false");
	}

	void VulkanContext::SwapBuffers()
	{
	}

	void VulkanContext::RecreateSwapChain()
	{
		CleanupSwapChain();

		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateDepthResources();
		CreateFrameBuffers();
	}

	void VulkanContext::CreateDepthResources()
	{
		VkFormat depthFormat = FindDepthFormat();

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(m_swapChainExtent.width);
		imageInfo.extent.height = static_cast<uint32_t>(m_swapChainExtent.height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = depthFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		VkResult result = vkCreateImage(m_device, &imageInfo, nullptr, &m_DepthImage);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create depthimage!");

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(m_device, m_DepthImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VkResult result1 = vkAllocateMemory(m_device, &allocInfo, nullptr, &m_DepthImageMemory);
		KR_CORE_ASSERT(result1 == VK_SUCCESS, "Failed to allocate depth image memeory");

		vkBindImageMemory(m_device, m_DepthImage, m_DepthImageMemory, 0);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_DepthImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = depthFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		result = vkCreateImageView(m_device, &viewInfo, nullptr, &m_DepthImageView);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create depth imageview");
	}

	VkFormat VulkanContext::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);
			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		KR_CORE_ASSERT(false, "Failed to find supported format!");
		return VkFormat{};
	}

	VkFormat VulkanContext::FindDepthFormat()
	{
		return FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	bool VulkanContext::HasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	uint32_t VulkanContext::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		KR_CORE_ASSERT(false, "Failed to find suitable memory type for imagebuffer");
		return 0;
	}

	void VulkanContext::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

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

		vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_graphicsQueue);

		vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
	}

	/*
	void VulkanContext::CreateTextureImageView()
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_TextureImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(m_device, &viewInfo, nullptr, &m_TextureImageView);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create texture image view");
	}
	*/

	void VulkanContext::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

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

		vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_graphicsQueue);

		vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
	}

	void VulkanContext::CreateCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VkResult result = vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create command pool!");
	}

	void VulkanContext::CleanupSwapChain()
	{
		for (auto framebuffer : m_swapChainFrameBuffers)
		{
			vkDestroyFramebuffer(m_device, framebuffer, nullptr);
		}

		vkDestroyImageView(m_device, m_DepthImageView, nullptr);
		vkDestroyImage(m_device, m_DepthImage, nullptr);
		vkFreeMemory(m_device, m_DepthImageMemory, nullptr);

		vkDestroyRenderPass(m_device, m_renderPass, nullptr);
		for (auto imageView : m_swapChainImageViews)
		{
			vkDestroyImageView(m_device, imageView, nullptr);
		}
		vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
	}

	void VulkanContext::CreateFrameBuffers()
	{
		m_swapChainFrameBuffers.resize(m_swapChainImages.size());

		for (size_t i = 0; i < m_swapChainImages.size(); i++)
		{
			std::array<VkImageView, 2> attachments = { m_swapChainImageViews[i], m_DepthImageView };

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = m_swapChainExtent.width;
			framebufferInfo.height = m_swapChainExtent.height;
			framebufferInfo.layers = 1;

			VkResult result = vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapChainFrameBuffers[i]);

			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create frame buffer");
		}
	}

	void VulkanContext::CreateSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice);

		// KarmaGui may have, MAY, different requirements.
		m_surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		m_presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);// Analogous to v-sync

		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

		m_MinImageCount = swapChainSupport.capabilities.minImageCount;
		uint32_t imageCount = m_MinImageCount + 1;

		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = m_surfaceFormat.format;
		createInfo.imageColorSpace = m_surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(),
			indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = m_presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VkResult result = vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create swapchain!");

		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
		m_swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());

		m_swapChainImageFormat = m_surfaceFormat.format;
		m_swapChainExtent = extent;
	}

	void VulkanContext::CreateImageViews()
	{
		m_swapChainImageViews.resize(m_swapChainImages.size());

		for (size_t i = 0; i < m_swapChainImages.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			VkResult result = vkCreateImageView(m_device, &createInfo, nullptr, &m_swapChainImageViews[i]);

			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create image views!");
		}
	}

	void VulkanContext::CreateSurface()
	{
		VkResult result = glfwCreateWindowSurface(m_Instance, m_windowHandle, nullptr, &m_surface);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create window surface");
	}

	void VulkanContext::CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(),
		indices.presentFamily.value() };

		if (bEnableValidationLayers)
		{
			KR_CORE_INFO("+-------------------------------------------------");
			KR_CORE_INFO("| Available Unique Queue Family Indices (Graphics Card):");
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
		if (m_SupportedDeviceFeatures.logicOp)
		{
			deviceFeatures.logicOp = VK_TRUE;
		}

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = DeviceExtensions.data();

		if (bEnableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
			createInfo.ppEnabledLayerNames = ValidationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		VkResult result = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create logical device!");

		vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
		vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
	}

	void VulkanContext::PickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			KR_CORE_ASSERT(false, "Failed to load GPU with Vulkan support");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

		if (bEnableValidationLayers)
		{
			PrintAvailablePhysicalDevices(devices);
		}

		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device))
			{
				m_physicalDevice = device;
				break;
			}
		}

		if (m_physicalDevice == VK_NULL_HANDLE)
		{
			KR_CORE_ASSERT(false, "Failed to find a suitable GPU!");
		}
	}

	void VulkanContext::PrintAvailablePhysicalDevices(const std::vector<VkPhysicalDevice>& physicalDevices)
	{
		uint32_t index = 1;
		KR_CORE_INFO("+-------------------------------------------------");
		KR_CORE_INFO("| Available Graphics cards:");
		for (auto physicalDevice : physicalDevices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
			KR_CORE_INFO("| {0}. Device Name: {1}", index++, deviceProperties.deviceName);
		}
		KR_CORE_INFO("+-------------------------------------------------");
	}

	bool VulkanContext::IsDeviceSuitable(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices = FindQueueFamilies(device);

		bool bExtensionsSupported = CheckDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (bExtensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		vkGetPhysicalDeviceFeatures(device, &m_SupportedDeviceFeatures);

		return indices.IsComplete() && bExtensionsSupported && swapChainAdequate && m_SupportedDeviceFeatures.samplerAnisotropy;
	}

	// Check if all the required extensions are there
	bool VulkanContext::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

#ifdef KR_MAC_PLATFORM
		// Case by case query for required extensions
		// One for MacOS: VK_KHR_portability_subset
		for (auto anExtention : availableExtensions)
		{
			if (strcmp(anExtention.extensionName, "VK_KHR_portability_subset") != 0)
			{
				DeviceExtensions.push_back("VK_KHR_portability_subset");
				break;
			}
		}
#endif

		std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

		if (bEnableValidationLayers)
		{
			KR_CORE_INFO("+-------------------------------------------------");
			KR_CORE_INFO("| Available Extensions:");
			uint32_t index = 1;
			for (auto anExtension : availableExtensions)
			{
				KR_CORE_INFO("| {0}. {1}", index++, anExtension.extensionName);
			}
			KR_CORE_INFO("+-------------------------------------------------");
			KR_CORE_INFO("+-------------------------------------------------");
			KR_CORE_INFO("| Required Extensions (shall be enabled...):");
			index = 1;
			for (auto swapchainExtension : requiredExtensions)
			{
				KR_CORE_INFO("| {0}. {1}", index++, swapchainExtension);
			}
			KR_CORE_INFO("+-------------------------------------------------");
		}

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	QueueFamilyIndices VulkanContext::FindQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			if (indices.IsComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

	SwapChainSupportDetails VulkanContext::QuerySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	void VulkanContext::CreateInstance()
	{
		if (bEnableValidationLayers)
		{
			PrintAvailableExtensions();
		}

		if (bEnableValidationLayers && !CheckValidationLayerSupport())
		{
			KR_CORE_WARN("Validation layers requested, but not available");
		}

		// Optional information about the application (or Engine in our case)
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
#ifdef KR_APPLICATION_NAME
		appInfo.pApplicationName = KR_APPLICATION_NAME;
#else
		appInfo.pApplicationName = "No Name";
#endif
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Karma";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		// Tell Vulkan which global extensions and validation layers we want to use
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// Validation layers
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (bEnableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
			createInfo.ppEnabledLayerNames = ValidationLayers.data();

			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)(&debugCreateInfo);
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		VkInstanceCreateFlags flagsToBeSet{};
		auto extensions = GetRequiredExtensions(flagsToBeSet);
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		createInfo.flags |= flagsToBeSet;

		VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan m_Instance.");
	}

	void VulkanContext::PrintAvailableExtensions()
	{
		uint32_t extensionCount = 0;

		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> vulkanExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vulkanExtensions.data());

		uint32_t index = 1;
		KR_CORE_INFO("+-------------------------------------------------");
		KR_CORE_INFO("| Available Vulkan instance extensions:");
		for (auto extension : vulkanExtensions)
		{
			KR_CORE_INFO("| {0}. {1}", index++, extension.extensionName);
		}
		KR_CORE_INFO("+-------------------------------------------------");
	}

	bool VulkanContext::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : ValidationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	// Return the required list of instance extensions based on whether validation layers are
	// enabled or not
	std::vector<const char*> VulkanContext::GetRequiredExtensions(VkInstanceCreateFlags& flagsToBeSet)
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef KR_MAC_PLATFORM
		// Case by case query for required instance extensions
		// One for MacOS: VK_KHR_portability_enumeration
		uint32_t extensionCount = 0;

		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> vulkanExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vulkanExtensions.data());

		uint32_t index = 1;
		for (auto anExtension : vulkanExtensions)
		{
			if (strcmp(anExtension.extensionName, "VK_KHR_portability_enumeration"))
			{
				extensions.push_back("VK_KHR_portability_enumeration");
				flagsToBeSet = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
				break;
			}
		}
#endif
		if (bEnableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

			uint32_t index = 1;
			KR_CORE_INFO("+-------------------------------------------------");
			KR_CORE_INFO("| GLFW and other required instance extensions:");
			for (auto extension : extensions)
			{
				KR_CORE_INFO("| {0}. {1}", index++, extension);
			}
			KR_CORE_INFO("+-------------------------------------------------");
		}

		return extensions;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			KR_CORE_INFO("Validation Layer: {0}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			KR_CORE_WARN("Validation Layer: {0}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			KR_CORE_ERROR("Validation Layer: {0}", pCallbackData->pMessage);
			break;
		default:
			//KR_CORE_TRACE("Validation Layer: {0}", pCallbackData->pMessage);
			break;
		}

		return VK_FALSE;
	}

	void VulkanContext::SetupDebugMessenger()
	{
		if (!bEnableValidationLayers)
		{
			return;
		}

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo(createInfo);

		VkResult result = CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &debugMessenger);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to set up debug messenger!");
	}

	void VulkanContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;
	}

	VkResult VulkanContext::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void VulkanContext::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

	VkSurfaceFormatKHR VulkanContext::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
				&& availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR VulkanContext::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		if (!bVSync)
		{
			for (const auto& availablePresentMode : availablePresentModes)
			{
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					return availablePresentMode;
				}
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanContext::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(m_windowHandle, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::max(capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	void VulkanContext::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = FindDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create render pass!");
	}

	void VulkanContext::SetVSync(bool bEnable)
	{
		bVSync = bEnable;

		vkDeviceWaitIdle(m_device);
		RecreateSwapChain();
	}
}
