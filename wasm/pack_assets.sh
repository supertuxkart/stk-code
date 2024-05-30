#!/bin/bash

set -e
set -x

BASE_DIR="$(realpath "$(dirname "$0")")"
SRC_DIR="$(dirname "$BASE_DIR")"
WEB_DIR="$BASE_DIR/web"

if [ ! "$1" ]; then
  echo "you must specify the assets directory"
  exit 1
fi

ASSETS_DIR="$(realpath "$1")"

tar -cf - -C "$SRC_DIR/data" . | gzip -9 - > "$WEB_DIR/game/data.tar.gz"
tar -cf - -C "$ASSETS_DIR" . | gzip -9 - > "$WEB_DIR/game/assets.tar.gz"