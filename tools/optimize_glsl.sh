#/bin/bash

VERSION=false
if grep -q "#version" "$1"; then
    echo "'#version 140' already in GLSL shader"
    VERSION="true"
else
    echo "Adding '#version 140' to shader"
    sed -i '1i  #version 140' $1
    VERSION="false"
fi

if [ "$2" = "-f" ]; then
    glslopt -f $1 $1
elif [ "$2" = "-v" ]; then
    glslopt -v $1 $1
else
    echo "Argument error"
fi
if [ $VERSION = "false" ]; then
    echo "Removing '#version 140'"
    sed -i '/#version 140/d' $1
else
    echo "Keeping '#version 140'"
fi
