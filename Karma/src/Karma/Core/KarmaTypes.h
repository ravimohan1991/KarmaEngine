#pragma once

#include <vector>
#include <cstdint>
#include <map>
#include <utility>
#include <algorithm>

/** Specifies why an actor is being deleted/removed from a level */
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

/** Specifies the goal/source of a UWorld object */
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

/// <summary>
/// Karma's std::map wrapper
/// </summary>
template <typename KeyType, typename ValueType>
class KarmaMap
{
public:
	typedef std::map<KeyType, ValueType> AMap;
	typedef AMap::iterator AnIterator;
	/**
	 * Find the value associated with a specified key, or if none exists,
	 * adds a value using the default constructor.
	 *
	 * Algorithm courtsey: https://stackoverflow.com/a/1409465
	 *
	 * @param Key The key to search for.
	 * @return A reference to the value associated with the specified key.
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
	 * Find the value associated with a specified key.
	 *
	 * @param Key The key to search for.
	 * @return A pointer to the value associated with the specified key, or nullptr if the key isn't contained in this map.  The pointer
	 *			is only valid until the next change to any key in the map.
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
private:
	AMap m_KeyValuePair;
};

/// <summary>
/// Karma's std::vector wrapper
/// </summary>
template<typename BuildingBlock>
class KarmaVector
{
public:
	KarmaVector()
	{
		m_Elements;
	}

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

	void Add(BuildingBlock aBlock)
	{
		m_Elements.push_back(aBlock);
	}

	bool Contains(BuildingBlock aBlock)
	{
		typename std::vector<BuildingBlock>::iterator iterator = m_Elements.begin();

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

	uint32_t Num()
	{
		return (uint32_t) m_Elements.size();
	}

	/**
	 * We just reset the vector. UE has the following implementation
	 * 
	 * Same as empty, but doesn't change memory allocations, unless the new size is larger than
	 * the current array. It calls the destructors on held items if needed and then zeros the ArrayNum.
	 *
	 * @param NewSize The expected usage size after calling this function.
	 */
	void Reset()
	{
		/*
		if (NewSize < 0)
		{
			OnInvalidNum((USizeType)NewSize);
		}

		// If we have space to hold the excepted size, then don't reallocate
		if (NewSize <= ArrayMax)
		{
			DestructItems(GetData(), ArrayNum);
			ArrayNum = 0;
		}
		else
		{
			Empty(NewSize);
		}*/

		// Maybe make smartpointer instead of manually deleting
		typename std::vector<BuildingBlock>::iterator iterator = m_Elements.begin();

		while (iterator != m_Elements.end())
		{
			if (iterator != nullptr)
			{
				delete iterator;
				iterator = nullptr;
			}

			// iterator increment statement here
		}

		m_Elements.clear();
	}
	
	/**
	 * Assuming the use of smart pointers
	 */
	void SmartReset()
	{
		m_Elements.clear();
	}

	inline const std::vector<BuildingBlock>& GetElements() const { return m_Elements; }

	typename std::vector<BuildingBlock>::iterator begin()
	{
		return m_Elements.begin();
	}

	typename std::vector<BuildingBlock>::iterator end()
	{
		return m_Elements.end();
	}

	/**
	 * Returns the UObject corresponding to index. Be advised this is only for very low level use.
	 *
	 * @param Index				index of object to return
	 * @return Object at this index
	 */
	FORCEINLINE BuildingBlock IndexToObject(int32_t Index)
	{
		KR_CORE_ASSERT(Index >= 0, "");
		if(Index < m_Elements.size())
		{
			return m_Elements.at(Index);
		}

		return nullptr;
	}

protected:
	std::vector<BuildingBlock> m_Elements;
};

// Traveling from server to server.
enum ETravelType
{
	/** Absolute URL. */
	TRAVEL_Absolute,

	/** Partial (carry name, reset server). */
	TRAVEL_Partial,
	
	/** Relative URL. */
	TRAVEL_Relative,
	
	TRAVEL_MAX,
};

/**
 * Helper for obtaining the default Url configuration
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

//URL structure.
struct KARMA_API FURL
{
	// Protocol, i.e. "karma" or "http".
	std::string m_Protocol;

	// Optional hostname, i.e. "204.157.115.40" or "unreal.epicgames.com", blank if local.
	std::string m_Host;

	// Optional host port.
	int32_t m_Port;

	int32_t m_Valid;

	// Map name, i.e. "SkyCity", default is "Entry".
	std::string m_Map;

	// Optional place to download Map if client does not possess it
	std::string m_RedirectURL;

	// Options.
	//TArray<FString> Op;

	// Portal to enter through, default is "".
	std::string m_Portal;

	// Statics.
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
