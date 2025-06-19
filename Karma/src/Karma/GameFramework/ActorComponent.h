/**
 * @file ActorComponent.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class UActorComponent.
 * @version 1.0
 * @date February 27, 2023
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "Karma/Core/Object.h"

class AActor;

namespace Karma
{
	/**
	 * @brief Enum for UActorComponent instance creation method
	 */
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
	 * @brief ActorComponent is the base class for components that define reusable behavior that can be added to different types of Actors.
	 * ActorComponents that have a transform are known as SceneComponents and those that can be rendered are PrimitiveComponents.
	 * Components are a special type of Object that Actors can attach to themselves as sub-objects.
	 * Components are useful for sharing common behaviors
	 *
	 * @see [ActorComponent](https://docs.unrealengine.com/latest/INT/Programming/UnrealArchitecture/Actors/Components/index.html#actorcomponents)
	 * @see https://docs.unrealengine.com/5.2/en-US/components-in-unreal-engine/
	 * @see USceneComponent
	 * @see UPrimitiveComponent
	 */
	class KARMA_API UActorComponent : public UObject
	{
		DECLARE_KARMA_CLASS(UActorComponent, UObject)

	private:
		/** Cached pointer to owning actor */
		mutable AActor* m_OwnerPrivate;

		/** Indicates that BeginPlay has been called, but EndPlay has not yet */
		uint8_t m_bHasBegunPlay : 1;

		/** Indicates that OnCreatedComponent has been called, but OnDestroyedComponent has not yet */
		uint8_t m_bHasBeenCreated : 1;

		/** Whether the component is currently active. */
		uint8_t m_bIsActive : 1;

		/** Indicates that InitializeComponent has been called, but UninitializeComponent has not yet */
		uint8_t m_bHasBeenInitialized : 1;

		/**
		 * Pointer to the world that this component is currently registered with.
		 * This is only non-NULL when the component is registered.
		 */
		UWorld* m_WorldPrivate;

	public:
		/**
		 * @brief A constructor
		 *
		 * @since Karma 1.0.0
		 */
		UActorComponent();

		/**
		 * @brief Function called every frame on this ActorComponent. Override this function to implement custom logic to be executed every frame.
		 * Only executes if the component is registered, and also PrimaryComponentTick.bCanEverTick must be set to true.
		 *
		 * @note Not functional yet this seems
		 * @param deltaTime - The time since the last tick.
		 * Rest of the params shall be functionla later
		 * @param TickType - The kind of tick this is, for example, are we paused, or 'simulating' in the editor
		 * @param ThisTickFunction - Internal tick function struct that caused this to run
		 */
		virtual void TickComponent(float deltaTime);//, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

		/**
		 * @brief Begins Play for the component.
		 * Called when the owning Actor begins play or when the component is created if the Actor has already begun play.
		 * Actor BeginPlay normally happens right after PostInitializeComponents but can be delayed for networked or child actors.
		 * Requires component to be registered and initialized.
		 *
		 * @see AActor::BeginPlay()
		 * @since Karma 1.0.0
		 */
		virtual void BeginPlay();

		/**
		 * @brief Ends gameplay for this component.
		 * //Called from AActor::EndPlay only if bHasBegunPlay is true// <- not quite sure about the call
		 *
		 * @see UActorComponent::BeginDestroy()
		 * @since Karma 1.0.0
		 */
		virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

		/** 
		 * @brief Follow the Outer chain to get the  AActor  that 'Owns' this component
		 *
		 * @since Karma 1.0.0
		 */
		AActor* GetOwner() const;

		/** 
		 * @brief Indicates that OnCreatedComponent has been called, but OnDestroyedComponent has not yet
		 *
		 * @since Karma 1.0.0
		 */
		bool HasBeenCreated() const { return m_bHasBeenCreated; }

		/** 
		 * @brief Called when a component is created (not loaded). This can happen in the editor or during gameplay.
		 *
		 * @since Karma 1.0.0
		 */
		virtual void OnComponentCreated();

		/** 
		 * @brief See if this component is currently registered
		 *
		 * @since Karma 1.0.0
		 */
		inline bool IsRegistered() const { return m_bRegistered; }

		/**
		 * @brief Returns whether the component is active or not
		 *
		 * @return - The active state of the component.
		 * @since Karma 1.0.0
		 */
		bool IsActive() const { return m_bIsActive; }

		/**
		 * @brief Activates the SceneComponent, should be overridden by native child classes.
		 *
		 * @param bReset - Whether the activation should happen even if ShouldActivate returns false.
		 * @since Karma 1.0.0
		 */
		virtual void Activate(bool bReset = false);

		/**
		 * @brief Sets the value of bIsActive without causing other side effects to this instance.
		 *
		 * Activate, Deactivate, and SetActive are preferred in most cases because they respect virtual behavior.
		 *
		 * @since Karma 1.0.0
		 */
		void SetActiveFlag(const bool bNewIsActive);

		/** 
		 * @brief Indicates that InitializeComponent has been called, but UninitializeComponent has not yet
		 *
		 * @since Karma 1.0.0
		 */
		bool HasBeenInitialized() const { return m_bHasBeenInitialized; }

		/**
		 * @brief Initializes the component.  Occurs at level startup or actor spawn. This is before BeginPlay (Actor or Component).
		 * All Components in the level will be Initialized on load before any Actor/Component gets BeginPlay
		 * Requires component to be registered, and bWantsInitializeComponent to be true.
		 *
		 * @since Karma 1.0.0
		 */
		virtual void InitializeComponent();

		/** 
		 * @brief Indicates that BeginPlay has been called, but EndPlay has not yet
		 *
		 * @since Karma 1.0.0
		 */
		bool HasBegunPlay() const { return m_bHasBegunPlay; }

		//~ Begin UObject Interface.
		/**
		 * @brief Overridden BeginDestroy() for UActorComponent
		 *
		 * @see UObject::BeginDestroy()
		 */
		virtual void BeginDestroy() override;
		//~ End UObject Interface.

		/**
		 * @brief Handle this component being Uninitialized.
		 *
		 * @see Called from AActor::EndPlay only if bHasBeenInitialized is true
		 */
		virtual void UninitializeComponent();

		/**
		 * @brief Called when a component is destroyed
		 *
		 * @param	bDestroyingHierarchy  - True if the entire component hierarchy is being torn down, allows avoiding expensive operations
		 */
		virtual void OnComponentDestroyed(bool bDestroyingHierarchy);

	public:
		/** Describes how a component instance will be created */
		EComponentCreationMethod m_CreationMethod;

		/** Whether the component is activated at creation or must be explicitly activated. */
		uint8_t m_bAutoActivate : 1;


		/** If true, we call the virtual InitializeComponent */
		uint8_t m_bWantsInitializeComponent : 1;

	protected:
		/**
		 *  Indicates if this ActorComponent is currently registered with a scene.
		 */
		uint8_t m_bRegistered : 1;

	protected:
		/** Return true if this component is in a state where it can be activated normally. */
		virtual bool ShouldActivate() const;
	};
}
