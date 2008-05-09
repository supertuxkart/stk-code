# run this script from the root directory to re-generate the .pot file
#
# ./data/po/update_pot.sh

CPP_FILE_LIST=`find ./src -name '*.cpp' -print`

echo "--------------------"
echo "    Files :"
echo "--------------------"
echo $CPP_FILE_LIST

echo "---------------------------"
echo "    Generating .pot file..."
xgettext -d supertuxkart -s --keyword=_ -p ./data/po -o supertuxkart.pot ./src/*.cpp ./src/*/*.cpp
echo "    Done"
echo "---------------------------"