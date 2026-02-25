#include "gnuplotpp/plot.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace {

class DummyBackend final : public gnuplotpp::IPlotBackend {
public:
  gnuplotpp::RenderResult render(const gnuplotpp::Figure&, const std::filesystem::path&) override {
    return gnuplotpp::RenderResult{.ok = true, .message = "ok"};
  }
};

}  // namespace

int main() {
  using namespace gnuplotpp;

  FigureSpec spec;
  spec.rows = 1;
  spec.cols = 2;

  Figure fig(spec);
  AxesSpec ax;
  ax.title = "test";
  fig.axes(0, 0).set(ax);

  const std::vector<double> x{0.0, 1.0, 2.0};
  const std::vector<double> y{1.0, 2.0, 3.0};
  fig.axes(0, 0).add_series(SeriesSpec{.label = "line"}, x, y);

  assert(fig.axes(0, 0).series().size() == 1U);
  assert(!fig.save("out").ok);

  fig.set_backend(std::make_shared<DummyBackend>());
  assert(fig.save("out").ok);

  return 0;
}
