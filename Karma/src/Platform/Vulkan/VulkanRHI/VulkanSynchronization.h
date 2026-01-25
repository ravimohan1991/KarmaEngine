/**
 * @file VulkanSynchronization.h
 * @brief Header file for Vulkan synchronization primitives.
 *
 * This file declares functions and types related to synchronization in Vulkan,
 * including fences, semaphores, and barriers.
 *
 * @author Ravi Mohan (the_cowboy)
 * @version 1.0
 * 
 * @date 22 January, 2026
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include <vulkan/vulkan.h>
#include "KarmaTypes.h"

namespace Karma
{
	class FVulkanFence;
	class FVulkanDevice;

	class FVulkanFenceManager
	{
	public:
		FVulkanFenceManager(FVulkanDevice& InDevice)
			: m_Device(InDevice)
		{
		}

		/**
		 * @brief Makes sure m_UsedFences are zero i.e no fences are currently in use by
		 * GPU or inflight-work
		 *
		 * @since Karma 1.0.0
		 */
		~FVulkanFenceManager();

		/**
		 * @brief Purpose is two-fold
		 * 	- 1. Makes sure no fences are currently in use (m_UsedFences.Num == 0)
		 * 	- 2. Clear off m_FreeFences
		 *
		 * @since Karma 1.0.0
		 */
		void Denit();

		/**
		 * @brief Looks in m_FreeFences, if array is not empty, uses the first fence and fills m_UsedFence.
		 * If m_FreeFences is empty, creates new FVulkanFence and fills m_UsedFence.
		 *
		 * @param bCreateSignaled							Should the fence be in Signaled state
		 */
		FVulkanFence* AllocateFence(bool bCreateSignaled = false);

		/**
		 * @brief Checks if the given fence is signaled.
		 * 
		 * First checks the CPU-side state of the fence. If the fence is already marked as signaled,
		 * returns true immediately. If not, queries the Vulkan API (via CheckFenceState) to check the 
		 * actual status of the fence.
		 *
		 * @param InFence						The fence to be checked
		 * @return true if the fence is signaled, false otherwise
		 * @see CheckFenceState
		 * @since Karma 1.0.0
		 */
		bool IsFenceSignaled(FVulkanFence* InFence);

		/**
		 * @brief Waits for the given fence to be signaled.
		 * 
		 * @todo Ponder over the use of this funciton
		 * @param Fence							The fence to wait for
		 * 
		 * @since Karma 1.0.0
		 */
		bool WaitForFence(FVulkanFence* Fence);

		/**
		 * @brief Resets the given fence to the unsignaled state.
		 * 
		 * @note Both CPU and GPU side states are reset.
		 * 
		 * @param Fence							The fence to be reset
		 * @since Karma 1.0.0
		 */
		void ResetFence(FVulkanFence* Fence);

		/**
		 * @brief Releases the given fence back to the manager.
		 * 
		 * Moves the fence from m_UsedFences to m_FreeFences for future reuse.
		 * 
		 * @note This is kind of reverse of AllocateFence
		 * @param Fence							The fence to be released
		 * @since Karma 1.0.0
		 */
		void ReleaseFence(FVulkanFence*& Fence);

		/**
		 * @brief Waits for the given fence to be signaled and then releases it.
		 * 
		 * Combines the functionality of WaitForFence and ReleaseFence.
		 * 
		 * @param Fence							The fence to wait for and release
		 * @since Karma 1.0.0
		 */
		void WaitAndReleaseFence(FVulkanFence*& Fence);

	protected:
		/**
		 * @brief Returns true if the fence is signaled, false otherwise.
		 * 
		 * Sets the state of the fence accordingly i.e
		 * 1. If vkGetFenceStatus returns VK_SUCCESS, fence state is set to Signaled
		 * 2. If vkGetFenceStatus returns VK_NOT_READY, fence state is set to NotReady,
		 * meaning associated GPU operations are still pending
		 * 
		 * Assumptions:
		 * 1. The fence is in NotReady state when this function is called. So this function
		 * is used for fences which are supposed to be signaled later after GPU operations complete.
		 * 2. The fence is owned by this manager and is present in m_UsedFences array
		 * 
		 * @note Typically usage is in polling scenarios where CPU wants to check completion status
		 * opportunistically without blocking. For blocking waits, use WaitForFence().
		 * 
		 * @param Fence							The fence to be checked
		 * @since Karma 1.0.0
		 */
		bool CheckFenceState(FVulkanFence* Fence);

		/**
		 * @brief Destroys the given fence and releases its resources.
		 * 
		 * @param InFence						The fence to be destroyed
		 * @since Karma 1.0.0
		 */
		void DestroyFence(FVulkanFence* InFence);

	protected:
		FVulkanDevice& m_Device;

		/**
		 * @brief Array of free fences to be used.
		 *
		 * @note This gets filled mainly by ReleaseFence and WaitAndReleaseFence once
		 * work is finished.
		 */
		KarmaVector<FVulkanFence*> m_FreeFences;
		
		/**
		 * @brief Array of fences currently in use by GPU or in-flight
		 * work.
		 */
		KarmaVector<FVulkanFence*> m_UsedFences;
	};
	
	/**
	 * @brief Represents a Vulkan fence used for synchronization between the CPU and GPU.
	 * 
	 * A fence is a synchronization primitive that can be used to coordinate operations between the CPU and GPU.
	 * It allows the CPU to wait for the completion of GPU operations, ensuring that resources are not accessed
	 * prematurely.
	 * 
	 * @since Karma 1.0.0
	 */
	class FVulkanFence
	{
	public:
		FVulkanFence(FVulkanDevice& InDevice, FVulkanFenceManager& InOwner, bool bCreateSignaled);

		inline bool IsSignaled() const
		{
			return m_State == EState::Signaled;
		}

		inline VkFence GetHandle() const
		{
			return m_Handle;
		}

		// Only owner can create and manage fences
		~FVulkanFence();

	protected:
		VkFence m_Handle;

		enum class EState
		{
			/**
			 * @brief The fence is not signaled, initial state after reset
			 *
			 * @note Fence must be signaled when created
			 * https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation#page_Waiting-for-the-previous-frame
			 */
			NotReady,

			/**
			 * @brief The fence is signaled, indicating that the associated operations have completed
			 *
			 * @note vkWaitForFences will pass when fence is in this state
			 */
			Signaled,
		};

		EState m_State;

		FVulkanFenceManager& m_Owner;

		friend FVulkanFenceManager;
	};
}
