#!/bin/sh

# Library directory
LIBDIR="bin"

# If we are launching from a symlink, such as /usr/local/bin/run_supertux.sh, we need to get where
# the symlink points to
pth="`readlink $0`"

# $pth will be empty if our start path wasnt a symlink
if [ $pth ]; then
	GAMEDIR="`dirname $pth`"
else
	GAMEDIR="`dirname $0`"
fi

# Change to the game dir, and go!
cd $GAMEDIR
# export game library directory
test -n "${LIBDIR}" && export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${GAMEDIR}/${LIBDIR}"

bin/supertuxkart $*

