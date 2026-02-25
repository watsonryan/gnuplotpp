#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace gnuplotpp {

enum class OutputFormat { Pdf, Svg, Eps, Png };

enum class Preset {
  IEEE_SingleColumn,
  IEEE_DoubleColumn,
  AIAA_Column,
  AIAA_Page,
  Custom
};

struct FigureSizeInches {
  double w = 3.5;
  double h = 2.5;
};

struct Style {
  std::string font = "Times-New-Roman";
  double font_pt = 9.0;
  double line_width_pt = 1.0;
  double point_size = 0.6;
  bool grid = false;
};

struct FigureSpec {
  Preset preset = Preset::IEEE_SingleColumn;
  FigureSizeInches size{};
  Style style{};
  int rows = 1;
  int cols = 1;
  std::vector<OutputFormat> formats{OutputFormat::Pdf};
  std::string title;
};

struct AxesSpec {
  std::string title;
  std::string xlabel;
  std::string ylabel;
  bool grid = false;
  bool legend = true;

  bool has_xlim = false;
  double xmin = 0.0;
  double xmax = 0.0;

  bool has_ylim = false;
  double ymin = 0.0;
  double ymax = 0.0;

  bool xlog = false;
  bool ylog = false;
};

enum class SeriesType { Line, Scatter, ErrorBars, Band };

struct SeriesSpec {
  SeriesType type = SeriesType::Line;
  std::string label;
  bool has_line_width = false;
  double line_width_pt = 1.0;
};

struct SeriesData {
  SeriesSpec spec;
  std::vector<double> x;
  std::vector<double> y;
};

class Axes {
public:
  void set(AxesSpec spec);

  void add_series(const SeriesSpec& spec,
                  std::span<const double> x,
                  std::span<const double> y);

  const AxesSpec& spec() const { return spec_; }
  const std::vector<SeriesData>& series() const { return series_; }

private:
  AxesSpec spec_{};
  std::vector<SeriesData> series_{};
};

class Figure;

struct RenderResult {
  bool ok = true;
  std::string message;
  std::filesystem::path script_path;
  std::vector<std::filesystem::path> outputs;
};

class IPlotBackend {
public:
  virtual ~IPlotBackend() = default;
  virtual RenderResult render(const Figure& fig,
                              const std::filesystem::path& out_dir) = 0;
};

class Figure {
public:
  explicit Figure(FigureSpec spec);

  Axes& axes(int r, int c);
  Axes& axes(int idx);

  const FigureSpec& spec() const { return spec_; }
  const std::vector<Axes>& all_axes() const { return axes_; }

  RenderResult save(const std::filesystem::path& out_dir) const;

  void set_backend(std::shared_ptr<IPlotBackend> backend);

private:
  FigureSpec spec_{};
  std::vector<Axes> axes_{};
  std::shared_ptr<IPlotBackend> backend_{};
};

}  // namespace gnuplotpp
