#include "gnuplotpp/statistics.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <limits>

namespace gnuplotpp {

std::vector<double> gaussian_kde(std::span<const double> samples,
                                 std::span<const double> x_grid,
                                 double bandwidth) {
  if (samples.empty() || x_grid.empty()) {
    return {};
  }

  const std::size_t n = samples.size();
  const double mean = std::accumulate(samples.begin(), samples.end(), 0.0) / static_cast<double>(n);
  double var = 0.0;
  for (const double v : samples) {
    const double d = v - mean;
    var += d * d;
  }
  var /= std::max<std::size_t>(1, n - 1);
  const double sigma = std::sqrt(std::max(1.0e-16, var));

  double h = bandwidth;
  if (h <= 0.0) {
    h = 1.06 * sigma * std::pow(static_cast<double>(n), -0.2);
  }
  h = std::max(1.0e-9, h);

  constexpr double inv_sqrt_2pi = 0.3989422804014327;
  const double norm = 1.0 / (static_cast<double>(n) * h);
  std::vector<double> density(x_grid.size(), 0.0);

  for (std::size_t i = 0; i < x_grid.size(); ++i) {
    const double x = x_grid[i];
    double acc = 0.0;
    for (const double s : samples) {
      const double u = (x - s) / h;
      acc += inv_sqrt_2pi * std::exp(-0.5 * u * u);
    }
    density[i] = norm * acc;
  }
  return density;
}

void ecdf(std::span<const double> samples, std::vector<double>& x_sorted, std::vector<double>& p) {
  x_sorted.assign(samples.begin(), samples.end());
  std::sort(x_sorted.begin(), x_sorted.end());
  p.resize(x_sorted.size());
  const double n = static_cast<double>(x_sorted.size());
  for (std::size_t i = 0; i < x_sorted.size(); ++i) {
    p[i] = (static_cast<double>(i) + 1.0) / n;
  }
}

void percentile_band(const std::vector<std::vector<double>>& ensemble,
                     double p_low,
                     double p_high,
                     std::vector<double>& low,
                     std::vector<double>& high) {
  if (ensemble.empty()) {
    low.clear();
    high.clear();
    return;
  }
  const std::size_t n = ensemble.front().size();
  low.resize(n);
  high.resize(n);
  const std::size_t i_low =
      static_cast<std::size_t>(std::clamp(p_low, 0.0, 1.0) * static_cast<double>(ensemble.size() - 1));
  const std::size_t i_high = static_cast<std::size_t>(std::clamp(p_high, 0.0, 1.0) *
                                                      static_cast<double>(ensemble.size() - 1));

  std::vector<double> col(ensemble.size());
  for (std::size_t j = 0; j < n; ++j) {
    for (std::size_t i = 0; i < ensemble.size(); ++i) {
      col[i] = ensemble[i][j];
    }
    std::sort(col.begin(), col.end());
    low[j] = col[i_low];
    high[j] = col[i_high];
  }
}

void fan_chart_bands(const std::vector<std::vector<double>>& ensemble,
                     const std::vector<double>& quantiles,
                     std::vector<std::vector<double>>& lows,
                     std::vector<std::vector<double>>& highs) {
  lows.clear();
  highs.clear();
  if (quantiles.size() < 2 || quantiles.size() % 2 != 0) {
    return;
  }
  const std::size_t half = quantiles.size() / 2;
  lows.resize(half);
  highs.resize(half);
  for (std::size_t i = 0; i < half; ++i) {
    const double ql = quantiles[i];
    const double qh = quantiles[quantiles.size() - 1 - i];
    percentile_band(ensemble, ql, qh, lows[i], highs[i]);
  }
}

void violin_profile(std::span<const double> samples,
                    std::vector<double>& y_grid,
                    std::vector<double>& half_width,
                    std::size_t points) {
  y_grid.clear();
  half_width.clear();
  if (samples.empty() || points < 2) {
    return;
  }
  const auto [it_min, it_max] = std::minmax_element(samples.begin(), samples.end());
  const double y_min = *it_min;
  const double y_max = *it_max;
  y_grid.resize(points);
  for (std::size_t i = 0; i < points; ++i) {
    y_grid[i] = y_min + (y_max - y_min) * static_cast<double>(i) / static_cast<double>(points - 1);
  }
  half_width = gaussian_kde(samples, y_grid);
  const double peak = *std::max_element(half_width.begin(), half_width.end());
  if (peak > 0.0) {
    for (double& v : half_width) {
      v /= peak;
    }
  }
}

std::vector<double> moving_average(std::span<const double> y, std::size_t window) {
  if (y.empty() || window == 0) {
    return {};
  }
  std::vector<double> out(y.size(), 0.0);
  double acc = 0.0;
  for (std::size_t i = 0; i < y.size(); ++i) {
    acc += y[i];
    if (i >= window) {
      acc -= y[i - window];
    }
    const std::size_t w = std::min(window, i + 1);
    out[i] = acc / static_cast<double>(w);
  }
  return out;
}

std::vector<double> downsample_uniform(std::span<const double> y, std::size_t k) {
  if (k == 0 || y.empty()) {
    return {};
  }
  std::vector<double> out;
  out.reserve((y.size() + k - 1) / k);
  for (std::size_t i = 0; i < y.size(); i += k) {
    out.push_back(y[i]);
  }
  return out;
}

std::vector<double> autocorrelation(std::span<const double> y, std::size_t max_lag) {
  if (y.empty()) {
    return {};
  }
  const std::size_t n = y.size();
  const std::size_t m = std::min(max_lag, n - 1);
  const double mean = std::accumulate(y.begin(), y.end(), 0.0) / static_cast<double>(n);
  double var = 0.0;
  for (double v : y) {
    const double d = v - mean;
    var += d * d;
  }
  if (var <= 0.0) {
    return std::vector<double>(m + 1, 0.0);
  }

  std::vector<double> ac(m + 1, 0.0);
  for (std::size_t lag = 0; lag <= m; ++lag) {
    double c = 0.0;
    for (std::size_t i = 0; i + lag < n; ++i) {
      c += (y[i] - mean) * (y[i + lag] - mean);
    }
    ac[lag] = c / var;
  }
  return ac;
}

namespace {
double inv_norm_cdf(const double p) {
  // Acklam approximation.
  static constexpr double a1 = -3.969683028665376e+01;
  static constexpr double a2 = 2.209460984245205e+02;
  static constexpr double a3 = -2.759285104469687e+02;
  static constexpr double a4 = 1.383577518672690e+02;
  static constexpr double a5 = -3.066479806614716e+01;
  static constexpr double a6 = 2.506628277459239e+00;
  static constexpr double b1 = -5.447609879822406e+01;
  static constexpr double b2 = 1.615858368580409e+02;
  static constexpr double b3 = -1.556989798598866e+02;
  static constexpr double b4 = 6.680131188771972e+01;
  static constexpr double b5 = -1.328068155288572e+01;
  static constexpr double c1 = -7.784894002430293e-03;
  static constexpr double c2 = -3.223964580411365e-01;
  static constexpr double c3 = -2.400758277161838e+00;
  static constexpr double c4 = -2.549732539343734e+00;
  static constexpr double c5 = 4.374664141464968e+00;
  static constexpr double c6 = 2.938163982698783e+00;
  static constexpr double d1 = 7.784695709041462e-03;
  static constexpr double d2 = 3.224671290700398e-01;
  static constexpr double d3 = 2.445134137142996e+00;
  static constexpr double d4 = 3.754408661907416e+00;
  static constexpr double p_low = 0.02425;
  static constexpr double p_high = 1 - p_low;

  if (p <= 0.0) return -std::numeric_limits<double>::infinity();
  if (p >= 1.0) return std::numeric_limits<double>::infinity();
  double q = 0.0;
  double r = 0.0;
  if (p < p_low) {
    q = std::sqrt(-2 * std::log(p));
    return (((((c1 * q + c2) * q + c3) * q + c4) * q + c5) * q + c6) /
           ((((d1 * q + d2) * q + d3) * q + d4) * q + 1);
  }
  if (p > p_high) {
    q = std::sqrt(-2 * std::log(1 - p));
    return -(((((c1 * q + c2) * q + c3) * q + c4) * q + c5) * q + c6) /
           ((((d1 * q + d2) * q + d3) * q + d4) * q + 1);
  }
  q = p - 0.5;
  r = q * q;
  return (((((a1 * r + a2) * r + a3) * r + a4) * r + a5) * r + a6) * q /
         (((((b1 * r + b2) * r + b3) * r + b4) * r + b5) * r + 1);
}
}  // namespace

void qq_plot_normal(std::span<const double> samples,
                    std::vector<double>& theo,
                    std::vector<double>& samp) {
  samp.assign(samples.begin(), samples.end());
  std::sort(samp.begin(), samp.end());
  theo.resize(samp.size());
  const double n = static_cast<double>(samp.size());
  for (std::size_t i = 0; i < samp.size(); ++i) {
    const double p = (static_cast<double>(i) + 0.5) / n;
    theo[i] = inv_norm_cdf(p);
  }
}

BoxSummary box_summary(std::span<const double> samples) {
  BoxSummary b{};
  if (samples.empty()) {
    return b;
  }
  std::vector<double> v(samples.begin(), samples.end());
  std::sort(v.begin(), v.end());
  const auto q_at = [&](double p) {
    const double x = p * static_cast<double>(v.size() - 1);
    const std::size_t i0 = static_cast<std::size_t>(std::floor(x));
    const std::size_t i1 = static_cast<std::size_t>(std::ceil(x));
    const double t = x - static_cast<double>(i0);
    return v[i0] * (1.0 - t) + v[i1] * t;
  };
  b.q1 = q_at(0.25);
  b.median = q_at(0.5);
  b.q3 = q_at(0.75);
  const double iqr = b.q3 - b.q1;
  const double lo = b.q1 - 1.5 * iqr;
  const double hi = b.q3 + 1.5 * iqr;
  b.whisker_low = v.front();
  b.whisker_high = v.back();
  for (double e : v) {
    if (e >= lo) {
      b.whisker_low = e;
      break;
    }
  }
  for (auto it = v.rbegin(); it != v.rend(); ++it) {
    if (*it <= hi) {
      b.whisker_high = *it;
      break;
    }
  }
  return b;
}

void confidence_ellipse(std::span<const double> x,
                        std::span<const double> y,
                        const double nsigma,
                        std::vector<double>& x_ellipse,
                        std::vector<double>& y_ellipse,
                        const std::size_t points) {
  x_ellipse.clear();
  y_ellipse.clear();
  if (x.size() != y.size() || x.empty() || points < 4) {
    return;
  }
  const std::size_t n = x.size();
  double mx = 0.0;
  double my = 0.0;
  for (std::size_t i = 0; i < n; ++i) {
    mx += x[i];
    my += y[i];
  }
  mx /= static_cast<double>(n);
  my /= static_cast<double>(n);

  double sxx = 0.0, syy = 0.0, sxy = 0.0;
  for (std::size_t i = 0; i < n; ++i) {
    const double dx = x[i] - mx;
    const double dy = y[i] - my;
    sxx += dx * dx;
    syy += dy * dy;
    sxy += dx * dy;
  }
  const double den = static_cast<double>(std::max<std::size_t>(1, n - 1));
  sxx /= den;
  syy /= den;
  sxy /= den;

  const double tr = sxx + syy;
  const double det = sxx * syy - sxy * sxy;
  const double disc = std::sqrt(std::max(0.0, tr * tr * 0.25 - det));
  const double l1 = std::max(0.0, tr * 0.5 + disc);
  const double l2 = std::max(0.0, tr * 0.5 - disc);
  const double angle = 0.5 * std::atan2(2.0 * sxy, sxx - syy);
  const double ca = std::cos(angle);
  const double sa = std::sin(angle);
  const double a = nsigma * std::sqrt(l1);
  const double b = nsigma * std::sqrt(l2);

  x_ellipse.resize(points);
  y_ellipse.resize(points);
  constexpr double two_pi = 6.28318530717958647692;
  for (std::size_t i = 0; i < points; ++i) {
    const double th = two_pi * static_cast<double>(i) / static_cast<double>(points - 1);
    const double ex = a * std::cos(th);
    const double ey = b * std::sin(th);
    x_ellipse[i] = mx + ca * ex - sa * ey;
    y_ellipse[i] = my + sa * ex + ca * ey;
  }
}

LinearFitResult linear_fit(std::span<const double> x, std::span<const double> y) {
  LinearFitResult fit{};
  if (x.size() != y.size() || x.size() < 2) {
    return fit;
  }
  const std::size_t n = x.size();
  double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
  for (std::size_t i = 0; i < n; ++i) {
    sx += x[i];
    sy += y[i];
    sxx += x[i] * x[i];
    sxy += x[i] * y[i];
  }
  const double den = static_cast<double>(n) * sxx - sx * sx;
  if (std::abs(den) < 1e-15) {
    return fit;
  }
  fit.slope = (static_cast<double>(n) * sxy - sx * sy) / den;
  fit.intercept = (sy - fit.slope * sx) / static_cast<double>(n);

  const double y_mean = sy / static_cast<double>(n);
  double ss_tot = 0.0;
  double ss_res = 0.0;
  for (std::size_t i = 0; i < n; ++i) {
    const double yh = fit.slope * x[i] + fit.intercept;
    const double dt = y[i] - y_mean;
    const double dr = y[i] - yh;
    ss_tot += dt * dt;
    ss_res += dr * dr;
  }
  fit.r2 = ss_tot > 0.0 ? std::max(0.0, 1.0 - ss_res / ss_tot) : 0.0;
  return fit;
}

std::vector<double> linear_fit_line(const LinearFitResult& fit, std::span<const double> x) {
  std::vector<double> yhat(x.size(), 0.0);
  for (std::size_t i = 0; i < x.size(); ++i) {
    yhat[i] = fit.slope * x[i] + fit.intercept;
  }
  return yhat;
}

}  // namespace gnuplotpp
