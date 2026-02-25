#include "gnuplotpp/transforms.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace gnuplotpp {

std::vector<double> transform_rolling_mean(std::span<const double> y, const std::size_t window) {
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

std::vector<double> transform_zscore(std::span<const double> y) {
  if (y.empty()) {
    return {};
  }
  const double mean = std::accumulate(y.begin(), y.end(), 0.0) / static_cast<double>(y.size());
  double var = 0.0;
  for (const double v : y) {
    const double d = v - mean;
    var += d * d;
  }
  var /= static_cast<double>(std::max<std::size_t>(1, y.size() - 1));
  const double sigma = std::sqrt(std::max(1e-16, var));
  std::vector<double> out(y.size(), 0.0);
  for (std::size_t i = 0; i < y.size(); ++i) {
    out[i] = (y[i] - mean) / sigma;
  }
  return out;
}

std::vector<double> transform_clip(std::span<const double> y, const double vmin, const double vmax) {
  if (y.empty()) {
    return {};
  }
  std::vector<double> out(y.begin(), y.end());
  for (double& v : out) {
    v = std::clamp(v, vmin, vmax);
  }
  return out;
}

TransformPipeline& TransformPipeline::set_input(std::vector<double> input) {
  data_ = std::move(input);
  return *this;
}

TransformPipeline& TransformPipeline::rolling_mean(const std::size_t window) {
  data_ = transform_rolling_mean(data_, window);
  return *this;
}

TransformPipeline& TransformPipeline::zscore() {
  data_ = transform_zscore(data_);
  return *this;
}

TransformPipeline& TransformPipeline::clip(const double vmin, const double vmax) {
  data_ = transform_clip(data_, vmin, vmax);
  return *this;
}

const std::vector<double>& TransformPipeline::values() const noexcept { return data_; }

}  // namespace gnuplotpp
