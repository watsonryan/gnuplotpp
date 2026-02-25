# Styling and Presets

This guide explains how to get a defined look quickly and override only what you need.

## Two Layers

1. `Preset` sets baseline size and publication-oriented defaults.
2. `StyleProfile` sets the visual language (font, line hierarchy, grid, typography scales, title weight).

## Common Setup

```cpp
gnuplotpp::FigureSpec fs;
fs.preset = gnuplotpp::Preset::Custom;
gnuplotpp::apply_preset_defaults(fs);
gnuplotpp::apply_style_profile(fs, gnuplotpp::StyleProfile::Tufte_Minimal);
```

## Available Profiles

- `Science`
- `IEEE_Strict`
- `AIAA_Strict`
- `Presentation`
- `DarkPrintSafe`
- `Tufte_Minimal`

## Typography Model

Typography is preset/profile-driven by default:

- `style.font_pt` is base font size.
- `style.tick_font_scale` scales tick labels.
- `style.label_font_scale` scales axis labels.
- `style.title_font_scale` scales plot titles.
- `style.title_bold` controls title weight.

If needed, override per-axis:

```cpp
ax.has_tick_font_pt = true;
ax.tick_font_pt = 11.0;
ax.has_label_font_pt = true;
ax.label_font_pt = 14.0;
ax.has_title_font_pt = true;
ax.title_font_pt = 16.0;
ax.has_title_bold = true;
ax.title_bold = true;
```

## Legend Control

```cpp
ax.legend = true;
ax.legend_spec.position = gnuplotpp::LegendPosition::TopLeft;
ax.legend_spec.boxed = true;
ax.legend_spec.opaque = true;
ax.legend_spec.has_font_pt = true;
ax.legend_spec.font_pt = 10.0;
```

## Grid and Frame Discipline

For low-ink figures (Tufte-like):

- keep `ax.grid = false`
- use `set border 3` via `ax.gnuplot_commands` for left/bottom axes only
- label lines directly or keep legend minimal

## Color and Monochrome

- IEEE presets may enforce monochrome.
- Per-series `has_color`/`has_opacity` can intentionally override.
- For dense ensembles, keep alpha low and line width moderate.
