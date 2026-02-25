#include "example_common.hpp"
#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"

#include <cmath>
#include <filesystem>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  using namespace gnuplotpp;

  const std::filesystem::path out_dir =
      example_common::parse_out_dir(argc, argv, "out/tufte_minimal_example");

  FigureSpec fs;
  fs.preset = Preset::Custom;
  apply_preset_defaults(fs);
  apply_style_profile(fs, StyleProfile::Tufte_Minimal);
  fs.size = FigureSizeInches{.w = 5.6, .h = 3.1};
  fs.formats = {OutputFormat::Pdf, OutputFormat::Png, OutputFormat::Svg};
  fs.title.clear();
  fs.auto_layout = true;
  fs.text_mode = TextMode::Enhanced;

  Figure fig(fs);

  AxesSpec ax;
  ax.xlabel = "Time [s]";
  ax.ylabel = "Normalized response";
  ax.legend = false;
  ax.grid = false;
  ax.has_xlim = true;
  ax.xmin = 0.0;
  ax.xmax = 20.0;
  ax.has_ylim = true;
  ax.ymin = 0.0;
  ax.ymax = 1.15;
  ax.has_xtick_step = true;
  ax.xtick_step = 5.0;
  ax.has_ytick_step = true;
  ax.ytick_step = 0.2;
  ax.frame.has_border_mask = true;
  ax.frame.border_mask = 3;
  ax.frame.has_border_line_width_pt = true;
  ax.frame.border_line_width_pt = 0.8;
  ax.frame.has_border_color = true;
  ax.frame.border_color = "#1c1c1c";
  ax.frame.has_ticks_out = true;
  ax.frame.ticks_out = true;
  ax.frame.has_ticks_mirror = true;
  ax.frame.ticks_mirror = false;
  ax.gnuplot_commands = {
      "unset key",
      "set arrow 10 from graph 0, first 1.0 to graph 1, first 1.0 nohead lc rgb '#a8a8a8' dt 3 lw 0.9 back"};
  fig.axes(0).set(ax);

  std::vector<double> t;
  std::vector<double> baseline;
  std::vector<double> treatment;
  t.reserve(320);
  baseline.reserve(320);
  treatment.reserve(320);

  for (int i = 0; i < 320; ++i) {
    const double x = 20.0 * static_cast<double>(i) / 319.0;
    t.push_back(x);
    baseline.push_back(1.0 - 0.42 * std::exp(-0.20 * x));
    treatment.push_back(1.0 - 0.68 * std::exp(-0.34 * x));
  }

  fig.axes(0).add_series(SeriesSpec{.label = "", .has_line_width = true, .line_width_pt = 2.2}, t, baseline);
  fig.axes(0).add_series(
      SeriesSpec{.label = "",
                 .has_line_width = true,
                 .line_width_pt = 2.2,
                 .has_color = true,
                 .color = "#4a4a4a"},
      t,
      treatment);

  const double x_label = 18.6;
  ax.gnuplot_commands.push_back("set label 10 'Baseline' at " + std::to_string(x_label) + "," +
                                std::to_string(baseline.back() + 0.035) +
                                " font 'Helvetica,10' textcolor rgb '#111111'");
  ax.gnuplot_commands.push_back("set label 11 'Treatment' at " + std::to_string(x_label) + "," +
                                std::to_string(treatment.back() - 0.05) +
                                " font 'Helvetica,10' textcolor rgb '#4a4a4a'");
  fig.axes(0).set(ax);

  return example_common::render_figure(fig, out_dir / "figures");
}
