#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

char* ReadFile(const char *filename, size_t *size) {
	FILE *file = fopen(filename, "rb");
	if (!file) return NULL;

	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *buffer = malloc(*size);
	if (buffer == NULL) return NULL;
	size_t bytesRead = fread(buffer, 1, *size, file);
	if (bytesRead != *size) {
		free(buffer);
		fclose(file);
		return NULL;
	} 
	fclose(file);
	return buffer;
}

VkShaderModule CreateShaderModule(VkDevice device, const char *code, size_t codeSize, VkResult *Result) {
	VkShaderModuleCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = codeSize,
		.pCode = (const uint32_t *)code,
	};

	VkShaderModule shaderModule;
	*Result = vkCreateShaderModule(device, &createInfo, NULL, &shaderModule);
	return shaderModule;
}

VkResult LoadShaderFromFile(VkDevice device, const char *ShaderFilename, VkShaderModule *shaderModule) {
	size_t shaderSize;
	char *shaderCode = ReadFile(ShaderFilename, &shaderSize);
	if (shaderCode == NULL) {
		free(shaderCode);
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	VkResult res;

	*shaderModule = CreateShaderModule(device, shaderCode, shaderSize, &res);
	if (res != VK_SUCCESS) {
		free(shaderCode);
		return res;
	}

	free(shaderCode);

	return VK_SUCCESS;
}

VkResult LoadShadersFromFiles(VkDevice device, const char *VertexShaderFilename, const char *FragmentShaderFilename, VkShaderModule *vertShaderModule, VkShaderModule *fragShaderModule) {
	size_t vertSize, fragSize;
	char *vertCode = ReadFile(VertexShaderFilename, &vertSize);
	if (vertCode == NULL) {
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	char *fragCode = ReadFile(FragmentShaderFilename, &fragSize);
	if (fragCode == NULL) {
		free(vertCode);
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	VkResult res1, res2;

	*vertShaderModule = CreateShaderModule(device, vertCode, vertSize, &res1);
	if (res1 != VK_SUCCESS) {
		free(vertCode);
		free(fragCode);
		return res1;
	}
	*fragShaderModule = CreateShaderModule(device, fragCode, fragSize, &res2);
	if (res2 != VK_SUCCESS) {
		free(vertCode);
		free(fragCode);
		return res2;
	}

	free(vertCode);
	free(fragCode);

	return VK_SUCCESS;
}
