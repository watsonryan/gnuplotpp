#pragma once

#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/statistics.hpp"

#include <span>
#include <string>

namespace gnuplotpp {

/**
 * @brief Add a linear fit overlay line and return fit summary.
 * @param ax Target axes.
 * @param x X samples.
 * @param y Y samples.
 * @param label Series label for fit line.
 * @return Fit summary.
 */
LinearFitResult add_linear_fit_overlay(Axes& ax,
                                       std::span<const double> x,
                                       std::span<const double> y,
                                       const std::string& label = "linear fit");

/**
 * @brief Add a polynomial fit overlay line and return fit summary.
 * @param ax Target axes.
 * @param x X samples.
 * @param y Y samples.
 * @param degree Polynomial degree (1=linear).
 * @param label Series label for fit line.
 * @return Polynomial fit summary.
 */
PolynomialFitResult add_polynomial_fit_overlay(Axes& ax,
                                               std::span<const double> x,
                                               std::span<const double> y,
                                               std::size_t degree,
                                               const std::string& label = "poly fit");

}  // namespace gnuplotpp
