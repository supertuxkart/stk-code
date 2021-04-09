. "${BASH_SOURCE%/*}"/devkita64.sh

export PORTLIBS_PREFIX=${DEVKITPRO}/portlibs/switch
export PATH=$PORTLIBS_PREFIX/bin:$PATH

export ARCH="-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec"
export CFLAGS="${ARCH} -O2 -ffunction-sections -fdata-sections"
export CXXFLAGS="${CFLAGS}"
export CPPFLAGS="-D__SWITCH__ -I ${PORTLIBS_PREFIX}/include -isystem ${DEVKITPRO}/libnx/include"
export LDFLAGS="${ARCH} -L${PORTLIBS_PREFIX}/lib -L${DEVKITPRO}/libnx/lib"
export LIBS="-lnx"
