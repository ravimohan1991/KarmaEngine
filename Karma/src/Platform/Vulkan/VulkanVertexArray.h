/**
 * @file VulkanVertexArray.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains VulkanVertexArray class which contains Vulkan specific implementation of VertexArray class.
 * @version 1.0
 * @date Jan 17, 2021
 * 
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "Karma/Renderer/VertexArray.h"
#include "vulkan/vulkan.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Karma/Renderer/Shader.h"
#include "Platform/Vulkan/VulkanShader.h"

namespace Karma
{
	/**
	 * @brief Vulkan specific implementation of VertexArray class.
	 * 
	 * This class handles the creation and management of Vulkan vertex arrays, including descriptor sets,
	 * pipeline layouts, and graphics pipelines. Furthermore the shaders, materials, vertex buffers, and index buffers
	 * are also managed here.
	 * 
	 * A VertexArray is an object, associated with a scene, containing all of the state needed to supply vertex data while drawing.
	 * 
	 * @see RenderCommand::DrawIndexed
	 * 
	 * @since Karma 1.0.0
	 */
	class KARMA_API VulkanVertexArray : public VertexArray, std::enable_shared_from_this<VulkanVertexArray>
	{
	public:
		/**
		 * @brief A constructor
		 * 
		 * @since Karma 1.0.0
		 */
		VulkanVertexArray();
		
		/**
		 * @brief A destructor
		 * 
		 * Cleans up vulkan resources associated with the vertex array including pipelines, pipeline layouts,
		 * descriptor set layouts, and descriptor pools.
		 * 
		 * @see VulkanVertexArray::CleanupPipeline()
		 * @since Karma 1.0.0
		 */
		virtual ~VulkanVertexArray();

		virtual void Bind() const override;
		virtual void UnBind() const override {}

		//////////////////////////////// LEGACY PURPOSES //////////////////////////////////
		/**
		 * @brief For legacy purposes. Use Mesh abstraction.
		 * 
		 * @param vertexBuffer				The vertex buffer to be added
		 * 
		 * @see Mesh, VulkanVertexArray::SetMesh()
		 * @since Karma 1.0.0
		 */
		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;

		/**
		 * @brief For legacy purposes. Use Mesh abstraction.
		 * 
		 * @param indexBuffer				The index buffer to be set
		 * 
		 * @see Mesh, VulkanVertexArray::SetMesh()
		 * @since Karma 1.0.0
		 */
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

		/**
		 * @brief For legacy purposes. Use Material abstraction.
		 * 
		 * @param shader					The shader to be set
		 * 
		 * @see Material, VulkanVertexArray::SetMaterial()
		 * @since Karma 1.0.0
		 */
		virtual void SetShader(std::shared_ptr<Shader> shader) override;
		
		//////////////////////////////// END LEGACY PURPOSES //////////////////////////////////
		
		/**
		 * @brief Sets the index and vertex buffers seperately
		 * 
		 * @param mesh					A collection of index and vertex buffers to be set
		 * 
		 * @see 
		 * @since Karma 1.0.0
		 */
		virtual void SetMesh(std::shared_ptr<Mesh> mesh) override;

		/**
		 * @brief Sets the material
		 * 
		 * @param material				Shader and texture setting
		 * 
		 * @since Karma 1.0.0
		 */
		virtual void SetMaterial(std::shared_ptr<Material> material) override;

		/**
		 * @brief Creates the descriptor set layout .
		 * 
		 * @see VulkanVertexArray::GenerateVulkanVA()
		 * @since Karma 1.0.0
		 */
		void CreateDescriptorSetLayout();

		/**
		 * @brief Creates the pipeline layout.
		 * 
		 * @see VulkanVertexArray::GenerateVulkanVA()
		 * @since Karma 1.0.0
		 */
		void CreatePipelineLayout();

		/**
		 * @brief Creates the graphics pipeline.
		 * 
		 * @see VulkanVertexArray::GenerateVulkanVA()
		 * @since Karma 1.0.0
		 */
		void CreateGraphicsPipeline();

		/**
		 * @brief Creates the graphics pipeline for KarmaGui window display.
		 * 
		 * @param renderPassKG				The render pass created specifically for KarmaGui offscreen texture rendering
		 * @param windowKGWidth				Width of the KarmaGui window
		 * 
		 * @param windowKGHeight			Height of the KarmaGui window
		 * 
		 * @see KarmaGuiVulkanHandler::CreateOffScreenTextureResources()
		 * @since Karma 1.0.0
		 */
        void CreateKarmaGuiGraphicsPipeline(VkRenderPass renderPassKG, float windowKGWidth, float windowKGHeight);
		
		/**
		 * @brief Creates the descriptor pool.
		 * 
		 * @see VulkanVertexArray::GenerateVulkanVA()
		 * @since Karma 1.0.0
		 */
		void CreateDescriptorPool();
		
		/**
		 * @brief Creates the descriptor sets.
		 * 
		 * In Vulkan, descriptor sets are allocated from a descriptor pool and are used to bind resources like uniform buffers and textures to shaders.
		 * Survey KarmaGuiRenderer::FrameRender to see how descriptor sets are bound before drawing.
		 * 
		 * @see VulkanVertexArray::GenerateVulkanVA()
		 * @since Karma 1.0.0
		 */
		void CreateDescriptorSets();

		/**
		 * @brief Generates the Vulkan Vertex Array by creating descriptor set layout, pipeline layout, graphics pipeline,
		 * descriptor pool, and descriptor sets.
		 * 
		 * Called when setting material or shader to setup the necessary Vulkan structures for rendering. Survey VulkanVertexArray::SetShader or
		 * VulkanVertexArray::SetMaterial.
		 * 
		 * @see VulkanVertexArray::CreateDescriptorSetLayout()
		 * @see VulkanVertexArray::CreatePipelineLayout()
		 * @see VulkanVertexArray::CreateGraphicsPipeline()
		 * @see VulkanVertexArray::CreateDescriptorPool()
		 * @see VulkanVertexArray::CreateDescriptorSets()
		 * @since Karma 1.0.0
		 */
		void GenerateVulkanVA();

		/**
		 * @brief Recreates the Vulkan Vertex Array.
		 * 
		 * Used when the swapchain is recreated (e.g. window resize) to ensure the vertex array is compatible with the new swapchain.
		 * 
		 * @see VulkanRendererAPI::RecreateCommandBuffersPipelineSwapchain(), KarmaGuiRenderer::GiveLoopBeginControlToVulkan()
		 * @since Karma 1.0.0
		 */
		void RecreateVulkanVA();

		/**
		 * @brief Cleans up the graphics pipeline and related resources (pipelinelayout and descriptorsets).
		 * 
		 * @see VulkanRendererAPI::RecreateCommandBuffersPipelineSwapchain()
		 * @since Karma 1.0.0
		 */
		void CleanupPipeline();

		/**
		 * @brief Cleans up the KarmaGui graphics pipeline (m_graphicsPipelineKGWindow).
		 * 
		 * @see KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_DestroyWindow
		 */
        void CleanupKarmaGuiGraphicsPipeline();

		/**
		 * @brief Creates a Vulkan shader module from SPIR-V bytecode.
		 * 
		 * This is specifically used when creating the graphics pipeline to load vertex and fragment shaders.
		 * 
		 * For instance while creating vertex shader stage info (pipeline creation):
		 * @code{.cpp}
		 *		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		 *		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		 *		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		 *		vertShaderStageInfo.module = vertShaderModule;
		 *		vertShaderStageInfo.pName = "main";
		 * @endcode
		 * 
		 * @param code					The SPIR-V bytecode as a vector of uint32_t
		 * 
		 * @return The created VkShaderModule
		 * @since Karma 1.0.0
		 */
		VkShaderModule CreateShaderModule(const std::vector<uint32_t>& code);

		// Getters
		VkPipeline GetGraphicsPipeline() const { return m_graphicsPipeline; }
        VkPipeline GetKarmaGuiGraphicsPipeline() const { return m_graphicsPipelineKGWindow; }
		VkPipelineLayout GetGraphicsPipelineLayout() const { return m_pipelineLayout; }
		const std::shared_ptr<VulkanShader>& GetShader() const { return m_Shader; }
		//const std::vector<VkDescriptorSet>& GetUBDescriptorSets() const { return m_descriptorSets; }
		const std::shared_ptr<VulkanVertexBuffer>& GetVertexBuffer() const { return m_VertexBuffer; }
		const std::vector<VkDescriptorSet>& GetDescriptorSets() const { return m_descriptorSets; }

		virtual std::shared_ptr<Material> GetMaterial() const override { return m_Materials.at(0); }

		virtual void UpdateProcessAndSetReadyForSubmission() const override;

		//////////////////////////////// LEGACY PURPOSES ////////////////////////////////// 

		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		virtual const VulkanIndexBuffer* GetIndexBuffer() const override { return m_IndexBuffer.get(); }

		//////////////////////////////// END LEGACY PURPOSES //////////////////////////////////
	private:
		// May need to consider batching for components of Meshes and Materials

		// Mesh relevant members
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<VulkanVertexBuffer> m_VertexBuffer;
		std::shared_ptr<VulkanIndexBuffer> m_IndexBuffer;

		// Material relevant members
		std::vector<std::shared_ptr<Material>> m_Materials;
		std::vector<std::shared_ptr<VulkanShader>> m_Shaders;
		std::shared_ptr<VulkanShader> m_Shader;

		VkDevice m_device;

		VkPipelineLayout m_pipelineLayout;
		VkDescriptorSetLayout m_descriptorSetLayout;

		VkPipeline m_graphicsPipeline;
        VkPipeline m_graphicsPipelineKGWindow;
		VkDescriptorPool m_descriptorPool;
		std::vector<VkDescriptorSet> m_descriptorSets;

		VkVertexInputBindingDescription m_bindingDescription{};
		std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;

		const VkPhysicalDeviceFeatures& m_SupportedDeviceFeatures;
	};

}
