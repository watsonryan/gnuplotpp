#include "gnuplotpp/model_overlays.hpp"

namespace gnuplotpp {

LinearFitResult add_linear_fit_overlay(Axes& ax,
                                       std::span<const double> x,
                                       std::span<const double> y,
                                       const std::string& label) {
  const auto fit = linear_fit(x, y);
  const auto yhat = linear_fit_line(fit, x);
  SeriesSpec s;
  s.label = label;
  s.has_line_width = true;
  s.line_width_pt = 1.6;
  s.has_color = true;
  s.color = "#d62728";
  ax.add_series(s, x, yhat);
  return fit;
}

}  // namespace gnuplotpp
