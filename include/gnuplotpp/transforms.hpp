#pragma once

#include <span>
#include <vector>

namespace gnuplotpp {

/** @brief Rolling mean transform. */
std::vector<double> transform_rolling_mean(std::span<const double> y, std::size_t window);

/** @brief Z-score normalization transform. */
std::vector<double> transform_zscore(std::span<const double> y);

/** @brief Clamp values into [vmin, vmax]. */
std::vector<double> transform_clip(std::span<const double> y, double vmin, double vmax);

/**
 * @brief Simple transform pipeline for 1D vectors.
 */
class TransformPipeline {
public:
  TransformPipeline& set_input(std::vector<double> input);
  TransformPipeline& rolling_mean(std::size_t window);
  TransformPipeline& zscore();
  TransformPipeline& clip(double vmin, double vmax);
  const std::vector<double>& values() const noexcept;

private:
  std::vector<double> data_{};
};

}  // namespace gnuplotpp
