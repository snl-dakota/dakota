# Configure the Dakota version, stable build indicator, and release date 

set(Dakota_VERSION_MAJOR 6)
set(Dakota_VERSION_MINOR 20)
set(Dakota_VERSION_PATCH 0)

# Toggle this to append to the version number for stable
#set(Dakota_VERSION_APPEND_STABLE "")  # Versioned release
set(Dakota_VERSION_APPEND_STABLE "+")  # Stable releases

# If empty, the build date will be used as the release date (could consider 
# using the Git rev date); use format Mmm dd yyyy, as in asctime()
#set(Dakota_RELEASE_DATE "Nov. 15 2023")  # Versioned release date Mmm dd yyyy
set(Dakota_RELEASE_DATE "")               # Stable release: empty -> build date


