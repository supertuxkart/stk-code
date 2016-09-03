#/bin/bash

echo Copying data/shaders to data/shaders_optimized
cp -R data/shaders data/shaders_optimized # Copy shaders to data/shaders_optimized

cd data/shaders_optimized

find . -type f -name "*.vert" -exec ../../tools/optimize_glsl.sh {} -v $1 \;

find . -type f -name "*.frag" -exec ../../tools/optimize_glsl.sh {} -f $1 \;

# Use unoptimized version of faulty shaders
cp ../shaders/particlesimheightmap.vert .
cp ../shaders/texturedquad.vert .
cp ../shaders/pointemitter.vert .
cp ../shaders/importance_sampling_specular.frag .
cp ../shaders/screenquad.vert .
cp ../shaders/colortexturedquad.vert .
