#include "gnuplotpp/presets.hpp"

namespace gnuplotpp {

PresetDefaults preset_defaults(Preset preset) {
  PresetDefaults defaults;

  switch (preset) {
    case Preset::IEEE_SingleColumn:
      defaults.size = FigureSizeInches{.w = 3.5, .h = 2.5};
      defaults.style.font = "Times-New-Roman";
      defaults.style.font_pt = 8.5;
      defaults.style.line_width_pt = 1.3;
      defaults.style.point_size = 0.8;
      defaults.style.grid = false;
      break;
    case Preset::IEEE_DoubleColumn:
      defaults.size = FigureSizeInches{.w = 7.16, .h = 2.8};
      defaults.style.font = "Times-New-Roman";
      defaults.style.font_pt = 8.5;
      defaults.style.line_width_pt = 1.3;
      defaults.style.point_size = 0.8;
      defaults.style.grid = false;
      break;
    case Preset::AIAA_Column:
      defaults.size = FigureSizeInches{.w = 3.25, .h = 2.4};
      defaults.style.font = "Times-New-Roman";
      defaults.style.font_pt = 8.0;
      defaults.style.line_width_pt = 1.0;
      defaults.style.point_size = 0.6;
      defaults.style.grid = false;
      break;
    case Preset::AIAA_Page:
      defaults.size = FigureSizeInches{.w = 7.0, .h = 2.8};
      defaults.style.font = "Times-New-Roman";
      defaults.style.font_pt = 8.0;
      defaults.style.line_width_pt = 1.0;
      defaults.style.point_size = 0.6;
      defaults.style.grid = false;
      break;
    case Preset::Custom:
      defaults.size = FigureSizeInches{.w = 3.5, .h = 2.5};
      defaults.style = Style{};
      break;
  }

  return defaults;
}

void apply_preset_defaults(FigureSpec& spec,
                           bool overwrite_size,
                           bool overwrite_style) {
  const auto defaults = preset_defaults(spec.preset);

  if (overwrite_size) {
    spec.size = defaults.size;
  }
  if (overwrite_style) {
    spec.style = defaults.style;
  }

  if ((spec.preset == Preset::AIAA_Column || spec.preset == Preset::AIAA_Page) &&
      spec.style.font_pt < 8.0) {
    spec.style.font_pt = 8.0;
  }
}

}  // namespace gnuplotpp
