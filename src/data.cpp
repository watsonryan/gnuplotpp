#include "gnuplotpp/data.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace gnuplotpp {

const std::vector<double>& DataTable::column(const std::string& name) const {
  const auto it = columns.find(name);
  if (it == columns.end()) {
    throw std::out_of_range("column not found: " + name);
  }
  return it->second;
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
  while (std::getline(in, line)) {
    if (line.empty()) {
      continue;
    }
    std::stringstream ss(line);
    std::string token;
    std::size_t i = 0;
    while (std::getline(ss, token, delimiter) && i < names.size()) {
      tbl.columns[names[i++]].push_back(std::stod(token));
    }
  }
  return tbl;
}

std::string label_with_unit(const std::string& name, const std::string& unit) {
  if (unit.empty()) {
    return name;
  }
  return name + " [" + unit + "]";
}

}  // namespace gnuplotpp
