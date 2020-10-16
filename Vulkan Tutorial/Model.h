#pragma once
#include "Device.h"
#include "Texture.h"
#include <vector>
#include "vulkan/vulkan.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include "tiny_obj_loader.h"
#include <unordered_map>
#include <string>

struct MaterialProperties {
	glm::vec4	m_color;
	float		m_reflectivity;
	float		m_shininess;
	float		m_metalness;
	float		m_fresnel;
	float		m_emission;
	float		m_transparency;
};

struct Material
{
	std::string m_name;
	MaterialProperties properties;
	Texture m_color_texture;
	Texture	m_reflectivity_texture;
	Texture	m_shininess_texture;
	Texture	m_metalness_texture;
	Texture	m_fresnel_texture;
	Texture	m_emission_texture;
};

struct Mesh
{
	std::string m_name;
	uint32_t m_material_idx;
	// Where this Mesh's vertices start
	uint32_t m_start_index;
	uint32_t m_number_of_vertices;
};

class Model {

public:
	Device* device;

	std::string name;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	std::vector<Texture> textures;
	std::vector<Material> materials;

	uint32_t swapChainSize;

	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<VkBuffer> descriptorBuffer;
	std::vector<VkDeviceMemory> descriptorMemory;

	std::vector<VkBuffer> materialBuffer;
	std::vector<VkDeviceMemory> materialMemory;

	glm::mat4 modelPos = glm::mat4(1.0f);
	float yaw = -90;
	float pitch = 0;
	bool positionChanged = false;

	uint32_t indices;

	void loadModel(std::string modelPath);
	void loadModel(std::string modelPath, std::string texturePath);
	void updateDescriptors(uint32_t idx);
	void updateModelpos(int keyFlags, float delta);
	void createDescriptorSetLayout();
	void createDescriptorSets();
	void createDescriptorBuffers();
	void updateMaterialBuffer(uint32_t idx);
	void createMaterialBuffers();
	void cleanup();
};
