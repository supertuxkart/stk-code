# Changelog
This file documents notable changes to SuperTuxKart across versions since its inception.

It should be kept in mind that some versions have a less complete changelog than others, and that this changelog does not list the details of the many small bugfixes and improvements which together make a significant part of the progress between releases.

For similar reasons, and because some features are vastly more complex than others, attributions of main changes should not be taken as a shortcut for overall contribution.

## SuperTuxKart 1.5

### Networking
* Improve the track-voting logic when no majority is achieved, by kimden
* Prevent tracks missed by spectators from limiting the choice of active players, by kimden
* Make bot indices start from one, by JipFr
* Various bugfixes, by kimden

### Gameplay
* Fix start positions for negative sideward distances, by kimden
* Make the rescue bird place the kart towards the ball in soccer mode, by Snoker101

### General
* Make the game's window resizable in all menu screens, by Benau and CodingJellyfish
* New benchmark mode, by Alayan:
- Can be run with a few clicks, allowing to easily test the performance of various settings or to compare different systems
- Robust performance metrics that better reflect the impact of varying frametimes than Average FPS and 1% Lows.
- Results are displayed along the active graphics settings, and detailed results can be saved to file
* Add the benchmark mode to commandline options, by ektor5
* Fix incorrect unlock information in Story Mode after a Grand Prix, by CodingJellyfish
* Make the progression of audio levels geometrical and increase default steps, allowing to set lower audio levels and better accuracy for low audio levels (especially useful for headphone users), by Alayan
* Fix drive-on sound from materials being played when the game is paused, by Alayan
* Fix a crash trying to read replays when the random starting position setting is enabled, by Alayan
* Handle track names with spaces in the replay reader, by Alayan
* Enable smooth scrolling for Irrlicht, by CodingJellyfish
* Add launchable tag and use rDNS format for AppData file, by AsciiWolf
* Fix the last lap music sometimes failing to play, and improvements in the last lap music transition, by Alayan
* Tweaks to the camera, by CodingJellyfish
* Various build system updates, by deveee, tobbi, ognevny and others
* Various compiler fixes, by heirecka, limburgher, nyllet and others
* Substantial changes improving code quality, by Alayan and kimden
* Update Wiiuse library to 0.15.6, SIMD-e to 0.8.2, MojoAL to latest (a9e2f30)
* Various tweaks, bugfixes and code-quality improvements

### Graphics
* Improve the accuracy of the framerate limiter, by Benau
* Add more maximum framerate options to the built-in framerate limiter, by Benau
* Add some graphical effects for legacy video drivers, by Benau
* Improved animations for the parachute and bubblegum shield, by Semphris
* Replace inaccurate normal compression algorithm with Octahedron Normal Vector, by CodingJellyfish
* Fix the Screen-Space Reflection shader, by CodingJellyfish
* Fix and improve the Screen-Space Ambiant Occlusion shader, by CodingJellyfish
* Implement Percentage-Closer Soft Shadows, by CodingJellyfish
* Guarantee an overall bone limit of 2048 for skinned mesh (up from 1024), by CodingJellyfish
* Improve Cascaded Shadow Mapping, by CodingJellyfish
* Improve the performance of scene node iteration, by CodingJellyfish
* Various improvements to the automatic computations of Level of Detail (LoD) distances, by Alayan
* Enable new higher LoD and shadows settings, by Alayan
* Integrate LoD (Geometry Detail) settings in the graphics presets, and add a 7th graphics preset, by Alayan
* Prefer displaying a lower quality LoD model over switching to a higher quality one when too close, by Alayan
* Allow to run the game's render resolution at higher than native (up to 200%), by Alayan
* Remove the distance limit on the display of on-track items (such as gift boxes), by Alayan
* Improve image quality on low and medium presets with better anisotropic filtering, by Alayan
* Various bugfixes and improvements, by zmike, Icenowy and others

### User Interface
* Allow users to select favorite karts/tracks/arenas, by Alayan and CodingJellyfish
* Five new skin variants for Cartoon, and a new Desert skin variant for Classic, by Alayan and CrystalDaEevee
* Improve the skin selection UX by separating "base theme" and "skin variant" selection, by Alayan
* Add a new Display tab in the Settings, by Alayan
* Allow users to search karts/arenas, by CodingJellyfish
* Allow users to group karts by kart classes, by CodingJellyfish
* Allow to rate addons with a keyboard or a controller, and notify when trying to rate an addon while not logged in, by CodingJellyfish
* Improve the typing bars, especially for the coal theme, by Alayan
* Implement a blog announcement system in the Online screen, by CodingJellyfish
* Various UI layout improvements (especially for 'tall' resolutions), by CodingJellyfish
* Generate higher resolution texture for scalable fonts, by CodingJellyfish
* Various enhancements, by QwertyChouskie, Nomagno, Nstelt and others



#### In-race UI
* Add color and sound indicators when an elimination is about to happen in Follow-The-Leader, by Alayan
* Correctly display the remaining time in FtL when extra-time is added, by Alayan
* Show score with color on the center of speedometer in battles, by CodingJellyfish
* Display correctly themed attachment icons if the base theme has been changed before the last restart, by CodingJellyfish

### Tracks and modeling
* New music for Las Dunas Arena/Las Dunas Soccer, by ALBatross
* Update Godette face texture, by ZAQraven99
* Fix Northern Resort skybox, by CrystalDaEevee
* Various cut/checkline fixes, by CrystalDaEevee

### Mobile
* Don't keep the rescue button active after it stops being touched, when the finger keeps touching the screen (e. g. to handle the steering wheel), by S0nter

## SuperTuxKart 1.4 (31. October 2022)
### General
* Enable ARMv7 build for Windows, by Benau
* Restore macOS <= 10.14 support, by Benau
* Allow setting the auto-center behavior of steering wheels, by Mstrodl
* Simplify making derivative UI skins by allowing to specify a base theme, by qwertychouskie
* Make sure old save data is only removed after new save data is written, to avoid data losses on full drives, by Benau
* Fixed camera rotation when using the gyroscope and driving on vertical surfaces, by Benau
* Lap trial mode, by mrkubax10
* Fix a parachute powerup bug, whereby karts behind the user would still lose their shield, by heuchi
* Add an option to randomize the player starting position, by Iwoithe
* Avoid triggering other goal lines when the goal is already scored, by kimden
* And some other minor bugfixes and enhancements too small or specific to be detailed

### Graphics
* Beta Vulkan renderer, by Benau
* Add an animation to the stars displayed after a kart gets hit, by Semphris
* Add intuitive animations for the respawn of on-track items (such as gift boxes), by Semphris
* LOD optimization, by Benau
* Implement HiDPI support in SDL2 properly, by Benau
* Increase the use of on-demand loading for textures, by Benau
* Make sky particle always fall vertically (instead of perpendicularly to the player camera), by Benau

### Tracks and modeling
* Updated Konqi, by ZAQraven99
* New Godette kart, by ZAQraven99
* New Hole Drop Soccer Field, by CrystalDaEevee, music by DernisNW
* New Oasis Soccer Field, by CrystalDaEevee
* Updated Battle Island and Cave X, by Typhon306
* Fix broken invisible wall in Antediluvian Abyss, by Benau
* New textures in Shifting Sands, by KartOym
* Balanced starting positions in all official soccer fields, by CrystalDaEevee

### Networking
* Make the in-server and in-game player limits independent, allowing extra slots for spectators, by Waldlaubsaengernest
* Allow using real addon karts (same hitbox and kart type as in local game), by Benau
* Sort the server list by number of real players in servers (ignoring AIs), by Benau

### User Interface
* Display per-kart difficulty in the end-screen for replays, by ldoyenard
* Add track searching to the network track screen, by Benau
* Minor enhancements and fixes in the end race screen

## SuperTuxKart 1.3 (28. September 2021)
### Networking
* Server bookmarks, by Benau
* Background download of addon packs, by Benau

### Graphics
* Introduce render resolution scaling for the modern renderer, by QwertyChouskie and Deve. For users with limited GPU power, this allows to get significant performance (FPS) gains at the cost of image quality. It can also allow additional graphics effects at the same performance. This is especially useful for users with high-resolution and high-DPI screens. The scaling only affects the 3D scene, the UI remains crisp at full-resolution.
* Simplification through removal of unused deprecated graphical effects, by Samuncle
* Many updates to texture-related code, by Benau
* Auto-compute Level of Detail distances for 3D models set to use LOD depending on track complexity, by Samuncle. The geometry level parameter allows partial control over the drawing distances.
* Improved screen space reflection, by QwertyChouskie

### General
* Switch port, by Mary
* Gamepad force feedback support, by Mary
* Highscore screen, by RQWorldblender
* Grand Prix highscores, by mrkubax10
* Updates to the bundled tinygettext library, by Benau
* Enable custom skidding sounds for karts, by Benau
* Greatly sped up loading of tracks with many checklines through optimization, by heuchi
* And many bugfixes and enhancements too small or specific to be detailed

### User Interface
* Display the skin-appropriate iconset without having to restart, by Benau
* Fix some issues with the options menu when used during a pause, by Benau
* Add a confirmation dialog before closing the game through the "go back" key/button, by Benau
* In the result screen, display the race position with a number with 10 karts or more, by mrkubax10
* In the result screen, display the challenge result and the requirements met or failed, by CodingJellyFish
* Many small menu and dialogs improvements, by RQWorldblender and others
* Usability and functionality improvements to the debug menu, by RQWorldblender
* Clickable URLs in text, by Benau
* Add a rainbow background to the color picker, making it more intuitive, by riso

#### In-race UI
* Add visual and sound feedback when a timed challenge or timed game is about to end, by mrkubax10 and Alayan

### Mobile
* Fix a precision issue that caused artefacted shadows, by icecream95 and Benau
#### iOS
* Use MojoAL instead of OpenAL, which iOS currently has issues with, by Benau

### Tracks and modeling
* Alien Signal, by Samuncle
* Ancient Colosseum Labyrinth, by Typhon306
* Improved Las Dunas Soccer, by Benau
* Add lap line extensions to Hacienda, Old Mine, Ravenbridge Mansion and Shifting Sands, by Benau
* New Pepper kart, by ZAQraven99
* Improved Adiumy, Emule, GNU and Sara karts, by ZAQraven99

## SuperTuxKart 1.2 (27. August 2020)

### Networking
* Add support for IPv6 LAN servers, by Benau
* Auto-detect client IPv4 and IPv6 support, by Benau
* Improved server creation speed and performance, by Benau
* Allow to use any addon karts online, on servers with live join enabled, by Benau. The kart has Tux's hitbox.
* Prevent undefined behaviour on battle/soccer servers with an excessive player limit, by Benau
* Use an improved rating formula for ranked play, by Alayan

### General
* Use SDL2 for window creation, allowing better gamepad support, by Benau
* Replace Fribidi by SheeBidi for RTL support, and remove the dependency to libraqm, by Benau
* Add support for in-race messages in scripting, by CodedOre
* Improved tutorial with free-flow racing (no more text interrupts), by CodedOre
* Haiku support, by AlwaysLivid
* Make the game's window resizable in-game, by Benau
* Download addon icons on-demand to speed-up the addons list loading, by Benau
* Allow theming of icons, by QwertyChouskie
* Many many bugfixes and enhancements

### Mobile
* Many minor bugfixes and compatibility enhancements
#### Android
* Use the android app bundle, allowing all tracks to be included, by Benau
* Use the native progress indicator during game data extraction, by Benau
* Add a launch splash screen, by Benau
#### iOS
* Allow server creation in-game by using only one process for client and server, by Benau

### User Interface
* Introduce element padding for skins, by QwertyChouskie
* New Cartoon skin, by LCP (art) and QwertyChouskie (code)
* Allow to rename control configurations, by GuillaumeBft
* Unthrottled keyboard navigations in menus, by Benau
* Show mobile players as such in server lobbies, using an emoji icon, by riso
* Always show the race end times in ms, by Fouks
* SVG icons support, by riso
* New tips for soccer mode
#### In-race UI
* Introduce camera settings in the options screen, by luffah
* More spectator cameras in online spectating, by luffah
* Add a minimap indicator for basketballs, by Alayan
* Add team chat for team games, by Benau
* Show the kart orientation on the soccer minimap, by riso
* Show a message once a goal is made in soccer, by riso

### Tracks and modeling
#### Karts
* New version of Kiki, by Typhon306 and ZAQraven99
* Improved karts, by CrystalDaEevee
    * Pidgin, Puffy
* Improved beastie animation, by D_ft Kid

## SuperTuxKart 1.1 (4. January 2020)

### Networking
* Add support for IPv6 clients and servers, by Benau
* Add AI support to local networking servers, by Benau. This AI is not as strong as the normal AI due to networking lag, but should offer enough challenge for a majority of players.
* Add support for AI bots for the server host, by Benau
* Improved handling of collisions, by Benau
* Improved support for playing addons on servers, by Benau
* Support using an IPV4 domain name to connect to a server, by Benau
* Add packet loss data to the server databases, by Benau
* New management features for server hosts, by Benau
    * IP / online ID ban tables
    * Server statistics
* Make the auto-end finish time more lenient, so it doesn't tank ranking score because of another player, by Alayan
* Add a player reporting function, by Benau
* Fix a bug that allowed to exceed a server's player limit if two players connected at the same time, by Benau
* Add lobby commands, by Benau

### Race gameplay and physics
* Random spawn point in local battle mode, by Aleman778
* Add an option to show everyone's items with the left characters icons, by Fouks
* Allow to choose the number of AI per team in soccer, by risostk
* In CTF mode, fix a bug that prevented scoring a point when the team's flag was in its immunity state, by Alayan
* Fix a bug that could result in the finish time being shown as 0:00:00, by Alayan
* Fix a bug that made AIs, in multiplayer and low difficulty, rubber-band towards the wrong player, by Alayan. Each player now correctly has its AIs, helping a weaker player to not be condemned to the last position.

### General
* Screen space reflection graphical effect, by samuncle
* Support unicode file paths, by Benau
* Complex text layout support, by Benau
* Add emoji support, by Benau
* Avoid "this program is not responding" warnings in Windows and Mac, by Benau
* Add a Story Mode timer, by Alayan
* Make text billboards size consistent accross resolutions, by Benau
* Don't trigger road sounds when the kart is flying, by Benau
* Remove an incorrect function call that could significantly hurt performance for Mac, by Benau
* Fix a nasty bug that made several sound effects to be played at a low volume after a crash, by Alayan
* Some code clean-up, by Alayan
* Addon skin support and skin-specified font by Benau
* Structure for allowing icon themes by Benau
* Add support for custom per-kart engine sounds, by Benau
* Fix an incorrect outdated driver warning for recent AMD cards on Windows, by Alayan
* Scripting enhancements, by deveee
* Add a continue button to cutscenes (mostly useful for touchscreen devices), by deveee
* Request the use of the performance GPU on Optimus systems, by deveee
* Fixed plural forms for translations in several languages, by deveee
* Close STK if Wayland crashes, by mstoeckl
* Support nested directories for addons, by Benau
* Scripting preprocessing for STK version, by Benau
* Updated Wiiuse library to 0.15.5
* Various tweaks and bugfixes, including crash fixes

### Mobile
* Add a screen allowing to download official assets that can't be included in the release package because of size limits, by Benau
* The tutorial text is now tailored for touchscreen usage, instead of the main keyboard text, by Benau
* Enhanced acceleration handle, by Benau
* Allow changing the touchscreen controller type in race paused dialog, by deveee
* Allow for optional auto-acceleration, by deveee
#### Android
* Various bugfixes by deveee, dos1
* Screen keyboard handling improvements, by deveee and Benau
* Allow copying text from the edit box, by Benau
* STK for Android now targets the version 29 of the API (Android 10.0), up from 26 (Android 8.0)
#### iOS
* Add iOS support, by Benau
    * The Android code has been widely put to use for iOS
    * Several changes or fixes to ensure proper behavior (input, screen-scaling, and more)
    
### User Interface
* Show tips for players when loading and after race ends, by CodingJellyfish
* Better scaling of many many UI elements to large resolutions, by CodingJellyfish and others
* Show country flags for servers and players in online multiplayer, by Benau
* Add a new option to change font size on the fly, by Benau and deveee
* Add icons for the available options in the race result screens, by CodingJellyfish
* Make the highscore list scrollable, by deveee
* Display all the relevant info in the challenge dialog in Story Mode, by Alayan
* New challenge selection interface, by CodingJellyfish
* Show the number of ranking points won or lost after a ranked race, by Benau
* Separate blurring visual effects from the main graphics presets, by Alayan
* Fix incorrect text resizing in the help menu, by Benau
* Make the custom random GP option more prominent in the track selection screen, by CodingJellyfish
* Improvements to the scrollbars, by QwertyChouskie
* Allow sorting lists with the keyboard, by Benau
* Allow tooltip drawing outside of menus, by deveee
* Prevent some font scaling blur caused by non-integer offsets, by Benau
* Add a (configurable) limit to chat message frequency, by GuillaumeBft and Benau
* Allow to explicitly set the flip direction of tabs, by CodingJellyfish
* Allow to download addons from the server lobby interface, by Benau
* Allow to filter installed and non-installed addons in the addons screen, by Alayan
* Many minor tweaks and bugfixes
#### In-race UI
* Scale down the lap time indicator next to the kart icons when there are many karts, by riso
* Limit how long chat and in-race messages can be, by Benau and Alayan
* Add an option to disable in-race messages only, by Alayan

### Tracks and modeling
#### Tracks
 * A handful of minor graphical fixes and enhancements
 * Fixed checklines that could be missed too easily in XR591 and the Old Mine.
#### Arenas
 * Pumpkin Park, by samuncle

## SuperTuxKart 1.0 (20. April 2019)
### Networking
* Networking support for normal race, time trial, free for all, capture the flag and soccer by Benau and hiker :
    * Access to a global server list, possibility to also create a LAN server
    * Track voting system to allow players to decide where the race will be
    * Communication between server and clients to exchange inputs and general game data
    * A lot of work on rewind code to make sure server and client are well synchronized
    * Chat lobby for general server information and discussion among players between races
    * Support for handicap, which can be changed before each game
* Spectating option for players having entered a server while a race or game is underway by Benau
* Option to join live an active game in FFA, CTF and soccer by Benau
* Global rankings by Benau (communication with main server) and Alayan (ranking formula)

### Race gameplay and physics
* New game mode Capture the Flag by Benau (online multiplayer only)
* New game mode Free for All by Benau
* Free for All available in offline mode with AI, by Mrxx99
* Revised kart characteristics for better balance between light, medium and heavy karts by Alayan
* Better random item distribution for various numbers of karts by Alayan and hiker
* Fix position interpolation causing some incorrect lapline validation by Auria
* Fix kart being uncontrollable and hovering when landing on some downward slopes by hiker
* Mitigate a physics issue which could send a kart flying on collisions by hiker
* Make kart turn radius based on kart class instead of kart model length (which made Adiumy unplayable and caused AI issues) by Alayan
* Revisited slipstreaming with boost easier to obtain, especially in curves, but not as strong as in 0.9.3 when the zipper boost was incorrectly activated, by Alayan
* Minor gameplay improvements (level 1 skid boost doesn't interrupt level 2 boost, fairer rubber-banding in low difficulties, boosted AI for some karts for more challenge in GPs, small balance change of GP points, more useful and consistent handicap option...)
* Start boost/penalty moved to the set phase for smoother networking
* Terrain slowdown works again as intended on several tracks where it was missing

### AI
* Improved powerup and nitro handling in AI by Alayan

### General
* Option to disable light scattering (for improved FPS), used in graphics level 3, by Partmedia
* Unlockable SuperTux challenges in Story Mode by Alayan
* Improvements to ghost replays (more data saved, live time difference, replay comparison, egg hunt replays) by Alayan
* Kart color customization by Benau
* Multithreading contention fixes by Benau
* Local multiplayer improvements by Ben Krajancic
* Major revamp of the achievement system to make adding new achievements much easier and flexible, also fixing some related bugs by Alayan
* Store up to 5 highscores for a track/difficulty/mode/kart number combination, instead of 3
* Smooth turning for non-keyboard inputs for improved consistency between input mode by deveee
* Updated standard replays by Alayan
* Visual improvements (new skidding particles, better rescue, bubblegum flashing before ending)
* Audio improvements (crash sound depending on speed/direction, no crash sound on rescue walls, sound cue in nitro challenges)
* Fix STK incorrectly connecting to the server when the internet option is disabled by Auria
* Updated WiiUse library
* Many bugfixes

### Android
* Gyroscope support by Pelya
* Numerous improvements to input on Android, including screen keyboard, keyboard, gamepads, by deveee
* Android TV support
* Improved UI and font scaling by deveee
* More compact APK thanks to png optimization, allows to have several tracks more than the 0.9.3 APK, by deveee and Alayan
* Wayland support and many other smaller improvements and fixes

### User Interface
* New networking user interface by Benau and hiker
* New coal skin by Alayan
* Support for collapsing widgets in UI by Mrxx99
* Multidirectional keyboard navigation in menus by Alayan
* Alternating background darkness in lists to enhance readability by Benau
* Support text breaks into multiple line in lists, e.g. in the addons menu, by Alayan
* Improved help menu with lateral tabs and a lot more information by Alayan
* Improved option menu with lateral tabs and a separate language tab by Alayan
* Many improvements to the ghost replay selection list (give access to egg hunt replays, kart icon shown in the list, replay's game version shown, hide player number column by default) and dialog (allow replay comparison, show track picture, show information about the selected replays) by Alayan
* Control configurations can be disabled without being deleted by Alayan
* Other significant enhancements (detailed progress of multi-goal achievements, reset password button, clearer checkbox status, milliseconds displayed in time-trials and egg hunts, nitro efficiency in the kart selection screen, better track info screen, better warnings when trying to do an online action with internet access disabled, hour support for GP time, default resolutions can't be used in fullscreen if unsupported by the monitor...)
* Many bugfixes and small enhancements mostly by deveee and Alayan
#### In-race UI
* New speedometer and nitro gauge, by Alayan
* Bigger minimap and new display options, by Alayan
* More legible text with outlines and for some bigger font, by Alayan

### Tracks and modeling
#### Tracks
* Black Forest by Sven Andreas Belting (replacing Green Valley)
* Ravenbridge Mansion by samuncle (replacing Black Hill Mansion)
* Unwanted shortcuts and exploits found by several players and fixed mostly by Auria in many tracks :
    * Antediluvian Abyss, Around the Lighthouse, Fort Magma, Grand Paradisio Island, Hacienda, Minigolf, Nessie's Pond, Northern Resort, The Old Mine, Oliver's Math Class, Shifting Sands, STK Enterprise, XR591
* Smoothness issues causing collisions and kart slowdown fixed by Auria :
    * Nessie's Pond, Old Mine, Shifting Sands, Volcano Island, XR591
* Item (boxes, nitro, bananas) positions improvement by Alayan and theThomasPat :
    * Around the Lighthouse, Black Forest, Candela City, Hacienda, Minigolf, Northern Resort, Oliver's Math Class, STK Enterprise, The Old Mine, Volcano Island, Zen Garden
    
#### Arenas
* Las Dunas Soccer, by samuncle
* Candela City arena by Benau, based on samuncle's track

#### Karts
* New version of Beastie by Jymis
* New version of Kiki by Benau

## SuperTuxKart 0.9.3 (28. October 2017)
* Reduced RAM and VRAM usage, reducing load times by Auria and Benau
* New mesh format optimized for space and hardware skinning
* Code refactoring of both render pipeline by Benau and Elderme
* Physics improvements and various physics bugfixes by hiker
* Kart GFX improvements (exhaust and headlight)
* In-game screen recording powered by libopenglrecorder
* High quality mipmap generation by Stragus
* New smoother camera by Auria
* New grand prix win scene
* Gamepad configuration bugfixes
* 3 Strikes Battles : added spare tire karts
* Visual representation of the start line in all tracks
* Various improvements (starting boost effect, wall driving fixes, parachutes, GP points, help page for bananas, cannon fixes, colorization shader)
### Tracks and modeling
#### Karts
* Kiki by Benau
* New versions of Wilber and Hexley by Jymis
* New version of Konqi by Benau
#### Tracks
* All tracks drivable in reverse, with arrows pointing in the correct direction
* Candela City by samuncle (replace Shiny Suburbs)
* Cornfield Crossing by samuncle (replace Bovine Barnyard)
* New battle track Las Dunas Arena by samuncle

## SuperTuxKart 0.9.2 (1. July 2016)
* Ghost replay races by Benau
* Battle mode AI by Benau
* Soccer mode AI by Benau
* TTF font rendering by Benau
* New ruby and forest skins by Benau
* Kart properties refactor by Flakebi
* Scripting work under the hood
* Work on the track editor by mhp
* Tweak to challenges
* New farm track song by 0zone0ne and Krobonil
* Bugfixes
### Tracks and modeling
#### Tracks
* Antediluvian Abysses by samuncle (replace Subsea)
* Volcano Island by Ponzino
* New icy soccer field by samuncle and Benau

## SuperTuxKart 0.9.1 (17. October 2015)
* Many bug fixes
* Started to use scripting in tracks
* Significant audio performance improvements
* Tweak to challenges
### Tracks and modeling
#### Tracks
* Better support for driving tracks in reverse
* New track layout and improved graphics for Oliver's Math Class, by samuncle
* Smaller tweaks and improvements to several tracks including 
    * XR591
    * Fort Magma
    * Gran Paradiso Island
    * Subsea

## SupertTuxKart 0.9 (24. April 2015)
* Fully shader-based rendering engine
* Online login which allows to:
    * connect with friends and see when they are playing
    * vote for addons
    * collect online achievements
* Grand Prix editor, including creation of random GPs 
* Different kart physics
### Tracks and modeling
#### Karts
* New karts Amanda and Gavroche by XGhost
* New and improved Tux, Adiumy, Sara the Wizard and the Racer, Xue
#### Tracks
* Cocoa Temple by samuncle (replace Amazonian Journey)
* Gran Paradiso Island by samuncle (replace The Island)
* Graphical improvements to many other tracks

## SuperTuxKart 0.8.1 (26. November 2013)
* New Soccer mode
* New Egg Hunt mode
* Added Tutorial
* Added new Supertux difficulty
* New bubblegum shield weapon
* New Speedometer and nitro meter
* Add ability to filter addons
* Add ability to save and resume Grand Prix
* Improve skid marks and nitro effects
* Wiimote support
### Tracks and modeling
#### Karts
* New karts Xue and Sara
* Updated Beastie kart
#### Tracks
* STK Enterprise by Rubberduck (replace Star Track)
* Redesign of Minigolf, by Rubberduck
* New longer track layout and improved graphics for Lighthouse, by samuncle
* Gameplay and graphical updates to several tracks :
    * The Old Mine
    * Zen Garden
#### Miscellaneous
* Updated nitro models

## SuperTuxKart 0.8 (11. December 2012)
* Story mode and new challenge set
* Improved AI
* Skidding and better collision physics
* Reverse mode
* Updated menus
* New music
### Tracks and modeling
#### Tracks
* Green Valley by Wolfs (replace Tux Tollway)
* Blackhill Mansion by samuncle (replace Crescent Crossing)
* New track layout and improved graphics for Shifting Sands (formerly Sand)
* Gameplay and graphical updates to several tracks :
    * XR591
    * Fort Magma
    * Jungle
    

## SuperTuxKart 0.7.3 (2. November 2011)
* New graphical effects
* New weapons 'Swatter' and 'Rubber Ball'
* 3 Strikes Battles now displays lives as spare tires
* Improved bubble gum
* See progression during Grand Prix
* Improve physics for tall karts (e.g. Adiumy)
* Lots of bug fixes
* Improved kart control at high speeds
* Better placement of rescued karts
* Transition track-making to blender 2.5/2.6
### Tracks and modeling
#### Karts
* New Suzanne kart
#### Tracks
* Zen Garden by samuncle (replace Secret Garden)
* Minigolf, by Mac
* New Subsea
* New Island battle arena
#### Miscellaneous
* Added Thunderbird as race referee

## SuperTuxKart 0.7.2 (15. July 2011)
* Added in-game addon manager
* Fixed major memory leaks
* Show when you get a highscore
* Improve gamepad configuration under Windows (add ability to tell gamepads apart)
* Various other tweaks done and glitches fixed
### Tracks and modeling
#### Karts
* New Beastie kart.
#### Tracks
* Improved Snow Peak by samuncle
* Improved Star Track UFO by Rudy

## SuperTuxKart 0.7.1b (21. April 2011)
* Fix circular dependency in challenges
* Updated translations

## SuperTuxKart 0.7.1 (15. April 2011)
* Particle  (smoke, splash, fire) and weather effects
* Added internet news
* Support for live language switch
* Added optional minimal race UI
* Temporary invincibility after being hit
* Added support for full-screen anti-aliasing
* Clearer multiplayer setup
* Renamed many tracks to nicer names
* Basic level-of-detail (LOD) support
* Debug features for track makers
* Update to bullet 2.77
* Replace more sounds to be DFSG-compliant
* Fixed character names that contain non-ASCII characters
* Full RTL (right to left) support
* Various other tweaks done and glitches fixed
### Tracks and modeling
#### Karts
* New Beagle kart by wolterh
#### Tracks
* New Fort Magma by samuncle
* New Shiny Suburbs by Horace

## SuperTuxKart 0.7 (December 2010)
Too many to list them all. Main points:

* Irrlicht:
  - Ported from plib to irrlicht
  - Added animations to karts and some tracks
* GUI
  - Completely new designed GUI
* Other improvements
  - Allowed alternative ways/shortcuts in tracks
  - New item 'switch'
### Tracks and modeling
#### Tracks
- Farm
- Hacienda by samuncle (replace Beach)
- Scotland by Canis Lupus
- Secret Garden

## SuperTuxKart 0.6.2a (October 2009)
* Bugfix: STK would crash while trying to save the config file
          on Windows Vista.

## SuperTuxKart 0.6.2 (July 2009)
* Bugfix: Game could crash in rare circumstances.
* Bugfix: Restarting a GP (with the in-race menu ESC) would not subtract already allocated points.
* Bugfix: A race could be finished with an invalid shortcut.
* Bugfix: Playing a challenge after a splitscreen game would play the challenge in split screen.
* Bugfix: Items explode over void.
* Bugfix: Grass in castle arena slowed down the kart.
* Bugfix: GP result showed kart identifier instead of name.
* Improvement: there is now 1 1 sec. wait period for the race result screen, avoiding the problem that someone presses space/enter at the end of a race, immediately quitting the menu before it can be read.

## SuperTuxKart 0.6.1a (February 2009)
* Bugfix: battle mode would not display track groups.

## SuperTuxKart 0.6.1 (February 2009)
* New music for Snow Mountain.
* Fixed bug in track selection screen that could cause a crash when track groups were used.
* Fixed crash in character selection that could happen if an old user config file existed.
* Fixed incorrect rescues in Fort Magma.
* Improved track selection screen to not display empty track groups.
* A plunger in the face is now removed when restarting.
* Added slow-down for karts driving backwards.
* Somewhat reduced 'shaking' of AI driven karts.
### Tracks and modeling
#### Karts
- New Puffy kart
#### Tracks
- New Cave battle map


## SuperTuxKart 0.6 (January 2009)
* New improved physics and kart handling
* Added sharp turns and nitro speed boost (replacing wheelies and jump)
* Totally rewrote powerups (plunger, bowling ball, cake, bubblegum) and new look for bananas
* New game mode : 3-Strikes Battle
* Major improvements to AI
* Improved user interface
* Karts now have a visible suspension effect
* Fully positional audio with OpenAL
* New music and sound effects (including engine, braking and skidding sounds)
* Better support for mods and add-ons (kart and track groups)
* New/updated translations (ga fi de nl sl fr it es ro sv)
* Allowed 'Grand Prix's of Time Trial, Follow the Leader, or any other mode
* Challenges are now specified and config files, and are thus easy to create by users
* Improved build system to better detect missing dependencies
* Improved shortcut-detection
* Initial work towards networking (disabled and hidden by default)
* Bug fixes and code refactor/cleanup/documentation
  - Fixed 'joystick locks' (kart would turn even if the joystick is in neutral),
    thanks to Samjam for the patch.
    
### Tracks and modeling
#### Karts
* Improved Wilber
* Eviltux
* Hexley
* Some old karts have been removed
#### Tracks
* Skyline
* Snow Mountain
* Race track
* Old Mine
* XR591
* Improved track :
    * Star track

## SuperTuxKart 0.5 (May 2008)
* Complete Challenges to unlock game modes, new tracks and a skidding preview
* New Follow the Leader game mode
* New Grand Prix
* Improved User Interface
* Improved game pad/joystick handling
* German, French, Dutch, Spanish, Italian and Swedish translations
* Additional music
* Many Bugfixes including:
	- a memory leak fix (Charlie Head)
	- an AI crash fix (Chris Morris)
	
### Tracks and modeling	
#### Tracks
* SnowTux Peak
* Amazonian Journey
* City
* Canyon
* BSODs Battlements renamed to Fort Magma
* Improved Crescent Crossing, Fort Magma, and Star Track

## SuperTuxKart 0.4 (February 2008)
* New physics handling using the bullet physics engine
* Improved AI
* New GUI handling, including resolution switching GUI
* Improved input handling	
* Jump and look-back feature
* Additional music and main theme
### Tracks and modeling
#### Karts
* New kart: wilber
	
#### Tracks
* Improved 'Shifting Sands' and 'Lighthouse'
	
## SuperTuxKart 0.3 (May 2007)
* Highscore lists	
* Shortcut detection 	
* Improved AI
* Fullscreen support
* New track: the island
* New penalty: bomb
* MacOSX support
* OpenAL and ogg-vorbis support
* Two new Grand Prixs	
* Improved user interface:
  - New racing interface
  - Better track map
  - Player kart dots in the track map are bigger than AI dots
  - Track selection screen has topview pictures
  - Added "Setup new race" option when a track is finished
  - Added "Restart race" option when a track is finished
  - The keyboard can skip vertical spaces between buttons
  - Better control configuration
  - Better in-game help
  - Added .desktop file for menus and icon
* Bugfixes:
  - Fixed bug in ssg_help::MinMax, which could cause a significant performance loss.
  - Fixed bug that allowed the joystick to erase the main menu
  - Fixed bug that allowed the joystick to "play the game while paused"
  - Fixed screen_manager assert failure bug
  - Fixed sound_manager assert failure bug
  - Fixed keyboard keys unable to work on the first key press bug
  - And others

## SuperTuxKart 0.2 (22. Sep 2006)
  * Significant performance improvement by using display lists
  * Improved AI
  * Support for different grand prixs
  * Too many bug fixes to list them all, but the important ones:
    * Work around for 'karts fall through track' compiler bug
    * Fixed rescue mode
  * Two new collectables: parachute and anvil
  * Track screen shots in the track select screen
  * Keyboard handling allows gradual turning
  * Improved physics (still work in progress)
    * All hard-coded properties like maximum velocity have
      been replaced by dynamically computed data dependent
      on kart parameters, allowing for karts having different
      characteristics.
  * Added help and about screens, added credits to track designer
  * Items were added to all tracks

## SuperTuxKart 0.1 (04. May 2006)  (not officially released)
  * Significant speedup by using a new HOT and collision algorithm --> all tracks are now playable
  * Removed all SDL dependencies, only plib is needed
  * Single and multi-window menu can be used
  * Code structure changes
  * Some bug fixes and small improvements
  * Added profile option to support automatic profiling

## SuperTuxKart 0.0.0 (22. Dec 2004)
  * new tracks
  * new characters and karts
  * new user-interface
  * some additional effects (skid-marks, smoke)

##TuxKart v0.4.0 (March 19th 2004)
  * Changes for compatibility with PLIB 1.8.0 and later.
  * Removed some features that were only there to support
    truly ancient graphics cards like 3Dfx Voodoo-1/2.

##TuxKart v0.3.0 (??)
  * Converted to use the new PLIB/PW library and thus
    avoid the need to link to GLUT.

##TuxKart v0.2.0 (Sept 3rd 2002)
  * Changes for compatibility with PLIB 1.6.0 and later.

##TuxKart v0.0.5 (??)
  * Changes for compatibility with PLIB 1.4.0 and later.

##TuxKart v0.0.4 (??)
  * Changes to suit rassin-frassin-Windoze-junk.
  * Steady-cam camera - courtesy of cowtan@ysbl.york.ac.uk
  * Changes for compatibility with PLIB 1.3.1 and later.
  * Added new music courtesy of Matt Thomas.

##TuxKart v0.0.3 (July 4th 2000)
  * Fixed bug in Keyboard driver when no
    joystick driver is installed.
  * More CygWin fixes.
  * Started new feature to allow you to be
    rescued from lava, etc.

##TuxKart v0.0.2 (July 2nd 2000)
  * Added ability to add new tracks without
    recompiling.
  * Can now drive using keyboard only - no joystick
    required.
  * Should compile and run under Windoze using CygWin.

##TuxKart v0.0.1 (July 1st 2000)
  * Fixed a couple of files missing in initial
    Distro.

##TuxKart v0.0.0 (June 29th 2000)
  * First CVS release.

##TuxKart (unnumbered) (April 13th 2000)
  * First hack.
