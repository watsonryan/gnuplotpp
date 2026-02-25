# Examples Cookbook

This file maps repository examples to practical use-cases.

## Core Examples

- `two_window_example.cpp`: simple multi-panel starter.
- `layout_2x2_example.cpp`: four-panel grid layout.
- `three_line_ieee_example.cpp`: IEEE-like black-and-white comparison.
- `tufte_minimal_example.cpp`: low-ink direct-label style.

## Advanced Examples

- `feature_rich_showcase.cpp`: confidence bands, histogram+KDE, heatmap, y2 axis.
- `interactive_facet_example.cpp`: facet grid + interactive preview script.
- `interactive_facet_example.cpp`: facet grid + shared legend + auto legend placement + DataTable column plotting.
- `yaml_spec_example.cpp`: declarative plotting from YAML.
- `stats_plot_examples.cpp`: Q-Q, violin, box summary, confidence ellipses, autocorrelation.
- `monte_carlo_alpha_example.cpp`: dense ensemble rendering + 3-sigma envelope.

## Typical Commands

```bash
./build/dev-debug/three_line_ieee_example --out out/three_line_ieee
./build/dev-debug/tufte_minimal_example --out out/tufte_minimal_example
./build/dev-debug/stats_plot_examples --out out/stats_plot_examples
./build/dev-debug/monte_carlo_alpha_example --out out/monte_carlo_alpha_example --npaths 1000 --lw 2 --alpha 0.3
./build/dev-debug/yaml_spec_example --out out/yaml_spec_example
```

## Where Outputs Go

Each example writes to:

- `out/<example_name>/figures/figure.pdf`
- `out/<example_name>/figures/figure.svg`
- `out/<example_name>/figures/figure.png` (if enabled)
- `out/<example_name>/figures/tmp/figure.gp`
- `out/<example_name>/figures/tmp/ax*_series*.dat`

## Reuse Pattern

When creating a new example, copy one close to your use-case:

1. Keep `FigureSpec` + profile setup.
2. Replace data generation with your source.
3. Keep `fig.save(out_dir / "figures")` convention.

## New Composition + DataTable Pattern

```cpp
auto tbl = gnuplotpp::read_csv_numeric("demo.csv");
gnuplotpp::apply_facet_axes(fig, base, {"A", "B", "C", "D"});
gnuplotpp::apply_panel_titles(fig, {"A", "B", "C", "D"});

for (int k = 0; k < 4; ++k) {
  const auto& y = tbl.column(cols_name[k]);
  auto ax_spec = fig.axes(k).spec();
  gnuplotpp::auto_place_legend(ax_spec, tbl.column("t"), y);
  fig.axes(k).set(ax_spec);
  tbl.add_line(fig.axes(k), {.label = "mean"}, "t", cols_name[k]);
}

gnuplotpp::LegendSpec shared_legend;
shared_legend.enabled = true;
shared_legend.position = gnuplotpp::LegendPosition::TopLeft;
gnuplotpp::apply_shared_legend(fig, shared_legend, 0);
```
