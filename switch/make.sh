#!/bin/bash

OLD_PWD="$(pwd)"

# Mac can prefix tool names with 'dkp-'
echo "CI: ${CI}"
if [[ "$CIq" == "trueq" ]]; then
  PACMAN=echo "(Simulated) pacman"
elif which dkp-pacman &>/dev/null; then
  PACMAN=dkp-pacman
elif which pacman &>/dev/null; then
  PACMAN=pacman
else
  echo "Couldn't find pacman in PATH! Is it installed?"
  echo "Please see https://devkitpro.org/wiki/devkitPro_pacman#Installing_devkitPro_Pacman for instructions to install it!"
  exit 1
fi

# Install deps. --needed means don't reinstall if already installed
sudo $PACMAN -S --needed \
  libnx devkit-env \
  devkitA64 general-tools \
  \
  devkitpro-pkgbuild-helpers \
  switch-curl switch-mbedtls \
  switch-freetype switch-libfribidi \
  switch-libogg switch-libvorbis \
  switch-libjpeg-turbo switch-libpng \
  switch-zlib switch-bzip2 \
  switch-physfs \
  switch-pkg-config \
  switch-sdl2 switch-mesa switch-libdrm_nouveau \
  libnx \
  switch-tools # elf2nro

# Users of MSYS2 or Arch Linux will already have Pacman installed but may not have the DKP repos on their system:
if [ $? -ne 0 ]; then
  echo "Failed to install packages! Did you add the repositories?"
  echo "Please see https://devkitpro.org/wiki/devkitPro_pacman#Customising_Existing_Pacman_Install for instructions!"
  exit 1
fi

# Unclear why this isn't in lib path
if [ ! -f "${DEVKITPRO}/portlibs/switch/lib/libpthread.a" ]; then
  sudo ln -s "${DEVKITPRO}/devkitA64/aarch64-none-elf/lib/libpthread.a" \
    "${DEVKITPRO}/portlibs/switch/lib/libpthread.a"
fi

SWITCH_DIR=$(realpath "$(dirname "$0")")
STK_DIR=$(dirname "${SWITCH_DIR}")

# Some shells don't set BASH_SOURCE. Let's set it just in case:
BASH_SOURCE="${DEVKITPRO}/switchvars.sh"
source "${DEVKITPRO}/switchvars.sh" # Sets environment variables needed for cross-compiling

if [ ! -d "${STK_DIR}/lib/harfbuzz/cmake_build" ]; then
  # Harfbuzz
  echo "Compiling Harfbuzz"
  mkdir "${STK_DIR}/lib/harfbuzz/cmake_build"
  cd "${STK_DIR}/lib/harfbuzz/cmake_build"
  cmake -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE="${DEVKITPRO}/switch.cmake" \
    -DUSE_SWITCH=ON -DCMAKE_INSTALL_PREFIX="${PORTLIBS_PREFIX}"  \
    -DHB_HAVE_FREETYPE=ON \
    ../
  
  make -j$(nproc)
  sudo make install
fi

if [ ! -d "${STK_DIR}/lib/openal/cmake_build" ]; then
  # OpenAL
  echo "Compiling OpenAL"
  mkdir "${STK_DIR}/lib/openal/cmake_build"
  cd "${STK_DIR}/lib/openal/cmake_build"
  cmake -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE="${DEVKITPRO}/switch.cmake" \
    -DUSE_SWITCH=ON -DALSOFT_UTILS=OFF -DLIBTYPE=STATIC -DALSOFT_EXAMPLES=OFF \
    -DALSOFT_REQUIRE_SDL2=ON -DALSOFT_BACKEND_SDL2=ON \
    -DSDL2_INCLUDE_DIR="${PORTLIBS_PREFIX}/include" \
    -DCMAKE_INSTALL_PREFIX="${PORTLIBS_PREFIX}"  \
    ../

  make -j$(nproc)
  sudo make install
fi

echo "Compiling STK"

mkdir "${STK_DIR}/cmake_build"
cd "${STK_DIR}/cmake_build"

cmake -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE="${DEVKITPRO}/switch.cmake" \
    -DUSE_SWITCH=ON \
    -DCMAKE_INSTALL_PREFIX=/  \
    ../

make -j$(nproc)
make install DESTDIR=./install

# Build nro (executable for switch)
"${SWITCH_DIR}/package.sh"

echo "Building package"

rm -rf sdcard
mkdir sdcard
# Move data over
mv install/share/supertuxkart/data sdcard/stk-data
# Add executable
mkdir sdcard/switch
mv bin/stk.nro sdcard/switch/stk.nro

echo "Compressing"

# Zip up actual release:
cd sdcard
ZIP_PATH="${STK_DIR}/cmake_build/bin/SuperTuxKart-Switch-${PROJECT_VERSION}.zip"
zip -r "${ZIP_PATH}" .

# Recover old pwd
cd $OLD_PWD

echo "Done. Package available at ${ZIP_PATH}"
