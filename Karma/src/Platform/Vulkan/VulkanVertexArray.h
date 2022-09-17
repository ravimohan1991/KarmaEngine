#pragma once

#include "Karma/Renderer/VertexArray.h"
#include "vulkan/vulkan.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Karma/Renderer/Shader.h"
#include "Platform/Vulkan/VulkanShader.h"

namespace Karma
{
	class KARMA_API VulkanVertexArray : public VertexArray, std::enable_shared_from_this<VulkanVertexArray>
	{
	public:
		VulkanVertexArray();
		virtual ~VulkanVertexArray();

		virtual void Bind() const override;
		virtual void UnBind() const override {}

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

		virtual void SetMesh(std::shared_ptr<Mesh> mesh) override;

		virtual void SetMaterial(std::shared_ptr<Material> material) override;

		virtual void SetShader(std::shared_ptr<Shader> shader) override;

		void CreateDescriptorSetLayout();
		void CreatePipelineLayout();
		void CreateGraphicsPipeline();
		void CreateDescriptorPool();
		void CreateDescriptorSets();

		//void CreateCommandBuffers();

		void GenerateVulkanVA();

		void RecreateVulkanVA();
		void CleanupPipeline();

		// Helper functions
		VkShaderModule CreateShaderModule(const std::vector<uint32_t>& code);

		// Getters
		VkPipeline GetGraphicsPipeline() const { return m_graphicsPipeline; }
		VkPipelineLayout GetGraphicsPipelineLayout() const { return m_pipelineLayout; }
		const std::shared_ptr<VulkanShader>& GetShader() const { return m_Shader; }
		//const std::vector<VkDescriptorSet>& GetUBDescriptorSets() const { return m_descriptorSets; }
		const std::shared_ptr<VulkanVertexBuffer>& GetVertexBuffer() const { return m_VertexBuffer; }
		const std::vector<VkDescriptorSet>& GetDescriptorSets() const { return m_descriptorSets; }

		virtual std::shared_ptr<Material> GetMaterial() const override { return m_Materials.at(0); }

		virtual void UpdateProcessAndSetReadyForSubmission() const override;

		// Overrides
		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		virtual const VulkanIndexBuffer* GetIndexBuffer() const override { return m_IndexBuffer.get(); }

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
		VkDescriptorPool m_descriptorPool;
		std::vector<VkDescriptorSet> m_descriptorSets;

		VkVertexInputBindingDescription m_bindingDescription{};
		std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;

		const VkPhysicalDeviceFeatures& m_SupportedDeviceFeatures;
	};

}
