#!/bin/sh

# Run this script from the root directory to get the latest translations from transifex :
# ./data/po/pull_from_transifex.sh

cd ./data/po
export PATH=$PATH:`pwd`

if [ -d "transifex/translations/supertuxkart.supertuxkartpot" ]; then
    cd transifex
else
    echo "==== Performing initial checkout ===="
    mkdir -p transifex
    cd transifex
    tx init
    tx set --auto-remote http://www.transifex.net/projects/p/supertuxkart/
fi

echo "==== Pulling all translations ===="
tx pull --all

echo "==== Copying files ===="
ls ./translations/supertuxkart.supertuxkartpot/*.po
cp -R ./translations/supertuxkart.supertuxkartpot/*.po ..

echo "==== Done ===="