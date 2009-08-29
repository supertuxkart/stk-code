# run this script from the root directory to re-generate the .pot file
#
# ./data/po/update_pot.sh

CPP_FILE_LIST=`find ./src -name '*.cpp' -print`
PERL_FILE_LIST="`find ./data -name '*.track' -print` `find ./data -name '*.challenge' -print` `find ./data -name '*.grandprix' -print`"
XML_FILE_LIST=`find ./data -name '*.xml' -print`
STKGUI_FILE_LIST=`find ./data -name '*.stkgui' -print`

echo "--------------------"
echo "    Source Files :"
echo "--------------------"
echo $CPP_FILE_LIST

echo "--------------------"
echo "    Data Files :"
echo "--------------------"
echo $PERL_FILE_LIST

echo "--------------------"
echo "    XMl Files :"
echo "--------------------"
echo $XML_FILE_LIST $STKGUI_FILE_LIST



echo "---------------------------"
echo "    Generating .pot file..."

# C++ Files
xgettext    -d supertuxkart -s --keyword=_ --add-comments="I18N:" -p ./data/po -o supertuxkart.pot $CPP_FILE_LIST

# Lisp files
xgettext -j -L perl -d supertuxkart -s --keyword=_ --add-comments="I18N:" -p ./data/po -o supertuxkart.pot $PERL_FILE_LIST

# XML Files
# (Since xgettext cannot read XML directly, strings are extracted using 'grep' and 'sed' first
grep -ho 'text=\"[^\"]*' $XML_FILE_LIST $STKGUI_FILE_LIST | sed 's/text=\"\(.*\)/_(\"\1\")/' > ./data/po/gui_strings.txt
xgettext -j -L perl -d supertuxkart -s --keyword=_ --add-comments="I18N:" -p ./data/po -o supertuxkart.pot ./data/po/gui_strings.txt

echo "    Done"
echo "---------------------------"