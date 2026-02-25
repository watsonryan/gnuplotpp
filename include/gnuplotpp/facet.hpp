#pragma once

#include "gnuplotpp/plot.hpp"

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

}  // namespace gnuplotpp
