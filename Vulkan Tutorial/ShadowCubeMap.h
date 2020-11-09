#pragma once
#include "vulkan/vulkan.h"
#include "Device.h"
#include "Model.h"
#include "Tools.h"

class ShadowCubeMap
{
	struct OffscreenUbo {
		glm::mat4 proj;
		glm::mat4 views[6];
		glm::vec4 lightPos;
	};
	uint32_t dim = 2048;
	//VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
	VkFormat colorFormat = VK_FORMAT_R32_SFLOAT;
	//VkFormat colorFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
	float depthBiasConstant = 1.25f;
	float depthBiasSlope = 1.75f;
	void createImage(VkImage&, VkDeviceMemory&, VkFormat, VkImageUsageFlags);
	void createView(VkImage&, VkImageView&, VkFormat, VkComponentSwizzle, VkImageAspectFlags);
	void createRenderPass(VkRenderPass*);
	void createFramebuffer(VkFramebuffer*, VkRenderPass&);
	void createDescriptorSetLayout(VkDescriptorSetLayout*);
	void createGraphicsPipeline(VkPipelineLayout*, VkPipeline*, VkRenderPass&, std::vector<VkDescriptorSetLayout>&);
public:
	Device* device;
	VkBuffer buffer;
	VkDeviceMemory bufferMemory;
	
	VkImage colorImage;
	VkDeviceMemory colorMemory;
	VkImageView colorView;
	
	VkImage depthImage;
	VkDeviceMemory depthMemory;
	VkImageView depthView;
	
	VkSampler sampler;
	VkDescriptorSet descriptor;
	VkDescriptorSetLayout descriptorLayout;

	void createShadowMap(glm::vec4, std::vector<std::shared_ptr<Model>>);
	void createTextureSampler();
	void cleanup();

};

