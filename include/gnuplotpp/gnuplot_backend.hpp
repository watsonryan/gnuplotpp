#pragma once

#include "gnuplotpp/plot.hpp"

#include <memory>
#include <string>

namespace gnuplotpp {

/**
 * @brief Gnuplot-backed renderer that emits .dat/.gp artifacts and runs gnuplot.
 */
class GnuplotBackend final : public IPlotBackend {
public:
  /**
   * @brief Construct backend with a gnuplot executable name/path.
   * @param executable Command used to invoke gnuplot.
   */
  explicit GnuplotBackend(std::string executable = "gnuplot");

  /**
   * @brief Render figure to requested output formats.
   * @param fig Figure to render.
   * @param out_dir Destination directory.
   * @return Render status and generated output paths.
   */
  RenderResult render(const Figure& fig,
                      const std::filesystem::path& out_dir) override;

private:
  std::string executable_;
};

/**
 * @brief Factory helper for backend creation.
 * @param executable Command used to invoke gnuplot.
 * @return Shared backend instance.
 */
std::shared_ptr<IPlotBackend> make_gnuplot_backend(std::string executable = "gnuplot");

}  // namespace gnuplotpp
