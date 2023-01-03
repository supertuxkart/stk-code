#!/bin/bash

OLD_PWD="$(pwd)"

SWITCH_DIR=$(realpath "$(dirname "$0")")
STK_DIR=$(dirname "${SWITCH_DIR}")

echo "Compiling STK"

if [[ ! -d "${STK_DIR}/cmake_build" ]]; then
  mkdir "${STK_DIR}/cmake_build"
fi
cd "${STK_DIR}/cmake_build"

"${DEVKITPRO}/portlibs/switch/bin/aarch64-none-elf-cmake" -G"Unix Makefiles" \
    -DCMAKE_INSTALL_PREFIX=/ -DNO_SHADERC=on \
    ../

make -j$(nproc) || exit 1
make install DESTDIR=./install || exit 1

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
ZIP_PATH="${STK_DIR}/cmake_build/bin/SuperTuxKart-${PROJECT_VERSION}-switch.zip"
if [[ -f "${ZIP_PATH}" ]]; then
  rm "${ZIP_PATH}"
fi
zip -r "${ZIP_PATH}" .

# Recover old pwd
cd $OLD_PWD

echo "Done. Package available at ${ZIP_PATH}"
