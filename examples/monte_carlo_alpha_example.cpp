#include "gnuplotpp/gnuplot_backend.hpp"
#include "gnuplotpp/logging.hpp"
#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"

#include <filesystem>
#include <random>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  using namespace gnuplotpp;

  std::filesystem::path out_dir = "out/monte_carlo_alpha_example";
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
  fs.cols = 1;
  fs.formats = {OutputFormat::Pdf, OutputFormat::Svg, OutputFormat::Eps};
  fs.title = "Monte Carlo Ensemble (N=1000)";

  Figure fig(fs);

  AxesSpec ax;
  ax.xlabel = "t [s]";
  ax.ylabel = "x(t)";
  ax.grid = true;
  ax.legend = false;
  fig.axes(0).set(ax);

  std::vector<double> t;
  t.reserve(220);
  for (int i = 0; i < 220; ++i) {
    t.push_back(0.1 * static_cast<double>(i));
  }

  std::mt19937_64 rng(123456ULL);
  std::normal_distribution<double> noise(0.0, 0.12);
  for (int k = 0; k < 1000; ++k) {
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
    s.line_width_pt = 0.5;
    s.has_color = true;
    s.color = "#000000";
    s.has_opacity = true;
    s.opacity = 0.08;
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
