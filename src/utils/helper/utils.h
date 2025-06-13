#ifndef UTILS_H
#define UTILS_H

#include <concepts>
#include <stdexcept>
#include <string_view>

#include "spdlog/spdlog.h"

namespace tt::chat {

  template <std::same_as<bool> T, std::convertible_to<std::string_view> S>
  void check_error(T test, S error_message) {
    if (test) {
      SPDLOG_ERROR("{}", error_message);
      throw std::runtime_error(error_message);
    }  }
} // namespace tt::chat

#endif // UTILS_H