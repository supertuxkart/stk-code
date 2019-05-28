#!/usr/bin/env python3

# usage: generate-countries-table.py > countries.csv
# in sqlite3 terminal:
#
# .mode csv
# .headers off
# .separator ";"
# .import `full path to countries.csv` `v(database_version)_countries`
#

import csv
import os
import sys

CSV_FILE = '../data/country_names.csv'
# Use another name in the country_code header if you want countries names in different language
READABLE_NAME = 'en'
# ord("🇦") - ord("A")
FLAG_OFFSET = 127397

if not os.path.exists(CSV_FILE):
    print("File = {} does not exist.".format(CSV_FILE))
    sys.exit(1)

with open(CSV_FILE, 'r') as csvfile:
    country = csv.DictReader(csvfile, delimiter=';', quotechar='"')
    # Skip header
    next(country)
    for row in country:
        country_code = row['country_code']
        codepoints = [ord(x) + FLAG_OFFSET for x in country_code]
        print('%s;%s;%s' % (country_code, chr(codepoints[0]) + chr(codepoints[1]), row[READABLE_NAME]))
