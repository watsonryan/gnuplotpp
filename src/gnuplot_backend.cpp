#include "gnuplotpp/gnuplot_backend.hpp"
#include "gnuplotpp/logging.hpp"

#include "gnuplot_backend_internal.hpp"

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

std::string shell_quote(const std::filesystem::path& p) {
#ifdef _WIN32
  return "\"" + p.string() + "\"";
#else
  return "'" + p.string() + "'";
#endif
}

std::string gnuplot_quote(const std::filesystem::path& p) { return "'" + p.string() + "'"; }

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
        if (series.spec.type == SeriesType::ErrorBars && i < series.yerr_low.size() &&
            i < series.yerr_high.size()) {
          data_os << " " << series.yerr_low[i] << " " << series.yerr_high[i];
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
  const bool any_series_opacity = detail::has_series_opacity(fig);

  for (const auto format : fig.spec().formats) {
    if (any_series_opacity && !detail::format_supports_line_alpha(format) &&
        fig.spec().export_policy.warn_line_alpha_on_vector) {
      gnuplotpp::log::Warn("line opacity requested but ", detail::format_name(format),
                           " terminal may ignore alpha; prefer PNG for true line transparency");
    }
    if (fig.spec().text_mode == TextMode::LaTeX && format == OutputFormat::Png) {
      gnuplotpp::log::Warn("LaTeX text mode requested; PNG output falls back to enhanced text");
    }

    const auto output_path = out_dir / ("figure." + detail::extension_for(format));
    result.outputs.push_back(output_path);

    script_os << detail::terminal_for(format, fig.spec()) << "\n";
    script_os << "set output " << gnuplot_quote(output_path) << "\n";
    detail::emit_plot_body(script_os, fig, data_files, format);
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

  if (fig.spec().interactive_preview) {
    const auto preview_path = out_dir / "tmp" / "interactive_preview.gp";
    std::ofstream preview_os(preview_path);
    if (preview_os.is_open()) {
      preview_os << "set encoding utf8\n";
      preview_os << "set terminal qt persist\n";
      detail::emit_plot_body(preview_os, fig, data_files, OutputFormat::Png);
      preview_os << "pause mouse close\n";
    } else {
      gnuplotpp::log::Warn("failed to write interactive preview script: ", preview_path.string());
    }
  }

  const std::string check_cmd =
#ifdef _WIN32
      "where " + executable_ + " >NUL 2>&1";
#else
      "command -v " + executable_ + " >/dev/null 2>&1";
#endif
  if (std::system(check_cmd.c_str()) != 0) {
    result.ok = false;
    result.status = RenderStatus::ExternalToolMissing;
    result.message =
        msg_text("gnuplot executable not found; generated script/data only", executable_);
    gnuplotpp::log::Error(result.message);
    return result;
  }

  const auto log_path = out_dir / "tmp" / "gnuplot.log";
  const std::string render_cmd =
      shell_quote(std::filesystem::path(executable_)) + " " + shell_quote(script_path) + " >" +
      shell_quote(log_path) +
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
