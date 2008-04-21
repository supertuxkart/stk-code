# run this script from the root directory to re-generate the .pot file
#
# ./data/po/update_pot.sh

xgettext -d supertuxkart -s --keyword=_ -p ./data/po -o supertuxkart.pot ./src/*.cpp ./src/*/*.cpp