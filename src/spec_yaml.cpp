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

  std::string line;
  while (std::getline(in, line)) {
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
    if (!value.empty() && value.front() == '"' && value.back() == '"') {
      value = value.substr(1, value.size() - 2);
    }

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
      if (key == "title") current_axis.title = value;
      else if (key == "xlabel") current_axis.xlabel = value;
      else if (key == "ylabel") current_axis.ylabel = value;
      else if (key == "y2label") current_axis.y2label = value;
      else if (key == "grid") current_axis.grid = parse_bool(value);
      else if (key == "legend") current_axis.legend = parse_bool(value);
      else if (key == "xlog") current_axis.xlog = parse_bool(value);
      else if (key == "ylog") current_axis.ylog = parse_bool(value);
      else if (key == "xmin") {
        current_axis.has_xlim = true;
        current_axis.xmin = std::stod(value);
      } else if (key == "xmax") {
        current_axis.has_xlim = true;
        current_axis.xmax = std::stod(value);
      } else if (key == "ymin") {
        current_axis.has_ylim = true;
        current_axis.ymin = std::stod(value);
      } else if (key == "ymax") {
        current_axis.has_ylim = true;
        current_axis.ymax = std::stod(value);
      }
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
