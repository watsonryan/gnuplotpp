#include "gnuplotpp/gnuplot_backend.hpp"
#include "gnuplotpp/logging.hpp"
#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"

#include <algorithm>
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
  fs.formats = {OutputFormat::Pdf, OutputFormat::Svg, OutputFormat::Eps, OutputFormat::Png};
  fs.title = "Three-Method Comparison";
  fs.style.line_width_pt = 1.5;

  Figure fig(fs);

  AxesSpec ax;
  ax.title = "Position Error Norm";
  ax.xlabel = "t [s]";
  ax.ylabel = "||e_p|| [m]";
  ax.legend = true;
  ax.grid = true;
  ax.ylog = true;

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

  double y_max_data = 0.0;
  for (const double v : srif) {
    y_max_data = std::max(y_max_data, v);
  }
  for (const double v : ukf) {
    y_max_data = std::max(y_max_data, v);
  }
  for (const double v : ekf) {
    y_max_data = std::max(y_max_data, v);
  }
  const double y_max_plot = y_max_data * 1.05;
  const double y_min_plot = std::max(1.0e-3, srif.back() * 0.8);
  const std::size_t idx_target = 80;  // x = 40 s, step = 0.5 s
  const double x_target = t[idx_target];
  const double y_target = srif[idx_target];

  ax.has_ylim = true;
  ax.ymin = y_min_plot;
  ax.ymax = y_max_plot;
  ax.gnuplot_commands = {
      "set label 1 'e_p(t)=e_0 e^{-{/Symbol l} t}' at 14," + std::to_string(y_max_plot * 0.86) +
          " font 'Times,8' front",
      "set arrow 1 from 18," + std::to_string(y_max_plot * 0.80) + " to " +
          std::to_string(x_target) + "," + std::to_string(y_target) +
          " lw 1.3 lc rgb '#000000' front"};
  fig.axes(0).set(ax);

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
