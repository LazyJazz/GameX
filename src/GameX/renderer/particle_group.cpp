#include "GameX/renderer/particle_group.h"

#include "GameX/renderer/scene.h"

namespace GameX::Graphics {

ParticleGroup::~ParticleGroup() {
  scene_->DestroyParticleGroup(this);
}
}  // namespace GameX::Graphics
