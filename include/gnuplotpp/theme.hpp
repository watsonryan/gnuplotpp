#pragma once

#include "gnuplotpp/plot.hpp"

#include <filesystem>

namespace gnuplotpp {

/** @brief Versioned built-in theme identifiers. */
enum class ThemePreset { IEEE_Strict_v1, Science_v1, Tufte_Minimal_v1 };

/**
 * @brief Save a reusable theme snapshot to a JSON file.
 * @param path Output JSON path.
 * @param spec Figure spec carrying style/palette/text settings.
 * @return true on success.
 */
bool save_theme_json(const std::filesystem::path& path, const FigureSpec& spec);

/**
 * @brief Load a theme snapshot from a JSON file.
 * @param path Input JSON path.
 * @param spec Figure spec updated in-place.
 * @return true on success.
 */
bool load_theme_json(const std::filesystem::path& path, FigureSpec& spec);

/**
 * @brief Apply a versioned built-in theme preset.
 * @param spec Figure spec to update.
 * @param preset Versioned theme preset.
 */
void apply_theme_preset(FigureSpec& spec, ThemePreset preset);

/**
 * @brief Return stable string id for a versioned theme preset.
 */
const char* theme_preset_id(ThemePreset preset) noexcept;

}  // namespace gnuplotpp
