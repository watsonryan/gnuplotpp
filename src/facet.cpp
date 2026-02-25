#include "gnuplotpp/facet.hpp"

#include <cmath>

namespace gnuplotpp {

std::pair<int, int> facet_grid(const int n) {
  if (n <= 0) {
    return {1, 1};
  }
  const int cols = static_cast<int>(std::ceil(std::sqrt(static_cast<double>(n))));
  const int rows = static_cast<int>(std::ceil(static_cast<double>(n) / static_cast<double>(cols)));
  return {rows, cols};
}

void apply_facet_axes(Figure& fig, const AxesSpec& base, const std::vector<std::string>& titles) {
  const auto& spec = fig.spec();
  const int total = spec.rows * spec.cols;
  for (int i = 0; i < total; ++i) {
    AxesSpec ax = base;
    if (i < static_cast<int>(titles.size())) {
      ax.title = titles[static_cast<std::size_t>(i)];
    }
    fig.axes(i).set(ax);
  }
}

}  // namespace gnuplotpp
