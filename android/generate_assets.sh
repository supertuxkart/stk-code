#!/bin/sh
#
# (C) 2016-2017 Dawid Gan, under the GPLv3
#
# A script that generates data files for Android apk


# Below you can find some simple configuration variables.
# It's allowed to set "all" for KARTS and TRACKS if it's intended to create
# package with full data.
# The karts and tracks directories shouldn't exist in ASSETS_DIRS variable
# because they are handled separately.
# The TEXTURE_SIZE and SOUND_QUALITY take effect only if DECREASE_QUALITY has
# value greater than 0.
# The CONVERT_TO_JPG variable enables converting all images that are safe to 
# convert and keeps other images untouched.
# The script needs imagemagick and ogg utils installed to use DECREASE_QUALITY
# feature

################################################################################

export KARTS="all"
export TRACKS="abyss battleisland cave cornfield_crossing endcutscene        \
               featunlocked fortmagma gplose gpwin hacienda icy_soccer_field \
               introcutscene introcutscene2 lighthouse mines olivermath      \
               overworld sandtrack scotland snowmountain snowtuxpeak         \
               soccer_field stadium tutorial zengarden"

export ASSETS_PATHS="../data                    \
                     ../../stk-assets           \
                     ../../supertuxkart-assets"

export ASSETS_DIRS="library models music sfx textures"

export TEXTURE_SIZE=256
export JPEG_QUALITY=85
export PNG_QUALITY=95
export SOUND_QUALITY=42
export SOUND_MONO=1
export SOUND_SAMPLE=32000

export RUN_OPTIMIZE_SCRIPT=0
export DECREASE_QUALITY=1
export CONVERT_TO_JPG=1

export CONVERT_TO_JPG_BLACKLIST="data/karts/hexley/hexley_kart_diffuse.png"

export BLACKLIST_FILES="data/music/cocoa_river_fast.ogg2"

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
    FILE="$1"
    FILE_TYPE="$2"
    echo "Convert file: $FILE"

    if [ ! -f "$FILE" ]; then
        echo "  File doesn't exist."
        return
    fi

    W=`identify -format "%[fx:w]" "$FILE"`
    H=`identify -format "%[fx:h]" "$FILE"`

    if [ -z $W ] || [ -z $H ]; then
        echo "Couldn't convert $FILE file"
        return
    fi

    if [ $W -gt $TEXTURE_SIZE ] || [ $H -gt $TEXTURE_SIZE ]; then
        if [ $W -gt $H ]; then
            SCALED_W=$TEXTURE_SIZE
            SCALED_H=$(($TEXTURE_SIZE * $H / $W))
        else
            SCALED_W=$(($TEXTURE_SIZE * $W / $H))
            SCALED_H=$TEXTURE_SIZE
        fi

        SCALE_CMD="-scale ${SCALED_W}x${SCALED_H}"
    fi

    if [ "$FILE_TYPE" = "jpg" ]; then
        QUALITY_CMD="-quality $JPEG_QUALITY"
    elif [ "$FILE_TYPE" = "png" ]; then
        QUALITY_CMD="-quality $PNG_QUALITY"
    fi

    convert $SCALE_CMD $QUALITY_CMD "$FILE" "tmp.$FILE_TYPE"

    if [ -s "tmp.$FILE_TYPE" ]; then
        SIZE_OLD=`du -k "$FILE" | cut -f1`
        SIZE_NEW=`du -k "tmp.$FILE_TYPE" | cut -f1`

        if [ $SIZE_NEW -lt $SIZE_OLD ]; then
            mv "tmp.$FILE_TYPE" "$FILE"
        fi
    fi

    rm -f "tmp.$FILE_TYPE"
}

convert_sound()
{
    FILE="$1"
    echo "Convert file: $FILE"

    if [ ! -f "$FILE" ]; then
        echo "  File doesn't exist."
        return
    fi

    oggdec "$FILE" -o tmp.wav

    if [ -s tmp.wav ]; then
        OGGENC_CMD=""

        if [ "$SOUND_MONO" -gt 0 ]; then
            OGGENC_CMD="$OGGENC_CMD --downmix"
        fi

        OGG_RATE=`ogginfo "$FILE" | grep "Rate: " | cut -f 2 -d " " \
                                                             | grep -o '[0-9]*'`

        if [ ! -z "$OGG_RATE" ] && [ "$OGG_RATE" -gt "$SOUND_SAMPLE" ]; then
            OGGENC_CMD="$OGGENC_CMD --resample $SOUND_SAMPLE"
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

convert_to_jpg()
{
    FILE="$1"
    echo "Convert file: $FILE"

    if [ ! -f "$FILE" ]; then
        echo "  File doesn't exist."
        return
    fi

    ALREADY_CONVERTED=0

    if [ -s "./converted_textures" ]; then
        while read -r CONVERTED_TEXTURE; do
            if [ "$FILE" = "$CONVERTED_TEXTURE" ]; then
                ALREADY_CONVERTED=1
                break
            fi
        done < "./converted_textures"
    fi

    if [ $ALREADY_CONVERTED -eq 1 ]; then
        return
    fi
    
    BLACKLISTED=0
    
    for BLACKLIST_FILE in $CONVERT_TO_JPG_BLACKLIST; do
        if [ "$FILE" = "assets/$BLACKLIST_FILE" ]; then
            BLACKLISTED=1
            break
        fi
    done
    
    if [ $BLACKLISTED -eq 1 ]; then
        #echo "  File is blacklisted. Ignore..."
        continue
    fi

    FILE_EXTENSION=`echo "$FILE" | tail -c 5`

    if [ `echo "$FILE_EXTENSION" | head -c 1` != "." ]; then
        #echo "  Unsupported file extension. Ignore..."
        continue
    fi

    FILE_FORMAT=`identify -format %m "$FILE"`

    if [ "$FILE_FORMAT" = "JPEG" ]; then
        #echo "  File is already JPEG. Ignore..."
        continue
    fi

    IS_OPAQUE=`identify -format '%[opaque]' "$FILE"`
    #HAS_ALPHA=`identify -format '%A' "$FILE"`

    if [ "$IS_OPAQUE" = "False" ] || [ "$IS_OPAQUE" = "false" ]; then
        #echo "  File has alpha channel. Ignore..."
        continue
    fi

    DIRNAME="`dirname "$FILE"`"
    BASENAME="`basename "$FILE"`"     
    IS_GLOSS_MAP=`find "$DIRNAME" -iname "*.xml" -exec cat {} \; \
                                            | grep -c "gloss-map=\"$BASENAME\""`
    
    if [ $IS_GLOSS_MAP -gt 0 ]; then
        #echo "  File is a gloss-map. Ignore..."
        continue
    fi

    NEW_FILE="`echo $FILE | head -c -5`.jpg"

    if [ -f "$NEW_FILE" ]; then
        #echo "  There is already a file with .jpg extension. Ignore..."
        continue
    fi

    # We can check if new file is smaller
    convert -quality $JPEG_QUALITY "$FILE" "$NEW_FILE"
    rm -f "$FILE"

    echo "$FILE" >> "./converted_textures"
}

convert_to_jpg_extract_b3dz()
{
    FILE="$1"
    echo "Convert file: $FILE"

    if [ ! -f "$FILE" ]; then
        echo "  File doesn't exist."
        return
    fi

    DIRNAME="`dirname "$FILE"`"

    unzip "$FILE" -d "$DIRNAME"
    rm -f "$FILE"

    TEXNAME="`basename "$FILE"`"
    NEWNAME="`echo $TEXNAME | head -c -6`.b3d"

    sed -i "s/\"$TEXNAME\"/\"$NEWNAME\"/g" "$DIRNAME/kart.xml"
}

convert_to_jpg_update_b3d()
{
    FILE="$1"
    echo "Convert file: $FILE"

    if [ ! -f "$1" ]; then
        echo "  File doesn't exist."
        return
    fi

    HEX_FILE=`hexdump -ve '1/1 "%.2x"' "$FILE"`

    TEXS_CHUNK="54455853"
    TEXS_CHUNK_POS=24

    FOUND_CHUNK=`echo $HEX_FILE | head -c $(($TEXS_CHUNK_POS + 8)) \
                                            | tail -c +$(($TEXS_CHUNK_POS + 1))`

    if [ -z "$FOUND_CHUNK" ] || [ "$FOUND_CHUNK" != "$TEXS_CHUNK" ]; then
        echo "  File has no textures."
        return
    fi

    TEXS_SIZE=`echo $HEX_FILE | head -c $(($TEXS_CHUNK_POS + 16)) | tail -c 8`

    TEXS_SIZE_CONVERTED=`echo $TEXS_SIZE | cut -c7-8`
    TEXS_SIZE_CONVERTED=$TEXS_SIZE_CONVERTED`echo $TEXS_SIZE | cut -c5-6`
    TEXS_SIZE_CONVERTED=$TEXS_SIZE_CONVERTED`echo $TEXS_SIZE | cut -c3-4`
    TEXS_SIZE_CONVERTED=$TEXS_SIZE_CONVERTED`echo $TEXS_SIZE | cut -c1-2`
    TEXS_SIZE_CONVERTED=`echo $((0x$TEXS_SIZE_CONVERTED))`

    if [ $TEXS_SIZE_CONVERTED -le 0 ]; then
        echo "  Invalid TEXS size value."
        return
    fi

    TEXS_BEGIN=$(($TEXS_CHUNK_POS + 16))
    TEXS_END=$(($TEXS_BEGIN + $TEXS_SIZE_CONVERTED * 2))
    HEX_TEXS=`echo $HEX_FILE | head -c $TEXS_END | tail -c +$(($TEXS_BEGIN+1))`
    CURR_POS=0

    while [ $CURR_POS -lt $TEXS_END ]; do
        NULL_POS=`echo $HEX_TEXS | tail -c +$(($CURR_POS+1)) | grep -b -o "00" \
                                                    | head -n 1 | cut -f1 -d":"`

        if [ -z $NULL_POS ]; then
            #echo "  Done."
            break
        fi

        if [ $NULL_POS -lt 4 ]; then
            echo "  Something went wrong..."
            break
        fi

        TEXNAME_BEGIN=$((($TEXS_BEGIN + $CURR_POS) / 2))
        TEXNAME_END=$((($TEXS_BEGIN + $CURR_POS + $NULL_POS) / 2))
        CURR_POS=$(($CURR_POS + $NULL_POS + 58))

        TEXTURE_NAME=`dd if="$FILE" bs=1 skip=$TEXNAME_BEGIN \
                          count=$(($TEXNAME_END - $TEXNAME_BEGIN)) 2> /dev/null`
        DIRNAME="`dirname "$FILE"`"
        TEXTURE_PATH="$DIRNAME/$TEXTURE_NAME"

        IS_CONVERTED=0

        while read -r CONVERTED_TEXTURE; do
            if [ "$TEXTURE_PATH" = "$CONVERTED_TEXTURE" ]; then
                IS_CONVERTED=1
                break
            fi
        done < "./converted_textures"

        if [ $IS_CONVERTED -eq 1 ]; then
            echo -n ".jpg" | dd of="$FILE" bs=1 seek=$(($TEXNAME_END - 4)) \
                                                       conv=notrunc 2> /dev/null
        fi;
    done
}

convert_to_jpg_update_spm()
{
    FILE="$1"
    echo "Convert file: $FILE"

    if [ ! -f "$1" ]; then
        echo "  File doesn't exist."
        return
    fi

    HEX_FILE=`hexdump -ve '1/1 "%.2x"' "$FILE"`

    SP_HEADER="5350"
    SP_FOUND=`echo $HEX_FILE | head -c 4`

    if [ -z "$SP_FOUND" ] || [ "$SP_FOUND" != "$SP_HEADER" ]; then
        echo "  Unsupported format."
        return
    fi

    TEXS_BEGIN=60
    TEXS_COUNT=`echo $HEX_FILE | head -c $TEXS_BEGIN | tail -c 4`

    TEXS_COUNT_CONVERTED=`echo $TEXS_COUNT | cut -c3-4`
    TEXS_COUNT_CONVERTED=$TEXS_COUNT_CONVERTED`echo $TEXS_COUNT | cut -c1-2`
    TEXS_COUNT_CONVERTED=`echo $((0x$TEXS_COUNT_CONVERTED))`
    TEXS_COUNT_CONVERTED=$(($TEXS_COUNT_CONVERTED * 2))

    if [ $TEXS_COUNT_CONVERTED -le 0 ]; then
        echo "  Invalid textures count value."
        return
    fi

    CURR_POS=$(($TEXS_BEGIN + 2))

    while [ $TEXS_COUNT_CONVERTED -gt 0 ]; do
        TEXS_COUNT_CONVERTED=$(($TEXS_COUNT_CONVERTED - 1))

        TEX_LEN=`echo $HEX_FILE | head -c $(($CURR_POS)) | tail -c 2`
        TEX_LEN=`echo $((0x$TEX_LEN))`

        TEXNAME_BEGIN=$(($CURR_POS / 2))
        TEXNAME_END=$(($CURR_POS / 2 + $TEX_LEN))
        CURR_POS=$(($CURR_POS + 2 + $TEX_LEN * 2))

        if [ $TEX_LEN -eq 0 ]; then
            #echo "  Empty texture name, ignore..."
            continue
        fi

        TEXTURE_NAME=`dd if="$FILE" bs=1 skip=$TEXNAME_BEGIN \
                          count=$(($TEXNAME_END - $TEXNAME_BEGIN)) 2> /dev/null`
      
        DIRNAME="`dirname "$FILE"`"
        TEXTURE_PATH="$DIRNAME/$TEXTURE_NAME"

        IS_CONVERTED=0

        while read -r CONVERTED_TEXTURE; do
            if [ "$TEXTURE_PATH" = "$CONVERTED_TEXTURE" ]; then
                IS_CONVERTED=1
                break
            fi
        done < "./converted_textures"

        if [ $IS_CONVERTED -eq 1 ]; then
            echo -n ".jpg" | dd of="$FILE" bs=1 seek=$(($TEXNAME_END - 4)) \
                                                       conv=notrunc 2> /dev/null
        fi
    done
}

convert_to_jpg_update_xml()
{
    FILE="$1"
    echo "Convert file: $FILE"

    if [ ! -f "$FILE" ]; then
        echo "  File doesn't exist."
        return
    fi

    DIRNAME="`dirname "$FILE"`"

    while read -r CONVERTED_TEXTURE; do
        DIRNAME_TEX="`dirname "$CONVERTED_TEXTURE"`"

        if [ "$DIRNAME_TEX" != "$DIRNAME" ]; then
            continue;
        fi

        TEXNAME="`basename "$CONVERTED_TEXTURE" | head -c -5`"

        sed -i "s/\"$TEXNAME.[pP][nN][gG]/\"$TEXNAME.jpg/g" "$FILE"
        sed -i "s/ $TEXNAME.[pP][nN][gG]/ $TEXNAME.jpg/g" "$FILE"
    done < "./converted_textures"
}


if [ $DECREASE_QUALITY -gt 0 ]; then
    find assets/data -iname "*.png" | while read f; do convert_image "$f" "png"; done
    find assets/data -iname "*.jpg" | while read f; do convert_image "$f" "jpg"; done
    find assets/data -iname "*.ogg" | while read f; do convert_sound "$f"; done
fi


if [ $CONVERT_TO_JPG -gt 0 ]; then
    rm -f "./converted_textures"
    
    find assets/data -not -path "assets/data/textures/*" -iname "*.png" | while read f; do convert_to_jpg "$f"; done

    find assets/data -iname "*.b3dz" | while read f; do convert_to_jpg_extract_b3dz "$f"; done
    find assets/data -iname "*.b3d" | while read f; do convert_to_jpg_update_b3d "$f"; done
    find assets/data -iname "*.spm" | while read f; do convert_to_jpg_update_spm "$f"; done
    find assets/data -iname "*.xml" | while read f; do convert_to_jpg_update_xml "$f"; done

    if [ -s "./converted_textures" ]; then
        echo "Converted textures:"
        cat "./converted_textures"
        rm -f "./converted_textures"
    fi
fi


# Copy data directory
echo "Copy data directory"
cp -a ../data/* assets/data/


# Remove unused files
for BLACKLIST_FILE in $BLACKLIST_FILES; do
    rm -f "assets/$BLACKLIST_FILE"
done


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
