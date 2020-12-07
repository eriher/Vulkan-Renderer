#pragma once
#include "RenderingPipeline.h"
#include "Model.h"
class PbrPipeline :
    public RenderingPipeline
{
public:
  void draw(VkCommandBuffer&, std::vector<VkDescriptorSet*>, int) override;
};

