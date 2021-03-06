#pragma once
#include "vulkan/vulkan.h"
#include "Device.h"
#include "Tools.h"
#include "stb_image.h"
class Texture {
	VkFormat imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
	//VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
	//VkFormat imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
public:
	bool valid = false;
	std::string filename;

	Device* device;
	VkImage               image;
	VkImageLayout         imageLayout;
	VkDeviceMemory        deviceMemory;
	VkImageView           view;
	uint32_t              width, height;
	uint32_t              mipLevels;
	uint32_t              layerCount;
	VkDescriptorImageInfo descriptor;
	VkSampler             sampler;
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;
	VkDescriptorSet descriptorSet;


	VkSampleCountFlagBits numSamples;
	VkFormat format;
	VkImageTiling tiling;
	VkImageUsageFlags usage;
	VkMemoryPropertyFlags properties;
	VkImageAspectFlags aspectFlags;

	void load(const std::string& filename);
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
	void generateMipmaps();
	void createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	void createTextureSampler();
	void cleanup();
	void createImage();
	void createImageView();
	void createDescriptors();
	void emptyTexture();
};