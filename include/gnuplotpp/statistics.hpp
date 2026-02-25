#pragma once

#include <span>
#include <vector>

namespace gnuplotpp {

/**
 * @brief Evaluate Gaussian KDE on a provided grid.
 * @param samples Input sample values.
 * @param x_grid Query points where density is evaluated.
 * @param bandwidth Kernel bandwidth; when <=0, Silverman's rule is used.
 * @return Density values aligned with x_grid.
 */
std::vector<double> gaussian_kde(std::span<const double> samples,
                                 std::span<const double> x_grid,
                                 double bandwidth = -1.0);

}  // namespace gnuplotpp
