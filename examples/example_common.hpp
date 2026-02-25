#pragma once

#include "gnuplotpp/gnuplot_backend.hpp"
#include "gnuplotpp/logging.hpp"
#include "gnuplotpp/plot.hpp"

#include <filesystem>
#include <string>

namespace example_common {

inline std::filesystem::path parse_out_dir(int argc,
                                           char** argv,
                                           const std::filesystem::path& default_out) {
  std::filesystem::path out_dir = default_out;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--out" && i + 1 < argc) {
      out_dir = argv[++i];
    }
  }
  return out_dir;
}

inline int render_figure(gnuplotpp::Figure& fig, const std::filesystem::path& out_dir) {
  fig.set_backend(gnuplotpp::make_gnuplot_backend());
  const auto result = fig.save(out_dir);
  if (!result.ok) {
    gnuplotpp::log::Error("plot render incomplete: ", result.message);
    if (!result.script_path.empty()) {
      gnuplotpp::log::Error("script: ", result.script_path.string());
    }
    return 1;
  }
  for (const auto& output : result.outputs) {
    gnuplotpp::log::Info("output: ", output.string());
  }
  return 0;
}

}  // namespace example_common
