#include "gnuplotpp/builder.hpp"
#include "gnuplotpp/data.hpp"
#include "gnuplotpp/facet.hpp"
#include "gnuplotpp/plot.hpp"
#include "gnuplotpp/presets.hpp"
#include "gnuplotpp/quickstart.hpp"
#include "gnuplotpp/spec_yaml.hpp"
#include "gnuplotpp/statistics.hpp"
#include "gnuplotpp/templates.hpp"
#include "gnuplotpp/theme.hpp"

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
  const auto quick_ax = make_quick_axes("A", "x", "y", true, false);
  assert(quick_ax.grid);
  assert(!quick_ax.legend);

  const auto ma = moving_average(y, 2);
  assert(ma.size() == y.size());
  const auto ds = downsample_uniform(y, 2);
  assert(!ds.empty());
  const auto ac = autocorrelation(y, 2);
  assert(ac.size() == 3);

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
  assert(label_with_unit("position", "m") == "position [m]");
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
          "    ylabel: y\n";
  }
  const auto ys = load_yaml_figure_spec(yaml_path);
  assert(ys.figure.title == "yaml");
  assert(!ys.axes.empty());
  std::filesystem::remove(yaml_path);

  const auto no_backend = fig.save("out");
  assert(!no_backend.ok);
  assert(no_backend.status == RenderStatus::InvalidInput);

  fig.set_backend(std::make_shared<DummyBackend>());
  const auto ok = fig.save("out");
  assert(ok.ok);
  assert(ok.status == RenderStatus::Success);

  return 0;
}
