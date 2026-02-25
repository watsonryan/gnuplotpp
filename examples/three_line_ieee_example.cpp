#include "example_common.hpp"
#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  using namespace gnuplotpp;

  const std::filesystem::path out_dir =
      example_common::parse_out_dir(argc, argv, "out/three_line_ieee_example");

  FigureSpec fs;
  fs.preset = Preset::IEEE_SingleColumn;
  apply_preset_defaults(fs);
  fs.rows = 1;
  fs.cols = 1;
  fs.formats = {OutputFormat::Pdf, OutputFormat::Svg, OutputFormat::Eps};
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
    srif.push_back(16.0 / (1.0 + 0.20 * x));
    ukf.push_back(11.0 / (1.0 + 0.080 * x));
    ekf.push_back(7.8 / (1.0 + 0.035 * x));
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

  ax.has_ylim = true;
  ax.ymin = y_min_plot;
  ax.ymax = y_max_plot;
  ax.gnuplot_commands = {
      "set label 1 'e_p(t)=e_0 e^{-{/Symbol l} t}' at graph 0.06,0.10 font 'Times,8' front"};
  fig.axes(0).set(ax);

  fig.axes(0).add_series(SeriesSpec{.type = SeriesType::Line, .label = "SRIF"}, t, srif);
  fig.axes(0).add_series(SeriesSpec{.type = SeriesType::Line, .label = "UKF"}, t, ukf);
  fig.axes(0).add_series(SeriesSpec{.type = SeriesType::Line, .label = "EKF"}, t, ekf);

  return example_common::render_figure(fig, out_dir / "figures");
}
