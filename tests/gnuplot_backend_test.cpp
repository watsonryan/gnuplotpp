#include "gnuplotpp/gnuplot_backend.hpp"
#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"

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
  spec.formats = {OutputFormat::Pdf};
  spec.export_policy.drop_line_alpha_for_vector = false;

  Figure fig(spec);

  AxesSpec ax0;
  ax0.title = "Position Error";
  ax0.xlabel = "t [s]";
  ax0.ylabel = "||e_p|| [m]";
  ax0.grid = true;

  AxesSpec ax1 = ax0;
  ax1.title = "Velocity Error";
  ax1.ylabel = "||e_v|| [m/s]";

  fig.axes(0, 0).set(ax0);
  fig.axes(0, 1).set(ax1);

  const std::vector<double> t{0.0, 1.0, 2.0};
  const std::vector<double> ep{2.0, 1.0, 0.5};
  const std::vector<double> ev{0.4, 0.3, 0.2};
  const std::vector<double> ep_mc{1.8, 1.1, 0.6};

  fig.axes(0, 0).add_series(SeriesSpec{.label = "SRIF"}, t, ep);
  fig.axes(0, 0).add_series(SeriesSpec{.label = "MC",
                                       .has_line_width = true,
                                       .line_width_pt = 0.6,
                                       .has_color = true,
                                       .color = "#000000",
                                       .has_opacity = true,
                                       .opacity = 0.25},
                            t,
                            ep_mc);
  fig.axes(0, 1).add_series(SeriesSpec{.label = "SRIF"}, t, ev);

  const auto out_dir = std::filesystem::temp_directory_path() / "gnuplotpp_backend_test";
  std::filesystem::remove_all(out_dir);

  fig.set_backend(make_gnuplot_backend("__gnuplot_missing__"));
  const auto result = fig.save(out_dir);

  assert(!result.ok);
  assert(result.status == RenderStatus::ExternalToolMissing);
  assert(std::filesystem::exists(result.script_path));

  const auto script = read_file(result.script_path);
  assert(script.find("set multiplot layout 1,2") != std::string::npos);
  assert(script.find("set terminal pdfcairo") != std::string::npos);
  assert(script.find("set monochrome") == std::string::npos);
  assert(script.find("dt 1") != std::string::npos);
  assert(script.find("lc rgb '#bf000000'") != std::string::npos);
  assert(script.find("plot") != std::string::npos);

  const auto data0 = out_dir / "tmp" / "ax0_series0.dat";
  const auto data_text = read_file(data0);
  assert(data_text.find("0.0000000000000000e+00") != std::string::npos);
  assert(data_text.find("2.0000000000000000e+00") != std::string::npos);

  std::filesystem::remove_all(out_dir);
  return 0;
}
