/**
 * @file PrimitiveComponent.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class UPrimitiveComponent.
 * @version 1.0
 * @date March 1, 2023
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "SceneComponent.h"

namespace Karma
{
	/**
	 * @brief PrimitiveComponents are SceneComponents that contain or generate some sort of geometry, generally to be rendered or used as collision data.
	 * There are several subclasses for the various types of geometry, but the most common by far are the ShapeComponents (Capsule, Sphere, Box), StaticMeshComponent, and SkeletalMeshComponent.
	 * ShapeComponents generate geometry that is used for collision detection but are not rendered, while StaticMeshComponents and SkeletalMeshComponents contain pre-built geometry that is rendered, but can also be used for collision detection.
	 */
	class KARMA_API UPrimitiveComponent : public USceneComponent
	{
		DECLARE_KARMA_CLASS(UPrimitiveComponent, USceneComponent)

	public:
		/** Controls whether the primitive component should cast a shadow or not. */
		uint8_t CastShadow : 1;

		/** 
		 * @brief Used to detach physics objects before simulation begins. This is needed because at runtime we can't have simulated objects inside the attachment hierarchy
		 *
		 * @since Karma 1.0.0
		 */
		virtual void BeginPlay() override;

	private:
		/**
		 * The value of WorldSettings->TimeSeconds for the frame when this component was last rendered.  This is written
		 * from the render thread, which is up to a frame behind the game thread, so you should allow this time to
		 * be at least a frame behind the game thread's world time before you consider the actor non-visible.
		 */
		mutable float LastRenderTime;

		/** Same as LastRenderTime but only updated if the component is on screen. Used by the texture streamer. */
		mutable float LastRenderTimeOnScreen;

		//friend class FPrimitiveSceneInfo;

	public:
		/**
		 * @brief Utility to cache the max draw distance based on cull distance volumes or the desired max draw distance
		 *
		 * @param newCachedMaxDrawDistance						The new max distance to be cached
		 * @since Karma 1.0.0
		 */
		void SetCachedMaxDrawDistance(const float newCachedMaxDrawDistance);
	};
}
