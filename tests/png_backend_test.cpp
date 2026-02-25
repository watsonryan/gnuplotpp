#include "gnuplotpp/png_backend.hpp"
#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <vector>

int main() {
  using namespace gnuplotpp;

  FigureSpec spec;
  spec.preset = Preset::IEEE_DoubleColumn;
  apply_preset_defaults(spec);
  spec.rows = 1;
  spec.cols = 2;
  spec.formats = {OutputFormat::Png};

  Figure fig(spec);

  AxesSpec ax;
  ax.grid = true;
  fig.axes(0).set(ax);
  fig.axes(1).set(ax);

  const std::vector<double> t{0.0, 1.0, 2.0};
  const std::vector<double> y0{2.0, 1.0, 0.5};
  const std::vector<double> y1{0.4, 0.3, 0.2};
  fig.axes(0).add_series(SeriesSpec{.label = "A"}, t, y0);
  fig.axes(1).add_series(SeriesSpec{.label = "B"}, t, y1);

  const auto out_dir = std::filesystem::temp_directory_path() / "gnuplotpp_png_backend_test";
  std::filesystem::remove_all(out_dir);

  fig.set_backend(make_png_backend());
  const auto result = fig.save(out_dir);

  assert(result.ok);
  assert(result.status == RenderStatus::Success);
  assert(!result.outputs.empty());
  assert(std::filesystem::exists(result.outputs.front()));

  std::ifstream in(result.outputs.front(), std::ios::binary);
  char sig[8]{};
  in.read(sig, 8);
  const unsigned char expected[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
  for (int i = 0; i < 8; ++i) {
    assert(static_cast<unsigned char>(sig[i]) == expected[i]);
  }

  std::filesystem::remove_all(out_dir);
  return 0;
}
