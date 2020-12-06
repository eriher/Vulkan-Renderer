#pragma once
#include "vulkan/vulkan.h"
#include "Device.h"
#include "Model.h"
#include "Tools.h"

class BrdfCompute
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

	VkDescriptorSet descriptorSet;
	VkDescriptorSetLayout descriptorSetLayout;

	void gen();
	void cleanup();

};

