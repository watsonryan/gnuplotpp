#pragma once

#include "gnuplotpp/plot.hpp"

#include <filesystem>
#include <vector>

namespace gnuplotpp {

/** @brief YAML loader behavior controls. */
struct YamlLoadOptions {
  /**
   * @brief Reject unknown keys when true.
   * @note Recommended for CI/config hygiene.
   */
  bool strict_unknown_keys = true;
};

/**
 * @brief Figure + axes spec parsed from a YAML declarative file.
 */
struct YamlFigureSpec {
  FigureSpec figure;
  std::vector<AxesSpec> axes;
};

/**
 * @brief Load a declarative YAML subset into figure/axes specs.
 * @param path YAML file path.
 * @param options Loader behavior controls.
 * @return Parsed specs.
 */
YamlFigureSpec load_yaml_figure_spec(const std::filesystem::path& path,
                                     const YamlLoadOptions& options = {});

}  // namespace gnuplotpp
