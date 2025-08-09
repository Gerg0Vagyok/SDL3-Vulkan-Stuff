#include "hardware_device.h"
#include <stdio.h>
#include <stdlib.h>

int IsDeviceSuitable(VkPhysicalDevice Device) {
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(Device, &props);
	return 	props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
			props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}

void GetPhysicalDevice(VkPhysicalDevice *physicalDevice, VkInstance instance, int Input_DeviceNumber) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
	if (deviceCount == 0) {
	    fprintf(stderr, "Failed to find GPUs with Vulkan support.\n");
	    exit(1);
	}

	VkPhysicalDevice devices[deviceCount];
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

	if (Input_DeviceNumber >= (int)deviceCount || Input_DeviceNumber < 0) {
		Input_DeviceNumber = 0;
	}

	for (uint32_t i = Input_DeviceNumber; i < deviceCount; i++) {
		if (IsDeviceSuitable(devices[i])) {
			*physicalDevice = devices[i];
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		for (uint32_t i = 0; (int)i < Input_DeviceNumber; i++) {
			if (IsDeviceSuitable(devices[i])) {
				*physicalDevice = devices[i];
				break;
			}
		}
	}
}
