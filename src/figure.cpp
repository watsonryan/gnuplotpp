#include "gnuplotpp/plot.hpp"

#include <stdexcept>

namespace gnuplotpp {

void Axes::set(AxesSpec spec) { spec_ = std::move(spec); }

void Axes::add_series(const SeriesSpec& spec,
                      std::span<const double> x,
                      std::span<const double> y) {
  if (x.size() != y.size()) {
    throw std::invalid_argument("x and y must have the same length");
  }

  SeriesData data;
  data.spec = spec;
  data.x.assign(x.begin(), x.end());
  data.y.assign(y.begin(), y.end());
  series_.push_back(std::move(data));
}

Figure::Figure(FigureSpec spec) : spec_(std::move(spec)) {
  if (spec_.rows <= 0 || spec_.cols <= 0) {
    throw std::invalid_argument("rows and cols must be positive");
  }
  axes_.resize(static_cast<std::size_t>(spec_.rows * spec_.cols));
}

Axes& Figure::axes(int r, int c) {
  if (r < 0 || c < 0 || r >= spec_.rows || c >= spec_.cols) {
    throw std::out_of_range("axes row/col out of range");
  }
  return axes_[static_cast<std::size_t>(r * spec_.cols + c)];
}

Axes& Figure::axes(int idx) {
  if (idx < 0 || idx >= static_cast<int>(axes_.size())) {
    throw std::out_of_range("axes index out of range");
  }
  return axes_[static_cast<std::size_t>(idx)];
}

RenderResult Figure::save(const std::filesystem::path& out_dir) const {
  if (!backend_) {
    return RenderResult{.ok = false,
                        .message =
                            "No backend configured. Call set_backend() before save()."};
  }
  return backend_->render(*this, out_dir);
}

void Figure::set_backend(std::shared_ptr<IPlotBackend> backend) {
  backend_ = std::move(backend);
}

}  // namespace gnuplotpp
