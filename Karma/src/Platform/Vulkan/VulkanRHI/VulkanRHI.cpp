#include "VulkanDynamicRHI.h"
#include "GLFW/glfw3.h"

#include "Application.h"

namespace Karma
{
	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

#ifdef KR_DEBUG
	bool FVulkanDynamicRHI::bEnableValidationLayers = true;
#else
	bool FVulkanDynamicRHI::bEnableValidationLayers = false;
#endif

	FVulkanDynamicRHI::FVulkanDynamicRHI()
	{
		KR_CORE_INFO("Initializing Vulkan RHI...");

		CreateInstance();
		SetupDebugMessenger();
		CreateSurface();
		SelectDevice();
	}

	bool FVulkanDynamicRHI::Init()
	{
		// initialize device
		KR_CORE_INFO("Vulkan RHI initialized successfully.");
		return true;
	}

	void FVulkanDynamicRHI::Shutdown()
	{
		// destroy device
		vkDestroySurfaceKHR(m_VulkanInstance, m_Surface, nullptr);
		DestroyDebugUtilsMessengerEXT(m_VulkanInstance, m_DebugMessenger, nullptr);
		vkDestroyInstance(m_VulkanInstance, nullptr);
		KR_CORE_INFO("Vulkan RHI shutdown complete by destroying Vulkan Instance.");
	}

	void FVulkanDynamicRHI::CreateInstance()
	{
		// Implementation for creating a Vulkan instance
		if (bEnableValidationLayers)
		{
			PrintAvailableExtensions();
		}

		if (bEnableValidationLayers && !CheckValidationLayerSupport())
		{
			KR_CORE_WARN("Validation layers requested, but not available");
		}

		// Optional information about the application (or Engine in our case)
		// TODO: Fill this info from Engine config (if/when we have the config)
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
#ifdef KR_APPLICATION_NAME
		appInfo.pApplicationName = KR_APPLICATION_NAME;
#else
		appInfo.pApplicationName = "No Name";
#endif
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Karma";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		// Tell Vulkan which global extensions and validation layers we want to use
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		// Validation layers
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (bEnableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)(&debugCreateInfo);
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		VkInstanceCreateFlags flagsToBeSet{};
		auto extensions = GetRequiredExtensions(flagsToBeSet);
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		createInfo.flags |= flagsToBeSet;

		VkResult result = vkCreateInstance(&createInfo, nullptr, &m_VulkanInstance);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan m_Instance.");
		
		KR_CORE_INFO("Vulkan Instance created successfully.");
	}

	void FVulkanDynamicRHI::SetupDebugMessenger()
	{
		if (!bEnableValidationLayers)
		{
			return;
		}

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo(createInfo);

		VkResult result = CreateDebugUtilsMessengerEXT(m_VulkanInstance, &createInfo, nullptr, &m_DebugMessenger);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to set up debug messenger!");
	}

	void FVulkanDynamicRHI::CreateSurface()
	{
		m_WindowHandle = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		VkResult result;

		if (m_WindowHandle != nullptr)
		{
			result = glfwCreateWindowSurface(m_VulkanInstance, m_WindowHandle, nullptr, &m_Surface);
		}

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create window surface");
	}

	void FVulkanDynamicRHI::SelectDevice()
	{
		// Implementation for selecting a physical device (GPU)
		
	}

	void FVulkanDynamicRHI::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL FVulkanDynamicRHI::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			KR_CORE_INFO("Validation Layer: {0}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			KR_CORE_WARN("Validation Layer: {0}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			KR_CORE_ERROR("Validation Layer: {0}", pCallbackData->pMessage);
			break;
		default:
			//KR_CORE_TRACE("Validation Layer: {0}", pCallbackData->pMessage);
			break;
		}

		return VK_FALSE;
	}

	// Return the required list of instance extensions based on whether validation layers are
	// enabled or not
	std::vector<const char*> FVulkanDynamicRHI::GetRequiredExtensions(VkInstanceCreateFlags& flagsToBeSet)
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef KR_MAC_PLATFORM
		// Case by case query for required instance extensions
		// One for MacOS: VK_KHR_portability_enumeration
		uint32_t extensionCount = 0;

		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> vulkanExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vulkanExtensions.data());

		uint32_t index = 1;
		for (auto anExtension : vulkanExtensions)
		{
			if (strcmp(anExtension.extensionName, "VK_KHR_portability_enumeration"))
			{
				extensions.push_back("VK_KHR_portability_enumeration");
				flagsToBeSet = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
				break;
			}
		}
#endif
		if (bEnableValidationLayers)// Enable printing of extensions when validation layers are enabled (debug mode)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

			uint32_t index = 1;
			KR_CORE_INFO("+-------------------------------------------------");
			KR_CORE_INFO("| GLFW and other required instance extensions:");
			for (auto extension : extensions)
			{
				KR_CORE_INFO("| {0}. {1}", index++, extension);
			}
			KR_CORE_INFO("+-------------------------------------------------");
		}

		return extensions;
	}

	void FVulkanDynamicRHI::PrintAvailableExtensions()
	{
		uint32_t extensionCount = 0;

		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> vulkanExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vulkanExtensions.data());

		uint32_t index = 1;
		KR_CORE_INFO("+-------------------------------------------------");
		KR_CORE_INFO("| Available Vulkan instance extensions:");
		for (auto extension : vulkanExtensions)
		{
			KR_CORE_INFO("| {0}. {1}", index++, extension.extensionName);
		}
		KR_CORE_INFO("+-------------------------------------------------");
	}

	bool FVulkanDynamicRHI::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	VkResult FVulkanDynamicRHI::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void FVulkanDynamicRHI::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		if (!bEnableValidationLayers)
		{
			return;
		}

		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}
}