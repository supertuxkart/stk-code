find ./build/Release/ -name ".svn" -exec rm -rf '{}' > /dev/null \; 2> /dev/null
find ./build/Release/ -name ".DS_Store" -exec rm -rf '{}' > /dev/null \; 2> /dev/null
find ./build/Release/ -name "Makefile*" -exec rm -rf '{}' > /dev/null \; 2> /dev/null 
exit 0