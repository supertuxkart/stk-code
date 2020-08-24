#!/usr/bin/env python
# Usage: ./generate-country-names.py /path/to/openjdk-src/
# It should have make/data/cldr/common/main/*.xml for country names generation
import xml.dom.minidom
import sys
import os

country_translations = {}
def traverse(file, node, lang):
    for e in node.childNodes:
        if e.localName == None:
            continue
        if e.nodeName == "territory" and e.hasAttribute("type"):
            country = e.getAttribute("type")
            # Skip invalid country
            if not (country[0] >= "A" and country[0] <= "Z") \
                or country == "EZ" or country == "UN" or country == "XA" or country == "XB" or country == "ZZ":
                continue

            translation = ""
            if country == "HK" or country == "MO" or country == "PS":
                if e.hasAttribute("alt"):
                    translation = e.firstChild.nodeValue
            elif not e.hasAttribute("alt"):
                translation = e.firstChild.nodeValue
            if translation == "":
                continue

            # Make sure no tab in translation
            translation = translation.replace("\t", " ")
            if not country in country_translations:
                country_translations[country] = {}
            country_translations[country][lang] = translation
        traverse(file, e, lang)

lang_list = []
for file in os.listdir("../data/po"):
    if file.endswith(".po"):
        lang_list.append(os.path.splitext(file)[0].replace("_", "-"))
lang_list.sort()

real_lang_list = []
for lang in lang_list:
    # Avoid fallback language except tranditional chinese
    target_name = lang.split("-")[0]
    if lang == "zh-TW":
        target_name = "zh_Hant"

    jdk_source = ' '.join(sys.argv[1:])
    target_file = jdk_source + "/make/data/cldr/common/main/" + target_name + ".xml"
    # Use english if no such translation
    if not os.path.isfile(target_file):
        continue
    try:
        doc = xml.dom.minidom.parse(target_file)
    except Exception as ex:
        print("============================================")
        print("/!\\ Expat doesn't like ", file, "! Error=", type(ex), " (", ex.args, ")")
        print("============================================")

    traverse(file, doc, lang)
    real_lang_list.append(lang)

f = open('../data/country_names.tsv', 'w')
f.write("country_code")
for language in real_lang_list:
    f.write("\t")
    f.write(language)
f.write("\n")

for country in country_translations.keys():
    f.write(country)
    for language in real_lang_list:
        f.write("\t")
        if language in country_translations[country].keys():
            f.write(country_translations[country][language])
        else:
            f.write(country_translations[country]["en"])
    f.write("\n")
