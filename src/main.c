#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <limits.h>
#include "hardware_device.h"
#include "queue_families.h"
#include "logical_device.h"
#include "swap_chain.h"
#include "shader_module_loader.h"
#include "error.h"

struct PassData {
	int width;
	int height;
	double x;
	double y;
	double scale;
};

int main(int argc, char **argv) {
	// This will be used later i swear!
	//int Input_Modes = 0;
	int Input_DeviceNumber = 0;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--help") == 0) {
			printf("Help menu i guess.\n");
			return 0;
		} else if (strcmp(argv[i], "--device-index") == 0 && i+1 != argc) {
			char *endptr;
			long value = strtol(argv[i+1], &endptr, 10);
			if (*endptr == '\0' && value >= INT_MIN && value <= INT_MAX) {
				Input_DeviceNumber = (int)value;
				i++;
			}
		} else {
			error(GetErrorFlags(1, REACTION_IGNORE, AUTO_NEWLINE), "Invalid arg: %s", argv[i]);
		}
	}



	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window *window = SDL_CreateWindow("Vulkan With SDL3 - By G0V", 800, 800, SDL_WINDOW_VULKAN);
	SDL_SetWindowResizable(window, 1);
	Uint32 VulkanNumberExtensions = 0;
	char const * const * VulkanExtensions = SDL_Vulkan_GetInstanceExtensions(&VulkanNumberExtensions);
	if (VulkanExtensions == NULL) {
		error(GetErrorFlags(2, REACTION_FATAL, AUTO_NEWLINE), "Failed to get Instance Extensions! Error: %s", SDL_GetError());
	}
	SDL_FunctionPointer GetInstanceProcessAddress = SDL_Vulkan_GetVkGetInstanceProcAddr();
	if (GetInstanceProcessAddress == NULL) {
		error(GetErrorFlags(2, REACTION_FATAL, AUTO_NEWLINE), "Failed to get Get Instance Process Address! Error: %s", SDL_GetError());
	}
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetInstanceProcessAddress;
	PFN_vkCreateInstance vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(NULL, "vkCreateInstance");
	VkInstance instance;
	VkApplicationInfo appInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = NULL,
		.pApplicationName = "TestApp",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "None",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_0,
	};
	VkInstanceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = NULL,
		.enabledExtensionCount = VulkanNumberExtensions,
		.ppEnabledExtensionNames = VulkanExtensions,
	};

	VkResult VulkanResultVar; // for holding vlaues to not make 100 vars just for this.
	VulkanResultVar = vkCreateInstance(&createInfo, NULL, &instance);
	if (VulkanResultVar != VK_SUCCESS) {
		error(GetErrorFlags(2, REACTION_FATAL, AUTO_NEWLINE), "Failed to create Vulkan Instance! Error code: %d", VulkanResultVar);
	}

	VkSurfaceKHR Surface;
	if (!SDL_Vulkan_CreateSurface(window, instance, NULL, &Surface)) {
		error(GetErrorFlags(2, REACTION_FATAL, AUTO_NEWLINE), "Failed to create Vulkan Surface! Error: %s", SDL_GetError());
	}

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	GetPhysicalDevice(&physicalDevice, instance, Input_DeviceNumber);
	if (physicalDevice == VK_NULL_HANDLE) {
		error(GetErrorFlags(2, REACTION_FATAL, AUTO_NEWLINE), "Failed to find GPU that supports Vulkan! No error code!");
	}

	QueueFamilyIndices QueueFamilies = {-1, -1};
	findOptimalQueueFamilies(&QueueFamilies, physicalDevice, Surface);
	if (QueueFamilies.graphicsFamily == -1 || QueueFamilies.presentFamily == -1) {
		error(GetErrorFlags(2, REACTION_FATAL, AUTO_NEWLINE), "Failed to find Queue Families! No error code!");
	}

	VkDevice device;
	VulkanResultVar = CreateVulkanDevice(physicalDevice, QueueFamilies, &device);
	if (VulkanResultVar != VK_SUCCESS) {
		error(GetErrorFlags(2, REACTION_FATAL, AUTO_NEWLINE), "Failed to create Logical Device! Error code: %d", VulkanResultVar);
	}

	VkQueue graphicsQueue, presentQueue;
	vkGetDeviceQueue(device, QueueFamilies.graphicsFamily, 0, &graphicsQueue);
	if (graphicsQueue == NULL) {
		error(GetErrorFlags(2, REACTION_FATAL, AUTO_NEWLINE), "Failed to create Graphics Family! No error code!");
	}
	vkGetDeviceQueue(device, QueueFamilies.presentFamily, 0, &presentQueue);
	if (presentQueue == NULL) {
		error(GetErrorFlags(2, REACTION_FATAL, AUTO_NEWLINE), "Failed to create Present Family! No error code!");
	}

	VkSurfaceCapabilitiesKHR Capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, Surface, &Capabilities);

	VkSurfaceFormatKHR surfaceFormat;
	VkPresentModeKHR presentMode;
	VkSwapchainKHR swapchain;
	VulkanResultVar = GetSwapchain(physicalDevice, device, Surface, &Capabilities, &swapchain, &surfaceFormat, &presentMode, QueueFamilies, window);
	if (VulkanResultVar != VK_SUCCESS) {
		error(GetErrorFlags(2, REACTION_FATAL, AUTO_NEWLINE), "Failed to create Swapchain! Error code: %d", VulkanResultVar);
	}

	uint32_t imageCount;
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, NULL);
	if (imageCount == 0) {
		error(GetErrorFlags(2, REACTION_FATAL, AUTO_NEWLINE), "Failed to get Swapchain Image Count! Error code: %d", VulkanResultVar);
	}
	VkImage swapchainImages[imageCount];
	if (vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages) != VK_SUCCESS) {
		error(GetErrorFlags(2, REACTION_FATAL, AUTO_NEWLINE), "Failed to get Swapchain Images! Error code: %d", VulkanResultVar);
	}

	VkImageView swapchainImageViews[imageCount];
	for (uint32_t i = 0; i < imageCount; i++) {
		VkImageViewCreateInfo viewInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = swapchainImages[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = surfaceFormat.format, // from swapchain creation
			.components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY, 
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};
		vkCreateImageView(device, &viewInfo, NULL, &swapchainImageViews[i]);
	}

	VkAttachmentDescription colorAttachment = {
		.format = surfaceFormat.format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};
	
	VkAttachmentReference colorAttachmentRef = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};
	
	VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef
	};
	
	VkRenderPassCreateInfo renderPassInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &colorAttachment,
		.subpassCount = 1,
		.pSubpasses = &subpass
	};

	VkRenderPass renderPass;
	vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass);

	VkFramebuffer swapchainFramebuffers[imageCount];
	for (uint32_t i = 0; i < imageCount; i++) {
		VkFramebufferCreateInfo framebufferInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = renderPass,
			.attachmentCount = 1,
			.pAttachments = &swapchainImageViews[i],
			.width = Capabilities.currentExtent.width,
			.height = Capabilities.currentExtent.height,
			.layers = 1
		};
		vkCreateFramebuffer(device, &framebufferInfo, NULL, &swapchainFramebuffers[i]);
	}

	VkShaderModule VertexShaderModule, FragmentShaderModule;
	VulkanResultVar = LoadShadersFromFiles(device, "spir-v/triangle.vert.spv", "spir-v/triangle.frag.spv", &VertexShaderModule, &FragmentShaderModule);
	if (VulkanResultVar != VK_SUCCESS) {
		error(GetErrorFlags(2, REACTION_FATAL, AUTO_NEWLINE), "Failed to create Shader Modules! Error code: %d", VulkanResultVar);
	}

	VkPipelineShaderStageCreateInfo shaderStages[] = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = VertexShaderModule,
			.pName = "main"
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = FragmentShaderModule,
			.pName = "main"
		}
	};

	VkPushConstantRange pushConstantRange = {
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		.offset = 0,
		.size = sizeof(struct PassData),
	};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 0,
		.vertexAttributeDescriptionCount = 0
	};
	
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};

	VkPipelineDynamicStateCreateInfo dynamicStateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = dynamicStates,
	};

	VkViewport viewport = {
		.x = 0.0f, .y = 0.0f,
		.width = (float)ChooseExtent(&Capabilities, window).width,
		.height = (float)ChooseExtent(&Capabilities, window).height,
		.minDepth = 0.0f, .maxDepth = 1.0f
	};
	
	VkRect2D scissor = {
		.offset = {0, 0},
		.extent = Capabilities.currentExtent
	};

	VkPipelineViewportStateCreateInfo viewportState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1, .pViewports = &viewport,
		.scissorCount = 1, .pScissors = &scissor
	};

	VkPipelineRasterizationStateCreateInfo rasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.lineWidth = 1.0f,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE
	};

	VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.sampleShadingEnable = VK_FALSE,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
	};

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_FALSE
	};

	VkPipelineColorBlendStateCreateInfo colorBlending = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &pushConstantRange
	};

	VkPipelineLayout pipelineLayout;
	vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout);

	VkGraphicsPipelineCreateInfo pipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = shaderStages,
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &inputAssembly,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pColorBlendState = &colorBlending,
		.layout = pipelineLayout,
		.renderPass = renderPass,
		.subpass = 0,
		.pDynamicState = &dynamicStateInfo,
	};

	VkPipeline graphicsPipeline;
	vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &graphicsPipeline);
	VkCommandPoolCreateInfo poolInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = QueueFamilies.graphicsFamily
	};

	VkCommandPool commandPool;
	vkCreateCommandPool(device, &poolInfo, NULL, &commandPool);

	VkCommandBufferAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkSemaphoreCreateInfo semaphoreInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};

	VkFenceCreateInfo fenceInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	VkSemaphore imageAvailableSemaphore, renderFinishedSemaphore;
	VkFence inFlightFence;

	vkCreateSemaphore(device, &semaphoreInfo, NULL, &imageAvailableSemaphore);
	vkCreateSemaphore(device, &semaphoreInfo, NULL, &renderFinishedSemaphore);
	vkCreateFence(device, &fenceInfo, NULL, &inFlightFence);

	struct PassData constants = {
		.width = ChooseExtent(&Capabilities, window).width,
		.height = ChooseExtent(&Capabilities, window).height,
		.x = 0.0,
		.y = 0.0,
		.scale = 1.0
	};

	int shouldRender = 1;
	int windowResized = 1;
	int MouseDown = 0;
	int MouseMove = 0;
	int MouseScrolled = 0;
	int running = 1;
	while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {running = 0;}
			else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
				windowResized = 1;
				shouldRender = 1;
			}
			else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) {
				MouseDown = 0;
				shouldRender = 1;
			}
			else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
				MouseDown = 1;
				shouldRender = 1;
			}
			else if (event.type == SDL_EVENT_MOUSE_MOTION && MouseDown) {
				constants.x += event.motion.xrel/constants.scale;
				constants.y += event.motion.yrel/constants.scale;
				shouldRender = 1;
			}
			else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
				if (event.wheel.y < 0) {
					constants.scale /= 1.1;
					MouseScrolled = 1;
				} else if (event.wheel.y > 0) {
					constants.scale *= 1.1;
					MouseScrolled = 1;
				}
				shouldRender = 1;
			}
		}

		if (!shouldRender) continue;

		int w, h;
		SDL_GetWindowSize(window, &w, &h);
		if (w == 0 || h == 0) continue;

		if (windowResized) {
			vkDeviceWaitIdle(device);

			for (uint32_t i = 0; i < imageCount; i++) {
				vkDestroyImageView(device, swapchainImageViews[i], NULL);
				vkDestroyFramebuffer(device, swapchainFramebuffers[i], NULL);
			}
			vkDestroySwapchainKHR(device, swapchain, NULL);

			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, Surface, &Capabilities);
			
			VulkanResultVar = GetSwapchain(physicalDevice, device, Surface, &Capabilities, 
										   &swapchain, &surfaceFormat, &presentMode, QueueFamilies, window);
			if (VulkanResultVar != VK_SUCCESS) {
				error(GetErrorFlags(2, REACTION_FATAL, AUTO_NEWLINE), "Failed to recreate Swapchain! Error code: %d", VulkanResultVar);
			}
			
			vkGetSwapchainImagesKHR(device, swapchain, &imageCount, NULL);
			vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages);
			
			for (uint32_t i = 0; i < imageCount; i++) {
				VkImageViewCreateInfo viewInfo = {
					.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
					.image = swapchainImages[i],
					.viewType = VK_IMAGE_VIEW_TYPE_2D,
					.format = surfaceFormat.format,
					.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, 
								   VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
					.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
				};    
				vkCreateImageView(device, &viewInfo, NULL, &swapchainImageViews[i]);
				      
				VkFramebufferCreateInfo framebufferInfo = {
					.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
					.renderPass = renderPass,
					.attachmentCount = 1,
					.pAttachments = &swapchainImageViews[i],
					.width = Capabilities.currentExtent.width,
					.height = Capabilities.currentExtent.height,
					.layers = 1
				};
				vkCreateFramebuffer(device, &framebufferInfo, NULL, &swapchainFramebuffers[i]);
			}

			viewport = (VkViewport){
				.x = 0.0f, .y = 0.0f,
				.width = (float)ChooseExtent(&Capabilities, window).width,
				.height = (float)ChooseExtent(&Capabilities, window).height,
				.minDepth = 0.0f, .maxDepth = 1.0f
			};
			
			scissor = (VkRect2D){
				.offset = {0, 0},
				.extent = Capabilities.currentExtent
			};
		}

		vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &inFlightFence);

		uint32_t imageIndex;
		vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		vkResetCommandBuffer(commandBuffer, 0);

		VkCommandBufferBeginInfo beginInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
		};
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkClearValue clearColor = {{{0.0f, 1.0f, 0.0f, 1.0f}}};
		VkRenderPassBeginInfo renderPassInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = renderPass,
			.framebuffer = swapchainFramebuffers[imageIndex],
			.renderArea = {{0, 0}, Capabilities.currentExtent},
			.clearValueCount = 1,
			.pClearValues = &clearColor
		};

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		if (windowResized) {
			constants.width = ChooseExtent(&Capabilities, window).width;
			constants.height = ChooseExtent(&Capabilities, window).height;

			vkCmdPushConstants(commandBuffer, pipelineLayout, 
							   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, 
							   sizeof(struct PassData), &constants);

			windowResized = 0;
		}

		vkCmdPushConstants(commandBuffer, pipelineLayout, 
						   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, 
						   sizeof(struct PassData), &constants);

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffer);
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &imageAvailableSemaphore,
			.pWaitDstStageMask = (VkPipelineStageFlags[]){VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
			.commandBufferCount = 1,
			.pCommandBuffers = &commandBuffer,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &renderFinishedSemaphore
		};

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence);

		VkPresentInfoKHR presentInfo = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &renderFinishedSemaphore,
			.swapchainCount = 1,
			.pSwapchains = &swapchain,
			.pImageIndices = &imageIndex
		};

		printf("Mouse X: %f | Mouse Y: %f\nWidth: %d | Height: %d\nScale: %f\n", constants.x, constants.y, constants.width, constants.height, constants.scale);

		vkQueuePresentKHR(presentQueue, &presentInfo);
		shouldRender = 0;
		MouseScrolled = 0;
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
