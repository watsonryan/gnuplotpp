#include "gnuplotpp/data.hpp"
#include "gnuplotpp/gnuplot_backend.hpp"
#include "gnuplotpp/logging.hpp"
#include "gnuplotpp/spec_yaml.hpp"

#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

void print_usage() {
  gnuplotpp::log::Info("Usage: gnuplotpp_cli --spec <figure.yaml> --out <out_dir>");
  gnuplotpp::log::Info(
      "YAML supports axes[].series[] with CSV bindings (recommended) and inline arrays.");
}

std::string as_string(const YAML::Node& n, const char* ctx) {
  if (!n || !n.IsScalar()) {
    throw std::runtime_error(std::string("expected scalar for ") + ctx);
  }
  return n.as<std::string>();
}

std::vector<double> as_double_vec(const YAML::Node& n, const char* ctx) {
  if (!n || !n.IsSequence()) {
    throw std::runtime_error(std::string("expected sequence for ") + ctx);
  }
  std::vector<double> out;
  out.reserve(n.size());
  for (const auto& e : n) {
    out.push_back(e.as<double>());
  }
  return out;
}

const std::vector<double>& csv_column(std::unordered_map<std::string, gnuplotpp::DataTable>& cache,
                                      const std::filesystem::path& spec_dir,
                                      const YAML::Node& csv_node,
                                      const char* key,
                                      std::vector<double>& scratch) {
  if (!csv_node || !csv_node.IsMap()) {
    throw std::runtime_error("missing csv: block for column-backed series field");
  }
  const auto rel = as_string(csv_node["path"], "csv.path");
  const auto path = (spec_dir / rel).lexically_normal();
  const auto path_key = path.string();
  auto it = cache.find(path_key);
  if (it == cache.end()) {
    it = cache.emplace(path_key, gnuplotpp::read_csv_numeric(path)).first;
  }
  const auto col_name = as_string(csv_node[key], key);
  scratch = it->second.column(col_name);
  return scratch;
}

gnuplotpp::SeriesType parse_series_type(const YAML::Node& node) {
  if (!node["type"]) {
    return gnuplotpp::SeriesType::Line;
  }
  const auto t = node["type"].as<std::string>();
  if (t == "line") return gnuplotpp::SeriesType::Line;
  if (t == "scatter") return gnuplotpp::SeriesType::Scatter;
  if (t == "band") return gnuplotpp::SeriesType::Band;
  if (t == "histogram") return gnuplotpp::SeriesType::Histogram;
  if (t == "heatmap") return gnuplotpp::SeriesType::Heatmap;
  if (t == "errorbars") return gnuplotpp::SeriesType::ErrorBars;
  throw std::runtime_error("unsupported series type: " + t);
}

gnuplotpp::SeriesSpec parse_series_spec(const YAML::Node& node) {
  gnuplotpp::SeriesSpec s;
  s.type = parse_series_type(node);
  if (node["label"]) s.label = node["label"].as<std::string>();
  if (node["use_y2"]) s.use_y2 = node["use_y2"].as<bool>();
  if (node["line_width_pt"]) {
    s.has_line_width = true;
    s.line_width_pt = node["line_width_pt"].as<double>();
  }
  if (node["color"]) {
    s.has_color = true;
    s.color = node["color"].as<std::string>();
  }
  if (node["opacity"]) {
    s.has_opacity = true;
    s.opacity = node["opacity"].as<double>();
  }
  return s;
}

std::vector<double> series_values(const YAML::Node& series_node,
                                  const std::filesystem::path& spec_dir,
                                  std::unordered_map<std::string, gnuplotpp::DataTable>& csv_cache,
                                  const char* key) {
  if (series_node[key]) {
    return as_double_vec(series_node[key], key);
  }
  std::vector<double> scratch;
  const auto& c = csv_column(csv_cache, spec_dir, series_node["csv"], key, scratch);
  return std::vector<double>(c.begin(), c.end());
}

void require_same_size(const std::vector<double>& a,
                       const std::vector<double>& b,
                       const char* an,
                       const char* bn) {
  if (a.size() != b.size()) {
    throw std::runtime_error(std::string("size mismatch: ") + an + " vs " + bn);
  }
}

void add_series_from_yaml(gnuplotpp::Axes& ax,
                          const YAML::Node& series_node,
                          const std::filesystem::path& spec_dir,
                          std::unordered_map<std::string, gnuplotpp::DataTable>& csv_cache) {
  auto spec = parse_series_spec(series_node);
  const auto type = spec.type;

  if (type == gnuplotpp::SeriesType::Line || type == gnuplotpp::SeriesType::Scatter) {
    auto x = series_values(series_node, spec_dir, csv_cache, "x");
    auto y = series_values(series_node, spec_dir, csv_cache, "y");
    require_same_size(x, y, "x", "y");
    ax.add_series(spec, x, y);
    return;
  }
  if (type == gnuplotpp::SeriesType::Band) {
    auto x = series_values(series_node, spec_dir, csv_cache, "x");
    auto y_low = series_values(series_node, spec_dir, csv_cache, "y_low");
    auto y_high = series_values(series_node, spec_dir, csv_cache, "y_high");
    require_same_size(x, y_low, "x", "y_low");
    require_same_size(x, y_high, "x", "y_high");
    ax.add_band(spec, x, y_low, y_high);
    return;
  }
  if (type == gnuplotpp::SeriesType::Histogram) {
    auto x = series_values(series_node, spec_dir, csv_cache, "x");
    auto y = series_values(series_node, spec_dir, csv_cache, "y");
    require_same_size(x, y, "x", "y");
    ax.add_histogram(spec, x, y);
    return;
  }
  if (type == gnuplotpp::SeriesType::Heatmap) {
    auto x = series_values(series_node, spec_dir, csv_cache, "x");
    auto y = series_values(series_node, spec_dir, csv_cache, "y");
    auto z = series_values(series_node, spec_dir, csv_cache, "z");
    require_same_size(x, y, "x", "y");
    require_same_size(x, z, "x", "z");
    ax.add_heatmap(spec, x, y, z);
    return;
  }
  if (type == gnuplotpp::SeriesType::ErrorBars) {
    auto x = series_values(series_node, spec_dir, csv_cache, "x");
    auto y = series_values(series_node, spec_dir, csv_cache, "y");
    auto y_low = series_values(series_node, spec_dir, csv_cache, "y_low");
    auto y_high = series_values(series_node, spec_dir, csv_cache, "y_high");
    require_same_size(x, y, "x", "y");
    require_same_size(x, y_low, "x", "y_low");
    require_same_size(x, y_high, "x", "y_high");
    ax.add_errorbars_asymmetric(spec, x, y, y_low, y_high);
    return;
  }
  throw std::runtime_error("unsupported series type");
}

}  // namespace

int main(int argc, char** argv) {
  std::filesystem::path spec_path;
  std::filesystem::path out_dir;
  for (int i = 1; i < argc; ++i) {
    const std::string a = argv[i];
    if (a == "--spec" && i + 1 < argc) {
      spec_path = argv[++i];
    } else if (a == "--out" && i + 1 < argc) {
      out_dir = argv[++i];
    } else if (a == "-h" || a == "--help") {
      print_usage();
      return 0;
    }
  }
  if (spec_path.empty() || out_dir.empty()) {
    print_usage();
    return 1;
  }

  try {
    const auto ys = gnuplotpp::load_yaml_figure_spec(
        spec_path, gnuplotpp::YamlLoadOptions{.strict_unknown_keys = false});
    gnuplotpp::Figure fig(ys.figure);
    for (std::size_t i = 0; i < ys.axes.size() && i < fig.all_axes().size(); ++i) {
      fig.axes(static_cast<int>(i)).set(ys.axes[i]);
    }

    const auto root = YAML::LoadFile(spec_path.string());
    const auto axes_node = root["axes"];
    std::unordered_map<std::string, gnuplotpp::DataTable> csv_cache;
    bool has_series = false;
    if (axes_node && axes_node.IsSequence()) {
      const auto spec_dir = spec_path.parent_path();
      for (std::size_t ai = 0; ai < axes_node.size() && ai < fig.all_axes().size(); ++ai) {
        const auto axis_yaml = axes_node[ai];
        const auto series_yaml = axis_yaml["series"];
        if (!series_yaml) {
          continue;
        }
        if (!series_yaml.IsSequence()) {
          throw std::runtime_error("axes[].series must be a sequence");
        }
        for (const auto& s : series_yaml) {
          add_series_from_yaml(fig.axes(static_cast<int>(ai)), s, spec_dir, csv_cache);
          has_series = true;
        }
      }
    }

    if (!has_series) {
      gnuplotpp::log::Error("cli spec has no series in axes[].series[]");
      return 4;
    }

    fig.set_backend(gnuplotpp::make_gnuplot_backend());
    const auto rr = fig.save(out_dir);
    if (!rr.ok) {
      gnuplotpp::log::Error(rr.message);
      return 2;
    }
    gnuplotpp::log::Info("rendered from CLI: ", out_dir.string());
  } catch (const std::exception& e) {
    gnuplotpp::log::Error("cli failure: ", e.what());
    return 3;
  }
  return 0;
}
