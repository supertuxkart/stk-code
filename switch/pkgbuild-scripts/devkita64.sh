#!/usr/bin/env bash
export DEVKITPRO=/opt/devkitpro
export PORTLIBS_ROOT=${DEVKITPRO}/portlibs
export PATH=${DEVKITPRO}/tools/bin:${DEVKITPRO}/devkitA64/bin:$PATH
export TOOL_PREFIX=aarch64-none-elf-
export AR=${TOOL_PREFIX}gcc-ar
export RANLIB=${TOOL_PREFIX}gcc-ranlib
