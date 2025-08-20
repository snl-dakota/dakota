#include <algorithm>
#include <cctype> // For std::isspace
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// Function to trim leading whitespace
std::string trim_left(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r\f\v"); // Common whitespace characters
    return (start == std::string::npos) ? "" : s.substr(start);
}

// Function to trim trailing whitespace
std::string trim_right(const std::string& s) {
    size_t end = s.find_last_not_of(" \t\n\r\f\v");
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

// Function to trim both leading and trailing whitespace
std::string trim(const std::string& s) {
    return trim_left(trim_right(s));
}

static bool starts_with(const std::string str, const std::string prefix) {
  return ((prefix.size() <= str.size()) &&
          std::equal(prefix.begin(), prefix.end(), str.begin()));
}

static std::vector<size_t> find_all_occurances(std::string str, char ch) {
  std::vector<size_t>
      positions; // holds all the positions that sub occurs within str

  size_t pos = str.find(ch, 0);
  while (pos != std::string::npos) {
    positions.push_back(pos);
    pos = str.find(ch, pos + 1);
  }

  return positions;
}

int main(int argc, char const *argv[]) {
  std::string dakota_base_dir = DAKOTA_SRC_DIR;
  dakota_base_dir = trim(dakota_base_dir);

  std::ifstream file(argv[1]);
  std::ofstream output("dakota_cmake_cache.h");
  output << "#include <string>" << std::endl;
  output << "#include <vector>" << std::endl;
  output << std::endl;
  output << "static std::vector<std::string> cmake_configuration={" << std::endl;
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      line = trim(line);
      // Remove all blank lines and comments
      if (!(starts_with(line, "//") || starts_with(line, "#") ||
            line.length() == 0)) {
        // Escape any strings in the file
        auto positions = find_all_occurances(line, '"');
        std::for_each(positions.rbegin(), positions.rend(),
                      [&](size_t &p) { line.insert(p, 1, '\\'); });

        // Remove entries marked internal
        {
          auto pos = line.find(":INTERNAL");
          if (pos != std::string::npos) {
            continue;
          }
        }
        // Remove empty entries, i.e., TPL_ENABLE_BoostLib:STRING=
        if (line.back() == '=') {
          continue;
        }

        // Remove any non-system level directories
        {
          fs::path dakota_path = dakota_base_dir;
          dakota_path = dakota_path.parent_path();
          auto pos = line.find(dakota_path.string());
          if (pos != std::string::npos) {
            line.replace(pos, dakota_path.string().length(), "${DAKOTA_DIR}");
          }
          output << "    \"" << line << "\"," << std::endl;
        }
      }
    }
  }
  file.close();
  output << "};";
  output.close();
  return 0;
}
