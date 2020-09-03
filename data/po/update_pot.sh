# run this script from the root directory to re-generate the .pot file
#
# ./data/po/update_pot.sh
if [ -z "$PYTHON" ]; then
  PYTHON="python"
fi

CPP_FILE_LIST="`find ./src                 \
                     -name '*.cpp' -or     \
                     -name '*.c' -or       \
                     -name '*.hpp' -or     \
                     -name "*.h" | sort -n \
              `"
XML_FILE_LIST="`find ./data                        \
                     ../stk-assets/tracks          \
                     ../stk-assets/karts           \
                     ../supertuxkart-assets/tracks \
                     ../supertuxkart-assets/karts  \
                     ./android/res/values          \
                     -name 'achievements.xml' -or  \
                     -name 'tips.xml' -or  \
                     -name 'kart.xml' -or          \
                     -name 'track.xml' -or         \
                     -name 'scene.xml' -or         \
                     -name '*.challenge' -or       \
                     -name '*.grandprix' -or       \
                     -name 'strings.xml' -or       \
                     -name '*.stkgui' | sort -n    \
              `"
ANGELSCRIPT_FILE_LIST="`find ./data                        \
                             ../stk-assets/tracks          \
                             ../supertuxkart-assets/tracks \
                             -name '*.as' | sort -n        \
                      `"

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
eval '$PYTHON ./data/po/extract_strings_from_XML.py $XML_FILE_LIST'



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

STK_DESKTOP_FILE_P1="[Desktop Entry]"
# Split it to avoid SuperTuxKart being translated
STK_DESKTOP_FILE_P2="Name=SuperTuxKart
Icon=supertuxkart"
STK_DESKTOP_FILE_P3="#I18N: Generic name in desktop file entry, used in linux
GenericName=A kart racing game
Exec=supertuxkart
Terminal=false
StartupNotify=false
Type=Application
Categories=Game;ArcadeGame;
#I18N: Keywords in desktop entry, translators please keep it separated with semicolons
Keywords=tux;game;race;
PrefersNonDefaultGPU=true"

echo "${STK_DESKTOP_FILE_P1}" > supertuxkart.desktop
echo "${STK_DESKTOP_FILE_P3}" >> supertuxkart.desktop

# Desktop entry
xgettext -j -d supertuxkart --add-comments="I18N:" \
                            -p ./data/po -o supertuxkart.pot \
                            --package-name=supertuxkart supertuxkart.desktop

echo "${STK_DESKTOP_FILE_P1}" > supertuxkart.desktop
echo "${STK_DESKTOP_FILE_P2}" >> supertuxkart.desktop
echo "${STK_DESKTOP_FILE_P3}" >> supertuxkart.desktop

for PO in $(ls data/po/*.po); do
    LANG=$(basename $PO .po)
    if [ "$LANG" = "en" ]; then
        continue
    fi
    printf "$LANG " >> data/po/LINGUAS
done

msgfmt --desktop -d data/po --template supertuxkart.desktop -o data/supertuxkart.desktop
rm -f ./supertuxkart.desktop
rm -f ./data/po/LINGUAS

echo "    Done"
echo "---------------------------"
