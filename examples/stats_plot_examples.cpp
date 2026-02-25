#include "gnuplotpp/gnuplot_backend.hpp"
#include "gnuplotpp/logging.hpp"
#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"
#include "gnuplotpp/statistics.hpp"

#include <cmath>
#include <filesystem>
#include <random>
#include <string>
#include <vector>

namespace {

gnuplotpp::FigureSpec make_spec(const std::string& title) {
  using namespace gnuplotpp;
  FigureSpec fs;
  fs.preset = Preset::IEEE_DoubleColumn;
  apply_preset_defaults(fs);
  fs.size = FigureSizeInches{.w = 4.8, .h = 3.4};
  fs.title = title;
  fs.formats = {OutputFormat::Pdf, OutputFormat::Png, OutputFormat::Svg};
  fs.palette = ColorPalette::Tab10;
  fs.panel_labels = false;
  fs.caption.clear();
  fs.auto_layout = true;
  return fs;
}

int render(gnuplotpp::Figure& fig, const std::filesystem::path& out_dir) {
  fig.set_backend(gnuplotpp::make_gnuplot_backend());
  const auto result = fig.save(out_dir);
  if (!result.ok) {
    gnuplotpp::log::Error("render failed: ", result.message);
    return 1;
  }
  for (const auto& p : result.outputs) {
    gnuplotpp::log::Info("output: ", p.string());
  }
  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  using namespace gnuplotpp;

  std::filesystem::path out_root = "out/stats_plot_examples";
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--out" && i + 1 < argc) {
      out_root = argv[++i];
    }
  }
  std::filesystem::create_directories(out_root);

  std::mt19937_64 rng(1234ULL);
  std::normal_distribution<double> nrm(0.2, 0.85);
  std::vector<double> samples(1000);
  for (double& s : samples) {
    s = nrm(rng);
  }

  // 1) QQ plot
  {
    auto fs = make_spec("Q-Q Plot vs Normal");
    Figure fig(fs);
    AxesSpec ax;
    ax.title = "Q-Q Plot";
    ax.xlabel = "Theoretical Quantile";
    ax.ylabel = "Sample Quantile";
    ax.grid = true;
    ax.legend = true;
    fig.axes(0).set(ax);

    std::vector<double> q_theory, q_sample;
    qq_plot_normal(samples, q_theory, q_sample);
    fig.axes(0).add_series(SeriesSpec{.type = SeriesType::Scatter, .label = "samples"},
                           q_theory,
                           q_sample);

    const double x0 = q_theory.front();
    const double x1 = q_theory.back();
    const double y0 = q_sample.front();
    const double y1 = q_sample.back();
    const double slope = (y1 - y0) / (x1 - x0);
    std::vector<double> x_line{x0, x1};
    std::vector<double> y_line{y0, y0 + slope * (x1 - x0)};
    fig.axes(0).add_series(
        SeriesSpec{.label = "reference", .has_line_width = true, .line_width_pt = 2.0},
        x_line,
        y_line);

    if (render(fig, out_root / "qq_plot" / "figures") != 0) return 1;
  }

  // 2) Violin profile
  {
    auto fs = make_spec("Violin Profile");
    Figure fig(fs);
    AxesSpec ax;
    ax.title = "Violin Density Profile";
    ax.xlabel = "Relative Density Width";
    ax.ylabel = "Value";
    ax.grid = true;
    ax.legend = true;
    fig.axes(0).set(ax);

    std::vector<double> y_grid, half_w;
    violin_profile(samples, y_grid, half_w, 180);
    std::vector<double> x_l(half_w.size()), x_r(half_w.size());
    for (std::size_t i = 0; i < half_w.size(); ++i) {
      x_l[i] = -half_w[i];
      x_r[i] = half_w[i];
    }
    fig.axes(0).add_series(SeriesSpec{.label = "left"}, x_l, y_grid);
    fig.axes(0).add_series(SeriesSpec{.label = "right"}, x_r, y_grid);

    if (render(fig, out_root / "violin_profile" / "figures") != 0) return 1;
  }

  // 3) Box summary + sample points
  {
    auto fs = make_spec("Box Summary");
    Figure fig(fs);
    AxesSpec ax;
    ax.title = "Box Summary (Tukey)";
    ax.xlabel = "group";
    ax.ylabel = "value";
    ax.grid = true;
    ax.legend = false;
    ax.has_xlim = true;
    ax.xmin = 0.5;
    ax.xmax = 1.5;
    fig.axes(0).set(ax);

    const auto box = box_summary(samples);
    std::vector<double> x_pts;
    std::vector<double> y_pts;
    x_pts.reserve(300);
    y_pts.reserve(300);
    std::uniform_real_distribution<double> jitter(-0.045, 0.045);
    for (int i = 0; i < 300; ++i) {
      x_pts.push_back(1.0 + jitter(rng));
      y_pts.push_back(samples[static_cast<std::size_t>(i)]);
    }
    fig.axes(0).add_series(
        SeriesSpec{.type = SeriesType::Scatter, .label = "", .has_color = true, .color = "#777777"},
        x_pts,
        y_pts);

    ax.gnuplot_commands = {
        "set object 1 rect from 0.85," + std::to_string(box.q1) + " to 1.15," +
            std::to_string(box.q3) + " fc rgb '#7aa6d8' fs solid 0.35 border lc rgb '#2f2f2f'",
        "set arrow 1 from 0.85," + std::to_string(box.median) + " to 1.15," +
            std::to_string(box.median) + " nohead lw 2.0 lc rgb '#1f1f1f'",
        "set arrow 2 from 1.0," + std::to_string(box.whisker_low) + " to 1.0," +
            std::to_string(box.q1) + " nohead lw 1.5 lc rgb '#1f1f1f'",
        "set arrow 3 from 1.0," + std::to_string(box.q3) + " to 1.0," +
            std::to_string(box.whisker_high) + " nohead lw 1.5 lc rgb '#1f1f1f'",
        "set arrow 4 from 0.93," + std::to_string(box.whisker_low) + " to 1.07," +
            std::to_string(box.whisker_low) + " nohead lw 1.5 lc rgb '#1f1f1f'",
        "set arrow 5 from 0.93," + std::to_string(box.whisker_high) + " to 1.07," +
            std::to_string(box.whisker_high) + " nohead lw 1.5 lc rgb '#1f1f1f'"};
    fig.axes(0).set(ax);

    if (render(fig, out_root / "box_summary" / "figures") != 0) return 1;
  }

  // 4) Confidence ellipse
  {
    auto fs = make_spec("Confidence Ellipse");
    Figure fig(fs);
    AxesSpec ax;
    ax.title = "2-Sigma Confidence Ellipse";
    ax.xlabel = "x";
    ax.ylabel = "y";
    ax.grid = true;
    ax.legend = true;
    fig.axes(0).set(ax);

    std::normal_distribution<double> nx(0.0, 1.0);
    std::normal_distribution<double> ny(0.0, 0.5);
    std::vector<double> x(900), y(900);
    for (std::size_t i = 0; i < x.size(); ++i) {
      const double vx = nx(rng);
      const double vy = ny(rng);
      x[i] = vx;
      y[i] = 0.55 * vx + vy;
    }
    fig.axes(0).add_series(SeriesSpec{.type = SeriesType::Scatter, .label = "samples"}, x, y);

    std::vector<double> ex, ey;
    confidence_ellipse(x, y, 2.0, ex, ey, 240);
    fig.axes(0).add_series(
        SeriesSpec{.label = "2-sigma", .has_line_width = true, .line_width_pt = 2.3}, ex, ey);

    if (render(fig, out_root / "confidence_ellipse" / "figures") != 0) return 1;
  }

  // 5) Autocorrelation
  {
    auto fs = make_spec("Autocorrelation");
    Figure fig(fs);
    AxesSpec ax;
    ax.title = "Autocorrelation (lags 0..60)";
    ax.xlabel = "lag";
    ax.ylabel = "rho(lag)";
    ax.grid = true;
    ax.legend = false;
    fig.axes(0).set(ax);

    std::vector<double> sig(800);
    sig[0] = 0.0;
    std::normal_distribution<double> wn(0.0, 1.0);
    for (std::size_t i = 1; i < sig.size(); ++i) {
      sig[i] = 0.86 * sig[i - 1] + 0.25 * wn(rng);
    }
    const auto ac = autocorrelation(sig, 60);
    std::vector<double> lags(ac.size());
    for (std::size_t i = 0; i < lags.size(); ++i) lags[i] = static_cast<double>(i);
    fig.axes(0).add_histogram(
        SeriesSpec{.label = "", .has_color = true, .color = "#5b8c5a", .has_opacity = true, .opacity = 0.85},
        lags,
        ac);

    if (render(fig, out_root / "autocorrelation" / "figures") != 0) return 1;
  }

  gnuplotpp::log::Info("generated stats examples under: ", out_root.string());
  return 0;
}
