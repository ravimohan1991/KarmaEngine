#pragma once

#include "krpch.h"

#include "Karma/Core/Object.h"

class AActor;

namespace Karma
{
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
		mutable AActor* OwnerPrivate;

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
	};
}