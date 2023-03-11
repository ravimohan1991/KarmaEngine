#pragma once

#include "krpch.h"

namespace Karma
{
	class UObject;
	class AActor;
	class UClass;
	//enum EObjectFlags;
	//enum class EInternalObjectFlags;

	extern std::vector<UObject> GUObjectStore;

#define	INVALID_OBJECT	(UObject*)-1

	// 32-bit signed integer <- find or write appropriate class for such type
	typedef signed int	 		int32;

	enum { INDEX_NONE = -1 };

	/**
	 * Flags describing an object instance
	 */
	enum EObjectFlags
	{
		// Do not add new flags unless they truly belong here. There are alternatives.
		// if you change any the bit of any of the RF_Load flags, then you will need legacy serialization
		RF_NoFlags = 0x00000000,	///< No flags, used to avoid a cast

		// This first group of flags mostly has to do with what kind of object it is. Other than transient, these are the persistent object flags.
		// The garbage collector also tends to look at these.
		RF_Public = 0x00000001,	///< Object is visible outside its package.
		RF_Standalone = 0x00000002,	///< Keep object around for editing even if unreferenced.
		RF_MarkAsNative = 0x00000004,	///< Object (UField) will be marked as native on construction (DO NOT USE THIS FLAG in HasAnyFlags() etc)
		RF_Transactional = 0x00000008,	///< Object is transactional.
		RF_ClassDefaultObject = 0x00000010,	///< This object is its class's default object
		RF_ArchetypeObject = 0x00000020,	///< This object is a template for another object - treat like a class default object
		RF_Transient = 0x00000040,	///< Don't save object.

		// This group of flags is primarily concerned with garbage collection.
		RF_MarkAsRootSet = 0x00000080,	///< Object will be marked as root set on construction and not be garbage collected, even if unreferenced (DO NOT USE THIS FLAG in HasAnyFlags() etc)
		RF_TagGarbageTemp = 0x00000100,	///< This is a temp user flag for various utilities that need to use the garbage collector. The garbage collector itself does not interpret it.

		// The group of flags tracks the stages of the lifetime of a uobject
		RF_NeedInitialization = 0x00000200,	///< This object has not completed its initialization process. Cleared when ~FObjectInitializer completes
		RF_NeedLoad = 0x00000400,	///< During load, indicates object needs loading.
		RF_KeepForCooker = 0x00000800,	///< Keep this object during garbage collection because it's still being used by the cooker
		RF_NeedPostLoad = 0x00001000,	///< Object needs to be postloaded.
		RF_NeedPostLoadSubobjects = 0x00002000,	///< During load, indicates that the object still needs to instance subobjects and fixup serialized component references
		RF_NewerVersionExists = 0x00004000,	///< Object has been consigned to oblivion due to its owner package being reloaded, and a newer version currently exists
		RF_BeginDestroyed = 0x00008000,	///< BeginDestroy has been called on the object.
		RF_FinishDestroyed = 0x00010000,	///< FinishDestroy has been called on the object.

		// Misc. Flags
		RF_BeingRegenerated = 0x00020000,	///< Flagged on UObjects that are used to create UClasses (e.g. Blueprints) while they are regenerating their UClass on load (See FLinkerLoad::CreateExport()), as well as UClass objects in the midst of being created
		RF_DefaultSubObject = 0x00040000,	///< Flagged on subobjects that are defaults
		RF_WasLoaded = 0x00080000,	///< Flagged on UObjects that were loaded
		RF_TextExportTransient = 0x00100000,	///< Do not export object to text form (e.g. copy/paste). Generally used for sub-objects that can be regenerated from data in their parent object.
		RF_LoadCompleted = 0x00200000,	///< Object has been completely serialized by linkerload at least once. DO NOT USE THIS FLAG, It should be replaced with RF_WasLoaded.
		RF_InheritableComponentTemplate = 0x00400000, ///< Archetype of the object can be in its super class
		RF_DuplicateTransient = 0x00800000,	///< Object should not be included in any type of duplication (copy/paste, binary duplication, etc.)
		RF_StrongRefOnFrame = 0x01000000,	///< References to this object from persistent function frame are handled as strong ones.
		RF_NonPIEDuplicateTransient = 0x02000000,	///< Object should not be included for duplication unless it's being duplicated for a PIE session
		RF_Dynamic /*UE_DEPRECATED(5.0, "RF_Dynamic should no longer be used. It is no longer being set by engine code.")*/ = 0x04000000,	///< Field Only. Dynamic field - doesn't get constructed during static initialization, can be constructed multiple times  // @todo: BP2CPP_remove
		RF_WillBeLoaded = 0x08000000,	///< This object was constructed during load and will be loaded shortly
		RF_HasExternalPackage = 0x10000000,	///< This object has an external package assigned and should look it up when getting the outermost package

		// RF_Garbage and RF_PendingKill are mirrored in EInternalObjectFlags because checking the internal flags is much faster for the Garbage Collector
		// while checking the object flags is much faster outside of it where the Object pointer is already available and most likely cached.
		// RF_PendingKill is mirrored in EInternalObjectFlags because checking the internal flags is much faster for the Garbage Collector
		// while checking the object flags is much faster outside of it where the Object pointer is already available and most likely cached.

		RF_PendingKill /*UE_DEPRECATED(5.0, "RF_PendingKill should not be used directly. Make sure references to objects are released using one of the existing engine callbacks or use weak object pointers.")*/ = 0x20000000,	///< Objects that are pending destruction (invalid for gameplay but valid objects). This flag is mirrored in EInternalObjectFlags as PendingKill for performance
		RF_Garbage /*UE_DEPRECATED(5.0, "RF_Garbage should not be used directly. Use MarkAsGarbage and ClearGarbage instead.")*/ = 0x40000000,	///< Garbage from logical point of view and should not be referenced. This flag is mirrored in EInternalObjectFlags as Garbage for performance
		RF_AllocatedInSharedPage = 0x80000000	///< Allocated from a ref-counted page shared with other UObjects
	};

	/**
	 * Objects flags for internal use (GC, low level UObject code)
	*
	* This MUST be kept in sync with EInternalObjectFlags defined in
	* Engine\Source\Programs\Shared\EpicGames.Core\UnrealEngineTypes.cs
	*/
	enum class EInternalObjectFlags : int32
	{
		None = 0,

		LoaderImport = 1 << 20, ///< Object is ready to be imported by another package during loading
		Garbage = 1 << 21, ///< Garbage from logical point of view and should not be referenced. This flag is mirrored in EObjectFlags as RF_Garbage for performance
		PersistentGarbage = 1 << 22, ///< Same as above but referenced through a persistent reference so it can't be GC'd
		ReachableInCluster = 1 << 23, ///< External reference to object in cluster exists
		ClusterRoot = 1 << 24, ///< Root of a cluster
		Native = 1 << 25, ///< Native (UClass only). 
		Async = 1 << 26, ///< Object exists only on a different thread than the game thread.
		AsyncLoading = 1 << 27, ///< Object is being asynchronously loaded.
		Unreachable = 1 << 28, ///< Object is not reachable on the object graph.
		PendingKill /*UE_DEPRECATED(5.0, "PendingKill flag should no longer be used. Use Garbage flag instead.")*/ = 1 << 29, ///< Objects that are pending destruction (invalid for gameplay but valid objects). This flag is mirrored in EObjectFlags as RF_PendingKill for performance
		RootSet = 1 << 30, ///< Object will not be garbage collected, even if unreferenced.
		PendingConstruction = 1 << 31, ///< Object didn't have its class constructor called yet (only the UObjectBase one to initialize its most basic members)

		GarbageCollectionKeepFlags = Native | Async | AsyncLoading | LoaderImport,
		MirroredFlags = Garbage | PendingKill, /// Flags mirrored in EObjectFlags

		//~ Make sure this is up to date!
		AllFlags = LoaderImport | Garbage | PersistentGarbage | ReachableInCluster | ClusterRoot | Native | Async | AsyncLoading | Unreachable | PendingKill | RootSet | PendingConstruction
	};

	/**
	* This struct is used for passing parameter values to the StaticConstructObject_Internal() method.  Only the constructor parameters are required to
	* be valid - all other members are optional.
	*/
	struct FStaticConstructObjectParameters
	{
		/** The class of the object to create */
		const UClass* m_Class;

		/** The object to create this object within (the Outer property for the new object will be set to the value specified here). */
		UObject* m_Outer;

		/** The name to give the new object.If no value(NAME_None) is specified, the object will be given a unique name in the form of ClassName_#. */
		std::string m_Name;

		/** The ObjectFlags to assign to the new object. some flags can affect the behavior of constructing the object. */
		EObjectFlags m_SetFlags = EObjectFlags::RF_NoFlags;

		/** The InternalObjectFlags to assign to the new object. some flags can affect the behavior of constructing the object. */
		EInternalObjectFlags m_InternalSetFlags = EInternalObjectFlags::None;

		/** If true, copy transient from the class defaults instead of the pass in archetype ptr(often these are the same) */
		bool m_bCopyTransientsFromClassDefaults = false;

		/** If true, Template is guaranteed to be an archetype */
		bool m_bAssumeTemplateIsArchetype = false;

		/**
		 * If specified, the property values from this object will be copied to the new object, and the new object's ObjectArchetype value will be set to this object.
		 * If nullptr, the class default object is used instead.
		 */
		UObject* m_Template = nullptr;

		/** Contains the mappings of instanced objects and components to their templates */
		//FObjectInstancingGraph* InstanceGraph = nullptr;

		/** Assign an external Package to the created object if non-null */
		//UPackage* ExternalPackage = nullptr;

		/** Callback for custom code to initialize properties before PostInitProperties runs */
		//TFunction<void()> PropertyInitCallback;

	private:
		//FObjectInitializer::FOverrides* SubobjectOverrides = nullptr;

	public:

		//COREUOBJECT_API FStaticConstructObjectParameters(const UClass* InClass);

		//friend FObjectInitializer;
	};

/**
 * Create a new instance of an object.  The returned object will be fully initialized.  If InFlags contains RF_NeedsLoad (indicating that the object still needs to load its object data from disk), components
 * are not instanced (this will instead occur in PostLoad()).  The different between StaticConstructObject and StaticAllocateObject is that StaticConstructObject will also call the class constructor on the object
 * and instance any components.
 *
 * @param	Params		The parameters to use when construction the object. @see FStaticConstructObjectParameters
 *
 * @return	A pointer to a fully initialized object of the specified class.
 */
KARMA_API UObject* StaticConstructObject_Internal(const FStaticConstructObjectParameters& Params);

/**
 * Create a new instance of an object or replace an existing object.  If both an Outer and Name are specified, and there is an object already in memory with the same Class, Outer, and Name, the
 * existing object will be destructed, and the new object will be created in its place.
 *
 * @param	Class		the class of the object to create
 * @param	InOuter		the object to create this object within (the Outer property for the new object will be set to the value specified here).
 * @param	Name		the name to give the new object. If no value (NAME_None) is specified, the object will be given a unique name in the form of ClassName_#.
 * @param	SetFlags	the ObjectFlags to assign to the new object. some flags can affect the behavior of constructing the object.
 * @param InternalSetFlags	the InternalObjectFlags to assign to the new object. some flags can affect the behavior of constructing the object.
 * 
 * Rest shall be made available when reflection code is enabled
 * @param bCanReuseSubobjects	if set to true, SAO will not attempt to destroy a subobject if it already exists in memory.
 * @param bOutReusedSubobject	flag indicating if the object is a subobject that has already been created (in which case further initialization is not necessary).
 * @param ExternalPackage	External Package assigned to the allocated object, if any
 * @return	a pointer to a fully initialized object of the specified class.
 */
KARMA_API UObject* StaticAllocateObject(const UClass* Class, UObject* InOuter, const std::string& name, EObjectFlags SetFlags = EObjectFlags::RF_NoFlags, EInternalObjectFlags InternalSetFlags = EInternalObjectFlags::None);


/**
 * Convenience template for constructing a gameplay object
 *
 * @param	Outer		the outer for the new object.  If not specified, object will be created in the transient package. For AActors, Outer is the ULevel
 * @param	Class		the class of object to construct
 * @param	Name		the name for the new object.  If not specified, the object will be given a transient name via MakeUniqueObjectName
 * @param	Flags		the object flags to apply to the new object
 * @param	Template	the object to use for initializing the new object.  If not specified, the class's default object will be used. Not functional atm
 * @param	bCopyTransientsFromClassDefaults	if true, copy transient from the class defaults instead of the pass in archetype ptr (often these are the same)
 * @param	InInstanceGraph						contains the mappings of instanced objects and components to their templates
 * @param	ExternalPackage						Assign an external Package to the created object if non-null
 *
 * @return	a pointer of type T to a new object of the specified class
 */
template< class T >
FUNCTION_NON_NULL_RETURN_START
T* NewObject(UObject* Outer, const UClass* Class, std::string name = "No_Name", EObjectFlags Flags = RF_NoFlags, UObject* Template = nullptr, bool bCopyTransientsFromClassDefaults = false/*, FObjectInstancingGraph* InInstanceGraph = nullptr, UPackage* ExternalPackage = nullptr*/)
FUNCTION_NON_NULL_RETURN_END
{
	if (name == "")
	{
		KR_CORE_ASSERT(false, "NewObject with empty name can't be used to create default subobjects");
		//FObjectInitializer::AssertIfInConstructor(Outer, TEXT("NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSubobject<> instead."));
	}

	FStaticConstructObjectParameters Params;
	Params.m_Outer = Outer;
	Params.m_Name = name;
	Params.m_SetFlags = Flags;
	//Params.m_Template = Template;
	Params.m_bCopyTransientsFromClassDefaults = bCopyTransientsFromClassDefaults;
	//Params.InstanceGraph = InInstanceGraph;
	//Params.ExternalPackage = ExternalPackage;

	return static_cast<T*>(StaticConstructObject_Internal(Params));
}

/**
 * A routine to find if the object is instantiated already. May need to modify in accordance with thread safety in future
 * UE name StaticFindObjectFastInternal
 * 
 * @param	ObjectClass			the class of object to construct 
 * @param	ObjectPackage		the outer where the object is supposed to be found
 * @param	ObjectName			the name for the object to be found
 * @param	bExactClass			class match check
 * @param	bAnyPackage			If there is no package (no InObjectPackage specified, and InName's package is "") and the caller specified any_package, then 
 *								accept it, regardless of its package.Or, if the object is a top-level package then accept it immediately
 * @param	ExcludeFlags		Don't return objects that have any of these exclusive flags set
 * @param	ExclusiveInternalFlags			Include (or not) pending kill objects
 * @param	ExternalPackage					Assign an external Package to the created object if non-null
 *
 * @return	a pointer of type UObject if found, else nulptr
 */
KARMA_API UObject* StaticFindObjectFastInternal(const UClass* ObjectClass, const UObject* ObjectPackage, const std::string& ObjectName, bool bExactClass = false, EObjectFlags ExcludeFlags = RF_NoFlags, EInternalObjectFlags ExclusiveInternalFlags = EInternalObjectFlags::None);
}