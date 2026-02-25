#pragma once

#include "gnuplotpp/plot.hpp"

namespace gnuplotpp {

/** @brief High-level style bundles for common publication/presentation looks. */
enum class StyleProfile {
  Science,
  IEEE_Strict,
  AIAA_Strict,
  Presentation,
  DarkPrintSafe,
  Tufte_Minimal
};

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

/**
 * @brief Apply a style profile on top of current figure style/palette settings.
 * @param spec Figure spec to update.
 * @param profile Style profile selector.
 */
void apply_style_profile(FigureSpec& spec, StyleProfile profile);

}  // namespace gnuplotpp
