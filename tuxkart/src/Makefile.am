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
  Driver.cxx  \
  Driver.h  \
  Explosion.cxx  \
  Explosion.h  \
  Herring.cxx  \
  Herring.h  \
  KartProperties.h \
  KartProperties.cxx \
  KartDriver.cxx  \
  Loader.cxx \
  Loader.h \
  PlayerDriver.cxx  \
  Projectile.cxx  \
  Track.cxx  \
  Track.h  \
  Traffic.cxx  \
  WorldLoader.cxx  \
  WorldLoader.h  \
  constants.h  \
  gfx.cxx  \
  gfx.h  \
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
  preprocessor.cxx  \
  preprocessor.h  \
  sound.cxx  \
  sound.h  \
  start_tuxkart.cxx  \
  start_tuxkart.h  \
  status.cxx  \
  status.h  \
  Shadow.h \
  Shadow.cxx \
  tuxkart.cxx  \
  tuxkart.h  \
  utils.cxx  \
  utils.h \
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
  gui/RaceMenu.cxx \
  gui/RaceMenu.h

# EOF #
