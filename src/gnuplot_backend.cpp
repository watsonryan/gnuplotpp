#include "gnuplotpp/gnuplot_backend.hpp"
#include "gnuplotpp/logging.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

#ifdef GNUPLOTPP_HAS_FMT
#include <fmt/format.h>
#endif

namespace gnuplotpp {
namespace {

std::string quote(const std::filesystem::path& p) { return "'" + p.string() + "'"; }

std::string esc(const std::string& s) {
  std::string out;
  out.reserve(s.size());
  for (char c : s) {
    if (c == '\'') {
      out += "''";
    } else {
      out += c;
    }
  }
  return out;
}

std::string msg_io(const std::string& prefix,
                   const std::filesystem::path& path,
                   const std::error_code& ec) {
#ifdef GNUPLOTPP_HAS_FMT
  return fmt::format("{} '{}': {}", prefix, path.string(), ec.message());
#else
  std::ostringstream os;
  os << prefix << " '" << path.string() << "': " << ec.message();
  return os.str();
#endif
}

std::string msg_text(const std::string& prefix, const std::string& detail) {
#ifdef GNUPLOTPP_HAS_FMT
  return fmt::format("{}: {}", prefix, detail);
#else
  return prefix + ": " + detail;
#endif
}

std::string extension_for(OutputFormat format) {
  switch (format) {
    case OutputFormat::Pdf:
      return "pdf";
    case OutputFormat::Svg:
      return "svg";
    case OutputFormat::Eps:
      return "eps";
    case OutputFormat::Png:
      return "png";
  }
  return "out";
}

bool is_ieee_preset(const Preset preset) {
  return preset == Preset::IEEE_SingleColumn || preset == Preset::IEEE_DoubleColumn;
}

const char* palette_color(const ColorPalette palette, const std::size_t index) {
  static constexpr const char* kDefault[] = {"#1f77b4", "#d62728", "#2ca02c", "#9467bd",
                                              "#ff7f0e", "#17becf", "#8c564b", "#e377c2"};
  static constexpr const char* kTab10[] = {"#1f77b4", "#ff7f0e", "#2ca02c", "#d62728",
                                            "#9467bd", "#8c564b", "#e377c2", "#7f7f7f",
                                            "#bcbd22", "#17becf"};
  static constexpr const char* kViridis[] = {"#440154", "#482878", "#3e4989", "#31688e",
                                              "#26828e", "#1f9e89", "#35b779", "#6ece58",
                                              "#b5de2b", "#fde725"};
  static constexpr const char* kGray[] = {"#111111", "#333333", "#555555", "#777777", "#999999",
                                           "#bbbbbb"};
  switch (palette) {
    case ColorPalette::Tab10:
      return kTab10[index % (sizeof(kTab10) / sizeof(kTab10[0]))];
    case ColorPalette::Viridis:
      return kViridis[index % (sizeof(kViridis) / sizeof(kViridis[0]))];
    case ColorPalette::Grayscale:
      return kGray[index % (sizeof(kGray) / sizeof(kGray[0]))];
    case ColorPalette::Default:
    default:
      return kDefault[index % (sizeof(kDefault) / sizeof(kDefault[0]))];
  }
}

bool is_hex_digit(const char c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool is_hex_rgb(const std::string& color) {
  if (color.size() != 7 || color.front() != '#') {
    return false;
  }
  for (std::size_t i = 1; i < color.size(); ++i) {
    if (!is_hex_digit(color[i])) {
      return false;
    }
  }
  return true;
}

std::string clamp_opacity_color(const std::string& color, const double opacity) {
  const double clamped = std::clamp(opacity, 0.0, 1.0);
  if (!is_hex_rgb(color)) {
    return color;
  }
  // gnuplot cairo uses ARGB with inverted alpha semantics:
  // 00 = fully opaque, ff = fully transparent.
  const int alpha = static_cast<int>(std::lround((1.0 - clamped) * 255.0));
  std::ostringstream os;
  os << "#" << std::hex << std::setfill('0') << std::nouppercase << std::setw(2) << alpha
     << color.substr(1);
  return os.str();
}

bool has_custom_color_or_opacity(const Figure& fig) {
  for (const auto& axis : fig.all_axes()) {
    for (const auto& series : axis.series()) {
      if (series.spec.has_color || series.spec.has_opacity) {
        return true;
      }
    }
  }
  return false;
}

bool has_series_opacity(const Figure& fig) {
  for (const auto& axis : fig.all_axes()) {
    for (const auto& series : axis.series()) {
      if (series.spec.has_opacity) {
        return true;
      }
    }
  }
  return false;
}

bool has_heatmap(const Figure& fig) {
  for (const auto& axis : fig.all_axes()) {
    for (const auto& series : axis.series()) {
      if (series.spec.type == SeriesType::Heatmap) {
        return true;
      }
    }
  }
  return false;
}

const char* format_name(const OutputFormat format) {
  switch (format) {
    case OutputFormat::Pdf:
      return "PDF";
    case OutputFormat::Svg:
      return "SVG";
    case OutputFormat::Eps:
      return "EPS";
    case OutputFormat::Png:
      return "PNG";
  }
  return "unknown";
}

bool format_supports_line_alpha(const OutputFormat format) { return format == OutputFormat::Png; }

std::string terminal_for(OutputFormat format, const FigureSpec& spec) {
  std::ostringstream os;
  os << std::fixed << std::setprecision(3);
  const double lw = std::max(0.5, spec.style.line_width_pt);
  const char* enhanced = spec.text_mode == TextMode::Plain ? "noenhanced" : "enhanced";

  switch (format) {
    case OutputFormat::Pdf:
      os << "set terminal pdfcairo size " << spec.size.w << "in," << spec.size.h << "in "
         << enhanced << " color font '" << spec.style.font << "," << spec.style.font_pt
         << "' linewidth " << lw << " rounded";
      return os.str();
    case OutputFormat::Svg:
      os << "set terminal svg size " << (spec.size.w * 96.0) << "," << (spec.size.h * 96.0)
         << " " << enhanced << " font '" << spec.style.font << "," << spec.style.font_pt
         << "' linewidth " << lw << " dynamic background rgb 'white'";
      return os.str();
    case OutputFormat::Eps:
      os << "set terminal epscairo size " << spec.size.w << "in," << spec.size.h << "in "
         << enhanced << " color font '" << spec.style.font << "," << spec.style.font_pt
         << "' linewidth " << lw;
      return os.str();
    case OutputFormat::Png:
      os << "set terminal pngcairo size " << (spec.size.w * 200.0) << "," << (spec.size.h * 200.0)
         << " enhanced background rgb 'white' font '" << spec.style.font << ","
         << spec.style.font_pt << "' linewidth " << lw;
      return os.str();
  }
  return {};
}

std::string using_clause(const SeriesData& series) {
  switch (series.spec.type) {
    case SeriesType::Band:
    case SeriesType::Heatmap:
      return "using 1:2:3";
    case SeriesType::Line:
    case SeriesType::Scatter:
    case SeriesType::ErrorBars:
    case SeriesType::Histogram:
      return "using 1:2";
  }
  return "using 1:2";
}

std::string with_clause(const SeriesData& series,
                        const Style& style,
                        const Preset preset,
                        const ColorPalette palette,
                        const std::size_t series_idx) {
  const double line_width =
      series.spec.has_line_width ? series.spec.line_width_pt : style.line_width_pt;
  const bool ieee = is_ieee_preset(preset);
  const bool use_ieee_dash = ieee && !series.spec.has_opacity;
  const int dt = static_cast<int>(series_idx % 7U) + 1;

  std::string color;
  if (series.spec.has_color) {
    color = series.spec.color;
  } else if (ieee || series.spec.has_opacity) {
    color = "#000000";
  } else {
    color = palette_color(palette, series_idx);
  }
  if (series.spec.has_opacity) {
    color = clamp_opacity_color(color, series.spec.opacity);
  }

  std::ostringstream os;
  os << std::fixed << std::setprecision(3);
  switch (series.spec.type) {
    case SeriesType::Line:
      os << "with lines lw " << std::max(0.5, line_width);
      if (!color.empty()) {
        os << " lc rgb '" << color << "'";
      }
      if (use_ieee_dash) {
        os << " dt " << dt;
      }
      break;
    case SeriesType::Scatter:
      os << "with points pt 7 ps " << std::max(0.5, style.point_size);
      if (!color.empty()) {
        os << " lc rgb '" << color << "'";
      }
      break;
    case SeriesType::ErrorBars:
      os << "with yerrorbars lw " << std::max(0.5, line_width);
      if (!color.empty()) {
        os << " lc rgb '" << color << "'";
      }
      if (use_ieee_dash) {
        os << " dt " << dt;
      }
      break;
    case SeriesType::Band:
      os << "with filledcurves";
      if (!color.empty()) {
        os << " lc rgb '" << color << "'";
      }
      if (series.spec.has_opacity) {
        os << " fs transparent solid " << std::clamp(series.spec.opacity, 0.0, 1.0)
           << " noborder";
      } else {
        os << " fs transparent solid 0.20 noborder";
      }
      break;
    case SeriesType::Histogram:
      os << "with boxes lw " << std::max(0.5, line_width);
      if (!color.empty()) {
        os << " lc rgb '" << color << "'";
      }
      if (series.spec.has_opacity) {
        os << " fs transparent solid " << std::clamp(series.spec.opacity, 0.0, 1.0)
           << " border rgb '#222222'";
      } else {
        os << " fs solid 1.0 border rgb '#222222'";
      }
      break;
    case SeriesType::Heatmap:
      os << "with points pt 5 ps 0.55 palette";
      break;
  }
  return os.str();
}

std::string legend_position_token(const LegendPosition pos) {
  switch (pos) {
    case LegendPosition::TopRight:
      return "top right";
    case LegendPosition::TopLeft:
      return "top left";
    case LegendPosition::BottomRight:
      return "bottom right";
    case LegendPosition::BottomLeft:
      return "bottom left";
    case LegendPosition::OutsideRight:
      return "outside right center";
    case LegendPosition::OutsideBottom:
      return "outside bottom center";
  }
  return "top right";
}

void emit_typed_objects(std::ostream& os, const AxesSpec& axis_spec) {
  int label_id = 1000;
  for (const auto& lbl : axis_spec.labels) {
    os << "set label " << label_id++ << " '" << esc(lbl.text) << "' at " << lbl.at;
    if (!lbl.font.empty()) {
      os << " font '" << esc(lbl.font) << "'";
    }
    if (lbl.front) {
      os << " front";
    }
    os << "\n";
  }

  int arrow_id = 1000;
  for (const auto& arr : axis_spec.arrows) {
    os << "set arrow " << arrow_id++ << " from " << arr.from << " to " << arr.to;
    os << (arr.heads ? " heads " : " nohead ");
    os << "lw " << std::max(0.2, arr.line_width_pt) << " lc rgb '" << arr.color << "'";
    if (arr.front) {
      os << " front";
    }
    os << "\n";
  }

  int obj_id = 1000;
  for (const auto& rect : axis_spec.rectangles) {
    os << "set object " << obj_id++ << " rect from " << rect.from << " to " << rect.to;
    if (rect.has_fill_opacity) {
      os << " fc rgb '" << rect.fill_color << "' fs transparent solid "
         << std::clamp(rect.fill_opacity, 0.0, 1.0);
    } else {
      os << " fc rgb '" << rect.fill_color << "' fs solid 1.0";
    }
    if (rect.border) {
      os << " border lc rgb '" << rect.border_color << "'";
    } else {
      os << " noborder";
    }
    os << (rect.front ? " front" : " back") << "\n";
  }
}

void emit_plot_body(std::ostream& os,
                    const Figure& fig,
                    const std::vector<std::vector<std::filesystem::path>>& data_files) {
  const auto& spec = fig.spec();
  const auto& all_axes = fig.all_axes();
  const bool ieee = is_ieee_preset(spec.preset);
  const bool custom_color_or_opacity = has_custom_color_or_opacity(fig);
  const bool heatmap = has_heatmap(fig);
  const double tick_font_pt = ieee ? 8.0 : spec.style.font_pt;
  const double label_font_pt = ieee ? 8.5 : spec.style.font_pt;
  const double title_font_pt = ieee ? 8.5 : spec.style.font_pt;

  os << "set border linewidth 0.5 linecolor rgb '#222222'\n";
  os << "set tics in nomirror scale 0.5,0.25\n";
  os << "unset x2tics\n";
  os << "unset y2tics\n";
  os << "set mxtics 2\n";
  os << "set mytics 2\n";
  os << "set xtics font '" << esc(spec.style.font) << "," << tick_font_pt << "'\n";
  os << "set ytics font '" << esc(spec.style.font) << "," << tick_font_pt << "'\n";
  os << "set format x '%.2g'\n";
  os << "set format y '%.2g'\n";
  if (heatmap) {
    os << "set palette rgbformulae 33,13,10\n";
    os << "set colorbox\n";
  } else {
    os << "unset colorbox\n";
  }
  if (ieee && !custom_color_or_opacity) {
    os << "set monochrome\n";
  }

  bool emit_global_title = !spec.title.empty();
  if (emit_global_title && spec.rows == 1 && spec.cols == 1 && !all_axes.empty() &&
      !all_axes.front().spec().title.empty()) {
    emit_global_title = false;
  }

  os << "set multiplot layout " << spec.rows << "," << spec.cols;
  if (emit_global_title) {
    os << " title '" << esc(spec.title) << "'";
  }
  os << "\n";

  for (std::size_t axis_idx = 0; axis_idx < all_axes.size(); ++axis_idx) {
    const auto& axis = all_axes[axis_idx];
    const auto& axis_spec = axis.spec();

    os << "unset title\n";
    os << "unset xlabel\n";
    os << "unset ylabel\n";
    os << "unset grid\n";
    os << "unset logscale x\n";
    os << "unset logscale y\n";
    os << "set autoscale x\n";
    os << "set autoscale y\n";

    const bool legend_enabled = axis_spec.legend && axis_spec.legend_spec.enabled;
    if (legend_enabled) {
      os << "set key " << legend_position_token(axis_spec.legend_spec.position) << "\n";
      os << "set key maxcols " << std::max(1, axis_spec.legend_spec.columns) << "\n";
      os << "set key " << (axis_spec.legend_spec.opaque ? "opaque" : "noopaque") << "\n";
      os << "set key box linewidth " << (axis_spec.legend_spec.boxed ? "0.5" : "0.0") << "\n";
      if (axis_spec.legend_spec.has_font_pt) {
        os << "set key font '" << esc(spec.style.font) << "," << axis_spec.legend_spec.font_pt
           << "'\n";
      } else if (ieee) {
        os << "set key font '" << esc(spec.style.font) << ",8.0'\n";
      }
    } else {
      os << "unset key\n";
    }

    if (!axis_spec.title.empty()) {
      os << "set title '" << esc(axis_spec.title) << "' font '" << esc(spec.style.font) << ","
         << title_font_pt << "'\n";
    }
    if (!axis_spec.xlabel.empty()) {
      os << "set xlabel '" << esc(axis_spec.xlabel) << "' font '" << esc(spec.style.font) << ","
         << label_font_pt << "'\n";
    }
    if (!axis_spec.ylabel.empty()) {
      os << "set ylabel '" << esc(axis_spec.ylabel) << "' font '" << esc(spec.style.font) << ","
         << label_font_pt << "'\n";
    }

    if (axis_spec.grid || spec.style.grid) {
      os << "set grid xtics ytics linewidth 0.5 linecolor rgb '#d0d0d0'\n";
    }
    if (axis_spec.xlog) {
      os << "set logscale x\n";
    }
    if (axis_spec.ylog) {
      os << "set logscale y\n";
    }
    if (axis_spec.has_xtick_step) {
      os << "set xtics " << axis_spec.xtick_step << "\n";
    }
    if (axis_spec.has_ytick_step) {
      os << "set ytics " << axis_spec.ytick_step << "\n";
    }
    if (axis_spec.has_xminor_count) {
      os << "set mxtics " << std::max(0, axis_spec.xminor_count) << "\n";
    }
    if (axis_spec.has_yminor_count) {
      os << "set mytics " << std::max(0, axis_spec.yminor_count) << "\n";
    }
    if (!axis_spec.xformat.empty()) {
      os << "set format x '" << esc(axis_spec.xformat) << "'\n";
    }
    if (!axis_spec.yformat.empty()) {
      os << "set format y '" << esc(axis_spec.yformat) << "'\n";
    }

    if (axis_spec.has_xlim) {
      os << "set xrange [" << axis_spec.xmin << ":" << axis_spec.xmax << "]\n";
    }
    if (axis_spec.has_ylim) {
      os << "set yrange [" << axis_spec.ymin << ":" << axis_spec.ymax << "]\n";
    }

    emit_typed_objects(os, axis_spec);
    for (const auto& cmd : axis_spec.gnuplot_commands) {
      os << cmd << "\n";
    }

    if (axis.series().empty()) {
      os << "plot 1/0 notitle\n";
      continue;
    }

    os << "plot ";
    for (std::size_t s = 0; s < axis.series().size(); ++s) {
      const auto& series = axis.series()[s];
      os << quote(data_files[axis_idx][s]) << " " << using_clause(series) << " "
         << with_clause(series, spec.style, spec.preset, spec.palette, s) << " title '"
         << esc(series.spec.label) << "'";
      if (s + 1 < axis.series().size()) {
        os << ", \\\n";
      } else {
        os << "\n";
      }
    }
  }

  os << "unset multiplot\n";
}

void write_manifest(const Figure& fig,
                    const std::filesystem::path& out_dir,
                    const std::filesystem::path& script_path,
                    const std::vector<std::filesystem::path>& outputs) {
  const auto manifest_path = out_dir / "manifest.json";
  std::ofstream os(manifest_path);
  if (!os.is_open()) {
    gnuplotpp::log::Warn("failed to write manifest: ", manifest_path.string());
    return;
  }
  os << "{\n";
  os << "  \"preset\": " << static_cast<int>(fig.spec().preset) << ",\n";
  os << "  \"palette\": " << static_cast<int>(fig.spec().palette) << ",\n";
  os << "  \"text_mode\": " << static_cast<int>(fig.spec().text_mode) << ",\n";
  os << "  \"size_in\": {\"w\": " << fig.spec().size.w << ", \"h\": " << fig.spec().size.h
     << "},\n";
  os << "  \"script\": \"" << script_path.string() << "\",\n";
  os << "  \"outputs\": [\n";
  for (std::size_t i = 0; i < outputs.size(); ++i) {
    os << "    \"" << outputs[i].string() << "\"";
    if (i + 1 < outputs.size()) {
      os << ",";
    }
    os << "\n";
  }
  os << "  ]\n";
  os << "}\n";
}

}  // namespace

GnuplotBackend::GnuplotBackend(std::string executable) : executable_(std::move(executable)) {}

RenderResult GnuplotBackend::render(const Figure& fig, const std::filesystem::path& out_dir) {
  RenderResult result;
  result.status = RenderStatus::Success;

  std::error_code ec;
  std::filesystem::create_directories(out_dir / "tmp", ec);
  if (ec) {
    result.ok = false;
    result.status = RenderStatus::IoError;
    result.message = msg_io("failed to create output directories", out_dir / "tmp", ec);
    gnuplotpp::log::Error(result.message);
    return result;
  }

  const auto& all_axes = fig.all_axes();
  std::vector<std::vector<std::filesystem::path>> data_files(all_axes.size());

  for (std::size_t axis_idx = 0; axis_idx < all_axes.size(); ++axis_idx) {
    const auto& axis = all_axes[axis_idx];
    data_files[axis_idx].reserve(axis.series().size());

    for (std::size_t series_idx = 0; series_idx < axis.series().size(); ++series_idx) {
      const auto data_path =
          out_dir / "tmp" /
          ("ax" + std::to_string(axis_idx) + "_series" + std::to_string(series_idx) + ".dat");
      data_files[axis_idx].push_back(data_path);

      const auto& series = axis.series()[series_idx];
      std::ofstream data_os(data_path);
      if (!data_os.is_open()) {
        result.ok = false;
        result.status = RenderStatus::IoError;
        result.message = msg_text("failed to open data file for writing", data_path.string());
        gnuplotpp::log::Error(result.message);
        return result;
      }
      data_os << std::scientific << std::setprecision(16);
      for (std::size_t i = 0; i < series.x.size(); ++i) {
        data_os << series.x[i] << " " << series.y[i];
        if (series.spec.type == SeriesType::Band && i < series.y2.size()) {
          data_os << " " << series.y2[i];
        }
        if (series.spec.type == SeriesType::Heatmap && i < series.z.size()) {
          data_os << " " << series.z[i];
        }
        data_os << "\n";
      }
      if (!data_os.good()) {
        result.ok = false;
        result.status = RenderStatus::IoError;
        result.message = msg_text("failed while writing data file", data_path.string());
        gnuplotpp::log::Error(result.message);
        return result;
      }
    }
  }

  const auto script_path = out_dir / "tmp" / "figure.gp";
  result.script_path = script_path;

  std::ofstream script_os(script_path);
  if (!script_os.is_open()) {
    result.ok = false;
    result.status = RenderStatus::IoError;
    result.message = msg_text("failed to open gnuplot script for writing", script_path.string());
    gnuplotpp::log::Error(result.message);
    return result;
  }
  script_os << "set encoding utf8\n";
  const bool any_series_opacity = has_series_opacity(fig);

  for (const auto format : fig.spec().formats) {
    if (any_series_opacity && !format_supports_line_alpha(format)) {
      gnuplotpp::log::Warn("line opacity requested but ", format_name(format),
                           " terminal may ignore alpha; prefer PNG for true line transparency");
    }
    if (fig.spec().text_mode == TextMode::LaTeX && format == OutputFormat::Png) {
      gnuplotpp::log::Warn("LaTeX text mode requested; PNG output falls back to enhanced text");
    }

    const auto output_path = out_dir / ("figure." + extension_for(format));
    result.outputs.push_back(output_path);

    script_os << terminal_for(format, fig.spec()) << "\n";
    script_os << "set output " << quote(output_path) << "\n";
    emit_plot_body(script_os, fig, data_files);
    script_os << "set output\n";
  }
  if (!script_os.good()) {
    result.ok = false;
    result.status = RenderStatus::IoError;
    result.message = msg_text("failed while writing gnuplot script", script_path.string());
    gnuplotpp::log::Error(result.message);
    return result;
  }
  script_os.close();
  if (!script_os) {
    result.ok = false;
    result.status = RenderStatus::IoError;
    result.message = msg_text("failed to finalize gnuplot script", script_path.string());
    gnuplotpp::log::Error(result.message);
    return result;
  }

  const std::string check_cmd = "command -v " + executable_ + " >/dev/null 2>&1";
  if (std::system(check_cmd.c_str()) != 0) {
    result.ok = false;
    result.status = RenderStatus::ExternalToolMissing;
    result.message =
        msg_text("gnuplot executable not found; generated script/data only", executable_);
    gnuplotpp::log::Error(result.message);
    return result;
  }

  const auto log_path = out_dir / "tmp" / "gnuplot.log";
  const std::string render_cmd = executable_ + " " + quote(script_path) + " >" + quote(log_path) +
                                 " 2>&1";
  const int rc = std::system(render_cmd.c_str());
  if (rc != 0) {
    result.ok = false;
    result.status = RenderStatus::ExternalToolFailure;
#ifdef GNUPLOTPP_HAS_FMT
    result.message = fmt::format("gnuplot failed (exit={}); inspect {}", rc, log_path.string());
#else
    result.message = "gnuplot failed; inspect gnuplot.log in output tmp directory";
#endif
    gnuplotpp::log::Error(result.message);
    return result;
  }

  for (const auto& out : result.outputs) {
    std::error_code fsec;
    const auto size = std::filesystem::file_size(out, fsec);
    if (fsec || size == 0) {
      result.ok = false;
      result.status = RenderStatus::ExternalToolFailure;
      result.message = msg_text("gnuplot completed but output is missing/empty", out.string());
      gnuplotpp::log::Error(result.message);
      return result;
    }
  }

  if (fig.spec().write_manifest) {
    write_manifest(fig, out_dir, result.script_path, result.outputs);
  }

  result.ok = true;
  result.status = RenderStatus::Success;
  result.message = "render completed";
  gnuplotpp::log::Info(result.message);
  return result;
}

std::shared_ptr<IPlotBackend> make_gnuplot_backend(std::string executable) {
  return std::make_shared<GnuplotBackend>(std::move(executable));
}

}  // namespace gnuplotpp
