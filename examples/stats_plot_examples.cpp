#include "gnuplotpp/gnuplot_backend.hpp"
#include "gnuplotpp/logging.hpp"
#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"
#include "gnuplotpp/statistics.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace {

gnuplotpp::FigureSpec make_spec(const std::string& title) {
  using namespace gnuplotpp;
  FigureSpec fs;
  fs.preset = Preset::Custom;
  apply_preset_defaults(fs);
  fs.size = FigureSizeInches{.w = 5.4, .h = 3.8};
  fs.title = title;
  fs.text_mode = TextMode::Enhanced;
  fs.formats = {OutputFormat::Pdf, OutputFormat::Png, OutputFormat::Svg};
  fs.palette = ColorPalette::Tab10;
  fs.style.font = "Helvetica";
  fs.style.font_pt = 15.0;
  fs.style.line_width_pt = 2.2;
  fs.style.point_size = 1.10;
  fs.style.grid = true;
  fs.font_fallbacks = {"Arial", "Nimbus Sans", "DejaVu Sans", "Helvetica"};
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
    ax.title = "Normal Q-Q Plot";
    ax.xlabel = "Theoretical Quantile";
    ax.ylabel = "Sample Quantile";
    ax.grid = true;
    ax.legend = true;
    ax.legend_spec.position = LegendPosition::TopLeft;
    ax.legend_spec.boxed = true;
    ax.legend_spec.opaque = true;
    ax.legend_spec.has_font_pt = true;
    ax.legend_spec.font_pt = 13.0;
    ax.has_xtick_step = true;
    ax.xtick_step = 1.0;
    ax.has_ytick_step = true;
    ax.ytick_step = 1.0;
    fig.axes(0).set(ax);

    std::vector<double> q_theory, q_sample;
    qq_plot_normal(samples, q_theory, q_sample);
    fig.axes(0).add_series(SeriesSpec{.type = SeriesType::Scatter,
                                      .label = "Samples",
                                      .has_color = true,
                                      .color = "#1f77b4"},
                           q_theory,
                           q_sample);

    const auto [x_min_it, x_max_it] = std::minmax_element(q_theory.begin(), q_theory.end());
    const auto [y_min_it, y_max_it] = std::minmax_element(q_sample.begin(), q_sample.end());
    const double min_axis = std::min(*x_min_it, *y_min_it);
    const double max_axis = std::max(*x_max_it, *y_max_it);
    const double pad = 0.12 * (max_axis - min_axis);
    ax.has_xlim = true;
    ax.xmin = min_axis - pad;
    ax.xmax = max_axis + pad;
    ax.has_ylim = true;
    ax.ymin = min_axis - pad;
    ax.ymax = max_axis + pad;
    fig.axes(0).set(ax);

    std::vector<double> x_line{min_axis - pad, max_axis + pad};
    std::vector<double> y_line{min_axis - pad, max_axis + pad};
    fig.axes(0).add_series(
        SeriesSpec{.label = "Reference (y = x)",
                   .has_line_width = true,
                   .line_width_pt = 2.4,
                   .has_color = true,
                   .color = "#d95f02"},
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
    ax.legend = false;
    ax.has_xlim = true;
    ax.xmin = -1.05;
    ax.xmax = 1.05;
    ax.has_ylim = true;
    ax.ymin = -2.6;
    ax.ymax = 3.2;
    ax.has_xtick_step = true;
    ax.xtick_step = 0.25;
    fig.axes(0).set(ax);

    std::vector<double> y_grid, half_w;
    violin_profile(samples, y_grid, half_w, 180);
    const auto summary = box_summary(samples);
    std::vector<double> x_l(half_w.size()), x_r(half_w.size());
    for (std::size_t i = 0; i < half_w.size(); ++i) {
      x_l[i] = -half_w[i];
      x_r[i] = half_w[i];
    }
    std::ostringstream fill_poly;
    fill_poly << std::fixed << std::setprecision(6);
    fill_poly << "set object 20 polygon from ";
    for (std::size_t i = 0; i < y_grid.size(); ++i) {
      fill_poly << x_l[i] << "," << y_grid[i] << " to ";
    }
    for (std::size_t i = y_grid.size(); i > 0; --i) {
      const std::size_t j = i - 1;
      fill_poly << x_r[j] << "," << y_grid[j];
      if (j > 0) {
        fill_poly << " to ";
      }
    }
    fill_poly << " fs transparent solid 0.32 fc rgb '#6aaed6' front";
    ax.gnuplot_commands = {
        fill_poly.str(),
        "set arrow 20 from -0.30," + std::to_string(summary.q1) + " to 0.30," +
            std::to_string(summary.q1) + " nohead lw 1.5 lc rgb '#1f1f1f' dt 2 front",
        "set arrow 21 from -0.36," + std::to_string(summary.median) + " to 0.36," +
            std::to_string(summary.median) + " nohead lw 2.0 lc rgb '#1f1f1f' front",
        "set arrow 22 from -0.30," + std::to_string(summary.q3) + " to 0.30," +
            std::to_string(summary.q3) + " nohead lw 1.5 lc rgb '#1f1f1f' dt 2 front"};
    fig.axes(0).set(ax);

    fig.axes(0).add_series(SeriesSpec{.label = "left",
                                      .has_line_width = true,
                                      .line_width_pt = 2.3,
                                      .has_color = true,
                                      .color = "#2c7fb8"},
                           x_l,
                           y_grid);
    fig.axes(0).add_series(SeriesSpec{.label = "right",
                                      .has_line_width = true,
                                      .line_width_pt = 2.3,
                                      .has_color = true,
                                      .color = "#2c7fb8"},
                           x_r,
                           y_grid);
    std::vector<double> x_obs;
    std::vector<double> y_obs;
    x_obs.reserve(350);
    y_obs.reserve(350);
    std::uniform_real_distribution<double> x_jitter(-0.05, 0.05);
    for (int i = 0; i < 350; ++i) {
      x_obs.push_back(x_jitter(rng));
      y_obs.push_back(samples[static_cast<std::size_t>(i)]);
    }
    fig.axes(0).add_series(
        SeriesSpec{.type = SeriesType::Scatter,
                   .label = "",
                   .has_color = true,
                   .color = "#2f2f2f",
                   .has_opacity = true,
                   .opacity = 0.45},
        x_obs,
        y_obs);

    if (render(fig, out_root / "violin_profile" / "figures") != 0) return 1;
  }

  // 3) Box summary + sample points
  {
    auto fs = make_spec("Box Summary");
    Figure fig(fs);
    AxesSpec ax;
    ax.title = "Box Summary (Tukey)";
    ax.xlabel = "Group";
    ax.ylabel = "Value";
    ax.grid = true;
    ax.legend = false;
    ax.has_xlim = true;
    ax.xmin = 0.7;
    ax.xmax = 1.3;
    ax.has_xtick_step = true;
    ax.xtick_step = 0.1;
    fig.axes(0).set(ax);

    const auto box = box_summary(samples);
    const auto [smin_it, smax_it] = std::minmax_element(samples.begin(), samples.end());
    const double ypad = 0.10 * (*smax_it - *smin_it);
    ax.has_ylim = true;
    ax.ymin = *smin_it - ypad;
    ax.ymax = *smax_it + ypad;
    ax.gnuplot_commands = {"set xtics ('Sample A' 1.0) font 'Helvetica,13'"};
    fig.axes(0).set(ax);

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
        SeriesSpec{.type = SeriesType::Scatter,
                   .label = "",
                   .has_color = true,
                   .color = "#4d4d4d",
                   .has_opacity = true,
                   .opacity = 0.55},
        x_pts,
        y_pts);

    ax.gnuplot_commands = {
        "set xtics ('Sample A' 1.0) font 'Helvetica,13'",
        "set object 1 rect from 0.88," + std::to_string(box.q1) + " to 1.12," +
            std::to_string(box.q3) + " fc rgb '#4c78a8' fs solid 0.30 border lc rgb '#2f2f2f'",
        "set arrow 1 from 0.88," + std::to_string(box.median) + " to 1.12," +
            std::to_string(box.median) + " nohead lw 2.0 lc rgb '#1f1f1f'",
        "set arrow 2 from 1.0," + std::to_string(box.whisker_low) + " to 1.0," +
            std::to_string(box.q1) + " nohead lw 1.5 lc rgb '#1f1f1f'",
        "set arrow 3 from 1.0," + std::to_string(box.q3) + " to 1.0," +
            std::to_string(box.whisker_high) + " nohead lw 1.5 lc rgb '#1f1f1f'",
        "set arrow 4 from 0.95," + std::to_string(box.whisker_low) + " to 1.05," +
            std::to_string(box.whisker_low) + " nohead lw 1.5 lc rgb '#1f1f1f'",
        "set arrow 5 from 0.95," + std::to_string(box.whisker_high) + " to 1.05," +
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
    ax.legend_spec.position = LegendPosition::TopLeft;
    ax.legend_spec.boxed = true;
    ax.legend_spec.opaque = true;
    ax.legend_spec.has_font_pt = true;
    ax.legend_spec.font_pt = 13.0;
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
    fig.axes(0).add_series(SeriesSpec{.type = SeriesType::Scatter,
                                      .label = "Samples",
                                      .has_color = true,
                                      .color = "#4d4d4d",
                                      .has_opacity = true,
                                      .opacity = 0.50},
                           x,
                           y);

    std::vector<double> ex, ey;
    confidence_ellipse(x, y, 2.0, ex, ey, 240);
    fig.axes(0).add_series(
        SeriesSpec{.label = "2{/Symbol s} ellipse",
                   .has_line_width = true,
                   .line_width_pt = 2.8,
                   .has_color = true,
                   .color = "#e45756"},
        ex,
        ey);

    if (render(fig, out_root / "confidence_ellipse" / "figures") != 0) return 1;
  }

  // 5) Autocorrelation
  {
    auto fs = make_spec("Autocorrelation");
    Figure fig(fs);
    AxesSpec ax;
    ax.title = "Autocorrelation (Lags 0..60)";
    ax.xlabel = "Lag";
    ax.ylabel = "\\rho(k)";
    ax.grid = true;
    ax.legend = true;
    ax.legend_spec.position = LegendPosition::TopRight;
    ax.legend_spec.boxed = true;
    ax.legend_spec.opaque = true;
    ax.legend_spec.has_font_pt = true;
    ax.legend_spec.font_pt = 13.0;
    ax.has_xlim = true;
    ax.xmin = -0.5;
    ax.xmax = 60.5;
    ax.has_ylim = true;
    ax.ymin = -0.2;
    ax.ymax = 1.05;
    ax.has_xtick_step = true;
    ax.xtick_step = 10.0;
    ax.has_ytick_step = true;
    ax.ytick_step = 0.2;
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
    const double ci = 1.96 / std::sqrt(static_cast<double>(sig.size()));
    std::vector<double> ci_pos(ac.size(), ci);
    std::vector<double> ci_neg(ac.size(), -ci);
    fig.axes(0).add_histogram(
        SeriesSpec{.label = "ACF",
                   .has_color = true,
                   .color = "#4c78a8",
                   .has_opacity = true,
                   .opacity = 0.42,
                   .has_line_width = true,
                   .line_width_pt = 1.4},
        lags,
        ac);
    fig.axes(0).add_series(SeriesSpec{.label = "95% bounds",
                                      .has_line_width = true,
                                      .line_width_pt = 2.0,
                                      .has_color = true,
                                      .color = "#d95f02"},
                           lags,
                           ci_pos);
    fig.axes(0).add_series(SeriesSpec{.label = "",
                                      .has_line_width = true,
                                      .line_width_pt = 2.0,
                                      .has_color = true,
                                      .color = "#d95f02"},
                           lags,
                           ci_neg);

    if (render(fig, out_root / "autocorrelation" / "figures") != 0) return 1;
  }

  gnuplotpp::log::Info("generated stats examples under: ", out_root.string());
  return 0;
}
