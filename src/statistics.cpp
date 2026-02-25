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

}  // namespace gnuplotpp
