#pragma once
#include "Pipeline.h"
#include "Model.h"
class RenderingPipeline :
    public Pipeline
{
public:
  std::vector<std::shared_ptr<Model>> models;
  virtual void draw(VkCommandBuffer&, std::vector<VkDescriptorSet*>, int);
  void createGraphicsPipeline(std::string vertFilePath, std::string fragFilePath) override;
  void cleanup() override;
  bool valid();
};

