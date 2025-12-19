/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "JSONProblemDescDB.hpp"

namespace {

  json
    load_json_from_file(const std::string& filename)
  {
    std::ifstream file(filename);
    if (!file.is_open()) {
      throw std::runtime_error("Could not open the file: " + filename);
    }

    json j;
    file >> j;
    return j;
  }
}

namespace Dakota {

JSONProblemDescDB::JSONProblemDescDB(const std::string& filename)
{

  json key_map_obj;
  try {
    // Load JSON objects from files
    jsonOptions = load_json_from_file(filename);
    key_map_obj = load_json_from_file("key_mapping_v2.json");
    // Output the loaded JSON objects
    //std::cout << "JSON Input: "   << jsonOptions.dump(4) << std::endl;
    //std::cout << "JSON Key Map: " << key_map_obj.dump(4) << std::endl;
  } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
  }

  // Parse the Dakota json input and populate data chace maps
  std::function<void(const json&, const std::string&)>
    parse_json = [&](const json& j, const std::string& path = "")
  {
    for (auto& [key, value] : j.items()) {
      std::string currentPath = path.empty() ? key : path + "/" + key;

      // Check if the value is a JSON object
      if (value.is_object()) {
        parse_json(value, currentPath); // Recursive call for nested objects
      }
      // Check if the value is a JSON array
      else if (value.is_array()) {
        // Print the array key without index
        std::cout << currentPath << ": [Array of size " << value.size() << "]" << std::endl;
        // Recurse to iterate through the array contents
        for (size_t i = 0; i < value.size(); ++i) {
          parse_json(value[i], currentPath);
        }
      }
      // Otherwise, it's a simple key-value pair
      else {
        std::cout << currentPath << ": " << value << std::endl;
        if( key_map_obj["xml_keywords_by_path"].contains(currentPath) ) {
          if( key_map_obj["xml_keywords_by_path"][currentPath].contains("pdb_key") ) {
            std::cout << currentPath << " --> " 
                       << "(" << key_map_obj["xml_keywords_by_path"][currentPath]["handler_type"] << ") "
                       << key_map_obj["xml_keywords_by_path"][currentPath]["pdb_key"]
                       << std::endl;
            if( "DIRECT_VALUE" == key_map_obj["xml_keywords_by_path"][currentPath]["storage_type"] ) {
              const std::string& type = key_map_obj["xml_keywords_by_path"][currentPath]["handler_type"];
              if( type == "int" )
                cachedData_int[key_map_obj["xml_keywords_by_path"][currentPath]["pdb_key"]] = value;
              else if( type == "sizet" )
                cachedData_size_t[key_map_obj["xml_keywords_by_path"][currentPath]["pdb_key"]] = value;
              else
                std::cout << "Need to implement data cavhing for type "
                          << key_map_obj["xml_keywords_by_path"][currentPath]["handler_type"]
                          << std::endl;
                     }
          }
          else
            std::cout << currentPath << " --> " << "No pdb_key" << std::endl;
        }
      }
    }
  };

  parse_json(jsonOptions, "");
  std::cout << "\n\n" << std::endl;
}

} // namespace Dakota

