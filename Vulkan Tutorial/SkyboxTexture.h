#pragma once
#include "vulkan/vulkan.h"
#include "stb_image.h"
#include "Device.h"
#include <iostream>
#include "HdrTexture.h"
#include "Tools.h"
#include "Model.h"
class SkyboxTexture
{
	struct OffscreenUbo {
		glm::mat4 proj;
		glm::mat4 views[6];
	};
	public:
		Device*								device;
		VkImage               image;
		VkImageView           view;
		VkImageLayout         imageLayout;
		VkDeviceMemory        deviceMemory;
		VkSampler             sampler;
		uint32_t mipLevels;
		int texWidth, texHeight, texChannels;
		void load(const std::string& );
		void loadHdr(const std::string& , int, Model*);
		void createImage();
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void copyBufferToImage(std::vector<VkBuffer>&);
		void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		void createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
		void createTextureSampler();
		void cleanup();
		void init(int);
};

