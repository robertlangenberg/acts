// This file is part of the Acts project.
//
// Copyright (C) 2019 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Acts/Utilities/TypeTraits.hpp"

#include <optional>
#include <string_view>
#include <variant>

namespace Acts {

template <typename Derived, typename... States>
class FiniteStateMachine {
 public:
  struct Terminated {
    constexpr static std::string_view name = "Terminated";
  };
  using StateVariant = std::variant<Terminated, States...>;

 protected:
  using fsm_base = FiniteStateMachine<Derived, States...>;

  using event_return = std::optional<StateVariant>;

 public:
  FiniteStateMachine()
      : m_state(
            typename std::tuple_element<0, std::tuple<States...>>::type{}){};

  FiniteStateMachine(StateVariant state) : m_state(std::move(state)){};

  const StateVariant& getState() const noexcept { return m_state; }

  StateVariant& getState() noexcept { return m_state; }
 private:
  template <typename T, typename S, typename... Args>
  using on_exit_t = decltype(
      std::declval<T>().on_exit(std::declval<S&>(), std::declval<Args>()...));

  template <typename T, typename S, typename... Args>
  using on_enter_t = decltype(
      std::declval<T>().on_enter(std::declval<S&>(), std::declval<Args>()...));

 public:
  template <typename State, typename... Args>
  void setState(State state, Args&&... args) {
    Derived& child = static_cast<Derived&>(*this);

    // call on exit function
    std::visit(
        [&](auto& s) {
          using state_type = decltype(s);
          if constexpr (concept ::exists<on_exit_t, Derived, state_type,
                                         Args...>) {
            child.on_exit(s, std::forward<Args>(args)...);
          }
        },
        m_state);

    m_state = std::move(state);

    // call on enter function, the type is known from the template argument.
    if constexpr (concept ::exists<on_enter_t, Derived, State, Args...>) {
      child.on_enter(std::get<State>(m_state), std::forward<Args>(args)...);
    }
  }

  template <typename S>
  bool is(const S& /*state*/) const noexcept {
    if (std::get_if<S>(&m_state)) {
      return true;
    }
    return false;
  }

  bool terminated() const noexcept { return is(Terminated{}); }

 private:
  template <typename T, typename S, typename E, typename... Args>
  using on_event_t = decltype(std::declval<T>().on_event(
      std::declval<S&>(), std::declval<E&>(), std::declval<Args>()...));

  template <typename T, typename... Args>
  using on_process_t =
      decltype(std::declval<T>().on_process(std::declval<Args>()...));

 protected:
  template <typename Event, typename... Args>
  event_return process_event(Event&& event, Args&&... args) {
    Derived& child = static_cast<Derived&>(*this);

    if constexpr (concept ::exists<on_process_t, Derived, Event>) {
      child.on_process(event);
    }

    auto new_state = std::visit(
        [&](auto& s) -> std::optional<StateVariant> {
          using state_type = decltype(s);

          if constexpr (concept ::exists<on_event_t, Derived, state_type, Event,
                                         Args...>) {
            auto s2 = child.on_event(s, std::forward<Event>(event),
                                     std::forward<Args>(args)...);

            if (s2) {
              std::visit(
                  [&](auto& s2_) {
                    if constexpr (concept ::exists<on_process_t, Derived,
                                                   state_type, Event,
                                                   decltype(s2_)>) {
                      child.on_process(s, event, s2_);
                    }
                  },
                  *s2);
            } else {
              if constexpr (concept ::exists<on_process_t, Derived, state_type,
                                             Event>) {
                child.on_process(s, event);
              }
            }
            return std::move(s2);
          } else {
            if constexpr (concept ::exists<on_process_t, Derived, state_type,
                                           Event, Terminated>) {
              child.on_process(s, event, Terminated{});
            }
            return Terminated{};
          }
        },
        m_state);
    return std::move(new_state);
  }

  template <typename Event, typename... Args>
  void dispatch(Event&& event, Args&&... args) {
    auto new_state = process_event(std::forward<Event>(event), args...);
    if (new_state) {
      std::visit(
          [&](auto& s) { setState(std::move(s), std::forward<Args>(args)...); },
          *new_state);
    }
  }

 private:
  StateVariant m_state;
};

}  // namespace Acts