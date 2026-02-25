# Best Practices Quick Reference

This page is a short checklist for making plots quickly while keeping them publication-ready and consistent.

## 1) Start From Preset + Profile

```cpp
gnuplotpp::FigureSpec fs;
fs.preset = gnuplotpp::Preset::IEEE_SingleColumn;
gnuplotpp::apply_preset_defaults(fs);
gnuplotpp::apply_style_profile(fs, gnuplotpp::StyleProfile::IEEE_Strict);
```

Use per-axis overrides only when necessary.

## 2) Keep Typography Hierarchy Consistent

- Base size from `fs.style.font_pt`
- Labels slightly larger than ticks
- Title larger than labels

Prefer structured overrides:

```cpp
ax.typography.has_tick_font_pt = true;
ax.typography.tick_font_pt = 9.0;
ax.typography.has_label_font_pt = true;
ax.typography.label_font_pt = 11.0;
ax.typography.has_title_font_pt = true;
ax.typography.title_font_pt = 13.0;
```

## 3) Dense Monte Carlo: Use Low Alpha

For many traces, high alpha saturates to black.

Recommended starting point:

```cpp
gnuplotpp::SeriesSpec s;
s.type = gnuplotpp::SeriesType::Line;
s.has_color = true;
s.color = "#000000";
s.has_line_width = true;
s.line_width_pt = 1.0;
s.has_opacity = true;
s.opacity = 0.02;  // tune upward if needed
```

## 4) Use Vector Outputs First

- Primary: `PDF`
- Also useful: `SVG`, `EPS`
- Use `PNG` for previews/raster-only flows

```cpp
fs.formats = {gnuplotpp::OutputFormat::Pdf, gnuplotpp::OutputFormat::Svg};
```

## 5) Keep Axes Clean

- Use `ax.frame.border_mask = 3` (left + bottom) for minimal/Tufte-like style.
- Disable mirrored ticks unless explicitly needed.
- Use major/minor ticks intentionally; avoid unnecessary grid density.

## 6) YAML in CI: Strict Mode

```cpp
auto ys = gnuplotpp::load_yaml_figure_spec(
    "figure.yaml",
    gnuplotpp::YamlLoadOptions{.strict_unknown_keys = true});
```

This catches typo keys and silent config drift.

## Common Recipes

### Three-Line IEEE Plot (Black, Distinct Styles)

1. Use `Preset::IEEE_SingleColumn` + `StyleProfile::IEEE_Strict`.
2. Keep monochrome with differentiated dash types.
3. Use concise legend labels and units in axis labels.

### Confidence Band Plot

1. Mean line (`Line`) with high contrast.
2. Band via `add_band(...)` with low opacity.
3. Observations as small points on top.

### 1/2/3-Sigma Ellipse Plot

1. Plot samples in gray with small point size.
2. Draw sigma contours with distinct colors/line widths.
3. Keep legend compact in top-left.

## Anti-Patterns

- Hardcoding font sizes per plot instead of using preset/profile baseline.
- Using high alpha (`>= 0.2`) for very dense ensembles.
- Overusing `gnuplot_commands` for things already covered by typed API fields.
- Mixing many style overrides across each subplot without a shared baseline.
- Allowing unknown YAML keys (non-strict mode) in production config.

## Practical Workflow

1. Build style baseline once with preset/profile.
2. Add data + minimal axis labels.
3. Only then apply targeted per-axis overrides.
4. Review exported `PDF` at final publication scale.
