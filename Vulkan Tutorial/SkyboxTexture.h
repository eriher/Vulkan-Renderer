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

	//VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
	VkFormat imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

	public:
		Device*								device;

		VkImage               skyboxImage;
		VkImageView           skyboxView;
		VkDeviceMemory        skyboxMemory;
		
		
		VkImage								irradianceImage;
		VkImageView						irradianceView;
		VkDeviceMemory				irradianceMemory;
		
		VkSampler             sampler;

		int texWidth, texHeight, texChannels;

		void loadHdr(const std::string& , int, Model*);
		void createImage();
		void createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkImageView*);
		void createTextureSampler();
		void cleanup();
};

