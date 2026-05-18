#ifndef DAKOTA_INPUT_READER_HPP
#define DAKOTA_INPUT_READER_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace dakota {

using json = nlohmann::json;

enum class InputFormat {
    Auto,
    DakotaFreeform,
    Json
};

bool read_freeform_input_file_to_json(const std::string& filename,
                                      json& output,
                                      std::vector<std::string>& errors,
                                      bool debug = false);
json read_freeform_input_file_to_json(const std::string& filename,
                                      bool debug = false);

bool read_freeform_input_string_to_json(const std::string& input,
                                        json& output,
                                        std::vector<std::string>& errors,
                                        bool debug = false);
json read_freeform_input_string_to_json(const std::string& input,
                                        bool debug = false);

bool read_json_input_file_to_json(const std::string& filename,
                                  json& output,
                                  std::vector<std::string>& errors,
                                  bool debug = false);
json read_json_input_file_to_json(const std::string& filename,
                                  bool debug = false);

bool read_json_input_string_to_json(const std::string& input,
                                    json& output,
                                    std::vector<std::string>& errors,
                                    bool debug = false);
json read_json_input_string_to_json(const std::string& input,
                                    bool debug = false);

bool validate_json_input_to_json(const json& input,
                                 json& output,
                                 std::vector<std::string>& errors,
                                 bool debug = false);
json validate_json_input_to_json(const json& input,
                                 bool debug = false);

// Compatibility entry points that auto-detect `.json` for file input.
bool read_input_file_to_json(const std::string& filename,
                             json& output,
                             std::vector<std::string>& errors,
                             InputFormat format = InputFormat::Auto,
                             bool debug = false);
json read_input_file_to_json(const std::string& filename,
                             InputFormat format = InputFormat::Auto,
                             bool debug = false);

} // namespace dakota

#endif // DAKOTA_INPUT_READER_HPP
