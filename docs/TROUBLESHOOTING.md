# Troubleshooting

## "gnuplot failed; inspect gnuplot.log"

Check:

- `out/<run>/figures/tmp/gnuplot.log`
- `out/<run>/figures/tmp/figure.gp`

The log usually points to the exact command and line that failed.

## Transparency Looks Wrong

Known behavior:

- `PNG` backend is most reliable for line alpha.
- `PDF`/`SVG`/`EPS` may ignore or approximate line transparency depending on terminal.

Recommendation:

- For density plots with alpha, export `PNG` in addition to vector formats.

## LaTeX/Symbol Text Not Rendering

- Use `TextMode::Enhanced` for gnuplot enhanced syntax.
- For symbols in labels use enhanced notation like `{/Symbol s}`.
- If you need full LaTeX, configure terminal/text pipeline accordingly.

## Legend Is Cluttered

- Empty labels are emitted as `notitle`.
- Keep only key representative series labeled.
- Use top-left or outside-right placement for dense overlays.

## Figure Looks Different Across Machines

- Font availability differs.
- Set `FigureSpec::font_fallbacks` to a robust list.
- Keep a style profile and explicit output sizes in inches.

## Interactive Preview Says Mouse Not Active

Enable crosshair/mouse in axes:

```cpp
ax.enable_crosshair = true;
```

Then run the emitted preview script in gnuplot terminal supporting mouse interaction.

## Build Works but Example Missing

- Confirm `BUILD_EXAMPLES=ON` (enabled in provided presets).
- Reconfigure if `CMakeLists.txt` changed:

```bash
cmake --preset dev-debug
cmake --build --preset build-debug
```
