# gnuplotpp

Pure C++20 plotting API with a gnuplot renderer for publication-ready IEEE/AIAA figures.

## Requirements

- CMake >= 3.20
- C++20 compiler
- `gnuplot` (required for final figure generation)

Install `gnuplot`:

```bash
# macOS
brew install gnuplot

# Ubuntu/Debian
sudo apt-get update && sudo apt-get install -y gnuplot

# Fedora
sudo dnf install -y gnuplot
```

## Build

```bash
cmake --preset dev-debug
cmake --build --preset build-debug
ctest --preset test-debug
```

## Architecture

```mermaid
flowchart LR
  A[FigureSpec / AxesSpec / SeriesSpec] --> B[Figure / Axes]
  B --> C[IPlotBackend]
  C --> D[GnuplotBackend]
  D --> E[tmp/*.dat]
  D --> F[tmp/figure.gp]
  D --> G[figure.pdf / figure.svg / figure.eps]
```

## Publication Workflow

1. Configure a publication preset (`IEEE_SingleColumn`, `IEEE_DoubleColumn`, `AIAA_Column`, `AIAA_Page`).
2. Set output formats to vector (`Pdf`, `Svg`, optional `Eps`).
3. Render via `GnuplotBackend` from your C++ executable.
4. Use PDF as primary submission asset.

## Publication Defaults in Renderer

- Cairo terminals with enhanced text
- Explicit line widths and rounded joins
- Tick formatting and outward tics
- Grid styling suitable for print
- Escaped plot labels/titles for robust script generation

### Strict IEEE Mode

When using `IEEE_SingleColumn` or `IEEE_DoubleColumn`, renderer behavior is tightened to print-safe defaults:

- 8.5 pt Times-style text defaults
- monochrome plotting (`set monochrome`)
- dashed line-style differentiation per series (`dt 1..N`)
- SciencePlots-like axis defaults: inward mirrored ticks, visible minor ticks, thin 0.5 axis/grid strokes
- vector-first outputs (`PDF`, `SVG`, `EPS`)

## Examples

```bash
./build/dev-debug/two_window_example --out out/two_window
./build/dev-debug/layout_2x2_example --out out/layout_2x2
./build/dev-debug/three_line_ieee_example --out out/three_line_ieee
```

### Annotated IEEE Example

Generated from:

```bash
./build/dev-debug/three_line_ieee_example --out out/three_line_ieee_readme
```

![Three-line IEEE example](docs/images/three_line_ieee_example.svg)

Expected outputs:

- `out/<name>/figures/figure.pdf`
- `out/<name>/figures/figure.svg`
- `out/<name>/figures/figure.eps`
- `out/<name>/figures/tmp/figure.gp`
- `out/<name>/figures/tmp/ax*_series*.dat`

### Axis Controls

Set axis limits and log scale using `AxesSpec`:

```cpp
gnuplotpp::AxesSpec ax;
ax.has_xlim = true;
ax.xmin = 0.0;
ax.xmax = 100.0;
ax.has_ylim = true;
ax.ymin = 1e-3;
ax.ymax = 10.0;
ax.ylog = true;
```

## Error Handling

`RenderResult.status` values:
- `Success`
- `InvalidInput`
- `IoError`
- `ExternalToolMissing`
- `ExternalToolFailure`
- `UnsupportedFormat`
