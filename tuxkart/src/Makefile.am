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
  BaseGUI.cxx \
  BaseGUI.h \
  MainMenu.cxx \
  MainMenu.h \
  RaceGUI.cxx \
  RaceGUI.h \
  isect.cxx  \
  isect.h  \
  joystick.h  \
  level.h  \
  material.cxx  \
  material.h  \
  oldgui.cxx \
  oldgui.h \
  preprocessor.cxx  \
  preprocessor.h  \
  sound.cxx  \
  sound.h  \
  start_tuxkart.cxx  \
  start_tuxkart.h  \
  status.cxx  \
  status.h  \
  tuxkart.cxx  \
  tuxkart.h  \
  utils.cxx  \
  utils.h

# EOF #
