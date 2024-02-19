#include "GameX/renderer/particle_groups/color_group.h"

#include "GameX/application/application.h"
#include "GameX/renderer/renderer.h"
#include "GameX/shaders/shaders.h"

namespace GameX::Graphics {

ColorParticleGroup::ColorParticleGroup(PScene scene,
                                       PModel model,
                                       uint32_t max_particle_num)
    : ParticleGroup(scene) {
  model_ = model;
  particle_info_buffer_ =
      std::make_unique<grassland::vulkan::DynamicBuffer<ParticleInfo>>(
          scene_->Renderer()->App()->VkCore(), max_particle_num,
          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  global_settings_buffer_ =
      std::make_unique<grassland::vulkan::DynamicBuffer<GlobalSettings>>(
          scene_->Renderer()->App()->VkCore(), 1,
          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

  global_settings_buffer_->At(0) = GlobalSettings{1.0f};

  scene_->Renderer()->RegisterSyncObject(particle_info_buffer_.get());
  scene_->Renderer()->RegisterSyncObject(global_settings_buffer_.get());

  descriptor_sets_.resize(
      scene_->Renderer()->App()->VkCore()->MaxFramesInFlight());
  for (int i = 0; i < descriptor_sets_.size(); i++) {
    descriptor_sets_[i] = std::make_unique<grassland::vulkan::DescriptorSet>(
        scene->Renderer()->App()->VkCore(), scene_->DescriptorPool(),
        ColorParticleGroupDescriptorSetLayout(scene_->Renderer()));

    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = global_settings_buffer_->GetBuffer(i)->Handle();
    buffer_info.offset = 0;
    buffer_info.range = global_settings_buffer_->Size();

    VkWriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.dstSet = descriptor_sets_[i]->Handle();
    write_descriptor_set.dstBinding = 0;
    write_descriptor_set.dstArrayElement = 0;
    write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_descriptor_set.descriptorCount = 1;
    write_descriptor_set.pBufferInfo = &buffer_info;

    vkUpdateDescriptorSets(
        scene_->Renderer()->App()->VkCore()->Device()->Handle(), 1,
        &write_descriptor_set, 0, nullptr);
  }
}

ColorParticleGroup::~ColorParticleGroup() {
  scene_->Renderer()->UnregisterSyncObject(particle_info_buffer_.get());
  scene_->Renderer()->UnregisterSyncObject(global_settings_buffer_.get());
}

grassland::vulkan::Pipeline *ColorParticleGroup::Pipeline() {
  return ColorParticleGroupPipeline(scene_->Renderer());
}

void ColorParticleGroup::Draw(VkCommandBuffer cmd_buffer, int frame_index) {
  if (particle_num_) {
    VkDescriptorSet descriptor_sets[] = {
        descriptor_sets_[frame_index]->Handle()};
    vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            ColorParticleGroupPipeline(scene_->Renderer())
                                ->Settings()
                                .pipeline_layout->Handle(),
                            1, 1, descriptor_sets, 0, nullptr);
    VkBuffer vertex_buffers[] = {
        model_->VertexBuffer(frame_index)->Handle(),
        particle_info_buffer_->GetBuffer(frame_index)->Handle()};
    VkDeviceSize offsets[] = {0, 0};
    vkCmdBindVertexBuffers(cmd_buffer, 0, 2, vertex_buffers, offsets);
    vkCmdBindIndexBuffer(cmd_buffer, model_->IndexBuffer(frame_index)->Handle(),
                         0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmd_buffer, model_->IndexCount(), particle_num_, 0, 0, 0);
  }
}

grassland::vulkan::DescriptorSetLayout *ColorParticleGroupDescriptorSetLayout(
    class Renderer *renderer) {
  static std::map<class Renderer *, grassland::vulkan::DescriptorSetLayout *>
      color_particle_group_descriptor_set_layouts;
  auto &color_particle_group_descriptor_set_layout =
      color_particle_group_descriptor_set_layouts[renderer];
  if (!color_particle_group_descriptor_set_layout) {
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.resize(1);
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bindings[0].pImmutableSamplers = nullptr;

    color_particle_group_descriptor_set_layout =
        new grassland::vulkan::DescriptorSetLayout(renderer->App()->VkCore(),
                                                   bindings);

    renderer->AddReleaseCallback(
        [color_particle_group_descriptor_set_layout]() {
          delete color_particle_group_descriptor_set_layout;
        });
  }
  return color_particle_group_descriptor_set_layout;
}

grassland::vulkan::Pipeline *ColorParticleGroupPipeline(
    class Renderer *renderer) {
  static std::map<class Renderer *, PipelineAssets>
      color_particle_group_pipelines;
  auto &pipeline = color_particle_group_pipelines[renderer];

  if (!pipeline.pipeline) {
    pipeline.vertex_shader = new grassland::vulkan::ShaderModule(
        renderer->App()->VkCore(),
        BuiltInShaderSpv("color_particle_group.vert"));
    pipeline.fragment_shader = new grassland::vulkan::ShaderModule(
        renderer->App()->VkCore(),
        BuiltInShaderSpv("color_particle_group.frag"));
    pipeline.pipeline_layout = new grassland::vulkan::PipelineLayout(
        renderer->App()->VkCore(),
        std::vector<grassland::vulkan::DescriptorSetLayout *>{
            CameraDescriptorSetLayout(renderer),
            ColorParticleGroupDescriptorSetLayout(renderer)});

    grassland::vulkan::PipelineSettings color_particle_group_pipeline_settings(
        renderer->RenderPipeline()->RenderPass(), pipeline.pipeline_layout, 0);

    color_particle_group_pipeline_settings.AddShaderStage(
        pipeline.vertex_shader, VK_SHADER_STAGE_VERTEX_BIT);
    color_particle_group_pipeline_settings.AddShaderStage(
        pipeline.fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT);

    color_particle_group_pipeline_settings.AddInputBinding(
        0, sizeof(Base::Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
    color_particle_group_pipeline_settings.AddInputAttribute(
        0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Base::Vertex, position));
    color_particle_group_pipeline_settings.AddInputAttribute(
        0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Base::Vertex, color));
    color_particle_group_pipeline_settings.AddInputAttribute(
        0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Base::Vertex, texCoord));
    color_particle_group_pipeline_settings.AddInputAttribute(
        0, 3, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Base::Vertex, normal));
    color_particle_group_pipeline_settings.AddInputAttribute(
        0, 4, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Base::Vertex, tangent));
    color_particle_group_pipeline_settings.AddInputBinding(
        1, sizeof(ColorParticleGroup::ParticleInfo),
        VK_VERTEX_INPUT_RATE_INSTANCE);
    color_particle_group_pipeline_settings.AddInputAttribute(
        1, 5, VK_FORMAT_R32G32B32_SFLOAT,
        offsetof(ColorParticleGroup::ParticleInfo, position));
    color_particle_group_pipeline_settings.AddInputAttribute(
        1, 6, VK_FORMAT_R32G32B32_SFLOAT,
        offsetof(ColorParticleGroup::ParticleInfo, color));

    color_particle_group_pipeline_settings.SetPrimitiveTopology(
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    color_particle_group_pipeline_settings.SetMultiSampleState(
        VK_SAMPLE_COUNT_1_BIT);

    color_particle_group_pipeline_settings.SetCullMode(VK_CULL_MODE_BACK_BIT);

    color_particle_group_pipeline_settings.SetBlendState(
        0, VkPipelineColorBlendAttachmentState{
               VK_FALSE,
               VK_BLEND_FACTOR_SRC_ALPHA,
               VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
               VK_BLEND_OP_ADD,
               VK_BLEND_FACTOR_SRC_ALPHA,
               VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
               VK_BLEND_OP_ADD,
               VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                   VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
           });

    pipeline.pipeline = new grassland::vulkan::Pipeline(
        renderer->App()->VkCore(), color_particle_group_pipeline_settings);

    renderer->AddReleaseCallback([pipeline]() {
      delete pipeline.pipeline;
      delete pipeline.pipeline_layout;
      delete pipeline.vertex_shader;
      delete pipeline.fragment_shader;
    });
  }

  return pipeline.pipeline;
}
}  // namespace GameX::Graphics
