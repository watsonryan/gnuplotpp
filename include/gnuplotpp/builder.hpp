#pragma once

#include "gnuplotpp/plot.hpp"

namespace gnuplotpp {

/**
 * @brief Fluent figure configuration helper.
 */
class FigureBuilder {
public:
  explicit FigureBuilder(FigureSpec spec) : spec_(std::move(spec)) {}

  FigureBuilder& title(std::string t) {
    spec_.title = std::move(t);
    return *this;
  }
  FigureBuilder& layout(int rows, int cols) {
    spec_.rows = rows;
    spec_.cols = cols;
    return *this;
  }
  FigureBuilder& formats(std::vector<OutputFormat> f) {
    spec_.formats = std::move(f);
    return *this;
  }
  FigureBuilder& palette(ColorPalette p) {
    spec_.palette = p;
    return *this;
  }
  FigureBuilder& shared_axes(bool share_x, bool share_y, bool hide_inner_labels = true) {
    spec_.share_x = share_x;
    spec_.share_y = share_y;
    spec_.hide_inner_tick_labels = hide_inner_labels;
    return *this;
  }
  FigureBuilder& manifest(bool enabled) {
    spec_.write_manifest = enabled;
    return *this;
  }

  Figure build() const { return Figure(spec_); }
  const FigureSpec& spec() const { return spec_; }

private:
  FigureSpec spec_{};
};

}  // namespace gnuplotpp
