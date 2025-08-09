#ifndef QUEUE_FAMILIES_H
#define QUEUE_FAMILIES_H

#include <vulkan/vulkan.h>

typedef struct QueueFamilyIndices {
    int graphicsFamily;
    int presentFamily;
} QueueFamilyIndices;

void findOptimalQueueFamilies(QueueFamilyIndices *Indicies, VkPhysicalDevice device, VkSurfaceKHR surface);

#endif // QUEUE_FAMILIES_H

