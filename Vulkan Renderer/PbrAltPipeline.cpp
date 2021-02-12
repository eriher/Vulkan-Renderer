#include "PbrAltPipeline.h"

void PbrAltPipeline::draw(VkCommandBuffer& cb, std::vector<VkDescriptorSet*> descriptors, int i )
{
  vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

  vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, descriptors[0], 0, nullptr);
  //vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pbr.pipeline.pipelineLayout, 3, 1, &shadowMap.descriptor, 0, nullptr);
  vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 3, 1, descriptors[1], 0, nullptr);
  vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 4, 1, descriptors[2], 0, nullptr);
  vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 5, 1, descriptors[3], 0, nullptr);
  for (auto& model : models) {
    model->draw(cb, pipelineLayout, i);
  }
}
