#!/usr/bin/env sh

PROJECTS="utilib coliny dakota pebbl pico all"
URL=https://software.sandia.gov/svn/public/acro
cwd=`pwd`

if [ -n "$1" ]; then
    WD="$1"
else
    WD=tmp_rebootstrap
fi

if [[ $WD == /* ]]; then
    ABS_WD="${WD}"
else
    ABS_WD="${cwd}/${WD}"
fi

# Setup
if ! test -d "${WD}/.svn"; then
    svn co -N https://software.sandia.gov/svn/public/acro "${WD}/."
fi
for x in $PROJECTS; do
    if ! test -d "${WD}/acro-$x"; then
        echo "Checking out acro-$x"
        svn co --ignore-externals $URL/acro-$x/trunk "${WD}/acro-$x"
	ln -s "${ABS_WD}/bootstrap" "${WD}/acro-$x/."
	ln -s "${ABS_WD}/bin" "${WD}/acro-$x/."
    fi
done
# Doing the projects first helps to fool cp_u into actually copying the
# bootstrap files over.
if ! test -d bootstrap; then
    echo "Checking out bootstrap"
    svn co $URL/common/trunk/bootstrap "${WD}/bootstrap"
fi
if ! test -d bin; then
    echo "Checking out bin"
    svn co $URL/common/trunk/bin "${WD}/bin"
fi

# Update
for x in $PROJECTS; do
    echo "Updating acro-$x"
    svn update --ignore-externals "${WD}/acro-$x"
done
echo "Updating bootstrap"
svn update "${WD}/bootstrap"
echo "Updating bin"
svn update "${WD}/bin"

# Bootstrap
for x in $PROJECTS; do
    cd "${WD}/acro-$x"
    ./bootstrap/bootstrap all
    cd "$cwd"
done

echo "Check changes with:"
echo "   svn st ${WD}/acro-* | grep -v ^X"
echo "If you are happy, commit with:"
echo "  svn ci ${WD}/acro-* -m '(re-syncing with common/bootstrap)'"
