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

bool validate_environment_block_json_to_json(const json& input,
                                             json& output,
                                             std::vector<std::string>& errors,
                                             bool debug = false);
json validate_environment_block_json_to_json(const json& input,
                                             bool debug = false);

bool validate_method_block_json_to_json(const json& input,
                                        json& output,
                                        std::vector<std::string>& errors,
                                        bool debug = false);
json validate_method_block_json_to_json(const json& input,
                                        bool debug = false);

bool validate_model_block_json_to_json(const json& input,
                                       json& output,
                                       std::vector<std::string>& errors,
                                       bool debug = false);
json validate_model_block_json_to_json(const json& input,
                                       bool debug = false);

bool validate_variables_block_json_to_json(const json& input,
                                           json& output,
                                           std::vector<std::string>& errors,
                                           bool debug = false);
json validate_variables_block_json_to_json(const json& input,
                                           bool debug = false);

bool validate_interface_block_json_to_json(const json& input,
                                           json& output,
                                           std::vector<std::string>& errors,
                                           bool debug = false);
json validate_interface_block_json_to_json(const json& input,
                                           bool debug = false);

bool validate_responses_block_json_to_json(const json& input,
                                           json& output,
                                           std::vector<std::string>& errors,
                                           bool debug = false);
json validate_responses_block_json_to_json(const json& input,
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
