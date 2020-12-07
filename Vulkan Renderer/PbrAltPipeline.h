#pragma once
#include "RenderingPipeline.h"
class PbrAltPipeline :
    public RenderingPipeline
{
public:
  void draw(VkCommandBuffer&, std::vector<VkDescriptorSet*>, int) override;
};

