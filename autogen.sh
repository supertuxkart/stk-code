#!/bin/sh

OSTYPE=`uname -s`
MACHINE=`uname -m`
AUTO_MAKE_VERSION=`automake --version | head -1 | awk '{print $4}' | sed -e 's/\.\([0-9]*\).*/\1/'`
if test $AUTO_MAKE_VERSION -lt 15; then
    echo ""
    echo "You need to upgrade to automake version 1.5 or greater."
    echo "Most distributions have packages available to install or you can"
    echo "find the source for the most recent version at"
    echo "ftp://ftp.gnu.org/gnu/automake"
    exit 1
fi

echo "Host info: $OSTYPE $MACHINE"
echo -n " automake: `automake --version | head -1 | awk '{print $4}'`"
echo " ($AUTO_MAKE_VERSION)"
echo ""

echo "Running aclocal"
aclocal

echo "Running automake --add-missing"
automake --add-missing

echo "Running autoconf"
autoconf

if [ ! -e configure ]; then
    echo "ERROR: configure was not created!"
    exit 1
fi

echo ""
echo "======================================"

if [ -f config.cache ]; then
    echo "config.cache exists.  Removing the config.cache file will force"
    echo "the ./configure script to rerun all it's tests rather than using"
    echo "the previously cached values."
    echo ""
fi

echo "Now you are ready to run './configure'"
echo "======================================"

# EOF #
