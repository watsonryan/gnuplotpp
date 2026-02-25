#pragma once

#include <filesystem>
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
