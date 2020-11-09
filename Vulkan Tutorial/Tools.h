#pragma once
#include "vulkan/vulkan.h"
#include <vector>
#include <iostream>
#include <fstream>
#include "Device.h"
class Tools {
public:
  
  static glm::mat4 projection(glm::mat4 &&proj) {
    const glm::mat4 clip(
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, -1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.5f, 0.0f,
      0.0f, 0.0f, 0.5f, 1.0f);
    return clip * proj;
  }

  static glm::mat4 projection2(glm::mat4&& proj) {
    const glm::mat4 clip(
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.5f, 0.0f,
      0.0f, 0.0f, 0.5f, 1.0f);
    return clip * proj;
  }

  static std::vector<char> readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
      throw std::runtime_error("failed to open file: "+ filename);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
  }
  
  static VkShaderModule createShaderModule(VkDevice &device, const std::vector<char> &code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
      throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
  }

  static VkFormat findSupportedFormat(VkPhysicalDevice &device,const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
      VkFormatProperties props;
      vkGetPhysicalDeviceFormatProperties(device, format, &props);

      if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
        return format;
      }
      else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
        return format;
      }
    }

    throw std::runtime_error("failed to find supported format!");
  }

  static VkFormat findDepthFormat(VkPhysicalDevice& device) {
    return findSupportedFormat(device,
      { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
  }

  static void createImage(Device* device, VkImage& image, VkDeviceMemory& memory, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkSampleCountFlagBits samples, VkMemoryPropertyFlags properties, uint32_t mipLevels = 1, uint32_t layers = 1);
  static void createImageView(Device* device, VkImage& image, VkImageView& view, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t layerCount = 1, uint32_t baseLayer = 0, uint32_t mipLevels = 1, uint32_t baseMip = 0);
  void transitionImageLayout(Device* device, VkImage &image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = 1, uint32_t mipLevel = 0, uint32_t layers = 1, uint32_t baseLayer = 0);
  void copyBufferToImage(Device* device, std::vector<VkBuffer> &buffers);
};