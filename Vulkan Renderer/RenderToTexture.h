#pragma once
#include "vulkan/vulkan.h"
#include "Device.h"
#include "Model.h"
#include "Tools.h"

class RenderToTexture
{
	uint32_t dim = 512;
	//VkFormat FORMAT = VK_FORMAT_R8G8B8A8_UNORM;
	VkFormat FORMAT = VK_FORMAT_R16G16_SFLOAT;
	//VkFormat FORMAT = VK_FORMAT_R32G32_SFLOAT;

public:
	Device* device;

	VkBuffer buffer;
	VkDeviceMemory bufferMemory;

	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;
	VkSampler imageSampler;

	VkDescriptorSet descriptor;
	VkDescriptorSetLayout descriptorLayout;

	void createShadowMap(glm::vec4, std::vector<std::shared_ptr<Model>>);
	void createTextureSampler();
	void cleanup();

};

