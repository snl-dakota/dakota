
# Prepare build for gcov (code coverage) reporting
set(DAKOTA_GCOV TRUE CACHE BOOL "Enable gcov")

# Use shared libraries (and don't use static)
set(BUILD_SHARED_LIBS TRUE  CACHE BOOL "Build shared libs?")

# No bounds checking in Teuchos
set(Teuchos_ENABLE_ABC FALSE CACHE BOOL "Enable bounds checking?")
