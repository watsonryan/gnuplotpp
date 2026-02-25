#include "example_common.hpp"
#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <sstream>
#include <random>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  using namespace gnuplotpp;

  std::filesystem::path out_dir = "out/monte_carlo_alpha_example";
  int n_paths = 1000;
  double line_width = 2.0;
  double alpha = 0.02;
  std::string plot_title;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--out" && i + 1 < argc) {
      out_dir = argv[++i];
    } else if (arg == "--npaths" && i + 1 < argc) {
      n_paths = std::max(1, std::stoi(argv[++i]));
    } else if (arg == "--lw" && i + 1 < argc) {
      line_width = std::max(0.1, std::stod(argv[++i]));
    } else if (arg == "--alpha" && i + 1 < argc) {
      alpha = std::clamp(std::stod(argv[++i]), 0.0, 1.0);
    }
  }

  FigureSpec fs;
  fs.preset = Preset::Custom;
  apply_preset_defaults(fs);
  apply_style_profile(fs, StyleProfile::Tufte_Minimal);
  fs.rows = 1;
  fs.cols = 1;
  fs.formats = {OutputFormat::Pdf, OutputFormat::Png, OutputFormat::Svg, OutputFormat::Eps};
  fs.size = FigureSizeInches{.w = 5.8, .h = 3.4};
  {
    std::ostringstream title;
    title << "Monte Carlo Ensemble (N=" << n_paths << ", alpha=" << alpha << ")";
    plot_title = title.str();
  }
  fs.title.clear();

  Figure fig(fs);

  AxesSpec ax;
  ax.title = plot_title;
  ax.xlabel = "t [s]";
  ax.ylabel = "x(t)";
  ax.grid = false;
  ax.legend = true;
  ax.legend_spec.position = LegendPosition::TopLeft;
  ax.legend_spec.boxed = true;
  ax.legend_spec.opaque = true;
  ax.legend_spec.has_font_pt = true;
  ax.legend_spec.font_pt = 12.0;
  ax.has_xlim = true;
  ax.xmin = 0.0;
  ax.xmax = 22.0;
  ax.has_xtick_step = true;
  ax.xtick_step = 5.0;
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
      "set arrow 10 from graph 0, first 0 to graph 1, first 0 nohead lc rgb '#9a9a9a' dt 3 lw 0.8 back"};
  fig.axes(0).set(ax);

  std::vector<double> t;
  t.reserve(220);
  for (int i = 0; i < 220; ++i) {
    t.push_back(0.1 * static_cast<double>(i));
  }
  std::vector<double> sigma_upper;
  std::vector<double> sigma_lower;
  sigma_upper.reserve(t.size());
  sigma_lower.reserve(t.size());

  std::mt19937_64 rng(123456ULL);
  std::normal_distribution<double> noise(0.0, 0.12);
  constexpr double phi = 0.985;
  constexpr double sigma_w = 0.12;
  const double phi2 = phi * phi;
  for (std::size_t i = 0; i < t.size(); ++i) {
    const double k = static_cast<double>(i + 1);
    const double var_k = (sigma_w * sigma_w) * (1.0 - std::pow(phi2, k)) / (1.0 - phi2);
    const double three_sigma = 3.0 * std::sqrt(std::max(0.0, var_k));
    sigma_upper.push_back(three_sigma);
    sigma_lower.push_back(-three_sigma);
  }

  for (int k = 0; k < n_paths; ++k) {
    std::vector<double> y;
    y.reserve(t.size());

    double state = 0.0;
    for (double x_sample : t) {
      (void)x_sample;
      state = 0.985 * state + noise(rng);
      y.push_back(state);
    }

    SeriesSpec s;
    s.type = SeriesType::Line;
    s.label = (k == 0) ? "MC run" : "";
    s.has_line_width = true;
    s.line_width_pt = line_width;
    s.has_color = true;
    s.color = "#000000";
    s.has_opacity = true;
    s.opacity = alpha;
    fig.axes(0).add_series(s, t, y);
  }

  fig.axes(0).add_series(SeriesSpec{.label = "3{/Symbol s} bound",
                                    .has_line_width = true,
                                    .line_width_pt = 2.0,
                                    .has_color = true,
                                    .color = "#d62728",
                                    .has_opacity = true,
                                    .opacity = 1.0},
                         t,
                         sigma_upper);
  fig.axes(0).add_series(SeriesSpec{.label = "",
                                    .has_line_width = true,
                                    .line_width_pt = 2.0,
                                    .has_color = true,
                                    .color = "#d62728",
                                    .has_opacity = true,
                                    .opacity = 1.0},
                         t,
                         sigma_lower);

  return example_common::render_figure(fig, out_dir / "figures");
}
