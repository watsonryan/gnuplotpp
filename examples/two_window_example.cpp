#include "gnuplotpp/gnuplot_backend.hpp"
#include "gnuplotpp/logging.hpp"
#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"
#include <filesystem>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  using namespace gnuplotpp;

  std::filesystem::path out_dir = "out/two_window_example";
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--out" && i + 1 < argc) {
      out_dir = argv[++i];
    }
  }

  FigureSpec fs;
  fs.preset = Preset::IEEE_DoubleColumn;
  apply_preset_defaults(fs);
  fs.rows = 1;
  fs.cols = 2;
  fs.formats = {OutputFormat::Pdf, OutputFormat::Svg, OutputFormat::Eps};
  fs.title = "Dynamic Compare";

  Figure fig(fs);

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

  std::vector<double> t;
  std::vector<double> ep;
  std::vector<double> ev;
  for (int i = 0; i < 200; ++i) {
    const double x = static_cast<double>(i) * 0.1;
    t.push_back(x);
    ep.push_back(2.0 / (1.0 + x));
    ev.push_back(0.5 / (1.0 + 0.4 * x));
  }

  fig.axes(0, 0).add_series(SeriesSpec{.type = SeriesType::Line, .label = "SRIF"}, t, ep);
  fig.axes(0, 1).add_series(SeriesSpec{.type = SeriesType::Line, .label = "SRIF"}, t, ev);

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
