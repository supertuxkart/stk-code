# Browser port verification

Tested locally on 2026-11-15 using Emscripten **4.0.10**, CMake/Ninja, and
headless Chrome **157.0.8103.51** on macOS. Chrome used SwiftShader for the
automated rendering checks; this does not establish hardware-GPU performance.

## Checks completed

- Compiled and linked the actual C++ game and bundled engine libraries to Wasm.
- Served the generated build at `http://localhost:8080` with the isolation headers.
- Launched through the page's Play button and rendered the Create User screen.
- Created an offline `web_user` profile and reached the player Login screen.
- Reloaded the page and confirmed restoration of the seven generated profile,
  configuration, input, highscore, avatar, and log files from IndexedDB.
- Started Oliver's Math Class with Tux through the game's quick-race options.
- Rendered an active race, held Arrow Up for six seconds, and observed the kart
  move along the track.
- Pressed Escape and rendered the pause menu.
- Passed the HTTP integration test for `.wasm` MIME type and COOP/COEP headers.
- Confirmed that native diagnostic messages reach the page's expandable game log.

## Quit and replay regression

`node web/test_replay.mjs` passed three consecutive native game startups and
quits in one launcher page. The second session entered fullscreen before quitting.
Each quit exited fullscreen, hid the terminated game, and enabled **Play again**
after preparing a fresh runtime. The launcher did not navigate or reload, and
there were no uncaught browser errors. A filesystem write made immediately before
each quit was restored in the following session, checking the final save flush.

The test clicks the actual Exit Game button on the Create User screen in a fresh
browser context; it does not simulate the runtime's quit callback. This check
complements the earlier race-driving/pause checks and does not cover every route
to quit, browser crashes, or forcibly closing the tab.

The local test media was the official **1.5-rc1 low-quality mobile pack**, merged
with this repository's data. Its checksum is recorded in [README.md](README.md).

## Limits of these checks

This verifies startup, rendering, basic driving, pausing, and persistent browser
storage. It does not certify every track/game mode, finishing a full race,
gamepad input, audible sound, mobile browsers, or other browsers. The original
native targets were not rebuilt or run during this work.

The initial browser target intentionally excludes online gameplay, add-on
downloads, native recording, and Web3 integration. Chrome emitted WebGL
performance warnings under software rendering; no frame-rate guarantee is made.
The game also logs legacy-renderer/version warnings and skips the color emoji
font with this FreeType port. The tested menu text renders, but color emoji
support remains a limitation of this build.

## Reproduce the local run

```sh
python3 web/build.py
python3 web/serve.py
```

Open `http://localhost:8080`, click Play, create an offline player if needed,
confirm the player, and choose Single Player. Build output and test screenshots
are generated locally and remain ignored by Git.
