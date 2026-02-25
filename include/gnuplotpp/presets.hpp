#pragma once

#include "gnuplotpp/plot.hpp"

namespace gnuplotpp {

/** @brief Resolved size/style defaults for one preset. */
struct PresetDefaults {
  FigureSizeInches size;
  Style style;
};

/**
 * @brief Return baseline defaults for a publication preset.
 * @param preset Preset selector.
 * @return Default figure size and style.
 */
PresetDefaults preset_defaults(Preset preset);

/**
 * @brief Apply preset defaults to a mutable FigureSpec.
 * @param spec Figure spec to update.
 * @param overwrite_size When true, replace spec.size with preset size.
 * @param overwrite_style When true, replace spec.style with preset style.
 */
void apply_preset_defaults(FigureSpec& spec,
                           bool overwrite_size = true,
                           bool overwrite_style = true);

}  // namespace gnuplotpp
