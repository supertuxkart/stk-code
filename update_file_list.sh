#! /bin/sh
FILES=`find src -path 'src/bullet' -prune -o -path 'src/ide' -prune -o -path 'src/wiiuse' -prune -o -regex ".*\.[ch]p*" -exec echo "{} " \;`
echo $FILES
sed "s/%LIST_OF_FILES%/$(echo $FILES | sed -e 's/\\/\\\\/g' -e 's/\//\\\//g' -e 's/&/\\\&/g')/g" CMakeLists.in.txt > CMakeLists.txt
