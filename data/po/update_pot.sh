# run this script from the root directory to re-generate the .pot file
#
# ./data/po/update_pot.sh

CPP_FILE_LIST="`find ./src -name '*.cpp' -print` `find ./src -name '*.hpp' -print`"
XML_FILE_LIST="`find ./data -name 'track.xml' -print` `find ./data -name '*.challenge' -print` `find ./data -name '*.grandprix' -print`"
OTHER_XML_FILES=`find ./data -name '*.stkgui' -print && find ./data -name '*.challenge' -print && find ./data -name '*.grandprix' -print && find ./data -name 'kart.xml' -print`

echo "--------------------"
echo "    Source Files :"
echo "--------------------"
echo $CPP_FILE_LIST

echo "--------------------"
echo "    XML Files :"
echo "--------------------"
echo $XML_FILE_LIST
echo $OTHER_XML_FILES

# XML Files
python ./data/po/extract_strings_from_XML.py $XML_FILE_LIST $OTHER_XML_FILES



echo "---------------------------"
echo "    Generating .pot file..."

# C++ Files
xgettext    -d supertuxkart -s --keyword=_ --keyword=_LTR --add-comments="I18N:" -p ./data/po -o supertuxkart.pot $CPP_FILE_LIST --package-name=supertuxkart

# Lisp files
#xgettext -j -L lisp -d supertuxkart -s --keyword=_ --add-comments="I18N:" -p ./data/po -o supertuxkart.pot $XML_FILE_LIST --package-name=supertuxkart

# XML Files
xgettext -j -d supertuxkart -s --keyword=_ --add-comments="I18N:" -p ./data/po -o supertuxkart.pot --from-code=UTF-8 ./data/po/gui_strings.h --package-name=supertuxkart

echo "    Done"
echo "---------------------------"
