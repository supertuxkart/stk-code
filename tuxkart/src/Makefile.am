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
  Driver.cxx  \
  Explosion.cxx  \
  Herring.cxx  \
  KartDriver.cxx  \
  PlayerDriver.cxx  \
  Projectile.cxx  \
  Track.cxx  \
  Traffic.cxx  \
  guNet.cxx  \
  isect.cxx  \
  joystick.h  \
  WorldLoader.cxx  \
  sound.cxx  \
  utils.cxx  \
  status.h  \
  gfx.cxx  \
  gfx.h  \
  gui.cxx  \
  gui.h  \
  material.cxx  \
  material.h  \
  preprocessor.cxx  \
  preprocessor.h  \
  start_tuxkart.cxx  \
  start_tuxkart.h  \
  status.cxx  \
  tuxkart.cxx  \
  tuxkart.h  \
  Camera.h  \
  Driver.h  \
  Explosion.h  \
  Herring.h  \
  Track.h  \
  constants.h  \
  guNet.h  \
  isect.h  \
  level.h  \
  WorldLoader.h  \
  sound.h  \
  utils.h \
  Loader.cxx \
  Loader.h

# EOF #
