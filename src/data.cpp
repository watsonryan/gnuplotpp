#include "gnuplotpp/data.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace gnuplotpp {

namespace {

std::string available_columns(const std::unordered_map<std::string, std::vector<double>>& cols) {
  std::vector<std::string> names;
  names.reserve(cols.size());
  for (const auto& [name, _] : cols) {
    names.push_back(name);
  }
  std::sort(names.begin(), names.end());
  std::string out;
  for (std::size_t i = 0; i < names.size(); ++i) {
    out += names[i];
    if (i + 1 < names.size()) {
      out += ", ";
    }
  }
  return out;
}

}  // namespace

const std::vector<double>& DataTable::column(const std::string& name) const {
  const auto it = columns.find(name);
  if (it == columns.end()) {
    throw std::out_of_range("column not found: " + name + " (available: " +
                            available_columns(columns) + ")");
  }
  return it->second;
}

bool DataTable::has_column(const std::string& name) const {
  return columns.find(name) != columns.end();
}

void DataTable::require_columns(std::span<const std::string> names) const {
  for (const auto& name : names) {
    if (!has_column(name)) {
      throw std::out_of_range("required column missing: " + name + " (available: " +
                              available_columns(columns) + ")");
    }
  }
}

std::size_t DataTable::row_count() const {
  if (columns.empty()) {
    return 0;
  }
  const auto first = columns.begin()->second.size();
  for (const auto& [name, values] : columns) {
    if (values.size() != first) {
      throw std::runtime_error("inconsistent row count in column: " + name);
    }
  }
  return first;
}

void DataTable::add_line(Axes& ax,
                         SeriesSpec spec,
                         const std::string& x_name,
                         const std::string& y_name) const {
  spec.type = SeriesType::Line;
  ax.add_series(spec, column(x_name), column(y_name));
}

void DataTable::add_scatter(Axes& ax,
                            SeriesSpec spec,
                            const std::string& x_name,
                            const std::string& y_name) const {
  spec.type = SeriesType::Scatter;
  ax.add_series(spec, column(x_name), column(y_name));
}

DataTable read_csv_numeric(const std::filesystem::path& path, char delimiter) {
  std::ifstream in(path);
  if (!in.is_open()) {
    throw std::runtime_error("failed to open CSV: " + path.string());
  }

  std::string header;
  if (!std::getline(in, header)) {
    throw std::runtime_error("CSV is empty: " + path.string());
  }

  std::vector<std::string> names;
  {
    std::stringstream ss(header);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
      names.push_back(token);
    }
  }
  if (names.empty()) {
    throw std::runtime_error("CSV header parse failed: " + path.string());
  }

  DataTable tbl;
  for (const auto& name : names) {
    tbl.columns[name] = {};
  }

  std::string line;
  std::size_t line_no = 1;
  while (std::getline(in, line)) {
    ++line_no;
    if (line.empty()) {
      continue;
    }
    std::stringstream ss(line);
    std::string token;
    std::size_t i = 0;
    while (std::getline(ss, token, delimiter) && i < names.size()) {
      tbl.columns[names[i++]].push_back(std::stod(token));
    }
    if (i != names.size()) {
      throw std::runtime_error("CSV row has wrong number of fields at line " +
                               std::to_string(line_no));
    }
  }
  (void)tbl.row_count();
  return tbl;
}

std::string label_with_unit(const std::string& name, const std::string& unit) {
  if (unit.empty()) {
    return name;
  }
  return name + " [" + unit + "]";
}

}  // namespace gnuplotpp
