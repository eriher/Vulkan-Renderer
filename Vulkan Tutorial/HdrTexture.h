#pragma once
#include "vulkan/vulkan.h"
#include "stb_image.h"
#include "Device.h"
#include <iostream>
class HdrTexture
{

	//VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
	//VkFormat imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	VkFormat imageFormat = VK_FORMAT_R32G32B32A32_SFLOAT;

	VkDeviceSize testSize;
	public:
		Device* device;
		VkImage               image;
		VkImageView           view;
		VkImageLayout         imageLayout;
		VkDeviceMemory        deviceMemory;
		VkSampler             sampler;
		uint32_t mipLevels;
		int texWidth, texHeight, texChannels;
		void load(const std::string& filename);
		void createImage();
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void copyBufferToImage(VkBuffer&);
		void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		void createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
		void createTextureSampler();
		void cleanup();
};
