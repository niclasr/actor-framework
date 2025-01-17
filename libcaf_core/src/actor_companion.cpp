// This file is part of CAF, the C++ Actor Framework. See the file LICENSE in
// the main distribution directory for license terms and copyright or visit
// https://github.com/actor-framework/actor-framework/blob/master/LICENSE.

#include "caf/locks.hpp"
#include "caf/actor_companion.hpp"

namespace caf {

actor_companion::actor_companion(actor_config& cfg) : extended_base(cfg) {
  // nop
}

actor_companion::~actor_companion() {
  // nop
}

void actor_companion::on_enqueue(enqueue_handler handler) {
  std::lock_guard<lock_type> guard(lock_);
  on_enqueue_ = std::move(handler);
}

void actor_companion::on_exit(on_exit_handler handler) {
  on_exit_ = std::move(handler);
}

bool actor_companion::enqueue(mailbox_element_ptr ptr, execution_unit*) {
  CAF_ASSERT(ptr);
  shared_lock<lock_type> guard(lock_);
  if (on_enqueue_) {
    on_enqueue_(std::move(ptr));
    return true;
  } else {
    return false;
  }
}

bool actor_companion::enqueue(strong_actor_ptr src, message_id mid,
                              message content, execution_unit* eu) {
  auto ptr = make_mailbox_element(std::move(src), mid, {}, std::move(content));
  return enqueue(std::move(ptr), eu);
}

void actor_companion::launch(execution_unit*, bool, bool hide) {
  if (!hide)
    register_at_system();
}

void actor_companion::on_exit() {
  enqueue_handler tmp;
  { // lifetime scope of guard
    std::lock_guard<lock_type> guard(lock_);
    on_enqueue_.swap(tmp);
  }
  if (on_exit_)
    on_exit_();
}

} // namespace caf
