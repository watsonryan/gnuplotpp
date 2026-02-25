#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"
#include "gnuplotpp/svg_backend.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

std::string read_file(const std::filesystem::path& path) {
  std::ifstream in(path);
  return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

}  // namespace

int main() {
  using namespace gnuplotpp;

  FigureSpec spec;
  spec.preset = Preset::IEEE_DoubleColumn;
  apply_preset_defaults(spec);
  spec.rows = 1;
  spec.cols = 2;
  spec.formats = {OutputFormat::Svg};
  spec.title = "Native SVG";

  Figure fig(spec);

  AxesSpec ax;
  ax.xlabel = "t [s]";
  ax.ylabel = "e";
  ax.grid = true;
  fig.axes(0).set(ax);
  fig.axes(1).set(ax);

  const std::vector<double> t{0.0, 1.0, 2.0};
  const std::vector<double> y0{2.0, 1.0, 0.5};
  const std::vector<double> y1{1.0, 1.2, 1.4};

  fig.axes(0).add_series(SeriesSpec{.label = "A"}, t, y0);
  fig.axes(1).add_series(SeriesSpec{.label = "B"}, t, y1);

  const auto out_dir = std::filesystem::temp_directory_path() / "gnuplotpp_svg_backend_test";
  std::filesystem::remove_all(out_dir);

  fig.set_backend(make_svg_backend());
  const auto result = fig.save(out_dir);

  assert(result.ok);
  assert(result.status == RenderStatus::Success);
  assert(!result.outputs.empty());
  assert(std::filesystem::exists(result.outputs.front()));

  const auto svg = read_file(result.outputs.front());
  assert(svg.find("<svg") != std::string::npos);
  assert(svg.find("<polyline") != std::string::npos);
  assert(svg.find("Native SVG") != std::string::npos);

  std::filesystem::remove_all(out_dir);
  return 0;
}
