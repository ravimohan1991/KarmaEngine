#pragma once

#include <vector>
#include <cstdint>

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

private:
	std::vector<BuildingBlock> m_Elements;
};
