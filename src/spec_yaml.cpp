#include "gnuplotpp/spec_yaml.hpp"

#include <algorithm>
#include <cctype>
#include <string>

#include <yaml-cpp/yaml.h>

namespace gnuplotpp {
namespace {

std::string trim(std::string s) {
  s.erase(s.begin(),
          std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isspace(c); }));
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char c) { return !std::isspace(c); }).base(),
          s.end());
  return s;
}

std::string lower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return s;
}

bool parse_bool(const std::string& v) {
  const auto l = lower(trim(v));
  return l == "true" || l == "1" || l == "yes" || l == "on";
}

OutputFormat parse_format(const std::string& s) {
  const auto l = lower(trim(s));
  if (l == "pdf") return OutputFormat::Pdf;
  if (l == "svg") return OutputFormat::Svg;
  if (l == "eps") return OutputFormat::Eps;
  if (l == "png") return OutputFormat::Png;
  throw std::invalid_argument("unsupported output format: " + s);
}

Preset parse_preset(const std::string& s) {
  const auto l = lower(trim(s));
  if (l == "ieee_singlecolumn") return Preset::IEEE_SingleColumn;
  if (l == "ieee_doublecolumn") return Preset::IEEE_DoubleColumn;
  if (l == "ieee_tran") return Preset::IEEE_Tran;
  if (l == "nature_1col") return Preset::Nature_1Col;
  if (l == "elsevier_1col") return Preset::Elsevier_1Col;
  if (l == "aiaa_column") return Preset::AIAA_Column;
  if (l == "aiaa_page") return Preset::AIAA_Page;
  if (l == "custom") return Preset::Custom;
  throw std::invalid_argument("unsupported preset: " + s);
}

ColorPalette parse_palette(const std::string& s) {
  const auto l = lower(trim(s));
  if (l == "default") return ColorPalette::Default;
  if (l == "tab10") return ColorPalette::Tab10;
  if (l == "viridis") return ColorPalette::Viridis;
  if (l == "grayscale") return ColorPalette::Grayscale;
  throw std::invalid_argument("unsupported palette: " + s);
}

LegendPosition parse_legend_position(const std::string& s) {
  const auto l = lower(trim(s));
  if (l == "topright" || l == "top_right") return LegendPosition::TopRight;
  if (l == "topleft" || l == "top_left") return LegendPosition::TopLeft;
  if (l == "bottomright" || l == "bottom_right") return LegendPosition::BottomRight;
  if (l == "bottomleft" || l == "bottom_left") return LegendPosition::BottomLeft;
  if (l == "outsideright" || l == "outside_right") return LegendPosition::OutsideRight;
  if (l == "outsidebottom" || l == "outside_bottom") return LegendPosition::OutsideBottom;
  throw std::invalid_argument("unsupported legend position: " + s);
}

ColorMap parse_colormap(const std::string& s) {
  const auto l = lower(trim(s));
  if (l == "viridis") return ColorMap::Viridis;
  if (l == "cividis") return ColorMap::Cividis;
  if (l == "turbo") return ColorMap::Turbo;
  if (l == "magma") return ColorMap::Magma;
  if (l == "coolwarm" || l == "cool_warm") return ColorMap::CoolWarm;
  if (l == "gray" || l == "grayscale") return ColorMap::Gray;
  throw std::invalid_argument("unsupported color map: " + s);
}

ColorNorm parse_color_norm(const std::string& s) {
  const auto l = lower(trim(s));
  if (l == "linear") return ColorNorm::Linear;
  if (l == "log" || l == "logarithmic") return ColorNorm::Log;
  throw std::invalid_argument("unsupported color norm: " + s);
}

bool parse_figure_key(FigureSpec& figure, const std::string& key, const std::string& value) {
  if (key == "title") {
    figure.title = value;
  } else if (key == "caption") {
    figure.caption = value;
  } else if (key == "preset") {
    figure.preset = parse_preset(value);
  } else if (key == "palette") {
    figure.palette = parse_palette(value);
  } else if (key == "rows") {
    figure.rows = std::max(1, std::stoi(value));
  } else if (key == "cols") {
    figure.cols = std::max(1, std::stoi(value));
  } else if (key == "panel_labels") {
    figure.panel_labels = parse_bool(value);
  } else if (key == "auto_layout") {
    figure.auto_layout = parse_bool(value);
  } else if (key == "interactive_preview") {
    figure.interactive_preview = parse_bool(value);
  } else if (key == "font") {
    figure.style.font = value;
  } else if (key == "font_pt") {
    figure.style.font_pt = std::stod(value);
  } else {
    return false;
  }
  return true;
}

bool parse_axis_key(AxesSpec& axis, const std::string& axis_key, const std::string& value) {
  if (axis_key == "title") axis.title = value;
  else if (axis_key == "xlabel") axis.xlabel = value;
  else if (axis_key == "ylabel") axis.ylabel = value;
  else if (axis_key == "y2label") axis.y2label = value;
  else if (axis_key == "grid") axis.grid = parse_bool(value);
  else if (axis_key == "legend") axis.legend = parse_bool(value);
  else if (axis_key == "enable_crosshair") axis.enable_crosshair = parse_bool(value);
  else if (axis_key == "xlog") axis.xlog = parse_bool(value);
  else if (axis_key == "ylog") axis.ylog = parse_bool(value);
  else if (axis_key == "y2log") axis.y2log = parse_bool(value);
  else if (axis_key == "xmin") {
    axis.has_xlim = true;
    axis.xmin = std::stod(value);
  } else if (axis_key == "xmax") {
    axis.has_xlim = true;
    axis.xmax = std::stod(value);
  } else if (axis_key == "ymin") {
    axis.has_ylim = true;
    axis.ymin = std::stod(value);
  } else if (axis_key == "ymax") {
    axis.has_ylim = true;
    axis.ymax = std::stod(value);
  } else if (axis_key == "y2min") {
    axis.has_y2lim = true;
    axis.y2min = std::stod(value);
  } else if (axis_key == "y2max") {
    axis.has_y2lim = true;
    axis.y2max = std::stod(value);
  } else if (axis_key == "has_xtick_step") {
    axis.has_xtick_step = parse_bool(value);
  } else if (axis_key == "xtick_step") {
    axis.has_xtick_step = true;
    axis.xtick_step = std::stod(value);
  } else if (axis_key == "has_ytick_step") {
    axis.has_ytick_step = parse_bool(value);
  } else if (axis_key == "ytick_step") {
    axis.has_ytick_step = true;
    axis.ytick_step = std::stod(value);
  } else if (axis_key == "has_xminor_count") {
    axis.has_xminor_count = parse_bool(value);
  } else if (axis_key == "xminor_count") {
    axis.has_xminor_count = true;
    axis.xminor_count = std::stoi(value);
  } else if (axis_key == "has_yminor_count") {
    axis.has_yminor_count = parse_bool(value);
  } else if (axis_key == "yminor_count") {
    axis.has_yminor_count = true;
    axis.yminor_count = std::stoi(value);
  } else if (axis_key == "xformat") {
    axis.xformat = value;
  } else if (axis_key == "yformat") {
    axis.yformat = value;
  } else if (axis_key == "color_map") {
    axis.color_map = parse_colormap(value);
  } else if (axis_key == "color_norm") {
    axis.color_norm = parse_color_norm(value);
  } else if (axis_key == "colorbar_label") {
    axis.colorbar_label = value;
  } else if (axis_key == "cbmin") {
    axis.has_cbrange = true;
    axis.cbmin = std::stod(value);
  } else if (axis_key == "cbmax") {
    axis.has_cbrange = true;
    axis.cbmax = std::stod(value);
  } else if (axis_key == "cbtick_step") {
    axis.has_cbtick_step = true;
    axis.cbtick_step = std::stod(value);
  } else if (axis_key == "legend_spec.position") {
    axis.legend_spec.position = parse_legend_position(value);
  } else if (axis_key == "legend_spec.columns") {
    axis.legend_spec.columns = std::max(1, std::stoi(value));
  } else if (axis_key == "legend_spec.boxed") {
    axis.legend_spec.boxed = parse_bool(value);
  } else if (axis_key == "legend_spec.opaque") {
    axis.legend_spec.opaque = parse_bool(value);
  } else if (axis_key == "legend_spec.font_pt") {
    axis.legend_spec.has_font_pt = true;
    axis.legend_spec.font_pt = std::stod(value);
  } else if (axis_key == "typography.tick_font_pt") {
    axis.typography.has_tick_font_pt = true;
    axis.typography.tick_font_pt = std::stod(value);
  } else if (axis_key == "typography.label_font_pt") {
    axis.typography.has_label_font_pt = true;
    axis.typography.label_font_pt = std::stod(value);
  } else if (axis_key == "typography.title_font_pt") {
    axis.typography.has_title_font_pt = true;
    axis.typography.title_font_pt = std::stod(value);
  } else if (axis_key == "typography.title_bold") {
    axis.typography.has_title_bold = true;
    axis.typography.title_bold = parse_bool(value);
  } else if (axis_key == "frame.border_mask") {
    axis.frame.has_border_mask = true;
    axis.frame.border_mask = std::stoi(value);
  } else if (axis_key == "frame.border_line_width_pt") {
    axis.frame.has_border_line_width_pt = true;
    axis.frame.border_line_width_pt = std::stod(value);
  } else if (axis_key == "frame.border_color") {
    axis.frame.has_border_color = true;
    axis.frame.border_color = value;
  } else if (axis_key == "frame.ticks_out") {
    axis.frame.has_ticks_out = true;
    axis.frame.ticks_out = parse_bool(value);
  } else if (axis_key == "frame.ticks_mirror") {
    axis.frame.has_ticks_mirror = true;
    axis.frame.ticks_mirror = parse_bool(value);
  } else {
    return false;
  }
  return true;
}

std::string scalar_as_string(const YAML::Node& n, const std::string& key) {
  if (!n.IsScalar()) {
    throw std::runtime_error("expected scalar for key '" + key + "'");
  }
  return n.as<std::string>();
}

void parse_formats(FigureSpec& figure, const YAML::Node& formats) {
  if (!formats.IsSequence()) {
    throw std::runtime_error("figure.formats must be a sequence");
  }
  figure.formats.clear();
  for (const auto& item : formats) {
    figure.formats.push_back(parse_format(item.as<std::string>()));
  }
}

void parse_figure(FigureSpec& figure, const YAML::Node& node, const YamlLoadOptions& options) {
  if (!node || !node.IsMap()) {
    throw std::runtime_error("'figure' must be a YAML map");
  }
  for (const auto& kv : node) {
    const auto key = kv.first.as<std::string>();
    if (key == "formats") {
      parse_formats(figure, kv.second);
      continue;
    }
    const auto value = scalar_as_string(kv.second, "figure." + key);
    const bool known = parse_figure_key(figure, key, value);
    if (!known && options.strict_unknown_keys) {
      throw std::runtime_error("unknown figure key: '" + key + "'");
    }
  }
}

void parse_axis_map(AxesSpec& axis, const YAML::Node& axis_node, const YamlLoadOptions& options) {
  if (!axis_node.IsMap()) {
    throw std::runtime_error("each axes entry must be a YAML map");
  }
  for (const auto& kv : axis_node) {
    const auto key = kv.first.as<std::string>();
    const auto& value_node = kv.second;

    if (key == "typography" || key == "frame" || key == "legend_spec") {
      if (!value_node.IsMap()) {
        throw std::runtime_error("axes." + key + " must be a YAML map");
      }
      for (const auto& skv : value_node) {
        const auto sub = skv.first.as<std::string>();
        const auto full = key + "." + sub;
        const bool known = parse_axis_key(axis, full, scalar_as_string(skv.second, "axes." + full));
        if (!known && options.strict_unknown_keys) {
          throw std::runtime_error("unknown axis key: '" + full + "'");
        }
      }
      continue;
    }

    const bool known = parse_axis_key(axis, key, scalar_as_string(value_node, "axes." + key));
    if (!known && options.strict_unknown_keys) {
      throw std::runtime_error("unknown axis key: '" + key + "'");
    }
  }
}

}  // namespace

YamlFigureSpec load_yaml_figure_spec(const std::filesystem::path& path,
                                     const YamlLoadOptions& options) {
  YamlFigureSpec out;
  out.figure = FigureSpec{};

  const auto root = YAML::LoadFile(path.string());
  parse_figure(out.figure, root["figure"], options);

  const auto axes_node = root["axes"];
  if (axes_node) {
    if (!axes_node.IsSequence()) {
      throw std::runtime_error("'axes' must be a YAML sequence");
    }
    for (const auto& axis_node : axes_node) {
      AxesSpec axis;
      parse_axis_map(axis, axis_node, options);
      out.axes.push_back(axis);
    }
  }

  if (out.figure.formats.empty()) {
    out.figure.formats = {OutputFormat::Pdf};
  }
  return out;
}

}  // namespace gnuplotpp
