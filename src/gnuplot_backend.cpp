#include "gnuplotpp/gnuplot_backend.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

namespace gnuplotpp {
namespace {

std::string quote(const std::filesystem::path& p) {
  return "'" + p.string() + "'";
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

std::string terminal_for(OutputFormat format, const FigureSpec& spec) {
  std::ostringstream os;
  os << std::fixed << std::setprecision(3);
  switch (format) {
    case OutputFormat::Pdf:
      os << "set terminal pdfcairo size " << spec.size.w << "in," << spec.size.h
         << "in font '" << spec.style.font << "," << spec.style.font_pt << "'";
      return os.str();
    case OutputFormat::Svg:
      os << "set terminal svg size " << (spec.size.w * 96.0) << "," << (spec.size.h * 96.0)
         << " font '" << spec.style.font << "," << spec.style.font_pt << "' dynamic";
      return os.str();
    case OutputFormat::Eps:
      os << "set terminal epscairo size " << spec.size.w << "in," << spec.size.h
         << "in font '" << spec.style.font << "," << spec.style.font_pt << "'";
      return os.str();
    case OutputFormat::Png:
      os << "set terminal pngcairo size " << (spec.size.w * 200.0) << ","
         << (spec.size.h * 200.0) << " font '" << spec.style.font << "," << spec.style.font_pt
         << "'";
      return os.str();
  }
  return {};
}

std::string with_clause(const SeriesData& series, const Style& style) {
  const double line_width =
      series.spec.has_line_width ? series.spec.line_width_pt : style.line_width_pt;

  std::ostringstream os;
  os << std::fixed << std::setprecision(3);
  switch (series.spec.type) {
    case SeriesType::Line:
      os << "with lines lw " << line_width;
      break;
    case SeriesType::Scatter:
      os << "with points pt 7 ps " << style.point_size;
      break;
    case SeriesType::ErrorBars:
      os << "with yerrorbars lw " << line_width;
      break;
    case SeriesType::Band:
      os << "with lines lw " << line_width;
      break;
  }
  return os.str();
}

void emit_plot_body(std::ostream& os,
                    const Figure& fig,
                    const std::vector<std::vector<std::filesystem::path>>& data_files) {
  const auto& spec = fig.spec();
  const auto& all_axes = fig.all_axes();

  os << "unset key\n";
  os << "set tics out\n";
  os << "set multiplot layout " << spec.rows << "," << spec.cols;
  if (!spec.title.empty()) {
    os << " title '" << spec.title << "'";
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

    if (axis_spec.legend) {
      os << "set key top right\n";
    } else {
      os << "unset key\n";
    }

    if (!axis_spec.title.empty()) {
      os << "set title '" << axis_spec.title << "'\n";
    }
    if (!axis_spec.xlabel.empty()) {
      os << "set xlabel '" << axis_spec.xlabel << "'\n";
    }
    if (!axis_spec.ylabel.empty()) {
      os << "set ylabel '" << axis_spec.ylabel << "'\n";
    }

    if (axis_spec.grid || spec.style.grid) {
      os << "set grid\n";
    }
    if (axis_spec.xlog) {
      os << "set logscale x\n";
    }
    if (axis_spec.ylog) {
      os << "set logscale y\n";
    }

    if (axis_spec.has_xlim) {
      os << "set xrange [" << axis_spec.xmin << ":" << axis_spec.xmax << "]\n";
    }
    if (axis_spec.has_ylim) {
      os << "set yrange [" << axis_spec.ymin << ":" << axis_spec.ymax << "]\n";
    }

    if (axis.series().empty()) {
      os << "plot 1/0 notitle\n";
      continue;
    }

    os << "plot ";
    for (std::size_t s = 0; s < axis.series().size(); ++s) {
      const auto& series = axis.series()[s];
      os << quote(data_files[axis_idx][s]) << " using 1:2 " << with_clause(series, spec.style)
         << " title '" << series.spec.label << "'";
      if (s + 1 < axis.series().size()) {
        os << ", \\\n";
      } else {
        os << "\n";
      }
    }
  }

  os << "unset multiplot\n";
}

}  // namespace

GnuplotBackend::GnuplotBackend(std::string executable)
    : executable_(std::move(executable)) {}

RenderResult GnuplotBackend::render(const Figure& fig,
                                    const std::filesystem::path& out_dir) {
  RenderResult result;

  std::error_code ec;
  std::filesystem::create_directories(out_dir / "tmp", ec);
  if (ec) {
    result.ok = false;
    result.message = "failed to create output directories: " + ec.message();
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
      data_os << std::scientific << std::setprecision(16);
      for (std::size_t i = 0; i < series.x.size(); ++i) {
        data_os << series.x[i] << " " << series.y[i] << "\n";
      }
    }
  }

  const auto script_path = out_dir / "tmp" / "figure.gp";
  result.script_path = script_path;

  std::ofstream script_os(script_path);
  script_os << "set encoding utf8\n";

  for (const auto format : fig.spec().formats) {
    const auto output_path = out_dir / ("figure." + extension_for(format));
    result.outputs.push_back(output_path);

    script_os << terminal_for(format, fig.spec()) << "\n";
    script_os << "set output " << quote(output_path) << "\n";
    emit_plot_body(script_os, fig, data_files);
    script_os << "set output\n";
  }

  const std::string check_cmd = "command -v " + executable_ + " >/dev/null 2>&1";
  if (std::system(check_cmd.c_str()) != 0) {
    result.ok = false;
    result.message = "gnuplot executable not found; generated script/data only";
    return result;
  }

  const std::string render_cmd = executable_ + " " + script_path.string() + " >/tmp/gnuplotpp.log 2>&1";
  if (std::system(render_cmd.c_str()) != 0) {
    result.ok = false;
    result.message = "gnuplot failed; inspect /tmp/gnuplotpp.log";
    return result;
  }

  result.ok = true;
  result.message = "render completed";
  return result;
}

std::shared_ptr<IPlotBackend> make_gnuplot_backend(std::string executable) {
  return std::make_shared<GnuplotBackend>(std::move(executable));
}

}  // namespace gnuplotpp
