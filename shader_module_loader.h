#ifndef SHADER_MODULE_LOADER_H
#define SHADER_MODULE_LOADER_H

#include <vulkan/vulkan.h>

char* ReadFile(const char *filename, size_t *size);
VkShaderModule CreateShaderModule(VkDevice device, const char *code, size_t codeSize, VkResult *Result);
VkResult LoadShaderFromFile(VkDevice device, const char *ShaderFilename, VkShaderModule *shaderModule);
VkResult LoadShadersFromFiles(VkDevice device, const char *VertexShaderFilename, const char *FragmentShaderFilename, VkShaderModule *vertShaderModule, VkShaderModule *fragShaderModule);

#endif // SHADER_MODULE_LOADER_H
