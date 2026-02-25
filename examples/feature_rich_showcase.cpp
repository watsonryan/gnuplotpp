#include "gnuplotpp/builder.hpp"
#include "gnuplotpp/gnuplot_backend.hpp"
#include "gnuplotpp/logging.hpp"
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
  apply_style_profile(fs, StyleProfile::Science);
  fs = FigureBuilder(fs)
           .title(title)
           .layout(1, 1)
           .formats({OutputFormat::Pdf, OutputFormat::Png, OutputFormat::Svg, OutputFormat::Eps})
           .palette(ColorPalette::Tab10)
           .manifest(true)
           .spec();
  fs.text_mode = TextMode::Enhanced;
  return fs;
}

int render_figure(gnuplotpp::Figure& fig, const std::filesystem::path& out_dir) {
  fig.set_backend(gnuplotpp::make_gnuplot_backend());
  const auto result = fig.save(out_dir);
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

}  // namespace

int main(int argc, char** argv) {
  using namespace gnuplotpp;

  std::filesystem::path out_dir = "out/feature_rich_showcase";
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--out" && i + 1 < argc) {
      out_dir = argv[++i];
    }
  }

  std::filesystem::create_directories(out_dir);

  std::vector<double> t(200);
  std::vector<double> mean(200);
  std::vector<double> lo(200);
  std::vector<double> hi(200);
  for (int i = 0; i < 200; ++i) {
    t[i] = 0.1 * static_cast<double>(i);
    mean[i] = 0.4 * std::sin(0.7 * t[i]) + 0.02 * t[i];
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
  std::vector<double> ecdf_x;
  std::vector<double> ecdf_p;
  ecdf(samples, ecdf_x, ecdf_p);

  std::vector<double> hx;
  std::vector<double> hy;
  std::vector<double> hz;
  for (int iy = 0; iy < 40; ++iy) {
    for (int ix = 0; ix < 40; ++ix) {
      const double x = -2.0 + 4.0 * static_cast<double>(ix) / 39.0;
      const double y = -2.0 + 4.0 * static_cast<double>(iy) / 39.0;
      hx.push_back(x);
      hy.push_back(y);
      hz.push_back(std::exp(-(x * x + y * y)));
    }
  }

  {
    FigureSpec fs = base_spec("Mean + Confidence Band");
    Figure fig(fs);
    AxesSpec ax;
    ax.title = "Mean + Confidence Band";
    ax.xlabel = "t [s]";
    ax.ylabel = "x(t)";
    ax.grid = true;
    ax.legend = true;
    ax.legend_spec.position = LegendPosition::TopLeft;
    ax.legend_spec.columns = 1;
    ax.labels.push_back(LabelAnnotation{
        .text = "x(t)={/Symbol m}(t){/Symbol \\261}{/Symbol s}",
        .at = "graph 0.05,0.92",
        .font = "Times,8",
        .front = true,
    });
    fig.axes(0).set(ax);

    SeriesSpec band;
    band.label = "95% band";
    band.has_color = true;
    band.color = "#1f77b4";
    band.has_opacity = true;
    band.opacity = 0.20;
    fig.axes(0).add_band(band, t, lo, hi);
    fig.axes(0).add_series(SeriesSpec{.label = "mean", .has_line_width = true, .line_width_pt = 1.7},
                           t,
                           mean);
    std::vector<double> e_low(200, 0.08);
    std::vector<double> e_high(200, 0.12);
    fig.axes(0).add_errorbars_asymmetric(
        SeriesSpec{.label = "obs", .has_color = true, .color = "#444444"}, t, mean, e_low, e_high);
    if (render_figure(fig, out_dir / "mean_band" / "figures") != 0) {
      return 1;
    }
    save_theme_json(out_dir / "mean_band" / "figures" / "science_theme.json", fs);
  }

  {
    FigureSpec fs = base_spec("Histogram + KDE");
    Figure fig(fs);
    AxesSpec ax;
    ax.title = "Histogram + KDE";
    ax.xlabel = "bin center";
    ax.ylabel = "count";
    ax.grid = true;
    ax.legend = true;
    ax.legend_spec.position = LegendPosition::TopRight;
    fig.axes(0).set(ax);

    SeriesSpec hist;
    hist.label = "hist";
    hist.has_color = true;
    hist.color = "#2ca02c";
    hist.has_opacity = true;
    hist.opacity = 0.45;
    fig.axes(0).add_histogram(hist, bins, counts);
    fig.axes(0).add_series(SeriesSpec{.label = "KDE", .has_line_width = true, .line_width_pt = 1.8},
                           bins,
                           kde_scaled);
    if (render_figure(fig, out_dir / "hist_kde" / "figures") != 0) {
      return 1;
    }
  }

  {
    FigureSpec fs = base_spec("Heatmap Samples");
    fs.size = FigureSizeInches{.w = 3.2, .h = 3.2};
    Figure fig(fs);
    AxesSpec ax;
    ax.title = "Heatmap Samples";
    ax.xlabel = "x";
    ax.ylabel = "y";
    ax.legend = false;
    ax.has_xlim = true;
    ax.xmin = -2.05;
    ax.xmax = 2.05;
    ax.has_ylim = true;
    ax.ymin = -2.05;
    ax.ymax = 2.05;
    fig.axes(0).set(ax);
    fig.axes(0).add_heatmap(SeriesSpec{.label = "density"}, hx, hy, hz);
    if (render_figure(fig, out_dir / "heatmap" / "figures") != 0) {
      return 1;
    }
  }

  {
    FigureSpec fs = base_spec("Lines + y2 ECDF");
    Figure fig(fs);
    AxesSpec ax;
    ax.title = "Lines + y2 ECDF";
    ax.xlabel = "t [s]";
    ax.ylabel = "value";
    ax.y2label = "p(x)";
    ax.grid = true;
    ax.legend = true;
    ax.legend_spec.position = LegendPosition::TopLeft;
    ax.legend_spec.columns = 1;
    ax.has_y2lim = true;
    ax.y2min = 0.0;
    ax.y2max = 1.0;
    fig.axes(0).set(ax);

    std::vector<double> y1(200);
    std::vector<double> y2(200);
    for (int i = 0; i < 200; ++i) {
      y1[i] = std::cos(0.5 * t[i]);
      y2[i] = std::sin(0.5 * t[i]);
    }
    fig.axes(0).add_series(SeriesSpec{.label = "cos"}, t, y1);
    fig.axes(0).add_series(SeriesSpec{.label = "sin"}, t, y2);
    fig.axes(0).add_series(
        SeriesSpec{.label = "ECDF", .use_y2 = true, .has_line_width = true, .line_width_pt = 1.3},
        ecdf_x,
        ecdf_p);
    if (render_figure(fig, out_dir / "lines_y2" / "figures") != 0) {
      return 1;
    }
  }

  gnuplotpp::log::Info("generated individual figures under: ", out_dir.string());
  return 0;
}
