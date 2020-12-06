#include "PBRMaps.h"

void PBRMaps::generateMaps(SkyboxTexture* skybox) {
  {
    //Create renderpass
    VkRenderPass renderPass;
    {
      VkAttachmentDescription colorAttachment{};
      colorAttachment.format = imageFormat;
      colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
      colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

      VkAttachmentReference colorAttachmentRef{};
      colorAttachmentRef.attachment = 0;
      colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

      VkSubpassDescription subpass{};
      subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
      subpass.colorAttachmentCount = 1;
      subpass.pColorAttachments = &colorAttachmentRef;

      //VkSubpassDependency dependency{};
      //dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
      //dependency.dstSubpass = 0;
      //dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      //dependency.srcAccessMask = 0;
      //dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      //dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

      // Use subpass dependencies for layout transitions
      std::array<VkSubpassDependency, 2> dependencies;
      dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
      dependencies[0].dstSubpass = 0;
      dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
      dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
      dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
      dependencies[1].srcSubpass = 0;
      dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
      dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
      dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
      dependencies[1].dstAccessMask = 0;
      dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

      std::array<VkAttachmentDescription, 1> attachments = { colorAttachment };// , colorAttachmentResolve};

      VkRenderPassCreateInfo renderPassInfo{};
      renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
      renderPassInfo.pAttachments = attachments.data();
      renderPassInfo.subpassCount = 1;
      renderPassInfo.pSubpasses = &subpass;
      renderPassInfo.dependencyCount = 0;
      //renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
      //renderPassInfo.pDependencies = dependencies.data();

      const uint32_t viewMask = 0b00111111;
      const uint32_t correlationMask = 0b00111111;

      VkRenderPassMultiviewCreateInfo renderPassMultiviewCI{};
      renderPassMultiviewCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
      renderPassMultiviewCI.subpassCount = 1;
      renderPassMultiviewCI.pViewMasks = &viewMask;
      renderPassMultiviewCI.correlationMaskCount = 1;
      renderPassMultiviewCI.pCorrelationMasks = &correlationMask;

      renderPassInfo.pNext = &renderPassMultiviewCI;

      if (vkCreateRenderPass(device->device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
      }
    }

    {
      VkDescriptorSetLayoutBinding vertLayoutBinding{};
      vertLayoutBinding.binding = 0;
      vertLayoutBinding.descriptorCount = 1;
      vertLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      vertLayoutBinding.pImmutableSamplers = nullptr;
      vertLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

      VkDescriptorSetLayoutBinding fragLayoutBinding{};
      fragLayoutBinding.binding = 1;
      fragLayoutBinding.descriptorCount = 1;
      fragLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      fragLayoutBinding.pImmutableSamplers = nullptr;
      fragLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      std::array<VkDescriptorSetLayoutBinding, 2> bindings = { vertLayoutBinding, fragLayoutBinding };
      VkDescriptorSetLayoutCreateInfo layoutInfo{};
      layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
      layoutInfo.pBindings = bindings.data();

      if (vkCreateDescriptorSetLayout(device->device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
      }
    }

    VkDeviceSize bufferSize = sizeof(OffscreenUbo);

    glm::mat4 proj = Tools::projection(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1.0f));
    OffscreenUbo ubo = { proj,  {
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f),  glm::vec3(0.0f, 1.0f, 0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f),  glm::vec3(0.0f, 1.0f, 0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec3(0.0f, 0.0f, 1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  -1.0f,  0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f),  glm::vec3(0.0f, 1.0f, 0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, 1.0f),  glm::vec3(0.0f, 1.0f, 0.0f))
      } };

    VkBuffer buffer;
    VkDeviceMemory memory;

    device->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, memory);
    void* data;
    vkMapMemory(device->device, memory, 0, sizeof(OffscreenUbo), 0, &data);
    memcpy(data, &ubo, sizeof(OffscreenUbo));
    vkUnmapMemory(device->device, memory);

    {
      std::vector<VkDescriptorSetLayout> layouts(1, descriptorSetLayout);
      VkDescriptorSetAllocateInfo allocInfo{};
      allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      allocInfo.descriptorPool = device->descriptorPool;
      allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
      allocInfo.pSetLayouts = layouts.data();

      if (vkAllocateDescriptorSets(device->device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
      }

      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = buffer;
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(OffscreenUbo);

      VkDescriptorImageInfo imageInfo{};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = skybox->skyboxView;
      imageInfo.sampler = skybox->sampler;

      std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

      descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[0].dstSet = descriptorSet;
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pBufferInfo = &bufferInfo;

      descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[1].dstSet = descriptorSet;
      descriptorWrites[1].dstBinding = 1;
      descriptorWrites[1].dstArrayElement = 0;
      descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptorWrites[1].descriptorCount = 1;
      descriptorWrites[1].pImageInfo = &imageInfo;

      vkUpdateDescriptorSets(device->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

    }

    //Generate IrradianceMap

    //Create images and views
    createImage(irradianceImage, irradianceDim, irradianceDim, 1, irradianceMemory);
    createImageView(irradianceImage, VK_IMAGE_ASPECT_COLOR_BIT, 1, &irradianceView);

    //Create Framebuffer
    VkFramebuffer framebuffer;
    {
      VkFramebufferCreateInfo framebufferInfo{};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = renderPass;
      framebufferInfo.attachmentCount = 1;
      framebufferInfo.pAttachments = &irradianceView;
      framebufferInfo.width = irradianceDim;
      framebufferInfo.height = irradianceDim;
      framebufferInfo.layers = 1;

      if (vkCreateFramebuffer(device->device, &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
      }
    }

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    {
      auto vertShaderCode = Tools::readFile("compiledshaders/cubemap.vert.spv");
      auto fragShaderCode = Tools::readFile("compiledshaders/irradiancemap.frag.spv");

      VkShaderModule vertShaderModule = Tools::createShaderModule(device->device, vertShaderCode);
      VkShaderModule fragShaderModule = Tools::createShaderModule(device->device, fragShaderCode);

      VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
      vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
      vertShaderStageInfo.module = vertShaderModule;
      vertShaderStageInfo.pName = "main";

      VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
      fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      fragShaderStageInfo.module = fragShaderModule;
      fragShaderStageInfo.pName = "main";

      VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

      VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
      vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

      auto bindingDescription = Vertex::getBindingDescription();
      auto attributeDescriptions = Vertex::getAttributeDescriptions();

      vertexInputInfo.vertexBindingDescriptionCount = 1;
      vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
      vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
      vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

      VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
      inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      inputAssembly.primitiveRestartEnable = VK_FALSE;

      VkViewport viewport{};
      viewport.x = 0.0f;
      viewport.y = 0.0f;
      viewport.width = (float)irradianceDim;
      viewport.height = (float)irradianceDim;
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;

      VkRect2D scissor{};
      scissor.offset.x = 0;
      scissor.offset.y = 0;
      scissor.extent.width = (float)irradianceDim;
      scissor.extent.height = (float)irradianceDim;

      VkPipelineViewportStateCreateInfo viewportState{};
      viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      viewportState.viewportCount = 1;
      viewportState.pViewports = &viewport;
      viewportState.scissorCount = 1;
      viewportState.pScissors = &scissor;

      VkPipelineRasterizationStateCreateInfo rasterizer{};
      rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
      rasterizer.depthClampEnable = VK_FALSE;
      rasterizer.rasterizerDiscardEnable = VK_FALSE;
      rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
      rasterizer.lineWidth = 1.0f;
      //rasterizer.cullMode = VK_CULL_MODE_NONE;
      rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
      rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
      rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
      rasterizer.depthBiasEnable = VK_FALSE;

      VkPipelineMultisampleStateCreateInfo multisampling{};
      multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      multisampling.sampleShadingEnable = VK_FALSE;
      multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

      VkPipelineColorBlendAttachmentState colorBlendAttachment{};
      colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
      colorBlendAttachment.blendEnable = VK_FALSE;

      VkPipelineColorBlendStateCreateInfo colorBlending{};
      colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
      colorBlending.logicOpEnable = VK_FALSE;
      colorBlending.logicOp = VK_LOGIC_OP_COPY;
      colorBlending.attachmentCount = 1;
      colorBlending.pAttachments = &colorBlendAttachment;
      colorBlending.blendConstants[0] = 0.0f;
      colorBlending.blendConstants[1] = 0.0f;
      colorBlending.blendConstants[2] = 0.0f;
      colorBlending.blendConstants[3] = 0.0f;

      VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
      pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      pipelineLayoutInfo.setLayoutCount = 1;
      pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

      if (vkCreatePipelineLayout(device->device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
      }

      VkGraphicsPipelineCreateInfo pipelineInfo{};
      pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
      pipelineInfo.stageCount = 2;
      pipelineInfo.pStages = shaderStages;
      pipelineInfo.pVertexInputState = &vertexInputInfo;
      pipelineInfo.pInputAssemblyState = &inputAssembly;
      pipelineInfo.pViewportState = &viewportState;
      pipelineInfo.pRasterizationState = &rasterizer;
      pipelineInfo.pMultisampleState = &multisampling;
      pipelineInfo.pColorBlendState = &colorBlending;
      pipelineInfo.layout = pipelineLayout;
      pipelineInfo.renderPass = renderPass;
      pipelineInfo.subpass = 0;
      pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

      if (vkCreateGraphicsPipelines(device->device, device->pipelineCache, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
      }

      vkDestroyShaderModule(device->device, fragShaderModule, nullptr);
      vkDestroyShaderModule(device->device, vertShaderModule, nullptr);
    }

    {
      auto commands = device->beginSingleTimeCommands(device->transientCommandPool);

      //must wait for 
      {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = skybox->skyboxImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        //barrier.srcAccessMask = 0;
        //barrier.dstAccessMask = 0;

        vkCmdPipelineBarrier(
          commands,
          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
          0,
          0, nullptr,
          0, nullptr,
          1, &barrier
        );
      }

      VkClearValue clearValues[1];
      clearValues[0].color = { 0.0f, 0.5f, 0.0f, 1.0f };

      VkRenderPassBeginInfo renderPassInfo{};
      renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      renderPassInfo.renderPass = renderPass;
      renderPassInfo.framebuffer = framebuffer;
      renderPassInfo.renderArea.offset = { 0, 0 };
      renderPassInfo.renderArea.extent.height = irradianceDim;
      renderPassInfo.renderArea.extent.width = irradianceDim;
      renderPassInfo.clearValueCount = 1;
      renderPassInfo.pClearValues = clearValues;

      vkCmdBeginRenderPass(commands, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

      VkDeviceSize offsets[1] = { 0 };


      VkBuffer vertexBuffers[] = { skybox->model.vertexBuffer };
      vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
      vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
      vkCmdBindVertexBuffers(commands, 0, 1, vertexBuffers, offsets);
      vkCmdBindIndexBuffer(commands, skybox->model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
      vkCmdDrawIndexed(commands, skybox->model.indices, 1, 0, 0, 0);


      vkCmdEndRenderPass(commands);

      if (vkEndCommandBuffer(commands) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
      }

      VkSubmitInfo submitInfo{};
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &commands;

      if (vkQueueSubmit(device->graphicsQueue, 1, &submitInfo, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
      }
      vkDeviceWaitIdle(device->device);
      vkFreeCommandBuffers(device->device, device->transientCommandPool, 1, &commands);
    }

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = samplerInfo.addressModeU;
    samplerInfo.addressModeW = samplerInfo.addressModeU;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f; // Optional
    //samplerInfo.maxLod = static_cast<float>(mipLevels);
    samplerInfo.maxLod = 0.0f;
    samplerInfo.mipLodBias = 0.0f; // Optional

    if (vkCreateSampler(device->device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
      throw std::runtime_error("failed to create texture sampler!");
    }

    //Cleanup
    vkDestroyFramebuffer(device->device, framebuffer, nullptr);
    vkDestroyPipeline(device->device, pipeline, nullptr);
    vkDestroyPipelineLayout(device->device, pipelineLayout, nullptr);

    //Generate reflectionmap
    {
      //uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(reflectDim, reflectDim)))) + 1;
      {
        auto vertShaderCode = Tools::readFile("compiledshaders/cubemap.vert.spv");
        auto fragShaderCode = Tools::readFile("compiledshaders/reflectmap.frag.spv");

        VkShaderModule vertShaderModule = Tools::createShaderModule(device->device, vertShaderCode);
        VkShaderModule fragShaderModule = Tools::createShaderModule(device->device, fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = nullptr;
        viewportState.scissorCount = 1;
        viewportState.pScissors = nullptr;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        //rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(float);

        VkDynamicState dynamicStates[] = {
          VK_DYNAMIC_STATE_VIEWPORT,
          VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
        dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.pNext = nullptr;
        dynamicStateCreateInfo.flags = 0;
        dynamicStateCreateInfo.dynamicStateCount = 2;
        dynamicStateCreateInfo.pDynamicStates = dynamicStates;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        pipelineLayoutInfo.pushConstantRangeCount = 1;

        if (vkCreatePipelineLayout(device->device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
          throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.pDynamicState = &dynamicStateCreateInfo;

        if (vkCreateGraphicsPipelines(device->device, device->pipelineCache, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
          throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device->device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device->device, vertShaderModule, nullptr);
      }

      uint32_t mipLevels = 8;
      //uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight))));
      createImage(reflectImage, reflectDim, reflectDim, mipLevels, reflectMemory);
      for (uint32_t mip = 0; mip < mipLevels; mip++) {
        createImageView(reflectImage, VK_IMAGE_ASPECT_COLOR_BIT, 1, &reflectView, 0, mip);

        float roughness = (float)mip / (float)(mipLevels - 1);
        //float roughness = 0.25;

        uint32_t mipDim = reflectDim * std::pow(0.5, mip);

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &reflectView;
        framebufferInfo.width = mipDim;
        framebufferInfo.height = mipDim;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device->device, &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
          throw std::runtime_error("failed to create framebuffer!");
        }

        auto commands = device->beginSingleTimeCommands(device->transientCommandPool);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)mipDim;
        viewport.height = (float)mipDim;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = (float)mipDim;
        scissor.extent.height = (float)mipDim;

        VkClearValue clearValues[1];
        clearValues[0].color = { 0.0f, 0.5f, 0.0f, 1.0f };

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = framebuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent.height = mipDim;
        renderPassInfo.renderArea.extent.width = mipDim;
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(commands, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdSetViewport(commands, 0, 1, &viewport);
        vkCmdSetScissor(commands, 0, 1, &scissor);
        vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

        vkCmdPushConstants(commands, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), (void*)&roughness);

        VkBuffer vertexBuffers[] = { skybox->model.vertexBuffer };
        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(commands, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commands, skybox->model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commands, skybox->model.indices, 1, 0, 0, 0);


        vkCmdEndRenderPass(commands);

        if (vkEndCommandBuffer(commands) != VK_SUCCESS) {
          throw std::runtime_error("failed to record command buffer!");
        }

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commands;

        if (vkQueueSubmit(device->graphicsQueue, 1, &submitInfo, nullptr) != VK_SUCCESS) {
          throw std::runtime_error("failed to submit draw command buffer!");
        }
        vkDeviceWaitIdle(device->device);
        vkFreeCommandBuffers(device->device, device->transientCommandPool, 1, &commands);
        vkDestroyFramebuffer(device->device, framebuffer, nullptr);
        vkDestroyImageView(device->device, reflectView, nullptr);
      }
      createImageView(reflectImage, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, &reflectView);

      VkSamplerCreateInfo samplerInfo{};
      samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
      samplerInfo.magFilter = VK_FILTER_LINEAR;
      samplerInfo.minFilter = VK_FILTER_LINEAR;
      samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      samplerInfo.addressModeV = samplerInfo.addressModeU;
      samplerInfo.addressModeW = samplerInfo.addressModeU;
      samplerInfo.anisotropyEnable = VK_TRUE;
      samplerInfo.maxAnisotropy = 16.0f;
      samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
      samplerInfo.unnormalizedCoordinates = VK_FALSE;
      samplerInfo.compareEnable = VK_FALSE;
      samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
      samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
      //samplerInfo.minLod = 0.0f; // Optional
      //samplerInfo.minLod = static_cast<float>(4);
      samplerInfo.maxLod = static_cast<float>(mipLevels);
      samplerInfo.mipLodBias = 0.0f; // Optional

      if (vkCreateSampler(device->device, &samplerInfo, nullptr, &reflectSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
      }
    }
    //cleanup
    vkFreeDescriptorSets(device->device, device->descriptorPool, 1, &descriptorSet);
    vkDestroyPipeline(device->device, pipeline, nullptr);
    vkDestroyPipelineLayout(device->device, pipelineLayout, nullptr);

    //clean model related
    vkDestroyDescriptorSetLayout(device->device, descriptorSetLayout, nullptr);
    vkDestroyBuffer(device->device, buffer, nullptr);
    vkFreeMemory(device->device, memory, nullptr);
    vkDestroyRenderPass(device->device, renderPass, nullptr);

    createDescriptors();
    //cleanup pipeline,renderpass ...
  }
}

void PBRMaps::createDescriptors() {
  VkDescriptorSetLayoutBinding irradianceMapBinding{};
  irradianceMapBinding.binding = 0;
  irradianceMapBinding.descriptorCount = 1;
  irradianceMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  irradianceMapBinding.pImmutableSamplers = nullptr;
  irradianceMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding reflectMapBinding{};
  reflectMapBinding.binding = 1;
  reflectMapBinding.descriptorCount = 1;
  reflectMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  reflectMapBinding.pImmutableSamplers = nullptr;
  reflectMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


  std::array<VkDescriptorSetLayoutBinding, 2> bindings = { irradianceMapBinding, reflectMapBinding };
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(device->device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = device->descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
  allocInfo.pSetLayouts = &descriptorSetLayout;

  if (vkAllocateDescriptorSets(device->device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate descriptor sets!");
  }

  VkDescriptorImageInfo imageInfo{};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = irradianceView;
  imageInfo.sampler = sampler;

  VkDescriptorImageInfo imageInfo2{};
  imageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo2.imageView = reflectView;
  imageInfo2.sampler = reflectSampler;

  std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

  descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[0].dstSet = descriptorSet;
  descriptorWrites[0].dstBinding = 0;
  descriptorWrites[0].dstArrayElement = 0;
  descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptorWrites[0].descriptorCount = 1;
  descriptorWrites[0].pImageInfo = &imageInfo;

  descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[1].dstSet = descriptorSet;
  descriptorWrites[1].dstBinding = 1;
  descriptorWrites[1].dstArrayElement = 0;
  descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptorWrites[1].descriptorCount = 1;
  descriptorWrites[1].pImageInfo = &imageInfo2;

  vkUpdateDescriptorSets(device->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void PBRMaps::createImage(VkImage& image, uint32_t width, uint32_t height, uint32_t mipLevels, VkDeviceMemory& memory)
{
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = mipLevels;
  imageInfo.arrayLayers = 6;
  imageInfo.format = imageFormat;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

  if (vkCreateImage(device->device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device->device, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  if (vkAllocateMemory(device->device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }

  vkBindImageMemory(device->device, image, memory, 0);
}

void PBRMaps::createImageView(VkImage image, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkImageView* view, uint32_t baseArray, uint32_t baseMip) {
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
  viewInfo.format = imageFormat;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = baseMip;
  viewInfo.subresourceRange.baseArrayLayer = baseArray;
  viewInfo.subresourceRange.layerCount = 6;
  viewInfo.subresourceRange.levelCount = mipLevels;


  if (vkCreateImageView(device->device, &viewInfo, nullptr, view) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image view!");
  }

}

void PBRMaps::cleanup() {
  vkFreeDescriptorSets(device->device, device->descriptorPool, 1, &descriptorSet);
  vkDestroyDescriptorSetLayout(device->device, descriptorSetLayout, VK_NULL_HANDLE);

  vkDestroySampler(device->device, sampler, nullptr);
  vkDestroyImageView(device->device, irradianceView, nullptr);
  vkDestroyImage(device->device, irradianceImage, nullptr);
  vkFreeMemory(device->device, irradianceMemory, nullptr);
  
  vkDestroySampler(device->device, reflectSampler, nullptr);
  vkDestroyImageView(device->device, reflectView, nullptr);
  vkDestroyImage(device->device, reflectImage, nullptr);
  vkFreeMemory(device->device, reflectMemory, nullptr);
}