#pragma once
#include "vulkan/vulkan.h"
#include <vector>
#include <fstream>
#include "Device.h"
//#include "Model.h"
class Pipeline
{

public:
  Device* device;
  VkExtent2D swapChainExtent;
  std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
  VkRenderPass renderPass;
  uint32_t pushConstantSize;
  VkSampleCountFlagBits msaaSamples;



	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout; 

  //std::vector<Model*> models;

	void createGraphicsPipeline(std::string, std::string);
  void createDescriptorSetLayout();
  void cleanup();
  static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
      throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
  }
  VkShaderModule createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
      throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
  }
};

