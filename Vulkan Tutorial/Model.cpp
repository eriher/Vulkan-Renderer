#include "Model.h"


void Model::cleanup() {

	for (auto texture : textures) {
		vkDestroySampler(device->device, texture.sampler, nullptr);
		vkDestroyImageView(device->device, texture.view, nullptr);

		vkDestroyImage(device->device, texture.image, nullptr);
		vkFreeMemory(device->device, texture.deviceMemory, nullptr);
	}

	vkDestroyBuffer(device->device, indexBuffer, nullptr);
	vkFreeMemory(device->device, indexBufferMemory, nullptr);

	vkDestroyBuffer(device->device, vertexBuffer, nullptr);
	vkFreeMemory(device->device, vertexBufferMemory, nullptr);

	for (auto i = 0; i < descriptorBuffer.size(); i++) {
		vkDestroyBuffer(device->device, descriptorBuffer[i], nullptr);
		vkFreeMemory(device->device, descriptorMemory[i], nullptr);
	}
	vkDestroyDescriptorSetLayout(device->device, descriptorSetLayout, nullptr);
};

void Model::loadModel(std::string modelPath, std::string texturePath) {
	Texture texture;
	texture.device = device;
	texture.load(texturePath);
	textures.push_back(texture);
	loadModel(modelPath);
}

void Model::loadModel(std::string modelPath) {

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;
	
	std::string matPath = modelPath.substr(0, modelPath.find_first_of('/'));

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str(), matPath.c_str())) {
		throw std::runtime_error(warn + err);
	}

	for (const auto& mat : materials) {

	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};
	std::vector<Vertex> modelVertices{};
	std::vector<uint32_t> modelIndices{};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};

			vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(modelVertices.size());
				modelVertices.push_back(vertex);
			}

			modelIndices.push_back(uniqueVertices[vertex]);
		}
	}
	indices = static_cast<uint32_t>(modelIndices.size());

	device->createStagedBuffer(modelVertices, vertexBuffer, vertexBufferMemory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	device->createStagedBuffer(modelIndices, indexBuffer, indexBufferMemory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);


}

void Model::updateDescriptors(uint32_t idx) {
	  void* data;
	  vkMapMemory(device->device, descriptorMemory[idx], 0, sizeof(modelPos), 0, &data);
	  memcpy(data, &modelPos, sizeof(modelPos));
	  vkUnmapMemory(device->device, descriptorMemory[idx]);
}

void Model::updateModelpos(int keyFlags, float delta) {
	float speed = 5.0f * delta;

	if (keyFlags & 16 && !(keyFlags & 32))
		modelPos = glm::translate(modelPos, glm::vec3(0.0f, 0.0f, 2.0f * speed));
	if (keyFlags & 32 && !(keyFlags & 16))
		modelPos = glm::translate(modelPos, glm::vec3(0.0f, 0.0f, 2.0f * -speed));


	if (keyFlags & 64 && !(keyFlags & 128))
		modelPos = glm::rotate(modelPos, glm::radians(90.0f * delta), glm::vec3(0, 1, 0));
	if (keyFlags & 128 && !(keyFlags & 64))
		modelPos = glm::rotate(modelPos, glm::radians(90.0f * delta), glm::vec3(0, -1, 0));

}

void Model::createDescriptorSets() {
	std::vector<VkDescriptorSetLayout> layouts(swapChainSize, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainSize);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(swapChainSize);
	if (vkAllocateDescriptorSets(device->device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < swapChainSize; i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = descriptorBuffer[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(glm::mat4);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textures[0].view;
		imageInfo.sampler = textures[0].sampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void Model::createDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device->device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

void Model::createDescriptorBuffers() {
	VkDeviceSize bufferSize = sizeof(glm::mat4);
	descriptorBuffer.resize(swapChainSize);
	descriptorMemory.resize(swapChainSize);
	for (size_t i = 0; i < swapChainSize; i++) {
		device->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, descriptorBuffer[i], descriptorMemory[i]);
		updateDescriptors(i);
	}
}