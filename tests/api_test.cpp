#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <vector>

namespace {

class DummyBackend final : public gnuplotpp::IPlotBackend {
public:
  gnuplotpp::RenderResult render(const gnuplotpp::Figure&, const std::filesystem::path&) override {
    return gnuplotpp::RenderResult{
        .ok = true, .status = gnuplotpp::RenderStatus::Success, .message = "ok"};
  }
};

}  // namespace

int main() {
  using namespace gnuplotpp;

  FigureSpec spec;
  spec.preset = Preset::IEEE_DoubleColumn;
  apply_preset_defaults(spec);
  spec.rows = 1;
  spec.cols = 2;

  assert(spec.size.w == 7.16);
  assert(spec.style.font_pt == 8.5);

  Figure fig(spec);
  AxesSpec ax;
  ax.title = "test";
  fig.axes(0, 0).set(ax);

  const std::vector<double> x{0.0, 1.0, 2.0};
  const std::vector<double> y{1.0, 2.0, 3.0};
  fig.axes(0, 0).add_series(SeriesSpec{.label = "line"}, x, y);
  bool threw = false;
  try {
    fig.axes(0, 0).add_series(SeriesSpec{.label = "bad", .has_opacity = true, .opacity = 1.2},
                              x,
                              y);
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  assert(threw);

  const std::vector<double> y_lo{0.8, 1.6, 2.4};
  const std::vector<double> y_hi{1.2, 2.4, 3.6};
  fig.axes(0, 0).add_band(SeriesSpec{.label = "band"}, x, y_lo, y_hi);
  fig.axes(0, 0).add_histogram(SeriesSpec{.label = "hist"}, x, y);
  fig.axes(0, 0).add_heatmap(SeriesSpec{.label = "hm"}, x, y, y);

  assert(fig.axes(0, 0).series().size() == 4U);
  assert(fig.axes(0, 0).series()[1].spec.type == SeriesType::Band);
  assert(fig.axes(0, 0).series()[2].spec.type == SeriesType::Histogram);
  assert(fig.axes(0, 0).series()[3].spec.type == SeriesType::Heatmap);

  apply_style_profile(spec, StyleProfile::Presentation);
  assert(spec.palette == ColorPalette::Viridis);
  assert(spec.style.font_pt >= 12.0);

  const auto no_backend = fig.save("out");
  assert(!no_backend.ok);
  assert(no_backend.status == RenderStatus::InvalidInput);

  fig.set_backend(std::make_shared<DummyBackend>());
  const auto ok = fig.save("out");
  assert(ok.ok);
  assert(ok.status == RenderStatus::Success);

  return 0;
}
