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

STK_DESCRIPTION="A 3D open-source kart racing game"
STK_DESKTOP_FILE_P1="[Desktop Entry]"
# Split it to avoid SuperTuxKart being translated
STK_DESKTOP_FILE_P2="Name=SuperTuxKart
Icon=supertuxkart"
STK_DESKTOP_FILE_P3="#I18N: Generic name in desktop file entry, \
summary in AppData and short description in Google Play
GenericName=$STK_DESCRIPTION
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

STK_APPDATA_P1="Karts. Nitro. Action! SuperTuxKart is a 3D open-source arcade racer \
with a variety of characters, tracks, and modes to play. \
Our aim is to create a game that is more fun than realistic, \
and provide an enjoyable experience for all ages."
STK_APPDATA_P2="Discover the mystery of an underwater world, \
or drive through the jungles of Val Verde and visit the famous Cocoa Temple. \
Race underground or in a spaceship, through a rural farmland or a strange alien planet. \
Or rest under the palm trees on the beach, watching the other karts overtake you. \
But don't eat the bananas! Watch for bowling balls, plungers, bubble gum, \
and cakes thrown by your opponents."
STK_APPDATA_P3="You can do a single race against other karts, \
compete in one of several Grand Prix, \
try to beat the high score in time trials on your own, \
play battle mode against the computer or your friends, \
and more! For a greater challenge, race online against players from all over the world \
and prove your racing skills!"
# Used in google play only for now
STK_APPDATA_P4="This game is free and without ads."
# Used in google play beta only for now
STK_APPDATA_P5="This is an unstable version of SuperTuxKart that contains latest improvements. \
It is released mainly for testing, to make stable STK as good as possible."
STK_APPDATA_P6="This version can be installed in parallel with the stable version on the device."
STK_APPDATA_P7="If you need more stability, consider using the stable version: %s"
STK_STABLE_URL="https://play.google.com/store/apps/details?id=org.supertuxkart.stk"

STK_APPDATA_FILE_1="<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<component type=\"desktop\">
  <id>supertuxkart.desktop</id>
  <metadata_license>CC0-1.0</metadata_license>
  <project_license>GPL-3.0+</project_license>"
# Split it to avoid SuperTuxKart being translated
STK_APPDATA_FILE_2="  <name>SuperTuxKart</name>"
STK_APPDATA_FILE_3="  <summary>A 3D open-source kart racing game</summary>
  <description>
    <p>
      "${STK_APPDATA_P1}"
    </p>
    <p>
      "${STK_APPDATA_P2}"
    </p>
    <p>
      "${STK_APPDATA_P3}"
    </p>"
STK_APPDATA_FILE_4="    <p>
      $STK_APPDATA_P4
    </p>
    <p>
      $STK_APPDATA_P5
    </p>
    <p>
      $STK_APPDATA_P6
    </p>
    <p>
      $STK_APPDATA_P7
    </p>"
STK_APPDATA_FILE_5="  </description>
  <screenshots>
    <screenshot type=\"default\">
      <image>https://supertuxkart.net/images/8/83/Supertuxkart-0.9.2-screenshot-3.jpg</image>
      <caption>Normal Race</caption>
    </screenshot>
    <screenshot>
      <image>https://supertuxkart.net/images/1/1f/Supertuxkart-0.9.2-screenshot-1.jpg</image>
      <caption>Battle</caption>
    </screenshot>
    <screenshot>
      <image>https://supertuxkart.net/images/2/2a/Supertuxkart-0.9.2-screenshot-2.jpg</image>
      <caption>Soccer</caption>
    </screenshot>
  </screenshots>
  <developer_name>SuperTuxKart Team</developer_name>
  <update_contact>supertuxkart-devel@lists.sourceforge.net</update_contact>
  <url type=\"homepage\">https://supertuxkart.net</url>
  <url type=\"bugtracker\">https://github.com/supertuxkart/stk-code/issues</url>
  <url type=\"donation\">https://supertuxkart.net/Donate</url>
  <url type=\"help\">https://supertuxkart.net/Community</url>
  <url type=\"translate\">https://supertuxkart.net/Translating_STK</url>
  <content_rating type=\"oars-1.1\">
    <content_attribute id=\"violence-cartoon\">mild</content_attribute>
    <content_attribute id=\"social-chat\">intense</content_attribute>
  </content_rating>
  <languages>"
STK_APPDATA_FILE_6="  </languages>
</component>"

echo "${STK_APPDATA_FILE_1}" > supertuxkart.appdata.xml
echo "${STK_APPDATA_FILE_3}" >> supertuxkart.appdata.xml
echo "${STK_APPDATA_FILE_4}" >> supertuxkart.appdata.xml
echo "${STK_APPDATA_FILE_5}" >> supertuxkart.appdata.xml
echo "${STK_APPDATA_FILE_6}" >> supertuxkart.appdata.xml

# Desktop and AppData entry
xgettext -j -d supertuxkart --add-comments="I18N:" \
                            -p ./data/po -o supertuxkart.pot \
                            --package-name=supertuxkart supertuxkart.desktop supertuxkart.appdata.xml

echo "${STK_DESKTOP_FILE_P1}" > supertuxkart.desktop
echo "${STK_DESKTOP_FILE_P2}" >> supertuxkart.desktop
echo "${STK_DESKTOP_FILE_P3}" >> supertuxkart.desktop

echo "${STK_APPDATA_FILE_1}" > supertuxkart.appdata.xml
echo "${STK_APPDATA_FILE_2}" >> supertuxkart.appdata.xml
echo "${STK_APPDATA_FILE_3}" >> supertuxkart.appdata.xml
# Skip google play message
echo "${STK_APPDATA_FILE_5}" >> supertuxkart.appdata.xml

# Manually copy zh_TW to zh_HK for fallback
cp data/po/zh_TW.po data/po/zh_HK.po
rm -rf ./google_play_msg

function translate_str()
{
    # Remove newline in msgid of po file first
    echo $(sed ':a;N;$!ba;s/\"\n\"//g' "$2" \
        | grep -A 1 -e "msgid \"$1\"" | sed -n 's/msgstr "\(.*\)"/\1/p')
}

for PO in $(ls data/po/*.po); do
    CUR_LANG=$(basename $PO .po)
    if [ "$CUR_LANG" != "en" ]; then
        printf "$CUR_LANG " >> data/po/LINGUAS
        PO_NO_FALLBACK=$PO
        if [ "$CUR_LANG" = "fr_CA" ]; then
            PO_NO_FALLBACK="data/po/fr.po"
        fi
        TOTAL_STR=$(sed ':a;N;$!ba;s/\"\n\"//g' $PO_NO_FALLBACK | grep "msgid \"" | wc -l)
        UNTRANSLATED_STR=$(sed ':a;N;$!ba;s/\"\n\"//g' $PO_NO_FALLBACK | grep "msgstr \"\"" | wc -l)
        TRANSLATED_STR=$(expr $TOTAL_STR - $UNTRANSLATED_STR)
        PERCENTAGE=$(python -c "print(int($TRANSLATED_STR / $TOTAL_STR * 100.0))")
        if [ "$PERCENTAGE" = "0" ]; then
            continue
        elif [ "$PERCENTAGE" != "100" ]; then
            printf "    <lang percentage=\"$PERCENTAGE\">$CUR_LANG</lang>" >> supertuxkart.appdata.xml
        else
            printf "    <lang>$CUR_LANG</lang>" >> supertuxkart.appdata.xml
        fi
    fi
    if [ "$1" != "--generate-google-play-msg" ]; then
        continue
    fi
    DESC=$(translate_str "$STK_DESCRIPTION" "$PO")
    P1=$(translate_str "$STK_APPDATA_P1" "$PO")
    P2=$(translate_str "$STK_APPDATA_P2" "$PO")
    P3=$(translate_str "$STK_APPDATA_P3" "$PO")
    P4=$(translate_str "$STK_APPDATA_P4" "$PO")
    P5=$(translate_str "$STK_APPDATA_P5" "$PO")
    P6=$(translate_str "$STK_APPDATA_P6" "$PO")
    P7=$(translate_str "$STK_APPDATA_P7" "$PO")
    if [ -n "$DESC" ] && [ -n "$P1" ] && [ -n "$P2" ] && [ -n "$P3" ] && \
        [ -n "$P4" ] && [ -n "$P5" ] && [ -n "$P6" ] && [ -n "$P7" ]; then
        mkdir -p ./google_play_msg/$CUR_LANG
        P7=$(sed "s|%s|$STK_STABLE_URL|g" <<< $P7)
        printf "$DESC" > google_play_msg/$CUR_LANG/short.txt
        printf "$P1\n\n$P2\n\n$P3\n\n$P4" > google_play_msg/$CUR_LANG/full.txt
        printf "$P1\n\n$P2\n\n$P3\n\n$P4\n\n---\n\n$P5\n\n$P6\n\n$P7" > google_play_msg/$CUR_LANG/full_beta.txt
    fi
done
echo "${STK_APPDATA_FILE_6}" >> supertuxkart.appdata.xml

msgfmt --desktop -d data/po --template supertuxkart.desktop -o data/supertuxkart.desktop
msgfmt --xml -d data/po --template supertuxkart.appdata.xml -o data/supertuxkart.appdata.xml
rm -f ./supertuxkart.desktop
rm -f ./supertuxkart.appdata.xml
rm -f ./data/po/LINGUAS
rm -f ./data/po/zh_HK.po

echo "    Done"
echo "---------------------------"
