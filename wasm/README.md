# SuperTuxKart WASM Port

Currently this port is still incomplete.

Working features:
- The game launches
- OpenGL ES 2 graphics (GLES 3 doesn't work)
- Audio
- Persistent user data
- Caching data and asset files in IndexedDB

TODO:
- Fix GLES 3 (maybe someone with more experience in webgl could help here)
- Webpage theming
- Better compression for assets
- Lazy load assets during gameplay
- Networking

Caveats:
- The RAM usage is rather high because the assets are always loaded in memory
- The performance isn't great, probably because the legacy renderer is still being used
- There is a large ~700MB download required immediately
- Some options, like anything related to online multiplayer, may hang the game

## Building
1. First, get a copy of the emsdk (it might help to have emscripten already installed with `sudo apt install emscripten`):
```
wasm/get_emsdk.sh
```
2. Compile all the dependencies:
```
wasm/build_deps.sh
```
3. Compile STK:
```
wasm/build.sh
```
4. Compress the assets using the script from the Android port:
```
sudo apt install imagemagick vorbis-tools pngquant advancecomp libjpeg-progs optipng
android/generate_assets.sh
(cd android/assets/data && ./optimize_data.sh)
```
5. Bundle the game data and assets:
```
wasm/pack_assets.sh android/assets/data/
```
6. Host a web server:
```
(cd wasm/web && python3 ../run_server.py)
```

## Project Structure:
- /wasm/build - Files for building the dependencies
- /wasm/prefix - Headers and library files
- /wasm/web - Web server root directory
- /wasm/emsdk - Emscripten SDK
- /wasm/fragments - Patches for emscripten's generated JS