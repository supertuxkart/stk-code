#!/bin/bash

set -e
set -x

BASE_DIR="$(realpath "$(dirname "$0")")"
SRC_DIR="$(dirname "$BASE_DIR")"
WEB_DIR="$BASE_DIR/web"
ASSETS_DIR="$(realpath "$1")"
ASSETS_SCRIPT="$SRC_DIR/android/generate_assets.sh"

LOW_QUALITY_DIR="$WEB_DIR/game/data_low"
MEDIUM_QUALITY_DIR="$WEB_DIR/game/data_mid"
HIGH_QUALITY_DIR="$WEB_DIR/game/data_high"

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

generate_dir() {
  local data_dir="$1"
  local output_path="$2"
  local texture_size="$3"
  ASSETS_PATHS="$ASSETS_DIR" OUTPUT_PATH="$data_dir" TEXTURE_SIZE="$texture_size" $ASSETS_SCRIPT
  (cd $data_dir/data && ./optimize_data.sh)
  pack_dir "$data_dir/data" "$output_path"
}

if [ ! "$ASSETS_DIR" ]; then
  echo "assets not found"
  exit 1
fi

generate_dir "$LOW_QUALITY_DIR" "$WEB_DIR/game/data_low.tar.gz" 256
generate_dir "$MEDIUM_QUALITY_DIR" "$WEB_DIR/game/data_mid.tar.gz" 512
generate_dir "$HIGH_QUALITY_DIR" "$WEB_DIR/game/data_high.tar.gz" 1024