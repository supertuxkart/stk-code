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

create_manifest() {
  local path="$1"
  local file_name="$(basename "$path")"
  local data_dir="$(dirname "$path")"
  local size="$(du -b "$path" | cut -f1)"
  local chunks="$(find "$data_dir" -name "$file_name.*" | sort)"

  echo "$size"
  for chunk in $chunks; do
    local chunk_name="$(basename "$chunk")"
    local ending="$(echo "$chunk_name" | rev | cut -d'.' -f1 | rev)"
    if [ "$ending" = "manifest" ]; then
      continue
    fi
    echo "$chunk_name"
  done
}

pack_dir() {
  local source_dir="$1"
  local out_path="$2"
  tar -cf - -C "$source_dir" . | gzip -9 - > "$out_path"
  split -b 20m --numeric-suffixes "$out_path" "$out_path."
  create_manifest "$out_path" > "$out_path.manifest"
  rm "$out_path"
}

pack_dir "$SRC_DIR/data" "$WEB_DIR/game/data.tar.gz"
pack_dir "$ASSETS_DIR" "$WEB_DIR/game/assets.tar.gz"