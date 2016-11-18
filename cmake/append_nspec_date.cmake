# Script to append NSPEC_DATE to NIDR_keywds0.h

#set(append-line "#define NSPEC_DATE \"${nspec_date}\"\n")
set(append-line
  "#define NSPEC_DATE \"${dakota_version} released ${release_date}\"\n")
file(APPEND "${append_file}" "${append-line}")
