# Opus Audio Support

SuperTuxKart optionally supports the [Opus audio codec](https://opus-codec.org/)
for both music streaming and sound effects. Opus achieves equivalent quality at
roughly 40% smaller file sizes compared to Vorbis (24 kbps Opus vs 42 kbps Vorbis).

Opus is BSD 3-clause licensed (IETF RFC 6716) and royalty-free.

## Building with Opus

Opus support requires the `libopusfile` development library.

### Install dependencies

```bash
# macOS
brew install opusfile

# Debian/Ubuntu
sudo apt install libopusfile-dev

# Fedora
sudo dnf install opusfile-devel

# Arch
sudo pacman -S opusfile
```

### CMake flag

Opus support is controlled by the `HAVE_OPUS` CMake option (default: `OFF`).

```bash
cmake .. -DHAVE_OPUS=ON
```

When enabled, the build will fail if `libopusfile` is not found. When disabled,
all Opus code is excluded via `#ifdef HAVE_OPUS` and the build is identical to
upstream.

### Behaviour at runtime

When `HAVE_OPUS` is enabled, the engine accepts both `.ogg` (Vorbis) and `.opus`
files. The codec is selected at load time based on the file extension. Vorbis
remains the default; Opus is purely additive.

## Converting assets to Opus

A conversion script is provided at `tools/convert_to_opus.sh`. It converts all
OGG/Vorbis audio in an `stk-assets` directory to Opus and updates the XML
references.

### Requirements

- `ffmpeg` (recommended) or `opusenc` (from `opus-tools`, WAV/FLAC input only)

### Usage

```bash
# Dry run (shows what would change, no modifications)
./tools/convert_to_opus.sh /path/to/stk-assets --dry-run

# Convert at 24 kbps (default)
./tools/convert_to_opus.sh /path/to/stk-assets

# Convert at a different bitrate
./tools/convert_to_opus.sh /path/to/stk-assets --bitrate 48
```

The script:

1. Converts all `.ogg` files in `music/` and `sfx/` to `.opus` (parallel, using all CPU cores)
2. Updates `.music` XML files (`file=` and `fast-filename=` attributes)
3. Updates `sfx/sfx.xml` (`filename=` attributes)
4. Updates track `scene.xml` files (`sound=` attributes on sfx-emitters)

Existing `.opus` files are skipped, so the script is safe to re-run.
