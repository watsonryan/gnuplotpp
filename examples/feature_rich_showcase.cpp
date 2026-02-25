#include "gnuplotpp/gnuplot_backend.hpp"
#include "gnuplotpp/logging.hpp"
#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"

#include <cmath>
#include <filesystem>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  using namespace gnuplotpp;

  std::filesystem::path out_dir = "out/feature_rich_showcase";
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--out" && i + 1 < argc) {
      out_dir = argv[++i];
    }
  }

  FigureSpec fs;
  fs.preset = Preset::IEEE_DoubleColumn;
  apply_preset_defaults(fs);
  apply_style_profile(fs, StyleProfile::Science);
  fs.rows = 2;
  fs.cols = 2;
  fs.formats = {OutputFormat::Pdf, OutputFormat::Png, OutputFormat::Svg, OutputFormat::Eps};
  fs.text_mode = TextMode::LaTeX;
  fs.write_manifest = true;
  fs.title = "Feature Showcase";

  Figure fig(fs);

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

  AxesSpec ax0;
  ax0.title = "Mean + Confidence Band";
  ax0.xlabel = "t [s]";
  ax0.ylabel = "x(t)";
  ax0.grid = true;
  ax0.legend = true;
  ax0.legend_spec.position = LegendPosition::TopLeft;
  ax0.legend_spec.columns = 1;
  ax0.has_xtick_step = true;
  ax0.xtick_step = 5.0;
  ax0.has_ytick_step = true;
  ax0.ytick_step = 0.5;
  ax0.labels.push_back(LabelAnnotation{
      .text = "$x(t)=\\mu(t)\\pm\\sigma$",
      .at = "graph 0.05,0.92",
      .font = "Times,8",
      .front = true,
  });
  ax0.rectangles.push_back(RectObject{
      .from = "graph 0.55,0.10",
      .to = "graph 0.95,0.35",
      .has_fill_opacity = true,
      .fill_opacity = 0.15,
      .fill_color = "#e0f3ff",
      .border = false,
      .border_color = "#000000",
      .front = false,
  });
  fig.axes(0).set(ax0);

  SeriesSpec band;
  band.label = "95% band";
  band.has_color = true;
  band.color = "#1f77b4";
  band.has_opacity = true;
  band.opacity = 0.20;
  fig.axes(0).add_band(band, t, lo, hi);

  SeriesSpec center;
  center.label = "mean";
  center.has_line_width = true;
  center.line_width_pt = 1.7;
  fig.axes(0).add_series(center, t, mean);

  AxesSpec ax1;
  ax1.title = "Histogram";
  ax1.xlabel = "bin center";
  ax1.ylabel = "count";
  ax1.grid = true;
  ax1.legend = false;
  fig.axes(1).set(ax1);

  std::vector<double> bins(20);
  std::vector<double> counts(20);
  for (int i = 0; i < 20; ++i) {
    bins[i] = -1.0 + 0.1 * static_cast<double>(i);
    const double d = bins[i] - 0.1;
    counts[i] = 120.0 * std::exp(-d * d / 0.18);
  }
  SeriesSpec hist;
  hist.label = "PDF proxy";
  hist.has_color = true;
  hist.color = "#2ca02c";
  hist.has_opacity = true;
  hist.opacity = 0.45;
  fig.axes(1).add_histogram(hist, bins, counts);

  AxesSpec ax2;
  ax2.title = "Heatmap Samples";
  ax2.xlabel = "x";
  ax2.ylabel = "y";
  ax2.legend = false;
  fig.axes(2).set(ax2);

  std::vector<double> hx;
  std::vector<double> hy;
  std::vector<double> hz;
  for (int iy = 0; iy < 35; ++iy) {
    for (int ix = 0; ix < 35; ++ix) {
      const double x = -2.0 + 4.0 * static_cast<double>(ix) / 34.0;
      const double y = -2.0 + 4.0 * static_cast<double>(iy) / 34.0;
      hx.push_back(x);
      hy.push_back(y);
      hz.push_back(std::exp(-(x * x + y * y)));
    }
  }
  fig.axes(2).add_heatmap(SeriesSpec{.label = "density"}, hx, hy, hz);

  AxesSpec ax3;
  ax3.title = "Legend + Tick Controls";
  ax3.xlabel = "t [s]";
  ax3.ylabel = "value";
  ax3.grid = true;
  ax3.legend = true;
  ax3.legend_spec.position = LegendPosition::OutsideBottom;
  ax3.legend_spec.columns = 2;
  ax3.legend_spec.boxed = true;
  ax3.has_xtick_step = true;
  ax3.xtick_step = 4.0;
  ax3.has_xminor_count = true;
  ax3.xminor_count = 4;
  ax3.xformat = "%.1f";
  ax3.yformat = "%.2f";
  fig.axes(3).set(ax3);

  std::vector<double> y1(200);
  std::vector<double> y2(200);
  for (int i = 0; i < 200; ++i) {
    y1[i] = std::cos(0.5 * t[i]);
    y2[i] = std::sin(0.5 * t[i]);
  }
  fig.axes(3).add_series(SeriesSpec{.label = "cos"}, t, y1);
  fig.axes(3).add_series(SeriesSpec{.label = "sin"}, t, y2);

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
