#!/bin/sh

set -e

WANT_AUTOMAKE=1.7

if [ -n "`which automake-1.7`" ]; then
    ACLOCAL="aclocal-1.7"
    AUTOMAKE="automake-1.7"
elif [ -n "`which automake-1.6`" ]; then
    ACLOCAL="aclocal-1.6"
    AUTOMAKE="automake-1.6"
else
    ACLOCAL="aclocal"
    AUTOMAKE="automake"
fi

case "`$AUTOMAKE --version`" in
    *1.5* | *1.4*)
        echo "Error: You need automake 1.6 or larger"
        exit 1
        ;;
esac

# automake tends to fail sometimes if Makefile or Makefile.in are left from old
# runs
find -name "Makefile" -o -name "Makefile.in" -exec rm {} ';'

echo "Running $ACLOCAL"
$ACLOCAL -I m4
echo "Running $AUTOMAKE"
$AUTOMAKE --add-missing --copy
echo "Running autoconf"
autoconf 

# EOF #
