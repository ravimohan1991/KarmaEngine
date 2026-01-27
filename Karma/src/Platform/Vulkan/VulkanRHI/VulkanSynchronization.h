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
		/**
		 * @brief Constructor
		 *
		 * @param InDevice						Reference to the owning FVulkanDevice
		 * @since Karma 1.0.0
		 */
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
		 * 	- 1. Makes sure no fences are currently in use (m_UsedFences.Num() == 0)
		 * 	- 2. Clear off m_FreeFences
		 *
		 * @since Karma 1.0.0
		 */
		void Denit();

		/**
		 * @brief Allocates fence for use
		 *
		 * Looks in m_FreeFences, if array is not empty, uses the first fence and fills m_UsedFence.
		 * If m_FreeFences is empty, creates new FVulkanFence and fills m_UsedFence.
		 *
		 * @param bCreateSignaled							Should the fence be in Signaled state
		 * @see KarmaGuiVulkanHandler::FillWindowData
		 * 
		 * @since Karma 1.0.0
		 */
		FVulkanFence* AllocateFence(bool bCreateSignaled = false);

		/**
		 * @brief Waits for the given fence to be signaled.
		 * 
		 * Makes sure the fence is in m_UsedFences before waiting.
		 * 
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
		 * 
		 * @see KarmaGuiVulkanHandler::DestroyFramesOnFlightData
		 * 
		 * @since Karma 1.0.0
		 */
		void ReleaseFence(FVulkanFence*& Fence);

	protected:
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
		/**
		 * @brief Constructor for FVulkanFence.
		 *
		 * @param InDevice						Reference to the owning FVulkanDevice
		 * @param InOwner						Reference to the owning FVulkanFenceManager
		 * 
		 * @param bCreateSignaled				Whether to create the fence in a signaled state
		 * 
		 * @since Karma 1.0.0
		 */
		FVulkanFence(FVulkanDevice& InDevice, FVulkanFenceManager& InOwner, bool bCreateSignaled);

		/**
		 * @brief Checks if the fence is currently signaled.
		 * 
		 * @return true if the fence is in the signaled state, false otherwise
		 * 
		 * @since Karma 1.0.0
		 */
		inline bool IsSignaled() const
		{
			return m_State == EState::Signaled;
		}

		/**
		 * @brief Retrieves the Vulkan fence handle.
		 * 
		 * @return VkFence The Vulkan fence handle.
		 * 
		 * @since Karma 1.0.0
		 */
		inline VkFence GetHandle() const
		{
			return m_Handle;
		}

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

		// Only owner can create and manage fences
		~FVulkanFence();

		friend FVulkanFenceManager;
	};
}
