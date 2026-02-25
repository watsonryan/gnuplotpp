# Plot Controls

This document is the full control reference for `gnuplotpp` plotting.

## Core Types

- `gnuplotpp::FigureSpec`
- `gnuplotpp::AxesSpec`
- `gnuplotpp::SeriesSpec`
- `gnuplotpp::Figure`
- `gnuplotpp::Axes`
- `gnuplotpp::gaussian_kde`
- `gnuplotpp::FigureBuilder`
- `gnuplotpp::make_quick_figure_spec` / `gnuplotpp::make_quick_figure` / `gnuplotpp::make_quick_axes`
- `gnuplotpp::save_theme_json` / `gnuplotpp::load_theme_json`
- `gnuplotpp::apply_theme_preset`
- `gnuplotpp::read_csv_numeric`
- `gnuplotpp::facet_grid` / `gnuplotpp::apply_facet_axes`
- `gnuplotpp::apply_panel_titles` / `gnuplotpp::apply_shared_legend` / `gnuplotpp::apply_shared_colorbar_label`
- `gnuplotpp::auto_legend_position` / `gnuplotpp::auto_place_legend`
- `gnuplotpp::load_yaml_figure_spec`
- `gnuplotpp::apply_plot_template`
- `gnuplotpp::linear_fit` / `gnuplotpp::add_linear_fit_overlay`
- `gnuplotpp::TransformPipeline`

## Figure-Level Controls (`FigureSpec`)

```cpp
gnuplotpp::FigureSpec fs;
fs.preset = gnuplotpp::Preset::IEEE_SingleColumn;
fs.rows = 1;
fs.cols = 1;
fs.title = "My Figure";
fs.formats = {gnuplotpp::OutputFormat::Pdf,
              gnuplotpp::OutputFormat::Svg,
              gnuplotpp::OutputFormat::Eps};
fs.palette = gnuplotpp::ColorPalette::Tab10;
fs.text_mode = gnuplotpp::TextMode::Enhanced; // Enhanced, Plain, LaTeX
fs.write_manifest = true; // emits out/.../manifest.json
fs.share_x = true;
fs.share_y = true;
fs.hide_inner_tick_labels = true;
fs.auto_layout = true;
fs.interactive_preview = true; // emits tmp/interactive_preview.gp
fs.panel_labels = true;
fs.caption = "Figure caption text";
fs.font_fallbacks = {"Helvetica", "Arial", "Times"};
fs.export_policy.drop_line_alpha_for_vector = true;
fs.export_policy.warn_line_alpha_on_vector = true;

// Optional manual style override
fs.style.font = "Times";
fs.style.font_pt = 8.5;
fs.style.line_width_pt = 1.5;
fs.style.point_size = 0.6;
fs.style.grid = false;
```

Preset sizes:
- `IEEE_SingleColumn` -> 3.5 in wide
- `IEEE_DoubleColumn` -> 7.16 in wide
- `AIAA_Column` -> 3.25 in wide
- `AIAA_Page` -> 7.0 in wide

Style profiles:
- `apply_style_profile(fs, StyleProfile::Science)`
- `apply_style_profile(fs, StyleProfile::IEEE_Strict)`
- `apply_style_profile(fs, StyleProfile::AIAA_Strict)`
- `apply_style_profile(fs, StyleProfile::Presentation)`
- `apply_style_profile(fs, StyleProfile::DarkPrintSafe)`

## Axes-Level Controls (`AxesSpec`)

```cpp
gnuplotpp::AxesSpec ax;
ax.title = "Position Error Norm";
ax.xlabel = "t [s]";
ax.ylabel = "||e_p|| [m]";
ax.y2label = "aux metric";
ax.grid = true;
ax.legend = true;
ax.legend_spec.position = gnuplotpp::LegendPosition::TopRight;
ax.legend_spec.columns = 2;
ax.legend_spec.boxed = true;
ax.legend_spec.opaque = false;
ax.legend_spec.has_font_pt = true;
ax.legend_spec.font_pt = 8.0;

// Axis limits
ax.has_xlim = true;
ax.xmin = 0.0;
ax.xmax = 100.0;
ax.has_ylim = true;
ax.ymin = 1e-3;
ax.ymax = 20.0;

// Log scales
ax.xlog = false;
ax.ylog = true;
ax.y2log = false;
ax.has_y2lim = true;
ax.y2min = 0.0;
ax.y2max = 1.0;
ax.color_map = gnuplotpp::ColorMap::Cividis;
ax.color_norm = gnuplotpp::ColorNorm::Linear;
ax.colorbar_label = "density";
ax.has_cbrange = true;
ax.cbmin = 0.0;
ax.cbmax = 1.0;
ax.has_cbtick_step = true;
ax.cbtick_step = 0.1;

// Tick/format controls
ax.has_xtick_step = true;
ax.xtick_step = 5.0;
ax.has_ytick_step = true;
ax.ytick_step = 0.5;
ax.has_xminor_count = true;
ax.xminor_count = 4;
ax.has_yminor_count = true;
ax.yminor_count = 4;
ax.xformat = "%.2f";
ax.yformat = "%.2e";

// Structured typography overrides
ax.typography.has_tick_font_pt = true;
ax.typography.tick_font_pt = 11.0;
ax.typography.has_label_font_pt = true;
ax.typography.label_font_pt = 14.0;
ax.typography.has_title_font_pt = true;
ax.typography.title_font_pt = 16.0;
ax.typography.has_title_bold = true;
ax.typography.title_bold = true;

// Structured frame overrides
ax.frame.has_border_mask = true;
ax.frame.border_mask = 3; // left + bottom
ax.frame.has_border_line_width_pt = true;
ax.frame.border_line_width_pt = 0.8;
ax.frame.has_border_color = true;
ax.frame.border_color = "#1c1c1c";
ax.frame.has_ticks_out = true;
ax.frame.ticks_out = true;
ax.frame.has_ticks_mirror = true;
ax.frame.ticks_mirror = false;

// Typed objects/annotations
ax.labels.push_back({.text="region A", .at="graph 0.1,0.9", .font="Times,8"});
ax.arrows.push_back({.from="graph 0.2,0.8", .to="graph 0.35,0.65", .heads=true});
ax.rectangles.push_back({
  .from="graph 0.6,0.1",
  .to="graph 0.9,0.3",
  .has_fill_opacity=true,
  .fill_opacity=0.15,
  .fill_color="#99ccee"
});

// Typed equation/callout annotations
ax.equations.push_back({
  .expression = "y = mx + b",
  .at = {.system = gnuplotpp::CoordSystem::Graph, .x = 0.05, .y = 0.93},
  .boxed = true
});
ax.callouts.push_back({
  .text = "trend",
  .from = {.system = gnuplotpp::CoordSystem::Graph, .x = 0.70, .y = 0.80},
  .to = {.system = gnuplotpp::CoordSystem::Data, .x = 15.0, .y = 1.2}
});

// Optional advanced gnuplot commands (annotation, arrows, etc.)
ax.gnuplot_commands = {
  "set label 1 'e_p(t)=e_0 e^{-{/Symbol l} t}' at 14,10.8 font 'Times,8' front",
  "set arrow 1 from 18,10.0 to 40,2.857 lw 1.3 lc rgb '#000000' front"
};
```

## Series-Level Controls (`SeriesSpec`)

```cpp
gnuplotpp::SeriesSpec s;
s.type = gnuplotpp::SeriesType::Line; // Line, Scatter, ErrorBars, Band, Histogram, Heatmap
s.label = "SRIF";
s.has_line_width = true;
s.line_width_pt = 1.6;
s.has_color = true;
s.color = "#000000";       // RGB hex
s.has_opacity = true;
s.opacity = 0.15;          // [0,1], converted to ARGB for gnuplot
```

Additional structured adders:

```cpp
fig.axes(0).add_band(spec_band, x, y_low, y_high);
fig.axes(0).add_histogram(spec_hist, bins, counts);
fig.axes(0).add_heatmap(spec_heat, x, y, z);

auto kde = gnuplotpp::gaussian_kde(samples, bins);
fig.axes(0).add_series({.label="KDE"}, bins, kde);

std::vector<double> ex, ep;
gnuplotpp::ecdf(samples, ex, ep);
fig.axes(0).add_series({.label="ECDF", .use_y2=true}, ex, ep);

auto smoothed = gnuplotpp::moving_average(signal, 8);
auto reduced = gnuplotpp::downsample_uniform(signal, 4);
auto ac = gnuplotpp::autocorrelation(signal, 50);
auto fit = gnuplotpp::linear_fit(x, y);
auto yfit = gnuplotpp::linear_fit_line(fit, x);
gnuplotpp::add_linear_fit_overlay(fig.axes(0), x, y, "linear fit");

gnuplotpp::TransformPipeline pipeline;
pipeline.set_input(signal).rolling_mean(5).zscore().clip(-3.0, 3.0);

std::vector<std::vector<double>> fan_lo, fan_hi;
gnuplotpp::fan_chart_bands(ensemble, {0.1, 0.25, 0.75, 0.9}, fan_lo, fan_hi);

std::vector<double> vy, vw;
gnuplotpp::violin_profile(samples, vy, vw, 120);

std::vector<double> q_theory, q_sample;
gnuplotpp::qq_plot_normal(samples, q_theory, q_sample);

auto box = gnuplotpp::box_summary(samples);

std::vector<double> ell_x, ell_y;
gnuplotpp::confidence_ellipse(x, y, 2.0, ell_x, ell_y, 200);
```

### CSV + Faceting

```cpp
auto tbl = gnuplotpp::read_csv_numeric("signals.csv");
if (tbl.has_column("time") && tbl.has_column("error")) {
  tbl.add_line(fig.axes(0), {.label = "error"}, "time", "error");
}
auto [rows, cols] = gnuplotpp::facet_grid(6);
fs.rows = rows;
fs.cols = cols;
gnuplotpp::Figure fig(fs);
gnuplotpp::AxesSpec base;
base.xlabel = gnuplotpp::label_with_unit("time", "s");
base.ylabel = gnuplotpp::label_with_unit("error", "m");
gnuplotpp::apply_facet_axes(fig, base, {"A","B","C","D","E","F"});

// Composition helpers
gnuplotpp::apply_panel_titles(fig, {"(a)", "(b)", "(c)", "(d)", "(e)", "(f)"});
gnuplotpp::LegendSpec shared_legend;
shared_legend.enabled = true;
shared_legend.position = gnuplotpp::LegendPosition::TopLeft;
gnuplotpp::apply_shared_legend(fig, shared_legend, 0);
gnuplotpp::apply_shared_colorbar_label(fig, "density", rows * cols - 1);
```

### YAML Figure Spec

```cpp
auto ys = gnuplotpp::load_yaml_figure_spec(
    "figure.yaml",
    gnuplotpp::YamlLoadOptions{.strict_unknown_keys = true});
gnuplotpp::Figure fig(ys.figure);
if (!ys.axes.empty()) {
  fig.axes(0).set(ys.axes[0]);
}
```

Example YAML (including nested style overrides):

```yaml
figure:
  title: "YAML quick figure"
  preset: IEEE_SingleColumn
  formats: [pdf, svg]
axes:
  - title: "Signal"
    xlabel: "t [s]"
    ylabel: "x(t)"
    legend_spec:
      position: top_left
      columns: 2
      boxed: true
      font_pt: 8.0
    typography:
      tick_font_pt: 10.0
      label_font_pt: 12.0
      title_font_pt: 14.0
      title_bold: true
    frame:
      border_mask: 3
      border_line_width_pt: 0.9
      border_color: "#222222"
      ticks_out: true
      ticks_mirror: false
```

Compatibility note:
- Legacy flat typography fields on `AxesSpec` are still accepted in C++, but are deprecated.
- Prefer `ax.typography.*` and `ax.frame.*` for all new code and YAML specs.

## Data and Rendering

```cpp
std::vector<double> x{...};
std::vector<double> y{...};

gnuplotpp::Figure fig(fs);
fig.axes(0).set(ax);
fig.axes(0).add_series(s, x, y);

fig.set_backend(gnuplotpp::make_gnuplot_backend());
auto result = fig.save("out/my_run/figures");
```

## Quickstart Helpers

```cpp
using namespace gnuplotpp;
auto fig = make_quick_figure(QuickFigureOptions{
    .preset = Preset::Custom,
    .profile = StyleProfile::Tufte_Minimal,
    .title = "Quick Plot"});
fig.axes(0).set(make_quick_axes("Response", "t [s]", "x(t)", false, true));
fig.axes(0).add_series({.label = "signal"}, x, y);
```

## Figure Recipes

### 0) Monte-Carlo Density (Many Lines, No Legend, Low Opacity)

```cpp
gnuplotpp::AxesSpec ax;
ax.legend = false;
ax.grid = true;
fig.axes(0).set(ax);

for (int k = 0; k < 1000; ++k) {
  gnuplotpp::SeriesSpec s;
  s.type = gnuplotpp::SeriesType::Line;
  s.label = "";
  s.has_line_width = true;
  s.line_width_pt = 0.5;
  s.has_color = true;
  s.color = "#000000";
  s.has_opacity = true;
  s.opacity = 0.08;
  fig.axes(0).add_series(s, t, y_mc[k]);
}
```

Practical note:
- For very large ensembles (e.g., 1000 lines), avoid high alpha like `0.3` with thick lines; overlap will saturate to near-black.
- Start around `opacity=0.01..0.05` and adjust.

### 1) Single IEEE Plot (1x1)

```cpp
gnuplotpp::FigureSpec fs;
fs.preset = gnuplotpp::Preset::IEEE_SingleColumn;
gnuplotpp::apply_preset_defaults(fs);
fs.rows = 1; fs.cols = 1;
fs.formats = {gnuplotpp::OutputFormat::Pdf, gnuplotpp::OutputFormat::Svg};

gnuplotpp::Figure fig(fs);
```

### 2) Two-Panel Comparison (1x2)

```cpp
fs.preset = gnuplotpp::Preset::IEEE_DoubleColumn;
fs.rows = 1; fs.cols = 2;
```

Then configure `fig.axes(0,0)` and `fig.axes(0,1)` separately.

### 3) Four-Panel Overview (2x2)

```cpp
fs.preset = gnuplotpp::Preset::AIAA_Page;
fs.rows = 2; fs.cols = 2;
```

Then configure `fig.axes(0..3)`.

### 4) Log-Y Plot with Data-Driven Y-Limits

```cpp
double ymax = 1.05 * max_value;
ax.has_ylim = true;
ax.ymin = 1e-3;
ax.ymax = ymax;
ax.ylog = true;
```

### 5) Three Black Lines with Different Styles + Legend

With IEEE presets, the backend emits monochrome dashed line styles per series (`dt 1..N`).

```cpp
ax.legend = true;
fig.axes(0).add_series({.type=gnuplotpp::SeriesType::Line, .label="SRIF"}, t, y1);
fig.axes(0).add_series({.type=gnuplotpp::SeriesType::Line, .label="UKF"}, t, y2);
fig.axes(0).add_series({.type=gnuplotpp::SeriesType::Line, .label="EKF"}, t, y3);
```

## Output Artifacts

For each render:
- `figure.pdf`
- `figure.svg`
- `figure.eps`
- `figure.png` (recommended when line alpha is important)
- `tmp/figure.gp`
- `tmp/ax*_series*.dat`
- `tmp/gnuplot.log`
- `manifest.json` (when `FigureSpec::write_manifest=true`)

## Object Transparency (Native gnuplot Commands)

Use `AxesSpec::gnuplot_commands` for transparent filled objects:

```cpp
ax.gnuplot_commands = {
  "set object 1 rect from graph 0.05,0.80 to graph 0.35,0.95 "
  "fc rgb '#000000' fs transparent solid 0.10 noborder"
};
```

## Existing End-to-End Examples

- `examples/two_window_example.cpp`
- `examples/layout_2x2_example.cpp`
- `examples/three_line_ieee_example.cpp`
- `examples/monte_carlo_alpha_example.cpp`
- `examples/feature_rich_showcase.cpp`
- `examples/interactive_facet_example.cpp`
- `examples/yaml_spec_example.cpp`
