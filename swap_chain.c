#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL3/SDL.h>
#include "swap_chain.h"
#include "queue_families.h"

VkExtent2D ChooseExtent(VkSurfaceCapabilitiesKHR *capabilities, SDL_Window *window) {
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    }
    
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    
    VkExtent2D actualExtent = {(uint32_t)width, (uint32_t)height};
    
    actualExtent.width = (actualExtent.width < capabilities->minImageExtent.width) ? 
                         capabilities->minImageExtent.width : actualExtent.width;
    actualExtent.width = (actualExtent.width > capabilities->maxImageExtent.width) ? 
                         capabilities->maxImageExtent.width : actualExtent.width;
                         
    actualExtent.height = (actualExtent.height < capabilities->minImageExtent.height) ? 
                          capabilities->minImageExtent.height : actualExtent.height;
    actualExtent.height = (actualExtent.height > capabilities->maxImageExtent.height) ? 
                          capabilities->maxImageExtent.height : actualExtent.height;
    
    return actualExtent;
}

VkResult GetSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR Surface, VkSurfaceCapabilitiesKHR *Capabilities, VkSwapchainKHR *swapchain, VkSurfaceFormatKHR *surfaceFormat, VkPresentModeKHR *presentMode, QueueFamilyIndices QueueFamilies, SDL_Window *window) {
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, Surface, &formatCount, NULL);
	
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, Surface, &presentModeCount, NULL);

	if (formatCount == 0 || presentModeCount == 0) {
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	/*VkSurfaceFormatKHR *availableFormats = malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	VkPresentModeKHR *availablePresentModes = malloc(presentModeCount * sizeof(VkPresentModeKHR));

	if (!availableFormats || !availablePresentModes) {
		free(availableFormats);
		free(availablePresentModes);
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}*/

	VkSurfaceFormatKHR availableFormats[formatCount];
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, Surface, &formatCount, availableFormats);
	VkPresentModeKHR availablePresentModes[presentModeCount];
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, Surface, &presentModeCount, availablePresentModes);

	*surfaceFormat = availableFormats[0]; // FALLBACK
	for (uint32_t i = 0; i < formatCount; i++) {
		if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			*surfaceFormat = availableFormats[i];
			break;
		}
	}

	*presentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (uint32_t i = 0; i < presentModeCount; i++) {
		if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			*presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
	}

	uint32_t imageCount = Capabilities->minImageCount + 1;
	if (Capabilities->maxImageCount > 0 && imageCount > Capabilities->maxImageCount) {
		imageCount = Capabilities->maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainInfo = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = Surface,
		.minImageCount = imageCount,
		.imageFormat = surfaceFormat->format,
		.imageColorSpace = surfaceFormat->colorSpace,
		.imageExtent = ChooseExtent(Capabilities, window),
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = (QueueFamilies.graphicsFamily != QueueFamilies.presentFamily) ? 
							VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = (QueueFamilies.graphicsFamily != QueueFamilies.presentFamily) ? 2 : 0,
		.pQueueFamilyIndices = (QueueFamilies.graphicsFamily != QueueFamilies.presentFamily) ? 
								(uint32_t[]){QueueFamilies.graphicsFamily, QueueFamilies.presentFamily} : NULL,
		.preTransform = Capabilities->currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = *presentMode,
		.clipped = VK_TRUE
	};

	return  vkCreateSwapchainKHR(device, &swapchainInfo, NULL, swapchain);
/*	VkResult result = vkCreateSwapchainKHR(device, &swapchainInfo, NULL, swapchain);

	free(availableFormats);
	free(availablePresentModes);

	return result;*/
}
