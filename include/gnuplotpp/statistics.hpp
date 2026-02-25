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

/**
 * @brief Compute empirical CDF points (x sorted, p in [0,1]).
 * @param samples Input values.
 * @param x_sorted Output sorted x values.
 * @param p Output cumulative probability values.
 */
void ecdf(std::span<const double> samples,
          std::vector<double>& x_sorted,
          std::vector<double>& p);

/**
 * @brief Compute percentile band across an ensemble matrix.
 * @param ensemble Row-major series vectors with equal length.
 * @param p_low Lower percentile in [0,1].
 * @param p_high Upper percentile in [0,1].
 * @param low Output lower band.
 * @param high Output upper band.
 */
void percentile_band(const std::vector<std::vector<double>>& ensemble,
                     double p_low,
                     double p_high,
                     std::vector<double>& low,
                     std::vector<double>& high);

/**
 * @brief Simple moving average.
 * @param y Input signal.
 * @param window Window length.
 * @return Smoothed signal.
 */
std::vector<double> moving_average(std::span<const double> y, std::size_t window);

/**
 * @brief Uniform downsample by keeping every k-th point.
 * @param y Input signal.
 * @param k Stride (>=1).
 * @return Downsampled signal.
 */
std::vector<double> downsample_uniform(std::span<const double> y, std::size_t k);

/**
 * @brief Auto-correlation for lags [0, max_lag].
 * @param y Input signal.
 * @param max_lag Maximum lag.
 * @return Correlation values.
 */
std::vector<double> autocorrelation(std::span<const double> y, std::size_t max_lag);

}  // namespace gnuplotpp
