# Sets CMAKE_MAP_IMPORTED_CONFIG_* so that if there isn't a perfect match between
# the current project's build type and the imported build, but they're both some
# kind of "Release" variant, things will just work.
#
# Original Author:
# 2015 Ryan Pavlik <ryan@sensics.com> <abiryan@ryand.net>
# http://academic.cleardefinition.com
#
# Copyright Sensics, Inc. 2015.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

# RelWithDebInfo falls back to Release, then MinSizeRel
set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO RelWithDebInfo Release MinSizeRel NoConfig)

# MinSizeRel falls back to Release, then RelWithDebInfo
set(CMAKE_MAP_IMPORTED_CONFIG_MINSIZEREL MinSizeRel Release RelWithDebInfo NoConfig)

# Release falls back to RelWithDebInfo, then MinSizeRel
set(CMAKE_MAP_IMPORTED_CONFIG_RELEASE Release RelWithDebInfo MinSizeRel NoConfig)
