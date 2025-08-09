#include <vulkan/vulkan.h>
#include "queue_families.h"

VkResult CreateVulkanDevice(VkPhysicalDevice physicalDevice, QueueFamilyIndices QueueFamilies, VkDevice *device) {
	float queuePriority = 1.0f;
	if (QueueFamilies.graphicsFamily == QueueFamilies.presentFamily) {
		VkDeviceQueueCreateInfo CreateInfoQueueFamilies[1];

		CreateInfoQueueFamilies[0] = (VkDeviceQueueCreateInfo) {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.queueFamilyIndex = QueueFamilies.graphicsFamily,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};

		VkDeviceCreateInfo deviceCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = CreateInfoQueueFamilies,
			.enabledLayerCount = 0,
			.ppEnabledLayerNames = NULL,
			.enabledExtensionCount = 1,
			.ppEnabledExtensionNames = (const char*[]){VK_KHR_SWAPCHAIN_EXTENSION_NAME},
			.pEnabledFeatures = NULL
		};

		return vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, device);
	} else {
		VkDeviceQueueCreateInfo CreateInfoQueueFamilies[2];

		CreateInfoQueueFamilies[0] = (VkDeviceQueueCreateInfo) {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.queueFamilyIndex = QueueFamilies.graphicsFamily,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};

		CreateInfoQueueFamilies[1] = (VkDeviceQueueCreateInfo) {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.queueFamilyIndex = QueueFamilies.presentFamily,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};

		VkDeviceCreateInfo deviceCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.queueCreateInfoCount = 2,
			.pQueueCreateInfos = CreateInfoQueueFamilies,
			.enabledLayerCount = 0,
			.ppEnabledLayerNames = NULL,
			.enabledExtensionCount = 1,
			.ppEnabledExtensionNames = (const char*[]){VK_KHR_SWAPCHAIN_EXTENSION_NAME},
			.pEnabledFeatures = NULL
		};

		return vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, device);
	}
}
