#pragma once

#include "GameX/utils/utils.h"

namespace GameX::Graphics {
GAMEX_DECLARE_CLASS(Scene)

GAMEX_CLASS(ParticleGroup) {
 public:
  ParticleGroup(PScene scene) : scene_(scene) {
  }

  virtual ~ParticleGroup();

  virtual grassland::vulkan::Pipeline *Pipeline() = 0;

  virtual void Draw(VkCommandBuffer cmd_buffer, int frame_index) = 0;

 protected:
  PScene scene_;
};
}  // namespace GameX::Graphics
