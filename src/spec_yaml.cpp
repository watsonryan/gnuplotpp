#include "gnuplotpp/spec_yaml.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

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
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
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
  return OutputFormat::Pdf;
}

Preset parse_preset(const std::string& s) {
  const auto l = lower(trim(s));
  if (l == "ieee_singlecolumn") return Preset::IEEE_SingleColumn;
  if (l == "ieee_doublecolumn") return Preset::IEEE_DoubleColumn;
  if (l == "ieee_tran") return Preset::IEEE_Tran;
  if (l == "nature_1col") return Preset::Nature_1Col;
  if (l == "elsevier_1col") return Preset::Elsevier_1Col;
  return Preset::Custom;
}

ColorPalette parse_palette(const std::string& s) {
  const auto l = lower(trim(s));
  if (l == "tab10") return ColorPalette::Tab10;
  if (l == "viridis") return ColorPalette::Viridis;
  if (l == "grayscale") return ColorPalette::Grayscale;
  return ColorPalette::Default;
}

LegendPosition parse_legend_position(const std::string& s) {
  const auto l = lower(trim(s));
  if (l == "topleft" || l == "top_left") return LegendPosition::TopLeft;
  if (l == "bottomright" || l == "bottom_right") return LegendPosition::BottomRight;
  if (l == "bottomleft" || l == "bottom_left") return LegendPosition::BottomLeft;
  if (l == "outsideright" || l == "outside_right") return LegendPosition::OutsideRight;
  if (l == "outsidebottom" || l == "outside_bottom") return LegendPosition::OutsideBottom;
  return LegendPosition::TopRight;
}

ColorMap parse_colormap(const std::string& s) {
  const auto l = lower(trim(s));
  if (l == "cividis") return ColorMap::Cividis;
  if (l == "turbo") return ColorMap::Turbo;
  if (l == "magma") return ColorMap::Magma;
  if (l == "coolwarm" || l == "cool_warm") return ColorMap::CoolWarm;
  if (l == "gray" || l == "grayscale") return ColorMap::Gray;
  return ColorMap::Viridis;
}

ColorNorm parse_color_norm(const std::string& s) {
  const auto l = lower(trim(s));
  if (l == "log" || l == "logarithmic") return ColorNorm::Log;
  return ColorNorm::Linear;
}

}  // namespace

YamlFigureSpec load_yaml_figure_spec(const std::filesystem::path& path) {
  std::ifstream in(path);
  if (!in.is_open()) {
    throw std::runtime_error("failed to open YAML spec: " + path.string());
  }

  YamlFigureSpec out;
  out.figure = FigureSpec{};
  std::string section;
  AxesSpec current_axis;
  bool in_axis = false;
  std::string axis_subsection;
  std::size_t line_no = 0;

  std::string line;
  while (std::getline(in, line)) {
    ++line_no;
    const auto first_non_ws = line.find_first_not_of(" \t");
    const int indent = first_non_ws == std::string::npos ? 0 : static_cast<int>(first_non_ws);
    const auto raw = trim(line);
    if (raw.empty() || raw[0] == '#') {
      continue;
    }
    if (raw == "figure:") {
      section = "figure";
      continue;
    }
    if (raw == "axes:") {
      section = "axes";
      continue;
    }
    if (section == "axes" && raw.rfind("- ", 0) == 0) {
      if (in_axis) {
        out.axes.push_back(current_axis);
      }
      current_axis = AxesSpec{};
      in_axis = true;
      axis_subsection.clear();
      const auto rest = trim(raw.substr(2));
      if (rest.empty()) {
        continue;
      }
      // fall through parsing "key: value" after "- "
      line = rest;
    } else if (section != "figure" && section != "axes") {
      continue;
    }

    const auto colon = line.find(':');
    if (colon == std::string::npos) {
      continue;
    }
    const auto key = trim(line.substr(0, colon));
    auto value = trim(line.substr(colon + 1));
    if (!value.empty() && value.size() >= 2 &&
        ((value.front() == '"' && value.back() == '"') ||
         (value.front() == '\'' && value.back() == '\''))) {
      value = value.substr(1, value.size() - 2);
    }

    try {
      if (section == "figure") {
        if (key == "title") out.figure.title = value;
        else if (key == "caption") out.figure.caption = value;
        else if (key == "preset") out.figure.preset = parse_preset(value);
        else if (key == "palette") out.figure.palette = parse_palette(value);
        else if (key == "rows") out.figure.rows = std::max(1, std::stoi(value));
        else if (key == "cols") out.figure.cols = std::max(1, std::stoi(value));
        else if (key == "panel_labels") out.figure.panel_labels = parse_bool(value);
        else if (key == "auto_layout") out.figure.auto_layout = parse_bool(value);
        else if (key == "interactive_preview") out.figure.interactive_preview = parse_bool(value);
        else if (key == "font") out.figure.style.font = value;
        else if (key == "font_pt") out.figure.style.font_pt = std::stod(value);
        else if (key == "formats") {
          out.figure.formats.clear();
          auto v = value;
          if (!v.empty() && v.front() == '[' && v.back() == ']') {
            v = v.substr(1, v.size() - 2);
          }
          std::stringstream ss(v);
          std::string tok;
          while (std::getline(ss, tok, ',')) {
            out.figure.formats.push_back(parse_format(tok));
          }
        }
      } else if (section == "axes" && in_axis) {
        if (indent <= 4 && value.empty() &&
            (key == "typography" || key == "frame" || key == "legend_spec")) {
          axis_subsection = key;
          continue;
        }
        if (indent <= 4 && key != "typography" && key != "frame" && key != "legend_spec") {
          axis_subsection.clear();
        }
        std::string axis_key = key;
        if (!axis_subsection.empty() && indent >= 6) {
          axis_key = axis_subsection + "." + key;
        }
        if (axis_key == "title") current_axis.title = value;
        else if (axis_key == "xlabel") current_axis.xlabel = value;
        else if (axis_key == "ylabel") current_axis.ylabel = value;
        else if (axis_key == "y2label") current_axis.y2label = value;
        else if (axis_key == "grid") current_axis.grid = parse_bool(value);
        else if (axis_key == "legend") current_axis.legend = parse_bool(value);
        else if (axis_key == "enable_crosshair") current_axis.enable_crosshair = parse_bool(value);
        else if (axis_key == "xlog") current_axis.xlog = parse_bool(value);
        else if (axis_key == "ylog") current_axis.ylog = parse_bool(value);
        else if (axis_key == "y2log") current_axis.y2log = parse_bool(value);
        else if (axis_key == "xmin") {
          current_axis.has_xlim = true;
          current_axis.xmin = std::stod(value);
        } else if (axis_key == "xmax") {
          current_axis.has_xlim = true;
          current_axis.xmax = std::stod(value);
        } else if (axis_key == "ymin") {
          current_axis.has_ylim = true;
          current_axis.ymin = std::stod(value);
        } else if (axis_key == "ymax") {
          current_axis.has_ylim = true;
          current_axis.ymax = std::stod(value);
        } else if (axis_key == "y2min") {
          current_axis.has_y2lim = true;
          current_axis.y2min = std::stod(value);
        } else if (axis_key == "y2max") {
          current_axis.has_y2lim = true;
          current_axis.y2max = std::stod(value);
        } else if (axis_key == "has_xtick_step") {
          current_axis.has_xtick_step = parse_bool(value);
        } else if (axis_key == "xtick_step") {
          current_axis.has_xtick_step = true;
          current_axis.xtick_step = std::stod(value);
        } else if (axis_key == "has_ytick_step") {
          current_axis.has_ytick_step = parse_bool(value);
        } else if (axis_key == "ytick_step") {
          current_axis.has_ytick_step = true;
          current_axis.ytick_step = std::stod(value);
        } else if (axis_key == "has_xminor_count") {
          current_axis.has_xminor_count = parse_bool(value);
        } else if (axis_key == "xminor_count") {
          current_axis.has_xminor_count = true;
          current_axis.xminor_count = std::stoi(value);
        } else if (axis_key == "has_yminor_count") {
          current_axis.has_yminor_count = parse_bool(value);
        } else if (axis_key == "yminor_count") {
          current_axis.has_yminor_count = true;
          current_axis.yminor_count = std::stoi(value);
        } else if (axis_key == "xformat") {
          current_axis.xformat = value;
        } else if (axis_key == "yformat") {
          current_axis.yformat = value;
        } else if (axis_key == "color_map") {
          current_axis.color_map = parse_colormap(value);
        } else if (axis_key == "color_norm") {
          current_axis.color_norm = parse_color_norm(value);
        } else if (axis_key == "colorbar_label") {
          current_axis.colorbar_label = value;
        } else if (axis_key == "cbmin") {
          current_axis.has_cbrange = true;
          current_axis.cbmin = std::stod(value);
        } else if (axis_key == "cbmax") {
          current_axis.has_cbrange = true;
          current_axis.cbmax = std::stod(value);
        } else if (axis_key == "cbtick_step") {
          current_axis.has_cbtick_step = true;
          current_axis.cbtick_step = std::stod(value);
        } else if (axis_key == "legend_spec.position") {
          current_axis.legend_spec.position = parse_legend_position(value);
        } else if (axis_key == "legend_spec.columns") {
          current_axis.legend_spec.columns = std::max(1, std::stoi(value));
        } else if (axis_key == "legend_spec.boxed") {
          current_axis.legend_spec.boxed = parse_bool(value);
        } else if (axis_key == "legend_spec.opaque") {
          current_axis.legend_spec.opaque = parse_bool(value);
        } else if (axis_key == "legend_spec.font_pt") {
          current_axis.legend_spec.has_font_pt = true;
          current_axis.legend_spec.font_pt = std::stod(value);
        } else if (axis_key == "typography.tick_font_pt") {
          current_axis.typography.has_tick_font_pt = true;
          current_axis.typography.tick_font_pt = std::stod(value);
        } else if (axis_key == "typography.label_font_pt") {
          current_axis.typography.has_label_font_pt = true;
          current_axis.typography.label_font_pt = std::stod(value);
        } else if (axis_key == "typography.title_font_pt") {
          current_axis.typography.has_title_font_pt = true;
          current_axis.typography.title_font_pt = std::stod(value);
        } else if (axis_key == "typography.title_bold") {
          current_axis.typography.has_title_bold = true;
          current_axis.typography.title_bold = parse_bool(value);
        } else if (axis_key == "frame.border_mask") {
          current_axis.frame.has_border_mask = true;
          current_axis.frame.border_mask = std::stoi(value);
        } else if (axis_key == "frame.border_line_width_pt") {
          current_axis.frame.has_border_line_width_pt = true;
          current_axis.frame.border_line_width_pt = std::stod(value);
        } else if (axis_key == "frame.border_color") {
          current_axis.frame.has_border_color = true;
          current_axis.frame.border_color = value;
        } else if (axis_key == "frame.ticks_out") {
          current_axis.frame.has_ticks_out = true;
          current_axis.frame.ticks_out = parse_bool(value);
        } else if (axis_key == "frame.ticks_mirror") {
          current_axis.frame.has_ticks_mirror = true;
          current_axis.frame.ticks_mirror = parse_bool(value);
        }
      }
    } catch (const std::exception& e) {
      throw std::runtime_error("YAML parse error at line " + std::to_string(line_no) + " ('" +
                               key + "'): " + e.what());
    }
  }

  if (in_axis) {
    out.axes.push_back(current_axis);
  }
  if (out.figure.formats.empty()) {
    out.figure.formats = {OutputFormat::Pdf};
  }
  return out;
}

}  // namespace gnuplotpp
