#/bin/bash

echo Copying data/shaders to data/shaders_optimized
cp -R data/shaders data/shaders_optimized # Copy shaders to data/shaders_optimized

cd data/shaders_optimized

find . -type f -name "*.vert" -exec sed -i '1i  #version 140' {} \;
glslopt -v coloredquad.vert coloredquad.vert

find . -type f -name "*.vert" -exec sed -i '/#version 140/d' {} \;

sed -i '1s/^/#version 140\n/' ./pass.vert # To fix the crash

