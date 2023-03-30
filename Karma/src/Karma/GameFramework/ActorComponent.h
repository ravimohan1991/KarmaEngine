#pragma once

#include "krpch.h"

#include "Karma/Core/Object.h"

class AActor;

namespace Karma
{
	enum class EComponentCreationMethod : uint8_t
	{
		/** A component that is part of a native class. */
		Native,
		/** A component that is created from a template defined in the Components section of the Blueprint. */
		SimpleConstructionScript,
		/**A dynamically created component, either from the UserConstructionScript or from a Add Component node in a Blueprint event graph. */
		UserConstructionScript,
		/** A component added to a single Actor instance via the Component section of the Actor's details panel. */
		Instance,
	};

	/**
	 * ActorComponent is the base class for components that define reusable behavior that can be added to different types of Actors.
	 * ActorComponents that have a transform are known as SceneComponents and those that can be rendered are PrimitiveComponents.
	 *
	 * @see [ActorComponent](https://docs.unrealengine.com/latest/INT/Programming/UnrealArchitecture/Actors/Components/index.html#actorcomponents)
	 * @see USceneComponent
	 * @see UPrimitiveComponent
	 */
	class KARMA_API UActorComponent : public UObject
	{
	private:
		/** Cached pointer to owning actor */
		mutable AActor* m_OwnerPrivate;

		/** Indicates that BeginPlay has been called, but EndPlay has not yet */
		uint8_t m_bHasBegunPlay : 1;


		/** Cached pointer to owning actor */
		mutable AActor* m_OwnerPrivate;

	public:
		/**
		 * Function called every frame on this ActorComponent. Override this function to implement custom logic to be executed every frame.
		 * Only executes if the component is registered, and also PrimaryComponentTick.bCanEverTick must be set to true.
		 *
		 * @param deltaTime - The time since the last tick.
		 * Rest of the params shall be functionla later
		 * @param TickType - The kind of tick this is, for example, are we paused, or 'simulating' in the editor
		 * @param ThisTickFunction - Internal tick function struct that caused this to run
		 */
		virtual void TickComponent(float deltaTime);//, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

		/**
		 * Begins Play for the component.
		 * Called when the owning Actor begins play or when the component is created if the Actor has already begun play.
		 * Actor BeginPlay normally happens right after PostInitializeComponents but can be delayed for networked or child actors.
		 * Requires component to be registered and initialized.
		 */
		virtual void BeginPlay();

		/**
		 * Ends gameplay for this component.
		 * Called from AActor::EndPlay only if bHasBegunPlay is true
		 */
		virtual void EndPlay();//const EEndPlayReason::Type EndPlayReason);

		/** Follow the Outer chain to get the  AActor  that 'Owns' this component */
		AActor* GetOwner() const;

	public:
		/** Describes how a component instance will be created */
		EComponentCreationMethod m_CreationMethod;
	};
}