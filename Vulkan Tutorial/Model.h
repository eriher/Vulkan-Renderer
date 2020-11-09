#pragma once
#include "Device.h"
#include "Texture.h"
#include <vector>
#include "vulkan/vulkan.h"
#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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
	std::string name;
	MaterialProperties properties;
	Texture color_texture;
	Texture	reflectivity_texture;
	Texture	shininess_texture;
	Texture	metalness_texture;
	Texture	fresnel_texture;
	Texture	emission_texture;

	std::vector<VkBuffer> buffer;
	std::vector<VkDeviceMemory> memory;

	std::vector<VkDescriptorSet> descriptorSets;
};

struct Mesh
{
	std::string name;
	uint32_t material;
	uint32_t texture;
	// Where this Mesh's vertices start
	uint32_t start_index;
	uint32_t indices;
};

class Model {

public:
	Device* device;

	std::string name;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	std::vector<Mesh> meshes;

	std::vector<Texture> textures;
	std::vector<Material> materials;

	uint32_t swapChainSize;

	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;

	VkDescriptorSetLayout materialDescriptorSetLayout;

	std::vector<VkBuffer> descriptorBuffer;
	std::vector<VkDeviceMemory> descriptorMemory;

	glm::mat4 modelPos = glm::mat4(1.0f);
	float yaw = -90;
	float pitch = 0;
	bool positionChanged = false;

	uint32_t indices;

	void loadModel(std::string modelPath);
	void loadModel(std::string modelPath, std::string texturePath);
	void updateDescriptors();
	void updateDescriptor(uint32_t idx);
	void updateModelpos(int keyFlags, float delta);
	void createDescriptorSetLayout();
	void createDescriptorSets();
	void createDescriptorBuffers();
	void updateMaterial(Material &mat);
	void createMaterialDescriptorSetLayout();
	void createMaterialBuffers();
	void cleanup();
};
