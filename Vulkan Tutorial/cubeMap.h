#pragma once
#include "vulkan/vulkan.h"

#include "HdrTexture.h"
#include "Device.h"
#include "Model.h"
class cubeMap
{
public:
	Device* device;
	VkFramebuffer convolutionFramebuffer;
	VkRenderPass convolutionRenderPass;
	VkImageView target;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	VkPipeline convolutionPipeline;
	VkPipelineLayout convolutionPipelineLayout;
	//void generateCubeMap();
};

