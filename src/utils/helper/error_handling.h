#pragma once

#include <string.h>

#include <stdexcept>
#include <string_view>

#include "spdlog/spdlog.h"
namespace helper {

bool check_error(bool test, std::string error_message,
                 bool raise_exception = true);

}  // namespace helper