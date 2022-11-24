#!/usr/bin/env python
# usage: update_translation.py [transifex token]
import os
import requests
import sys
import threading
import time

text_headers = {
    "accept": "*/*",
    "content-type": "text/plain",
    "authorization": "Bearer " + sys.argv[1]
}

json_headers = {
    "accept": "application/vnd.api+json",
    "content-type": "application/vnd.api+json",
    "authorization": "Bearer " + sys.argv[1]
}

def get_translation(lang):
    payload = '{\"data\":{\"attributes\":{\"callback_url\":null,\"content_encoding\":\"text\",\"file_type\":\"default\",\"mode\":\"default\",\"pseudo\":false},\"relationships\":{\"language\":{\"data\":{\"type\":\"languages\",\"id\":\"l:' + lang + '\"}},\"resource\":{\"data\":{\"type\":\"resources\",\"id\":\"o:supertuxkart:p:supertuxkart:r:supertuxkartpot\"}}},\"type\":\"resource_translations_async_downloads\"}}'
    url = "https://rest.api.transifex.com/resource_translations_async_downloads"
    response = requests.post(url, data=payload, headers=json_headers)
    if not "data" in response.json().keys():
        return

    if response.status_code == 202:
        location = response.headers["Content-Location"]
        if location:
            po_file = open(lang + ".po", "w")
            po_file.write(requests.get(location, headers=text_headers).text)
            po_file.close()
            print("Saving " + lang + ".po",)
            return

    for t in range(0, 10):
        response = requests.get(url + "/" + response.json()["data"]["id"], headers=json_headers)
        if "application/json" in response.headers.get("Content-Type", ""):
            json = response.json()
            status = ""
            if "data" in json.keys():
                status = json["data"]["attributes"]["status"]
            if status == "failed" or t == 9:
                print(lang + " failed to download")
                break
            else:
                time.sleep(1)
        else:
            po_file = open(lang + ".po", "w")
            po_file.write(response.text)
            po_file.close()
            print("Saving " + lang + ".po",)
            break

threads = []
for f in os.listdir("./"):
    if not f.endswith(".po"):
        continue
    lang = os.path.splitext(f)[0]
    threads.append(threading.Thread(target=get_translation, args=[lang]))

for t in threads:
    t.start()
for t in threads:
    t.join()
