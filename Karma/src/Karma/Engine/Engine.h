#pragma once

#include "krpch.h"

#include "Object.h"
#include "Core/KarmaTypes.h"

#include "GameFramework/World.h"

namespace Karma
{
	/**
	 * FWorldContext:
	 * A context for dealing with UWorlds at the engine level. As the engine brings up and destroys world, we need a way to keep straight
	 * what world belongs to what.
	 *
	 * WorldContexts can be thought of as a track. By default we have 1 track that we load and unload levels on. Adding a second context is adding
	 * a second track; another track of progression for worlds to live on.
	 *
	 * For the GameEngine, there will be one WorldContext until we decide to support multiple simultaneous worlds.
	 * For the EditorEngine, there may be one WorldContext for the EditorWorld and one for the PIE World.
	 *
	 * FWorldContext provides both a way to manage 'the current PIE UWorld*' as well as state that goes along with connecting/travelling to
	 * new worlds.
	 *
	 * FWorldContext should remain internal to the UEngine classes. Outside code should not keep pointers or try to manage FWorldContexts directly.
	 * Outside code can still deal with UWorld*, and pass UWorld*s into Engine level functions. The Engine code can look up the 	relevant context
	 * for a given UWorld*.
	 *
	 * For convenience, FWorldContext can maintain outside pointers to UWorld*s. For example, PIE can tie UWorld* UEditorEngine::PlayWorld to the PIE
	 * world context. If the PIE UWorld changes, the UEditorEngine::PlayWorld pointer will be automatically updated. This is done with AddRef() and
	 * SetCurrentWorld().
	 *
	 */
	struct FWorldContext
	{
		EWorldType::Type	m_WorldType;

		std::string	m_ContextHandle;

		class UGameViewportClient*	m_GameViewport;

		class UGameInstance* m_OwningGameInstance;


		/** A list of active net drivers */
		//UPROPERTY(transient)
		//TArray<FNamedNetDriver> ActiveNetDrivers;


		/** The PIE instance of this world, -1 is default */
		//int32   PIEInstance;


		/** The Prefix in front of PIE level names, empty is default */
		//FString PIEPrefix;


		/** The feature level that PIE world should use */
		//ERHIFeatureLevel::Type PIEWorldFeatureLevel;


		/** Is this running as a dedicated server */
		bool    m_RunAsDedicated;


		/** Is this world context waiting for an online login to complete (for PIE) */
		bool    m_bWaitingOnOnlineSubsystem;


		/** Is this the 'primary' PIE instance.  Primary is preferred when, for example, unique hardware like a VR headset can be used by only one PIE instance. */
		bool    m_bIsPrimaryPIEInstance;


		/** Handle to this world context's audio device.*/
		uint32_t m_AudioDeviceID;


		/** Custom description to be display in blueprint debugger UI */
		//std::string CustomDescription;


		// If > 0, tick this world at a fixed rate in PIE
		float PIEFixedTickSeconds  = 0.f;
		float PIEAccumulatedTickSeconds = 0.f;


		/** On a transition to another level (e.g. LoadMap), the engine will verify that these objects have been cleaned up by garbage collection */
		//TSet<FObjectKey> GarbageObjectsToVerify;

		/**************************************************************/

		/** Outside pointers to CurrentWorld that should be kept in sync if current world changes  */
		//TArray<TObjectPtr<UWorld>*>
		KarmaVector<UWorld**> m_ExternalReferences;


		void AddRef(UWorld*& WorldPtr)
		{
			WorldPtr = m_ThisCurrentWorld;
			m_ExternalReferences.AddUnique(&WorldPtr);
		}


		/** Removes an external reference */
		void RemoveRef(UWorld*& WorldPtr)
		{
			m_ExternalReferences.Remove(&WorldPtr);
			WorldPtr = nullptr;
		}


		/** Set CurrentWorld and update external reference pointers to reflect this*/
		void SetCurrentWorld(UWorld *World);


		/** Collect FWorldContext references for garbage collection */
		//void AddReferencedObjects(FReferenceCollector& Collector, const UObject* ReferencingObject);


		FORCEINLINE UWorld* World() const
		{
			return m_ThisCurrentWorld;
		}

		FWorldContext()
			: m_WorldType(EWorldType::None)
			//, m_ContextHandle(NAME_None)
			//, TravelURL()
			//, TravelType(0)
			//, PendingNetGame(nullptr)
			//, bShouldCommitPendingMapChange(0)
			, m_GameViewport(nullptr)
			, m_OwningGameInstance(nullptr)
			//, PIEInstance(INDEX_NONE)
			//, PIEWorldFeatureLevel(ERHIFeatureLevel::Num)
			, m_RunAsDedicated(false)
			, m_bWaitingOnOnlineSubsystem(false)
			, m_bIsPrimaryPIEInstance(false)
			, m_AudioDeviceID(INDEX_NONE)
			, m_ThisCurrentWorld(nullptr)
		{ }

	private:
		UWorld*	m_ThisCurrentWorld;
	};

	/**
	 * Base class of all Engine classes, responsible for management of systems critical to editor or game systems.
	 * Also defines default classes for certain engine systems.
	 *
	 * ATM we have only this class. In future we may subclass to EditorEngine and GameEngine subclasses
	 */
	class KARMA_API KEngine: public UObject
	{
		DECLARE_KARMA_CLASS(KEngine, UObject)

		class UGameInstance* m_GameInstance;

	public:
		/* Default constructor */
		KEngine();

		void Init(/*IEngineLoop* InEngineLoop*/);

		/** Update everything.  Should be economic for processor and rest of the resources. */
		virtual void Tick(float DeltaSeconds, bool bIdleMode);

		/** Clean up the GameViewport */
		void CleanupGameViewport();

		FWorldContext& CreateNewWorldContext(EWorldType::Type WorldType);

		/** Needs to be called when a world is added to broadcast messages. */
		virtual void			WorldAdded(UWorld* World) {}

		/** Needs to be called when a world is destroyed to broadcast messages. */
		virtual void			WorldDestroyed(UWorld* InWorld) {}

		/** Get current gameintance. */
		virtual class UGameInstance*		GetCurrentGameInstance() const { return m_GameInstance; }

	protected:
		KarmaVector<FWorldContext*> m_WorldList;
	};

	/** Global engine pointer. Can be 0 so don't use without checking. */
	extern KARMA_API KEngine*			GEngine;
}
