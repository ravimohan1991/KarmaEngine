#include "SceneComponent.h"
#include "Ganit/Transform.h"

#include "GameFramework/Actor.h"

namespace Karma
{
	USceneComponent::USceneComponent()
	{
		m_AttachParent = nullptr;
	}

	void USceneComponent::SetWorldLocation(glm::vec3 newLocation)
	{
		// Find parent and set relative position with respect to the parent
	}

	inline std::shared_ptr<USceneComponent> USceneComponent::GetAttachParent() const
	{
		return m_AttachParent;
	}

	FTransform USceneComponent::GetSocketTransform(const std::string& SocketName, ERelativeTransformSpace TransformSpace) const
	{
		switch (TransformSpace)
		{
		case RTS_Actor:
		{
			return GetComponentTransform().GetRelativeTransform(GetOwner()->GetTransform());
			break;
		}
		case RTS_Component:
		case RTS_ParentBoneSpace:
		{
			return FTransform::Identity();
		}
		default:
		{
			return GetComponentTransform();
		}
		}
	}

	void USceneComponent::SetWorldTransform(const FTransform& NewTransform)
	{
		// If attached to something, transform into local space
		if (GetAttachParent() != nullptr)
		{
			const FTransform ParentToWorld = GetAttachParent()->GetSocketTransform(GetAttachSocketName());
			FTransform RelativeTM = NewTransform.GetRelativeTransform(ParentToWorld);

			// Absolute location, rotation, and scale use the world transform directly.
			if (IsUsingAbsoluteLocation())
			{
				RelativeTM.CopyTranslation(NewTransform);
			}

			if (IsUsingAbsoluteRotation())
			{
				RelativeTM.CopyRotation(NewTransform);
			}

			if (IsUsingAbsoluteScale())
			{
				RelativeTM.CopyScale3D(NewTransform);
			}

			SetRelativeTransform(RelativeTM, bSweep, OutSweepHitResult, Teleport);
		}
		else
		{
			SetRelativeTransform(NewTransform, bSweep, OutSweepHitResult, Teleport);
		}
	}
}