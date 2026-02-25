#include "gnuplotpp/theme.hpp"

#include <fstream>
#include <sstream>
#include <string>

namespace gnuplotpp {
namespace {

template <typename T>
bool parse_scalar(const std::string& content, const std::string& key, T& out) {
  const auto k = "\"" + key + "\"";
  const auto p = content.find(k);
  if (p == std::string::npos) {
    return false;
  }
  const auto c = content.find(':', p + k.size());
  if (c == std::string::npos) {
    return false;
  }
  std::istringstream iss(content.substr(c + 1));
  iss >> out;
  return !iss.fail();
}

bool parse_string(const std::string& content, const std::string& key, std::string& out) {
  const auto k = "\"" + key + "\"";
  const auto p = content.find(k);
  if (p == std::string::npos) {
    return false;
  }
  const auto q1 = content.find('"', p + k.size());
  if (q1 == std::string::npos) {
    return false;
  }
  const auto q2 = content.find('"', q1 + 1);
  if (q2 == std::string::npos) {
    return false;
  }
  out = content.substr(q1 + 1, q2 - q1 - 1);
  return true;
}

}  // namespace

bool save_theme_json(const std::filesystem::path& path, const FigureSpec& spec) {
  std::ofstream os(path);
  if (!os.is_open()) {
    return false;
  }
  os << "{\n";
  os << "  \"palette\": " << static_cast<int>(spec.palette) << ",\n";
  os << "  \"text_mode\": " << static_cast<int>(spec.text_mode) << ",\n";
  os << "  \"font\": \"" << spec.style.font << "\",\n";
  os << "  \"font_pt\": " << spec.style.font_pt << ",\n";
  os << "  \"line_width_pt\": " << spec.style.line_width_pt << ",\n";
  os << "  \"point_size\": " << spec.style.point_size << ",\n";
  os << "  \"grid\": " << (spec.style.grid ? 1 : 0) << "\n";
  os << "}\n";
  return os.good();
}

bool load_theme_json(const std::filesystem::path& path, FigureSpec& spec) {
  std::ifstream in(path);
  if (!in.is_open()) {
    return false;
  }
  const std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  int palette = static_cast<int>(spec.palette);
  int text_mode = static_cast<int>(spec.text_mode);
  int grid = spec.style.grid ? 1 : 0;
  parse_scalar(content, "palette", palette);
  parse_scalar(content, "text_mode", text_mode);
  parse_string(content, "font", spec.style.font);
  parse_scalar(content, "font_pt", spec.style.font_pt);
  parse_scalar(content, "line_width_pt", spec.style.line_width_pt);
  parse_scalar(content, "point_size", spec.style.point_size);
  parse_scalar(content, "grid", grid);

  spec.palette = static_cast<ColorPalette>(palette);
  spec.text_mode = static_cast<TextMode>(text_mode);
  spec.style.grid = grid != 0;
  return true;
}

}  // namespace gnuplotpp
