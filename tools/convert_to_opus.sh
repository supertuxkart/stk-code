#!/bin/bash
# convert_to_opus.sh — Convert STK audio assets from OGG/Vorbis to Opus
# and update all XML references.
#
# Requires: opusenc (from opus-tools)
#
# Usage:
#   ./tools/convert_to_opus.sh <stk-assets-dir> [--bitrate KBPS] [--dry-run]
#
# Examples:
#   ./tools/convert_to_opus.sh ../stk-assets
#   ./tools/convert_to_opus.sh ../stk-assets --bitrate 48
#   ./tools/convert_to_opus.sh ../stk-assets --dry-run

set -euo pipefail

BITRATE=24
DRY_RUN=false
ASSETS_DIR=""

usage()
{
    echo "Usage: $0 <stk-assets-dir> [--bitrate KBPS] [--dry-run]"
    echo ""
    echo "  --bitrate KBPS   Opus encoding bitrate in kbps (default: 24)"
    echo "  --dry-run        Show what would be done without making changes"
    exit 1
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --bitrate)
            BITRATE="$2"
            shift 2
            ;;
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        --help|-h)
            usage
            ;;
        *)
            if [[ -z "$ASSETS_DIR" ]]; then
                ASSETS_DIR="$1"
            else
                echo "Error: unexpected argument '$1'"
                usage
            fi
            shift
            ;;
    esac
done

if [[ -z "$ASSETS_DIR" ]]; then
    echo "Error: stk-assets directory not specified"
    usage
fi

if [[ ! -d "$ASSETS_DIR" ]]; then
    echo "Error: '$ASSETS_DIR' is not a directory"
    exit 1
fi

if command -v ffmpeg &> /dev/null; then
    ENCODER="ffmpeg"
elif command -v opusenc &> /dev/null; then
    ENCODER="opusenc"
else
    echo "Error: neither ffmpeg nor opusenc found. Install one of:"
    echo "  brew install ffmpeg        (macOS, recommended)"
    echo "  brew install opus-tools    (macOS, WAV/FLAC input only)"
    echo "  apt install ffmpeg         (Debian/Ubuntu)"
    exit 1
fi

ASSETS_DIR="$(cd "$ASSETS_DIR" && pwd)"
JOBS=$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

xml_updated=0

log()
{
    echo "[convert_to_opus] $*"
}

# ── Step 1: Convert .ogg files to .opus (parallel) ──────────────────

TMPDIR_WORK=$(mktemp -d)
trap 'rm -rf "$TMPDIR_WORK"' EXIT

# Worker function called by xargs — runs in a subshell, writes results to files
convert_ogg_worker()
{
    local ogg_file="$1"
    local bitrate="$2"
    local assets_dir="$3"
    local results_dir="$4"
    local opus_file="${ogg_file%.ogg}.opus"
    local rel_path="${ogg_file#$assets_dir/}"
    local basename
    basename=$(basename "$ogg_file")

    if [[ -f "$opus_file" ]]; then
        echo "SKIP (exists): $rel_path"
        echo skip > "$results_dir/$basename.result"
        return
    fi

    local encoder="$5"
    local encode_ok=false

    if [[ "$encoder" == "ffmpeg" ]]; then
        ffmpeg -y -i "$ogg_file" -c:a libopus -b:a "${bitrate}k" "$opus_file" \
            -loglevel error 2>/dev/null && encode_ok=true
    else
        opusenc --quiet --bitrate "$bitrate" "$ogg_file" "$opus_file" \
            2>/dev/null && encode_ok=true
    fi

    if $encode_ok; then
        local ogg_size opus_size
        ogg_size=$(stat -f%z "$ogg_file" 2>/dev/null || stat -c%s "$ogg_file")
        opus_size=$(stat -f%z "$opus_file" 2>/dev/null || stat -c%s "$opus_file")
        local savings=$(( (ogg_size - opus_size) * 100 / ogg_size ))
        echo "[convert_to_opus] OK: $rel_path → ${rel_path%.ogg}.opus  (${savings}% smaller)"
        echo ok > "$results_dir/$basename.result"
    else
        echo "[convert_to_opus] FAIL: $rel_path"
        echo fail > "$results_dir/$basename.result"
    fi
}
export -f convert_ogg_worker

# Collect all .ogg files to convert
ogg_files=()
for ogg in "$ASSETS_DIR"/music/*.ogg "$ASSETS_DIR"/sfx/*.ogg; do
    [[ -f "$ogg" ]] || continue
    ogg_files+=("$ogg")
done

if $DRY_RUN; then
    log "Converting ${#ogg_files[@]} audio files (dry run)..."
    for ogg in "${ogg_files[@]}"; do
        rel_path="${ogg#$ASSETS_DIR/}"
        log "WOULD convert: $rel_path → ${rel_path%.ogg}.opus  (${BITRATE} kbps)"
    done
    converted=${#ogg_files[@]}
    skipped=0
    failed=0
else
    log "Converting ${#ogg_files[@]} audio files using $JOBS parallel jobs ($ENCODER)..."
    printf '%s\n' "${ogg_files[@]}" | \
        xargs -P "$JOBS" -I{} bash -c \
            'convert_ogg_worker "$@"' _ {} "$BITRATE" "$ASSETS_DIR" "$TMPDIR_WORK" "$ENCODER"

    # Tally results
    converted=$(grep -rl '^ok$' "$TMPDIR_WORK/" 2>/dev/null | wc -l | tr -d ' ' || echo 0)
    skipped=$(grep -rl '^skip$' "$TMPDIR_WORK/" 2>/dev/null | wc -l | tr -d ' ' || echo 0)
    failed=$(grep -rl '^fail$' "$TMPDIR_WORK/" 2>/dev/null | wc -l | tr -d ' ' || echo 0)
fi

# ── Step 2: Update .music XML files ─────────────────────────────────

log "Updating .music XML files..."
for music_file in "$ASSETS_DIR"/music/*.music; do
    [[ -f "$music_file" ]] || continue
    rel_path="${music_file#$ASSETS_DIR/}"

    if ! grep -q '\.ogg' "$music_file"; then
        continue
    fi

    if $DRY_RUN; then
        log "WOULD update: $rel_path"
        ((xml_updated++)) || true
        continue
    fi

    # Replace .ogg with .opus in file= and fast-filename= attributes
    sed -i.bak 's/\.ogg"/\.opus"/g' "$music_file"
    rm -f "${music_file}.bak"
    log "UPDATED: $rel_path"
    ((xml_updated++)) || true
done

# ── Step 3: Update sfx/sfx.xml ──────────────────────────────────────

SFX_XML="$ASSETS_DIR/sfx/sfx.xml"
if [[ -f "$SFX_XML" ]] && grep -q '\.ogg' "$SFX_XML"; then
    rel_path="sfx/sfx.xml"
    if $DRY_RUN; then
        log "WOULD update: $rel_path"
        ((xml_updated++)) || true
    else
        sed -i.bak 's/\.ogg"/\.opus"/g' "$SFX_XML"
        rm -f "${SFX_XML}.bak"
        log "UPDATED: $rel_path"
        ((xml_updated++)) || true
    fi
fi

# ── Step 4: Update track scene.xml sfx-emitter references ───────────

log "Updating track sfx-emitter references..."
while IFS= read -r scene_file; do
    rel_path="${scene_file#$ASSETS_DIR/}"

    if $DRY_RUN; then
        log "WOULD update: $rel_path"
        ((xml_updated++)) || true
        continue
    fi

    # Only replace .ogg in sound= attributes (sfx-emitters)
    sed -i.bak 's/sound="\([^"]*\)\.ogg"/sound="\1.opus"/g' "$scene_file"
    rm -f "${scene_file}.bak"
    log "UPDATED: $rel_path"
    ((xml_updated++)) || true
done < <(grep -rl 'sound="[^"]*\.ogg"' "$ASSETS_DIR/tracks/" --include="*.xml" 2>/dev/null || true)

# ── Summary ──────────────────────────────────────────────────────────

echo ""
log "════════════════════════════════════════"
log "  Audio files converted: $converted"
log "  Audio files skipped:   $skipped"
log "  Audio files failed:    $failed"
log "  XML files updated:     $xml_updated"
log "  Bitrate:               ${BITRATE} kbps"
if $DRY_RUN; then
    log "  (dry run — no changes made)"
fi
log "════════════════════════════════════════"
