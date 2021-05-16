#pragma once

#include "Karma/Renderer/VertexArray.h"
#include "vulkan/vulkan.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Karma/Renderer/Shader.h"
#include "Platform/Vulkan/VulkanShader.h"

namespace Karma
{
	class KARMA_API VulkanVertexArray : public VertexArray
	{
	public:
		VulkanVertexArray();
		virtual ~VulkanVertexArray();

		virtual void Bind() const override;
		virtual void UnBind() const override {}

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

		virtual void SetShader(std::shared_ptr<Shader> shader) override;

		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		virtual const std::shared_ptr<IndexBuffer> GetIndexBuffer() const override { return m_IndexBuffer; }

		void CreateDescriptorSetLayout();
		void CreateGraphicsPipeline();
		void CreateDescriptorPool();
		void CreateDescriptorSets();

		void CreateCommandBuffers();

		void GenerateVulkanVA();

		void RecreateSwapChainAndPipeline();
		void CleanupPipelineandCommandBuffers();

		// Helper functions
		static std::vector<char> ReadFile(const std::string& filename);
		VkShaderModule CreateShaderModule(const std::vector<uint32_t>& code);

		// Getters
		const std::vector<VkCommandBuffer>& GetCommandBuffers() const { return m_commandBuffers; }
		std::shared_ptr<VulkanShader> GetShader() const { return m_Shader; }
		const std::vector<VkDescriptorSet>& GetUBDescriptorSets() const { return m_descriptorSets; }

	private:
		std::shared_ptr<VulkanShader> m_Shader;

		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<VulkanVertexBuffer> m_VertexBuffer;
		std::shared_ptr<VulkanIndexBuffer> m_IndexBuffer;

		VkDevice m_device;

		VkDescriptorSetLayout m_descriptorSetLayout;
		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_graphicsPipeline;
		VkDescriptorPool m_descriptorPool;
		std::vector<VkDescriptorSet> m_descriptorSets;

		std::vector<VkCommandBuffer> m_commandBuffers;

		/*std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;
		std::vector<VkFence> m_imagesInFlight;*/

		VkVertexInputBindingDescription m_bindingDescription{};
		std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;

		//const int MAX_FRAMES_IN_FLIGHT = 2;
	};

}