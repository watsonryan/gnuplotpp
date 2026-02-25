#include "gnuplotpp/gnuplot_backend.hpp"
#include "gnuplotpp/logging.hpp"
#include "gnuplotpp/spec_yaml.hpp"

#include <filesystem>
#include <iostream>
#include <string>

namespace {

void print_usage() {
  std::cout << "Usage: gnuplotpp_cli --spec <figure.yaml> --out <out_dir>\n";
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
    const auto ys =
        gnuplotpp::load_yaml_figure_spec(spec_path, gnuplotpp::YamlLoadOptions{.strict_unknown_keys = true});
    gnuplotpp::Figure fig(ys.figure);
    for (std::size_t i = 0; i < ys.axes.size() && i < fig.all_axes().size(); ++i) {
      fig.axes(static_cast<int>(i)).set(ys.axes[i]);
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
