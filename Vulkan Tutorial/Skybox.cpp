#include "Skybox.h"

//
//void setupOffscreenTexture() {
//  offscreenTexture.device = &device;
//  offscreenTexture.width = offscreenDim;
//  offscreenTexture.height = offscreenDim;
//  offscreenTexture.mipLevels = 1;
//  offscreenTexture.numSamples = VK_SAMPLE_COUNT_1_BIT;
//  offscreenTexture.format = offscreenFormat;
//  //offscreenTexture.format = swapChainImageFormat;
//  offscreenTexture.tiling = VK_IMAGE_TILING_OPTIMAL;
//  offscreenTexture.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
//  offscreenTexture.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
//  offscreenTexture.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
//
//  offscreenTexture.createImage();
//  offscreenTexture.createImageView();
//  offscreenTexture.createTextureSampler();
//
//
//
//
//  //offscreenUbo.proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1.0f);
//  offscreenUbo.proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
//  //offscreenUbo.proj[1][1] *= -1;
//  //offscreenUbo.view = glm::mat4(1.0f);
//
//
//}
//
//void createOffscreenRenderpass() {
//  VkAttachmentDescription colorAttachment{};
//  colorAttachment.format = offscreenFormat;
//  //colorAttachment.format = swapChainImageFormat;
//  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//  //colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//  //colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
//
//  VkAttachmentReference colorAttachmentRef{};
//  colorAttachmentRef.attachment = 0;
//  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//  VkAttachmentDescription colorAttachmentResolve{};
//  colorAttachmentResolve.format = swapChainImageFormat;
//  colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
//  colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//  colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//  colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//  colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//  colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//  //colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//  //colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//  //colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//  colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
//
//  VkAttachmentReference colorAttachmentResolveRef{};
//  colorAttachmentResolveRef.attachment = 1;
//  colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//  VkSubpassDescription subpass{};
//  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//  subpass.colorAttachmentCount = 1;
//  subpass.pColorAttachments = &colorAttachmentRef;
//  //subpass.pResolveAttachments = &colorAttachmentResolveRef;
//
//  VkSubpassDependency dependency{};
//  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//  dependency.dstSubpass = 0;
//  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//  dependency.srcAccessMask = 0;
//  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//
//  // Use subpass dependencies for layout transitions
//  //std::array<VkSubpassDependency, 2> dependencies;
//  //dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
//  //dependencies[0].dstSubpass = 0;
//  //dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
//  //dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//  //dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//  //dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//  //dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//  //dependencies[1].srcSubpass = 0;
//  //dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
//  //dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//  //dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
//  //dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//  //dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//  //dependencies[1].dstAccessMask = 0;
//  //dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//
//  std::array<VkAttachmentDescription, 1> attachments = { colorAttachment };// , colorAttachmentResolve};
//
//  VkRenderPassCreateInfo renderPassInfo{};
//  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
//  renderPassInfo.pAttachments = attachments.data();
//  renderPassInfo.subpassCount = 1;
//  renderPassInfo.pSubpasses = &subpass;
//  renderPassInfo.dependencyCount = 1;
//  renderPassInfo.pDependencies = &dependency;
//
//  if (vkCreateRenderPass(device.device, &renderPassInfo, nullptr, &offscreenRenderPass) != VK_SUCCESS) {
//    throw std::runtime_error("failed to create render pass!");
//  }
//}
//
//void createOffscreenFramebuffer() {
//  std::array<VkImageView, 1> attachments = {
//    offscreenTexture.view,
//    //swapChainImageViews2[0]
//  };
//  VkFramebufferCreateInfo framebufferInfo{};
//  framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//  framebufferInfo.renderPass = offscreenRenderPass;
//  framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
//  framebufferInfo.pAttachments = attachments.data();
//  framebufferInfo.width = offscreenDim;
//  framebufferInfo.height = offscreenDim;
//  framebufferInfo.layers = 1;
//
//  if (vkCreateFramebuffer(device.device, &framebufferInfo, nullptr, &offscreenFramebuffer) != VK_SUCCESS) {
//    throw std::runtime_error("failed to create framebuffer!");
//  }
//}
//
//void createOffscreenPipeline() {
//
//  {
//    VkDescriptorSetLayoutBinding vertLayoutBinding{};
//    vertLayoutBinding.binding = 0;
//    vertLayoutBinding.descriptorCount = 1;
//    vertLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//    vertLayoutBinding.pImmutableSamplers = nullptr;
//    vertLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
//
//    VkDescriptorSetLayoutBinding fragLayoutBinding{};
//    fragLayoutBinding.binding = 1;
//    fragLayoutBinding.descriptorCount = 1;
//    fragLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//    fragLayoutBinding.pImmutableSamplers = nullptr;
//    fragLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
//
//    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { vertLayoutBinding, fragLayoutBinding };
//    VkDescriptorSetLayoutCreateInfo layoutInfo{};
//    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
//    layoutInfo.pBindings = bindings.data();
//
//    if (vkCreateDescriptorSetLayout(device.device, &layoutInfo, nullptr, &offscreenDescriptorSetLayout) != VK_SUCCESS) {
//      throw std::runtime_error("failed to create descriptor set layout!");
//    }
//  }
//
//  {
//    auto vertShaderCode = readFile("shaders/cubemap_vert.spv");
//    auto fragShaderCode = readFile("shaders/cubemap_frag.spv");
//
//    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
//    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
//
//    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
//    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
//    vertShaderStageInfo.module = vertShaderModule;
//    vertShaderStageInfo.pName = "main";
//
//    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
//    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//    fragShaderStageInfo.module = fragShaderModule;
//    fragShaderStageInfo.pName = "main";
//
//    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
//
//    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
//    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//
//    auto bindingDescription = Vertex::getBindingDescription();
//    auto attributeDescriptions = Vertex::getAttributeDescriptions();
//
//    vertexInputInfo.vertexBindingDescriptionCount = 1;
//    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
//    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
//    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
//
//    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
//    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//    inputAssembly.primitiveRestartEnable = VK_FALSE;
//
//    VkViewport viewport{};
//    viewport.x = 0.0f;
//    viewport.y = 0.0f;
//    viewport.width = (float)offscreenDim;
//    viewport.height = (float)offscreenDim;
//    viewport.minDepth = 0.0f;
//    viewport.maxDepth = 1.0f;
//
//    VkRect2D scissor{};
//    scissor.offset.x = 0;
//    scissor.offset.y = 0;
//    scissor.extent.height = (float)offscreenDim;
//    scissor.extent.width = (float)offscreenDim;
//
//    VkPipelineViewportStateCreateInfo viewportState{};
//    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//    viewportState.viewportCount = 1;
//    viewportState.pViewports = &viewport;
//    viewportState.scissorCount = 1;
//    viewportState.pScissors = &scissor;
//
//    VkPipelineRasterizationStateCreateInfo rasterizer{};
//    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//    rasterizer.depthClampEnable = VK_FALSE;
//    rasterizer.rasterizerDiscardEnable = VK_FALSE;
//    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
//    rasterizer.lineWidth = 1.0f;
//    rasterizer.cullMode = VK_CULL_MODE_NONE;
//    //rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
//    //rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
//    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
//    rasterizer.depthBiasEnable = VK_FALSE;
//
//    VkPipelineMultisampleStateCreateInfo multisampling{};
//    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//    multisampling.sampleShadingEnable = VK_FALSE;
//    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
//
//    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
//    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//    colorBlendAttachment.blendEnable = VK_FALSE;
//
//    VkPipelineColorBlendStateCreateInfo colorBlending{};
//    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//    colorBlending.logicOpEnable = VK_FALSE;
//    colorBlending.logicOp = VK_LOGIC_OP_COPY;
//    colorBlending.attachmentCount = 1;
//    colorBlending.pAttachments = &colorBlendAttachment;
//    colorBlending.blendConstants[0] = 0.0f;
//    colorBlending.blendConstants[1] = 0.0f;
//    colorBlending.blendConstants[2] = 0.0f;
//    colorBlending.blendConstants[3] = 0.0f;
//
//    VkPushConstantRange pushConstantRange{};
//    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
//    pushConstantRange.offset = 0;
//    pushConstantRange.size = sizeof(UniformBufferObject);
//
//    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
//    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//    pipelineLayoutInfo.setLayoutCount = 1;
//    pipelineLayoutInfo.pSetLayouts = &offscreenDescriptorSetLayout;
//
//    pipelineLayoutInfo.pushConstantRangeCount = 1;
//    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
//
//    if (vkCreatePipelineLayout(device.device, &pipelineLayoutInfo, nullptr, &offscreenPipelineLayout) != VK_SUCCESS) {
//      throw std::runtime_error("failed to create pipeline layout!");
//    }
//
//    VkGraphicsPipelineCreateInfo pipelineInfo{};
//    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//    pipelineInfo.stageCount = 2;
//    pipelineInfo.pStages = shaderStages;
//    pipelineInfo.pVertexInputState = &vertexInputInfo;
//    pipelineInfo.pInputAssemblyState = &inputAssembly;
//    pipelineInfo.pViewportState = &viewportState;
//    pipelineInfo.pRasterizationState = &rasterizer;
//    pipelineInfo.pMultisampleState = &multisampling;
//    pipelineInfo.pColorBlendState = &colorBlending;
//    pipelineInfo.layout = offscreenPipelineLayout;
//    pipelineInfo.renderPass = offscreenRenderPass;
//    pipelineInfo.subpass = 0;
//    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
//
//    if (vkCreateGraphicsPipelines(device.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &offscreenPipeline) != VK_SUCCESS) {
//      throw std::runtime_error("failed to create graphics pipeline!");
//    }
//
//    vkDestroyShaderModule(device.device, fragShaderModule, nullptr);
//    vkDestroyShaderModule(device.device, vertShaderModule, nullptr);
//  }
//}
//
//void setupOffscreenSkybox() {
//  offscreenHDR.device = &device;
//  offscreenHDR.load("envmaps/001.hdr");
//
//  //offscreenModel.modelPos = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//  //offscreenModel.modelPos = glm::lookAt(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//  //offscreenModel.modelPos[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
//  offscreenModel.device = &device;
//  offscreenModel.swapChainSize = 1;
//  offscreenModel.descriptorPool = descriptorPool;
//  offscreenModel.loadModel("skybox/cube.obj");
//  offscreenModel.descriptorSetLayout = offscreenDescriptorSetLayout;
//  offscreenModel.createDescriptorBuffers();
//
//
//  //allocate descriptorsets
//  {
//    std::vector<VkDescriptorSetLayout> layouts(1, offscreenDescriptorSetLayout);
//    VkDescriptorSetAllocateInfo allocInfo{};
//    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//    allocInfo.descriptorPool = descriptorPool;
//    allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
//    allocInfo.pSetLayouts = layouts.data();
//
//    offscreenModel.descriptorSets.resize(1);
//    if (vkAllocateDescriptorSets(device.device, &allocInfo, offscreenModel.descriptorSets.data()) != VK_SUCCESS) {
//      throw std::runtime_error("failed to allocate descriptor sets!");
//    }
//
//    VkDescriptorBufferInfo bufferInfo{};
//    bufferInfo.buffer = offscreenModel.descriptorBuffer[0];
//    bufferInfo.offset = 0;
//    bufferInfo.range = sizeof(glm::mat4);
//
//    VkDescriptorImageInfo imageInfo{};
//    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//    imageInfo.imageView = offscreenHDR.view;
//    imageInfo.sampler = offscreenHDR.sampler;
//    //imageInfo.imageView = offscreenModel.textures[0].view;
//    //imageInfo.sampler = offscreenModel.textures[0].sampler;
//
//    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
//
//    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//    descriptorWrites[0].dstSet = offscreenModel.descriptorSets[0];
//    descriptorWrites[0].dstBinding = 0;
//    descriptorWrites[0].dstArrayElement = 0;
//    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//    descriptorWrites[0].descriptorCount = 1;
//    descriptorWrites[0].pBufferInfo = &bufferInfo;
//
//    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//    descriptorWrites[1].dstSet = offscreenModel.descriptorSets[0];
//    descriptorWrites[1].dstBinding = 1;
//    descriptorWrites[1].dstArrayElement = 0;
//    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//    descriptorWrites[1].descriptorCount = 1;
//    descriptorWrites[1].pImageInfo = &imageInfo;
//
//    vkUpdateDescriptorSets(device.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
//  }
//
//}
//
//void createOffscreenCommandBuffer() {
//  VkCommandBufferAllocateInfo allocInfo{};
//  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//  allocInfo.commandPool = device.graphicsCommandPool;
//  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//  allocInfo.commandBufferCount = 1;
//
//  if (vkAllocateCommandBuffers(device.device, &allocInfo, &offscreenCommandBuffer) != VK_SUCCESS) {
//    throw std::runtime_error("failed to allocate command buffers!");
//  }
//  VkCommandBufferBeginInfo beginInfo{};
//  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//
//  if (vkBeginCommandBuffer(offscreenCommandBuffer, &beginInfo) != VK_SUCCESS) {
//    throw std::runtime_error("failed to begin recording command buffer!");
//  }
//
//  VkClearValue clearValues[1];
//  clearValues[0].color = { 0.0f, 0.5f, 0.0f, 1.0f };
//
//  VkRenderPassBeginInfo renderPassInfo{};
//  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//  renderPassInfo.renderPass = offscreenRenderPass;
//  renderPassInfo.framebuffer = offscreenFramebuffer;
//  renderPassInfo.renderArea.offset = { 0, 0 };
//  renderPassInfo.renderArea.extent.height = offscreenDim;
//  renderPassInfo.renderArea.extent.width = offscreenDim;
//  renderPassInfo.clearValueCount = 1;
//  renderPassInfo.pClearValues = clearValues;
//
//  for (int i = 0; i < offScreencaptureViews.size(); i++) {
//    vkCmdBeginRenderPass(offscreenCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
//    offscreenUbo.view = offScreencaptureViews[i];
//    vkCmdPushConstants(offscreenCommandBuffer, offscreenPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UniformBufferObject), (void*)&offscreenUbo);
//
//    VkDeviceSize offsets[1] = { 0 };
//
//
//    VkBuffer vertexBuffers[] = { offscreenModel.vertexBuffer };
//    vkCmdBindPipeline(offscreenCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenPipeline);
//    vkCmdBindDescriptorSets(offscreenCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenPipelineLayout, 0, 1, &offscreenModel.descriptorSets[0], 0, nullptr);
//    vkCmdBindVertexBuffers(offscreenCommandBuffer, 0, 1, vertexBuffers, offsets);
//    vkCmdBindIndexBuffer(offscreenCommandBuffer, offscreenModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
//    vkCmdDrawIndexed(offscreenCommandBuffer, offscreenModel.indices, 1, 0, 0, 0);
//
//
//    vkCmdEndRenderPass(offscreenCommandBuffer);
//
//    //VkImageMemoryBarrier preBarrier{};
//    //preBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//
//    //preBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//    //preBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//    //preBarrier.image = sTex.image;
//    //preBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//    //preBarrier.subresourceRange.baseMipLevel = 0;
//    //preBarrier.subresourceRange.levelCount = 1;
//    //preBarrier.subresourceRange.baseArrayLayer = i;
//    //preBarrier.subresourceRange.layerCount = 1;
//
//    //preBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//    //preBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//
//    //preBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//    //preBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//
//    //vkCmdPipelineBarrier(
//    //  offscreenCommandBuffer,
//    //  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
//    //  0,
//    //  0, nullptr,
//    //  0, nullptr,
//    //  1, &preBarrier
//    //);
//
//    {
//      VkImageCopy imageCopyRegion{};
//      imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//      imageCopyRegion.srcSubresource.baseArrayLayer = 0;
//      imageCopyRegion.srcSubresource.layerCount = 1;
//      imageCopyRegion.srcSubresource.mipLevel = 0;
//
//      imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//      imageCopyRegion.dstSubresource.layerCount = 1;
//      imageCopyRegion.dstSubresource.baseArrayLayer = i;
//      imageCopyRegion.dstSubresource.mipLevel = 0;
//
//      imageCopyRegion.extent.width = offscreenDim;
//      imageCopyRegion.extent.height = offscreenDim;
//      imageCopyRegion.extent.depth = 1;
//
//      vkCmdCopyImage(offscreenCommandBuffer, offscreenTexture.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, sTex.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
//
//      VkImageMemoryBarrier dstBarrier{};
//      dstBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//
//      dstBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//      dstBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//      dstBarrier.image = sTex.image;
//      dstBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//      dstBarrier.subresourceRange.baseMipLevel = 0;
//      dstBarrier.subresourceRange.levelCount = 1;
//      dstBarrier.subresourceRange.baseArrayLayer = i;
//      dstBarrier.subresourceRange.layerCount = 1;
//
//      dstBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//      dstBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//
//      dstBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//      dstBarrier.dstAccessMask = 0;
//
//      vkCmdPipelineBarrier(
//        offscreenCommandBuffer,
//        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
//        0,
//        0, nullptr,
//        0, nullptr,
//        1, &dstBarrier
//      );
//    }
//
//
//
//    //vkCmdBeginRenderPass(offscreenCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
//    //offscreenUbo.view = offScreencaptureViews[1];
//    //vkCmdPushConstants(offscreenCommandBuffer, offscreenPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UniformBufferObject), (void*)&offscreenUbo);
//
//    ////VkDeviceSize offsets[1] = { 0 };
//
//
//    ////VkBuffer vertexBuffers[] = { offscreenModel.vertexBuffer };
//    //vkCmdBindPipeline(offscreenCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenPipeline);
//    //vkCmdBindDescriptorSets(offscreenCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenPipelineLayout, 0, 1, &offscreenModel.descriptorSets[0], 0, nullptr);
//    //vkCmdBindVertexBuffers(offscreenCommandBuffer, 0, 1, vertexBuffers, offsets);
//    //vkCmdBindIndexBuffer(offscreenCommandBuffer, offscreenModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
//    //vkCmdDrawIndexed(offscreenCommandBuffer, offscreenModel.indices, 1, 0, 0, 0);
//
//
//    //vkCmdEndRenderPass(offscreenCommandBuffer);
//
//
//    //{
//    //  VkImageCopy imageCopyRegion{};
//    //  imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//    //  imageCopyRegion.srcSubresource.baseArrayLayer = 0;
//    //  imageCopyRegion.srcSubresource.layerCount = 1;
//    //  imageCopyRegion.srcSubresource.mipLevel = 0;
//
//    //  imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//    //  imageCopyRegion.dstSubresource.layerCount = 6;
//    //  imageCopyRegion.dstSubresource.baseArrayLayer = 1;
//    //  imageCopyRegion.dstSubresource.mipLevel = 0;
//
//    //  imageCopyRegion.extent.width = offscreenDim;
//    //  imageCopyRegion.extent.height = offscreenDim;
//    //  imageCopyRegion.extent.depth = 1;
//
//    //  vkCmdCopyImage(offscreenCommandBuffer, offscreenTexture.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, sTex.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
//    //}
//    //{
//    //  VkImageMemoryBarrier dstBarrier{};
//    //  dstBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//
//    //  dstBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//    //  dstBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//    //  dstBarrier.image = sTex.image;
//    //  dstBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//    //  dstBarrier.subresourceRange.baseMipLevel = 0;
//    //  dstBarrier.subresourceRange.levelCount = 1;
//    //  dstBarrier.subresourceRange.baseArrayLayer = 1;
//    //  dstBarrier.subresourceRange.layerCount = 6;
//
//    //  dstBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//    //  dstBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//
//    //  dstBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//    //  dstBarrier.dstAccessMask = 0;
//
//    //  vkCmdPipelineBarrier(
//    //    offscreenCommandBuffer,
//    //    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
//    //    0,
//    //    0, nullptr,
//    //    0, nullptr,
//    //    1, &dstBarrier
//    //  );
//    //}
//  }
//
//  if (vkEndCommandBuffer(offscreenCommandBuffer) != VK_SUCCESS) {
//    throw std::runtime_error("failed to record command buffer!");
//  }
//}
//
//void renderOffscreen() {
//  //for (int i = 0; i < offScreencaptureViews.size(); i++) {
//  auto pushConstantCB = device.beginSingleTimeCommands();
//  offscreenUbo.view = offScreencaptureViews[0];
//  vkCmdPushConstants(pushConstantCB, offscreenPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UniformBufferObject), (void*)&offscreenUbo);
//  vkEndCommandBuffer(pushConstantCB);
//  std::array<VkCommandBuffer, 2> submitCommandBuffers =
//  { pushConstantCB, offscreenCommandBuffer };
//  VkSubmitInfo submitInfo{};
//  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//  submitInfo.commandBufferCount = 1;
//  submitInfo.pCommandBuffers = &offscreenCommandBuffer;
//  //submitInfo.commandBufferCount = static_cast<uint32_t>(submitCommandBuffers.size());
//  //submitInfo.pCommandBuffers = submitCommandBuffers.data();
//  //VkSemaphore waitSemaphores[] = { offscreenImageAvailableSemaphore };
//  //VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
//  //submitInfo.waitSemaphoreCount = 1;
//  //submitInfo.pWaitSemaphores = waitSemaphores;
//  //submitInfo.pWaitDstStageMask = waitStages;
//  //VkSemaphore signalSemaphores[] = { offscreenRenderFinishedSemaphore };
//  //submitInfo.signalSemaphoreCount = 1;
//  //submitInfo.pSignalSemaphores = signalSemaphores;
//
//  //dstStageMask = COLOR_ATTACHMENT_OUTPUT
//
//  if (vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, nullptr) != VK_SUCCESS) {
//    throw std::runtime_error("failed to submit draw command buffer!");
//  }
//
//  //  VkCommandBuffer commandBuffer = device.beginSingleTimeCommands(device.transferCommandPool);
//
//  //  //srcStageMask = COLOR_ATTACHMENT_OUTPUT
//
//  //  VkImageCopy imageCopyRegion{};
//  //  imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//  //  imageCopyRegion.srcSubresource.layerCount = 1;
//  //  imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//  //  imageCopyRegion.dstSubresource.layerCount = 1;
//  //  imageCopyRegion.extent.width = offscreenDim;
//  //  imageCopyRegion.extent.height = offscreenDim;
//  //  imageCopyRegion.extent.depth = 1;
//  //  imageCopyRegion.dstSubresource.mipLevel = 0;
//  //  vkCmdCopyImage(commandBuffer, offscreenTexture.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, sTex.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
//
//  //  VkImageMemoryBarrier dstBarrier{};
//  //  dstBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//
//  //  dstBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//  //  dstBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//  //  dstBarrier.image = sTex.image;
//  //  dstBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//  //  dstBarrier.subresourceRange.baseMipLevel = 0;
//  //  dstBarrier.subresourceRange.levelCount = 1;
//  //  dstBarrier.subresourceRange.baseArrayLayer = i;
//  //  dstBarrier.subresourceRange.layerCount = 6;
//  //  dstBarrier.srcAccessMask = 0;
//  //  dstBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//
//  //  dstBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//  //  dstBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//  //  dstBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//  //  dstBarrier.dstAccessMask = 0;
//
//  //  vkCmdPipelineBarrier(
//  //    commandBuffer,
//  //    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
//  //    0,
//  //    0, nullptr,
//  //    0, nullptr,
//  //    1, &dstBarrier
//  //  );
//
//  //  device.endSingleTimeCommands(device.transferCommandPool, device.transferQueue, commandBuffer);
//  //}
//
//
//  //{
//  //  VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
//
//  //  VkImageMemoryBarrier barrier{};
//  //  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//  //  barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//  //  barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//  //  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//  //  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//  //  barrier.image = swapChainImages2[0];
//  //  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//  //  barrier.subresourceRange.baseMipLevel = 0;
//  //  barrier.subresourceRange.levelCount = 1;
//  //  barrier.subresourceRange.baseArrayLayer = 0;
//  //  barrier.subresourceRange.layerCount = 1;
//
//  //  VkPipelineStageFlags sourceStage;
//  //  VkPipelineStageFlags destinationStage;
//
//  //  barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//  //  sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//
//  //  barrier.dstAccessMask = 0;
//  //  destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
//
//  //  vkCmdPipelineBarrier(
//  //    commandBuffer,
//  //    sourceStage, destinationStage,
//  //    0,
//  //    0, nullptr,
//  //    0, nullptr,
//  //    1, &barrier
//  //  );
//
//  //  device.endSingleTimeCommands(commandBuffer);
//  //}
//
//
//
//  //{
//  //  VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
//
//  //  VkImageMemoryBarrier barrier{};
//  //  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//  //  barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//  //  barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//  //  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//  //  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//  //  barrier.image = swapChainImages2[0];
//  //  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//  //  barrier.subresourceRange.baseMipLevel = 0;
//  //  barrier.subresourceRange.levelCount = 1;
//  //  barrier.subresourceRange.baseArrayLayer = 0;
//  //  barrier.subresourceRange.layerCount = 1;
//
//  //  VkPipelineStageFlags sourceStage;
//  //  VkPipelineStageFlags destinationStage;
//
//  //  barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//  //  sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//
//  //  barrier.dstAccessMask = 0;
//  //  destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
//
//  //  vkCmdPipelineBarrier(
//  //    commandBuffer,
//  //    sourceStage, destinationStage,
//  //    0,
//  //    0, nullptr,
//  //    0, nullptr,
//  //    1, &barrier
//  //  );
//
//  //  device.endSingleTimeCommands(commandBuffer);
//  //}
//}
//
//void presentOffscreen() {
//  uint32_t imageIndex;
//  VkResult result = vkAcquireNextImageKHR(device.device, swapChain2, UINT64_MAX, offscreenImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
//  VkSemaphore signalSemaphores[] = { offscreenRenderFinishedSemaphore };
//  VkPresentInfoKHR presentInfo{};
//  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//  presentInfo.waitSemaphoreCount = 1;
//  presentInfo.pWaitSemaphores = signalSemaphores;
//
//  VkSwapchainKHR swapChains[] = { swapChain2 };
//  presentInfo.swapchainCount = 1;
//  presentInfo.pSwapchains = swapChains;
//
//  presentInfo.pImageIndices = &imageIndex;
//
//  result = vkQueuePresentKHR(device.presentQueue, &presentInfo);
//
//  if (result != VK_SUCCESS) {
//    throw std::runtime_error("failed to present swap chain image!");
//  }
//}
//
//void createSkybox() {
//  //create cube model
//  //will need 6 textures
//  //will need seperate pipeline since it will need special shaders
//  //fragment shader will need texture sampler
//  //vertex shader will not need texture coordinates, instead a vector is used
//
//
//  //HdrTexture sTex;
//  sTex.device = &device;
//  //sTex.load("skybox");
//  sTex.init(offscreenDim);
//  //sTex.load("envmaps/001.hdr");
//
//
//  VkDescriptorSetLayout descriptorSetLayout;
//  VkDescriptorSetLayoutBinding vertLayoutBinding{};
//  vertLayoutBinding.binding = 0;
//  vertLayoutBinding.descriptorCount = 1;
//  vertLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//  vertLayoutBinding.pImmutableSamplers = nullptr;
//  vertLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
//
//  VkDescriptorSetLayoutBinding fragLayoutBinding{};
//  fragLayoutBinding.binding = 1;
//  fragLayoutBinding.descriptorCount = 1;
//  fragLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//  fragLayoutBinding.pImmutableSamplers = nullptr;
//  fragLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
//
//  std::array<VkDescriptorSetLayoutBinding, 2> bindings = { vertLayoutBinding, fragLayoutBinding };
//  VkDescriptorSetLayoutCreateInfo layoutInfo{};
//  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
//  layoutInfo.pBindings = bindings.data();
//
//  if (vkCreateDescriptorSetLayout(device.device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
//    throw std::runtime_error("failed to create descriptor set layout!");
//  }
//
//  //Model* model = new Model;
//  /*model->device = &device;
//  model->swapChainSize = swapChainImages.size();
//  model->descriptorPool = descriptorPool;
//  model->loadModel("skybox/cube.obj");
//  model->descriptorSetLayout = descriptorSetLayout;
//  model->createDescriptorBuffers();*/
//
//  skyboxModel.device = &device;
//  skyboxModel.swapChainSize = swapChainImages.size();
//  skyboxModel.descriptorPool = descriptorPool;
//  skyboxModel.loadModel("skybox/cube.obj");
//  skyboxModel.descriptorSetLayout = descriptorSetLayout;
//  skyboxModel.createDescriptorBuffers();
//
//
//  auto pipeline = new Pipeline;
//  pipeline->device = &device;
//  pipeline->descriptorSetLayout = descriptorSetLayout;
//  pipeline->swapChainExtent = swapChainExtent;
//  pipeline->renderPass = renderPass;
//  pipeline->pushConstantSize = sizeof(UniformBufferObject);
//  pipeline->msaaSamples = msaaSamples;
//  pipeline->createGraphicsPipeline("shaders/skybox_vert.spv", "shaders/skybox_frag.spv");
//  //pipeline->createGraphicsPipeline("shaders/equi_vert.spv", "shaders/equi_frag.spv");
//  ////create the models
//  //std::string mpath = "models/cube.obj";
//  //std::cout << mpath << std::endl;
//  //auto model = new Model();
//  //model->device = &device;
//  //model->swapChainSize = swapChainImages.size();
//  //model->descriptorPool = descriptorPool;
//  //model->loadModel(mpath);
//  ////model->createDescriptorSetLayout();
//  //model->descriptorSetLayout = descriptorSetLayout;
//  //model->createDescriptorBuffers();
//  //model->createMaterialBuffers();
//  ////model->createDescriptorSets();
//  ////#######TEMP STUFF################
//  std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
//  VkDescriptorSetAllocateInfo allocInfo{};
//  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//  allocInfo.descriptorPool = descriptorPool;
//  allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
//  allocInfo.pSetLayouts = layouts.data();
//
//  skyboxModel.descriptorSets.resize(swapChainImages.size());
//  if (vkAllocateDescriptorSets(device.device, &allocInfo, skyboxModel.descriptorSets.data()) != VK_SUCCESS) {
//    throw std::runtime_error("failed to allocate descriptor sets!");
//  }
//
//  for (size_t i = 0; i < swapChainImages.size(); i++) {
//    VkDescriptorBufferInfo bufferInfo{};
//    bufferInfo.buffer = skyboxModel.descriptorBuffer[i];
//    bufferInfo.offset = 0;
//    bufferInfo.range = sizeof(glm::mat4);
//
//    VkDescriptorImageInfo imageInfo{};
//    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//    imageInfo.imageView = sTex.view;
//    imageInfo.sampler = sTex.sampler;
//
//    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
//
//    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//    descriptorWrites[0].dstSet = skyboxModel.descriptorSets[i];
//    descriptorWrites[0].dstBinding = 0;
//    descriptorWrites[0].dstArrayElement = 0;
//    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//    descriptorWrites[0].descriptorCount = 1;
//    descriptorWrites[0].pBufferInfo = &bufferInfo;
//
//    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//    descriptorWrites[1].dstSet = skyboxModel.descriptorSets[i];
//    descriptorWrites[1].dstBinding = 1;
//    descriptorWrites[1].dstArrayElement = 0;
//    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//    descriptorWrites[1].descriptorCount = 1;
//    descriptorWrites[1].pImageInfo = &imageInfo;
//
//    vkUpdateDescriptorSets(device.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
//  }
//  pipeline->models.push_back(&skyboxModel);
//
//  //models.push_back(&skyboxModel);
//
//  pipelines.push_back(pipeline);
//}

void Skybox::load(std::string, std::string)
{
	//set up render target texture

	//set up renderpass,pipeline,framebuffer

	//set up skybox

	//render and copy each layer of the skybox texture
}
