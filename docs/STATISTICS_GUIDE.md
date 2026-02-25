# Statistical Plot Guide

This guide covers common stats visuals in `gnuplotpp`.

## Q-Q Plot vs Normal

```cpp
std::vector<double> q_theory, q_sample;
gnuplotpp::qq_plot_normal(samples, q_theory, q_sample);
fig.axes(0).add_series({.type = gnuplotpp::SeriesType::Scatter, .label = "Samples"},
                       q_theory, q_sample);
```

## Violin Profile

```cpp
std::vector<double> y_grid, half_w;
gnuplotpp::violin_profile(samples, y_grid, half_w, 180);
```

Use `half_w` to draw left/right contours, filled body, and optional raw sample jitter overlay.

## Box Summary (Tukey)

```cpp
auto box = gnuplotpp::box_summary(samples);
```

Use this for:

- box from `q1` to `q3`
- median line
- whiskers from `whisker_low` to `whisker_high`

## Confidence Ellipse (1/2/3 sigma)

```cpp
std::vector<double> ex1, ey1, ex2, ey2, ex3, ey3;
gnuplotpp::confidence_ellipse(x, y, 1.0, ex1, ey1, 240);
gnuplotpp::confidence_ellipse(x, y, 2.0, ex2, ey2, 240);
gnuplotpp::confidence_ellipse(x, y, 3.0, ex3, ey3, 240);
```

## KDE + Histogram

```cpp
auto kde = gnuplotpp::gaussian_kde(samples, bins);
fig.axes(0).add_histogram(hist_spec, bins, counts);
fig.axes(0).add_series({.label = "KDE"}, bins, kde_scaled);
```

## ECDF

```cpp
std::vector<double> x_sorted, p;
gnuplotpp::ecdf(samples, x_sorted, p);
fig.axes(0).add_series({.label="ECDF"}, x_sorted, p);
```

## Autocorrelation

```cpp
auto ac = gnuplotpp::autocorrelation(signal, 60);
```

Render as bars and optionally overlay confidence bounds.

## Monte Carlo Envelope (+/-3 sigma)

For AR(1)-like processes you can plot theoretical envelope:

```cpp
const double var_k = (sigma_w*sigma_w) * (1.0 - std::pow(phi*phi, k)) / (1.0 - phi*phi);
const double three_sigma = 3.0 * std::sqrt(var_k);
```

Then add upper/lower envelope curves as separate series.

## Output Recommendation

- Use `PDF`/`SVG` for publication vectors.
- Use `PNG` when alpha blending is critical.
