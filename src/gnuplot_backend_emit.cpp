#include "gnuplot_backend_internal.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

namespace gnuplotpp::detail {
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

std::string color_map_script(const ColorMap map) {
  switch (map) {
    case ColorMap::Viridis:
      return "set palette defined (0 '#440154', 0.25 '#31688e', 0.5 '#35b779', 0.75 '#8fd744', 1 '#fde725')";
    case ColorMap::Cividis:
      return "set palette defined (0 '#00224e', 0.25 '#35456c', 0.5 '#666970', 0.75 '#948f65', 1 '#fee838')";
    case ColorMap::Turbo:
      return "set palette rgbformulae 33,13,10";
    case ColorMap::Magma:
      return "set palette defined (0 '#000004', 0.25 '#51127c', 0.5 '#b73779', 0.75 '#fc8961', 1 '#fcfdbf')";
    case ColorMap::CoolWarm:
      return "set palette defined (0 '#3b4cc0', 0.5 '#f7f7f7', 1 '#b40426')";
    case ColorMap::Gray:
      return "set palette gray";
  }
  return "set palette rgbformulae 33,13,10";
}

std::string panel_label_text(const std::size_t idx) {
  const char ch = static_cast<char>('a' + static_cast<int>(idx % 26));
  return std::string("(") + ch + ")";
}

std::string resolve_font(const FigureSpec& spec, const OutputFormat format) {
  (void)format;
  if (!spec.style.font.empty()) {
    return spec.style.font;
  }
  for (const auto& f : spec.font_fallbacks) {
    if (!f.empty()) {
      return f;
    }
  }
  return "Helvetica";
}

std::string using_clause(const SeriesData& series) {
  switch (series.spec.type) {
    case SeriesType::Band:
    case SeriesType::Heatmap:
      return "using 1:2:3";
    case SeriesType::ErrorBars:
      if (!series.yerr_low.empty() && !series.yerr_high.empty()) {
        return "using 1:2:3:4";
      }
      return "using 1:2";
    case SeriesType::Line:
    case SeriesType::Scatter:
    case SeriesType::Histogram:
      return "using 1:2";
  }
  return "using 1:2";
}

std::string coord_token(const Coord2D& c) {
  const char* mode = "first";
  switch (c.system) {
    case CoordSystem::Data:
      mode = "first";
      break;
    case CoordSystem::Graph:
      mode = "graph";
      break;
    case CoordSystem::Screen:
      mode = "screen";
      break;
  }
  std::ostringstream os;
  os << mode << " " << c.x << "," << c.y;
  return os.str();
}

std::string axes_clause(const SeriesData& series) {
  if (series.spec.use_y2) {
    return "axes x1y2";
  }
  return {};
}

std::string with_clause(const SeriesData& series,
                        const Style& style,
                        const Preset preset,
                        const ColorPalette palette,
                        const std::size_t series_idx,
                        const OutputFormat format,
                        const ExportPolicy& export_policy) {
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
  const bool supports_line_alpha = format == OutputFormat::Png;
  const bool apply_line_alpha =
      series.spec.has_opacity &&
      !(export_policy.drop_line_alpha_for_vector && !supports_line_alpha);
  if (apply_line_alpha) {
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
      os << "with image";
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
    os << (arr.heads ? " head " : " nohead ");
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

  for (const auto& eq : axis_spec.equations) {
    os << "set label " << label_id++ << " '" << esc(eq.expression) << "' at "
       << coord_token(eq.at);
    if (eq.boxed) {
      os << " boxed";
    }
    if (eq.front) {
      os << " front";
    }
    os << "\n";
  }

  for (const auto& c : axis_spec.callouts) {
    os << "set arrow " << arrow_id++ << " from " << coord_token(c.from) << " to "
       << coord_token(c.to) << (c.heads ? " head " : " nohead ") << "lw "
       << std::max(0.2, c.line_width_pt) << " lc rgb '" << c.color << "'";
    if (c.front) {
      os << " front";
    }
    os << "\n";
    os << "set label " << label_id++ << " '" << esc(c.text) << "' at " << coord_token(c.from);
    if (c.front) {
      os << " front";
    }
    os << "\n";
  }
}

void emit_auto_layout(std::ostream& os, const FigureSpec& spec, const AxesSpec& axis_spec) {
  if (!spec.auto_layout) {
    return;
  }
  const std::size_t yl = axis_spec.ylabel.size();
  const std::size_t y2l = axis_spec.y2label.size();
  const std::size_t xl = axis_spec.xlabel.size();
  const std::size_t tl = axis_spec.title.size();
  const int lmargin = static_cast<int>(std::clamp<std::size_t>(8 + yl / 2, 8, 16));
  const int rmargin = static_cast<int>(std::clamp<std::size_t>(4 + y2l / 2, 4, 14));
  const int bmargin = static_cast<int>(std::clamp<std::size_t>(4 + xl / 10, 4, 8));
  const int tmargin = static_cast<int>(std::clamp<std::size_t>(2 + tl / 18, 2, 6));
  os << "set lmargin " << lmargin << "\n";
  os << "set rmargin " << rmargin << "\n";
  os << "set bmargin " << bmargin << "\n";
  os << "set tmargin " << tmargin << "\n";
}

}  // namespace

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

std::string terminal_for(OutputFormat format, const FigureSpec& spec) {
  std::ostringstream os;
  os << std::fixed << std::setprecision(3);
  const double lw = std::max(0.5, spec.style.line_width_pt);
  const char* enhanced = spec.text_mode == TextMode::Plain ? "noenhanced" : "enhanced";
  const std::string font = resolve_font(spec, format);

  switch (format) {
    case OutputFormat::Pdf:
      os << "set terminal pdfcairo size " << spec.size.w << "in," << spec.size.h << "in "
         << enhanced << " color font '" << font << "," << spec.style.font_pt
         << "' linewidth " << lw << " rounded";
      return os.str();
    case OutputFormat::Svg:
      os << "set terminal svg size " << (spec.size.w * 96.0) << "," << (spec.size.h * 96.0)
         << " " << enhanced << " font '" << font << "," << spec.style.font_pt
         << "' linewidth " << lw << " dynamic background rgb 'white'";
      return os.str();
    case OutputFormat::Eps:
      os << "set terminal epscairo size " << spec.size.w << "in," << spec.size.h << "in "
         << enhanced << " color font '" << font << "," << spec.style.font_pt
         << "' linewidth " << lw;
      return os.str();
    case OutputFormat::Png:
      os << "set terminal pngcairo size " << (spec.size.w * 300.0) << "," << (spec.size.h * 300.0)
         << " enhanced background rgb 'white' font '" << font << ","
         << spec.style.font_pt << "' linewidth " << lw;
      return os.str();
  }
  return {};
}

void emit_plot_body(std::ostream& os,
                    const Figure& fig,
                    const std::vector<std::vector<std::filesystem::path>>& data_files,
                    const OutputFormat format) {
  const auto& spec = fig.spec();
  const auto& all_axes = fig.all_axes();
  const bool ieee = is_ieee_preset(spec.preset);
  const bool ieee_strict_typography =
      ieee && spec.style.font == "Times" && spec.style.font_pt <= 9.0;
  const bool custom_color_or_opacity = has_custom_color_or_opacity(fig);
  const bool heatmap = has_heatmap(fig);
  const double tick_font_pt =
      ieee_strict_typography ? 8.0 : spec.style.font_pt * std::max(0.1, spec.style.tick_font_scale);
  const double label_font_pt =
      ieee_strict_typography ? 8.5 : spec.style.font_pt * std::max(0.1, spec.style.label_font_scale);
  const double title_font_pt =
      ieee_strict_typography ? 8.5 : spec.style.font_pt * std::max(0.1, spec.style.title_font_scale);

  os << "set border linewidth 0.9 linecolor rgb '#222222'\n";
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
    os << color_map_script(ColorMap::Viridis) << "\n";
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
    const auto& tx = axis_spec.typography;
    const double axis_tick_font_pt = tx.has_tick_font_pt
                                         ? tx.tick_font_pt
                                         : (axis_spec.has_tick_font_pt ? axis_spec.tick_font_pt
                                                                       : tick_font_pt);
    const double axis_label_font_pt =
        tx.has_label_font_pt ? tx.label_font_pt
                             : (axis_spec.has_label_font_pt ? axis_spec.label_font_pt
                                                            : label_font_pt);
    const double axis_title_font_pt =
        tx.has_title_font_pt ? tx.title_font_pt
                             : (axis_spec.has_title_font_pt ? axis_spec.title_font_pt
                                                            : title_font_pt);
    emit_auto_layout(os, spec, axis_spec);

    os << "unset title\n";
    os << "unset xlabel\n";
    os << "unset ylabel\n";
    os << "unset y2label\n";
    os << "unset grid\n";
    os << "unset logscale x\n";
    os << "unset logscale y\n";
    os << "unset logscale y2\n";
    os << "set autoscale x\n";
    os << "set autoscale y\n";
    os << "set autoscale y2\n";
    os << "unset y2tics\n";
    os << "set format x '%.2g'\n";
    os << "set format y '%.2g'\n";
    const int border_mask =
        axis_spec.frame.has_border_mask ? axis_spec.frame.border_mask : 15;
    const double border_lw = axis_spec.frame.has_border_line_width_pt
                                 ? axis_spec.frame.border_line_width_pt
                                 : 0.9;
    const std::string border_color =
        axis_spec.frame.has_border_color ? axis_spec.frame.border_color : "#222222";
    const bool ticks_out =
        axis_spec.frame.has_ticks_out ? axis_spec.frame.ticks_out : false;
    const bool ticks_mirror =
        axis_spec.frame.has_ticks_mirror ? axis_spec.frame.ticks_mirror : false;
    os << "set border " << border_mask << " linewidth " << std::max(0.2, border_lw)
       << " linecolor rgb '" << border_color << "'\n";
    os << "set tics " << (ticks_out ? "out" : "in") << " "
       << (ticks_mirror ? "mirror" : "nomirror") << " scale 0.5,0.25\n";
    os << "set xtics font '" << esc(spec.style.font) << "," << axis_tick_font_pt << "'\n";
    os << "set ytics font '" << esc(spec.style.font) << "," << axis_tick_font_pt << "'\n";

    const bool legend_enabled = axis_spec.legend && axis_spec.legend_spec.enabled;
    if (legend_enabled) {
      LegendPosition legend_pos = axis_spec.legend_spec.position;
      if (spec.auto_layout && axis.series().size() >= 4 &&
          legend_pos == LegendPosition::TopRight) {
        legend_pos = LegendPosition::OutsideRight;
      }
      os << "set key " << legend_position_token(legend_pos) << "\n";
      os << "set key maxcols " << std::max(1, axis_spec.legend_spec.columns) << "\n";
      os << "set key " << (axis_spec.legend_spec.opaque ? "opaque" : "noopaque") << "\n";
      os << "set key box linewidth " << (axis_spec.legend_spec.boxed ? "0.5" : "0.0") << "\n";
      if (axis_spec.legend_spec.has_font_pt) {
        os << "set key font '" << esc(spec.style.font) << "," << axis_spec.legend_spec.font_pt
           << "'\n";
      } else {
        os << "set key font '" << esc(spec.style.font) << "," << axis_tick_font_pt << "'\n";
      }
    } else {
      os << "unset key\n";
    }

    if (!axis_spec.title.empty()) {
      const bool title_bold = tx.has_title_bold
                                  ? tx.title_bold
                                  : (axis_spec.has_title_bold ? axis_spec.title_bold
                                                              : spec.style.title_bold);
      const std::string title_text = title_bold ? "{/:Bold " + esc(axis_spec.title) + "}"
                                                : esc(axis_spec.title);
      os << "set title '" << title_text << "' font '" << esc(spec.style.font) << ","
         << axis_title_font_pt << "'\n";
    }
    if (!axis_spec.xlabel.empty()) {
      os << "set xlabel '" << esc(axis_spec.xlabel) << "' font '" << esc(spec.style.font) << ","
         << axis_label_font_pt << "'\n";
    }
    if (!axis_spec.ylabel.empty()) {
      os << "set ylabel '" << esc(axis_spec.ylabel) << "' font '" << esc(spec.style.font) << ","
         << axis_label_font_pt << "'\n";
    }
    if (!axis_spec.y2label.empty()) {
      os << "set y2label '" << esc(axis_spec.y2label) << "' font '" << esc(spec.style.font) << ","
         << axis_label_font_pt << "'\n";
    }

    if (axis_spec.grid || spec.style.grid) {
      os << "set grid xtics ytics linewidth 0.60 linecolor rgb '#c8c8c8'\n";
    }
    if (!axis_spec.colorbar_label.empty()) {
      os << "set cblabel '" << esc(axis_spec.colorbar_label) << "' font '" << esc(spec.style.font)
         << "," << axis_label_font_pt << "'\n";
    }
    if (axis_spec.has_cbrange) {
      os << "set cbrange [" << axis_spec.cbmin << ":" << axis_spec.cbmax << "]\n";
    } else {
      os << "set autoscale cb\n";
    }
    if (axis_spec.has_cbtick_step) {
      os << "set cbtics " << axis_spec.cbtick_step << "\n";
    }
    if (axis_spec.color_norm == ColorNorm::Log) {
      os << "set logscale cb\n";
    } else {
      os << "unset logscale cb\n";
    }
    os << color_map_script(axis_spec.color_map) << "\n";
    if (axis_spec.enable_crosshair) {
      os << "set mouse\n";
      os << "set mxtics 4\n";
      os << "set mytics 4\n";
      os << "set grid mxtics mytics linewidth 0.25 linecolor rgb '#f0f0f0'\n";
    }
    if (axis_spec.xlog) {
      os << "set logscale x\n";
    }
    if (axis_spec.ylog) {
      os << "set logscale y\n";
    }
    if (axis_spec.y2log) {
      os << "set logscale y2\n";
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
    if (axis_spec.has_y2lim) {
      os << "set y2range [" << axis_spec.y2min << ":" << axis_spec.y2max << "]\n";
    }

    bool has_y2_series = false;
    for (const auto& s : axis.series()) {
      if (s.spec.use_y2) {
        has_y2_series = true;
        break;
      }
    }
    if (has_y2_series) {
      os << "set y2tics\n";
    }

    const std::size_t row = axis_idx / static_cast<std::size_t>(spec.cols);
    const std::size_t col = axis_idx % static_cast<std::size_t>(spec.cols);
    if (spec.hide_inner_tick_labels) {
      if (spec.share_x && row + 1 < static_cast<std::size_t>(spec.rows)) {
        os << "set format x ''\n";
        os << "unset xlabel\n";
      }
      if (spec.share_y && col > 0) {
        os << "set format y ''\n";
        os << "unset ylabel\n";
      }
    }

    emit_typed_objects(os, axis_spec);
    if (spec.panel_labels) {
      os << "set label " << (2000 + static_cast<int>(axis_idx)) << " '"
         << panel_label_text(axis_idx) << "' at graph 0.01,0.99 font '" << esc(spec.style.font)
         << "," << std::max(8.0, label_font_pt) << "' front\n";
    }
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
         << axes_clause(series) << " "
         << with_clause(series,
                        spec.style,
                        spec.preset,
                        spec.palette,
                        s,
                        format,
                        spec.export_policy)
         << " ";
      if (series.spec.label.empty()) {
        os << "notitle";
      } else {
        os << "title '" << esc(series.spec.label) << "'";
      }
      if (s + 1 < axis.series().size()) {
        os << ", \\\n";
      } else {
        os << "\n";
      }
    }
  }

  if (!spec.caption.empty()) {
    os << "set label 9999 '" << esc(spec.caption) << "' at screen 0.5,0.01 center font '"
       << esc(spec.style.font) << "," << std::max(8.0, label_font_pt - 0.5) << "'\n";
  }
  os << "unset multiplot\n";
}

}  // namespace gnuplotpp::detail
