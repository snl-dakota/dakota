

set(Dakota_VERSION_MAJOR 6)
set(Dakota_VERSION_MINOR 16)
set(Dakota_VERSION_PATCH 0)

# --- Release time toggles

# Toggle this to append to the version number for stable
#set(Dakota_VERSION_APPEND_STABLE "")   # Versioned release
set(Dakota_VERSION_APPEND_STABLE "+")  # Stable releases

# If empty, the build date will be used as the release date (could consider 
# using the Git rev date); use format Mmm dd yyyy, as in asctime()
#set(Dakota_RELEASE_DATE "May 16 2022")    # Versioned release date Mmm dd yyyy
set(Dakota_RELEASE_DATE "")               # Stable release: empty -> build date

# Define convenience variables used here and in subdirs, such as
#   Dakota_VERSION_{PAIR, TRIPLE, STRING, SRC}
include(DakotaVersionUtils)
dakota_version_set_helper_vars()

