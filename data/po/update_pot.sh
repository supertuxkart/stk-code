# run this script from the root directory to re-generate the .pot file
#
# ./data/po/update_pot.sh

CPP_FILE_LIST="`find ./src              \
                     -name '*.cpp' -or  \
                     -name '*.c' -or    \
                     -name '*.hpp' -or  \
                     -name "*.h"        \
              `"
XML_FILE_LIST="`find ./data ../stk-assets ../supertuxkart-assets \
                     -name 'achievements.xml' -or                \
                     -name 'kart.xml' -or                        \
                     -name 'track.xml' -or                       \
                     -name 'scene.xml' -or                       \
                     -name '*.challenge' -or                     \
                     -name '*.grandprix' -or                     \
                     -name '*.stkgui'                            \
              `"

echo "--------------------"
echo "    Source Files :"
echo "--------------------"
echo $CPP_FILE_LIST

echo "--------------------"
echo "    XML Files :"
echo "--------------------"
echo $XML_FILE_LIST

# XML Files
python ./data/po/extract_strings_from_XML.py $XML_FILE_LIST



echo "---------------------------"
echo "    Generating .pot file..."

# C++ Files
xgettext    -d supertuxkart -s --keyword=_ --keyword=N_ --keyword=_LTR \
                               --keyword=_C:1c,2 --keyword=_P:1,2 \
                               --keyword=_CP:1c,2,3 --add-comments="I18N:" \
                               -p ./data/po -o supertuxkart.pot $CPP_FILE_LIST \
                               --package-name=supertuxkart

# XML Files
xgettext -j -d supertuxkart -s --keyword=_ --add-comments="I18N:" \
                               -p ./data/po -o supertuxkart.pot \
                               --from-code=UTF-8 ./data/po/gui_strings.h \
                               --package-name=supertuxkart

echo "    Done"
echo "---------------------------"
