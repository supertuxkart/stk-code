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

TSV_FILE = '../data/country_names.tsv'
# Use another name in the country_code header if you want countries names in different language
READABLE_NAME = 'en'
# ord("🇦") - ord("A")
FLAG_OFFSET = 127397

if not os.path.exists(TSV_FILE):
    print("File = {} does not exist.".format(TSV_FILE))
    sys.exit(1)

with open(TSV_FILE, 'r') as tsvfile:
    country = csv.DictReader(tsvfile, delimiter='\t', quotechar='"')
    # Skip header
    next(country)
    for row in country:
        country_code = row['country_code']
        codepoints = [ord(x) + FLAG_OFFSET for x in country_code]
        print('%s;%s;%s' % (country_code, chr(codepoints[0]) + chr(codepoints[1]), row[READABLE_NAME]))
