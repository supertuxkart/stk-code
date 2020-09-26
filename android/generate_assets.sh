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
# The script needs imagemagick, pngquant and ogg utils installed in order to 
# use DECREASE_QUALITY feature

################################################################################

export KARTS_DEFAULT="all"
export TRACKS_DEFAULT="all"

export TEXTURE_SIZE_DEFAULT=256
export JPEG_QUALITY_DEFAULT=85
export PNG_QUALITY_DEFAULT=95
export PNGQUANT_QUALITY_DEFAULT=90
export SOUND_QUALITY_DEFAULT=42
export SOUND_MONO_DEFAULT=1
export SOUND_SAMPLE_DEFAULT=32000

export RUN_OPTIMIZE_SCRIPT_DEFAULT=0
export DECREASE_QUALITY_DEFAULT=1
export CONVERT_TO_JPG_DEFAULT=1
export ONLY_ASSETS_DEFAULT=0

export ASSETS_PATHS_DEFAULT="../data                    \
                             ../../stk-assets           \
                             ../../supertuxkart-assets"
                             
export OUTPUT_PATH_DEFAULT="assets"

export ASSETS_DIRS="library models music sfx textures"

export CONVERT_TO_JPG_BLACKLIST="data/models/traffic_light.png"

export BLACKLIST_FILES="data/supertuxkart.icns \
                        data/supertuxkart_1024.png \
                        data/supertuxkart_128.png \
                        data/supertuxkart_16.png \
                        data/supertuxkart_256.png \
                        data/supertuxkart_32.png \
                        data/supertuxkart_48.png \
                        data/supertuxkart_512.png \
                        data/supertuxkart_64.png"

################################################################################

export LANG=C

cd "`dirname "$0"`"

# Set default configuration if not changed outside of the script
if [ -z "$KARTS" ]; then
    export KARTS="$KARTS_DEFAULT"
fi

if [ -z "$TRACKS" ]; then
    export TRACKS="$TRACKS_DEFAULT"
fi

if [ -z "$TEXTURE_SIZE" ]; then
    export TEXTURE_SIZE="$TEXTURE_SIZE_DEFAULT"
fi

if [ -z "$JPEG_QUALITY" ]; then
    export JPEG_QUALITY="$JPEG_QUALITY_DEFAULT"
fi

if [ -z "$PNG_QUALITY" ]; then
    export PNG_QUALITY="$PNG_QUALITY_DEFAULT"
fi

if [ -z "$PNGQUANT_QUALITY" ]; then
    export PNGQUANT_QUALITY="$PNGQUANT_QUALITY_DEFAULT"
fi

if [ -z "$SOUND_QUALITY" ]; then
    export SOUND_QUALITY="$SOUND_QUALITY_DEFAULT"
fi

if [ -z "$SOUND_MONO" ]; then
    export SOUND_MONO="$SOUND_MONO_DEFAULT"
fi

if [ -z "$SOUND_SAMPLE" ]; then
    export SOUND_SAMPLE="$SOUND_SAMPLE_DEFAULT"
fi

if [ -z "$RUN_OPTIMIZE_SCRIPT" ]; then
    export RUN_OPTIMIZE_SCRIPT="$RUN_OPTIMIZE_SCRIPT_DEFAULT"
fi

if [ -z "$DECREASE_QUALITY" ]; then
    export DECREASE_QUALITY="$DECREASE_QUALITY_DEFAULT"
fi

if [ -z "$CONVERT_TO_JPG" ]; then
    export CONVERT_TO_JPG="$CONVERT_TO_JPG_DEFAULT"
fi

if [ -z "$ONLY_ASSETS" ]; then
    export ONLY_ASSETS="$ONLY_ASSETS_DEFAULT"
fi

if [ -z "$ASSETS_PATHS" ]; then
    export ASSETS_PATHS="$ASSETS_PATHS_DEFAULT"
fi

if [ -z "$OUTPUT_PATH" ]; then
    export OUTPUT_PATH="$OUTPUT_PATH_DEFAULT"
fi

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
rm -rf "$OUTPUT_PATH"


# Copy all assets
echo "Copy all assets"

mkdir -p "$OUTPUT_PATH/data"

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
        cp -a "$ASSETS_PATH/$DIR" "$OUTPUT_PATH/data/"
    fi
done;


# Copy selected tracks
echo "Copy selected tracks"

mkdir -p "$OUTPUT_PATH/data/tracks"

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
        cp -a "$ASSETS_PATH/tracks/$DIR" "$OUTPUT_PATH/data/tracks/"
    fi
done


# Copy selected karts
echo "Copy selected karts"

mkdir -p "$OUTPUT_PATH/data/karts"

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
        cp -a "$ASSETS_PATH/karts/$DIR" "$OUTPUT_PATH/data/karts/"
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
    
    SCALE_CMD=""
    QUALITY_CMD=""

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

optimize_png()
{
    FILE="$1"
    echo "Optimize file: $FILE"

    if [ ! -f "$FILE" ]; then
        echo "  File doesn't exist."
        return
    fi
    
    pngquant --force --skip-if-larger --quality 0-$PNGQUANT_QUALITY --output "$FILE" -- "$FILE"
}

convert_to_jpg()
{
    FILE="$1"
    echo "Convert file to jpg: $FILE"

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
        if [ "$FILE" = "$OUTPUT_PATH/$BLACKLIST_FILE" ]; then
            BLACKLISTED=1
            break
        fi
    done
    
    if [ $BLACKLISTED -eq 1 ]; then
        #echo "  File is blacklisted. Ignore..."
        return
    fi

    FILE_EXTENSION=`echo "$FILE" | tail -c 5`

    if [ `echo "$FILE_EXTENSION" | head -c 1` != "." ]; then
        #echo "  Unsupported file extension. Ignore..."
        return
    fi

    FILE_FORMAT=`identify -format %m "$FILE"`

    if [ "$FILE_FORMAT" = "JPEG" ]; then
        #echo "  File is already JPEG. Ignore..."
        return
    fi

    IS_OPAQUE=`identify -format '%[opaque]' "$FILE"`
    #HAS_ALPHA=`identify -format '%A' "$FILE"`

    if [ "$IS_OPAQUE" = "False" ] || [ "$IS_OPAQUE" = "false" ]; then
        #echo "  File has alpha channel. Ignore..."
        return
    fi

    DIRNAME="`dirname "$FILE"`"
    BASENAME="`basename "$FILE"`"     
    IS_GLOSS_MAP=`find "$DIRNAME" -iname "*.xml" -exec cat {} \; \
                                            | grep -c "gloss-map=\"$BASENAME\""`
    
    if [ $IS_GLOSS_MAP -gt 0 ]; then
        #echo "  File is a gloss-map. Ignore..."
        return
    fi

    NEW_FILE="`echo $FILE | head -c -5`.jpg"

    if [ -f "$NEW_FILE" ]; then
        #echo "  There is already a file with .jpg extension. Ignore..."
        return
    fi

    # We can check if new file is smaller
    convert -quality $JPEG_QUALITY "$FILE" "$NEW_FILE"
    rm -f "$FILE"

    echo "$FILE" >> "./converted_textures"
}

convert_to_jpg_extract_b3dz()
{
    FILE="$1"
    echo "Extract b3dz file: $FILE"

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
    echo "Update b3d file: $FILE"

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
    echo "Update spm file: $FILE"

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
    echo "Update xml file: $FILE"

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
    find "$OUTPUT_PATH/data" -iname "*.png" | while read f; do convert_image "$f" "png"; done
    find "$OUTPUT_PATH/data" -iname "*.jpg" | while read f; do convert_image "$f" "jpg"; done
    find "$OUTPUT_PATH/data" -iname "*.ogg" | while read f; do convert_sound "$f"; done
fi


if [ $CONVERT_TO_JPG -gt 0 ]; then
    rm -f "./converted_textures"
    
    find "$OUTPUT_PATH/data" -not -path "$OUTPUT_PATH/data/textures/*" \
                             -not -path "$OUTPUT_PATH/data/karts/*"    \
                             -iname "*.png" | while read f; do convert_to_jpg "$f"; done

    find "$OUTPUT_PATH/data" -iname "*.b3dz" | while read f; do convert_to_jpg_extract_b3dz "$f"; done
    find "$OUTPUT_PATH/data" -iname "*.b3d" | while read f; do convert_to_jpg_update_b3d "$f"; done
    find "$OUTPUT_PATH/data" -iname "*.spm" | while read f; do convert_to_jpg_update_spm "$f"; done
    find "$OUTPUT_PATH/data" -iname "*.xml" | while read f; do convert_to_jpg_update_xml "$f"; done

    if [ -s "./converted_textures" ]; then
        echo "Converted textures:"
        cat "./converted_textures"
        rm -f "./converted_textures"
    fi
fi

if [ $DECREASE_QUALITY -gt 0 ]; then
    find "$OUTPUT_PATH/data" -iname "*.png" | while read f; do optimize_png "$f" "png"; done
fi


# Copy data directory
if [ $ONLY_ASSETS -eq 0 ]; then
    echo "Copy data directory"
    cp -a ../data/* "$OUTPUT_PATH/data"
fi


# Remove unused files
for BLACKLIST_FILE in $BLACKLIST_FILES; do
    rm -f "$OUTPUT_PATH/$BLACKLIST_FILE"
done


# Run optimize_data.sh script
if [ $RUN_OPTIMIZE_SCRIPT -gt 0 ]; then
    echo "Run optimize_data.sh script"
    sh -c "cd "$OUTPUT_PATH/data"; ../../../data/optimize_data.sh"
fi


# Generate files list
echo "Generate files list"
find "$OUTPUT_PATH"/* -type d| sort > tmp1.txt
sed -i 's/$/\//' tmp1.txt
find "$OUTPUT_PATH"/* -type f| sort > tmp2.txt
cat tmp1.txt tmp2.txt | sort > "$OUTPUT_PATH/files.txt"
rm tmp1.txt tmp2.txt
sed -i s/".\/$OUTPUT_PATH\/"// "$OUTPUT_PATH/files.txt"
sed -i s/"$OUTPUT_PATH\/"// "$OUTPUT_PATH/files.txt"

# A file that can be used to check if apk has assets
echo "has_assets" > "$OUTPUT_PATH/has_assets.txt"


# It will be probably ignored by ant, but create it anyway...
touch "$OUTPUT_PATH/.nomedia"


echo "Done."
exit 0
