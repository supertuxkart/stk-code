#SuperTuxKart Installation Instructions

Note : If you obtained this source code from github, you also need to download the game assets from sourceforge using SVN.

`svn checkout https://svn.code.sf.net/p/supertuxkart/code/stk-assets stk-assets`

Place the `stk-assets` folder next to the source root `stk-code` folder.
See <http://supertuxkart.sourceforge.net/Source_control> for more information


##Building STK on Linux

First, make sure that you have the following packages installed:

  * OpenGL (mesa)
  * OpenAL (recommended: openal-soft-devel)
  * Ogg    (libogg-dev)
  * Vorbis (libvorbis-dev)
  * libcurl (libcurl-devel)
  * libbluetooth (bluez-devel)
  * libpng (libpng-devel)
  * zlib (zlib-devel)
  * jpeg (libjpeg-turbo-devel)

Ubuntu command:

```
sudo apt-get install autoconf automake build-essential cmake libogg-dev libvorbis-dev libopenal-dev libxxf86vm-dev \
libgl1-mesa-dev libglu1-mesa-dev libcurl4-openssl-dev libfribidi-dev libbluetooth-dev libxrandr-dev
```
  
Unpack the files from the tarball like this:

```
tar xzf supertuxkart-*.tar.gz
cd supertuxkart-*
```

where `*` is the version of SuperTuxkart you downloaded - eg `0.8.0`. Then:


Compile SuperTuxKart:

```
mkdir cmake_build
cd cmake_build
cmake ..
make VERBOSE=1 -j2
```

To create a debug version of STK, use:

`cmake .. -DCMAKE_BUILD_TYPE=Debug`

To test the compilation, supertuxkart can be run from the build
directory by ./bin/supertuxkart 

To install the file, as root execute:

`make install`

The default install location is `/usr/local`, i.e. the data files will
be written to `/usr/local/share/games/supertuxkart`, the executable
will be copied to `/usr/local/bin`. To change the default installation
location, specify `CMAKE_INSTALL_PREFIX` when running cmake, e.g.:
`cmake .. -DCMAKE_INSTALL_PREFIX=/opt/stk`

  
##Building STK on OS X
See <http://supertuxkart.sourceforge.net/Building_and_packaging_on_OSX>


##Building STK on Windows
See <http://supertuxkart.sourceforge.net/How_to_build_the_Windows_version>
