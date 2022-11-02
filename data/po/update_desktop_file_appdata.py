#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import shutil
import sys

def traslate_po(po, translation):
    search = 'msgid "' + translation + '"'
    idx = po.find(search)
    if idx == -1:
        return ''
    # 9 characters after msgid "xxx" for the translated message
    begin = idx + len(search) + 9
    # Search next " with newline
    end = po.find('"\n', begin)
    if end == -1:
        return ''
    return po[begin : end]

STK_DESCRIPTION = 'A 3D open-source kart racing game'
STK_DESKTOP_FILE_P1 = """[Desktop Entry]
"""
# Split it to avoid SuperTuxKart being translated
STK_DESKTOP_FILE_P2 = """Name=SuperTuxKart
Icon=supertuxkart
"""
STK_DESKTOP_FILE_P3 = """#I18N: Generic name in desktop file entry, summary in AppData and short description in Google Play
GenericName=""" + STK_DESCRIPTION + """
Exec=supertuxkart
Terminal=false
StartupNotify=false
Type=Application
Categories=Game;ArcadeGame;
#I18N: Keywords in desktop entry, translators please keep it separated with semicolons
Keywords=tux;game;race;
PrefersNonDefaultGPU=true
"""

desktop_file = open('supertuxkart.desktop', 'w')
desktop_file.write(STK_DESKTOP_FILE_P1 + STK_DESKTOP_FILE_P3)
desktop_file.close()

STK_APPDATA_P1 = 'Karts. Nitro. Action! SuperTuxKart is a 3D open-source arcade racer \
with a variety of characters, tracks, and modes to play. \
Our aim is to create a game that is more fun than realistic, \
and provide an enjoyable experience for all ages.'
STK_APPDATA_P2 = 'We have several tracks with various themes for players to enjoy, \
from driving underwater, rural farmlands, jungles or even in space! \
Try your best while avoiding other karts as they may overtake you, \
but don\'t eat the bananas! Watch for bowling balls, plungers, bubble gum, \
and cakes thrown by your opponents.'
STK_APPDATA_P3 = 'You can do a single race against other karts, \
compete in one of several Grand Prix, \
try to beat the high score in time trials on your own, \
play battle mode against the computer or your friends, \
and more! For a greater challenge, join online and meet players from all over the world \
and prove your racing skills!'
# Used in google play only for now
STK_APPDATA_P4 = 'This game has no ads.'
# Used in google play beta only for now
STK_APPDATA_P5 = 'This is an unstable version of SuperTuxKart that contains latest improvements. \
It is released mainly for testing, to make stable STK as good as possible.'
STK_APPDATA_P6 = 'This version can be installed in parallel with the stable version on the device.'
STK_APPDATA_P7 = 'If you need more stability, consider using the stable version: %s'
STK_STABLE_URL = 'https://play.google.com/store/apps/details?id=org.supertuxkart.stk'

STK_APPDATA_FILE_1 = """<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<component type=\"desktop\">
  <id>supertuxkart.desktop</id>
  <metadata_license>CC0-1.0</metadata_license>
  <project_license>GPL-3.0+</project_license>
"""
# Split it to avoid SuperTuxKart being translated
STK_APPDATA_FILE_2 = """  <name>SuperTuxKart</name>
"""
STK_APPDATA_FILE_3 = """  <summary>""" + STK_DESCRIPTION + """</summary>
  <description>
    <p>
      """ + STK_APPDATA_P1 + """
    </p>
    <p>
      """ + STK_APPDATA_P2 + """
    </p>
    <p>
      """ + STK_APPDATA_P3 + """
    </p>
"""
STK_APPDATA_FILE_4 = """    <p>
      """ + STK_APPDATA_P4 + """
    </p>
    <p>
      """ + STK_APPDATA_P5 + """
    </p>
    <p>
      """ + STK_APPDATA_P6 + """
    </p>
    <p>
      """ + STK_APPDATA_P7 + """
    </p>
"""
STK_APPDATA_FILE_5 = """  </description>
  <screenshots>
    <screenshot type=\"default\">
      <image>https://supertuxkart.net/assets/wiki/STK1.3_1.jpg</image>
      <caption>Normal Race</caption>
    </screenshot>
    <screenshot>
      <image>https://supertuxkart.net/assets/wiki/STK1.3_5.jpg</image>
      <caption>Battle</caption>
    </screenshot>
    <screenshot>
      <image>https://supertuxkart.net/assets/wiki/STK1.3_6.jpg</image>
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
  <url type="faq">https://supertuxkart.net/FAQ</url>
  <url type="vcs-browser">https://github.com/supertuxkart/stk-code</url>
  <url type="contribute">https://supertuxkart.net/Community</url>
  <content_rating type=\"oars-1.1\">
    <content_attribute id=\"violence-cartoon\">mild</content_attribute>
    <content_attribute id=\"social-chat\">intense</content_attribute>
  </content_rating>
  <releases>
    <release version="1.4" date="2022-10-31">
      <url>https://blog.supertuxkart.net/2022/11/supertuxkart-14-release.html</url>
    </release>
    <release version="1.3" date="2021-09-28">
      <url>https://blog.supertuxkart.net/2021/09/supertuxkart-13-release.html</url>
    </release>
    <release version="1.2" date="2020-08-27"/>
    <release version="1.1" date="2020-01-05"/>
    <release version="1.0" date="2019-04-21"/>
  </releases>
  <languages>
"""
STK_APPDATA_FILE_6 = """  </languages>
  <provides>
    <binary>supertuxkart</binary>
  </provides>
  <supports>
    <control>pointing</control>
    <control>keyboard</control>
    <control>gamepad</control>
  </supports>
  <requires>
    <memory>1024</memory>
  </requires>
</component>
"""

appdata_file = open('supertuxkart.appdata.xml', 'w')
appdata_file.write(STK_APPDATA_FILE_1 + STK_APPDATA_FILE_3 + STK_APPDATA_FILE_4 \
+ STK_APPDATA_FILE_5 + STK_APPDATA_FILE_6)
appdata_file.close()

os.system('xgettext -j -d supertuxkart --add-comments=\"I18N:\" \
                    -p ./data/po -o supertuxkart.pot \
                    --package-name=supertuxkart supertuxkart.desktop supertuxkart.appdata.xml')

desktop_file = open('supertuxkart.desktop', 'w')
desktop_file.write(STK_DESKTOP_FILE_P1 + STK_DESKTOP_FILE_P2 + STK_DESKTOP_FILE_P3)
desktop_file.close()

appdata = STK_APPDATA_FILE_1 + STK_APPDATA_FILE_2 + STK_APPDATA_FILE_3
# Skip google play message
appdata += STK_APPDATA_FILE_5

# Manually copy zh_TW to zh_HK for fallback
shutil.copyfile('./data/po/zh_TW.po', './data/po/zh_HK.po')
shutil.rmtree('./google_play_msg', ignore_errors = True)

lingas = open('./data/po/LINGUAS', 'w')
po_list = [f for f in os.listdir('./data/po/') if f.endswith('.po')]
po_list.sort(reverse = False);
fr_percentage = 0
for po_filename in po_list:
    po_file = open('./data/po/' + po_filename, 'r')
    po = po_file.read()
    po_file.close()
    # Remove all newlines in msgid
    po = po.replace('"\n"', '')

    cur_lang = po_filename.removesuffix('.po')
    if cur_lang != 'en':
        lingas.write(cur_lang + '\n')
        total_str = po.count('msgid "')
        untranslated_str = po.count('msgstr ""')
        translated_str = total_str - untranslated_str
        percentage = int(translated_str / total_str * 100.0)

        # Special handling for fr_CA, list has been sorted
        if cur_lang == 'fr':
            fr_percentage = percentage
        elif cur_lang == 'fr_CA':
            percentage = fr_percentage

        if percentage == 0:
            continue
        elif percentage != 100:
            appdata += '    <lang percentage="' + str(percentage) + '">' + cur_lang +'</lang>\n'
        else:
            appdata += '    <lang>' + cur_lang +'</lang>\n'

        if cur_lang != 'fr_CA' and len(sys.argv) == 2 and sys.argv[1] == '--generate-google-play-msg':
            desc = traslate_po(po, STK_DESCRIPTION)
            p1 = traslate_po(po, STK_APPDATA_P1)
            p2 = traslate_po(po, STK_APPDATA_P2)
            p3 = traslate_po(po, STK_APPDATA_P3)
            p4 = traslate_po(po, STK_APPDATA_P4)
            p5 = traslate_po(po, STK_APPDATA_P5)
            p6 = traslate_po(po, STK_APPDATA_P6)
            p7 = traslate_po(po, STK_APPDATA_P7)
            if desc and p1 and p2 and p3 and p4 and p5 and p6 and p7:
                os.makedirs('./google_play_msg/' + cur_lang)
                p7 = p7.replace('%s', STK_STABLE_URL)
                short = open('./google_play_msg/' + cur_lang + '/short.txt', 'w')
                short.write(desc)
                short.close()
                full = open('./google_play_msg/' + cur_lang + '/full.txt', 'w')
                full.write(p1 + '\n\n' + p2 + '\n\n' + p3 + '\n\n' + p4)
                full.close()
                full_beta = open('./google_play_msg/' + cur_lang + '/full_beta.txt', 'w')
                full_beta.write(p1 + '\n\n' + p2 + '\n\n' + p3 + '\n\n' + p4 +
                    '\n\n---\n\n' + p5 + '\n\n' + p6 + '\n\n' + p7)
                full_beta.close()

lingas.close()
appdata += STK_APPDATA_FILE_6
appdata_file = open('supertuxkart.appdata.xml', 'w')
appdata_file.write(appdata)
appdata_file.close()

os.system('msgfmt --desktop -d data/po --template supertuxkart.desktop -o data/supertuxkart.desktop')
os.system('msgfmt --xml -d data/po --template supertuxkart.appdata.xml -o data/supertuxkart.appdata.xml')
os.remove('./supertuxkart.desktop')
os.remove('./supertuxkart.appdata.xml')
os.remove('./data/po/LINGUAS')
os.remove('./data/po/zh_HK.po')
