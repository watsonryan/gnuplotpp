#pragma once

#include "gnuplotpp/plot.hpp"

#include <span>
#include <string>
#include <utility>
#include <vector>

namespace gnuplotpp {

/**
 * @brief Compute near-square facet layout for n panels.
 * @param n Number of panels.
 * @return {rows, cols}
 */
std::pair<int, int> facet_grid(int n);

/**
 * @brief Apply facet titles/spec to each axes in row-major order.
 * @param fig Figure with pre-sized layout.
 * @param base Base axes specification.
 * @param titles Per-panel titles.
 */
void apply_facet_axes(Figure& fig, const AxesSpec& base, const std::vector<std::string>& titles);

/**
 * @brief Apply titles to existing axes in row-major order.
 * @param fig Figure with configured layout.
 * @param titles Per-panel titles.
 */
void apply_panel_titles(Figure& fig, const std::vector<std::string>& titles);

/**
 * @brief Configure one shared legend location and disable legends in remaining panels.
 * @param fig Figure with configured layout.
 * @param legend Shared legend specification.
 * @param anchor_axes_index Axes index that owns the legend.
 */
void apply_shared_legend(Figure& fig, const LegendSpec& legend, int anchor_axes_index = 0);

/**
 * @brief Keep one shared colorbar label on a designated axes and clear others.
 * @param fig Figure with configured layout.
 * @param label Shared colorbar label.
 * @param owner_axes_index Axes index that owns the colorbar label.
 */
void apply_shared_colorbar_label(Figure& fig,
                                 const std::string& label,
                                 int owner_axes_index = -1);

/**
 * @brief Estimate a low-overlap legend corner from point density.
 * @param x X samples.
 * @param y Y samples.
 * @return Suggested legend corner.
 */
LegendPosition auto_legend_position(std::span<const double> x, std::span<const double> y);

/**
 * @brief Apply heuristic legend placement to an axes spec.
 * @param ax Axes spec to modify.
 * @param x X samples.
 * @param y Y samples.
 */
void auto_place_legend(AxesSpec& ax, std::span<const double> x, std::span<const double> y);

/**
 * @brief Apply shared-range/tick defaults for small multiples.
 * @param fig Figure to update.
 * @param sync_x_limits Enable global x-range across panels.
 * @param sync_y_limits Enable global y-range across panels.
 */
void apply_small_multiples_defaults(Figure& fig,
                                    bool sync_x_limits = true,
                                    bool sync_y_limits = true);

}  // namespace gnuplotpp
