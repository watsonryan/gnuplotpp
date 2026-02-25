#include "example_common.hpp"
#include "gnuplotpp/data.hpp"
#include "gnuplotpp/facet.hpp"
#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"
#include "gnuplotpp/statistics.hpp"

#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  using namespace gnuplotpp;

  const std::filesystem::path out_dir =
      example_common::parse_out_dir(argc, argv, "out/interactive_facet_example");

  std::filesystem::create_directories(out_dir);
  const auto csv_path = out_dir / "demo.csv";
  {
    std::ofstream os(csv_path);
    os << "t,g0,g1,g2,g3\n";
    for (int i = 0; i < 250; ++i) {
      const double t = 0.05 * static_cast<double>(i);
      os << t << "," << std::sin(0.7 * t) << "," << std::cos(0.6 * t) << ","
         << std::sin(0.45 * t + 0.7) << "," << std::cos(0.5 * t - 0.5) << "\n";
    }
  }
  const auto tbl = read_csv_numeric(csv_path);
  const auto& t = tbl.column("t");

  FigureSpec fs;
  fs.preset = Preset::IEEE_DoubleColumn;
  apply_preset_defaults(fs);
  const auto [rows, cols] = facet_grid(4);
  fs.rows = rows;
  fs.cols = cols;
  fs.formats = {OutputFormat::Pdf, OutputFormat::Svg, OutputFormat::Png};
  fs.title = "Interactive Facets with Fan Bands";
  fs.share_x = true;
  fs.share_y = true;
  fs.hide_inner_tick_labels = true;
  fs.auto_layout = true;
  fs.interactive_preview = true;
  fs.write_manifest = true;

  Figure fig(fs);
  AxesSpec base;
  base.xlabel = label_with_unit("time", "s");
  base.ylabel = label_with_unit("state", "a.u.");
  base.grid = true;
  base.legend = true;
  base.legend_spec.position = LegendPosition::TopLeft;
  base.enable_crosshair = true;
  apply_facet_axes(fig, base, {"Group A", "Group B", "Group C", "Group D"});
  apply_panel_titles(fig, {"Group A", "Group B", "Group C", "Group D"});

  std::mt19937_64 rng(42ULL);
  std::normal_distribution<double> nrm(0.0, 0.08);
  const std::vector<std::string> cols_name{"g0", "g1", "g2", "g3"};
  for (int k = 0; k < 4; ++k) {
    const auto& y = tbl.column(cols_name[static_cast<std::size_t>(k)]);
    std::vector<std::vector<double>> ensemble(80, std::vector<double>(y.size(), 0.0));
    for (auto& member : ensemble) {
      for (std::size_t i = 0; i < y.size(); ++i) {
        member[i] = y[i] + nrm(rng);
      }
    }

    std::vector<std::vector<double>> fan_lo;
    std::vector<std::vector<double>> fan_hi;
    fan_chart_bands(ensemble, {0.1, 0.25, 0.75, 0.9}, fan_lo, fan_hi);
    for (std::size_t b = 0; b < fan_lo.size(); ++b) {
      SeriesSpec s;
      s.label = b == 0 ? "fan" : "";
      s.has_color = true;
      s.color = "#1f77b4";
      s.has_opacity = true;
      s.opacity = (b == 0) ? 0.14 : 0.24;
      fig.axes(k).add_band(s, t, fan_lo[b], fan_hi[b]);
    }

    auto ax_spec = fig.axes(k).spec();
    auto_place_legend(ax_spec, t, y);
    fig.axes(k).set(ax_spec);
    tbl.add_line(fig.axes(k),
                 SeriesSpec{.label = "mean", .has_line_width = true, .line_width_pt = 1.8},
                 "t",
                 cols_name[static_cast<std::size_t>(k)]);
  }

  LegendSpec shared_legend;
  shared_legend.enabled = true;
  shared_legend.position = LegendPosition::TopLeft;
  shared_legend.columns = 1;
  shared_legend.boxed = true;
  apply_shared_legend(fig, shared_legend, 0);
  apply_small_multiples_defaults(fig, true, true);

  if (example_common::render_figure(fig, out_dir / "figures") != 0) {
    return 1;
  }
  gnuplotpp::log::Info("interactive preview script: ",
                       (out_dir / "figures" / "tmp" / "interactive_preview.gp").string());
  return 0;
}
