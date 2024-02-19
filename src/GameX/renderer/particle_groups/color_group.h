#pragma once
#include "GameX/renderer/model.h"
#include "GameX/renderer/particle_group.h"

namespace GameX::Graphics {
GAMEX_DECLARE_CLASS(Scene)

GAMEX_CLASS(ColorParticleGroup) : public ParticleGroup {
 public:
  struct GlobalSettings {
    float scale{1.0f};
  };

  struct ParticleInfo {
    glm::vec3 position;
    glm::vec3 color;
  };

  ColorParticleGroup(PScene scene, PModel model,
                     uint32_t max_particle_num = 1048576);

  ~ColorParticleGroup() override;

  grassland::vulkan::Pipeline *Pipeline() override;

  void Draw(VkCommandBuffer cmd_buffer, int frame_index) override;

  void SetGlobalSettings(const GlobalSettings &settings) {
    global_settings_buffer_->At(0) = settings;
  }

  void SetParticleInfo(const std::vector<ParticleInfo> &particle_info) {
    particle_num_ = particle_info.size();
    if (particle_num_ > particle_info_buffer_->Length()) {
      particle_num_ = particle_info_buffer_->Length();
    }
    particle_info_buffer_->UploadContents(particle_info.data(), particle_num_);
  }

 private:
  PModel model_;
  std::unique_ptr<grassland::vulkan::DynamicBuffer<ParticleInfo>>
      particle_info_buffer_;
  std::unique_ptr<grassland::vulkan::DynamicBuffer<GlobalSettings>>
      global_settings_buffer_;
  std::vector<std::unique_ptr<grassland::vulkan::DescriptorSet>>
      descriptor_sets_;
  uint32_t particle_num_{0};
};

grassland::vulkan::DescriptorSetLayout *ColorParticleGroupDescriptorSetLayout(
    class Renderer *renderer);

grassland::vulkan::Pipeline *ColorParticleGroupPipeline(
    class Renderer *renderer);
}  // namespace GameX::Graphics
