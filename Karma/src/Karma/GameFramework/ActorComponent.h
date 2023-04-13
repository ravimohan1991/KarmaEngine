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
		virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

		/** Follow the Outer chain to get the  AActor  that 'Owns' this component */
		AActor* GetOwner() const;

		/** Indicates that OnCreatedComponent has been called, but OnDestroyedComponent has not yet */
		bool HasBeenCreated() const { return m_bHasBeenCreated; }

		/** Called when a component is created (not loaded). This can happen in the editor or during gameplay */
		virtual void OnComponentCreated();

		/** See if this component is currently registered */
		inline bool IsRegistered() const { return m_bRegistered; }

		/**
		 * Returns whether the component is active or not
		 * @return - The active state of the component.
		 */
		bool IsActive() const { return m_bIsActive; }

		/**
		 * Activates the SceneComponent, should be overridden by native child classes.
		 * @param bReset - Whether the activation should happen even if ShouldActivate returns false.
		 */
		virtual void Activate(bool bReset = false);

		/**
		 * Sets the value of bIsActive without causing other side effects to this instance.
		 *
		 * Activate, Deactivate, and SetActive are preferred in most cases because they respect virtual behavior.
		 */
		void SetActiveFlag(const bool bNewIsActive);

		/** Indicates that InitializeComponent has been called, but UninitializeComponent has not yet */
		bool HasBeenInitialized() const { return m_bHasBeenInitialized; }

		/**
		 * Initializes the component.  Occurs at level startup or actor spawn. This is before BeginPlay (Actor or Component).
		 * All Components in the level will be Initialized on load before any Actor/Component gets BeginPlay
		 * Requires component to be registered, and bWantsInitializeComponent to be true.
		 */
		virtual void InitializeComponent();

		/** 
		 * Indicates that BeginPlay has been called, but EndPlay has not yet 
		 */
		bool HasBegunPlay() const { return m_bHasBegunPlay; }

		//~ Begin UObject Interface.
		virtual void BeginDestroy() override;
		//~ End UObject Interface.

		/**
		 * Handle this component being Uninitialized.
		 * 
		 * @see Called from AActor::EndPlay only if bHasBeenInitialized is true
		 */
		virtual void UninitializeComponent();

		/**
		 * Called when a component is destroyed
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