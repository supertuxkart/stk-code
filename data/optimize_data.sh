#!/bin/sh
#
# (C) 2013 Lauri Kasanen, under the GPLv3
#
# Script to optimize the data, currently PNG, JPEG, B3DZ.
# Run it before making a release, and after adding new data.
# Takes 5-10min depending on your cpu.

# Spaces in filenames are supported.

# Start checks

fail=0
which awk >/dev/null && which jpegtran >/dev/null && which advdef >/dev/null && \
which advzip >/dev/null || fail=1

[ "$fail" -eq 1 ] && echo "Please install awk, advdef and jpegtran" && exit 1

# Defines

IFS="
"

export LANG=C

# Go

BEFORE=`du -sk | awk '{print $1}'`

for png in `find -name "*.png"`; do
	advdef -z4 "$png"
done

for jpeg in `find -name "*.jpg"`; do
	jpegtran -optimize -copy none -outfile "$jpeg".$$ "$jpeg"
	mv "$jpeg".$$ "$jpeg"
done

for b3dz in `find -name "*.b3dz"`; do
	advzip -z4 "$b3dz"
done

# Add optimizations for other types if necessary

AFTER=`du -sk | awk '{print $1}'`

# Print stats out
echo $BEFORE $AFTER | awk '{print "Before: " $1/1024 "mb, after: " $2/1024 "mb" }'
echo $BEFORE $AFTER | awk '{print "Saved " (1-($2/$1)) * 100 "%" }'
