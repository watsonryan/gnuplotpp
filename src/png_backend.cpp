#include "gnuplotpp/png_backend.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace gnuplotpp {
namespace {

struct AxisBounds {
  double xmin = 0.0;
  double xmax = 1.0;
  double ymin = 0.0;
  double ymax = 1.0;
};

struct Color {
  std::uint8_t r = 0;
  std::uint8_t g = 0;
  std::uint8_t b = 0;
  std::uint8_t a = 255;
};

class Canvas {
public:
  Canvas(int w, int h, Color bg) : w_(w), h_(h), pix_(static_cast<std::size_t>(w * h * 4U), 0) {
    for (int y = 0; y < h_; ++y) {
      for (int x = 0; x < w_; ++x) {
        set(x, y, bg);
      }
    }
  }

  int width() const { return w_; }
  int height() const { return h_; }
  const std::vector<std::uint8_t>& data() const { return pix_; }

  void set(int x, int y, Color c) {
    if (x < 0 || y < 0 || x >= w_ || y >= h_) {
      return;
    }
    const std::size_t idx = static_cast<std::size_t>((y * w_ + x) * 4);
    pix_[idx + 0] = c.r;
    pix_[idx + 1] = c.g;
    pix_[idx + 2] = c.b;
    pix_[idx + 3] = c.a;
  }

  void dot(int x, int y, int radius, Color c) {
    const int r2 = radius * radius;
    for (int yy = y - radius; yy <= y + radius; ++yy) {
      for (int xx = x - radius; xx <= x + radius; ++xx) {
        const int dx = xx - x;
        const int dy = yy - y;
        if (dx * dx + dy * dy <= r2) {
          set(xx, yy, c);
        }
      }
    }
  }

  void line(int x0, int y0, int x1, int y1, Color c, int thickness = 1) {
    const int dx = std::abs(x1 - x0);
    const int sx = x0 < x1 ? 1 : -1;
    const int dy = -std::abs(y1 - y0);
    const int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    const int radius = std::max(1, thickness) / 2;

    while (true) {
      dot(x0, y0, radius, c);
      if (x0 == x1 && y0 == y1) {
        break;
      }
      const int e2 = 2 * err;
      if (e2 >= dy) {
        err += dy;
        x0 += sx;
      }
      if (e2 <= dx) {
        err += dx;
        y0 += sy;
      }
    }
  }

  void rect(int x, int y, int w, int h, Color c, int thickness = 1) {
    line(x, y, x + w, y, c, thickness);
    line(x + w, y, x + w, y + h, c, thickness);
    line(x + w, y + h, x, y + h, c, thickness);
    line(x, y + h, x, y, c, thickness);
  }

private:
  int w_;
  int h_;
  std::vector<std::uint8_t> pix_;
};

AxisBounds compute_bounds(const Axes& axis) {
  AxisBounds b;
  const auto& spec = axis.spec();

  if (spec.has_xlim) {
    b.xmin = spec.xmin;
    b.xmax = spec.xmax;
  } else {
    b.xmin = 1e300;
    b.xmax = -1e300;
    for (const auto& s : axis.series()) {
      for (const auto x : s.x) {
        b.xmin = std::min(b.xmin, x);
        b.xmax = std::max(b.xmax, x);
      }
    }
    if (!(std::isfinite(b.xmin) && std::isfinite(b.xmax)) || b.xmin == b.xmax) {
      b.xmin = 0.0;
      b.xmax = 1.0;
    }
  }

  if (spec.has_ylim) {
    b.ymin = spec.ymin;
    b.ymax = spec.ymax;
  } else {
    b.ymin = 1e300;
    b.ymax = -1e300;
    for (const auto& s : axis.series()) {
      for (const auto y : s.y) {
        b.ymin = std::min(b.ymin, y);
        b.ymax = std::max(b.ymax, y);
      }
    }
    if (!(std::isfinite(b.ymin) && std::isfinite(b.ymax)) || b.ymin == b.ymax) {
      b.ymin = 0.0;
      b.ymax = 1.0;
    }
  }

  const double xpad = (b.xmax - b.xmin) * 0.03;
  const double ypad = (b.ymax - b.ymin) * 0.06;
  b.xmin -= xpad;
  b.xmax += xpad;
  b.ymin -= ypad;
  b.ymax += ypad;
  return b;
}

Color palette(std::size_t i) {
  static constexpr std::array<Color, 8> k{
      Color{0x00, 0x72, 0xB2, 0xFF}, Color{0xD5, 0x5E, 0x00, 0xFF},
      Color{0x00, 0x9E, 0x73, 0xFF}, Color{0xCC, 0x79, 0xA7, 0xFF},
      Color{0x56, 0xB4, 0xE9, 0xFF}, Color{0xE6, 0x9F, 0x00, 0xFF},
      Color{0xF0, 0xE4, 0x42, 0xFF}, Color{0x00, 0x00, 0x00, 0xFF}};
  return k[i % k.size()];
}

std::array<std::uint8_t, 7> glyph(char ch) {
  switch (static_cast<char>(std::toupper(static_cast<unsigned char>(ch)))) {
    case '0': return {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E};
    case '1': return {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E};
    case '2': return {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F};
    case '3': return {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E};
    case '4': return {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02};
    case '5': return {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E};
    case '6': return {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E};
    case '7': return {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08};
    case '8': return {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E};
    case '9': return {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C};
    case 'A': return {0x04, 0x0A, 0x11, 0x11, 0x1F, 0x11, 0x11};
    case 'B': return {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E};
    case 'C': return {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E};
    case 'D': return {0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C};
    case 'E': return {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F};
    case 'F': return {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10};
    case 'G': return {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0E};
    case 'H': return {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
    case 'I': return {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E};
    case 'L': return {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F};
    case 'M': return {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11};
    case 'N': return {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11};
    case 'O': return {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    case 'P': return {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10};
    case 'R': return {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11};
    case 'S': return {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E};
    case 'T': return {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
    case 'U': return {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    case 'V': return {0x11, 0x11, 0x11, 0x11, 0x0A, 0x0A, 0x04};
    case 'X': return {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11};
    case 'Y': return {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04};
    case '[': return {0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E};
    case ']': return {0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E};
    case '_': return {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F};
    case '/': return {0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10};
    case '-': return {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00};
    case '.': return {0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06};
    case ':': return {0x00, 0x06, 0x06, 0x00, 0x06, 0x06, 0x00};
    case '|': return {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
    case ' ': return {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    default: return {0x1F, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04};
  }
}

int text_width_px(const std::string& s, int scale) {
  return static_cast<int>(s.size()) * (6 * scale);
}

void draw_char(Canvas& cv, int x, int y, char ch, int scale, Color c) {
  const auto g = glyph(ch);
  for (int row = 0; row < 7; ++row) {
    for (int col = 0; col < 5; ++col) {
      if ((g[static_cast<std::size_t>(row)] >> (4 - col)) & 0x1) {
        for (int yy = 0; yy < scale; ++yy) {
          for (int xx = 0; xx < scale; ++xx) {
            cv.set(x + col * scale + xx, y + row * scale + yy, c);
          }
        }
      }
    }
  }
}

void draw_text(Canvas& cv, int x, int y, const std::string& s, int scale, Color c) {
  int cursor = x;
  for (char ch : s) {
    draw_char(cv, cursor, y, ch, scale, c);
    cursor += 6 * scale;
  }
}

void draw_text_vertical(Canvas& cv, int x, int y, const std::string& s, int scale, Color c) {
  int cursor = y;
  for (char ch : s) {
    draw_char(cv, x, cursor, ch, scale, c);
    cursor += 8 * scale;
  }
}

std::string fmt_tick(double v, double span) {
  std::ostringstream os;
  const double av = std::abs(v);
  if ((av > 0.0 && av < 1e-2) || av >= 1e4 || span < 1e-2) {
    os << std::scientific << std::setprecision(1) << v;
  } else if (span < 1.0) {
    os << std::fixed << std::setprecision(2) << v;
  } else if (span < 10.0) {
    os << std::fixed << std::setprecision(1) << v;
  } else {
    os << std::fixed << std::setprecision(0) << v;
  }
  return os.str();
}

std::uint32_t crc32(const std::uint8_t* data, std::size_t len) {
  std::uint32_t crc = 0xFFFFFFFFu;
  for (std::size_t i = 0; i < len; ++i) {
    crc ^= static_cast<std::uint32_t>(data[i]);
    for (int k = 0; k < 8; ++k) {
      const std::uint32_t mask = -(crc & 1u);
      crc = (crc >> 1) ^ (0xEDB88320u & mask);
    }
  }
  return ~crc;
}

std::uint32_t adler32(const std::uint8_t* data, std::size_t len) {
  constexpr std::uint32_t mod = 65521u;
  std::uint32_t a = 1u;
  std::uint32_t b = 0u;
  for (std::size_t i = 0; i < len; ++i) {
    a = (a + data[i]) % mod;
    b = (b + a) % mod;
  }
  return (b << 16) | a;
}

void append_u32_be(std::vector<std::uint8_t>& out, std::uint32_t v) {
  out.push_back(static_cast<std::uint8_t>((v >> 24) & 0xFF));
  out.push_back(static_cast<std::uint8_t>((v >> 16) & 0xFF));
  out.push_back(static_cast<std::uint8_t>((v >> 8) & 0xFF));
  out.push_back(static_cast<std::uint8_t>(v & 0xFF));
}

void append_chunk(std::vector<std::uint8_t>& out,
                  const char type[4],
                  const std::vector<std::uint8_t>& data) {
  append_u32_be(out, static_cast<std::uint32_t>(data.size()));
  const std::size_t type_pos = out.size();
  out.push_back(static_cast<std::uint8_t>(type[0]));
  out.push_back(static_cast<std::uint8_t>(type[1]));
  out.push_back(static_cast<std::uint8_t>(type[2]));
  out.push_back(static_cast<std::uint8_t>(type[3]));
  out.insert(out.end(), data.begin(), data.end());
  const std::uint32_t crc = crc32(out.data() + type_pos, 4 + data.size());
  append_u32_be(out, crc);
}

bool write_png_rgba(const std::filesystem::path& path,
                    int width,
                    int height,
                    const std::vector<std::uint8_t>& rgba) {
  if (width <= 0 || height <= 0) {
    return false;
  }

  std::vector<std::uint8_t> raw;
  raw.reserve(static_cast<std::size_t>(height) * static_cast<std::size_t>(1 + width * 4));
  for (int y = 0; y < height; ++y) {
    raw.push_back(0);
    const std::size_t row = static_cast<std::size_t>(y * width * 4);
    raw.insert(raw.end(), rgba.begin() + static_cast<std::ptrdiff_t>(row),
               rgba.begin() + static_cast<std::ptrdiff_t>(row + width * 4));
  }

  std::vector<std::uint8_t> z;
  z.push_back(0x78);
  z.push_back(0x01);

  std::size_t off = 0;
  while (off < raw.size()) {
    const std::size_t rem = raw.size() - off;
    const std::uint16_t block = static_cast<std::uint16_t>(std::min<std::size_t>(rem, 65535u));
    const bool final = (off + block == raw.size());
    z.push_back(final ? 0x01 : 0x00);
    z.push_back(static_cast<std::uint8_t>(block & 0xFF));
    z.push_back(static_cast<std::uint8_t>((block >> 8) & 0xFF));
    const std::uint16_t nlen = static_cast<std::uint16_t>(~block);
    z.push_back(static_cast<std::uint8_t>(nlen & 0xFF));
    z.push_back(static_cast<std::uint8_t>((nlen >> 8) & 0xFF));
    z.insert(z.end(), raw.begin() + static_cast<std::ptrdiff_t>(off),
             raw.begin() + static_cast<std::ptrdiff_t>(off + block));
    off += block;
  }
  append_u32_be(z, adler32(raw.data(), raw.size()));

  std::vector<std::uint8_t> png;
  const std::array<std::uint8_t, 8> sig{0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
  png.insert(png.end(), sig.begin(), sig.end());

  std::vector<std::uint8_t> ihdr;
  append_u32_be(ihdr, static_cast<std::uint32_t>(width));
  append_u32_be(ihdr, static_cast<std::uint32_t>(height));
  ihdr.push_back(8);
  ihdr.push_back(6);
  ihdr.push_back(0);
  ihdr.push_back(0);
  ihdr.push_back(0);

  append_chunk(png, "IHDR", ihdr);
  append_chunk(png, "IDAT", z);
  append_chunk(png, "IEND", {});

  std::ofstream out(path, std::ios::binary);
  if (!out.is_open()) {
    return false;
  }
  out.write(reinterpret_cast<const char*>(png.data()), static_cast<std::streamsize>(png.size()));
  return out.good();
}

}  // namespace

RenderResult PngBackend::render(const Figure& fig,
                                const std::filesystem::path& out_dir) {
  RenderResult r;
  r.status = RenderStatus::Success;

  std::error_code ec;
  std::filesystem::create_directories(out_dir, ec);
  if (ec) {
    r.ok = false;
    r.status = RenderStatus::IoError;
    r.message = "failed to create output directory: " + ec.message();
    return r;
  }

  bool requested_png = false;
  for (const auto fmt : fig.spec().formats) {
    if (fmt == OutputFormat::Png) {
      requested_png = true;
      break;
    }
  }
  if (!requested_png) {
    r.ok = false;
    r.status = RenderStatus::UnsupportedFormat;
    r.message = "PngBackend only supports OutputFormat::Png";
    return r;
  }

  const auto& spec = fig.spec();
  const int dpi = 300;
  const int w = std::max(1, static_cast<int>(std::round(spec.size.w * static_cast<double>(dpi))));
  const int h = std::max(1, static_cast<int>(std::round(spec.size.h * static_cast<double>(dpi))));
  Canvas cv(w, h, Color{255, 255, 255, 255});

  const int font_scale = std::max(1, static_cast<int>(std::round(spec.style.font_pt / 4.0)));
  const int axis_thickness = std::max(1, static_cast<int>(std::round(0.8 * dpi / 72.0)));
  const int line_thickness =
      std::max(1, static_cast<int>(std::round(spec.style.line_width_pt * dpi / 72.0)));

  const int left = 90;
  const int top = 50;
  const int right = 30;
  const int bottom = 70;
  const int cell_w = std::max(1, (w - left - right) / spec.cols);
  const int cell_h = std::max(1, (h - top - bottom) / spec.rows);

  if (!spec.title.empty()) {
    const int tw = text_width_px(spec.title, font_scale);
    draw_text(cv, (w - tw) / 2, 10, spec.title, font_scale, Color{0, 0, 0, 255});
  }

  const auto& axes = fig.all_axes();
  for (std::size_t idx = 0; idx < axes.size(); ++idx) {
    const int row = static_cast<int>(idx) / spec.cols;
    const int col = static_cast<int>(idx) % spec.cols;
    const int x0 = left + col * cell_w;
    const int y0 = top + row * cell_h;

    const int plot_l = x0 + 18;
    const int plot_t = y0 + 24;
    const int plot_r = x0 + cell_w - 24;
    const int plot_b = y0 + cell_h - 30;
    const int plot_w = std::max(1, plot_r - plot_l);
    const int plot_h = std::max(1, plot_b - plot_t);

    const auto& axis = axes[idx];
    const auto& axis_spec = axis.spec();
    const auto b = compute_bounds(axis);

    if (!axis_spec.title.empty()) {
      const int tw = text_width_px(axis_spec.title, font_scale);
      draw_text(cv, plot_l + (plot_w - tw) / 2, y0 + 4, axis_spec.title, font_scale,
                Color{0, 0, 0, 255});
    }

    if (axis_spec.grid || spec.style.grid) {
      for (int g = 1; g < 5; ++g) {
        const int gx = plot_l + (plot_w * g) / 5;
        const int gy = plot_t + (plot_h * g) / 5;
        cv.line(gx, plot_t, gx, plot_b, Color{228, 228, 228, 255}, 1);
        cv.line(plot_l, gy, plot_r, gy, Color{228, 228, 228, 255}, 1);
      }
    }

    cv.rect(plot_l, plot_t, plot_w, plot_h, Color{20, 20, 20, 255}, axis_thickness);

    const auto map_x = [&](double x) {
      return plot_l + static_cast<int>((x - b.xmin) * static_cast<double>(plot_w) / (b.xmax - b.xmin));
    };
    const auto map_y = [&](double y) {
      return plot_b - static_cast<int>((y - b.ymin) * static_cast<double>(plot_h) / (b.ymax - b.ymin));
    };

    const int ticks = 5;
    for (int t = 0; t <= ticks; ++t) {
      const double fx = static_cast<double>(t) / ticks;
      const int tx = plot_l + static_cast<int>(fx * plot_w);
      cv.line(tx, plot_b, tx, plot_b + 8, Color{20, 20, 20, 255}, axis_thickness);
      const double xv = b.xmin + fx * (b.xmax - b.xmin);
      const auto label = fmt_tick(xv, b.xmax - b.xmin);
      const int tw = text_width_px(label, font_scale);
      draw_text(cv, tx - tw / 2, plot_b + 12, label, font_scale, Color{0, 0, 0, 255});

      const double fy = static_cast<double>(t) / ticks;
      const int ty = plot_b - static_cast<int>(fy * plot_h);
      cv.line(plot_l - 8, ty, plot_l, ty, Color{20, 20, 20, 255}, axis_thickness);
      const double yv = b.ymin + fy * (b.ymax - b.ymin);
      const auto ylabel = fmt_tick(yv, b.ymax - b.ymin);
      const int yw = text_width_px(ylabel, font_scale);
      draw_text(cv, plot_l - 12 - yw, ty - 3 * font_scale, ylabel, font_scale,
                Color{0, 0, 0, 255});
    }

    if (!axis_spec.xlabel.empty()) {
      const int tw = text_width_px(axis_spec.xlabel, font_scale);
      draw_text(cv, plot_l + (plot_w - tw) / 2, plot_b + 36, axis_spec.xlabel, font_scale,
                Color{0, 0, 0, 255});
    }
    if (!axis_spec.ylabel.empty()) {
      draw_text_vertical(cv, x0 + 2, plot_t + 4, axis_spec.ylabel, font_scale, Color{0, 0, 0, 255});
    }

    for (std::size_t s = 0; s < axis.series().size(); ++s) {
      const auto color = palette(s);
      const auto& series = axis.series()[s];
      if (series.x.size() < 2) {
        continue;
      }
      const int lw = series.spec.has_line_width
                         ? std::max(1, static_cast<int>(std::round(series.spec.line_width_pt * dpi / 72.0)))
                         : line_thickness;
      for (std::size_t i = 1; i < series.x.size(); ++i) {
        if (!std::isfinite(series.x[i - 1]) || !std::isfinite(series.y[i - 1]) ||
            !std::isfinite(series.x[i]) || !std::isfinite(series.y[i])) {
          continue;
        }
        cv.line(map_x(series.x[i - 1]), map_y(series.y[i - 1]), map_x(series.x[i]),
                map_y(series.y[i]), color, lw);
      }
    }
  }

  const auto out = out_dir / "figure.png";
  if (!write_png_rgba(out, cv.width(), cv.height(), cv.data())) {
    r.ok = false;
    r.status = RenderStatus::IoError;
    r.message = "failed writing PNG output";
    return r;
  }

  r.ok = true;
  r.status = RenderStatus::Success;
  r.message = "native png render completed";
  r.outputs.push_back(out);
  return r;
}

std::shared_ptr<IPlotBackend> make_png_backend() {
  return std::make_shared<PngBackend>();
}

}  // namespace gnuplotpp
