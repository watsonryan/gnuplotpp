#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

/**
 * @namespace gnuplotpp
 * @brief Pure C++ plotting API that renders through a gnuplot backend.
 */
namespace gnuplotpp {

/** @brief Output file formats supported by the renderer. */
enum class OutputFormat { Pdf, Svg, Eps, Png };

/** @brief Figure-wide palette selection for automatic series colors. */
enum class ColorPalette { Default, Tab10, Viridis, Grayscale };

/** @brief Text rendering mode used by terminal setup. */
enum class TextMode { Enhanced, Plain, LaTeX };

/** @brief Built-in publication presets for size and style defaults. */
enum class Preset {
  IEEE_SingleColumn,
  IEEE_DoubleColumn,
  AIAA_Column,
  AIAA_Page,
  IEEE_Tran,
  Nature_1Col,
  Elsevier_1Col,
  Custom
};

/** @brief Figure dimensions in inches. */
struct FigureSizeInches {
  double w = 3.5;
  double h = 2.5;
};

/** @brief Shared style defaults applied at figure scope. */
struct Style {
  std::string font = "Times-New-Roman";
  double font_pt = 9.0;
  double line_width_pt = 1.0;
  double point_size = 0.6;
  bool grid = false;
};

/** @brief High-level figure configuration. */
struct FigureSpec {
  Preset preset = Preset::IEEE_SingleColumn;
  FigureSizeInches size{};
  Style style{};
  ColorPalette palette = ColorPalette::Default;
  TextMode text_mode = TextMode::Enhanced;
  int rows = 1;
  int cols = 1;
  std::vector<OutputFormat> formats{OutputFormat::Pdf};
  std::string title;
  bool write_manifest = false;
  bool share_x = false;
  bool share_y = false;
  bool hide_inner_tick_labels = false;
  bool auto_layout = true;
  bool interactive_preview = false;
};

/** @brief Legend placement presets. */
enum class LegendPosition {
  TopRight,
  TopLeft,
  BottomRight,
  BottomLeft,
  OutsideRight,
  OutsideBottom
};

/** @brief Legend configuration for one axes. */
struct LegendSpec {
  bool enabled = true;
  LegendPosition position = LegendPosition::TopRight;
  int columns = 1;
  bool boxed = false;
  bool opaque = false;
  bool has_font_pt = false;
  double font_pt = 8.0;
};

/** @brief Typed label annotation. */
struct LabelAnnotation {
  std::string text;
  std::string at = "graph 0.05,0.95";
  std::string font;
  bool front = true;
};

/** @brief Typed arrow annotation. */
struct ArrowAnnotation {
  std::string from = "graph 0.1,0.9";
  std::string to = "graph 0.2,0.8";
  bool heads = true;
  double line_width_pt = 1.0;
  std::string color = "#000000";
  bool front = true;
};

/** @brief Typed rectangle object for highlights/masks. */
struct RectObject {
  std::string from = "graph 0.1,0.1";
  std::string to = "graph 0.2,0.2";
  bool has_fill_opacity = false;
  double fill_opacity = 0.15;
  std::string fill_color = "#000000";
  bool border = false;
  std::string border_color = "#000000";
  bool front = false;
};

/** @brief Axes-level labels, limits, and grid/log controls. */
struct AxesSpec {
  std::string title;
  std::string xlabel;
  std::string ylabel;
  std::string y2label;
  bool grid = false;
  bool legend = true;
  LegendSpec legend_spec{};
  bool enable_crosshair = false;

  bool has_xlim = false;
  double xmin = 0.0;
  double xmax = 0.0;

  bool has_ylim = false;
  double ymin = 0.0;
  double ymax = 0.0;

  bool xlog = false;
  bool ylog = false;
  bool y2log = false;

  bool has_y2lim = false;
  double y2min = 0.0;
  double y2max = 0.0;

  bool has_xtick_step = false;
  double xtick_step = 1.0;
  bool has_ytick_step = false;
  double ytick_step = 1.0;
  bool has_xminor_count = false;
  int xminor_count = 2;
  bool has_yminor_count = false;
  int yminor_count = 2;
  std::string xformat;
  std::string yformat;

  std::vector<LabelAnnotation> labels;
  std::vector<ArrowAnnotation> arrows;
  std::vector<RectObject> rectangles;

  // Optional raw gnuplot commands for advanced annotations (arrows/labels/etc).
  std::vector<std::string> gnuplot_commands;
};

/** @brief Supported series drawing types. */
enum class SeriesType { Line, Scatter, ErrorBars, Band, Histogram, Heatmap };

/** @brief Series metadata and optional per-series style override. */
struct SeriesSpec {
  SeriesType type = SeriesType::Line;
  std::string label;
  bool has_line_width = false;
  double line_width_pt = 1.0;
  /** @brief Optional explicit line/point color (e.g., "#112233"). */
  bool has_color = false;
  std::string color = "#000000";
  /**
   * @brief Optional opacity in [0, 1].
   * @note Backends map this to gnuplot ARGB color strings when possible.
   */
  bool has_opacity = false;
  double opacity = 1.0;
  bool use_y2 = false;
};

/** @brief In-memory series samples used by backends for emission/rendering. */
struct SeriesData {
  SeriesSpec spec;
  std::vector<double> x;
  std::vector<double> y;
  std::vector<double> y2;
  std::vector<double> yerr_low;
  std::vector<double> yerr_high;
  std::vector<double> z;
};

/**
 * @brief One subplot in a figure layout.
 */
class Axes {
public:
  /**
   * @brief Set axes configuration.
   * @param spec Axes settings to store.
   */
  void set(AxesSpec spec);

  /**
   * @brief Add one x/y series to this axes.
   * @param spec Series metadata and styling.
   * @param x X samples.
   * @param y Y samples; must match x length.
   * @throws std::invalid_argument if x and y lengths differ.
   */
  void add_series(const SeriesSpec& spec,
                  std::span<const double> x,
                  std::span<const double> y);

  /**
   * @brief Add a confidence band from lower/upper curves.
   * @param spec Series style metadata.
   * @param x X samples.
   * @param y_low Lower bound values.
   * @param y_high Upper bound values.
   * @throws std::invalid_argument if lengths differ.
   */
  void add_band(const SeriesSpec& spec,
                std::span<const double> x,
                std::span<const double> y_low,
                std::span<const double> y_high);

  /**
   * @brief Add a histogram-style series from bin centers and counts.
   * @param spec Series metadata.
   * @param bin_centers Histogram bin centers.
   * @param counts Histogram values.
   * @throws std::invalid_argument if lengths differ.
   */
  void add_histogram(const SeriesSpec& spec,
                     std::span<const double> bin_centers,
                     std::span<const double> counts);

  /**
   * @brief Add heatmap samples as x/y/z triplets.
   * @param spec Series metadata.
   * @param x X coordinates.
   * @param y Y coordinates.
   * @param z Intensity values.
   * @throws std::invalid_argument if lengths differ.
   */
  void add_heatmap(const SeriesSpec& spec,
                   std::span<const double> x,
                   std::span<const double> y,
                   std::span<const double> z);

  /**
   * @brief Add asymmetric y-errorbar series.
   * @param spec Series metadata.
   * @param x X samples.
   * @param y Central values.
   * @param y_low Absolute lower errors.
   * @param y_high Absolute upper errors.
   */
  void add_errorbars_asymmetric(const SeriesSpec& spec,
                                std::span<const double> x,
                                std::span<const double> y,
                                std::span<const double> y_low,
                                std::span<const double> y_high);

  /** @return Current axes configuration. */
  const AxesSpec& spec() const { return spec_; }

  /** @return All series added to this axes. */
  const std::vector<SeriesData>& series() const { return series_; }

private:
  AxesSpec spec_{};
  std::vector<SeriesData> series_{};
};

class Figure;

/** @brief Backend render status and generated output paths. */
enum class RenderStatus {
  Success,
  InvalidInput,
  IoError,
  ExternalToolMissing,
  ExternalToolFailure,
  UnsupportedFormat
};

/** @brief Backend render status and generated output paths. */
struct RenderResult {
  bool ok = true;
  RenderStatus status = RenderStatus::Success;
  std::string message;
  std::filesystem::path script_path;
  std::vector<std::filesystem::path> outputs;
};

/**
 * @brief Renderer backend abstraction.
 */
class IPlotBackend {
public:
  virtual ~IPlotBackend() = default;

  /**
   * @brief Render a figure to output files.
   * @param fig Figure and series data.
   * @param out_dir Output directory for artifacts.
   * @return Render status and paths.
   */
  virtual RenderResult render(const Figure& fig,
                              const std::filesystem::path& out_dir) = 0;
};

/**
 * @brief Figure container with a fixed subplot grid and backend delegation.
 */
class Figure {
public:
  /**
   * @brief Construct a figure from specification.
   * @param spec Figure spec including layout.
   * @throws std::invalid_argument if rows or cols are not positive.
   */
  explicit Figure(FigureSpec spec);

  /**
   * @brief Access axes by row/column index.
   * @param r Zero-based row.
   * @param c Zero-based column.
   * @return Mutable axes reference.
   * @throws std::out_of_range when indices are invalid.
   */
  Axes& axes(int r, int c);

  /**
   * @brief Access axes by flattened index.
   * @param idx Zero-based index in row-major order.
   * @return Mutable axes reference.
   * @throws std::out_of_range when index is invalid.
   */
  Axes& axes(int idx);

  /** @return Figure specification. */
  const FigureSpec& spec() const { return spec_; }

  /** @return All axes in row-major order. */
  const std::vector<Axes>& all_axes() const { return axes_; }

  /**
   * @brief Save the figure by invoking the configured backend.
   * @param out_dir Output directory.
   * @return Render status and output paths.
   * @note Returns ok=false when no backend is set.
   */
  RenderResult save(const std::filesystem::path& out_dir) const;

  /**
   * @brief Attach a backend used by save().
   * @param backend Backend implementation.
   */
  void set_backend(std::shared_ptr<IPlotBackend> backend);

private:
  FigureSpec spec_{};
  std::vector<Axes> axes_{};
  std::shared_ptr<IPlotBackend> backend_{};
};

}  // namespace gnuplotpp
