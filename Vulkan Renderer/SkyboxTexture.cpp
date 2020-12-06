#include "SkyboxTexture.h"


void SkyboxTexture::cleanup() {
  vkDestroyDescriptorSetLayout(device->device, skyboxDescriptorSetLayout, nullptr);
  vkDestroySampler(device->device, sampler, nullptr);
  vkDestroyImageView(device->device, skyboxView, nullptr);
  vkDestroyImage(device->device, skyboxImage, nullptr);
  vkFreeMemory(device->device, skyboxMemory, nullptr);
  
  model.cleanup();
  pipeline.cleanup();
}

void SkyboxTexture::draw(VkCommandBuffer& cb)
{
  vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.graphicsPipeline);

  VkBuffer vertexBuffers[] = { model.vertexBuffer };
  VkDeviceSize offsets[] = { 0 };
  vkCmdBindVertexBuffers(cb, 0, 1, vertexBuffers, offsets);

  vkCmdBindIndexBuffer(cb, model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

  vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 0, 1, &model.descriptorSets[0], 0, nullptr);
  vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 1, 1, &skyboxDescriptorSet, 0, nullptr);

  vkCmdDrawIndexed(cb, model.indices, 1, 0, 0, 0);
}

void SkyboxTexture::loadFromFiles(const std::string directory) {
  const std::array<std::string, 6> directions = { "right","left","top","bottom","front","back" };
  std::vector<VkBuffer> buffers;
  std::vector<VkDeviceMemory> buffersMemory;
  for (int i = 0; i < 6; i++) {
    //int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load((directory + "/" + directions[i] + ".jpg").c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    //std::cout << texChannels << std::endl;
    //std::cout << texWidth << std::endl;
    //std::cout << texHeight << std::endl;
    //std::cout << texWidth * texHeight * texChannels << std::endl;
    //std::cout << sizeof(pixels) << std::endl;
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    //mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels) {
      throw std::runtime_error("failed to load" + directions[i] + " texture image!");
    }
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    device->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device->device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device->device, stagingBufferMemory);

    stbi_image_free(pixels);
    buffers.push_back(stagingBuffer);
    buffersMemory.push_back(stagingBufferMemory);
  }

  createImage(skyboxImage, texWidth, texHeight, 1, skyboxMemory);

  transitionImageLayout(skyboxImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
  copyBufferToImage(buffers);
  //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

  for (int i = 0; i < buffers.size(); i++) {
    vkDestroyBuffer(device->device, buffers[i], nullptr);
    vkFreeMemory(device->device, buffersMemory[i], nullptr);
  }

  //generateMipmaps(image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
  transitionImageLayout(skyboxImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
  createImageView(skyboxImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 1, &skyboxView);
  createTextureSampler();

}

void SkyboxTexture::loadHdr(const std::string& filename, int dim)
{
  texWidth = dim;
  texHeight = dim;
  createImage(skyboxImage, dim, dim, 1, skyboxMemory);
  createImageView(skyboxImage, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, &skyboxView);
  //createImageView(irradianceImage, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 1, &irradianceView);
  createTextureSampler();

  //Generate the textures for each side by looking at hdr texture from different angles
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

    // Use subpass dependencies for layout transitions
    //std::array<VkSubpassDependency, 2> dependencies;
    //dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    //dependencies[0].dstSubpass = 0;
    //dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    //dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    //dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    //dependencies[1].srcSubpass = 0;
    //dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    //dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    //dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    //dependencies[1].dstAccessMask = 0;
    //dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    
    //std::array<VkSubpassDependency, 1> dependencies;
    //dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    //dependencies[0].dstSubpass = 0;
    //dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //dependencies[0].srcAccessMask = 0;
    //dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

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
  VkFramebuffer framebuffer;
  {
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &skyboxView;
    framebufferInfo.width = texWidth;
    framebufferInfo.height = texHeight;
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

  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;
  {
    auto vertShaderCode = Tools::readFile("compiledshaders/cubemap.vert.spv");
    auto fragShaderCode = Tools::readFile("compiledshaders/cubemap.frag.spv");
    //auto fragShaderCode = Tools::readFile("shaders/irradiancemap.spv");

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
    viewport.width = (float)texWidth;
    viewport.height = (float)texHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = (float)texWidth;
    scissor.extent.height = (float)texHeight;

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
    //rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
  
  //Texture hdrTex;
  HdrTexture hdrTex;
  hdrTex.device = device;
  hdrTex.load(filename.c_str());

  VkDeviceSize bufferSize = sizeof(OffscreenUbo);

  //glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1.0f);
  //proj[1][1] *= -1;
  glm::mat4 proj = Tools::projection(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1.0f));
  OffscreenUbo ubo = { proj,  {
     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f),  glm::vec3(0.0f, 1.0f, 0.0f)),
     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f),  glm::vec3(0.0f, 1.0f, 0.0f)),
     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec3(0.0f, 0.0f, 1.0f)),
     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  -1.0f,  0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f),  glm::vec3(0.0f, 1.0f, 0.0f)),
     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, 1.0f),  glm::vec3(0.0f, 1.0f, 0.0f))
    } };

/*
     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f),   glm::vec3(0.0f, -1.0f, 0.0f)),
     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  -1.0f,  0.0f),  glm::vec3(0.0f, 0.0f, -1.0f)),
     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec3(0.0f, 0.0f, 1.0f)),
     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f),  glm::vec3(0.0f, -1.0f, 0.0f)),
     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f),  glm::vec3(0.0f, -1.0f, 0.0f))
*/

  VkBuffer buffer;
  VkDeviceMemory memory;

  device->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, memory);
  void* data;
  vkMapMemory(device->device, memory, 0, sizeof(OffscreenUbo), 0, &data);
  memcpy(data, &ubo, sizeof(OffscreenUbo));
  vkUnmapMemory(device->device, memory);

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

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
    
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(OffscreenUbo);

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = hdrTex.view;
    imageInfo.sampler = hdrTex.sampler;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(device->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
  }

  {
    auto commands = device->beginSingleTimeCommands(device->transientCommandPool);

    VkClearValue clearValues[1];
    clearValues[0].color = { 0.0f, 0.5f, 0.0f, 1.0f };

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent.height = texHeight;
    renderPassInfo.renderArea.extent.width = texWidth;
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commands, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkDeviceSize offsets[1] = { 0 };


    VkBuffer vertexBuffers[] = { model.vertexBuffer };
    vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    vkCmdBindVertexBuffers(commands, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commands, model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commands, model.indices, 1, 0, 0, 0);


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
  createSkyboxDescriptors();
  //texture has now been rendered
  //cleanup
  hdrTex.cleanup();
  //vkFreeDescriptorSets(device->device, device->descriptorPool, 1, &descriptorSet);
  vkDestroyFramebuffer(device->device, framebuffer, nullptr);
  vkDestroyPipeline(device->device, pipeline, nullptr);
  vkDestroyPipelineLayout(device->device, pipelineLayout, nullptr);
  vkFreeDescriptorSets(device->device, device->descriptorPool, 1, &descriptorSet);
  vkDestroyDescriptorSetLayout(device->device, descriptorSetLayout, nullptr);
  vkDestroyBuffer(device->device, buffer, nullptr);
  vkFreeMemory(device->device, memory, nullptr);
  vkDestroyRenderPass(device->device, renderPass, nullptr);
}

void SkyboxTexture::createImage(VkImage &image, uint32_t width, uint32_t height, uint32_t mipLevels, VkDeviceMemory &memory)
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

  //imageInfo.extent.width = irradianceDim;
  //imageInfo.extent.height = irradianceDim;
  ////imageInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
  //imageInfo.format = imageFormat;

  //if (vkCreateImage(device->device, &imageInfo, nullptr, &irradianceImage) != VK_SUCCESS) {
  //  throw std::runtime_error("failed to create image!");
  //}

  //vkGetImageMemoryRequirements(device->device, irradianceImage, &memRequirements);

  //allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  //allocInfo.allocationSize = memRequirements.size;
  //allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  //if (vkAllocateMemory(device->device, &allocInfo, nullptr, &irradianceMemory) != VK_SUCCESS) {
  //  throw std::runtime_error("failed to allocate image memory!");
  //}

  //vkBindImageMemory(device->device, irradianceImage, irradianceMemory, 0);
}

void SkyboxTexture::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkImageView* view, uint32_t baseArray, uint32_t baseMip) {
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = baseMip;
  viewInfo.subresourceRange.baseArrayLayer = baseArray;
  viewInfo.subresourceRange.layerCount = 6;
  //viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.levelCount = mipLevels;


  if (vkCreateImageView(device->device, &viewInfo, nullptr, view) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image view!");
  }

}

void SkyboxTexture::createTextureSampler() {
  {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f; // Optional
    //samplerInfo.maxLod = static_cast<float>(mipLevels);
    samplerInfo.maxLod = 1.0f;
    samplerInfo.mipLodBias = 0.0f; // Optional

    if (vkCreateSampler(device->device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
      throw std::runtime_error("failed to create texture sampler!");
    }
  }
}

void SkyboxTexture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
  VkCommandBuffer commandBuffer = device->beginSingleTimeCommands();

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = mipLevels;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  vkCmdPipelineBarrier(
    commandBuffer,
    sourceStage, destinationStage,
    0,
    0, nullptr,
    0, nullptr,
    1, &barrier
  );

  device->endSingleTimeCommands(commandBuffer);
}

void SkyboxTexture::copyBufferToImage(std::vector<VkBuffer>& buffers) {
  VkCommandBuffer commandBuffer = device->beginSingleTimeCommands(device->transferCommandPool);
  for (int i = 0; i < buffers.size(); i++) {
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = i;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        static_cast<uint32_t>(texWidth),
        static_cast<uint32_t>(texHeight),
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffers[i], skyboxImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
  }
  device->endSingleTimeCommands(device->transferCommandPool, device->transferQueue, commandBuffer);
}

void SkyboxTexture::createSkyboxDescriptors() {

  VkDescriptorSetLayoutBinding fragLayoutBinding{};
  fragLayoutBinding.binding = 0;
  fragLayoutBinding.descriptorCount = 1;
  fragLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  fragLayoutBinding.pImmutableSamplers = nullptr;
  fragLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 1> bindings = { fragLayoutBinding };
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(device->device, &layoutInfo, nullptr, &skyboxDescriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = device->descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
  allocInfo.pSetLayouts = &skyboxDescriptorSetLayout;


  if (vkAllocateDescriptorSets(device->device, &allocInfo, &skyboxDescriptorSet) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate descriptor sets!");
  }

  VkDescriptorImageInfo imageInfo{};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = skyboxView;
  imageInfo.sampler = sampler;

  std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

  descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[0].dstSet = skyboxDescriptorSet;
  descriptorWrites[0].dstBinding = 0;
  descriptorWrites[0].dstArrayElement = 0;
  descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptorWrites[0].descriptorCount = 1;
  descriptorWrites[0].pImageInfo = &imageInfo;

  vkUpdateDescriptorSets(device->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

}
