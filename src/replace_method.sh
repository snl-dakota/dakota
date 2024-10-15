#!/bin/bash

# Function to display usage information
help() {
  echo "Usage: $0 <old_method_name> <new_method_name>"
  echo
  echo "This script refactors C++ code by replacing method calls on 'model' or 'Model' objects."
  echo
  echo "Arguments:"
  echo "  <old_method_name>  The name of the method to be replaced"
  echo "  <new_method_name>  The name of the new method to be inserted"
  echo
  echo "Example:"
  echo "  $0 num_linear_ineq_constraints user_defined_constraints"
  echo
  echo "This will replace 'model.num_linear_ineq_constraints' with"
  echo "'model.user_defined_constraints().num_linear_ineq_constraints'"
  echo "in all .cpp and .hpp files in the current directory and its subdirectories."
}

# Function to process a single file
process_file() {
  local file="$1"
  local old_method="$2"
  local new_method="$3"
  local backup="${file}.bak"

  # Create a backup of the original file
  cp "$file" "$backup"

  # Replace method calls
  sed -i "s/\(\w*\)odel\.$old_method/\1odel.$new_method().$old_method/g" "$file"

  # Compare the modified file with the backup
  if diff -q "$file" "$backup" >/dev/null; then
    # No changes were made, restore the original file
    mv "$backup" "$file"
    echo "No changes in $file"
  else
    # Changes were made, remove the backup
    rm "$backup"
    echo "Updated $file"
  fi
}

# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <old_method_name> <new_method_name>"
  exit 1
fi

OLD_METHOD="$1"
NEW_METHOD="$2"


# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
    echo "Error: Incorrect number of arguments." >&2
    echo >&2
    show_usage
    exit 1
fi


# Process all .cpp and .hpp files in the current directory and subdirectories
find . -type f \( -name "*.cpp" -o -name "*.hpp" \) | while read -r file; do
  process_file "$file" "$OLD_METHOD" "$NEW_METHOD"
done

echo "Refactoring complete."
