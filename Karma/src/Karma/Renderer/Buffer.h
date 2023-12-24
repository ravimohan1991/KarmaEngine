/**
 * @file Buffer.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains base classes for various kinds of buffers used by the Engine.
 * @version 1.0
 * @date December 21, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "glm/gtc/type_ptr.hpp"
#include "stb_image.h"

namespace Karma
{
	/**
	 * @brief A C++ data structure for shader language (GLSL or HLSL) data, like uniforms for instance and datatype for vertex or index buffers. For use case see \ref Mesh::ProcessTheRawMesh.
	 */
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

	/**
	 * @brief A routine to compute the size of ShaderDataType in bytes
	 *
	 * @param type						ShaderDataType specification
	 * @return size of the type in bytes
	 *
	 * @note We may and should consider sizes for different architectures. Just think first before implementing.
	 * @since Karma 1.0.0
	 */
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
			case ShaderDataType::None:
				KR_CORE_WARN("ShaderDataType is none. Size shall be considered 0.");
				return 0;
		}

		KR_CORE_ASSERT(false, "Unknown ShaderDataType");
		return 0;
	}

	/**
	 * @brief A datastructure for creating interleaved data for Mesh with a specifc format (BufferLayout)
	 */
	struct BufferElement
	{
		/**
		 * @brief A name for vertex data, created by interleaving of various attributes
		 *
		 * @see MeshAttribute
		 * @since Karma 1.0.0
		 */
		std::string Name;

		/**
		 * @brief The location (offset) of this element in the vertex or index data set
		 *
		 * @see CalculateOffsetsAndStride()
		 * @since Karma 1.0.0
		 */
		uint64_t Offset;

		/**
		 * @brief The size of this element
		 *
		 * @see ShaderDataTypeSize
		 * @see CalculateOffsetsAndBufferSize
		 *
		 * @since Karma 1.0.0
		 */
		uint32_t Size;

		/**
		 * @brief The type of the attribute
		 *
		 * @since Karma 1.0.0
		 */
		ShaderDataType Type;
		bool Normalized;

		/**
		 * @brief A constructor
		 *
		 * @param type						One of the defined \ref ShaderDataType
		 * @param name						name of the attribute (\ref MeshAttribute)
		 * @param normalized				For glVertexAttribPointer, if normalized is set to true, it indicates that values stored in an integer format are to be mapped to the range [-1,1] (for signed values) or [0,1] (for unsigned values) when they are accessed and converted to floating point. Otherwise, values will be converted to floats directly without normalization
		 * @param offset					The amount of gap to be left, for instance in vertexarray
		 *
		 * @todo Think about vulkan needs of normalized parameter, with opengl in context, and try to abstract accordingly. Also may need to think about the custom default offsets
		 * @since Karma 1.0.0
		 */
		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false, uint64_t offset = 0)
			: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(offset), Normalized(normalized)
		{
		}

		/**
		 * @brief Routine to compute number of components of this element
		 *
		 * @since Karma 1.0.0
		 */
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

	/**
	 * @brief A format for vertex buffer say
	 *
	 * @see GaugeVertexDataLayout
	 */
	class KARMA_API BufferLayout
	{
	public:
		/**
		 * @brief A default constructor
		 * @see VulkanVertexBuffer
		 *
		 * @since Karma 1.0.0
		 */
		BufferLayout()
		{
		}

		/**
		 * @brief Constructor with specified buffer data format
		 *
		 * @param elements
		 *
		 * @note For legacy purpose only. For instance https://github.com/ravimohan1991/KarmaEngine/blob/138c172ccedf31acfab982af51ae130f9a37d3bb/Application/src/KarmaApp.cpp#L21.
		 * See Mesh abstraction for industry purpose usage.
		 * @since Karma 1.0.0
		 */
		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		/**
		 * @brief A getter for m_Stride
		 *
		 * @since Karma 1.0.0
		 */
		inline uint32_t GetStride() const { return m_Stride; }

		/**
		 * @brief A getter for m_Elements
		 *
		 * @since Karma 1.0.0
		 */
		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		/**
		 * @brief For modifying the list of elements and set paremeters accordingly
		 *
		 * @since Karma 1.0.0
		 */
		inline void PushElement(const BufferElement& element) { m_Elements.push_back(element); CalculateOffsetsAndStride(); }

		/**
		 * @brief Getter for iterator begin
		 *
		 * @since Karma 1.0.0
		 */
		std::vector<BufferElement>::iterator begin()
		{
			return m_Elements.begin();
		}

		/**
		 * @brief Getter for iterator end
		 *
		 * @since Karma 1.0.0
		 */
		std::vector<BufferElement>::iterator end()
		{
			return m_Elements.end();
		}

		/**
		 * @brief Getter for const_iterator begin
		 *
		 * @since Karma 1.0.0
		 */
		std::vector<BufferElement>::const_iterator begin() const
		{
			return m_Elements.begin();
		}

		/**
		 * @brief Getter for const_iterator end
		 *
		 * @since Karma 1.0.0
		 */
		std::vector<BufferElement>::const_iterator end() const
		{
			return m_Elements.end();
		}

	private:
		/**
		 * @brief Computes the offset (after which subsequent elements are to be added) and m_Stride (the total size in bytes)
		 *
		 * @since Karma 1.0.0
		 */
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

	/**
	 * @brief An abstract class for vertex buffer
	 */
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

	/**
	 * @brief An abstract class for index buffer
	 */
	class KARMA_API IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		virtual uint32_t GetCount() const = 0;

		static IndexBuffer* Create(uint32_t* vertices, uint32_t size);
	};

	/**
	 * @brief Abstract class for image buffer
	 */
	class KARMA_API ImageBuffer
	{
	public:
		static ImageBuffer* Create(const char* filename);
		uint32_t GetBindingPointIndex() const
		{
			return m_BindingPoint;
		}
	protected:
		uint32_t m_BindingPoint;
	};

	/**
	 * @brief Class for holding UBO (uniform buffer object) data (for instance projection matrix and view matrix) pointer
	 */
	class KARMA_API UBODataPointer
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @param data					The data pointer of UBO which could be view matrix or projection matrix
		 * @since Karma 1.0.0
		 */
		UBODataPointer(const void* data) : m_DataPointer(data)
		{}

		/**
		 * @brief Getter for UBO data
		 *
		 * @since Karma 1.0.0
		 */
		const void* GetDataPointer() const
		{
			return m_DataPointer;
		}
	private:
		const void* m_DataPointer;
	};

	/**
	 * @brief Abstract class for UBO (uniform buffer object)
	 */
	struct KARMA_API UniformBufferObject
	{
		/**
		 * @brief A function for appropriate initialization of UBO based on programmer selected rendered (Vulkan or OpenGL)
		 *
		 * @param dataTypes				List of data types for uniforms to be uploaded to GPU (like used in shaders),
		 * 								for instance https://github.com/ravimohan1991/KarmaEngine/blob/138c172ccedf31acfab982af51ae130f9a37d3bb/Application/src/KarmaApp.cpp#L39 where Mat4 are for https://github.com/ravimohan1991/KarmaEngine/blob/138c172ccedf31acfab982af51ae130f9a37d3bb/Resources/Shaders/shader.vert#L9-L13
		 * @param bindingPointIndex				the binding of shader specified index
		 */
		static UniformBufferObject* Create(std::vector<ShaderDataType> dataTypes, uint32_t bindingPointIndex);

		/**
		 * @brief An agnostic constructor provided for precomputation of offset and buffer size
		 *
		 * @param dataTypes				See \ref Create for information
		 * @param bindingPointIndex		See \ref Create for information
		 *
		 * @since Karma 1.0.0
		 */
		UniformBufferObject(std::vector<ShaderDataType> dataTypes, uint32_t bindingPointIndex);

		/**
		 * @brief Pure virtual destructor and sub class should have appropriate implementation
		 *
		 * @since Karma 1.0.0
		 */
		virtual ~UniformBufferObject() = default;

		/**
		 * @brief Set the m_UniformList with latest supplied uniforms
		 *
		 * @see Material::OnUpdate
		 * @since Karma 1.0.0
		 */
		template<typename... T>
		void UpdateUniforms(T&&... uniforms)
		{
			m_UniformList = { uniforms... };
		}

		/**
		 * @brief An overridable function to upload the uniform buffer
		 *
		 * @note The function is pure virtual with default value provided.
		 * @todo Need to rewrite implementation of this virtual function because Material::ProcessForSubmission() makes agnostic call and VulkanContext::UploadUBO
		 * also makes call to upload uniform which is seperate from Material::ProcessForSubmission.
		 *
		 * @since Karma 1.0.0
		 */
		virtual void UploadUniformBuffer(size_t frameIndex = 0) = 0;

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
		/**
		 * @brief Calculate offsets and buffer size of the uniform buffer object(s).
		 *
		 * Basically individual uniform's offset (from the beginning) and size are computed and cached in 
		 * \ref m_UniformAlignedOffsets and \ref m_UniformSizes list. The offsets are boundary aligned to the multiple of individual uniform size.
		 *
		 * @since Karma 1.0.0
		 */
		void CalculateOffsetsAndBufferSize();

	protected:
		/**
		 * @brief Total size of the buffer in bytes
		 *
		 * @since Karma 1.0.0
		 */
		uint32_t m_BufferSize;

		/**
		 * @brief The binding point prescribed in vertex shader for instance https://github.com/ravimohan1991/KarmaEngine/blob/5ff57f5747c19efcea050646bc2d217c99d74015/Resources/Shaders/shader.vert#L14-L18
		 *
		 * @since Karma 1.0.0
		 */
		uint32_t m_BindingPoint;

		/**
		 * @brief List of uniforms to be uploaded
		 *
		 * @since Karma 1.0.0
		 */
		std::vector<UBODataPointer> m_UniformList;

		/**
		 * @brief List of data types (\ref ShaderDataType) for the m_UniformList
		 *
		 * @since Karma 1.0.0
		 */
		std::vector<ShaderDataType> m_UniformDataType;

		/**
		 * @brief List of individual uniform boundary aligned (with multiple of individual uniform size) offsets
		 *
		 * @since Karma 1.0.0
		 */
		std::vector<uint32_t> m_UniformAlignedOffsets;

		/**
		 * @brief List of individual uniform sizes
		 *
		 * @since Karam 1.0.0
		 */
		std::vector<uint32_t> m_UniformSizes;

	private:
		/**
		 * @brief Computing size of pre defined \ref ShaderDataType
		 *
		 * @since Karma 1.0.0
		 */
		uint32_t ComputeBaseAlignment(ShaderDataType dataType);
	};
}
