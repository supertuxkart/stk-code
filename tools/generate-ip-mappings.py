#!/usr/bin/env python3
# usage: generate-ip-mappings.py
# 2 files ipv4.csv and ipv6.csv will be generated
# in sqlite3 terminal:
#
# .mode csv
# .import `full path to ipv4.csv` ip_mapping
# .import `full path to ipv6.csv` ipv6_mapping
#

# For query by ip:
# SELECT * FROM ip_mapping WHERE `ip_start` <= ip-in-decimal AND `ip_end` >= ip-in-decimal ORDER BY `ip_start` DESC LIMIT 1;
# SELECT * FROM ipv6_mapping WHERE `ip_start` <= upperIPv6("ipv6_addr") AND `ip_end` >= upperIPv6("ipv6_addr") ORDER BY `ip_start` DESC LIMIT 1;
import socket
import struct
import csv
import os
import sys
# import zipfile
# import urllib.request

def ip2int(addr):
    return struct.unpack("!I", socket.inet_aton(addr))[0]

# Keep only the upper 64bit, as we only need that for geolocation
def ipv62int64(addr):
    hi, lo = struct.unpack('!QQ', socket.inet_pton(socket.AF_INET6, addr))
    return hi

CSV_WEB_LINK = 'https://download.db-ip.com/free/dbip-city-lite-2020-01.csv.gz'
CSV_FILE = 'dbip-city-lite-2020-01.csv'

if not os.path.exists(CSV_FILE):
    print("File = {} does not exist. Download it from = {} ".format(CSV_FILE, CSV_WEB_LINK))
    sys.exit(1)

# Format: 1.0.0.0,1.0.0.255,OC,AU,Queensland,"South Brisbane",-27.4748,153.017
with open(CSV_FILE, 'r') as csvfile, open('ipv4.csv', 'w') as ipv4, open('ipv6.csv', 'w') as ipv6:
    iplist = csv.reader(csvfile, delimiter=',', quotechar='"')
    for row in iplist:
        # Skip reserved range
        if row[3] == "ZZ":
            continue
        # Skip empty latitude and longitude
        if row[6] is "" or row[7] is "":
            continue

        if row[0].find(':') == -1:
            ipv4_line = True
        else:
            ipv4_line = False

        if ipv4_line:
            ip_start = ip2int(row[0])
            ip_end = ip2int(row[1])
        else:
            ip_start = ipv62int64(row[0])
            ip_end = ipv62int64(row[1])

        # Some IPv6 entries are duplicated after removing the lower 64bit
        if ip_start == ip_end:
            continue

        # Sqlite doesn't support unsigned int 64
        _int64_max = pow(2, 63) - 1
        if ip_start > _int64_max or ip_end > _int64_max:
            continue
        latitude = float(row[6])
        longitude = float(row[7])
        country = row[3]
        if ipv4_line:
            print('%d,%d,%f,%f,%s' % (ip_start, ip_end, latitude, longitude, country), file = ipv4)
        else:
            print('%d,%d,%f,%f,%s' % (ip_start, ip_end, latitude, longitude, country), file = ipv6)
