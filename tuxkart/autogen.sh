#!/bin/sh

set -e

echo "Running aclocal"
aclocal -I m4
echo "Running automake"
automake --add-missing --copy
echo "Running autoconf"
autoconf 

# EOF #
