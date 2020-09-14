#pragma once
#include "Device.h"
#include <vector>
#include "vulkan/vulkan.h"

#include "tiny_obj_loader.h"
#include <unordered_map>
#include <string>


class Model {

	

public:
	Device* device;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	std::vector<VkDescriptorSet> descriptorSets;

	glm::mat4 modelPos = glm::mat4(1.0f);
	float yaw = -90;
	float pitch = 0;
	bool positionChanged = false;

	uint32_t indices;
	void loadModel(std::string modelPath);
	void cleanup();


};
