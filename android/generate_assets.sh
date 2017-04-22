#!/bin/sh
#
# (C) 2016-2017 Dawid Gan, under the GPLv3
#
# A script that generates data files for Android apk


# Below are simple configuration variables
# It's allowed to set "all" for KARTS and TRACKS if it's intended to create
# package with full data.
# The karts and tracks directories shouldn't exist in ASSETS_DIRS variable
# because they are handled separately
# The TEXTURE_SIZE and SOUND_QUALITY take effect only if DECREASE_QUALITY has
# value greater than 0
# The script needs imagemagick and ogg utils installed to use DECREASE_QUALITY
# feature

################################################################################

export KARTS="tux nolok xue"
export TRACKS="battleisland cornfield_crossing featunlocked gplose gpwin   \
               hacienda introcutscene introcutscene2 lighthouse olivermath \
               overworld snowmountain snowtuxpeak soccer_field tutorial"

export ASSETS_PATHS="../data                    \
                     ../../stk-assets           \
                     ../../supertuxkart-assets"

export ASSETS_DIRS="library models music sfx textures"

export TEXTURE_SIZE=256
export SOUND_QUALITY=42
export SOUND_MONO=1
export SOUND_SAMPLE=32000

export RUN_OPTIMIZE_SCRIPT=0
export DECREASE_QUALITY=1

################################################################################

export LANG=C

cd "`dirname "$0"`"

# Find assets path
for ASSETS_PATH in $ASSETS_PATHS; do
   if [ -d $ASSETS_PATH ] && [ `ls $ASSETS_PATH | grep -c tracks` -gt 0 ]; then
      echo "Assets found in $ASSETS_PATH"
      ASSETS_PATH_FOUND=1
      break
   fi
done

if [ -z $ASSETS_PATH_FOUND ]; then
   echo "Couldn't find assets path"
   exit 1
fi

if [ ! -d "../data" ]; then
   echo "Couldn't find data directory"
   exit 1
fi


# Clear previous assets directory
echo "Clear previous assets directory"
rm -rf assets


# Copy all assets
echo "Copy all assets"

mkdir -p assets/data

for DIR in `ls $ASSETS_PATH`; do
   CAN_BE_COPIED=0

   for ASSETS_DIR in $ASSETS_DIRS; do
      if [ $DIR = $ASSETS_DIR ]; then
         CAN_BE_COPIED=1
         break
      fi
   done;

   # Don't copy karts and tracks. It will be handled later
   BLACKLIST_ASSETS="karts tracks"
   for ASSETS_DIR in $BLACKLIST_ASSETS; do
      if [ $DIR = $ASSETS_DIR ]; then
         CAN_BE_COPIED=0
         break
      fi
   done;

   if [ $CAN_BE_COPIED -gt 0 ]; then
      cp -a "$ASSETS_PATH/$DIR" assets/data/
   fi
done;


# Copy selected tracks
echo "Copy selected tracks"

mkdir -p assets/data/tracks

for DIR in `ls $ASSETS_PATH/tracks`; do
   CAN_BE_COPIED=0

   if [ "$TRACKS" != "all" ]; then
      for TRACK in $TRACKS; do
         if [ $DIR = $TRACK ]; then
            CAN_BE_COPIED=1
            break
         fi
      done;
   else
      CAN_BE_COPIED=1
   fi

   if [ $CAN_BE_COPIED -gt 0 ]; then
      cp -a "$ASSETS_PATH/tracks/$DIR" assets/data/tracks/
   fi
done


# Copy selected karts
echo "Copy selected karts"

mkdir -p assets/data/karts

for DIR in `ls $ASSETS_PATH/karts`; do
   CAN_BE_COPIED=0

   if [ "$KARTS" != "all" ]; then
      for KART in $KARTS; do
         if [ $DIR = $KART ]; then
            CAN_BE_COPIED=1
            break
         fi
      done;
   else
      CAN_BE_COPIED=1
   fi

   if [ $CAN_BE_COPIED -gt 0 ]; then
      cp -a "$ASSETS_PATH/karts/$DIR" assets/data/karts/
   fi
done


# Decrease assets quality in order to save some disk space and RAM
echo "Decrease assets quality"

convert_image()
{
   if [ -z "$1" ]; then
      echo "No file to convert"
      return
   fi

   FILE="$1"

   W=`identify -format "%[fx:w]" "$FILE"`
   H=`identify -format "%[fx:h]" "$FILE"`

   if [ -z $W ] || [ -z $H ]; then
      echo "Couldn't convert $FILE file"
      return
   fi

   if [ $W -le $TEXTURE_SIZE ] && [ $H -le $TEXTURE_SIZE ]; then
      return
   fi

   if [ $W -gt $H ]; then
      SCALED_W=$TEXTURE_SIZE
      SCALED_H=$(($TEXTURE_SIZE * $H / $W))
   else
      SCALED_W=$(($TEXTURE_SIZE * $W / $H))
      SCALED_H=$TEXTURE_SIZE
   fi

   convert -scale $SCALED_WE\x$SCALED_H "$FILE" "$FILE"
}

convert_sound()
{
   if [ -z "$1" ]; then
      echo "No file to convert"
      return
   fi

   FILE="$1"

   oggdec "$FILE" -o tmp.wav

   if [ -s tmp.wav ]; then
      OGGENC_CMD=""
      
      if [ "$SOUND_MONO" -gt 0 ]; then
         OGGENC_CMD="$OGGENC_CMD --downmix"
      fi
      
      OGG_RATE=`ogginfo "$FILE" | grep "Rate: " | cut -f 2 -d " " \
                                                            | grep -o '[0-9]*'`
      
      if [ ! -z "$OGG_RATE" ] && [ "$OGG_RATE" -gt "32000" ]; then
         OGGENC_CMD="$OGGENC_CMD --resample 32000"
      fi
      
      OGGENC_CMD="$OGGENC_CMD -b $SOUND_QUALITY"
      
      oggenc $OGGENC_CMD tmp.wav -o tmp.ogg
   fi

   if [ -s tmp.ogg ]; then
      SIZE_OLD=`du -k "$FILE" | cut -f1`
      SIZE_NEW=`du -k "tmp.ogg" | cut -f1`

      if [ $SIZE_NEW -lt $SIZE_OLD ]; then
         mv tmp.ogg "$FILE"
      fi
   fi

   rm -f tmp.wav tmp.ogg
}

if [ $DECREASE_QUALITY -gt 0 ]; then
   find assets/data -iname "*.png" | while read f; do convert_image "$f"; done
   find assets/data -iname "*.jpg" | while read f; do convert_image "$f"; done
   find assets/data -iname "*.ogg" | while read f; do convert_sound "$f"; done
fi


# Copy data directory
echo "Copy data directory"
cp -a ../data/* assets/data/


# Run optimize_data.sh script
if [ $RUN_OPTIMIZE_SCRIPT -gt 0 ]; then
   echo "Run optimize_data.sh script"
   sh -c 'cd assets/data; ../../../data/optimize_data.sh'
fi


# Generate directories list
echo "Generate directories list"
find assets/* -type d > assets/directories.txt
sed -i s/'.\/assets\/'// assets/directories.txt
sed -i s/'assets\/'// assets/directories.txt


# It will be probably ignored by ant, but create it anyway...
touch assets/.nomedia


echo "Done."
exit 0
