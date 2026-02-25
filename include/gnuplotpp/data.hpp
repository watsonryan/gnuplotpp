#pragma once

#include "gnuplotpp/plot.hpp"

#include <filesystem>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace gnuplotpp {

/**
 * @brief Simple numeric table loaded from CSV.
 */
struct DataTable {
  std::unordered_map<std::string, std::vector<double>> columns;

  /** @brief Access a column by name; throws when missing. */
  const std::vector<double>& column(const std::string& name) const;

  /** @brief Returns true when a named column exists. */
  bool has_column(const std::string& name) const;

  /**
   * @brief Validate that all requested columns exist.
   * @param names Required column names.
   * @throws std::out_of_range with available column list when missing.
   */
  void require_columns(std::span<const std::string> names) const;

  /** @brief Returns row count; throws when columns have inconsistent lengths. */
  std::size_t row_count() const;

  /**
   * @brief Add a line series from named columns.
   * @param ax Target axes.
   * @param spec Series style.
   * @param x_name X column.
   * @param y_name Y column.
   */
  void add_line(Axes& ax,
                SeriesSpec spec,
                const std::string& x_name,
                const std::string& y_name) const;

  /**
   * @brief Add a scatter series from named columns.
   * @param ax Target axes.
   * @param spec Series style.
   * @param x_name X column.
   * @param y_name Y column.
   */
  void add_scatter(Axes& ax,
                   SeriesSpec spec,
                   const std::string& x_name,
                   const std::string& y_name) const;
};

/**
 * @brief Load numeric CSV with header row.
 * @param path File path.
 * @param delimiter Field delimiter.
 * @return Parsed table.
 */
DataTable read_csv_numeric(const std::filesystem::path& path, char delimiter = ',');

/**
 * @brief Build axis label with unit suffix.
 * @param name Quantity name.
 * @param unit Unit text.
 * @return Label in "name [unit]" form, or just name when unit empty.
 */
std::string label_with_unit(const std::string& name, const std::string& unit);

}  // namespace gnuplotpp
