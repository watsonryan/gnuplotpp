#include "gnuplotpp/gnuplot_backend.hpp"
#include "gnuplotpp/logging.hpp"
#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"
#include <filesystem>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  using namespace gnuplotpp;

  std::filesystem::path out_dir = "out/layout_2x2_example";
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--out" && i + 1 < argc) {
      out_dir = argv[++i];
    }
  }

  FigureSpec fs;
  fs.preset = Preset::AIAA_Page;
  apply_preset_defaults(fs);
  fs.rows = 2;
  fs.cols = 2;
  fs.formats = {OutputFormat::Pdf, OutputFormat::Svg, OutputFormat::Eps};
  fs.title = "State Error Overview";

  Figure fig(fs);

  const std::vector<std::string> ylabels{
      "e_x [m]", "e_y [m]", "e_z [m]", "||e_v|| [m/s]"};
  const std::vector<std::string> titles{
      "X Position", "Y Position", "Z Position", "Velocity Norm"};

  for (int idx = 0; idx < 4; ++idx) {
    AxesSpec ax;
    ax.title = titles[idx];
    ax.xlabel = "t [s]";
    ax.ylabel = ylabels[idx];
    ax.grid = true;
    fig.axes(idx).set(ax);
  }

  std::vector<double> t;
  std::vector<double> ex;
  std::vector<double> ey;
  std::vector<double> ez;
  std::vector<double> ev;

  for (int i = 0; i < 250; ++i) {
    const double x = static_cast<double>(i) * 0.2;
    t.push_back(x);
    ex.push_back(8.0 / (1.0 + 0.05 * x));
    ey.push_back(6.0 / (1.0 + 0.06 * x));
    ez.push_back(5.0 / (1.0 + 0.08 * x));
    ev.push_back(0.8 / (1.0 + 0.03 * x));
  }

  fig.axes(0).add_series(SeriesSpec{.type = SeriesType::Line, .label = "SRIF"}, t, ex);
  fig.axes(1).add_series(SeriesSpec{.type = SeriesType::Line, .label = "SRIF"}, t, ey);
  fig.axes(2).add_series(SeriesSpec{.type = SeriesType::Line, .label = "SRIF"}, t, ez);
  fig.axes(3).add_series(SeriesSpec{.type = SeriesType::Line, .label = "SRIF"}, t, ev);

  fig.set_backend(make_gnuplot_backend());
  const auto result = fig.save(out_dir / "figures");

  if (!result.ok) {
    gnuplotpp::log::Error("plot render incomplete: ", result.message);
    gnuplotpp::log::Error("install gnuplot and rerun this example");
    gnuplotpp::log::Error("script: ", result.script_path.string());
    return 1;
  }

  for (const auto& output : result.outputs) {
    gnuplotpp::log::Info("output: ", output.string());
  }

  return 0;
}
