/**
 * @file VulkanBuffer.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains VulkanVertex/Index/ImageBuffer classes.
 * @version 1.0
 * @date January 10, 2021
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "Karma/Renderer/Buffer.h"
#include "vulkan/vulkan.h"

namespace Karma
{
	/**
	 * @brief Vulkan's vertex buffer class. Vertex buffer is used in agnostic Mesh instance
	 *
	 * @since Karma 1.0.0
	 */
	class KARMA_API VulkanVertexBuffer : public VertexBuffer
	{
	public:
		/**
		 * @brief Constructor
		 *
		 * The single vertex buffer works correctly, but the memory type that allows us to access it from the CPU may not be the most optimal memory type for the
		 * graphics card itself to read from. The most optimal memory has the VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT flag and is usually not accessible by the CPU
		 * on dedicated graphics cards. In this chapter (https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer) we're going to create two vertex buffers.
		 * One staging buffer in CPU accessible memory to upload the data from the vertex array to, and the final vertex buffer in device (GPU) local memory. We'll then use a buffer copy
		 * command to move the data from the staging buffer to the actual vertex buffer.
		 *
		 * @param vertices						float array of interleaved vertex data (including position, uv, color, normal, and tangent) based on the BufferLayout
		 * @param size							Size (in bytes) of the vertex buffer (number of mesh vertices * sum of each vertex attribute's size). For instance:
		 *										@code{}
		 * 											float vertices[3 * 7] = {
		 * 														-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 * 														0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 * 														0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
		 * 											};
		 *										@endcode
		 * 										will have size = 3 * (7 * sizeof(float)).
		 *
		 * @see Mesh::DealVertexIndexBufferData, VulkanVertexBuffer::CreateBuffer
		 * @since Karma 1.0.0
		 */
		VulkanVertexBuffer(float* vertices, uint32_t size);

		/**
		 * @brief Destructor
		 *
		 * Deletes the buffers and clears up Vulkan relevant resources
		 *
		 * @since Karma 1.0.0
		 */
		virtual ~VulkanVertexBuffer();

		/**
		 * @brief Not useful for Vulkan API atm
		 *
		 * @todo Ponder over Vulkan equivalent of glBindBuffer
		 * @since Karma 1.0.0
		 */
		virtual void Bind() const override;

		/**
		 * @brief Not useful for Vulkan API atm
		 *
		 * @todo Ponder over Vulkan equivalent of glBindBuffer(GL_ARRAY_BUFFER, 0);
		 * @since Karma 1.0.0
		 */
		virtual void UnBind() const override;

		/**
		 * @brief Getter for the layout of the vertex buffer
		 *
		 * @since Karma 1.0.0
		 */
		virtual const BufferLayout& GetLayout() const override
		{
			return m_Layout;
		}

		/**
		 * @brief Sets the layout of the vertexbuffer
		 *
		 * @param layout						The reference to layout to be set
		 *
		 * @since Karma 1.0.0
		 */
		virtual void SetLayout(const BufferLayout& layout) override
		{
			m_Layout = layout;
		}

		/**
		 * @brief Actual creation of buffer using Vulkan API. Memory is also allocated appropriately.
		 *
		 * @param size								The size (in bytes) of the buffer object to be created
		 * @param usage								The bitmask of VkBufferUsageFlagBits (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBufferUsageFlagBits.html) specifying allowed usages of the buffer.
		 * @param properties						The bitmask of VkMemoryPropertyFlagBits of properties for this memory type of the buffer.
		 * @param buffer							The pointer to a VkBuffer handle in which the resulting buffer object is returned.
		 * @param bufferMemory						The pointer to a VkDeviceMemory handle in which information about the allocated memory is returned.
		 *
		 * @since Karma 1.0.0
		 */
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
			VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		/**
		 * @brief Copy buffer
		 *
		 * @param srcBuffer							The source buffer to copy data from
		 * @param dstBuffer							The destination buffer to copy data to
		 * @param size								The total size of data in bytes
		 * @since Karma 1.0.0
		 */
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		/**
		 * @brief Finds appropriate memory type with demanded properties. Basically a loop is run from counter i = 0 to VkPhysicalDeviceMemoryProperties.memoryTypeCount
		 * (number of valid elements in the memoryTypes array) and memoryType[i] is queried for appropriate properties. On condition satisfaction, counter i is returned.
		 *
		 * Graphics cards can offer different types of memory to allocate from. Each type of memory varies in terms of allowed operations and performance characteristics.
		 * We need to combine the requirements of the buffer and our own application requirements to find the right type of memory to use.
		 *
		 * @param typeFilter								A bitmask, and contains one bit set for every supported memory type for the resource. Bit i is set if and only if the memory type i in the VkPhysicalDeviceMemoryProperties structure for the physical device is supported for the resource.
		 * @param properties								The demanded properties (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBufferUsageFlagBits.html).
		 *
		 * @return Memory type index i
		 * @see VulkanUniformBuffer::CreateBuffer
		 * @since Karma 1.0.0
		 */
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		/**
		 * @brief Getter for vertex buffer.
		 *
		 * @since Karma 1.0.0
		 */
		inline VkBuffer GetVertexBuffer() const { return m_VertexBuffer; }

		/**
		 * @brief Getter for vertex buffer memory
		 *
		 * @since Karma 1.0.0
		 */
		inline VkDeviceMemory GetVertexBufferMemory() const { return m_VertexBufferMemory; }

		/**
		 * @brief Getter for buffer size (in bytes)
		 */
		inline size_t GetBufferSize() { return m_BufferSize; }

	private:
		VkDevice m_Device;
		BufferLayout m_Layout;

		VkBuffer m_VertexBuffer;
		VkDeviceMemory m_VertexBufferMemory;

		size_t m_BufferSize;
	};

	/**
	 * @brief Vulkan's index buffer class, used in agnostic Mesh instance
	 *
	 * @since Karma 1.0.0
	 */
	class KARMA_API VulkanIndexBuffer : public IndexBuffer
	{
	public:
		/**
		 * @brief Constructor
		 *
		 * Initializes the relevant buffers and uploads them to GPU.
		 *
		 * @param indices						The array of floats containing indexbuffer data
		 * @param count							The length of the indexbuffer array
		 *
		 * @see VulkanVertexBuffer::VulkanVertexBuffer for the role played by the staging buffer.
		 * @since Karma 1.0.0
		 */
		VulkanIndexBuffer(uint32_t* indices, uint32_t count);

		/**
		 * @brief Destructor involving destruction of index buffer and freeing up of memory
		 *
		 * @since Karma 1.0.0
		 */
		virtual ~VulkanIndexBuffer();

		/**
		 * @brief Not useful for Vulkan API atm.
		 *
		 * @todo Ponder over Vulkan equivalent of glBindBuffer
		 * @since Karma 1.0.0
		 */
		virtual void Bind() const override;

		/**
		 * @brief Not useful for Vulkan API atm.
		 *
		 * @todo Ponder over Vulkan equivalent of glBindBuffer
		 * @since Karma 1.0.0
		 */
		virtual void UnBind() const override;

		/**
		 * @brief Actual creation of buffer using Vulkan API. Memory is also allocated appropriately.
		 *
		 * @param size								The size (in bytes) of the buffer object to be created
		 * @param usage								The bitmask of VkBufferUsageFlagBits (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBufferUsageFlagBits.html) specifying allowed usages of the buffer.
		 * @param properties						The bitmask of VkMemoryPropertyFlagBits of properties for this memory type of the buffer.
		 * @param buffer							The pointer to a VkBuffer handle in which the resulting buffer object is returned.
		 * @param bufferMemory						The pointer to a VkDeviceMemory handle in which information about the allocated memory is returned.
		 *
		 * @since Karma 1.0.0
		 */
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
			VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		/**
		 * @brief Copy buffer
		 *
		 * @param srcBuffer							The source buffer to copy data from
		 * @param dstBuffer							The destination buffer to copy data to
		 * @param size								The total size of data in bytes
		 * @since Karma 1.0.0
		 */
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		/**
		 * @brief Finds appropriate memory type with demanded properties. Basically a loop is run from counter i = 0 to VkPhysicalDeviceMemoryProperties.memoryTypeCount
		 * (number of valid elements in the memoryTypes array) and memoryType[i] is queried for appropriate properties. On condition satisfaction, counter i is returned.
		 *
		 * @param typeFilter								A bitmask, and contains one bit set for every supported memory type for the resource. Bit i is set if and only if the memory type i in the VkPhysicalDeviceMemoryProperties structure for the physical device is supported for the resource.
		 * @param properties								The demanded properties (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBufferUsageFlagBits.html).
		 *
		 * @return Memory type index i
		 * @since Karma 1.0.0
		 */
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		/**
		 * @brief Getter for the number of vertices to draw.
		 *
		 * @note To be used in vkCmdDrawIndexed mostly
		 * @see KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_RenderDrawData
		 * @since Karma 1.0.0
		 */
		virtual uint32_t GetCount() const override { return m_Count; }

		/**
		 * @brief Getter for indexbuffer
		 *
		 * @since Karma 1.0.0
		 */
		inline VkBuffer GetIndexBuffer() const { return m_IndexBuffer; }

		/**
		 * @brief Getter for index buffer memory
		 *
		 * @since Karma 1.0.0
		 */
		inline VkDeviceMemory GetIndexBufferMemory() const { return m_IndexBufferMemory; }

		/**
		 * @brief Getter for indexbuffer size in bytes
		 *
		 * @since Karma 1.0.0
		 */
		inline size_t GetBufferSize() { return m_BufferSize; }

	private:
		VkDevice m_Device;
		uint32_t m_Count;

		VkBuffer m_IndexBuffer;
		VkDeviceMemory m_IndexBufferMemory;

		size_t m_BufferSize;
	};

	/**
	 * @brief The actual uniform buffer class with implemented function using Vulkan API
	 *
	 * @since Karma 1.0.0
	 */
	struct KARMA_API VulkanUniformBuffer : public UniformBufferObject
	{
	public:
		/**
		 * @brief Constructor for Vulkan buffer. Calls VulkanUniformBuffer::BufferCreation().
		 *
		 * @param dataTypes						List of data types for uniforms to be uploaded to GPU (like used in shaders),
		 * 								for instance https://github.com/ravimohan1991/KarmaEngine/blob/138c172ccedf31acfab982af51ae130f9a37d3bb/Application/src/KarmaApp.cpp#L39 where Mat4 are for https://github.com/ravimohan1991/KarmaEngine/blob/138c172ccedf31acfab982af51ae130f9a37d3bb/Resources/Shaders/shader.vert#L9-L13
		 * @param bindingPointIndex				The binding of shader specified index
		 *
		 * @since Karma 1.0.0
		 */
		VulkanUniformBuffer(std::vector<ShaderDataType> dataTypes, uint32_t bindingPointIndex);

		/**
		 * @brief Destructor. For freeing up UBO resources, use VulkanUniformBuffer::ClearBuffer()
		 *
		 * @see VulkanContext::ClearUBO
		 * @since Karma 1.0.0
		 */
		virtual ~VulkanUniformBuffer();

		/**
		 * @brief Creates Vulkan buffer (VKbuffer) for ubo use and allocates device memory appropriately
		 *
		 * @param size							The  size, in bytes, of the buffer to be created
		 * @param properties					The demanded properties (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBufferUsageFlagBits.html). For instance (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
		 *
		 * @see VulkanUniformBuffer::BufferCreation()
		 * @since Karma 1.0.0
		 */
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
			VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		/**
		 * @brief Finds appropriate memory type with demanded properties. Basically a loop is run from counter i = 0 to VkPhysicalDeviceMemoryProperties.memoryTypeCount
		 * (number of valid elements in the memoryTypes array) and memoryType[i] is queried for appropriate properties. On condition satisfaction, counter i is returned.
		 *
		 * @param typeFilter								A bitmask, and contains one bit set for every supported memory type for the resource. Bit i is set if and only if the memory type i in the VkPhysicalDeviceMemoryProperties structure for the physical device is supported for the resource.
		 * @param properties								The demanded properties (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBufferUsageFlagBits.html).
		 *
		 * @return Memory type index i
		 * @since Karma 1.0.0
		 */
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		/**
		 * @brief Getter for the uniform buffers
		 *
		 * @since Karma 1.0.0
		 */
		const std::vector<VkBuffer>& GetUniformBuffers() const { return m_UniformBuffers; }

		/**
		 * @brief Frees up the resources by destroying the buffer object(s) and clearing memory
		 *
		 * @since Karma 1.0.0
		 */
		void ClearBuffer();

		/**
		 * @brief Based on MAX_FRAMES_IN_FLIGHT (number of images (to work upon (CPU side) whilst an image is being rendered (GPU side processing)) + 1), uniform
		 * buffers are created with appropriate buffer size.
		 *
		 * @since Karma 1.0.0.
		 */
		void BufferCreation();

		/**
		 * @brief Uploads (copies) the data from buffer memory (m_UniformBuffersMemory) to  host-accessible (CPU) pointer, m_UniformList, to the beginning of the mapped range
		 *
		 * @param frameIndex								The m_CurrentFrame index representing index of MAX_FRAMES_IN_FLIGHT (number of images (to work upon (CPU side) whilst an image is being rendered (GPU side processing)) + 1)
		 *
		 * @see VulkanRendererAPI::SubmitCommandBuffers()
		 * @since Karma 1.0.0
		 */
		void UploadUniformBuffer(size_t frameIndex) override;

	private:
		VkDevice m_Device;
		std::vector<VkBuffer> m_UniformBuffers;
		std::vector<VkDeviceMemory> m_UniformBuffersMemory;
	};

	/**
	 * @brief Vulkan specific implemetation of ImageBuffer class.
	 */
	class KARMA_API VulkanImageBuffer : public ImageBuffer
	{
	public:
		/**
		 * @brief Creates a GPU memory buffer for storing image texture
		 *
		 * @param filename								The path to the file, including filename, containing the image texture
		 * @since Karma 1.0.0
		 */
		VulkanImageBuffer(const char* filename);

		/**
		 * @brief Frees up device resources
		 *
		 * @since Karma 1.0.0
		 */
		virtual ~VulkanImageBuffer();

		/**
		 * @brief Creates Vulkan buffer (VKbuffer) for image texture's use and allocates device memory appropriately
		 *
		 * @param size							The  size, in bytes, of the buffer to be created
		 * @param properties					The demanded properties (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBufferUsageFlagBits.html). For instance (VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
		 *
		 * @see VulkanUniformBuffer::BufferCreation()
		 * @since Karma 1.0.0
		 */
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
			VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		/**
		 * @brief Finds appropriate memory type with demanded properties. Basically a loop is run from counter i = 0 to VkPhysicalDeviceMemoryProperties.memoryTypeCount
		 * (number of valid elements in the memoryTypes array) and memoryType[i] is queried for appropriate properties. On condition satisfaction, counter i is returned.
		 *
		 * @param typeFilter								A bitmask, and contains one bit set for every supported memory type for the resource. Bit i is set if and only if the memory type i in the VkPhysicalDeviceMemoryProperties structure for the physical device is supported for the resource.
		 * @param properties								The demanded properties (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBufferUsageFlagBits.html).
		 *
		 * @return Memory type index i
		 * @since Karma 1.0.0
		 */
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		const inline VkBuffer& GetBuffer() const { return m_StagingBuffer; }

		// Getters
		/**
		 * @brief Getter for the width (in pixels?) of the image texture
		 *
		 * @since Karma 1.0.0
		 */
		int GetTextureWidth() const { return texWidth; }

		/**
		 * @brief Getter for the height (in pixels?) of the image texture
		 *
		 * @since Karma 1.0.0
		 */
		int GetTextureHeight() const { return texHeight; }

		/**
		 * @brief Getter for desired channels (if non-zero, # of image components requested in result).
		 *
		 * @see KarmaUtilities::GetImagePixelData
		 * @since Karma 1.0.0
		 */
		int GetTextureChannels() const { return texChannels; }

	private:
		VkDevice m_Device;
		VkBuffer m_StagingBuffer;
		VkDeviceMemory m_StagingBufferMemory;

		// Image props (properties)
		int texWidth;
		int texHeight;
		int texChannels;
	};
}
