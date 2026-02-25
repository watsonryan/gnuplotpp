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
 * @brief Build multiple percentile ribbons for fan-chart visualization.
 * @param ensemble Row-major series vectors with equal length.
 * @param quantiles Ascending quantiles in (0,1), e.g. {0.1,0.25,0.75,0.9}.
 * @param lows Output low bands matching each ribbon.
 * @param highs Output high bands matching each ribbon.
 */
void fan_chart_bands(const std::vector<std::vector<double>>& ensemble,
                     const std::vector<double>& quantiles,
                     std::vector<std::vector<double>>& lows,
                     std::vector<std::vector<double>>& highs);

/**
 * @brief Approximate violin density profile.
 * @param samples Input values.
 * @param y_grid Output y positions.
 * @param half_width Output normalized half-width density [0,1].
 * @param points Number of y-grid points.
 */
void violin_profile(std::span<const double> samples,
                    std::vector<double>& y_grid,
                    std::vector<double>& half_width,
                    std::size_t points = 120);

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

/**
 * @brief Compute Q-Q plot points against a normal distribution.
 * @param samples Input samples.
 * @param theo Quantile positions of reference normal.
 * @param samp Sorted sample quantiles.
 */
void qq_plot_normal(std::span<const double> samples,
                    std::vector<double>& theo,
                    std::vector<double>& samp);

/**
 * @brief Five-number summary for boxplot style rendering.
 */
struct BoxSummary {
  double q1 = 0.0;
  double median = 0.0;
  double q3 = 0.0;
  double whisker_low = 0.0;
  double whisker_high = 0.0;
};

/**
 * @brief Compute boxplot summary using Tukey 1.5*IQR whiskers.
 * @param samples Input data.
 * @return Summary statistics.
 */
BoxSummary box_summary(std::span<const double> samples);

/**
 * @brief 2D confidence ellipse points from covariance-like samples.
 * @param x Samples for x.
 * @param y Samples for y.
 * @param nsigma Sigma scale (e.g., 1,2,3).
 * @param x_ellipse Output x points.
 * @param y_ellipse Output y points.
 * @param points Number of points.
 */
void confidence_ellipse(std::span<const double> x,
                        std::span<const double> y,
                        double nsigma,
                        std::vector<double>& x_ellipse,
                        std::vector<double>& y_ellipse,
                        std::size_t points = 200);

}  // namespace gnuplotpp
