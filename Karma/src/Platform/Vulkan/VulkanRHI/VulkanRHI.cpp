#include "VulkanDynamicRHI.h"
#include "GLFW/glfw3.h"

#include "Application.h"

namespace Karma
{
	const std::vector<const char*> FVulkanDynamicRHI::validationLayers = {"VK_LAYER_KHRONOS_validation"};
	std::vector<const char*> FVulkanDynamicRHI:: deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

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
		SelectDevice();// physical device selection and logical device creation
	}

	bool FVulkanDynamicRHI::Init()
	{
		// initialize device
		InitInstance();

		KR_CORE_INFO("Vulkan RHI initialized successfully.");
		return true;
	}

	void FVulkanDynamicRHI::InitInstance()
	{
		m_Device->InitGPU();
	}

	void FVulkanDynamicRHI::Shutdown()
	{
		m_Device->Destroy();
		vkDestroySurfaceKHR(m_VulkanInstance, m_Surface, nullptr);
		DestroyDebugUtilsMessengerEXT(m_VulkanInstance, m_DebugMessenger, nullptr);
		vkDestroyInstance(m_VulkanInstance, nullptr);

		delete m_Device;
		m_Device = nullptr;

		KR_CORE_INFO("Vulkan RHI shutdown complete");
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
		KR_CORE_INFO("Vulkan RHI finished setting up Vulkan debug messenger with appropriate logging callback");
	}

	void FVulkanDynamicRHI::CreateSurface()
	{
		m_WindowHandle = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		VkResult result;

		if (m_WindowHandle != nullptr)
		{
			result = glfwCreateWindowSurface(m_VulkanInstance, m_WindowHandle, nullptr, &m_Surface);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create window surface");

			KR_CORE_INFO("Vulkan RHI successfully created abstract surface to present rendered images");
		}
		else
		{
			KR_CORE_ASSERT(false, "No GLFW window handle found for surface creation");
		}
	}

	void FVulkanDynamicRHI::ComputeNumberOfSwapchainImagesSupported()
	{
		m_SwapChainImageCount = m_GPUSwapChainSupport.capabilities.minImageCount + 1;

		if (m_GPUSwapChainSupport.capabilities.maxImageCount > 0 && m_SwapChainImageCount > m_GPUSwapChainSupport.capabilities.maxImageCount)
		{
			m_SwapChainImageCount = m_GPUSwapChainSupport.capabilities.maxImageCount;
		}
	}

	void FVulkanDynamicRHI::SelectDevice()// Pick physical device (GPU) and create Vulkan logical device
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			KR_CORE_ASSERT(false, "Failed to load GPU with Vulkan support");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, devices.data());

		if (bEnableValidationLayers)
		{
			PrintAvailablePhysicalDevices(devices);
		}

		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device))
			{
				m_PhysicalDevice = device;
				ComputeNumberOfSwapchainImagesSupported();

				break;
			}
		}

		if (m_PhysicalDevice == VK_NULL_HANDLE)
		{
			KR_CORE_ASSERT(false, "Failed to find a suitable GPU!");
		}
		else
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(m_PhysicalDevice, &deviceProperties);

			KR_CORE_INFO("Physical Device (GPU) {0} has appropriate support for Karma Engine", deviceProperties.deviceName);
		}

		m_Device = new FVulkanDevice(this, m_PhysicalDevice);
	}

	bool FVulkanDynamicRHI::IsDeviceSuitable(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices = FindQueueFamilies(device);

		bool bExtensionsSupported = CheckDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (bExtensionsSupported)
		{
			m_GPUSwapChainSupport = QuerySwapChainSupport(device);
			swapChainAdequate = !m_GPUSwapChainSupport.formats.empty() && !m_GPUSwapChainSupport.presentModes.empty();
		}

		vkGetPhysicalDeviceFeatures(device, &m_SupportedDeviceFeatures);

		return indices.IsComplete() && bExtensionsSupported && swapChainAdequate && m_SupportedDeviceFeatures.samplerAnisotropy;
	}

	SwapChainSupportDetails FVulkanDynamicRHI::QuerySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	// Check if all the required extensions are there
	bool FVulkanDynamicRHI::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

#ifdef KR_MAC_PLATFORM
		// Case by case query for required extensions
		// One for MacOS: VK_KHR_portability_subset
		for (auto anExtention : availableExtensions)
		{
			if (strcmp(anExtention.extensionName, "VK_KHR_portability_subset") != 0)
			{
				deviceExtensions.push_back("VK_KHR_portability_subset");
				break;
			}
		}
#endif

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		if (bEnableValidationLayers)
		{
			KR_CORE_INFO("+-------------------------------------------------");
			KR_CORE_INFO("| Available Device (GPU) Extensions:");
			uint32_t index = 1;
			for (auto anExtension : availableExtensions)
			{
				KR_CORE_INFO("| {0}. {1}", index++, anExtension.extensionName);
			}
			KR_CORE_INFO("+-------------------------------------------------");
			KR_CORE_INFO("+-------------------------------------------------");
			KR_CORE_INFO("| Required Extensions (shall be enabled...):");
			index = 1;
			for (auto swapchainExtension : requiredExtensions)
			{
				KR_CORE_INFO("| {0}. {1}", index++, swapchainExtension);
			}
			KR_CORE_INFO("+-------------------------------------------------");
		}

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	QueueFamilyIndices FVulkanDynamicRHI::FindQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);

			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			if (indices.IsComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

	void FVulkanDynamicRHI::PrintAvailablePhysicalDevices(const std::vector<VkPhysicalDevice>& physicalDevices)
	{
		uint32_t index = 1;
		KR_CORE_INFO("+-------------------------------------------------");
		KR_CORE_INFO("| Available Graphics cards:");
		for (auto physicalDevice : physicalDevices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
			KR_CORE_INFO("| {0}. Device Name: {1}", index++, deviceProperties.deviceName);
		}
		KR_CORE_INFO("+-------------------------------------------------");
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