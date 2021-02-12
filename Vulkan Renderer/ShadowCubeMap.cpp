#include "ShadowCubeMap.h"

void ShadowCubeMap::setupShadowMap(glm::vec4 *lightPos, std::vector<std::shared_ptr<Model>> &m, int frameCount) {
  this->lightPos = lightPos;
  models = m;
	//create the color and depth images
  colorImage.resize(frameCount);
  colorView.resize(frameCount);
  colorMemory.resize(frameCount);
  for(auto i = 0; i < frameCount; i++){
    createImage(colorImage[i],colorMemory[i], colorFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    createView(colorImage[i], colorView[i], colorFormat, VK_COMPONENT_SWIZZLE_R, VK_IMAGE_ASPECT_COLOR_BIT);
    //createView(colorImage, colorView, colorFormat, VK_COMPONENT_SWIZZLE_IDENTITY, VK_IMAGE_ASPECT_COLOR_BIT);
  }

  createImage(depthImage, depthMemory, Tools::findDepthFormat(device->physicalDevice), VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
  createView(depthImage, depthView, Tools::findDepthFormat(device->physicalDevice), VK_COMPONENT_SWIZZLE_IDENTITY, VK_IMAGE_ASPECT_DEPTH_BIT);

  createRenderPass(&renderPass);

  framebuffer.resize(frameCount);
  for (auto i = 0; i < frameCount; i++)
    createFramebuffer(i, renderPass);

  //create descriptorSetLayout
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

    if (vkCreateDescriptorSetLayout(device->device, &layoutInfo, nullptr, &renderDescriptorSetLayout) != VK_SUCCESS) {
      throw std::runtime_error("failed to create descriptor set layout!");
    }
  }

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { renderDescriptorSetLayout, models[0]->descriptorSetLayout};

	//create graphicspipeline
  createGraphicsPipeline(&pipelineLayout, &pipeline, renderPass, descriptorSetLayouts);

  //create render descriptor buffer
  device->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory);
  {
    std::vector<VkDescriptorSetLayout> layouts(1, renderDescriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = device->descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(device->device, &allocInfo, &renderDescriptorSet) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate descriptor sets!");
    }

    std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = bufferSize;

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = renderDescriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(device->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
  }


  //THE IMAGE TO BE SAMPELED FROM

  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeV = samplerInfo.addressModeU;
  samplerInfo.addressModeW = samplerInfo.addressModeU;
  samplerInfo.anisotropyEnable = VK_FALSE;
  samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.maxAnisotropy = 1.0f;
  samplerInfo.minLod = 0.0f; // Optional
  samplerInfo.maxLod = 1.0f;
  samplerInfo.mipLodBias = 0.0f; // Optional

  if (vkCreateSampler(device->device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
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

  if (vkCreateDescriptorSetLayout(device->device, &layoutInfo, nullptr, &shadowDescriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }

  shadowDescriptorSet.resize(frameCount);
  std::vector<VkDescriptorSetLayout> layouts(frameCount, shadowDescriptorSetLayout);
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = device->descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
  allocInfo.pSetLayouts = layouts.data();

  if (vkAllocateDescriptorSets(device->device, &allocInfo, shadowDescriptorSet.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate descriptor sets!");
  }

  for (auto i = 0; i < frameCount; i++) {
    std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = colorView[i];
    imageInfo.sampler = sampler;

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = shadowDescriptorSet[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(device->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
  }
}

void ShadowCubeMap::cleanup()
{

  vkDestroyPipeline(device->device, pipeline, nullptr);
  vkDestroyPipelineLayout(device->device, pipelineLayout, nullptr);
  vkDestroyRenderPass(device->device, renderPass, nullptr);

  vkFreeDescriptorSets(device->device, device->descriptorPool, 1, &renderDescriptorSet);
  vkDestroyDescriptorSetLayout(device->device, renderDescriptorSetLayout, nullptr);

  vkDestroyBuffer(device->device, buffer, nullptr);
  vkFreeMemory(device->device, bufferMemory, nullptr);

  
  for(auto i = 0; i < colorImage.size(); i++){
    vkDestroyFramebuffer(device->device, framebuffer[i], nullptr);
    vkFreeDescriptorSets(device->device, device->descriptorPool, 1, &shadowDescriptorSet[i]);
    vkDestroyImageView(device->device, colorView[i], nullptr);
    vkDestroyImage(device->device, colorImage[i], nullptr);
    vkFreeMemory(device->device, colorMemory[i], nullptr);
  }

  vkDestroySampler(device->device, sampler, nullptr);
  vkDestroyImageView(device->device, depthView, nullptr);
  vkDestroyImage(device->device, depthImage, nullptr);
  vkFreeMemory(device->device, depthMemory, nullptr);

  
  vkDestroyDescriptorSetLayout(device->device, shadowDescriptorSetLayout, VK_NULL_HANDLE);

}

void ShadowCubeMap::draw(VkCommandBuffer &commands, int idx) {
  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
  clearValues[1].depthStencil = { 1.0f, 0 };
  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = renderPass;
  renderPassInfo.framebuffer = framebuffer[idx];
  renderPassInfo.renderArea.offset = { 0, 0 };
  renderPassInfo.renderArea.extent.height = dim;
  renderPassInfo.renderArea.extent.width = dim;
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commands, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &renderDescriptorSet, 0, nullptr);

  for (auto& m : models) {
    vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &m->descriptorSets[idx], 0, nullptr);
    VkDeviceSize offsets[] = { 0 };
    VkBuffer vertexBuffers[] = { m->vertexBuffer };

    vkCmdBindVertexBuffers(commands, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commands, m->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    
    for(auto& mesh: m->meshes)
      vkCmdDrawIndexed(commands, mesh.indices, 1, mesh.start_index, 0, 0);
  }

  vkCmdEndRenderPass(commands);

}

void ShadowCubeMap::createRenderPass(VkRenderPass* renderPass) {

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = colorFormat;
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

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = Tools::findDepthFormat(device->physicalDevice);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    //std::array<VkSubpassDependency, 2> dependencies;
    //dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    //dependencies[0].dstSubpass = 0;
    //dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    //dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    //dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    //dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    //dependencies[1].srcSubpass = 0;
    //dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    //dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    //dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    //std::array<VkSubpassDependency, 1> dependencies;
    //dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    //dependencies[0].dstSubpass = 0;
    //dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //dependencies[0].srcAccessMask = 0;
    //dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::array<VkSubpassDependency, 1> dependencies;
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = 0;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    //std::array<VkAttachmentDescription, 1> attachments = { colorAttachment };

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    const uint32_t viewMask = 0b00111111;
    const uint32_t correlationMask = 0b00111111;

    VkRenderPassMultiviewCreateInfo renderPassMultiviewCI{};
    renderPassMultiviewCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
    renderPassMultiviewCI.subpassCount = 1;
    renderPassMultiviewCI.pViewMasks = &viewMask;
    renderPassMultiviewCI.correlationMaskCount = 1;
    renderPassMultiviewCI.pCorrelationMasks = &correlationMask;

    renderPassInfo.pNext = &renderPassMultiviewCI;

    if (vkCreateRenderPass(device->device, &renderPassInfo, nullptr, renderPass) != VK_SUCCESS) {
      throw std::runtime_error("failed to create render pass!");
    }
}

void ShadowCubeMap::createFramebuffer(int i, VkRenderPass &renderPass) {

  std::array<VkImageView, 2> attachments = {
    colorView[i],
    depthView
  };

  //std::array<VkImageView, 1> attachments = {
  //colorView
  //};
  
  VkFramebufferCreateInfo framebufferInfo{};
  framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferInfo.renderPass = renderPass;
  framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  framebufferInfo.pAttachments = attachments.data();
  framebufferInfo.width = dim;
  framebufferInfo.height = dim;
  framebufferInfo.layers = 1;

  if (vkCreateFramebuffer(device->device, &framebufferInfo, nullptr, &framebuffer[i]) != VK_SUCCESS) {
    throw std::runtime_error("failed to create framebuffer!");
  }
  
}

void ShadowCubeMap::createGraphicsPipeline(VkPipelineLayout* pipelineLayout, VkPipeline* pipeline, VkRenderPass &renderPass, std::vector< VkDescriptorSetLayout> & descriptorSetLayouts) {
  {
    
    auto vertShaderCode = Tools::readFile("compiledshaders/shadowcubemap.vert.spv");
    auto fragShaderCode = Tools::readFile("compiledshaders/shadowcubemap.frag.spv");

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

    std::array<VkPipelineShaderStageCreateInfo,2> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

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

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    //rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    //rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
    //rasterizer.cullMode = VK_CULL_MODE_NONE;
    


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

    if (vkCreatePipelineLayout(device->device, &pipelineLayoutInfo, nullptr, pipelineLayout) != VK_SUCCESS) {
      throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.layout = *pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(device->device, device->pipelineCache, 1, &pipelineInfo, nullptr, pipeline) != VK_SUCCESS) {
      throw std::runtime_error("failed to create graphics pipeline!");
    }
    vkDestroyShaderModule(device->device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device->device, vertShaderModule, nullptr);
  }
}

void ShadowCubeMap::createImage(VkImage& image, VkDeviceMemory& memory, VkFormat format, VkImageUsageFlags flags) {
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = dim;
  imageInfo.extent.height = dim;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 6;
  //imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = flags;
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

void ShadowCubeMap::createView(VkImage &image, VkImageView &view, VkFormat format, VkComponentSwizzle swizzle, VkImageAspectFlags aspectFlag) {
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
  //viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.components = { swizzle };
  viewInfo.subresourceRange.aspectMask = aspectFlag;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 6;
  //viewInfo.subresourceRange.layerCount = 1;
  viewInfo.subresourceRange.levelCount = 1;


  if (vkCreateImageView(device->device, &viewInfo, nullptr, &view) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image view!");
  }
}

void ShadowCubeMap::updateRenderDescriptorSets() {
  glm::vec3 eye = *lightPos;
  float near_plane = 1.0f, far_plane = 100.0f;

  glm::mat4 proj = Tools::projection2(glm::perspective(glm::radians(90.0f), 1.0f, near_plane, far_plane));

  OffscreenUbo ubo = { proj,  {
   glm::lookAt(eye, eye + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
   glm::lookAt(eye, eye + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
   glm::lookAt(eye, eye + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
   glm::lookAt(eye, eye + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
   glm::lookAt(eye, eye + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
   glm::lookAt(eye, eye + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
  }, *lightPos };

  void* data;
  vkMapMemory(device->device, bufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, &ubo, bufferSize);
  vkUnmapMemory(device->device, bufferMemory);
}