#pragma once

#include "gnuplotpp/plot.hpp"

#include <memory>

namespace gnuplotpp {

/**
 * @brief Native C++ SVG renderer backend (no external plotting command).
 */
class SvgBackend final : public IPlotBackend {
public:
  /**
   * @brief Render figure content directly to SVG output.
   * @param fig Figure to render.
   * @param out_dir Output directory.
   * @return Render status and generated paths.
   */
  RenderResult render(const Figure& fig,
                      const std::filesystem::path& out_dir) override;
};

/** @brief Factory helper for native SVG backend. */
std::shared_ptr<IPlotBackend> make_svg_backend();

}  // namespace gnuplotpp
