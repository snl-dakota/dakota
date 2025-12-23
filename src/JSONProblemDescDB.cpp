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
    std::cout << "Dakota JSON Input: "   << jsonOptions.dump(4) << std::endl;
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
        auto const& key_map_item = key_map_obj["xml_keywords_by_path"];
        if( key_map_item.contains(currentPath) )
        {
          // Support multiple cache map keys
          // ... but might consider consolidating into only a single client key, cf
          // ... method.convergence_tolerance and the many aliases
          std::vector<String> cache_keys;
          if( key_map_item[currentPath].contains("pdb_key") )
            cache_keys.push_back(key_map_item[currentPath]["pdb_key"]);
          else if( key_map_item[currentPath].contains("pdb_keys") )
            for (auto const& k : key_map_item[currentPath]["pdb_keys"])
              cache_keys.push_back(k);

          if( cache_keys.size() > 1 )
            std::for_each(cache_keys.begin(), cache_keys.end(), [](const String& str) {
                std::cout << "   " << str << std::endl;
                });

          const std::string& storage_type = key_map_item[currentPath]["storage_type"];

          for( const auto& ckey : cache_keys )
          {
            std::cout << currentPath << " --> " 
                       << "(" << key_map_item[currentPath]["handler_type"] << ") "
                       << ckey
                       << std::endl;
            if( "DIRECT_VALUE" == storage_type ) {
              const std::string& type = key_map_item[currentPath]["handler_type"];
              if( type == "int" )
                cachedData_int[ckey] = value;
              else if( type == "sizet" )
                cachedData_size_t[ckey] = value;
              else if( type == "Real" )
                cachedData_Real[ckey] = value;
              else
                std::cout << "Need to implement data caching for type "
                          << type
                          << std::endl;
            }
            else if( "PRESENCE_LITERAL" == storage_type ) {
              const std::string& type = key_map_item[currentPath]["handler_type"];
              if( type == "lit" ) {
                std::cout << currentPath << " --> " 
                           << "(" << key_map_item[currentPath]["is_oneof"] << ") "
                           << std::endl;
                if( !key_map_item[currentPath]["oneof_anchor"].is_null() ) {
                  const std::string& anchor = key_map_item[currentPath]["oneof_anchor"];
                  std::cout << "   anchor: " << anchor << std::endl;
                }
                else
                {
                  const String& keyword = cachedData_String[key_map_item[currentPath]["stored_value"]];
                  cachedData_String[ckey] = keyword;
                }
              }
              else
                std::cout << "Need to implement data caching for type "
                          << type
                          << std::endl;
            }
            else if( "PRESENCE_BOOL" == storage_type ) {
              const std::string& type = key_map_item[currentPath]["handler_type"];
              assert( "true" == type );
              cachedData_bool[ckey] = true;
              }
          }
          if( cache_keys.empty() )
            std::cout << currentPath << " --> " << "No pdb_key" << std::endl;
        }
        else
          std::cout << currentPath << " --> " << "Not in xml_keywords_by_path file" << std::endl;
      }
    }
  };

  parse_json(jsonOptions, "");
  std::cout << "\n\n" << std::endl;
  //throw;//(std::system_error(ecode, "Dakota aborted"));
}

} // namespace Dakota

