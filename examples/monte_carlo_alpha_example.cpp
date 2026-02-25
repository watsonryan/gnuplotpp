#include "gnuplotpp/gnuplot_backend.hpp"
#include "gnuplotpp/logging.hpp"
#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"

#include <algorithm>
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
  fs.style.font = "Helvetica";
  fs.style.font_pt = 12.5;
  fs.style.line_width_pt = 1.6;
  fs.style.grid = false;
  {
    std::ostringstream title;
    title << "Monte Carlo Ensemble (N=" << n_paths << ", alpha=" << alpha << ")";
    plot_title = title.str();
  }
  fs.title.clear();

  Figure fig(fs);

  AxesSpec ax;
  ax.xlabel = "t [s]";
  ax.ylabel = "x(t)";
  ax.grid = false;
  ax.legend = false;
  ax.has_xlim = true;
  ax.xmin = 0.0;
  ax.xmax = 22.0;
  ax.has_xtick_step = true;
  ax.xtick_step = 5.0;
  ax.gnuplot_commands = {
      "set border 3 linewidth 0.8 linecolor rgb '#1c1c1c'",
      "set tics out nomirror",
      "set arrow 10 from graph 0, first 0 to graph 1, first 0 nohead lc rgb '#9a9a9a' dt 3 lw 0.8 back",
      "set xlabel 't [s]' font 'Helvetica,16'",
      "set ylabel 'x(t)' font 'Helvetica,16'",
      "set title '{/:Bold " + plot_title + "}' font 'Helvetica,18'"};
  fig.axes(0).set(ax);

  std::vector<double> t;
  t.reserve(220);
  for (int i = 0; i < 220; ++i) {
    t.push_back(0.1 * static_cast<double>(i));
  }

  std::mt19937_64 rng(123456ULL);
  std::normal_distribution<double> noise(0.0, 0.12);
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
    s.has_line_width = true;
    s.line_width_pt = line_width;
    s.has_color = true;
    s.color = "#000000";
    s.has_opacity = true;
    s.opacity = alpha;
    fig.axes(0).add_series(s, t, y);
  }

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
