#include "queue_families.h"
#include <vulkan/vulkan.h>
#include <stdio.h>

void findOptimalQueueFamilies(QueueFamilyIndices *Indicies, VkPhysicalDevice device, VkSurfaceKHR surface) {
	if (Indicies == NULL) {
		fprintf(stderr, "Indicies is NULL -> findOptimalQueueFamilies()!\n");
		return;
	}
	Indicies->graphicsFamily = -1;
	Indicies->presentFamily = -1;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

	VkQueueFamilyProperties queueFamilies[queueFamilyCount];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			Indicies->graphicsFamily = i;
		}

		VkBool32 PresentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &PresentSupport);

		if (PresentSupport) {
			Indicies->presentFamily = i;
		}

		if (Indicies->graphicsFamily >= 0 && Indicies->presentFamily >= 0) {
			break;
		}
	}

	if (Indicies->graphicsFamily < 0) {
		fprintf(stderr, "Device does not have a graphics-capable queue family.\n");
	}
	if (Indicies->presentFamily < 0) {
		fprintf(stderr, "Device does not have a present-capable queue family.\n");
	}
}
