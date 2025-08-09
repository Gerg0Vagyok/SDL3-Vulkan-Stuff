#ifndef HARDWARE_DEVICE_H
#define HARDWARE_DEVICE_H

#include <vulkan/vulkan.h>

int IsDeviceSuitable(VkPhysicalDevice Device);
void GetPhysicalDevice(VkPhysicalDevice *physicalDevice, VkInstance instance, int Input_DeviceNumber);

#endif // HARDWARE_DEVICE_H
