#include "example_common.hpp"
#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/spec_yaml.hpp"
#include "gnuplotpp/templates.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  using namespace gnuplotpp;

  const std::filesystem::path out_dir =
      example_common::parse_out_dir(argc, argv, "out/yaml_spec_example");
  std::filesystem::create_directories(out_dir);
  write_template_gallery_yaml(out_dir / "templates");

  const auto yaml_path = out_dir / "figure.yaml";
  {
    std::ofstream os(yaml_path);
    os << "figure:\n"
          "  title: YAML Driven Figure\n"
          "  caption: YAML config demo\n"
          "  preset: IEEE_DoubleColumn\n"
          "  palette: tab10\n"
          "  panel_labels: true\n"
          "  auto_layout: true\n"
          "  formats: [pdf, svg, png]\n"
          "axes:\n"
          "  - title: Loaded from YAML\n"
          "    xlabel: t [s]\n"
          "    ylabel: y\n"
          "    grid: true\n"
          "    legend: true\n";
  }

  const auto spec = load_yaml_figure_spec(yaml_path);
  Figure fig(spec.figure);
  if (!spec.axes.empty()) {
    fig.axes(0).set(spec.axes[0]);
  }

  std::vector<double> t(240), y1(240), y2(240);
  for (int i = 0; i < 240; ++i) {
    t[i] = 0.05 * static_cast<double>(i);
    y1[i] = std::sin(0.7 * t[i]);
    y2[i] = 0.8 * std::cos(0.6 * t[i] - 0.4);
  }
  fig.axes(0).add_series(SeriesSpec{.label = "series A"}, t, y1);
  fig.axes(0).add_series(SeriesSpec{.label = "series B"}, t, y2);

  return example_common::render_figure(fig, out_dir / "figures");
}
