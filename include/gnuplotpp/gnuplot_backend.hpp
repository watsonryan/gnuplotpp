#pragma once

#include "gnuplotpp/plot.hpp"

#include <memory>
#include <string>

namespace gnuplotpp {

class GnuplotBackend final : public IPlotBackend {
public:
  explicit GnuplotBackend(std::string executable = "gnuplot");

  RenderResult render(const Figure& fig,
                      const std::filesystem::path& out_dir) override;

private:
  std::string executable_;
};

std::shared_ptr<IPlotBackend> make_gnuplot_backend(std::string executable = "gnuplot");

}  // namespace gnuplotpp
