#pragma once

#include "gnuplotpp/presets.hpp"
#include "gnuplotpp/plot.hpp"

#include <string>
#include <utility>
#include <vector>

namespace gnuplotpp {

/**
 * @brief High-level options for "quick and consistent" figure setup.
 */
struct QuickFigureOptions {
  Preset preset = Preset::Custom;
  StyleProfile profile = StyleProfile::Tufte_Minimal;
  FigureSizeInches size{.w = 5.6, .h = 3.4};
  std::vector<OutputFormat> formats{OutputFormat::Pdf, OutputFormat::Svg, OutputFormat::Png};
  std::string title;
  int rows = 1;
  int cols = 1;
};

/**
 * @brief Build a figure spec with preset + style profile already applied.
 * @param options Quick figure options.
 * @return Ready-to-use figure spec.
 */
inline FigureSpec make_quick_figure_spec(const QuickFigureOptions& options = {}) {
  FigureSpec spec;
  spec.preset = options.preset;
  apply_preset_defaults(spec);
  apply_style_profile(spec, options.profile);
  spec.size = options.size;
  spec.formats = options.formats;
  spec.title = options.title;
  spec.rows = options.rows;
  spec.cols = options.cols;
  spec.auto_layout = true;
  return spec;
}

/**
 * @brief Build a figure directly from quick options.
 * @param options Quick figure options.
 * @return Figure constructed from quick spec.
 */
inline Figure make_quick_figure(const QuickFigureOptions& options = {}) {
  return Figure(make_quick_figure_spec(options));
}

/**
 * @brief Build a clean axes spec with optional title/labels and sensible defaults.
 * @param title Axes title.
 * @param xlabel X label.
 * @param ylabel Y label.
 * @param grid Enable major grid.
 * @param legend Enable legend.
 * @return Configured AxesSpec.
 */
inline AxesSpec make_quick_axes(std::string title = {},
                                std::string xlabel = {},
                                std::string ylabel = {},
                                bool grid = false,
                                bool legend = true) {
  AxesSpec ax;
  ax.title = std::move(title);
  ax.xlabel = std::move(xlabel);
  ax.ylabel = std::move(ylabel);
  ax.grid = grid;
  ax.legend = legend;
  return ax;
}

}  // namespace gnuplotpp
