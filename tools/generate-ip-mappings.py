#!/usr/bin/env python3
# Download from https://dev.maxmind.com/geoip/geoip2/geolite2/
# You need GeoLite2-City-Blocks-IPv4.csv and GeoLite2-City-Locations-en.csv from GeoLite2 City
# license of the DB is CC-BY-SA 4.0
#
# This product includes GeoLite2 data created by MaxMind, available from
# http://www.maxmind.com

# usage: generate-ip-mappings.py > ip.csv
# in sqlite3 terminal:
#
# .mode csv
# .import `full path to ip.csv` ip_mapping
#

# For query by ip:
# SELECT * FROM ip_mapping WHERE `ip_start` <= ip-in-decimal AND `ip_end` >= ip-in-decimal ORDER BY `ip_start` DESC LIMIT 1;
import socket
import struct
import csv
import os
import sys
# import zipfile
# import urllib.request

CSV_WEB_LINK = 'http://geolite.maxmind.com/download/geoip/database/GeoLite2-City-CSV.zip'
CSV_FILE = 'GeoLite2-City-Blocks-IPv4.csv'
CSV_LOCATION = 'GeoLite2-City-Locations-en.csv'

if not os.path.exists(CSV_LOCATION):
    print("File = {} does not exist. Download it from = {} ".format(CSV_LOCATION, CSV_WEB_LINK))
    sys.exit(1)

COUNTRY_DICT = {}
with open(CSV_LOCATION, 'r') as csvfile:
    locationlist = csv.reader(csvfile, delimiter=',', quotechar='"')
    # Skip header
    next(locationlist)
    for row in locationlist:
        COUNTRY_DICT[row[0]] = row[4]

if not os.path.exists(CSV_FILE):
    print("File = {} does not exist. Download it from = {} ".format(CSV_FILE, CSV_WEB_LINK))
    sys.exit(1)

with open(CSV_FILE, 'r') as csvfile:
    iplist = csv.reader(csvfile, delimiter=',', quotechar='"')
    # Skip header
    next(iplist)
    for row in iplist:
        if row[7] is "" or row[8] is "":
            continue

        ip, net_bits = row[0].split('/')

        # Convert submask ip to range
        ip_start = (struct.unpack("!I", socket.inet_aton(ip)))[0]
        ip_end = ip_start + ((1 << (32 - int(net_bits))) - 1)

        latitude = float(row[7])
        longitude = float(row[8])
        country = COUNTRY_DICT.get(row[1], "")
        print('%d,%d,%f,%f,%s' % (ip_start, ip_end, latitude, longitude, country))
