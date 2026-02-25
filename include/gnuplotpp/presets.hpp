#pragma once

#include "gnuplotpp/plot.hpp"

namespace gnuplotpp {

struct PresetDefaults {
  FigureSizeInches size;
  Style style;
};

PresetDefaults preset_defaults(Preset preset);

void apply_preset_defaults(FigureSpec& spec,
                           bool overwrite_size = true,
                           bool overwrite_style = true);

}  // namespace gnuplotpp
