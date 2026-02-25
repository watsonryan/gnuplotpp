#include "gnuplotpp/facet.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

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

void apply_panel_titles(Figure& fig, const std::vector<std::string>& titles) {
  const auto& spec = fig.spec();
  const int total = spec.rows * spec.cols;
  for (int i = 0; i < total && i < static_cast<int>(titles.size()); ++i) {
    auto ax = fig.axes(i).spec();
    ax.title = titles[static_cast<std::size_t>(i)];
    fig.axes(i).set(ax);
  }
}

void apply_shared_legend(Figure& fig, const LegendSpec& legend, const int anchor_axes_index) {
  const auto& spec = fig.spec();
  const int total = spec.rows * spec.cols;
  if (anchor_axes_index < 0 || anchor_axes_index >= total) {
    throw std::out_of_range("anchor_axes_index out of range");
  }
  for (int i = 0; i < total; ++i) {
    auto ax = fig.axes(i).spec();
    if (i == anchor_axes_index) {
      ax.legend = legend.enabled;
      ax.legend_spec = legend;
    } else {
      ax.legend = false;
    }
    fig.axes(i).set(ax);
  }
}

void apply_shared_colorbar_label(Figure& fig, const std::string& label, int owner_axes_index) {
  const auto& spec = fig.spec();
  const int total = spec.rows * spec.cols;
  if (owner_axes_index < 0) {
    owner_axes_index = total - 1;
  }
  if (owner_axes_index < 0 || owner_axes_index >= total) {
    throw std::out_of_range("owner_axes_index out of range");
  }
  for (int i = 0; i < total; ++i) {
    auto ax = fig.axes(i).spec();
    ax.colorbar_label = (i == owner_axes_index) ? label : "";
    fig.axes(i).set(ax);
  }
}

LegendPosition auto_legend_position(std::span<const double> x, std::span<const double> y) {
  if (x.empty() || y.empty() || x.size() != y.size()) {
    return LegendPosition::TopRight;
  }
  double xmin = x.front();
  double xmax = x.front();
  double ymin = y.front();
  double ymax = y.front();
  for (std::size_t i = 1; i < x.size(); ++i) {
    xmin = std::min(xmin, x[i]);
    xmax = std::max(xmax, x[i]);
    ymin = std::min(ymin, y[i]);
    ymax = std::max(ymax, y[i]);
  }
  const double xspan = std::max(1e-12, xmax - xmin);
  const double yspan = std::max(1e-12, ymax - ymin);

  struct Candidate {
    LegendPosition pos;
    double cx;
    double cy;
  };
  const Candidate corners[] = {
      {LegendPosition::TopLeft, 0.1, 0.9},
      {LegendPosition::TopRight, 0.9, 0.9},
      {LegendPosition::BottomLeft, 0.1, 0.1},
      {LegendPosition::BottomRight, 0.9, 0.1},
  };

  double best_score = std::numeric_limits<double>::infinity();
  LegendPosition best = LegendPosition::TopRight;
  for (const auto& c : corners) {
    double score = 0.0;
    for (std::size_t i = 0; i < x.size(); ++i) {
      const double xn = (x[i] - xmin) / xspan;
      const double yn = (y[i] - ymin) / yspan;
      const double dx = xn - c.cx;
      const double dy = yn - c.cy;
      const double d2 = dx * dx + dy * dy;
      score += 1.0 / (1e-3 + d2);
    }
    if (score < best_score) {
      best_score = score;
      best = c.pos;
    }
  }
  return best;
}

void auto_place_legend(AxesSpec& ax, std::span<const double> x, std::span<const double> y) {
  ax.legend = true;
  ax.legend_spec.position = auto_legend_position(x, y);
}

}  // namespace gnuplotpp
