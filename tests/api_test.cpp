#include "gnuplotpp/builder.hpp"
#include "gnuplotpp/data.hpp"
#include "gnuplotpp/facet.hpp"
#include "gnuplotpp/model_overlays.hpp"
#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"
#include "gnuplotpp/quickstart.hpp"
#include "gnuplotpp/spec_yaml.hpp"
#include "gnuplotpp/statistics.hpp"
#include "gnuplotpp/templates.hpp"
#include "gnuplotpp/theme.hpp"
#include "gnuplotpp/transforms.hpp"

#include <cassert>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <vector>
#include <filesystem>

namespace {

class DummyBackend final : public gnuplotpp::IPlotBackend {
public:
  gnuplotpp::RenderResult render(const gnuplotpp::Figure&, const std::filesystem::path&) override {
    return gnuplotpp::RenderResult{
        .ok = true, .status = gnuplotpp::RenderStatus::Success, .message = "ok"};
  }
};

}  // namespace

int main() {
  using namespace gnuplotpp;

  FigureSpec spec;
  spec.preset = Preset::IEEE_DoubleColumn;
  apply_preset_defaults(spec);
  spec.rows = 1;
  spec.cols = 2;

  assert(spec.size.w == 7.16);
  assert(spec.style.font_pt == 8.5);

  Figure fig(spec);
  AxesSpec ax;
  ax.title = "test";
  ax.has_title_font_pt = true;
  ax.title_font_pt = 15.0;
  fig.axes(0, 0).set(ax);
  assert(fig.axes(0, 0).spec().typography.has_title_font_pt);
  assert(fig.axes(0, 0).spec().typography.title_font_pt == 15.0);

  const std::vector<double> x{0.0, 1.0, 2.0};
  const std::vector<double> y{1.0, 2.0, 3.0};
  fig.axes(0, 0).add_series(SeriesSpec{.label = "line"}, x, y);
  bool threw = false;
  try {
    fig.axes(0, 0).add_series(SeriesSpec{.label = "bad", .has_opacity = true, .opacity = 1.2},
                              x,
                              y);
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  assert(threw);

  const std::vector<double> y_lo{0.8, 1.6, 2.4};
  const std::vector<double> y_hi{1.2, 2.4, 3.6};
  fig.axes(0, 0).add_band(SeriesSpec{.label = "band"}, x, y_lo, y_hi);
  fig.axes(0, 0).add_histogram(SeriesSpec{.label = "hist"}, x, y);
  fig.axes(0, 0).add_heatmap(SeriesSpec{.label = "hm"}, x, y, y);

  assert(fig.axes(0, 0).series().size() == 4U);
  assert(fig.axes(0, 0).series()[1].spec.type == SeriesType::Band);
  assert(fig.axes(0, 0).series()[2].spec.type == SeriesType::Histogram);
  assert(fig.axes(0, 0).series()[3].spec.type == SeriesType::Heatmap);

  apply_style_profile(spec, StyleProfile::Presentation);
  assert(spec.palette == ColorPalette::Viridis);
  assert(spec.style.font_pt >= 12.0);
  apply_style_profile(spec, StyleProfile::Tufte_Minimal);
  assert(spec.palette == ColorPalette::Grayscale);
  assert(!spec.style.grid);
  assert(spec.style.line_width_pt >= 1.5);
  assert(spec.style.label_font_scale > 1.0);
  assert(spec.style.title_font_scale > spec.style.label_font_scale);
  assert(spec.style.title_bold);

  FigureSpec built_spec = FigureBuilder(spec).layout(1, 1).manifest(true).spec();
  assert(built_spec.write_manifest);
  const auto quick_spec = make_quick_figure_spec(
      QuickFigureOptions{.preset = Preset::Custom,
                         .profile = StyleProfile::Tufte_Minimal,
                         .size = FigureSizeInches{.w = 5.0, .h = 3.0},
                         .title = "quick"});
  assert(quick_spec.style.title_bold);
  assert(quick_spec.title == "quick");
  auto quick_fig = make_quick_figure(
      QuickFigureOptions{.preset = Preset::IEEE_SingleColumn, .profile = StyleProfile::Science});
  assert(quick_fig.spec().rows == 1);
  auto pub_fig = make_publication_figure(PublicationFigureOptions{
      .preset = Preset::IEEE_SingleColumn,
      .profile = StyleProfile::IEEE_Strict,
      .figure_title = "pub",
      .axes_title = "signal",
      .xlabel = "t",
      .ylabel = "x"});
  assert(pub_fig.spec().title == "pub");
  assert(pub_fig.axes(0).spec().title == "signal");
  const auto quick_ax = make_quick_axes("A", "x", "y", true, false);
  assert(quick_ax.grid);
  assert(!quick_ax.legend);

  const auto ma = moving_average(y, 2);
  assert(ma.size() == y.size());
  const auto ds = downsample_uniform(y, 2);
  assert(!ds.empty());
  const auto ac = autocorrelation(y, 2);
  assert(ac.size() == 3);
  const auto fit = linear_fit(x, y);
  assert(fit.r2 > 0.99);
  const auto yhat = linear_fit_line(fit, x);
  assert(yhat.size() == x.size());
  Figure fit_fig(spec);
  fit_fig.axes(0, 0).set(AxesSpec{});
  const auto fit_overlay = add_linear_fit_overlay(fit_fig.axes(0, 0), x, y, "fit");
  assert(fit_overlay.r2 > 0.99);

  std::vector<double> ex;
  std::vector<double> ep;
  ecdf(y, ex, ep);
  assert(ex.size() == y.size());

  std::vector<std::vector<double>> ens{{1.0, 2.0, 3.0}, {2.0, 3.0, 4.0}, {3.0, 4.0, 5.0}};
  std::vector<double> lo_b;
  std::vector<double> hi_b;
  percentile_band(ens, 0.1, 0.9, lo_b, hi_b);
  assert(lo_b.size() == 3);
  std::vector<std::vector<double>> fan_lo;
  std::vector<std::vector<double>> fan_hi;
  fan_chart_bands(ens, {0.1, 0.25, 0.75, 0.9}, fan_lo, fan_hi);
  assert(fan_lo.size() == 2);
  std::vector<double> vy;
  std::vector<double> vw;
  violin_profile(y, vy, vw, 32);
  assert(vy.size() == vw.size());
  std::vector<double> qx;
  std::vector<double> qy;
  qq_plot_normal(y, qx, qy);
  assert(qx.size() == y.size());
  const auto bx = box_summary(y);
  assert(bx.whisker_high >= bx.whisker_low);
  std::vector<double> exx;
  std::vector<double> exy;
  confidence_ellipse(y, y, 2.0, exx, exy, 64);
  assert(exx.size() == 64);

  const auto csv_path = std::filesystem::temp_directory_path() / "gnuplotpp_data_test.csv";
  {
    std::ofstream os(csv_path);
    os << "t,pos,vel\n";
    os << "0,1.0,0.1\n";
    os << "1,2.0,0.2\n";
  }
  const auto tbl = read_csv_numeric(csv_path);
  assert(tbl.column("t").size() == 2U);
  assert(tbl.has_column("pos"));
  assert(tbl.row_count() == 2U);
  assert(label_with_unit("position", "m") == "position [m]");
  const auto z = transform_zscore(tbl.column("pos"));
  assert(z.size() == tbl.column("pos").size());
  const auto clipped = transform_clip(z, -0.5, 0.5);
  assert(clipped.size() == z.size());
  TransformPipeline pipe;
  pipe.set_input(std::vector<double>{1.0, 2.0, 3.0, 4.0}).rolling_mean(2).zscore().clip(-1.0, 1.0);
  assert(pipe.values().size() == 4U);
  std::filesystem::remove(csv_path);

  const auto rc = facet_grid(5);
  assert(rc.first * rc.second >= 5);
  FigureSpec facet_spec = spec;
  facet_spec.rows = rc.first;
  facet_spec.cols = rc.second;
  Figure facet_fig(facet_spec);
  apply_facet_axes(facet_fig, AxesSpec{.xlabel = "x"}, {"a", "b", "c", "d", "e"});
  assert(facet_fig.axes(0).spec().title == "a");

  const auto theme_path = std::filesystem::temp_directory_path() / "gnuplotpp_theme_test.json";
  assert(save_theme_json(theme_path, built_spec));
  FigureSpec loaded = built_spec;
  assert(load_theme_json(theme_path, loaded));
  std::filesystem::remove(theme_path);

  FigureSpec ts;
  AxesSpec ta;
  apply_plot_template(ts, ta, PlotTemplate::EstimationBand);
  assert(!ts.title.empty());
  const auto tdir = std::filesystem::temp_directory_path() / "gnuplotpp_template_gallery";
  write_template_gallery_yaml(tdir);
  assert(std::filesystem::exists(tdir / "estimation_band.yaml"));
  std::filesystem::remove_all(tdir);

  const auto yaml_path = std::filesystem::temp_directory_path() / "gnuplotpp_yaml_spec.yaml";
  {
    std::ofstream os(yaml_path);
    os << "figure:\n"
          "  title: yaml\n"
          "  preset: IEEE_SingleColumn\n"
          "  formats: [pdf, svg]\n"
          "axes:\n"
          "  - title: a0\n"
          "    xlabel: t\n"
          "    ylabel: y\n"
          "    y2label: p(event)\n"
          "    y2log: true\n"
          "    y2min: 0.1\n"
          "    y2max: 0.9\n"
          "    legend_spec:\n"
          "      position: top_left\n"
          "      columns: 2\n"
          "      boxed: true\n"
          "      opaque: false\n"
          "      font_pt: 8.0\n"
          "    typography:\n"
          "      tick_font_pt: 10.0\n"
          "      label_font_pt: 12.0\n"
          "      title_font_pt: 14.0\n"
          "      title_bold: true\n"
          "    frame:\n"
          "      border_mask: 3\n"
          "      border_line_width_pt: 0.9\n"
          "      border_color: '#111111'\n"
          "      ticks_out: true\n"
          "      ticks_mirror: false\n";
  }
  const auto ys = load_yaml_figure_spec(yaml_path, YamlLoadOptions{.strict_unknown_keys = true});
  assert(ys.figure.title == "yaml");
  assert(!ys.axes.empty());
  assert(ys.axes[0].y2label == "p(event)");
  assert(ys.axes[0].y2log);
  assert(ys.axes[0].has_y2lim);
  assert(ys.axes[0].y2min == 0.1);
  assert(ys.axes[0].y2max == 0.9);
  assert(ys.axes[0].legend_spec.position == LegendPosition::TopLeft);
  assert(ys.axes[0].legend_spec.columns == 2);
  assert(ys.axes[0].legend_spec.boxed);
  assert(ys.axes[0].legend_spec.has_font_pt);
  assert(ys.axes[0].legend_spec.font_pt == 8.0);
  assert(ys.axes[0].typography.has_tick_font_pt);
  assert(ys.axes[0].typography.tick_font_pt == 10.0);
  assert(ys.axes[0].typography.has_title_bold);
  assert(ys.axes[0].typography.title_bold);
  assert(ys.axes[0].frame.has_border_mask);
  assert(ys.axes[0].frame.border_mask == 3);
  assert(ys.axes[0].frame.has_border_color);
  assert(ys.axes[0].frame.border_color == "#111111");
  assert(ys.axes[0].frame.has_ticks_out);
  assert(ys.axes[0].frame.ticks_out);
  assert(ys.axes[0].frame.has_ticks_mirror);
  assert(!ys.axes[0].frame.ticks_mirror);
  std::filesystem::remove(yaml_path);

  FigureSpec composition_spec = spec;
  composition_spec.rows = 2;
  composition_spec.cols = 2;
  Figure composition_fig(composition_spec);
  for (int i = 0; i < 4; ++i) {
    composition_fig.axes(i).set(AxesSpec{});
    composition_fig.axes(i).add_series(SeriesSpec{.label = "s"}, x, y);
  }
  apply_panel_titles(composition_fig, {"A", "B", "C", "D"});
  assert(composition_fig.axes(2).spec().title == "C");
  LegendSpec shared_legend;
  shared_legend.enabled = true;
  shared_legend.position = LegendPosition::TopLeft;
  apply_shared_legend(composition_fig, shared_legend, 0);
  assert(composition_fig.axes(0).spec().legend);
  assert(!composition_fig.axes(1).spec().legend);
  apply_shared_colorbar_label(composition_fig, "density", 3);
  assert(composition_fig.axes(3).spec().colorbar_label == "density");
  assert(composition_fig.axes(0).spec().colorbar_label.empty());
  apply_small_multiples_defaults(composition_fig, true, true);
  assert(composition_fig.axes(0).spec().has_xlim);

  std::vector<double> lx{0.0, 1.0, 2.0, 3.0};
  std::vector<double> ly{0.0, 0.1, 0.2, 1.5};
  AxesSpec auto_leg_ax;
  auto_place_legend(auto_leg_ax, lx, ly);
  assert(auto_leg_ax.legend);
  auto_leg_ax.equations.push_back({.expression = "y=mx+b",
                                   .at = Coord2D{.system = CoordSystem::Graph, .x = 0.05, .y = 0.95},
                                   .boxed = true});
  auto_leg_ax.callouts.push_back({.text = "peak",
                                  .from = Coord2D{.system = CoordSystem::Data, .x = 2.0, .y = 0.2},
                                  .to = Coord2D{.system = CoordSystem::Data, .x = 3.0, .y = 1.5}});
  assert(!auto_leg_ax.equations.empty());

  Figure table_fig(spec);
  AxesSpec table_ax;
  table_ax.title = "table";
  table_fig.axes(0, 0).set(table_ax);
  tbl.add_line(table_fig.axes(0, 0), SeriesSpec{.label = "pos"}, "t", "pos");
  assert(table_fig.axes(0, 0).series().size() == 1U);

  FigureSpec themed_spec = spec;
  apply_theme_preset(themed_spec, ThemePreset::Tufte_Minimal_v1);
  assert(std::string(theme_preset_id(ThemePreset::Tufte_Minimal_v1)) == "tufte_minimal_v1");

  const auto no_backend = fig.save("out");
  assert(!no_backend.ok);
  assert(no_backend.status == RenderStatus::InvalidInput);

  fig.set_backend(std::make_shared<DummyBackend>());
  const auto ok = fig.save("out");
  assert(ok.ok);
  assert(ok.status == RenderStatus::Success);

  return 0;
}
