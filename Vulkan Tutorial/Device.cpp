#include "Device.h"

void Device::createDescriptorPool() {

	VkDescriptorPoolSize pool_sizes[11] =
	{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 200 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 100 * 11;
	pool_info.poolSizeCount = 11;
	pool_info.pPoolSizes = pool_sizes;


	if (vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}

}

	void Device::cleanup() {
		vkDestroyCommandPool(device, graphicsCommandPool, nullptr);
		vkDestroyCommandPool(device, transientCommandPool, nullptr);
		vkDestroyCommandPool(device, resetCommandPool, nullptr);
		vkDestroyCommandPool(device, transferCommandPool, nullptr);
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		vkDestroyDevice(device, nullptr);
	}

	void Device::initDevice(VkPhysicalDevice pDevice, QueueFamilyIndices qfi, bool enableValidationLayers) {
			physicalDevice = pDevice;
			queueFamilies = qfi;
			GetPhysicalDeviceProperties();
			createLogicalDevice(enableValidationLayers);
			createCommandPools();
			createDescriptorPool();
	}
	//createBuffer

	void Device::GetPhysicalDeviceProperties() {
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
	}

	VkCommandBuffer Device::beginSingleTimeCommands() {
		return beginSingleTimeCommands(transientCommandPool);
	}

	VkCommandBuffer Device::beginSingleTimeCommands(VkCommandPool commandPool) {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void Device::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
		endSingleTimeCommands(transientCommandPool, graphicsQueue, commandBuffer);

	}

	void Device::endSingleTimeCommands(VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

		vkQueueWaitIdle(queue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	void Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	void Device::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		//VkCommandBuffer commandBuffer = beginSingleTimeCommands(transferCommandPool);
		VkCommandBuffer commandBuffer = beginSingleTimeCommands(transferCommandPool);

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		//endSingleTimeCommands(transferCommandPool, transferQueue, commandBuffer);
		endSingleTimeCommands(transferCommandPool, transferQueue, commandBuffer);
	}

/*	template<class T>
	void Device::createStagedBuffer(std::vector<T>& vertices, VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkFlags flags) {
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
	}*/

	void Device::createCommandPools() {
		//QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		//GraphicsPool
		createCommandPool(&graphicsCommandPool, 0, queueFamilies.graphicsFamily.value());
		//GraphicsPool
		createCommandPool(&transientCommandPool, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, queueFamilies.graphicsFamily.value());
		//GraphicsPool
		createCommandPool(&resetCommandPool, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, queueFamilies.graphicsFamily.value());
		//TransferPool
		createCommandPool(&transferCommandPool, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, queueFamilies.transferFamily.value());
		//IMGUIPool
		//createCommandPool(&imGuiCommandPool, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, queueFamilies.graphicsFamily.value());

		/*
		VkCommandPoolCreateInfo graphicsPoolInfo{};
		graphicsPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		graphicsPoolInfo.queueFamilyIndex = queueFamilies.graphicsFamily.value();

		if (vkCreateCommandPool(device, &graphicsPoolInfo, nullptr, &graphicsCommandPool) != VK_SUCCESS) {
				throw std::runtime_error("failed to create command pool!");
		}

		VkCommandPoolCreateInfo transferPoolInfo{};
		transferPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		transferPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		transferPoolInfo.queueFamilyIndex = queueFamilies.transferFamily.value();

		if (vkCreateCommandPool(device, &transferPoolInfo, nullptr, &transferCommandPool) != VK_SUCCESS) {
				throw std::runtime_error("failed to create command pool!");
		}*/

	}

	void Device::createCommandPool(VkCommandPool* commandPool, VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex) {
		VkCommandPoolCreateInfo commandPoolCreateInfo = {};
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
		commandPoolCreateInfo.flags = flags;

		if (vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, commandPool) != VK_SUCCESS) {
			throw std::runtime_error("Could not create graphics command pool");
		}
	}

	uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	void Device::createLogicalDevice(bool enableValidationLayers) {
		//QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { queueFamilies.graphicsFamily.value(), queueFamilies.presentFamily.value(), queueFamilies.transferFamily.value() };
		//std::set<uint32_t> uniqueQueueFamilies = { queueFamilies.graphicsFamily.value(), queueFamilies.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.sampleRateShading = VK_TRUE; // enable sample shading feature for the device

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(device, queueFamilies.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, queueFamilies.presentFamily.value(), 0, &presentQueue);
		vkGetDeviceQueue(device, queueFamilies.transferFamily.value(), 0, &transferQueue);
	}