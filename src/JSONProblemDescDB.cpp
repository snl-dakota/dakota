/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_global_defs.hpp"
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
    auto const& key_map_item = key_map_obj["xml_keywords_by_path"];

    for (auto& [key, value] : j.items()) {
      std::string currentPath = path.empty() ? key : path + "/" + key;

      // Check if the value is a JSON object
      if (value.is_object()) {
        if( key_map_item.contains(currentPath) ) {
          std::cout << "Handling non-leaf keyword : " << currentPath << std::endl;
          handle_keyword(key_map_item, currentPath, value);
        }
        parse_json(value, currentPath); // Recursive call for nested objects
      }
      // Check if the value is a JSON array
      else if( value.is_array() && key_map_item.at(currentPath).at("handler_type") == "start" )
      {
        // Print the array key without index
        std::cout << currentPath << ": [Array of size " << value.size() << "]" << std::endl;
        // Recurse to iterate through the array contents
        for (size_t i = 0; i < value.size(); ++i) {
          parse_json(value[i], currentPath);
        }
      }
      // Otherwise, it's a simple key-value pair
      else {
        handle_keyword(key_map_item, currentPath, value);
      }
    }
  };

  parse_json(jsonOptions, "");
  std::cout << "\n\n" << std::endl;
  //throw;//(std::system_error(ecode, "Dakota aborted"));
}

void
JSONProblemDescDB::handle_keyword(const json& key_map_item, const std::string& currentPath, const json& value)
{
  // Remove trailing "/" 
  //if( currentPath.back() == '/' )
  //  currentPath.pop_back();
  std::cout << currentPath << ": " << value << std::endl;
  if( key_map_item.contains(currentPath) )
  {
    // Support multiple cache map keys
    // ... but might consider consolidating into only a single client key, cf
    // ... method.convergence_tolerance and the many aliases
    std::vector<Dakota::String> cache_keys;
    if( key_map_item[currentPath].contains("pdb_key") )
      cache_keys.push_back(key_map_item[currentPath]["pdb_key"]);
    else if( key_map_item[currentPath].contains("pdb_keys") )
      for (auto const& k : key_map_item[currentPath]["pdb_keys"])
        cache_keys.push_back(k);

    if( cache_keys.size() > 1 )
      std::for_each(cache_keys.begin(), cache_keys.end(), [](const Dakota::String& str) {
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
        if( type == "str" )
          cachedData_String[ckey] = value;
        else if( type == "int" )
          cachedData_int[ckey] = value;
        else if( type == "sizet" )
          cachedData_size_t[ckey] = value;
        else if( type == "Real" )
          cachedData_Real[ckey] = value;
        else if( type == "strL" )
          //std::cout << "DIRECT_VALUE, strL: " << value << std::endl;
          cachedData_StringArray[ckey] = value.get<StringArray>();
        else if( type == "str2D" )
          cachedData_String2DArray[ckey] = value.get<String2DArray>();
        else if( type == "intset" )
          cachedData_IntSet[ckey] = value.get<IntSet>();
        else if( type == "id_to_index_set" )
          cachedData_SizetSet[ckey] = value.get<SizetSet>();
        else if( type == "rvec"   ||
                 type == "RealL"  || // need to use the proper kw handler
                 type == "RealLb" || // need to use the proper kw handler
                 type == "RealUb" || // need to use the proper kw handler
                 type == "RealDL"  ) // need to use the proper kw handler
          cachedData_RealVector[ckey] = value.get<JSONRealVector>().value;
        else if( type == "ivec" )
          cachedData_IntVector[ckey] = value.get<JSONIntVector>().value;
        else if( type == "categorical" )
          cachedData_BitArray[ckey] = value.get<JSONBitArray>().value;
        else if( type == "szarray" )
          cachedData_SizetArray[ckey] = value.get<SizetArray>();
        else if( type == "usharray" )
          cachedData_UShortArray[ckey] = value.get<UShortArray>();
        else if( type == "newrvec" ) {
          std::cout << "DIRECT_VALUE, newrvec: " << value << std::endl;
          cachedData_RealSymMatrix[ckey] = value.get<JSONRealSymMatrix>().value;
        }
        else if( type == "resplevs01" ) {
          std::cout << "DIRECT_VALUE, resplevs01: " << value << std::endl;
          cachedData_RealVectorArray[ckey] = value.get<JSONRealVectorArray>().value;
        }
        else
          std::cout << "DIRECT_VALUE: Need to implement data caching for type "
            << type
            << std::endl;
      }
      else if( "PRESENCE_LITERAL" == storage_type ) {
        const std::string& type = key_map_item[currentPath]["handler_type"];
        if( type == "lit" ) {
          const String& keyword = key_map_item[currentPath]["stored_value"];
          cachedData_String[ckey] = keyword;
          std::cout << "Setting " << ckey 
            << " (" << currentPath << ") --> " << keyword << std::endl;
        }
        else
          std::cout << "PRESENCE_LITERAL: Need to implement data caching for type "
            << type
            << std::endl;
      }
      else if( "PRESENCE_BOOL" == storage_type ) {
        const std::string& type = key_map_item[currentPath]["handler_type"];
        assert( "true" == type );
        cachedData_bool[ckey] = true;
      }
      else if( "PRESENCE_ENUM" == storage_type ) {
        const std::string& enum_str = key_map_item[currentPath]["stored_value"];
        if( dakEnumMap().count(enum_str) )
          cachedData_short[ckey] = dakEnumMap().at(enum_str);
        else
          std::cout << "PRESENCE_ENUM: " << enum_str << " has not been registered." << std::endl;
        std::cout << "PRESENCE_ENUM: " << ckey << " = " << enum_str << std::endl;
      }
    }
    if( cache_keys.empty() )
      std::cout << currentPath << " --> " << "No pdb_key" << std::endl;
  }
  else
    std::cout << currentPath << " --> " << "Not in xml_keywords_by_path file" << std::endl;
}

} // namespace Dakota

