#pragma once

#include "Karma/Core.h"
#include <stdint.h>

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
				return 1;
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

		std::vector<BufferElement>::iterator begin()
		{
			return m_Elements.begin();
		}

		std::vector<BufferElement>::iterator end()
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
		virtual ~VertexBuffer()
		{
		}

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;

		static VertexBuffer* Create(float* vertices, uint32_t size);
	};

	class KARMA_API IndexBuffer
	{
	public:
		virtual ~IndexBuffer()
		{
		}

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		virtual uint32_t GetCount() const = 0;

		static IndexBuffer* Create(uint32_t* vertices, uint32_t size);
	};
}