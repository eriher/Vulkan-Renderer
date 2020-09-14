#include "Model.h"


	void Model::cleanup() {
		vkDestroyBuffer(device->device, indexBuffer, nullptr);
		vkFreeMemory(device->device, indexBufferMemory, nullptr);

		vkDestroyBuffer(device->device, vertexBuffer, nullptr);
		vkFreeMemory(device->device, vertexBufferMemory, nullptr);
	}

	void Model::loadModel(std::string modelPath) {

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
			throw std::runtime_error(warn + err);
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


