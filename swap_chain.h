#ifndef SWAP_CHAIN_H
#define SWAP_CHAIN_H

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include "queue_families.h"

VkExtent2D ChooseExtent(VkSurfaceCapabilitiesKHR *capabilities, SDL_Window *window);
VkResult GetSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR Surface, VkSurfaceCapabilitiesKHR *Capabilities, VkSwapchainKHR *swapchain, VkSurfaceFormatKHR *surfaceFormat, VkPresentModeKHR *presentMode, QueueFamilyIndices QueueFamilies, SDL_Window *window);

#endif // SWAP_CHAIN_H

