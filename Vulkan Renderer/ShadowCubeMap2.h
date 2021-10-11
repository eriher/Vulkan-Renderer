#pragma once
#include "vulkan/vulkan.h"
#include "Device.h"
#include "Model.h"
#include "Tools.h"

class ShadowCubeMap2
{
	glm::vec4* lightPos;
	std::vector<std::shared_ptr<Model>> models;
	struct OffscreenUbo {
		glm::mat4 proj;
		glm::mat4 views[6];
		glm::vec4 lightPos;
	};
	VkDeviceSize bufferSize = sizeof(OffscreenUbo);
	uint32_t dim = 2*1024;
	//VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
	//VkFormat colorFormat = VK_FORMAT_R32_SFLOAT;
	VkFormat colorFormat = VK_FORMAT_D16_UNORM;
	//VkFormat colorFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
	float depthBiasConstant = 1.25f;
	float depthBiasSlope = 1.75f;
	void createImage(VkImage&, VkDeviceMemory&, VkFormat, VkImageUsageFlags);
	void createView(VkImage&, VkImageView&, VkFormat, VkComponentSwizzle, VkImageAspectFlags);
	void createRenderPass(VkRenderPass*);
	void createFramebuffer(int, VkRenderPass&);
	void createGraphicsPipeline(VkPipelineLayout*, VkPipeline*, VkRenderPass&, std::vector<VkDescriptorSetLayout>&);
public:
	Device* device;

	VkBuffer buffer;
	VkDeviceMemory bufferMemory;
	
	std::vector<VkImage> colorImage;
	std::vector<VkDeviceMemory> colorMemory;
	std::vector<VkImageView> colorView;
	
	VkImage depthImage;
	VkDeviceMemory depthMemory;
	VkImageView depthView;
	
	VkSampler sampler;

	std::vector<VkDescriptorSet> shadowDescriptorSet;
	VkDescriptorSetLayout shadowDescriptorSetLayout;

	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;

	VkRenderPass renderPass;
	std::vector<VkFramebuffer> framebuffer;

	VkDescriptorSet renderDescriptorSet;
	VkDescriptorSetLayout renderDescriptorSetLayout;

	void draw(VkCommandBuffer&, int);
	void setupShadowMap(glm::vec4*, std::vector<std::shared_ptr<Model>>&, int);
	void cleanup();
	void updateRenderDescriptorSets();
};

