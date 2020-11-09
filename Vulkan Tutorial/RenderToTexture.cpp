#include "RenderToTexture.h"

void RenderToTexture::createShadowMap(glm::vec4 lightPos, std::vector<std::shared_ptr<Model>> models) {
	//create the image holding depth buffer
	Tools::createImage(device, image, imageMemory, dim, dim, FORMAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	Tools::createImageView(device, image, imageView, FORMAT, VK_IMAGE_ASPECT_COLOR_BIT);
  //create renderpass
  VkRenderPass renderPass;
  {
    VkAttachmentDescription attachment{};
    attachment.format = FORMAT;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference attachmentRef{};
    attachmentRef.attachment = 0;
    attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 1> attachments = { attachment };// , colorAttachmentResolve};

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(1);
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device->device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
      throw std::runtime_error("failed to create render pass!");
    }
  }
	//create framebuffer
  VkFramebuffer framebuffer;
  {
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &imageView;
    framebufferInfo.width = dim;
    framebufferInfo.height = dim;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device->device, &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
  VkDescriptorSetLayout descriptorSetLayout;
  {
    VkDescriptorSetLayoutBinding vertLayoutBinding{};
    vertLayoutBinding.binding = 0;
    vertLayoutBinding.descriptorCount = 1;
    vertLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vertLayoutBinding.pImmutableSamplers = nullptr;
    vertLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    std::array<VkDescriptorSetLayoutBinding, 1> bindings = { vertLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device->device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
      throw std::runtime_error("failed to create descriptor set layout!");
    }
  }
  std::array< VkDescriptorSetLayout, 2> descriptorSetLayouts = { descriptorSetLayout ,models[0]->descriptorSetLayout};
	//create graphicspipeline
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;
  {
    auto vertShaderCode = Tools::readFile("shaders/basic.vert.spv");
    auto fragShaderCode = Tools::readFile("shaders/basic.frag.spv");

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
    viewport.width = (float)dim;
    viewport.height = (float)dim;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = (float)dim;
    scissor.extent.height = (float)dim;

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
    rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
    //rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
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
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

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


    if (vkCreateGraphicsPipelines(device->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
      throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(device->device, vertShaderModule, nullptr);
    vkDestroyShaderModule(device->device, fragShaderModule, nullptr);
  }

  float near_plane = 1.0f, far_plane = 150.0f;
  //glm::mat4 depthProjectionMatrix = glm::perspective(glm::radians(60.0f), 1.0f, near_plane, far_plane);
  //depthProjectionMatrix[1][1] *= -1;
  //glm::mat4 depthViewMatrix = glm::lookAt(glm::vec3(lightPos), glm::vec3(lightPos)+glm::vec3(0.0f, 0.0f,  -1.0f),  glm::vec3(0.0f, 1.0f, 0.0f));

  //glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, near_plane, far_plane);
  //proj[1][1] *= -1;

  //glm::mat4 proj = Tools::projection(glm::perspective(glm::radians(90.0f), 1.0f, near_plane, far_plane));
  glm::mat4 proj = Tools::projection(glm::perspective(glm::radians(90.0f), 1.0f, near_plane, far_plane));
  /*OffscreenUbo ubo = { proj,  {
     glm::lookAt(eye, eye + glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
     glm::lookAt(eye, eye + glm::vec3(-1.0f, 0.0f,  0.0f),   glm::vec3(0.0f, -1.0f, 0.0f)),
     glm::lookAt(eye, eye + glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec3(0.0f, 0.0f, 1.0f)),
     glm::lookAt(eye, eye + glm::vec3(0.0f,  -1.0f,  0.0f),  glm::vec3(0.0f, 0.0f, -1.0f)),
     glm::lookAt(eye, eye + glm::vec3(0.0f,  0.0f,  1.0f),  glm::vec3(0.0f, -1.0f, 0.0f)),
     glm::lookAt(eye, eye + glm::vec3(0.0f,  0.0f, -1.0f),  glm::vec3(0.0f, -1.0f, 0.0f))
    }, lightPos };*/

  //glm::mat4 lightViewMatrix = depthProjectionMatrix * depthViewMatrix;

  //front
  //glm::lookAt(eye, eye + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)
  //back
  //glm::lookAt(eye, eye + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)
  //up
  //glm::lookAt(eye, eye + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)
  //down
  //glm::lookAt(eye, eye + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)
  //left
  //glm::lookAt(eye, eye + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)
  //right
  //glm::lookAt(eye, eye + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)

  glm::vec3 eye = lightPos;
  glm::mat4 lightViewMatrix = proj * glm::lookAt(eye, eye + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  VkDeviceSize bufferSize = sizeof(glm::mat4);
  
  glm::mat4 viewMatrix = glm::mat4(1.0f);
  viewMatrix = glm::translate(viewMatrix, glm::vec3(lightPos));
  //viewMatrix = glm::rotate(viewMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //right
  //viewMatrix = glm::rotate(viewMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //left
  //viewMatrix = glm::rotate(viewMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); //down
  //viewMatrix = glm::rotate(viewMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); //up
  // default view matrix //front
  viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //back
  viewMatrix = proj * viewMatrix;
  device->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory);
  void* data;
  vkMapMemory(device->device, bufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, &lightViewMatrix, bufferSize);
  vkUnmapMemory(device->device, bufferMemory);

  VkDescriptorSet descriptorSet;

  //allocate descriptorsets
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

    std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = bufferSize;

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(device->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
  }

  {
    auto commands = device->beginSingleTimeCommands(device->transientCommandPool);

    VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent.height = dim;
    renderPassInfo.renderArea.extent.width = dim;
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commands, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

    for (auto& m : models) {
      vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &m->descriptorSets[0], 0, nullptr);
      VkDeviceSize offsets[1] = { 0 };
      VkBuffer vertexBuffers[] = { m->vertexBuffer };

      vkCmdBindVertexBuffers(commands, 0, 1, vertexBuffers, offsets);
      vkCmdBindIndexBuffer(commands, m->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
      
      for(auto& mesh: m->meshes)
        vkCmdDrawIndexed(commands, mesh.indices, 1, mesh.start_index, 0, 0);
    }

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
	//commandbuffer, render from lights position

  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = 16.0f;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

  if (vkCreateSampler(device->device, &samplerInfo, nullptr, &imageSampler) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture sampler!");
  }

  VkDescriptorSetLayoutBinding samplerLayoutBinding{};
  samplerLayoutBinding.binding = 0;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 1> bindings = { samplerLayoutBinding };

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(device->device, &layoutInfo, nullptr, &descriptorLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = device->descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &descriptorLayout;

  if (vkAllocateDescriptorSets(device->device, &allocInfo, &descriptor) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate descriptor sets!");
  }


  std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

  VkDescriptorImageInfo imageInfo{};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = imageView;
  imageInfo.sampler = imageSampler;

  descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[0].dstSet = descriptor;
  descriptorWrites[0].dstBinding = 0;
  descriptorWrites[0].dstArrayElement = 0;
  descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptorWrites[0].descriptorCount = 1;
  descriptorWrites[0].pImageInfo = &imageInfo;


  vkUpdateDescriptorSets(device->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

}