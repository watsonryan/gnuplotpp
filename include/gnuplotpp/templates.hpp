#pragma once

#include "gnuplotpp/plot.hpp"

#include <filesystem>

namespace gnuplotpp {

/** @brief Built-in style/data templates for quick-start plotting. */
enum class PlotTemplate {
  EstimationBand,
  DistributionKDE,
  HeatmapField,
  DualAxisProbability
};

/**
 * @brief Apply a template style to figure+axes defaults.
 * @param spec Figure spec to modify.
 * @param ax Axes spec to modify.
 * @param t Template selector.
 */
void apply_plot_template(FigureSpec& spec, AxesSpec& ax, PlotTemplate t);

/**
 * @brief Write starter YAML template files into a folder.
 * @param dir Output directory.
 */
void write_template_gallery_yaml(const std::filesystem::path& dir);

}  // namespace gnuplotpp
