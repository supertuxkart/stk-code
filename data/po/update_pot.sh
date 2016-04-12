# run this script from the root directory to re-generate the .pot file
#
# ./data/po/update_pot.sh

CPP_FILE_LIST="`find ./src              \
                     -name '*.cpp' -or  \
                     -name '*.c' -or    \
                     -name '*.hpp' -or  \
                     -name "*.h"        \
              `"
XML_FILE_LIST="`find ./data ../stk-assets/tracks ../stk-assets/karts \
                     -name 'achievements.xml' -or                \
                     -name 'kart.xml' -or                        \
                     -name 'track.xml' -or                       \
                     -name 'scene.xml' -or                       \
                     -name '*.challenge' -or                     \
                     -name '*.grandprix' -or                     \
                     -name '*.stkgui'                            \
              `"
ANGELSCRIPT_FILE_LIST="`find ./data ../stk-assets/tracks -name '*.as'`"

echo "--------------------"
echo "    Source Files :"
echo "--------------------"
echo $CPP_FILE_LIST
echo $ANGELSCRIPT_FILE_LIST

echo "--------------------"
echo "    XML Files :"
echo "--------------------"
echo $XML_FILE_LIST

# XML Files
python ./data/po/extract_strings_from_XML.py $XML_FILE_LIST



echo "---------------------------"
echo "    Generating .pot file..."

# XML Files
xgettext  -d supertuxkart --keyword=_ --add-comments="I18N:" \
                               -p ./data/po -o supertuxkart.pot \
                               --no-location --from-code=UTF-8 ./data/po/gui_strings.h \
                               --package-name=supertuxkart

# C++ Files
xgettext  -j  -d supertuxkart --keyword=_ --keyword=N_ --keyword=_LTR \
                               --keyword=_C:1c,2 --keyword=_P:1,2 \
                               --keyword=_CP:1c,2,3 --add-comments="I18N:" \
                               -p ./data/po -o supertuxkart.pot $CPP_FILE_LIST \
                               --package-name=supertuxkart

# Angelscript files (xgettext doesn't support AS so pretend it's c++)
xgettext  -j  -d supertuxkart --keyword="translate" --add-comments="I18N:" \
                               -p ./data/po -o supertuxkart.pot $ANGELSCRIPT_FILE_LIST \
                               --package-name=supertuxkart --language=c++


echo "    Done"
echo "---------------------------"
