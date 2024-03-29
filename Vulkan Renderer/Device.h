#pragma once

//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "vulkan/vulkan.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>
#include <optional>
#include <stdexcept>
#include <vector>
#include <set>
#include <array>

struct EmptyImage{
	VkImage image;
	VkDeviceMemory memory;
	VkImageView view;
	VkSampler sampler;
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> transferFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
	}
};

struct Vertex {
	glm::vec3 pos;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec3 tangent;
	//glm::vec3 bitangent;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, texCoord);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, normal);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, tangent);

		//attributeDescriptions[4].binding = 0;
		//attributeDescriptions[4].location = 4;
		//attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
		//attributeDescriptions[4].offset = offsetof(Vertex, bitangent);

		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const {
		return pos == other.pos && texCoord == other.texCoord && normal == other.normal && tangent == other.tangent/* && bitangent == other.bitangent*/;
	}

};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec2>()(vertex.texCoord) << 1)  >> 1) ^ hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^ (hash<glm::vec3>()(vertex.tangent) >> 1) << 1/*) ^ (hash<glm::vec3>()(vertex.bitangent) >> 1)) << 1*/;
			//return 
			//	((((hash<glm::vec3>()(vertex.pos) ^
			//	(hash<glm::vec2>()(vertex.texCoord) << 1)) >> 1) ^
			//	(hash<glm::vec3>()(vertex.normal) << 1) >> 1 ) ^
			//	(hash<glm::vec3>()(vertex.tangent) << 1) >> 1) ^
			//	(hash<glm::vec3>()(vertex.bitangent) << 1);
		}
	};
}


class Device {


	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_MULTIVIEW_EXTENSION_NAME
	};

public:
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkPhysicalDeviceProperties physicalDeviceProperties;

	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue transferQueue;

	QueueFamilyIndices queueFamilies;

	VkCommandPool graphicsCommandPool;
	VkCommandPool transientCommandPool;
	VkCommandPool resetCommandPool;
	VkCommandPool transferCommandPool;

	VkDescriptorPool descriptorPool;

	EmptyImage emptyImage;

	VkPipelineCache pipelineCache;

	void createDescriptorPool();

	void initDevice(VkPhysicalDevice pDevice, QueueFamilyIndices qfi, bool enableValidationLayers);
	//createBuffer

	void GetPhysicalDeviceProperties();
	void cleanup();
	VkCommandBuffer beginSingleTimeCommands(VkCommandPool commandPool);

	VkCommandBuffer beginSingleTimeCommands();

	void endSingleTimeCommands(VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer);

	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	template<class T>
	void createStagedBuffer(std::vector<T>& vertices, VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkFlags flags) {
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);

		copyBuffer(stagingBuffer, buffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void createCommandPools();

	void createCommandPool(VkCommandPool* commandPool, VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void createLogicalDevice(bool enableValidationLayers);

};