#include "Tools.h"

void Tools::createImage(Device* device, VkImage& image, VkDeviceMemory& memory, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkSampleCountFlagBits samples, VkMemoryPropertyFlags properties, uint32_t mipLevels, uint32_t layers){
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = mipLevels;
  imageInfo.arrayLayers = layers;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.samples = samples;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateImage(device->device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device->device, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(device->device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }

  vkBindImageMemory(device->device, image, memory, 0);
}

void Tools::createImageView(Device* device, VkImage &image, VkImageView &view, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t layerCount, uint32_t baseLayer, uint32_t mipLevels, uint32_t baseMip) {
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.layerCount = layerCount;
  viewInfo.subresourceRange.baseArrayLayer = baseLayer;
  viewInfo.subresourceRange.levelCount = mipLevels;
  viewInfo.subresourceRange.baseMipLevel = baseMip;
  
  if (vkCreateImageView(device->device, &viewInfo, nullptr, &view) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image view!");
  }
}
