#include "Model.h"


void Model::cleanup() {

	vkDestroyBuffer(device->device, indexBuffer, nullptr);
	vkFreeMemory(device->device, indexBufferMemory, nullptr);

	vkDestroyBuffer(device->device, vertexBuffer, nullptr);
	vkFreeMemory(device->device, vertexBufferMemory, nullptr);

	for (auto i = 0; i < descriptorBuffer.size(); i++) {
		vkDestroyBuffer(device->device, descriptorBuffer[i], nullptr);
		vkFreeMemory(device->device, descriptorMemory[i], nullptr);
	}

	for (auto i = 0; i < materials.size(); i++) {
		if(materials[i].descriptorSets.size() > 0)
			vkFreeDescriptorSets(device->device, device->descriptorPool, materials[i].descriptorSets.size(), materials[i].descriptorSets.data());
		for(int j = 0; j < swapChainSize; j++){
			if(materials[i].buffer.size() > 0){
				vkDestroyBuffer(device->device, materials[i].buffer[j], nullptr);
				vkFreeMemory(device->device, materials[i].memory[j], nullptr);
			}
		}
		if (materials[i].colorTexture.valid)
		{
			materials[i].colorTexture.cleanup();
		}
		if (materials[i].metalnessTexture.valid)
		{
			materials[i].metalnessTexture.cleanup();
		}
		if (materials[i].shininessTexture.valid)
		{
			materials[i].shininessTexture.cleanup();
		}
		if (materials[i].normalTexture.valid)
		{
			materials[i].normalTexture.cleanup();
		}
		if (materials[i].aoTexture.valid)
		{
			materials[i].aoTexture.cleanup();
		}
		if (materials[i].emissionTexture.valid)
		{
			materials[i].emissionTexture.cleanup();
		}
		if (materials[i].depthTexture.valid)
		{
			materials[i].depthTexture.cleanup();
		}
	}

	//vkDestroyDescriptorSetLayout(device->device, descriptorSetLayout, nullptr);
};

void Model::loadModel(std::string modelPath) {
	name = modelPath.substr(modelPath.find_last_of('/')+1, modelPath.find_first_of('.') - (modelPath.find_last_of('/')+1));
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> mats;
	std::string warn, err;
	
	std::string matPath = modelPath.substr(0, modelPath.find_last_of('/'));

	if (!tinyobj::LoadObj(&attrib, &shapes, &mats, &warn, &err, modelPath.c_str(), matPath.c_str())) {
		throw std::runtime_error(warn + err);
	}

	if (mats.size() == 0) {
		Material material{};
		material.name = "empty";
		materials.push_back(material);
	}

	for (const auto& m : mats) {
		Material material{};
		material.name = m.name;
		material.properties.m_color = glm::vec4(m.diffuse[0], m.diffuse[1], m.diffuse[2], 1.0f);
		if (m.diffuse_texname != "") {
			material.colorTexture.device = device;
			material.colorTexture.load(matPath + "/" + m.diffuse_texname);
		}

		material.properties.m_reflectivity = m.specular[0];
		if (m.specular_texname != "") {
			material.reflectivityTexture.device = device;
			material.reflectivityTexture.load(matPath + "/" + m.specular_texname);
		}

		material.properties.m_metalness = m.metallic;
		if (m.metallic_texname != "") {
			material.metalnessTexture.device = device;
			material.metalnessTexture.load(matPath + "/" + m.metallic_texname);
		}

		material.properties.m_fresnel = m.sheen;
		//if (m.sheen_texname != "") {
		//	material.m_fresnel_texture.load(directory + m.sheen_texname, 1);
		//}

		material.properties.m_shininess = m.roughness;
		if (m.roughness_texname != "") {
			material.shininessTexture.device = device;
			material.shininessTexture.load(matPath + "/" + m.metallic_texname);
		}

		material.properties.m_emission = m.emission[0];
		if (m.emissive_texname != "") {
			material.emissionTexture.device = device;
			material.emissionTexture.load(matPath + "/" + m.emissive_texname);
		}

		if (m.normal_texname != "") {
			material.normalTexture.device = device;
			material.normalTexture.load(matPath + "/" + m.normal_texname);
		}


		if (m.ambient_texname != "") {
			material.aoTexture.device = device;
			material.aoTexture.load(matPath + "/" + m.ambient_texname);
		}

		if (m.displacement_texname != "") {
			material.depthTexture.device = device;
			material.depthTexture.load(matPath + "/" + m.displacement_texname);
		}

		material.properties.m_transparency = m.transmittance[0];

		materials.push_back(material);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};
	std::vector<Vertex> modelVertices{};
	std::vector<uint32_t> modelIndices{};

	for (const auto& shape : shapes) {
		//std::vector<glm::vec4> gen_normals(attrib.vertices.size());
		//std::vector<glm::vec4> tangents(attrib.vertices.size());
		//std::vector<glm::vec4> bitangents(attrib.vertices.size());
		std::vector<glm::vec3> tangents;
		//std::vector<glm::vec3> bitangents;
		std::vector<glm::vec3> gen_normals;
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
			gen_normals.push_back(face_normal);
			//gen_normals[shape.mesh.indices[face * 3 + 0].vertex_index] += glm::vec4(face_normal, 1.0f);
			//gen_normals[shape.mesh.indices[face * 3 + 1].vertex_index] += glm::vec4(face_normal, 1.0f);
			//gen_normals[shape.mesh.indices[face * 3 + 2].vertex_index] += glm::vec4(face_normal, 1.0f);

			glm::vec2 uv0 = glm::vec2(attrib.texcoords[shape.mesh.indices[face * 3 + 0].texcoord_index * 2 + 0], 1.0f - attrib.texcoords[shape.mesh.indices[face * 3 + 0].texcoord_index * 2 + 1]);
			glm::vec2 uv1 = glm::vec2(attrib.texcoords[shape.mesh.indices[face * 3 + 1].texcoord_index * 2 + 0], 1.0f - attrib.texcoords[shape.mesh.indices[face * 3 + 1].texcoord_index * 2 + 1]);
			glm::vec2 uv2 = glm::vec2(attrib.texcoords[shape.mesh.indices[face * 3 + 2].texcoord_index * 2 + 0], 1.0f - attrib.texcoords[shape.mesh.indices[face * 3 + 2].texcoord_index * 2 + 1]);


			e0 = v1 - v0;
			e1 = v2 - v0;
			glm::vec2 deltaUV1 = uv1 - uv0;
			glm::vec2 deltaUV2 = uv2 - uv0;

			glm::vec3 tangent;
			glm::vec3 bitangent;

			float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

			tangent.x = f * (deltaUV2.y * e0.x - deltaUV1.y * e1.x);
			tangent.y = f * (deltaUV2.y * e0.y - deltaUV1.y * e1.y);
			tangent.z = f * (deltaUV2.y * e0.z - deltaUV1.y * e1.z);
			tangents.push_back(tangent);
			//bitangents.push_back(cross(face_normal, tangent));
			//bitangent.x = f * (-deltaUV2.x * e0.x + deltaUV1.x * e1.x);
			//bitangent.y = f * (-deltaUV2.x * e0.y + deltaUV1.x * e1.y);
			//bitangent.z = f * (-deltaUV2.x * e0.z + deltaUV1.x * e1.z);
			//bitangents.push_back(bitangent);
			
			//tangents[shape.mesh.indices[face * 3 + 0].vertex_index] += glm::vec4(tangent, 1.0f);
			//tangents[shape.mesh.indices[face * 3 + 1].vertex_index] += glm::vec4(tangent, 1.0f);
			//tangents[shape.mesh.indices[face * 3 + 2].vertex_index] += glm::vec4(tangent, 1.0f);

			
			//bitangents[shape.mesh.indices[face * 3 + 0].vertex_index] += glm::vec4(bitangent, 1.0f);
			//bitangents[shape.mesh.indices[face * 3 + 1].vertex_index] += glm::vec4(bitangent, 1.0f);
			//bitangents[shape.mesh.indices[face * 3 + 2].vertex_index] += glm::vec4(bitangent, 1.0f);

		}
	//}
	
	/*for (auto& normal : auto_normals) {
		normal = (1.0f / normal.w) * normal;
	}*/
	
	/*for (int i = 0; i < gen_normals.size(); i++) {
		gen_normals[i] = glm::normalize((1.0f / gen_normals[i].w) * gen_normals[i]);
		tangents[i] = glm::normalize((1.0f / tangents[i].w) * tangents[i]);
		bitangents[i] = glm::normalize((1.0f / bitangents[i].w) * bitangents[i]);
	}*/


	//for (const auto& shape : shapes) {
		Mesh mesh{};
		mesh.texture = 0;
		mesh.start_index = indices;
		mesh.material = std::max(shape.mesh.material_ids[0], 0);
		mesh.name = shape.name;
		int fetchIndx = 0;
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

			vertex.tangent = tangents[fetchIndx / 3];
			//vertex.bitangent = bitangents[fetchIndx / 3];
			//auto test = glm::cross(vertex.tangent, vertex.bitangent);
			
			if (index.normal_index == -1) {
				// No normal, use the autogenerated
				//vertex.normal = glm::vec3(gen_normals[index.vertex_index]);
				//vertex.normal = glm::cross(vertex.tangent, vertex.bitangent);
				vertex.normal = gen_normals[fetchIndx / 3];
			}
			else{
				vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
				};
			}
			//auto test = glm::cross(vertex.normal, vertex.tangent);
			//vertex.tangent = glm::vec3(tangents[index.vertex_index]);
			//vertex.bitangent = glm::vec3(bitangents[index.vertex_index]);



			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(modelVertices.size());
				modelVertices.push_back(vertex);
			}

			modelIndices.push_back(uniqueVertices[vertex]);
			indices++;
			fetchIndx++;
		}
		mesh.indices = indices - mesh.start_index;
		meshes.push_back(mesh);
		
	}
	//indices = static_cast<uint32_t>(modelIndices.size());

	device->createStagedBuffer(modelVertices, vertexBuffer, vertexBufferMemory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	device->createStagedBuffer(modelIndices, indexBuffer, indexBufferMemory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

}

void Model::draw(VkCommandBuffer &cb, VkPipelineLayout &pipelineLayout, int i)
{
	VkBuffer vertexBuffers[] = { vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(cb, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(cb, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &descriptorSets[i], 0, nullptr);

	for (auto& mesh : meshes) {
		vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 3, 1, &materials[mesh.material].descriptorSets[i], 0, nullptr);
		vkCmdDrawIndexed(cb, mesh.indices, 1, mesh.start_index, 0, 0);
	}
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

	if ((keyFlags & 1)  && !(keyFlags & 2))
		modelPos = glm::translate(modelPos, glm::vec3(0.0f, 0.0f, 2.0f * speed));
	if (keyFlags & 2 && !(keyFlags & 1))
		modelPos = glm::translate(modelPos, glm::vec3(0.0f, 0.0f, 2.0f * -speed));
	if (keyFlags & 4 && !(keyFlags & 8))
		modelPos = glm::rotate(modelPos, glm::radians(90.0f * delta), glm::vec3(0, 1, 0));
	if (keyFlags & 8 && !(keyFlags & 4))
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

		std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = descriptorBuffer[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(glm::mat4);

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;


		vkUpdateDescriptorSets(device->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
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

	std::vector<VkDescriptorSetLayout> layouts(swapChainSize, materialDescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = device->descriptorPool;
	allocInfo.descriptorSetCount = swapChainSize;
	allocInfo.pSetLayouts = layouts.data();

	VkDeviceSize bufferSize = sizeof(MaterialProperties);
	for (auto& mat : materials) {

		mat.buffer.resize(swapChainSize);
		mat.memory.resize(swapChainSize);
		
		for (size_t i = 0; i < swapChainSize; i++) {
			device->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mat.buffer[i], mat.memory[i]);
		}
		
		updateMaterial(mat);
		
		mat.descriptorSets.resize(swapChainSize);
		
		if (vkAllocateDescriptorSets(device->device, &allocInfo, mat.descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < swapChainSize; i++) {

			std::vector<VkWriteDescriptorSet> descriptorWrites;
			{
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = mat.buffer[i];
				bufferInfo.offset = 0;
				bufferInfo.range = bufferSize;
				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = mat.descriptorSets[i];
				descriptorWrite.dstBinding = 0;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo = &bufferInfo;
				descriptorWrites.push_back(descriptorWrite);
			}

			{
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				if (mat.colorTexture.valid) {
					imageInfo.imageView = mat.colorTexture.view;
					imageInfo.sampler = mat.colorTexture.sampler;
				}
				else {
					imageInfo.imageView = device->emptyImage.view;
					imageInfo.sampler = device->emptyImage.sampler;
				}
				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = mat.descriptorSets[i];
				descriptorWrite.dstBinding = 1;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pImageInfo = &imageInfo;
				descriptorWrites.push_back(descriptorWrite);
			}

			{
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				if (mat.metalnessTexture.valid) {
					imageInfo.imageView = mat.metalnessTexture.view;
					imageInfo.sampler = mat.metalnessTexture.sampler;
				}
				else {
					imageInfo.imageView = device->emptyImage.view;
					imageInfo.sampler = device->emptyImage.sampler;
				}
				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = mat.descriptorSets[i];
				descriptorWrite.dstBinding = 2;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pImageInfo = &imageInfo;
				descriptorWrites.push_back(descriptorWrite);
			}

			{
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				if (mat.shininessTexture.valid) {
					imageInfo.imageView = mat.shininessTexture.view;
					imageInfo.sampler = mat.shininessTexture.sampler;
				}
				else {
					imageInfo.imageView = device->emptyImage.view;
					imageInfo.sampler = device->emptyImage.sampler;
				}
				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = mat.descriptorSets[i];
				descriptorWrite.dstBinding = 3;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pImageInfo = &imageInfo;
				descriptorWrites.push_back(descriptorWrite);
			}

			{
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				if (mat.normalTexture.valid) {
					imageInfo.imageView = mat.normalTexture.view;
					imageInfo.sampler = mat.normalTexture.sampler;
				}
				else {
					imageInfo.imageView = device->emptyImage.view;
					imageInfo.sampler = device->emptyImage.sampler;
				}
				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = mat.descriptorSets[i];
				descriptorWrite.dstBinding = 4;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pImageInfo = &imageInfo;
				descriptorWrites.push_back(descriptorWrite);
			}

			{
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				if (mat.aoTexture.valid) {
					imageInfo.imageView = mat.aoTexture.view;
					imageInfo.sampler = mat.aoTexture.sampler;
				}
				else {
					imageInfo.imageView = device->emptyImage.view;
					imageInfo.sampler = device->emptyImage.sampler;
				}
				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = mat.descriptorSets[i];
				descriptorWrite.dstBinding = 5;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pImageInfo = &imageInfo;
				descriptorWrites.push_back(descriptorWrite);
			}

			{
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				if (mat.emissionTexture.valid) {
					imageInfo.imageView = mat.emissionTexture.view;
					imageInfo.sampler = mat.emissionTexture.sampler;
				}
				else {
					imageInfo.imageView = device->emptyImage.view;
					imageInfo.sampler = device->emptyImage.sampler;
				}
				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = mat.descriptorSets[i];
				descriptorWrite.dstBinding = 6;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pImageInfo = &imageInfo;
				descriptorWrites.push_back(descriptorWrite);
			}

			{
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				if (mat.depthTexture.valid) {
					imageInfo.imageView = mat.depthTexture.view;
					imageInfo.sampler = mat.depthTexture.sampler;
				}
				else {
					imageInfo.imageView = device->emptyImage.view;
					imageInfo.sampler = device->emptyImage.sampler;
				}
				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = mat.descriptorSets[i];
				descriptorWrite.dstBinding = 7;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pImageInfo = &imageInfo;
				descriptorWrites.push_back(descriptorWrite);
			}

			vkUpdateDescriptorSets(device->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}

		
	}

	//vkDestroyDescriptorSetLayout(device->device, tempDescriptorSetLayout, nullptr);




}

void Model::updateMaterial(Material &mat)
{
	for(auto i = 0; i < swapChainSize; i++){
		void* data;
		vkMapMemory(device->device, mat.memory[i], 0, sizeof(MaterialProperties), 0, &data);
		memcpy(data, &mat.properties, sizeof(MaterialProperties));
		vkUnmapMemory(device->device, mat.memory[i]);
	}
}