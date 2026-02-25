#include "gnuplotpp/templates.hpp"

#include "gnuplotpp/presets.hpp"

#include <filesystem>
#include <fstream>

namespace gnuplotpp {

void apply_plot_template(FigureSpec& spec, AxesSpec& ax, const PlotTemplate t) {
  apply_preset_defaults(spec);
  spec.auto_layout = true;
  spec.panel_labels = true;
  spec.write_manifest = true;
  spec.formats = {OutputFormat::Pdf, OutputFormat::Svg, OutputFormat::Png};
  ax.grid = true;
  ax.legend = true;

  switch (t) {
    case PlotTemplate::EstimationBand:
      spec.palette = ColorPalette::Tab10;
      spec.title = "Estimation with Uncertainty";
      ax.title = "State Estimate";
      ax.xlabel = "t [s]";
      ax.ylabel = "x(t)";
      break;
    case PlotTemplate::DistributionKDE:
      spec.palette = ColorPalette::Default;
      spec.title = "Distribution Summary";
      ax.title = "Histogram and KDE";
      ax.xlabel = "value";
      ax.ylabel = "count";
      break;
    case PlotTemplate::HeatmapField:
      spec.palette = ColorPalette::Viridis;
      spec.title = "2D Field";
      ax.title = "Heatmap";
      ax.xlabel = "x";
      ax.ylabel = "y";
      ax.color_map = ColorMap::Cividis;
      ax.colorbar_label = "intensity";
      break;
    case PlotTemplate::DualAxisProbability:
      spec.palette = ColorPalette::Tab10;
      spec.title = "Primary and Secondary Axis";
      ax.title = "Signal + Probability";
      ax.xlabel = "t [s]";
      ax.ylabel = "signal";
      ax.y2label = "P(event)";
      break;
  }
}

void write_template_gallery_yaml(const std::filesystem::path& dir) {
  std::filesystem::create_directories(dir);
  auto write = [&](const std::string& name, const std::string& body) {
    std::ofstream os(dir / name);
    os << body;
  };

  write("estimation_band.yaml",
        "figure:\n"
        "  title: Estimation with Uncertainty\n"
        "  preset: IEEE_DoubleColumn\n"
        "  palette: tab10\n"
        "  formats: [pdf, svg, png]\n"
        "  panel_labels: true\n"
        "axes:\n"
        "  - title: State Estimate\n"
        "    xlabel: t [s]\n"
        "    ylabel: x(t)\n"
        "    grid: true\n"
        "    legend: true\n");

  write("distribution_kde.yaml",
        "figure:\n"
        "  title: Distribution Summary\n"
        "  preset: IEEE_SingleColumn\n"
        "  palette: default\n"
        "  formats: [pdf, svg, png]\n"
        "axes:\n"
        "  - title: Histogram and KDE\n"
        "    xlabel: value\n"
        "    ylabel: count\n"
        "    grid: true\n");
}

}  // namespace gnuplotpp
