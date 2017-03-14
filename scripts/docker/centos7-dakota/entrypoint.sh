#!/bin/bash

# Add local user
# Either use the LOCAL_USER_ID if passed in at runtime or
# fallback

USER_ID=${LOCAL_USER_ID:-9001}

echo "Starting with UID : $USER_ID"
useradd --shell /bin/bash -u $USER_ID -o -c "" -m dakuser
# useradd runs after volumes are mounted and won't use skel. workaround.
export HOME=/home/dakuser
# cp /etc/skel/* ${HOME}

# chown /dakota
##chown -R dakuser /dakota

# Up to this point, everthing was done as root. Switch to standard user now.
exec /usr/local/bin/gosu dakuser "$@"

