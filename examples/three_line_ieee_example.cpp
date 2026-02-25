#include "gnuplotpp/gnuplot_backend.hpp"
#include "gnuplotpp/logging.hpp"
#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"

#include <filesystem>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  using namespace gnuplotpp;

  std::filesystem::path out_dir = "out/three_line_ieee_example";
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--out" && i + 1 < argc) {
      out_dir = argv[++i];
    }
  }

  FigureSpec fs;
  fs.preset = Preset::IEEE_SingleColumn;
  apply_preset_defaults(fs);
  fs.rows = 1;
  fs.cols = 1;
  fs.formats = {OutputFormat::Pdf, OutputFormat::Svg, OutputFormat::Eps};
  fs.title = "Three-Method Comparison";

  Figure fig(fs);

  AxesSpec ax;
  ax.title = "Position Error Norm";
  ax.xlabel = "t [s]";
  ax.ylabel = "||e_p|| [m]";
  ax.legend = true;
  ax.grid = true;
  ax.ylog = true;
  fig.axes(0).set(ax);

  std::vector<double> t;
  std::vector<double> srif;
  std::vector<double> ukf;
  std::vector<double> ekf;

  for (int i = 0; i < 200; ++i) {
    const double x = static_cast<double>(i) * 0.5;
    t.push_back(x);
    srif.push_back(12.0 / (1.0 + 0.08 * x));
    ukf.push_back(10.0 / (1.0 + 0.06 * x));
    ekf.push_back(11.0 / (1.0 + 0.07 * x));
  }

  fig.axes(0).add_series(SeriesSpec{.type = SeriesType::Line, .label = "SRIF"}, t, srif);
  fig.axes(0).add_series(SeriesSpec{.type = SeriesType::Line, .label = "UKF"}, t, ukf);
  fig.axes(0).add_series(SeriesSpec{.type = SeriesType::Line, .label = "EKF"}, t, ekf);

  fig.set_backend(make_gnuplot_backend());
  const auto result = fig.save(out_dir / "figures");
  if (!result.ok) {
    gnuplotpp::log::Error("plot render incomplete: ", result.message);
    gnuplotpp::log::Error("script: ", result.script_path.string());
    return 1;
  }

  for (const auto& output : result.outputs) {
    gnuplotpp::log::Info("output: ", output.string());
  }

  return 0;
}
