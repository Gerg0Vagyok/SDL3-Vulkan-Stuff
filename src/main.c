#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3_image/SDL_image.h>
#include <vulkan/vulkan.h>
#include <limits.h>
#include "globals.h"
#include "hardware_device.h"
#include "queue_families.h"
#include "logical_device.h"
#include "swap_chain.h"
#include "shader_module_loader.h"
#include "error.h"
#include "metadata.h"

struct PassData {
	int width;
	int height;
};

int main(int argc, char **argv) {
	int Input_DeviceNumber = 0;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--help") == 0) {
			printf("%s! %s Branch!\n", NAME, BRANCH);
			printf("Made by Gerg0Vagyok\n");
			printf("\n");
			printf("How to use:\n");
			printf("./main [args]\n");
			printf("\n");
			printf("Args:\n");
			printf("    --help                      Display this menu and exit.\n");
			printf("    --version                   Display version number and exit.\n");
			printf("    --device-index [number]     Specify which device to use from the device list,\n");
			printf("                                if its over the max loop back to the first.\n");
			printf("    --test-error-fatal          Display test fatal error.\n");
			printf("    --test-error-warning        Display test warning.\n");
			printf("    --test-error-ignore         Display test ignore.\n");
			printf("\n");
			return 0;
		} else if (strcmp(argv[i], "--version") == 0) {
			printf("%s\n%s\n", NAME_STR, VERSION_STR);
			return 0;
		} else if (strcmp(argv[i], "--verbose") == 0) {
			SetArgFlag(VERBOSE, 1);
		} else if (strcmp(argv[i], "--device-index") == 0 && i+1 != argc) {
			char *endptr;
			long value = strtol(argv[i+1], &endptr, 10);
			if (*endptr == '\0' && value >= INT_MIN && value <= INT_MAX) {
				Input_DeviceNumber = (int)value;
				i++;
			}
		} else if (strcmp(argv[i], "--test-error-fatal") == 0) {
			error(GetErrorFlags(2, REACTION_FATAL, AUTO_NEWLINE), "'%s' argument passed!", argv[i]);
		} else if (strcmp(argv[i], "--test-error-warning") == 0) {
			error(GetErrorFlags(2, REACTION_WARNING, AUTO_NEWLINE), "'%s' argument passed!", argv[i]);
		} else if (strcmp(argv[i], "--test-error-ignore") == 0) {
			error(GetErrorFlags(2, REACTION_IGNORE, AUTO_NEWLINE), "'%s' argument passed!", argv[i]);
		} else {
			error(GetErrorFlags(2, REACTION_IGNORE, AUTO_NEWLINE), "Invalid arg: %s", argv[i]);
		}
	}



	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window *window = SDL_CreateWindow("Vulkan With SDL3 - By G0V", 800, 800, SDL_WINDOW_VULKAN);
	#if WINDOW_RESIZABLE
	SDL_SetWindowResizable(window, WINDOW_RESIZABLE);
	#endif
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
		.pApplicationName = VULKAN_APP_NAME,
		.applicationVersion = VK_MAKE_VERSION(VULKAN_APP_VER1, VULKAN_APP_VER2, VULKAN_APP_VER3),
		.pEngineName = VULKAN_ENGINE_NAME,
		.engineVersion = VK_MAKE_VERSION(VULKAN_ENGINE_VER1, VULKAN_ENGINE_VER2, VULKAN_ENGINE_VER3),
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
	};;

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

	#if WINDOW_RESIZABLE
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};

	VkPipelineDynamicStateCreateInfo dynamicStateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = dynamicStates,
	};
	#endif

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
		#if WINDOW_RESIZABLE
		.pDynamicState = &dynamicStateInfo,
		#endif
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
	};

	SDL_Surface *FONT_ATLAS_u = IMG_Load("font_atlas.png");
	if (!FONT_ATLAS_u) {
		error(GetErrorFlags(2, REACTION_FATAL, AUTO_NEWLINE), "Failed to load Font Atlas! No error code!");
	}

	SDL_Surface *FONT_ATLAS = SDL_ConvertSurface(FONT_ATLAS_u, SDL_PIXELFORMAT_RGBA32);
	SDL_DestroySurface(FONT_ATLAS_u);
	if (!FONT_ATLAS) {
		error(GetErrorFlags(2, REACTION_FATAL, AUTO_NEWLINE), "Failed to load Font Atlas! No error code!");
	}

	int FONT_ATLAS_W = FONT_ATLAS->w; // Get image width
	int FONT_ATLAS_H = FONT_ATLAS->h; // Get image height
	void *FONT_ATLAS_P = FONT_ATLAS->pixels; // Get image pixels
	VkDeviceSize FONT_ATLAS_S = (VkDeviceSize)FONT_ATLAS_W * FONT_ATLAS_H * 4; // Image Size

	// Add code to load image to gpu and stuff

	int shouldRender = 1;
	int windowResized = 1;
	int running = 1;
	while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {running = 0;}
			else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
				windowResized = 1;
				shouldRender = 1;
			}
		}

		if (!shouldRender) continue;

		int w, h;
		SDL_GetWindowSize(window, &w, &h);
		if (w == 0 || h == 0) continue;

		#if WINDOW_RESIZABLE
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
		#endif
	  
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

		#if WINDOW_RESIZABLE
		if (windowResized) {
			constants.width = ChooseExtent(&Capabilities, window).width;
			constants.height = ChooseExtent(&Capabilities, window).height;

			windowResized = 0;
		}
		#endif

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
		vkQueuePresentKHR(presentQueue, &presentInfo);
	}

	SDL_DestroySurface(FONT_ATLAS);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
