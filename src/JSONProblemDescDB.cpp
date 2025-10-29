/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "JSONProblemDescDB.hpp"

namespace Dakota {

JSONProblemDescDB::JSONProblemDescDB(const std::string& filename)
{
  // Open the JSON file
  std::ifstream inputFile(filename);
  if (!inputFile.is_open()) {
    std::cerr << "Error opening file: " << filename << std::endl;
    return;
  }

  // Parse the JSON file
  inputFile >> jsonOptions;

  // Determine block ids
  allowedBlocks = {"environment", "method", "variables", "responses"};

  for (size_t i = 0; i < jsonOptions.size(); ++i) {
    for (auto& element : jsonOptions[i].items()) {
      blockIds[element.key()] = i;
      if( allowedBlocks.find(element.key()) == allowedBlocks.end() )
        std::cout << "json parameter block \"" << element.key() << "\" is unknown to Dakota." << std::endl;
    }
  }

  // Debugging/sanity check
  if( false ) {
    for( const auto & b : blockIds )
      std::cout << "json parameter block \"" << b.first << "\" has id " << b.second << std::endl;
  }
}

} // namespace Dakota

