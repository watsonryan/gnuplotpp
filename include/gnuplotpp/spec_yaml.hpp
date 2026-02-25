#pragma once

#include "gnuplotpp/plot.hpp"

#include <filesystem>
#include <vector>

namespace gnuplotpp {

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
 * @return Parsed specs.
 */
YamlFigureSpec load_yaml_figure_spec(const std::filesystem::path& path);

}  // namespace gnuplotpp
