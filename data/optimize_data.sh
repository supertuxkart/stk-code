#!/bin/bash
#
# (C) 2013 Lauri Kasanen, under the GPLv3
# (C) 2014-2015 Odd0002, under the GPLv3
#
# Script to optimize the data, currently PNG, JPEG, B3DZ.
# experimental B3D to B3DZ compression added, not enabled by default
# Run it before making a release, and after adding new data.

#Takes 30+ minutes, depending on your cpu or compression options.

# Spaces in filenames are supported.

#define number of threads
threads=$(nproc)

# Start checks
#if you do not want to use a program, set the variable for it to false here
jpegtran=true
advdef=true
advzip=true
optipng=true
convert=true

#WARNING! SETTING TO TRUE MAY POSSIBLY INCREASE LOAD TIMES ON A SLOW CPU (UNTESTED) OR LEAD TO FILE NOT FOUND ERRORS WHEN RUNNING SUPERTUXKART!
compress_b3d=false


#---------------------------------------------------------
# Begin main code

#Check for reqired programs; if a program is not available, disable it or quit the program

#TODO: make awk optional
if [ ! $(which awk) ]
then
	echo "Please install awk. It is required by this program. \nQuitting..."
	exit 1
fi

#check for jpegtran
if [ ! $(which jpegtran) ]
then
	echo "jpegtran not installed, therefore it will not be used.  It is included in the package \"libjpeg-progs\"."; jpegtran=false
	sleep 2
fi

#check for advdef
if [ ! $(which advdef) ]
then
	echo "advdef is not installed, therefore it will not be used.  It is included in the package \"advancecomp\"."; advdef=false
	sleep 2
fi

#check for advzip
if [ ! $(which advzip) ]
then
	echo "advzip is not installed, therefore it will not be used.  It is included in the package \"advancecomp\"."; advzip=false
	sleep 2
fi

#check for convert
if [ ! $(which convert) ]
then
	echo "convert is not installed, therefore it will not be used."; convert=false
	sleep 2
fi

#check for optipng
if [ ! $(which optipng) ]
then
	echo "optipng is not installed, therefore it will not be used."; optipng=false
	sleep 2
fi


# Defines

#Internal Field Seperator
IFS="
"

export LANG=C

# Go

#store disk usage of files beforehand
BEFORE=`du -sk | awk '{print $1}'`


#functions for xargs multithreading, used instead of GNU parallel for cross-compatibility
#TODO: let next set of optimization scripts run if one set is stuck on a single file at the end to decrease total runtime

#strip ICC information off PNG's
strippng () {
for arg; do
	convert "$arg" -strip "$arg"
done
}
export -f strippng

#optimize PNG's
optimpng () {
for arg; do
	optipng -quiet -o3 "$arg"
#level 3 = 16 trials, which according to http://optipng.sourceforge.net/pngtech/optipng.html (retrieved October 2014) should be satisfactory for all users
done
}
export -f optimpng


#compress PNG in-stream data
comprpng () {
for arg; do
	advdef -z4 "$arg"
done
}
export -f comprpng


#optimize and recompress jpeg files (losslessly)
optimjpg () {
for arg; do
	jpegtran -optimize -copy none -outfile "$arg".$$ "$arg"
	mv "$arg".$$ "$arg"
done
}
export -f optimjpg


#recompress b3dz files
recomprb3dz () {
for arg; do
	advzip -z4 "$arg"
done
}
export -f recomprb3dz
#END MULTITHREADING FUNCTIONS

#strip png icc information
if [ "$convert" = true ]; then
	find . -path .svn -prune -o -name "*.png" -print0 | xargs -0 -n 1 -P "$threads" bash -c 'strippng "$@"' -- #multithread the png stripping
else echo "convert not installed. Ignoring commands using convert..."; sleep 1
fi

#lossless png image optimization
if [ "$optipng" = true ]; then
	find . -path .svn -prune -o -name "*.png" -print0 | xargs -0 -n 1 -P "$threads" bash -c 'optimpng "$@"' -- #multithread the png optimization
else echo "optipng not installed. Ignoring commands using optipng..."; sleep 1
fi


#in-stream data/png compression
if [ "$advdef" = true ]; then
	find . -path .svn -prune -o -name "*.png" -print0 | xargs -0 -n 1 -P "$threads" bash -c 'comprpng "$@"' -- #multithread image compression
else echo "advdef is not installed. Ignoring commands using advdef..."; sleep 1
fi


#lossless jpeg optimization/recompression
if [ "$jpegtran" = true ]; then
	find . -path .svn -prune -o -name "*.jpg" -print0 | xargs -0 -n 1 -P "$threads" bash -c 'optimjpg "$@"' -- #multithread jpg compression and optimization
else echo "jpegtran not installed. Ignoring commands using jpegtran..."; sleep 1
fi


#b3dz to b3dz compression
#WARNING: BETA, MAY CAUSE MISSING FILE WARNINGS!
if [ "$compress_b3d" = true ]; then
	for xmls in $(find . -name "*.xml"); do
		sed 's/b3d/b3dz/g' "$xmls" > "$xmls".$$
		mv "$xmls".$$ "$xmls"
		#echo "$xmls"
		sed 's/b3dzz/b3dz/g' "$xmls" > "$xmls".$$
		mv "$xmls".$$ "$xmls"
		sed 's/b3dzz/b3dz/g' "$xmls" > "$xmls".$$
		mv "$xmls".$$ "$xmls"
	done

	find . -name "*.b3d" -execdir zip '{}.zip' '{}' \;

	for b3dzip in $(find -name "*.b3d.zip"); do
		b3dz="${b3dzip%.zip}"
		#echo "$b3dz"
		mv "$b3dzip" "${b3dz}z"
	done

	find . -type d -name "models" -prune -o -name "*.b3d" -print0 | xargs -0 rm

else echo "b3d to b3dz compression disabled.  Ignoring actions..."; sleep 1
fi

#b3dz file stream compression
if [ "$advzip" = true ]; then
	find . -path .svn -prune -o -name "*.b3dz" -print0 | xargs -0 -n 1 -P "$threads" bash -c 'recomprb3dz "$@"' -- #multithread b3dz recompression
else echo "advzip not installed. Ignoring commands using advzip..."; sleep 1
fi


# Add optimizations for other types if necessary

# get and store new disk usage info
AFTER=`du -sk | awk '{print $1}'`


# Print stats out
echo $BEFORE $AFTER | awk '{print "Size before: " $1/1024 "mb;  Size after: " $2/1024 "mb" }'
echo $BEFORE $AFTER | awk '{print "Saved " (1-($2/$1)) * 100 "%" }'
