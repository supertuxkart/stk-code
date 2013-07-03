#!/bin/sh
#
#	Run a syntax check on all the shaders.
#
#	The utility glslopt may be gotten from github.com/clbr/glsl-optimizer.

out() {
	rm -f *.out
	echo Failed: $1
	exit 1
}


for vert in *.vert; do
	glslopt -v $vert
	[ $? -ne 0 ] && out $vert
done

for frag in *.frag; do
	glslopt -f $frag
	[ $? -ne 0 ] && out $frag
done


rm -f *.out
