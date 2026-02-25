# API Contracts

This document defines expected behavior for key public helpers.

## Include Surface

- Preferred full include: `#include "gnuplotpp/api.hpp"`
- Internal headers in `src/` are not public API and can change without notice.

## Rendering Contract

### `gnuplotpp::validate_figure_for_render`

Minimal usage:

```cpp
gnuplotpp::RenderResult vr = gnuplotpp::validate_figure_for_render(fig);
if (!vr.ok) {
  // vr.status == RenderStatus::InvalidInput
}
```

Failure modes:
- negative/zero layout dimensions
- empty output formats
- per-series shape mismatches
- invalid opacity values

Expected behavior:
- no I/O, pure validation
- deterministic error message in `vr.message`

### `gnuplotpp::Figure::save`

Expected behavior:
- validates figure first
- returns `InvalidInput` before backend render when invalid
- returns backend-provided status otherwise

## DataTable Contract

### `DataTable::column`

Failure mode:
- throws `std::out_of_range` with available column names when missing.

### `DataTable::require_columns`

Minimal usage:

```cpp
tbl.require_columns({"t", "signal"});
```

Failure mode:
- throws `std::out_of_range` listing missing and available columns.

### `DataTable::add_line` / `add_scatter`

Expected behavior:
- resolves named columns and appends one series
- throws when column missing or x/y lengths differ

## Style Resolution Contract

Renderer style precedence (highest to lowest):
1. Axis explicit override (`ax.typography.*`, `ax.frame.*`, `ax.legend_spec.*`)
2. Axis legacy compatibility fields (normalized by `Axes::set`)
3. Figure style defaults from preset/profile

Expected behavior:
- consistent style resolution for title/label/tick/legend/frame in one pass
- format/export policy applied after style resolution

## Model Overlay Contract

### `add_linear_fit_overlay`
- adds one line series representing least-squares fit
- returns fit summary (`slope`, `intercept`, `r2`)

### `add_polynomial_fit_overlay`
- same behavior with polynomial degree `d >= 1`
- degree `1` is equivalent to linear form

Failure modes:
- ill-conditioned fit returns empty/default coefficients/result

## Transforms Contract

### `TransformPipeline`

Minimal usage:

```cpp
gnuplotpp::TransformPipeline p;
p.set_input(data).rolling_mean(5).zscore().clip(-3.0, 3.0);
```

Expected behavior:
- operations apply in chain order
- each op replaces internal vector
- empty input yields empty output
