#include "Model.h"


void Model::cleanup() {
	vkDestroyBuffer(device->device, indexBuffer, nullptr);
	vkFreeMemory(device->device, indexBufferMemory, nullptr);

	vkDestroyBuffer(device->device, vertexBuffer, nullptr);
	vkFreeMemory(device->device, vertexBufferMemory, nullptr);

	for(auto i = 0; i < descriptorBuffer.size(); i++){
		vkDestroyBuffer(device->device, descriptorBuffer[i], nullptr);
		vkFreeMemory(device->device, descriptorMemory[i], nullptr);
	}
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