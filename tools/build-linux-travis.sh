#!/bin/sh
#
# Automate the build process on Linux based on 
# http://supertuxkart.net/Build_STK_on_Linux

# CMake build type
BUILDTYPE=Debug

# Number of threads to use during compilation (make -j)
THREADS=`lscpu -p | grep -v '^#' | wc -l`

# Relative path to the root directory of this Git repository
REPOROOT=..

export LANG=C

if [ "$CI" = 'true' -a "$TRAVIS" = 'true' ]
then
  THREADS=4
fi

CURRENTDIR=`pwd`
SCRIPTDIR=`dirname "$0"`

cd "$SCRIPTDIR"
cd "$REPOROOT"
rm -rf cmake_build

# One might want to do that to REALLY clean up
#git reset --hard
#git checkout master
#git pull
REVISION=`git rev-parse HEAD`

# If you had Git submodules:
#git submodule foreach git reset --hard
#git submodule foreach git checkout master
#git submodule foreach git pull

mkdir cmake_build
cd cmake_build

cmake .. -DCMAKE_BUILD_TYPE=$BUILDTYPE 2>&1
EXITCODE=$?
if [ $EXITCODE -ne 0 ]
then
  echo
  echo 'ERROR: CMAKE failed with exit code '"$EXITCODE"
  echo 'Git revision: '"$REVISION"
  cd "$CURRENTDIR"
  exit $EXITCODE
fi

make VERBOSE=1 -j $THREADS 2>&1
EXITCODE=$?
if [ $EXITCODE -ne 0 ]
then
  echo
  echo 'ERROR: MAKE failed with exit code '"$EXITCODE"
  echo 'Git revision: '"$REVISION"
  cd "$CURRENTDIR"
  exit $EXITCODE
fi

cd "$SCRIPTDIR"
echo
echo 'SUCCESS: Build succeeded.'
echo 'Git revision: '"$REVISION"
echo
#git status
#git submodule foreach git status
#git submodule foreach git rev-parse HEAD
#ls -l cmake_build/bin/supertuxkart

cd "$CURRENTDIR"
