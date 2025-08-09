#ifndef LOGICAL_DEVICE_H
#define LOGICAL_DEVICE_H

#include <vulkan/vulkan.h>
#include "queue_families.h"

VkResult CreateVulkanDevice(VkPhysicalDevice physicalDevice, QueueFamilyIndices QueueFamilies, VkDevice *device);

#endif // LOGICAL_DEVICE_H
