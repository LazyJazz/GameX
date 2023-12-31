#include "GameBall/logic/logic.h"

namespace GameBall::Logic {

Manager::Manager() {
  metronome_ = std::make_unique<GameX::Base::Metronome>();
  world_ = std::make_unique<class World>();
}

Manager::~Manager() {
  world_.reset();
  metronome_.reset();
}

class World *Manager::World() const {
  return world_.get();
}

void Manager::Start() {
  logic_thread_ = std::thread(&Manager::LogicThread, this);
}

void Manager::Stop() {
  stop_signal_ = true;
  logic_thread_.join();
}

void Manager::LogicThread() {
  while (!stop_signal_) {
    {
      std::lock_guard<std::mutex> lock(logic_mutex_);
      world_->UpdateTick();
    }
    metronome_->Tick();
  }
}
}  // namespace GameBall::Logic
