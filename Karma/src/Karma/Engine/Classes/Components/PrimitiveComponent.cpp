#include "PrimitiveComponent.h"
#include "Buffer.h"

namespace Karma
{
	UPrimitiveComponent::UPrimitiveComponent() : USceneComponent()
	{
		m_ComponentTransformUniform.reset(UniformBufferObject::Create({ ShaderDataType::Mat4 }, 0));
		
		m_UniformTranformMatrix = GetComponentTransform().ToMatrixWithScale();
		
		UBODataPointer uModelMatrix(&m_UniformTranformMatrix);
		m_ComponentTransformUniform->UpdateUniforms(uModelMatrix);
	}
	
	void UPrimitiveComponent::BeginPlay()
	{
	}

	void UPrimitiveComponent::SetCachedMaxDrawDistance(const float newCachedMaxDrawDistance)
	{
	}

	void UPrimitiveComponent::SetWorldTransform(const FTransform& NewTransform)
	{
		Super::SetWorldTransform(NewTransform);
		
		m_UniformTranformMatrix = GetComponentTransform().ToMatrixWithScale();
	}
}

