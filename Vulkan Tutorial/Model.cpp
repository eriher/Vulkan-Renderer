#include "Model.h"


void Model::cleanup() {

	for (auto texture : textures) {
		texture.cleanup();
		
		//vkDestroySampler(device->device, texture.sampler, nullptr);
		//vkDestroyImageView(device->device, texture.view, nullptr);

		//vkDestroyImage(device->device, texture.image, nullptr);
		//vkFreeMemory(device->device, texture.deviceMemory, nullptr);
	}

	vkDestroyBuffer(device->device, indexBuffer, nullptr);
	vkFreeMemory(device->device, indexBufferMemory, nullptr);

	vkDestroyBuffer(device->device, vertexBuffer, nullptr);
	vkFreeMemory(device->device, vertexBufferMemory, nullptr);

	for (auto i = 0; i < descriptorBuffer.size(); i++) {
		vkDestroyBuffer(device->device, descriptorBuffer[i], nullptr);
		vkFreeMemory(device->device, descriptorMemory[i], nullptr);
	}

	for (auto i = 0; i < materialBuffer.size(); i++) {
		vkDestroyBuffer(device->device, materialBuffer[i], nullptr);
		vkFreeMemory(device->device, materialMemory[i], nullptr);
	}

	//vkDestroyDescriptorSetLayout(device->device, descriptorSetLayout, nullptr);
};

void Model::loadModel(std::string modelPath, std::string texturePath) {
	Texture texture;
	texture.device = device;
	texture.load(texturePath);
	textures.push_back(texture);
	loadModel(modelPath);
}

void Model::loadModel(std::string modelPath) {
	name = modelPath.substr(modelPath.find_last_of('/')+1, modelPath.find_first_of('.') - (modelPath.find_last_of('/')+1));
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> mats;
	std::string warn, err;
	
	std::string matPath = modelPath.substr(0, modelPath.find_first_of('/'));

	if (!tinyobj::LoadObj(&attrib, &shapes, &mats, &warn, &err, modelPath.c_str(), matPath.c_str())) {
		throw std::runtime_error(warn + err);
	}

	for (const auto& m : mats) {
		Material material;
		material.m_name = m.name;
		material.properties.m_color = glm::vec4(m.diffuse[0], m.diffuse[1], m.diffuse[2], 1.0f);
		//if (m.diffuse_texname != "") {
		//	material.m_color_texture.load(directory + m.diffuse_texname, 4);
		//}

		material.properties.m_reflectivity = m.specular[0];
		//if (m.specular_texname != "") {
		//	material.m_reflectivity_texture.load(directory + m.specular_texname, 1);
		//}

		material.properties.m_metalness = m.metallic;
		//if (m.metallic_texname != "") {
		//	material.m_metalness_texture.load(directory + m.metallic_texname, 1);
		//}

		material.properties.m_fresnel = m.sheen;
		//if (m.sheen_texname != "") {
		//	material.m_fresnel_texture.load(directory + m.sheen_texname, 1);
		//}

		material.properties.m_shininess = m.roughness;
		//if (m.roughness_texname != "") {
		//	material.m_fresnel_texture.load(directory + m.sheen_texname, 1);
		//}

		material.properties.m_emission = m.emission[0];
		//if (m.emissive_texname != "") {
		//	material.m_emission_texture.load(directory + m.emissive_texname, 4);
		//}

		material.properties.m_transparency = m.transmittance[0];
		materials.push_back(material);
	}

	std::vector<glm::vec4> auto_normals(attrib.vertices.size() / 3);
	for (const auto& shape : shapes) {
		for (int face = 0; face < int(shape.mesh.indices.size()) / 3; face++) {

			glm::vec3 v0 = glm::vec3(
				attrib.vertices[shape.mesh.indices[face * 3 + 0].vertex_index * 3 + 0],
				attrib.vertices[shape.mesh.indices[face * 3 + 0].vertex_index * 3 + 1],
				attrib.vertices[shape.mesh.indices[face * 3 + 0].vertex_index * 3 + 2]
			);
			glm::vec3 v1 = glm::vec3(
				attrib.vertices[shape.mesh.indices[face * 3 + 1].vertex_index * 3 + 0],
				attrib.vertices[shape.mesh.indices[face * 3 + 1].vertex_index * 3 + 1],
				attrib.vertices[shape.mesh.indices[face * 3 + 1].vertex_index * 3 + 2]
			);
			glm::vec3 v2 = glm::vec3(
				attrib.vertices[shape.mesh.indices[face * 3 + 2].vertex_index * 3 + 0],
				attrib.vertices[shape.mesh.indices[face * 3 + 2].vertex_index * 3 + 1],
				attrib.vertices[shape.mesh.indices[face * 3 + 2].vertex_index * 3 + 2]
			);

			glm::vec3 e0 = glm::normalize(v1 - v0);
			glm::vec3 e1 = glm::normalize(v2 - v0);
			glm::vec3 face_normal = cross(e0, e1);

			auto_normals[shape.mesh.indices[face * 3 + 0].vertex_index] += glm::vec4(face_normal, 1.0f);
			auto_normals[shape.mesh.indices[face * 3 + 1].vertex_index] += glm::vec4(face_normal, 1.0f);
			auto_normals[shape.mesh.indices[face * 3 + 2].vertex_index] += glm::vec4(face_normal, 1.0f);
		}
	}
	for (auto& normal : auto_normals) {
		normal = (1.0f / normal.w) * normal;
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

			if (index.normal_index == -1) {
				// No normal, use the autogenerated
				vertex.normal = glm::vec3(auto_normals[index.vertex_index]);
			}
			else{
				vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
				};
			}

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

void Model::updateDescriptor(uint32_t idx) {
	  void* data;
	  vkMapMemory(device->device, descriptorMemory[idx], 0, sizeof(modelPos), 0, &data);
	  memcpy(data, &modelPos, sizeof(modelPos));
	  vkUnmapMemory(device->device, descriptorMemory[idx]);
}

void Model::updateDescriptors() {
	for (auto i = 0; i < swapChainSize; i++)
		updateDescriptor(i);
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
	allocInfo.descriptorPool = device->descriptorPool;
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
		updateDescriptor(i);
	}
}

void Model::createMaterialBuffers() {
	VkDeviceSize bufferSize = sizeof(MaterialProperties);
	materialBuffer.resize(swapChainSize);
	materialMemory.resize(swapChainSize);
	for (size_t i = 0; i < swapChainSize; i++) {
		device->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, materialBuffer[i], materialMemory[i]);
		updateMaterialBuffer(i);
	}
}

void Model::updateMaterialBuffer(uint32_t idx)
{
	void* data;
	vkMapMemory(device->device, materialMemory[idx], 0, sizeof(MaterialProperties), 0, &data);
	memcpy(data, &materials[0].properties, sizeof(MaterialProperties));
	vkUnmapMemory(device->device, materialMemory[idx]);
}
