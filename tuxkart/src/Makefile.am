# src/

bindir=$(prefix)/games

bin_PROGRAMS = tuxkart

if HAVE_LIBSDL
VIDEO_FILES = sdldrv.cxx sdldrv.h
else
VIDEO_FILES = pwdrv.cxx pwdrv.h
endif

tuxkart_SOURCES = \
  $(VIDEO_FILES) \
  AutoDriver.cxx  \
  Camera.cxx  \
  Camera.h  \
  CupData.h \
  CupData.cxx \
  Driver.cxx  \
  Driver.h  \
  Explosion.cxx  \
  Explosion.h  \
  Herring.cxx  \
  Herring.h  \
  KartProperties.h \
  KartProperties.cxx \
  KartManager.h \
  KartManager.cxx \
  KartDriver.cxx  \
  Loader.cxx \
  Loader.h \
  PlayerDriver.h  \
  PlayerDriver.cxx  \
  Projectile.cxx  \
  Track.cxx  \
  Track.h  \
  Controller.h \
  GrandPrixSetup.h \
  GrandPrixSetup.cxx \
  TrackManager.cxx \
  TrackManager.h \
  Traffic.cxx  \
  WorldLoader.cxx  \
  WorldLoader.h  \
  StartScreen.h \
  StartScreen.cxx \
  Screen.h \
  ScreenManager.h \
  ScreenManager.cxx \
  constants.h  \
  guNet.cxx  \
  guNet.h  \
  WidgetSet.cxx  \
  WidgetSet.h  \
  widget_image.cxx \
  widget_image.h \
  isect.cxx  \
  isect.h  \
  joystick.h  \
  level.h  \
  material.cxx  \
  material.h  \
  plibwrap.h \
  preprocessor.cxx  \
  preprocessor.h  \
  sound.cxx  \
  sound.h  \
  start_tuxkart.cxx  \
  start_tuxkart.h  \
  StringUtils.h \
  StringUtils.cxx \
  SkidMark.h \
  SkidMark.cxx \
  Shadow.h \
  Shadow.cxx \
  tuxkart.cxx  \
  tuxkart.h  \
  TrackData.h \
  TrackData.cxx \
  utils.cxx  \
  utils.h \
  lispreader.cxx \
  lispreader.h \
  World.h \
  World.cxx \
  WorldScreen.h \
  WorldScreen.cxx \
  ParticleSystem.h \
  ParticleSystem.cxx \
  gui/RaceGUI.cxx \
  gui/RaceGUI.h \
  gui/MainMenu.cxx \
  gui/MainMenu.h \
  gui/BaseGUI.cxx \
  gui/BaseGUI.h \
  gui/CharSel.cxx \
  gui/CharSel.h \
  gui/Difficulty.cxx \
  gui/Difficulty.h \
  gui/GameMode.cxx \
  gui/GameMode.h \
  gui/Options.cxx \
  gui/Options.h \
  gui/TrackSel.cxx \
  gui/TrackSel.h \
  gui/NumPlayers.cxx \
  gui/NumPlayers.h \
  gui/ConfigControls.cxx \
  gui/ConfigControls.h \
  gui/PlayerControls.cxx \
  gui/PlayerControls.h \
  gui/RaceResultsGUI.cxx \
  gui/RaceResultsGUI.h \
  gui/RaceMenu.cxx \
  gui/RaceMenu.h

# EOF #
