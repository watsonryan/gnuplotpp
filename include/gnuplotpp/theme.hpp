#pragma once

#include "gnuplotpp/plot.hpp"

#include <filesystem>

namespace gnuplotpp {

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

}  // namespace gnuplotpp
