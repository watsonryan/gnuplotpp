#include "gnuplotpp/svg_backend.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <utility>

#ifdef GNUPLOTPP_HAS_FMT
#include <fmt/format.h>
#endif

#ifdef GNUPLOTPP_HAS_SPDLOG
#include <spdlog/spdlog.h>
#endif

namespace gnuplotpp {
namespace {

struct AxisBounds {
  double xmin = 0.0;
  double xmax = 1.0;
  double ymin = 0.0;
  double ymax = 1.0;
};

std::string xml_escape(const std::string& in) {
  std::string out;
  out.reserve(in.size());
  for (const char c : in) {
    switch (c) {
      case '&':
        out += "&amp;";
        break;
      case '<':
        out += "&lt;";
        break;
      case '>':
        out += "&gt;";
        break;
      case '\"':
        out += "&quot;";
        break;
      case '\'':
        out += "&apos;";
        break;
      default:
        out += c;
        break;
    }
  }
  return out;
}

AxisBounds compute_bounds(const Axes& axis) {
  AxisBounds bounds;
  const auto& spec = axis.spec();

  if (spec.has_xlim) {
    bounds.xmin = spec.xmin;
    bounds.xmax = spec.xmax;
  } else {
    bounds.xmin = std::numeric_limits<double>::infinity();
    bounds.xmax = -std::numeric_limits<double>::infinity();
    for (const auto& s : axis.series()) {
      for (const auto x : s.x) {
        bounds.xmin = std::min(bounds.xmin, x);
        bounds.xmax = std::max(bounds.xmax, x);
      }
    }
    if (!std::isfinite(bounds.xmin) || !std::isfinite(bounds.xmax) ||
        bounds.xmin == bounds.xmax) {
      bounds.xmin = 0.0;
      bounds.xmax = 1.0;
    }
  }

  if (spec.has_ylim) {
    bounds.ymin = spec.ymin;
    bounds.ymax = spec.ymax;
  } else {
    bounds.ymin = std::numeric_limits<double>::infinity();
    bounds.ymax = -std::numeric_limits<double>::infinity();
    for (const auto& s : axis.series()) {
      for (const auto y : s.y) {
        bounds.ymin = std::min(bounds.ymin, y);
        bounds.ymax = std::max(bounds.ymax, y);
      }
    }
    if (!std::isfinite(bounds.ymin) || !std::isfinite(bounds.ymax) ||
        bounds.ymin == bounds.ymax) {
      bounds.ymin = 0.0;
      bounds.ymax = 1.0;
    }
  }

  const double xpad = (bounds.xmax - bounds.xmin) * 0.03;
  const double ypad = (bounds.ymax - bounds.ymin) * 0.06;
  bounds.xmin -= xpad;
  bounds.xmax += xpad;
  bounds.ymin -= ypad;
  bounds.ymax += ypad;
  return bounds;
}

std::string color_for(std::size_t idx) {
  static constexpr std::array<const char*, 8> kColors{
      "#0072B2", "#D55E00", "#009E73", "#CC79A7",
      "#56B4E9", "#E69F00", "#F0E442", "#000000"};
  return kColors[idx % kColors.size()];
}

double safe_value(const double v, const double fallback = 0.0) {
  return std::isfinite(v) ? v : fallback;
}

std::string msg_text(const std::string& prefix, const std::string& detail) {
#ifdef GNUPLOTPP_HAS_FMT
  return fmt::format("{}: {}", prefix, detail);
#else
  return prefix + ": " + detail;
#endif
}

void log_info(const std::string& msg) {
#ifdef GNUPLOTPP_HAS_SPDLOG
  spdlog::info("{}", msg);
#else
  std::clog << "[gnuplotpp] info: " << msg << "\n";
#endif
}

void log_error(const std::string& msg) {
#ifdef GNUPLOTPP_HAS_SPDLOG
  spdlog::error("{}", msg);
#else
  std::cerr << "[gnuplotpp] error: " << msg << "\n";
#endif
}

}  // namespace

RenderResult SvgBackend::render(const Figure& fig,
                                const std::filesystem::path& out_dir) {
  RenderResult result;
  result.status = RenderStatus::Success;
  std::error_code ec;
  std::filesystem::create_directories(out_dir, ec);
  if (ec) {
    result.ok = false;
    result.status = RenderStatus::IoError;
    result.message = msg_text("failed to create output directory", ec.message());
    log_error(result.message);
    return result;
  }

  const auto& spec = fig.spec();
  bool requested_svg = false;
  for (const auto fmt : spec.formats) {
    if (fmt == OutputFormat::Svg) {
      requested_svg = true;
      break;
    }
  }
  if (!requested_svg) {
    result.ok = false;
    result.status = RenderStatus::UnsupportedFormat;
    result.message = "SvgBackend only supports OutputFormat::Svg";
    log_error(result.message);
    return result;
  }

  const double width_px = std::max(1.0, spec.size.w * 96.0);
  const double height_px = std::max(1.0, spec.size.h * 96.0);

  std::ostringstream svg;
  svg << std::fixed << std::setprecision(3);
  svg << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
      << "xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
      << "version=\"1.1\" width=\"" << width_px << "px\" height=\"" << height_px
      << "px\" viewBox=\"0 0 " << width_px << " " << height_px << "\">\n";
  svg << "<rect x=\"0\" y=\"0\" width=\"" << width_px << "\" height=\"" << height_px
      << "\" fill=\"white\"/>\n";

  const double left = 20.0;
  const double top = 28.0;
  const double right = 10.0;
  const double bottom = 18.0;
  const double cell_w = (width_px - left - right) / static_cast<double>(spec.cols);
  const double cell_h = (height_px - top - bottom) / static_cast<double>(spec.rows);

  if (!spec.title.empty()) {
    svg << "<text x=\"" << (width_px / 2.0) << "\" y=\"16\" text-anchor=\"middle\" "
        << "font-family=\"" << xml_escape(spec.style.font) << "\" font-size=\""
        << (spec.style.font_pt + 1.0) << "\">" << xml_escape(spec.title) << "</text>\n";
  }

  const auto& axes = fig.all_axes();
  for (std::size_t idx = 0; idx < axes.size(); ++idx) {
    const int row = static_cast<int>(idx) / spec.cols;
    const int col = static_cast<int>(idx) % spec.cols;

    const double x0 = left + col * cell_w;
    const double y0 = top + row * cell_h;

    const double plot_l = x0 + 42.0;
    const double plot_r = x0 + cell_w - 8.0;
    const double plot_t = y0 + 16.0;
    const double plot_b = y0 + cell_h - 24.0;
    const double plot_w = std::max(1.0, plot_r - plot_l);
    const double plot_h = std::max(1.0, plot_b - plot_t);

    const auto& axis = axes[idx];
    const auto& axis_spec = axis.spec();
    const auto bounds = compute_bounds(axis);

    svg << "<rect x=\"" << plot_l << "\" y=\"" << plot_t << "\" width=\"" << plot_w
        << "\" height=\"" << plot_h << "\" fill=\"none\" stroke=\"#444\" stroke-width=\"1\"/>\n";

    if (axis_spec.grid || spec.style.grid) {
      for (int g = 1; g < 5; ++g) {
        const double gx = plot_l + (plot_w * g) / 5.0;
        const double gy = plot_t + (plot_h * g) / 5.0;
        svg << "<line x1=\"" << gx << "\" y1=\"" << plot_t << "\" x2=\"" << gx
            << "\" y2=\"" << plot_b << "\" stroke=\"#ddd\" stroke-width=\"1\"/>\n";
        svg << "<line x1=\"" << plot_l << "\" y1=\"" << gy << "\" x2=\"" << plot_r
            << "\" y2=\"" << gy << "\" stroke=\"#ddd\" stroke-width=\"1\"/>\n";
      }
    }

    const auto map_x = [&](const double x) {
      return safe_value(plot_l + (x - bounds.xmin) * plot_w / (bounds.xmax - bounds.xmin), plot_l);
    };
    const auto map_y = [&](const double y) {
      return safe_value(plot_b - (y - bounds.ymin) * plot_h / (bounds.ymax - bounds.ymin), plot_b);
    };

    for (std::size_t s = 0; s < axis.series().size(); ++s) {
      const auto& series = axis.series()[s];
      if (series.x.empty()) {
        continue;
      }
      const auto color = color_for(s);
      const double lw =
          series.spec.has_line_width ? series.spec.line_width_pt : spec.style.line_width_pt;

      if (series.spec.type == SeriesType::Scatter) {
        for (std::size_t i = 0; i < series.x.size(); ++i) {
          svg << "<circle cx=\"" << map_x(series.x[i]) << "\" cy=\"" << map_y(series.y[i])
              << "\" r=\"2.5\" fill=\"" << color << "\"/>\n";
        }
      } else {
        svg << "<polyline fill=\"none\" stroke=\"" << color << "\" stroke-width=\"" << lw
            << "\" stroke-linejoin=\"round\" stroke-linecap=\"round\" points=\"";
        for (std::size_t i = 0; i < series.x.size(); ++i) {
          if (!std::isfinite(series.x[i]) || !std::isfinite(series.y[i])) {
            continue;
          }
          svg << map_x(series.x[i]) << "," << map_y(series.y[i]);
          if (i + 1 < series.x.size()) {
            svg << " ";
          }
        }
        svg << "\"/>\n";
      }
    }

    if (!axis_spec.title.empty()) {
      svg << "<text x=\"" << (plot_l + plot_w / 2.0) << "\" y=\"" << (y0 + 12.0)
          << "\" text-anchor=\"middle\" font-family=\"" << xml_escape(spec.style.font)
          << "\" font-size=\"" << spec.style.font_pt << "\">"
          << xml_escape(axis_spec.title) << "</text>\n";
    }

    if (!axis_spec.xlabel.empty()) {
      svg << "<text x=\"" << (plot_l + plot_w / 2.0) << "\" y=\"" << (y0 + cell_h - 4.0)
          << "\" text-anchor=\"middle\" font-family=\"" << xml_escape(spec.style.font)
          << "\" font-size=\"" << (spec.style.font_pt - 0.5) << "\">"
          << xml_escape(axis_spec.xlabel) << "</text>\n";
    }

    if (!axis_spec.ylabel.empty()) {
      svg << "<g transform=\"translate(" << (x0 + 12.0) << "," << (plot_t + plot_h / 2.0)
          << ") rotate(-90)\">"
          << "<text x=\"0\" y=\"0\" text-anchor=\"middle\" font-family=\""
          << xml_escape(spec.style.font) << "\" font-size=\"" << (spec.style.font_pt - 0.5)
          << "\">" << xml_escape(axis_spec.ylabel) << "</text></g>\n";
    }
  }

  svg << "</svg>\n";

  const auto out_path = out_dir / "figure.svg";
  std::ofstream out(out_path);
  if (!out.is_open()) {
    result.ok = false;
    result.status = RenderStatus::IoError;
    result.message = msg_text("failed to open svg output for writing", out_path.string());
    log_error(result.message);
    return result;
  }
  out << svg.str();
  if (!out.good()) {
    result.ok = false;
    result.status = RenderStatus::IoError;
    result.message = msg_text("failed while writing svg output", out_path.string());
    log_error(result.message);
    return result;
  }

  result.ok = true;
  result.status = RenderStatus::Success;
  result.message = "native svg render completed";
  result.outputs.push_back(out_path);
  log_info(result.message);
  return result;
}

std::shared_ptr<IPlotBackend> make_svg_backend() {
  return std::make_shared<SvgBackend>();
}

}  // namespace gnuplotpp
