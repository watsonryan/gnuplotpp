#pragma once

#include "gnuplotpp/plot.hpp"

#include <filesystem>
#include <iosfwd>
#include <string>
#include <vector>

namespace gnuplotpp::detail {

std::string extension_for(OutputFormat format);
const char* format_name(OutputFormat format);
bool format_supports_line_alpha(OutputFormat format);
std::string terminal_for(OutputFormat format, const FigureSpec& spec);
bool has_series_opacity(const Figure& fig);

void emit_plot_body(std::ostream& os,
                    const Figure& fig,
                    const std::vector<std::vector<std::filesystem::path>>& data_files);

}  // namespace gnuplotpp::detail
