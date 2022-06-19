#pragma once

#include "Karma/Core.h"
#include "glm/gtc/type_ptr.hpp"
#include <stdint.h>
#include "stb_image.h"

// PCH stuff
#include <string>
#include <vector>

namespace Karma
{
	enum class ShaderDataType
	{
		None = 0,
		Float,
		Float2,
		Float3,
		Float4,
		Mat3,
		Mat4,
		Int,
		Int2,
		Int3,
		Int4,
		Bool
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:
				return 4;
			case ShaderDataType::Float2:
				return 4 * 2;
			case ShaderDataType::Float3:
				return 4 * 3;
			case ShaderDataType::Float4:
				return 4 * 4;
			case ShaderDataType::Mat3:
				return 4 * 3 * 3;
			case ShaderDataType::Mat4:
				return 4 * 4 * 4;
			case ShaderDataType::Int:
				return 4;
			case ShaderDataType::Int2:
				return 4 * 2;
			case ShaderDataType::Int3:
				return 4 * 3;
			case ShaderDataType::Int4:
				return 4 * 4;
			case ShaderDataType::Bool:
				return 4;
		}

		KR_CORE_ASSERT(false, "Unknown ShaderDataType");
		return 0;
	}
	
	struct BufferElement
	{
		std::string Name;
		uint64_t Offset;
		uint32_t Size;
		ShaderDataType Type;
		bool Normalized;

		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{
		}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
				case Karma::ShaderDataType::None:
					return 0;
				case Karma::ShaderDataType::Float:
					return 1;
				case Karma::ShaderDataType::Float2:
					return 2;
				case Karma::ShaderDataType::Float3:
					return 3;
				case Karma::ShaderDataType::Float4:
					return 4;
				case Karma::ShaderDataType::Mat3:
					return 3 * 3;
				case Karma::ShaderDataType::Mat4:
					return 4 * 4;
				case Karma::ShaderDataType::Int:
					return 1;
				case Karma::ShaderDataType::Int2:
					return 2;
				case Karma::ShaderDataType::Int3:
					return 3;
				case Karma::ShaderDataType::Int4:
					return 4;
				case Karma::ShaderDataType::Bool:
					return 1;
			}

			KR_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	};

	class KARMA_API BufferLayout
	{
	public:
		BufferLayout()
		{
		}
		
		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		inline uint32_t GetStride() const { return m_Stride; }
		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		inline void PushElement(const BufferElement& element) { m_Elements.push_back(element); CalculateOffsetsAndStride(); }

		std::vector<BufferElement>::iterator begin()
		{
			return m_Elements.begin();
		}

		std::vector<BufferElement>::iterator end()
		{
			return m_Elements.end();
		}

		std::vector<BufferElement>::const_iterator begin() const
		{
			return m_Elements.begin();
		}

		std::vector<BufferElement>::const_iterator end() const
		{
			return m_Elements.end();
		}
	
	private:
		void CalculateOffsetsAndStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}
	
	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};
	
	class KARMA_API VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;

		static VertexBuffer* Create(float* vertices, uint32_t size);
	};

	class KARMA_API IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		virtual uint32_t GetCount() const = 0;

		static IndexBuffer* Create(uint32_t* vertices, uint32_t size);
	};

	class KARMA_API ImageBuffer
	{
	public:
		static ImageBuffer* Create(const char* filename);
	};

	class KARMA_API UBODataPointer
	{
	public:
		UBODataPointer(const void* data) : m_DataPointer(data)
		{}

		const void* GetDataPointer() const
		{
			return m_DataPointer;
		}
	private:
		const void* m_DataPointer;
	};

	struct KARMA_API UniformBufferObject
	{
		static UniformBufferObject* Create(std::vector<ShaderDataType> dataTypes, uint32_t bindingPointIndex);
		
		UniformBufferObject(std::vector<ShaderDataType> dataTypes, uint32_t bindingPointIndex);
		virtual ~UniformBufferObject() = default;

		template<typename... T>
		void UpdateUniforms(T&&... uniforms)
		{
			m_UniformList = { uniforms... };
		}

		virtual void UploadUniformBuffer() {}

		uint32_t GetBufferSize() const
		{
			return m_BufferSize;
		}

		const std::vector<UBODataPointer>& GetUniformList() const
		{
			return m_UniformList;
		}

		const std::vector<ShaderDataType>& GetUniformDataType() const
		{
			return m_UniformDataType;
		}

		const std::vector<uint32_t>& GetAlignedOffsets() const
		{
			return m_UniformAlignedOffsets;
		}

		const std::vector<uint32_t>& GetUniformSize() const
		{
			return m_UniformSizes;
		}

		uint32_t GetBindingPointIndex() const
		{
			return m_BindingPoint;
		}

	protected:
		void CalculateOffsetsAndBufferSize();

	protected:
		uint32_t m_BufferSize;
		uint32_t m_BindingPoint;
		std::vector<UBODataPointer> m_UniformList;
		std::vector<ShaderDataType> m_UniformDataType;
		std::vector<uint32_t> m_UniformAlignedOffsets;
		std::vector<uint32_t> m_UniformSizes;

	private:
		uint32_t ComputeBaseAlignment(ShaderDataType dataType);
	};
}
