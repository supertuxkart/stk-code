#!/usr/bin/env python3
"""
Usage: ./tools/update_google_play_listings.py /path/to/account_file.json
Pass --beta at the end to generate listings for beta version of stk
"""

import sys
import os
from google.auth.transport.requests import Request
from google.oauth2 import service_account
import googleapiclient.discovery

# List of google play supported locale, this dict allow conversion from po file
lang_dict = {
'af': 'af', # Afrikaans
'sq': 'sq', # Albanian
'am': 'am', # Amharic
'ar': 'ar', # Arabic
'hy': 'hy-AM', # Armenian
'az': 'az-AZ', # Azerbaijani
'bn': 'bn-BD', # Bangla
'eu': 'eu-ES', # Basque
'be': 'be', # Belarusian
'bg': 'bg', # Bulgarian
'my': 'my-MM', # Burmese
'ca': 'ca', # Catalan
'zh_HK': 'zh-HK', # Chinese (Hong Kong)
'zh_CN': 'zh-CN', # Chinese (Simplified)
'zh_TW': 'zh-TW', # Chinese (Traditional)
'hr': 'hr', # Croatian
'cs': 'cs-CZ', # Czech
'da': 'da-DK', # Danish
'nl': 'nl-NL', # Dutch
'en': 'en-US', # English
'et': 'et', # Estonian
'fil': 'fil', # Filipino
'fi': 'fi-FI', # Finnish
'fr_CA': 'fr-CA', # French (Canada)
'fr': 'fr-FR', # French (France)
'gl': 'gl-ES', # Galician
'ka': 'ka-GE', # Georgian
'de': 'de-DE', # German
'el': 'el-GR', # Greek
'gu': 'gu', # Gujarati
'he': 'iw-IL', # Hebrew
'hi': 'hi-IN', # Hindi
'hu': 'hu-HU', # Hungarian
'is': 'is-IS', # Icelandic
'id': 'id', # Indonesian
'it': 'it-IT', # Italian
'ja': 'ja-JP', # Japanese
'kn': 'kn-IN', # Kannada
'kk': 'kk', # Kazakh
'km': 'km-KH', # Khmer
'ko': 'ko-KR', # Korean
'ky': 'ky-KG', # Kyrgyz
'lo': 'lo-LA', # Lao
'lv': 'lv', # Latvian
'lt': 'lt', # Lithuanian
'mk': 'mk-MK', # Macedonian
'ms': 'ms', # Malay
'ml': 'ml-IN', # Malayalam
'mr': 'mr-IN', # Marathi
'mn': 'mn-MN', # Mongolian
'ne': 'ne-NP', # Nepali
'no': 'no-NO', # Norwegian
'fa': 'fa', # Persian
'pl': 'pl-PL', # Polish
'pt_BR': 'pt-BR', # Portuguese (Brazil)
'pt': 'pt-PT', # Portuguese (Portugal)
'pa': 'pa', # Punjabi
'ro': 'ro', # Romanian
'rm': 'rm', # Romansh
'ru': 'ru-RU', # Russian
'sr': 'sr', # Serbian
'si': 'si-LK', # Sinhala
'sk': 'sk', # Slovak
'sl': 'sl', # Slovenian
'es': 'es-ES', # Spanish (Spain)
'sw': 'sw', # Swahili
'sv': 'sv-SE', # Swedish
'ta': 'ta-IN', # Tamil
'te': 'te-IN', # Telugu
'th': 'th', # Thai
'tr': 'tr-TR', # Turkish
'uk': 'uk', # Ukrainian
'ur': 'ur', # Urdu
'vi': 'vi', # Vietnamese
'zu': 'zu', # Zulu
}

package = 'org.supertuxkart.stk'
account_file = sys.argv[1]
is_beta = False
if len(sys.argv) == 3 and sys.argv[2] == '--beta':
    package += '_beta'
    is_beta = True

SCOPES = ['https://www.googleapis.com/auth/androidpublisher']
credentials = service_account.Credentials.from_service_account_file(
    account_file, scopes = SCOPES)
credentials.refresh(Request())
from googleapiclient.discovery import build
service = build('androidpublisher', 'v3', credentials = credentials)

edit_request = service.edits().insert(body = {}, packageName = package)
result = edit_request.execute()
edit_id = result['id']

for lang in os.listdir('./google_play_msg'):
    if not lang in lang_dict:
        continue
    language_name = lang_dict[lang]
    print('Updating', language_name)
    listing_response = service.edits().listings().update(
        editId = edit_id,
        language = language_name,
        packageName = package,
        body = {
        'language': language_name,
        'title': 'SuperTuxKart Beta' if is_beta else 'SuperTuxKart',
        'fullDescription':
            open('./google_play_msg/' + lang + ('/full_beta.txt'
            if is_beta else '/full.txt'), 'r').read(),
        'shortDescription':
            open('./google_play_msg/' + lang + '/short.txt', 'r').read(),
        }).execute()
commit_request = service.edits().commit(
    editId = edit_id, packageName = package).execute()
