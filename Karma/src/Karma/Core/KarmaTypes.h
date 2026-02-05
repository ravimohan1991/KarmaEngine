/**
 * @file KarmaTypes.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the custom types used in Engine's logic
 * @version 1.0
 * @date March 30, 2023
 *
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include <vector>
#include <cstdint>
#include <map>
#include <utility>
#include <algorithm>
#include <glm/glm.hpp>

enum class EAllowShrinking
{
	/** Allow the array to shrink its allocation if possible */
	Yes,

	/** Prevent the array from shrinking its allocation */
	No
};

/** @brief Specifies why an actor is being deleted/removed from a level */
namespace EEndPlayReason
{
	enum Type : int
	{
		/** When the Actor or Component is explicitly destroyed. */
		Destroyed,
		/** When the world is being unloaded for a level transition. */
		LevelTransition,
		/** When the world is being unloaded because PIE is ending. */
		EndPlayInEditor,
		/** When the level it is a member of is streamed out. */
		RemovedFromWorld,
		/** When the application is being exited. */
		Quit,
	};
}

/**
 * @brief euphemism for no index situations
 * 
 * @see KarmaVector::Find, KarmaVector::RemoveSingleSwap etc
 */
enum { INDEX_NONE = -1 };

/** @brief Specifies the goal/source of a UWorld object */
namespace EWorldType
{
	enum Type
	{
		/** An untyped world, in most cases this will be the vestigial worlds of streamed in sub-levels */
		None,

		/** The game world */
		Game,

		/** A world being edited in the editor */
		Editor,

		/** A Play In Editor world */
		PIE,

		/** A preview world for an editor tool */
		EditorPreview,

		/** A preview world for a game */
		GamePreview,

		/** A minimal RPC world for a game */
		GameRPC,

		/** An editor world that was loaded but not currently being edited in the level editor */
		Inactive
	};
}

/**
 * @brief Karma's std::map wrapper
 */
template <typename KeyType, typename ValueType>
class KarmaMap
{
public:
	typedef std::map<KeyType, ValueType> AMap;
	typedef typename AMap::iterator AnIterator;

	/**
	 * @brief Constructor
	 *
	 * @since Karma 1.0.0
	 */
	KarmaMap()
	{
	}

	/**
	 * @brief Addes a key-value pair to the map
	 * 
	 * @since Karma 1.0.0
	 */
	void Add(const KeyType& Key, const ValueType& Value)
	{
		m_KeyValuePair.insert(typename AMap::value_type(Key, Value));
	}

	/**
	 * @brief Checks if the map contains a specified key
	 *
	 * @param Key The key to search for.
	 * @return true if the map contains the specified key, false otherwise.
	 *
	 * @since Karma 1.0.0
	 */
	bool Contains(const KeyType& Key) const
	{
		return m_KeyValuePair.find(Key) != m_KeyValuePair.end();
	}

	/**
	 * @brief Find the value associated with a specified key, or if none exists,
	 * adds a value using the default constructor.
	 *
	 * Algorithm courtsey: https://stackoverflow.com/a/1409465
	 *
	 * @param Key 				The key to search for.
	 * @return A reference to the value associated with the specified key
	 *
	 * @since Karma 1.0.0
	 */
	ValueType& FindOrAdd(const KeyType& Key)
	{
		ValueType aValue;
		std::pair<AnIterator, bool> const& result = m_KeyValuePair.insert(typename AMap::value_type(Key, aValue));

		// a simple return.first->second could work. doing this for better understandibility
		if(result.second)
		{
			// Value was inserted, meaning wasn't there earlier
			return result.first->second;
		}
		else
		{
			// Nothing inserted, meaning value with key already exists
			return result.first->second;
		}
	}

	/**
	 * @brief Find the value associated with a specified key.
	 *
	 * @param Key The key to search for.
	 * @return A pointer to the value associated with the specified key, or nullptr if the key isn't contained in this map.  The pointer
	 *			is only valid until the next change to any key in the map.
	 *
	 * @since Karma 1.0.0
	 */

	ValueType* Find(const KeyType& Key)
	{
		AnIterator result = m_KeyValuePair.find(Key);

		if(result != m_KeyValuePair.end())
		{
			return &result->second;
		}

		return nullptr;
	}

	/**
	 * @brief Overloaded subscript operator for accessing values by key
	 *
	 * @param Key The key whose associated value is to be accessed.
	 * @return A reference to the value associated with the specified key.
	 *
	 * @note If the key does not exist in the map, a new entry is created with a default-constructed value. A mutable reference
	 * is returned.
	 * @since Karma 1.0.0
	 */
	ValueType& operator[](const KeyType& Key)
	{
		return m_KeyValuePair[Key];
	}

	/**
	 * @brief Overloaded subscript operator for accessing values by key (const version)
	 *
	 * @param Key						The key whose associated value is to be accessed.
	 * @return A const reference to the value associated with the specified key.
	 *
	 * @note Returns immutable reference
	 * @since Karma 1.0.0
	 */
	const ValueType& operator[](const KeyType& Key) const
	{
		return m_KeyValuePair.at(Key);
	}

	uint32_t Num() const
	{
		return (uint32_t) m_KeyValuePair.size();
	}

	auto begin()
	{
		return m_KeyValuePair.begin();
	}

	auto end()
	{
		return m_KeyValuePair.end();
	}

	auto begin() const
	{
		return m_KeyValuePair.cbegin();
	}

	auto end() const
	{
		return m_KeyValuePair.cend();
	}

protected:
	AMap m_KeyValuePair;
};

/**
 * @brief Karma's std::vector wrapper with additional functionalities
 * 
 * @since Karma 1.0.0
 */
template<typename BuildingBlock>
class KarmaVector
{
public:
	/**
	 * @brief Constructor
	 *
	 * @since Karma 1.0.0
	 */
	KarmaVector()
	{
	}

	/**
	 * @brief Destructor
	 *
	 * @since Karma 1.0.0
	 */
	~KarmaVector()
	{
	}

	/**
	 * @brief Removes an element from the vector
	 *
	 * The function removes all occurrences of the specified element from the vector
	 * 
	 * @note The vector size is reduced by the number of removed elements
	 * 
	 * @param aBlock	The element to be removed
	 * @since Karma 1.0.0
	 */
	uint32_t Remove(BuildingBlock aBlock)
	{
		uint32_t occurences = 0;
		typename std::vector<BuildingBlock>::iterator iterator = m_Elements.begin();

		while (iterator != m_Elements.end())
		{
			if (*iterator == aBlock)
			{
				iterator = m_Elements.erase(iterator);
				occurences++;
			}
			else
			{
				++iterator;
			}
		}

		return occurences;
	}

	/**
	 * @brief Add an element to the vector
	 *
	 * @aBlock	The element to be added
	 * @since Karma 1.0.0
	 */
	void Add(BuildingBlock aBlock)
	{
		m_Elements.push_back(aBlock);
	}

	/**
	 * @brief Sanity check for the index
	 * 
	 * @since Karma 1.0.0
	 */
	void RangeCheck(uint32_t Index) const
	{
		KR_CORE_ASSERT((Index >= 0 && Index < Num()), "KarmaVector: Range check failed");
	}

	/**
	 * Adds unique element to array if the element doesn't exist.
	 *
	 * Move semantics version.
	 *
	 * @param Item						Item to add
	 * @returns Index of the element in the array.
	 *
	 * @see Add, AddDefaulted, AddZeroed, Append, Insert
	 * @since Karma 1.0.0
	 */
	FORCEINLINE int32_t AddUnique(const BuildingBlock& Item)
	{
		int32_t index = Find(Item);

		if(index == -1)
		{
			Add(Item);
			return Num() - 1;
		}
		else
		{
			return index;
		}
	}

	/**
	 * Finds first occurance of an element within the array.
	 *
	 * @param Item Item to look for.
	 * @returns Index of the found element. INDEX_NONE otherwise.
	 *
	 * @see FindLast, FindLastByPredicate
	 * @see https://www.techiedelight.com/find-index-element-vector-cpp/
	 *
	 * @since Karma 1.0.0
	 */
	int32_t Find(const BuildingBlock& Item) const
	{
		int32_t returnIndex = 0;

		typename std::vector<BuildingBlock>::const_iterator iter = m_Elements.begin();

		for (; iter != m_Elements.end(); iter++)
		{
			if (m_Elements[returnIndex] == Item)
			{
				break;
			}
			returnIndex++;
		}

		if (iter != m_Elements.end())
		{
			return returnIndex;
		}

		return INDEX_NONE;
	}

	/**
	 * @brief Sees if the vector contains the specified element
	 *
	 * @param aBlock	The element to be see in the vector
	 * @returns true if the element exists in the vector
	 *
	 * @since Karam 1.0.0
	 */
	bool Contains(BuildingBlock aBlock) const
	{
		typename std::vector<BuildingBlock>::const_iterator iterator = m_Elements.begin();

		while (iterator != m_Elements.end())
		{
			if (*iterator == aBlock)
			{
				return true;
			}
			else
			{
				++iterator;
			}
		}

		return false;
	}

	/**
	 * @brief Removes an element at given location, swapping the last element into its place, and
	 * shrinking the array.
	 * 
	 * @param Index					The index of the element to be removed
	 * @param AllowShrinking		Whether to allow the array to shrink its allocation if possible
	 * 
	 * 
	 * @note This implementation is differenct from UE and here vector shrinks automatically
	 * @since Karma 1.0.0
	 */
	void RemoveAtSwap(int32_t Index/*, EAllowShrinking AllowShrinking = EAllowShrinking::Yes*/)
	{
		RangeCheck(Index);
		RemoveAtSwapImpl(Index);

		//if (AllowShrinking == EAllowShrinking::Yes)
		//{
			// not functional yet
		//}
	}

	/**
	 * @brief Removes a first appearence of single element from the array, swapping the last element 
	 * into its place, and  shrinking the array.
	 *
	 * @param Item					The item to be removed
	 * @param AllowShrinking		Whether to allow the array to shrink its allocation if possible
	 * @returns The number of elements removed (0 or 1)
	 *
	 * @note AllowShrinking is automatic different from UE
	 * @since Karma 1.0.0
	 */
	uint32_t RemoveSingleSwap(const BuildingBlock& Item/*, EAllowShrinking AllowShrinking = EAllowShrinking::Yes*/)
	{
		int32_t Index = Find(Item);

		if (Index == INDEX_NONE)
		{
			return 0;
		}

		RemoveAtSwap(Index/*, AllowShrinking*/);

		return 1;
	}

	/**
	 * @brief Returns the total number of elements in a vector
	 *
	 * @since Karma 1.0.0
	 */
	uint32_t Num() const
	{
		return (uint32_t) m_Elements.size();
	}

	/**
	 * @brief We just reset the vector. UE has the following implementation
	 *
	 * Same as empty, but doesn't change memory allocations, unless the new size is larger than
	 * the current array. It calls the destructors on held items if needed and then zeros the ArrayNum.
	 *
	 * @param NewSize The expected usage size after calling this function.
	 * @warning Included deletion of possible UObjects. Hence needs rewriting once ShivaActor is functional
	 *
	 * @since Karma 1.0.0
	 */
	void Reset()
	{
		// Maybe make smartpointer instead of manually deleting
		typename std::vector<BuildingBlock>::iterator iter = m_Elements.begin();

		// solution got from https://www.reddit.com/r/cpp_questions/comments/panivh/constexpr_if_statement_to_check_if_template/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
		if constexpr(std::is_pointer_v<BuildingBlock>)
		{
			for (iter = m_Elements.begin(); iter != m_Elements.end(); iter++)
			{
				if (*iter != nullptr)
				{
					KR_CORE_INFO("Deleting UObjects");
					delete *iter;
					*iter = nullptr;
				}
			}
		}

		m_Elements.clear();
	}

	void Resize(uint32_t NewSize)
	{
		m_Elements.resize(NewSize);
	}

	/**
	 * @brief Just clear the elements
	 *
	 * @remark Assuming the use of smart pointers
	 * @warning Still need to think deletion of UObjects
	 *
	 * @since Karma 1.0.0
	 */
	void SmartReset()
	{
		m_Elements.clear();
	}

	/**
	 * @brief Set the element of a vector using the vector index
	 *
	 * @param Index					The index of the vector element to be set
	 * @param Value					The value to be assigned to the element
	 *
	 * @since Karma 1.0.0
	 */
	void SetVectorElementByIndex(int32_t Index, BuildingBlock Value)
	{
		m_Elements[Index] = Value;
	}

	/**
	 * @brief Clears the vector from all the elements
	 *
	 * @note Duplicate of SmartReset
	 * @since Karma 1.0.0
	 */
	inline void Clear() { m_Elements.clear(); }
	
	/**
	 * @brief Getter for the elements of vector
	 *
	 * @since Karma 1.0.0
	 */
	inline const std::vector<BuildingBlock>& GetElements() const { return m_Elements; }

	/**
	 * @brief Getter for elements of vector for modification in appropriate way
	 *
	 * @remark A modifyable reference is returned
	 * @since Karma 1.0.0
	 */
	inline std::vector<BuildingBlock>& ModifyElements() { return m_Elements; }

	/**
	 * @brief Getter for first vector element
	 * 
	 * @since Karma 1.0.0
	 */
	typename std::vector<BuildingBlock>::iterator begin()
	{
		return m_Elements.begin();
	}

	/**
	 * @brief Getter for the last vector element
	 *
	 * @since Karma 1.0.0
	 */
	typename std::vector<BuildingBlock>::iterator end()
	{
		return m_Elements.end();
	}

	typename std::vector<BuildingBlock>::const_iterator begin() const
	{
		return m_Elements.cbegin();
	}

	typename std::vector<BuildingBlock>::const_iterator end() const
	{
		return m_Elements.cend();
	}

	/**
	 * Returns the UObject corresponding to index. Be advised this is only for very low level use.
	 *
	 * @param Index				index of object to return
	 * @return Object at this index
	 *
	 * @since Karma 1.0.0
	 */
	FORCEINLINE BuildingBlock& IndexToObject(int32_t Index)
	{
		KR_CORE_ASSERT(Index >= 0, "");
		
		if(Index < Num())
		{
			return m_Elements.at(Index);
		}

		KR_CORE_ASSERT(false, "Shouldn't happen");

		static BuildingBlock aBlock;
		return aBlock;
	}

	/**
	 * Tests if index is valid, i.e. greater than or equal to zero, and less than the number of elements in the array.
	 *
	 * @param Index Index to test.
	 * @returns True if index is valid. False otherwise.
	 *
	 * @since Karma 1.0.0
	 */
	FORCEINLINE bool IsValidIndex(int32_t Index) const
	{
		return Index >= 0 && Index < Num();
	}

	/**
	 * @brief Helper function to return typed pointer to the first entry of array
	 * 
	 * @since Karma 1.0.0
	 */
	FORCEINLINE BuildingBlock const* GetData() const { return m_Elements.data(); }

	BuildingBlock& operator[](int32_t Index)
	{
		RangeCheck(Index);
		return m_Elements[Index];
	}

private:
	/**
	 * @brief Internal implementation of RemoveAtSwap
	 * 
	 * @param Index					The index of the element to be removed
	 * @since Karma 1.0.0
	 */
	void RemoveAtSwapImpl(int32_t Index)
	{
		const BuildingBlock* Data = GetData();
		const BuildingBlock* Dest = Data + Index;

		// Number of elements (after the generated vacancy) to move
		const int32_t NumElementsAfterHole = Num() - Index - 1;
		const int32_t NumElementsToMoveIntoHole = glm::min(1, NumElementsAfterHole);

		if (NumElementsToMoveIntoHole)
		{
			//std::memmove((void *) Dest, (const void*)(Data + Num() - NumElementsToMoveIntoHole),
			//	sizeof(BuildingBlock) * NumElementsToMoveIntoHole);
			const int32_t LastElementIndex = Num() - 1;
			
			m_Elements[Index] = m_Elements[LastElementIndex];
		}
		m_Elements.pop_back();
	}

protected:
	std::vector<BuildingBlock> m_Elements;
};

/**
 * @brief Traveling from server to server.
 */
enum ETravelType
{
	/** Absolute URL. */
	TRAVEL_Absolute,

	/** Partial (carry name, reset server). */
	TRAVEL_Partial,

	/** Relative URL. */
	TRAVEL_Relative,

	/** No clue. */
	TRAVEL_MAX,
};

/** Whether to teleport physics body or not */
enum class ETeleportType : uint8_t
{
	/** Do not teleport physics body. This means velocity will reflect the movement between initial and final position, and collisions along the way will occur */
	None,

	/** Teleport physics body so that velocity remains the same and no collision occurs */
	TeleportPhysics,

	/** Teleport physics body and reset physics state completely */
	ResetPhysics,
};

/**
 * @brief Helper for obtaining the default Url configuration
 */
struct KARMA_API FUrlConfig
{
	std::string m_DefaultProtocol;
	std::string m_DefaultName;
	std::string m_DefaultHost;
	std::string m_DefaultPortal;
	std::string m_DefaultSaveExt;
	int32_t m_DefaultPort;

	/**
	 * Initialize with defaults from ini
	 */
	void Init();

	/**
	 * Reset state
	 */
	void Reset();
};

/**
 * @brief URL structure.
 */
struct KARMA_API FURL
{
	/** Protocol, i.e. "karma" or "http".*/
	std::string m_Protocol;

	/** Optional hostname, i.e. "204.157.115.40" or "unreal.epicgames.com", blank if local.*/
	std::string m_Host;

	/** Optional host port.*/
	int32_t m_Port;

	int32_t m_Valid;

	/** Map name, i.e. "SkyCity", default is "Entry".*/
	std::string m_Map;

	/** Optional place to download Map if client does not possess it*/
	std::string m_RedirectURL;

	// Options.
	//TArray<FString> Op;

	/** Portal to enter through, default is "".*/
	std::string m_Portal;

	/** Statics.*/
	static FUrlConfig UrlConfig;
	static bool m_bDefaultsInitialized;

	/**
	 * Prevent default from being generated.
	 */
	//explicit FURL(ENoInit) { }

	/**
	 * Construct a purely default, local URL from an optional filename.
	 */
	FURL(const std::string& Filename = "");

	/**
	 * Construct a URL from text and an optional relative base.
	 */
	//FURL(FURL* Base, const TCHAR* TextURL, ETravelType Type);

	//static void StaticInit();
	//static void StaticExit();

	/**
	 * Static: Removes any special URL characters from the specified string
	 *
	 * @param Str String to be filtered
	 */
	//static void FilterURLString(const std::string& Str);

	/**
	 * Returns whether this URL corresponds to an internal object, i.e. an Unreal
	 * level which this app can try to connect to locally or on the net. If this
	 * is false, the URL refers to an object that a remote application like Internet
	 * Explorer can execute.
	 */
	//bool IsInternal() const;

	/**
	 * Returns whether this URL corresponds to an internal object on this local
	 * process. In this case, no Internet use is necessary.
	 */
	//bool IsLocalInternal() const;

	/**
	 * Tests if the URL contains an option string.
	 */
	//bool HasOption(const TCHAR* Test) const;

	/**
	 * Returns the value associated with an option.
	 *
	 * @param Match The name of the option to get.
	 * @param Default The value to return if the option wasn't found.
	 *
	 * @return The value of the named option, or Default if the option wasn't found.
	 */
	//const TCHAR* GetOption(const TCHAR* Match, const TCHAR* Default) const;

	/**
	 * Load URL from config.
	 */
	//void LoadURLConfig(const TCHAR* Section, const std::string& Filename = "GGameIni");

	/**
	 * Save URL to config.
	 */
	//void SaveURLConfig(const TCHAR* Section, const TCHAR* Item, const std::string& Filename = "GGameIni") const;

	/**
	 * Add a unique option to the URL, replacing any existing one.
	 */
	//void AddOption(const TCHAR* Str);

	/**
	 * Remove an option from the URL
	 */
	//void RemoveOption(const TCHAR* Key, const TCHAR* Section = nullptr, const std::string& Filename = "GGameIni");

	/**
	 * Convert this URL to text.
	 */
	//std::string ToString(bool FullyQualified = 0) const;

	/**
	 * Prepares the Host and Port values into a standards compliant string
	 */
	//std::string GetHostPortString() const;

	/**
	 * Serializes a FURL to or from an archive.
	 */
	//ENGINE_API friend FArchive& operator<<(FArchive& Ar, FURL& U);

	/**
	 * Compare two URLs to see if they refer to the same exact thing.
	 */
	//bool operator==(const FURL& Other) const;
};
