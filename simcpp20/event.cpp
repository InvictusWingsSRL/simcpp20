// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include "event.hpp"

#include "simulation.hpp"

namespace simcpp20 {
// event

event::event(simulation &sim) : shared(std::make_shared<shared_state>(sim)) {}

void event::trigger() {
  if (triggered()) {
    return;
  }

  shared->state = event_state::triggered;
  shared->sim.schedule(0, *this);
}

void event::trigger_delayed(simtime delay) {
  if (triggered()) {
    return;
  }

  shared->sim.schedule(delay, *this);
}

void event::add_callback(std::function<void(event &)> cb) {
  if (processed()) {
    return;
  }

  shared->cbs.emplace_back(cb);
}

bool event::pending() {
  return shared->state == event_state::pending;
}

bool event::triggered() {
  return shared->state == event_state::triggered ||
         shared->state == event_state::processed;
}

bool event::processed() {
  return shared->state == event_state::processed;
}

void event::process() {
  if (processed()) {
    return;
  }

  shared->state = event_state::processed;

  for (auto &handle : shared->handles) {
    handle.resume();
  }

  shared->handles.clear();

  for (auto &cb : shared->cbs) {
    cb(*this);
  }

  shared->cbs.clear();
}

void event::add_handle(std::coroutine_handle<> handle) {
  if (processed()) {
    return;
  }

  shared->handles.emplace_back(handle);
}

// event::shared_state

event::shared_state::shared_state(simulation &sim) : sim(sim) {}

event::shared_state::~shared_state() {
  for (auto &handle : handles) {
    handle.destroy();
  }
}
} // namespace simcpp20
