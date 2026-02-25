#include "example_common.hpp"
#include "gnuplotpp/builder.hpp"
#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"
#include "gnuplotpp/statistics.hpp"
#include "gnuplotpp/theme.hpp"

#include <cmath>
#include <filesystem>
#include <random>
#include <string>
#include <vector>

namespace {

gnuplotpp::FigureSpec base_spec(const std::string& title) {
  using namespace gnuplotpp;
  FigureSpec fs;
  fs.preset = Preset::IEEE_Tran;
  apply_preset_defaults(fs);
  apply_theme_preset(fs, ThemePreset::Science_v1);
  fs = FigureBuilder(fs)
           .title(title)
           .layout(1, 1)
           .formats({OutputFormat::Pdf, OutputFormat::Png, OutputFormat::Svg, OutputFormat::Eps})
           .palette(ColorPalette::Tab10)
           .manifest(true)
           .spec();
  fs.text_mode = TextMode::Enhanced;
  // Screen-review readable while still IEEE-like in proportions.
  fs.style.font = "Helvetica";
  fs.style.font_pt = 13.0;
  fs.style.line_width_pt = 2.2;
  fs.style.point_size = 1.0;
  fs.size = FigureSizeInches{.w = 5.4, .h = 3.8};
  return fs;
}

}  // namespace

int main(int argc, char** argv) {
  using namespace gnuplotpp;

  const std::filesystem::path out_dir =
      example_common::parse_out_dir(argc, argv, "out/feature_rich_showcase");

  std::filesystem::create_directories(out_dir);

  std::vector<double> t(200);
  std::vector<double> mean(200);
  std::vector<double> lo(200);
  std::vector<double> hi(200);
  for (int i = 0; i < 200; ++i) {
    t[i] = 0.1 * static_cast<double>(i);
    mean[i] = 0.55 * std::exp(-0.035 * t[i]) * std::sin(0.72 * t[i]) + 0.03;
    lo[i] = mean[i] - 0.25;
    hi[i] = mean[i] + 0.25;
  }

  std::vector<double> bins(20);
  std::vector<double> counts(20);
  std::vector<double> samples;
  samples.reserve(1200);
  std::mt19937_64 rng(7ULL);
  std::normal_distribution<double> nrm(0.1, 0.35);
  for (int i = 0; i < 1200; ++i) {
    samples.push_back(nrm(rng));
  }
  const double bin_w = 0.12;
  for (int i = 0; i < 20; ++i) {
    bins[i] = -1.2 + bin_w * static_cast<double>(i);
  }
  for (const double s : samples) {
    const int b = static_cast<int>(std::floor((s + 1.2) / bin_w));
    if (b >= 0 && b < 20) {
      counts[static_cast<std::size_t>(b)] += 1.0;
    }
  }
  const auto kde = gaussian_kde(samples, bins);
  std::vector<double> kde_scaled(kde.size(), 0.0);
  for (std::size_t i = 0; i < kde.size(); ++i) {
    kde_scaled[i] = kde[i] * static_cast<double>(samples.size()) * bin_w;
  }
  std::vector<double> hx;
  std::vector<double> hy;
  std::vector<double> hz;
  for (int iy = 0; iy < 40; ++iy) {
    for (int ix = 0; ix < 60; ++ix) {
      const double x = -2.0 + 4.0 * static_cast<double>(ix) / 59.0;
      const double y = -2.0 + 4.0 * static_cast<double>(iy) / 39.0;
      hx.push_back(x);
      hy.push_back(y);
      hz.push_back(std::exp(-((x - 0.2) * (x - 0.2) / 1.4 + (y + 0.1) * (y + 0.1) / 1.0)));
    }
  }

  {
    FigureSpec fs = base_spec("Mean + Confidence Band");
    Figure fig(fs);
    AxesSpec ax;
    ax.title = "State Estimate with 95% Interval";
    ax.xlabel = "t [s]";
    ax.ylabel = "x(t)";
    ax.grid = true;
    ax.legend = true;
    ax.legend_spec.position = LegendPosition::BottomLeft;
    ax.legend_spec.columns = 1;
    ax.legend_spec.boxed = true;
    ax.legend_spec.has_font_pt = true;
    ax.legend_spec.font_pt = 11.5;
    ax.has_ytick_step = true;
    ax.ytick_step = 0.2;
    ax.has_xlim = true;
    ax.xmin = 0.0;
    ax.xmax = 20.0;
    fig.axes(0).set(ax);

    SeriesSpec band;
    band.label = "95% band";
    band.has_color = true;
    band.color = "#1f77b4";
    band.has_opacity = true;
    band.opacity = 0.20;
    fig.axes(0).add_band(band, t, lo, hi);
    fig.axes(0).add_series(SeriesSpec{.label = "mean", .has_line_width = true, .line_width_pt = 2.8},
                           t,
                           mean);
    std::vector<double> t_obs;
    std::vector<double> y_obs;
    for (int i = 0; i < 200; i += 10) {
      t_obs.push_back(t[static_cast<std::size_t>(i)]);
      y_obs.push_back(mean[static_cast<std::size_t>(i)] + 0.02 * std::sin(2.7 * t[static_cast<std::size_t>(i)]));
    }
    fig.axes(0).add_series(SeriesSpec{.type = SeriesType::Scatter, .label = "obs", .has_color = true, .color = "#444444"},
                           t_obs,
                           y_obs);
    if (example_common::render_figure(fig, out_dir / "mean_band" / "figures") != 0) {
      return 1;
    }
    save_theme_json(out_dir / "mean_band" / "figures" / "science_theme.json", fs);
  }

  {
    FigureSpec fs = base_spec("Histogram + KDE");
    Figure fig(fs);
    AxesSpec ax;
    ax.title = "Distribution: Histogram and KDE";
    ax.xlabel = "bin center";
    ax.ylabel = "count";
    ax.grid = true;
    ax.legend = true;
    ax.legend_spec.position = LegendPosition::TopRight;
    ax.legend_spec.boxed = true;
    ax.legend_spec.has_font_pt = true;
    ax.legend_spec.font_pt = 11.5;
    ax.yformat = "%.0f";
    ax.xformat = "%.1f";
    ax.has_xlim = true;
    ax.xmin = -1.3;
    ax.xmax = 1.3;
    fig.axes(0).set(ax);

    SeriesSpec hist;
    hist.label = "hist";
    hist.has_color = true;
    hist.color = "#2ca02c";
    hist.has_opacity = true;
    hist.opacity = 0.45;
    fig.axes(0).add_histogram(hist, bins, counts);
    fig.axes(0).add_series(SeriesSpec{.label = "KDE", .has_line_width = true, .line_width_pt = 2.6},
                           bins,
                           kde_scaled);
    if (example_common::render_figure(fig, out_dir / "hist_kde" / "figures") != 0) {
      return 1;
    }
  }

  {
    FigureSpec fs = base_spec("Heatmap Samples");
    fs.size = FigureSizeInches{.w = 4.6, .h = 4.6};
    Figure fig(fs);
    AxesSpec ax;
    ax.title = "2D Density Field";
    ax.xlabel = "x";
    ax.ylabel = "y";
    ax.legend = false;
    ax.grid = false;
    ax.has_xlim = true;
    ax.xmin = -2.05;
    ax.xmax = 2.05;
    ax.has_ylim = true;
    ax.ymin = -2.05;
    ax.ymax = 2.05;
    ax.gnuplot_commands = {"set size ratio -1"};
    fig.axes(0).set(ax);
    fig.axes(0).add_heatmap(SeriesSpec{.label = "density"}, hx, hy, hz);
    if (example_common::render_figure(fig, out_dir / "heatmap" / "figures") != 0) {
      return 1;
    }
  }

  {
    FigureSpec fs = base_spec("Primary Signal and Detection Probability");
    Figure fig(fs);
    AxesSpec ax;
    ax.title = "Primary Signal and Detection Probability";
    ax.xlabel = "t [s]";
    ax.ylabel = "value";
    ax.y2label = "P(event)";
    ax.grid = true;
    ax.legend = true;
    ax.legend_spec.position = LegendPosition::BottomRight;
    ax.legend_spec.columns = 1;
    ax.legend_spec.boxed = true;
    ax.legend_spec.has_font_pt = true;
    ax.legend_spec.font_pt = 11.5;
    ax.has_y2lim = true;
    ax.y2min = 0.0;
    ax.y2max = 1.0;
    fig.axes(0).set(ax);

    std::vector<double> y1(200);
    std::vector<double> y2(200);
    for (int i = 0; i < 200; ++i) {
      y1[i] = 0.9 * std::exp(-0.03 * t[i]) * std::cos(0.62 * t[i]);
      y2[i] = 0.9 * std::exp(-0.03 * t[i]) * std::sin(0.62 * t[i]);
    }
    fig.axes(0).add_series(SeriesSpec{.label = "signal A", .has_line_width = true, .line_width_pt = 2.6}, t, y1);
    fig.axes(0).add_series(SeriesSpec{.label = "signal B", .has_line_width = true, .line_width_pt = 2.6}, t, y2);
    std::vector<double> p_event(200);
    for (int i = 0; i < 200; ++i) {
      p_event[static_cast<std::size_t>(i)] = 1.0 / (1.0 + std::exp(-(t[static_cast<std::size_t>(i)] - 10.0) / 1.8));
    }
    fig.axes(0).add_series(
        SeriesSpec{.label = "P(event)", .use_y2 = true, .has_line_width = true, .line_width_pt = 2.4},
        t,
        p_event);
    if (example_common::render_figure(fig, out_dir / "lines_y2" / "figures") != 0) {
      return 1;
    }
  }

  gnuplotpp::log::Info("generated individual figures under: ", out_dir.string());
  return 0;
}
