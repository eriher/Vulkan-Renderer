#pragma once
#include "vulkan/vulkan.h"
#include "stb_image.h"
#include "Device.h"
#include <iostream>
#include "HdrTexture.h"
#include "Tools.h"
#include "Model.h"
#include "Pipeline.h"
class SkyboxTexture
{
	struct OffscreenUbo {
		glm::mat4 proj;
		glm::mat4 views[6];
	};

	VkFormat imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
	//VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
	//VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
	//VkFormat imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	//VkFormat imageFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
	int irradianceDim = 64;
	int reflectDim = 256;
	public:
		Device*								device;

		VkImage               skyboxImage;
		VkImageView           skyboxView;
		VkDeviceMemory        skyboxMemory;
		
		VkSampler             sampler;
		
		Pipeline pipeline;
		Model model;

		VkDescriptorSetLayout skyboxDescriptorSetLayout;
		//std::vector<VkDescriptorSet> descriptorSets;
		VkDescriptorSet skyboxDescriptorSet;

		int texWidth, texHeight, texChannels;

		void loadFromFiles(const std::string);
		void loadHdr(const std::string& , int);
		void createImage(VkImage &image, uint32_t width, uint32_t height, uint32_t mipLevels, VkDeviceMemory &memory);
		void createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkImageView*, uint32_t baseArray = 0, uint32_t baseMip = 0);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void copyBufferToImage(std::vector<VkBuffer>& buffers);
		void createTextureSampler();
		void createSkyboxDescriptors();
		void cleanup();
		void draw(VkCommandBuffer&, VkDescriptorSet&);
};

