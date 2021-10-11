#pragma once
#include "vulkan/vulkan.h"
#include "Device.h"
#include "Model.h"
#include "Tools.h"
#include "SkyboxTexture.h"

class PBRMaps
{
	struct OffscreenUbo {
		glm::mat4 proj;
		glm::mat4 views[6];
	};
	//VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
	VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
	int irradianceDim = 64;
	int reflectDim = 256;
	void createImage(VkImage& image, uint32_t width, uint32_t height, uint32_t mipLevels, VkDeviceMemory& memory);
	void createImageView(VkImage image, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkImageView*, uint32_t baseArray = 0, uint32_t baseMip = 0);
public:
	Device* device;

	VkImage								irradianceImage;
	VkImageView						irradianceView;
	VkDeviceMemory				irradianceMemory;
	VkSampler             sampler;

	VkImage								reflectImage;
	VkImageView						reflectView;
	VkDeviceMemory				reflectMemory;
	VkSampler             reflectSampler;

	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;

	int texWidth, texHeight, texChannels;

	void loadFromFiles(const std::string);
	void createDescriptors();
	void cleanup();
	void generateMaps(SkyboxTexture*);
};

