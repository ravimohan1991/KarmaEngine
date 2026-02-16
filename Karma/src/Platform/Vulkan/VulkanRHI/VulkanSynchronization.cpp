#include "VulkanSynchronization.h"
#include "VulkanDevice.h"

namespace Karma
{
	FVulkanFence::FVulkanFence(FVulkanDevice& InDevice, FVulkanFenceManager& InOwner, bool bCreateSignaled) :
		m_Owner(InOwner), m_State(bCreateSignaled ? EState::Signaled : EState::NotReady)
	{
		VkFenceCreateInfo fenceInfo = {};
		
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = bCreateSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

		VkResult result = vkCreateFence(InDevice.GetLogicalDevice(), &fenceInfo, nullptr, &m_Handle);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create fence");

	}

	FVulkanFence::~FVulkanFence()
	{
		KR_CORE_ASSERT(m_Handle == VK_NULL_HANDLE, "Fence must be destroyed by owner before being deleted");
	}

	void FVulkanFenceManager::DestroyFence(FVulkanFence* Fence)
	{
		vkDestroyFence(m_Device.GetLogicalDevice(), Fence->m_Handle, nullptr);

		Fence->m_Handle = VK_NULL_HANDLE;
		delete Fence;
	}

	FVulkanFenceManager::~FVulkanFenceManager()
	{
		KR_CORE_ASSERT(m_UsedFences.Num() == 0, "All used fences must be released before destroying the manager");
	}

	FVulkanFence* FVulkanFenceManager::AllocateFence(bool bCreateSignaled)
	{
		if (m_FreeFences.Num() != 0)
		{
			FVulkanFence* Fence = m_FreeFences.IndexToObject(0);
			
			m_FreeFences.RemoveAtSwap(0/*, EAllowShrinking::No*/);
			m_UsedFences.Add(Fence);

			if (bCreateSignaled)
			{
				Fence->m_State = FVulkanFence::EState::Signaled;
			}
			else
			{
				Fence->m_State = FVulkanFence::EState::NotReady;
			}

			return Fence;
		}

		FVulkanFence* NewFence = new FVulkanFence(m_Device, *this, bCreateSignaled);
		m_UsedFences.Add(NewFence);

		return NewFence;
	}

	void FVulkanFenceManager::Denit()
	{
		KR_CORE_ASSERT(m_UsedFences.Num() == 0, "Not all fences are done")
		
		for(FVulkanFence* fence : m_FreeFences)
		{
			DestroyFence(fence);
		}
	}

	bool FVulkanFenceManager::WaitForFence(FVulkanFence* Fence)
	{
		KR_CORE_ASSERT(m_UsedFences.Contains(Fence), "Not a usable fence");
		
		VkResult result = vkWaitForFences(m_Device.GetLogicalDevice(), 1, &Fence->m_Handle, true, UINT64_MAX);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to wait");
		
		switch (result)
		{
			case VK_SUCCESS:
				Fence->m_State = FVulkanFence::EState::Signaled;
				return true;
			case VK_TIMEOUT:
				break;
			default:
				break;
		}
		
		return false;
	}

	void FVulkanFenceManager::ResetFence(FVulkanFence* Fence)
	{
		if (Fence->m_State != FVulkanFence::EState::NotReady)
		{
			VkResult result = vkResetFences(m_Device.GetLogicalDevice(), 1, &Fence->m_Handle);
			Fence->m_State = FVulkanFence::EState::NotReady;
		}
	}

	void FVulkanFenceManager::ReleaseFence(FVulkanFence*& Fence)
	{
		ResetFence(Fence);
		m_UsedFences.RemoveSingleSwap(Fence/*, EAllowShrinking::No*/);

		m_FreeFences.Add(Fence);// add copy of the pointer to free list
		Fence = nullptr;// nullify the caller's pointer
	}

	FVulkanSemaphore::FVulkanSemaphore(FVulkanDevice& InDevice) : m_Device(InDevice)
	{
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkResult result = vkCreateSemaphore(m_Device.GetLogicalDevice(), &semaphoreInfo, VK_NULL_HANDLE, &m_Handle);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create Semaphore");
	}

	FVulkanSemaphore::~FVulkanSemaphore()
	{
		vkDestroySemaphore(m_Device.GetLogicalDevice(), m_Handle, VK_NULL_HANDLE);
	}
}
