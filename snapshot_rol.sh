#!/bin/sh

# Assuming you are at the top level of Daktoa core and on the correct branch (eg rol_integ) ...

DAKOTA_SRC_DIR=${DAKOTA_DIR:-"/home/rhoope/Projects/Dakota"}
TRILINOS_SRC_DIR=${TRILINOS_DIR:-"/home/rhoope/Projects/Trilinos"}

# Clean out all existing rol content
cd $DAKOTA_SRC_DIR/packages/external/rol
git checkout rol_integ
rm $DAKOTA_SRC_DIR/packages/external/rol/include/*
#git checkout -- CMakeLists.txt 

cd $TRILINOS_SRC_DIR/packages
git fetch
git checkout origin/develop
rm -f $DAKOTA_SRC_DIR/packages/external/rol/ROL_Version.txt
echo 'Trilinos sha1: ' `git log --pretty=format:%h -1` > $DAKOTA_SRC_DIR/packages/external/rol/ROL_Version.txt ' '
echo 'Build Date: ' `git log --pretty=format:%ci -1` >> $DAKOTA_SRC_DIR/packages/external/rol/ROL_Version.txt ' '


ROL_SOURCE=$TRILINOS_SRC_DIR/packages/rol \
  ROL_HOME=$DAKOTA_SRC_DIR/packages/external/rol \
  ./rol/install.sh

cd $DAKOTA_SRC_DIR/packages/external/rol
cp $TRILINOS_SRC_DIR/packages/rol/cmake/ROL_config.h.in .
sed -e 's/cmakedefine/define/; T; s/$/ 0/' ROL_config.h.in > include/ROL_config.h

cd $DAKOTA_SRC_DIR/packages/external
git add rol
git status

# Now commit the updated snapshot if desired ...
