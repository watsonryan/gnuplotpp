# Getting Started

## Requirements

- CMake >= 3.20
- C++20 compiler
- `gnuplot` on `PATH`

Install gnuplot:

```bash
# macOS
brew install gnuplot

# Ubuntu/Debian
sudo apt-get update && sudo apt-get install -y gnuplot
```

## Build

```bash
cmake --preset dev-debug
cmake --build --preset build-debug
ctest --preset test-debug
```

## First Plot

Run a ready example:

```bash
./build/dev-debug/two_window_example --out out/two_window
```

Output files appear under:

- `out/two_window/figures/figure.pdf`
- `out/two_window/figures/figure.svg`
- `out/two_window/figures/tmp/figure.gp`
- `out/two_window/figures/tmp/ax*_series*.dat`

## Minimal C++ Flow

```cpp
gnuplotpp::FigureSpec fs;
fs.preset = gnuplotpp::Preset::IEEE_SingleColumn;
gnuplotpp::apply_preset_defaults(fs);
fs.formats = {gnuplotpp::OutputFormat::Pdf, gnuplotpp::OutputFormat::Svg};

gnuplotpp::Figure fig(fs);
gnuplotpp::AxesSpec ax;
ax.xlabel = "t [s]";
ax.ylabel = "x(t)";
fig.axes(0).set(ax);

fig.axes(0).add_series({.label = "signal"}, t, y);
fig.set_backend(gnuplotpp::make_gnuplot_backend());
auto result = fig.save("out/my_first_plot/figures");
```

## Important Notes

- Use `PNG` when you need line alpha/transparency to be exact.
- Prefer `PDF`/`SVG` for publication vector export.
- If rendering fails, inspect `tmp/gnuplot.log` next to generated script/data.
