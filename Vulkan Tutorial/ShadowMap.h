#pragma once
#include "vulkan/vulkan.h"
#include "Device.h"
#include "Model.h"
#include "Tools.h"

class ShadowMap
{
	uint32_t dim = 2048;
	VkFormat DEPTH_FORMAT = VK_FORMAT_D16_UNORM;
	float depthBiasConstant = 1.25f;
	float depthBiasSlope = 1.75f;
public:
	Device* device;
	VkBuffer buffer;
	VkDeviceMemory bufferMemory;
	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView view;
	VkSampler sampler;

	VkDescriptorSet descriptor;
	VkDescriptorSetLayout descriptorLayout;

	void createShadowMap(glm::vec4, std::vector<std::shared_ptr<Model>>);
	void createTextureSampler();
	void cleanup();

};

