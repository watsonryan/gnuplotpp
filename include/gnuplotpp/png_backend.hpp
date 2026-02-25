#pragma once

#include "gnuplotpp/plot.hpp"

#include <memory>

namespace gnuplotpp {

/**
 * @brief Native C++ PNG renderer backend.
 */
class PngBackend final : public IPlotBackend {
public:
  /**
   * @brief Render figure content directly to PNG output.
   * @param fig Figure to render.
   * @param out_dir Output directory.
   * @return Render status and generated paths.
   */
  RenderResult render(const Figure& fig,
                      const std::filesystem::path& out_dir) override;
};

/** @brief Factory helper for native PNG backend. */
std::shared_ptr<IPlotBackend> make_png_backend();

}  // namespace gnuplotpp
