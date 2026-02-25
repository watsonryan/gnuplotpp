/**
 * @file logging.hpp
 * @brief Logging helpers for gnuplotpp examples and runtime diagnostics.
 */
#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#ifdef GNUPLOTPP_HAS_SPDLOG
#include <spdlog/spdlog.h>
#endif

namespace gnuplotpp::log {

template <typename... Args>
inline std::string BuildMessage(Args&&... args) {
  std::ostringstream oss;
  (oss << ... << std::forward<Args>(args));
  return oss.str();
}

template <typename... Args>
inline void Info(Args&&... args) {
  const auto msg = BuildMessage(std::forward<Args>(args)...);
#ifdef GNUPLOTPP_HAS_SPDLOG
  spdlog::info("{}", msg);
#else
  std::clog << "[gnuplotpp] info: " << msg << "\n";
#endif
}

template <typename... Args>
inline void Warn(Args&&... args) {
  const auto msg = BuildMessage(std::forward<Args>(args)...);
#ifdef GNUPLOTPP_HAS_SPDLOG
  spdlog::warn("{}", msg);
#else
  std::clog << "[gnuplotpp] warn: " << msg << "\n";
#endif
}

template <typename... Args>
inline void Error(Args&&... args) {
  const auto msg = BuildMessage(std::forward<Args>(args)...);
#ifdef GNUPLOTPP_HAS_SPDLOG
  spdlog::error("{}", msg);
#else
  std::clog << "[gnuplotpp] error: " << msg << "\n";
#endif
}

}  // namespace gnuplotpp::log
