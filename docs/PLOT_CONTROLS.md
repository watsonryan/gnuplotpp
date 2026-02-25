# Plot Controls

This document is the full control reference for `gnuplotpp` plotting.

## Core Types

- `gnuplotpp::FigureSpec`
- `gnuplotpp::AxesSpec`
- `gnuplotpp::SeriesSpec`
- `gnuplotpp::Figure`
- `gnuplotpp::Axes`

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

## Axes-Level Controls (`AxesSpec`)

```cpp
gnuplotpp::AxesSpec ax;
ax.title = "Position Error Norm";
ax.xlabel = "t [s]";
ax.ylabel = "||e_p|| [m]";
ax.grid = true;
ax.legend = true;

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

// Optional advanced gnuplot commands (annotation, arrows, etc.)
ax.gnuplot_commands = {
  "set label 1 'e_p(t)=e_0 e^{-{/Symbol l} t}' at 14,10.8 font 'Times,8' front",
  "set arrow 1 from 18,10.0 to 40,2.857 lw 1.3 lc rgb '#000000' front"
};
```

## Series-Level Controls (`SeriesSpec`)

```cpp
gnuplotpp::SeriesSpec s;
s.type = gnuplotpp::SeriesType::Line; // Line, Scatter, ErrorBars, Band
s.label = "SRIF";
s.has_line_width = true;
s.line_width_pt = 1.6;
```

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

## Figure Recipes

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
- `tmp/figure.gp`
- `tmp/ax*_series*.dat`
- `tmp/gnuplot.log`

## Existing End-to-End Examples

- `examples/two_window_example.cpp`
- `examples/layout_2x2_example.cpp`
- `examples/three_line_ieee_example.cpp`
