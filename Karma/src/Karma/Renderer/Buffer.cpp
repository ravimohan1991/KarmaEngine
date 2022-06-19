#include "Buffer.h"
#include "Karma/Core.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace Karma
{
	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				KR_CORE_ASSERT(false, "RendererAPI::None is not supported");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return new OpenGLVertexBuffer(vertices, size);
			case RendererAPI::API::Vulkan:
				return new VulkanVertexBuffer(vertices, size);
		}

		KR_CORE_ASSERT(false, "Unknown RendererAPI specified");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				KR_CORE_ASSERT(false, "RendererAPI::None is not supported");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return new OpenGLIndexBuffer(indices, size);
			case RendererAPI::API::Vulkan:
				return new VulkanIndexBuffer(indices, size);
		}

		KR_CORE_ASSERT(false, "Unknown RendererAPI specified");
		return nullptr;
	}

	ImageBuffer* ImageBuffer::Create(const char* filename)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				KR_CORE_ASSERT(false, "RendererAPI::None is not supported");
				return nullptr;
			case RendererAPI::API::OpenGL:
				OpenGLImageBuffer::SetUpImageBuffer(filename);
				return nullptr;
			case RendererAPI::API::Vulkan:
				KR_CORE_ASSERT(false, "Vulkan is not supported yet!");
				return nullptr;
		}

		KR_CORE_ASSERT(false, "Unknown RendererAPI specified");
		return nullptr;
	}

	UniformBufferObject* UniformBufferObject::Create(std::vector<ShaderDataType> dataTypes, uint32_t bindingPointIndex)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				KR_CORE_ASSERT(false, "RendererAPI::None is not supported");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return new OpenGLUniformBuffer(dataTypes, bindingPointIndex);
			case RendererAPI::API::Vulkan:
				return new VulkanUniformBuffer(dataTypes, bindingPointIndex);
		}

		KR_CORE_ASSERT(false, "Unknown RendererAPI specified");
		return nullptr;
	}

	UniformBufferObject::UniformBufferObject(std::vector<ShaderDataType> dataTypes, uint32_t bindingPointIndex) :
		m_UniformDataType(dataTypes), m_BindingPoint(bindingPointIndex)
	{
		CalculateOffsetsAndBufferSize();
	}

	void UniformBufferObject::CalculateOffsetsAndBufferSize()
	{
		uint32_t uPreviousAlignedOffset = 0;
		uint32_t uPreviousSize = 0;
		uint32_t index = 0;

		for (auto& uniformDataType : m_UniformDataType)
		{
			uint32_t uniformBaseAlignment = ComputeBaseAlignment(uniformDataType);
			uint32_t uniformAlignedOffset = 0;
			uint32_t uniformSize = ShaderDataTypeSize(uniformDataType);

			if (index == 0)
			{
				uniformAlignedOffset = uPreviousAlignedOffset;
			}
			else
			{
				uniformAlignedOffset = uPreviousAlignedOffset + uPreviousSize;
				while (uniformAlignedOffset % uniformBaseAlignment != 0)
				{
					uniformAlignedOffset++;
				}
			}
			m_UniformAlignedOffsets.push_back(uniformAlignedOffset);
			m_UniformSizes.push_back(uniformSize);

			uPreviousAlignedOffset = uniformAlignedOffset;
			uPreviousSize = uniformSize;
			index++;
		}

		m_BufferSize = uPreviousAlignedOffset + uPreviousSize;
	}

	uint32_t UniformBufferObject::ComputeBaseAlignment(ShaderDataType dataType)
	{
		switch (dataType)
		{
			case Karma::ShaderDataType::None:
				KR_CORE_ASSERT(false, "None ShaderDataType not supported yet!");
				return 0;
			case Karma::ShaderDataType::Float:
				return sizeof(float);
			case Karma::ShaderDataType::Float2:
				return sizeof(glm::vec2);
			case Karma::ShaderDataType::Float3:
				return sizeof(glm::vec4);
			case Karma::ShaderDataType::Float4:
				return sizeof(glm::vec4);
			case Karma::ShaderDataType::Mat3:
				return sizeof(glm::vec4);
			case Karma::ShaderDataType::Mat4:
				return sizeof(glm::vec4);
			case Karma::ShaderDataType::Int:
				return sizeof(int);
			case Karma::ShaderDataType::Int2:
				return sizeof(glm::ivec2);
			case Karma::ShaderDataType::Int3:
				return sizeof(glm::ivec4);
			case Karma::ShaderDataType::Int4:
				return sizeof(glm::ivec4);
			case Karma::ShaderDataType::Bool:
				return sizeof(bool);
		}
		KR_CORE_ASSERT(false, "Unknown ShaderDataType");
		return 0;
	}
}