#include "gnuplotpp/statistics.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace gnuplotpp {

std::vector<double> gaussian_kde(std::span<const double> samples,
                                 std::span<const double> x_grid,
                                 double bandwidth) {
  if (samples.empty() || x_grid.empty()) {
    return {};
  }

  const std::size_t n = samples.size();
  const double mean = std::accumulate(samples.begin(), samples.end(), 0.0) / static_cast<double>(n);
  double var = 0.0;
  for (const double v : samples) {
    const double d = v - mean;
    var += d * d;
  }
  var /= std::max<std::size_t>(1, n - 1);
  const double sigma = std::sqrt(std::max(1.0e-16, var));

  double h = bandwidth;
  if (h <= 0.0) {
    h = 1.06 * sigma * std::pow(static_cast<double>(n), -0.2);
  }
  h = std::max(1.0e-9, h);

  constexpr double inv_sqrt_2pi = 0.3989422804014327;
  const double norm = 1.0 / (static_cast<double>(n) * h);
  std::vector<double> density(x_grid.size(), 0.0);

  for (std::size_t i = 0; i < x_grid.size(); ++i) {
    const double x = x_grid[i];
    double acc = 0.0;
    for (const double s : samples) {
      const double u = (x - s) / h;
      acc += inv_sqrt_2pi * std::exp(-0.5 * u * u);
    }
    density[i] = norm * acc;
  }
  return density;
}

void ecdf(std::span<const double> samples, std::vector<double>& x_sorted, std::vector<double>& p) {
  x_sorted.assign(samples.begin(), samples.end());
  std::sort(x_sorted.begin(), x_sorted.end());
  p.resize(x_sorted.size());
  const double n = static_cast<double>(x_sorted.size());
  for (std::size_t i = 0; i < x_sorted.size(); ++i) {
    p[i] = (static_cast<double>(i) + 1.0) / n;
  }
}

void percentile_band(const std::vector<std::vector<double>>& ensemble,
                     double p_low,
                     double p_high,
                     std::vector<double>& low,
                     std::vector<double>& high) {
  if (ensemble.empty()) {
    low.clear();
    high.clear();
    return;
  }
  const std::size_t n = ensemble.front().size();
  low.resize(n);
  high.resize(n);
  const std::size_t i_low =
      static_cast<std::size_t>(std::clamp(p_low, 0.0, 1.0) * static_cast<double>(ensemble.size() - 1));
  const std::size_t i_high = static_cast<std::size_t>(std::clamp(p_high, 0.0, 1.0) *
                                                      static_cast<double>(ensemble.size() - 1));

  std::vector<double> col(ensemble.size());
  for (std::size_t j = 0; j < n; ++j) {
    for (std::size_t i = 0; i < ensemble.size(); ++i) {
      col[i] = ensemble[i][j];
    }
    std::sort(col.begin(), col.end());
    low[j] = col[i_low];
    high[j] = col[i_high];
  }
}

std::vector<double> moving_average(std::span<const double> y, std::size_t window) {
  if (y.empty() || window == 0) {
    return {};
  }
  std::vector<double> out(y.size(), 0.0);
  double acc = 0.0;
  for (std::size_t i = 0; i < y.size(); ++i) {
    acc += y[i];
    if (i >= window) {
      acc -= y[i - window];
    }
    const std::size_t w = std::min(window, i + 1);
    out[i] = acc / static_cast<double>(w);
  }
  return out;
}

std::vector<double> downsample_uniform(std::span<const double> y, std::size_t k) {
  if (k == 0 || y.empty()) {
    return {};
  }
  std::vector<double> out;
  out.reserve((y.size() + k - 1) / k);
  for (std::size_t i = 0; i < y.size(); i += k) {
    out.push_back(y[i]);
  }
  return out;
}

std::vector<double> autocorrelation(std::span<const double> y, std::size_t max_lag) {
  if (y.empty()) {
    return {};
  }
  const std::size_t n = y.size();
  const std::size_t m = std::min(max_lag, n - 1);
  const double mean = std::accumulate(y.begin(), y.end(), 0.0) / static_cast<double>(n);
  double var = 0.0;
  for (double v : y) {
    const double d = v - mean;
    var += d * d;
  }
  if (var <= 0.0) {
    return std::vector<double>(m + 1, 0.0);
  }

  std::vector<double> ac(m + 1, 0.0);
  for (std::size_t lag = 0; lag <= m; ++lag) {
    double c = 0.0;
    for (std::size_t i = 0; i + lag < n; ++i) {
      c += (y[i] - mean) * (y[i + lag] - mean);
    }
    ac[lag] = c / var;
  }
  return ac;
}

}  // namespace gnuplotpp
