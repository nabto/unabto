#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

BUILDDIR=`pwd`

AXIS_BUILDTYPE=$1

if [ "x$VERSION_MINOR" != "x" ]; then
    sed -ie 's/APPMINORVERSION.*/APPMINORVERSION=\"$VERSION_MINOR\"/g' $DIR/package.conf
fi

echo using $AXIS_SDK as axis sdk directory.
cd $AXIS_SDK || exit 1
source ./init_env || exit 1

MIPS_TARGET=mipsisa32r2el-axis-linux-gnu
ARM_TARGET=armv6-axis-linux-gnueabi
ARMHF_TARGET=armv7-axis-linux-gnueabihf

mkdir -p $BUILDDIR/$MIPS_TARGET
mkdir -p $BUILDDIR/$ARM_TARGET
mkdir -p $BUILDDIR/$ARMHF_TARGET

MIPS_COMPILER=/usr/local/mipsisa32r2el/r23/bin/mipsisa32r2el-axis-linux-gnu-gcc
ARM_COMPILER=/usr/local/arm/r21/bin/arm-axis-linux-gnueabi-gcc
ARMHF_COMPILER=/usr/local/armhf/r27/bin/arm-axis-linux-gnueabihf-gcc


if [ -e $MIPS_COMPILER ]; then

    export CC=$MIPS_COMPILER
    cd $BUILDDIR/$MIPS_TARGET || exit 1
    cmake -DCMAKE_BUILD_TYPE=Release -DAXIS_BUILDTYPE=$MIPS_TARGET $DIR || exit 1
    make -j || exit 1
    cd $BUILDDIR || exit 1
    create-package.sh mipsisa32r2el || exit 1
else
    echo "no mips compiler found skipping mips"
fi

if [ -e $ARM_COMPILER ]; then
    export CC=$ARM_COMPILER
    cd $BUILDDIR/$ARM_TARGET || exit 1
    cmake -DCMAKE_BUILD_TYPE=Release -DAXIS_BUILDTYPE=$ARM_TARGET $DIR || exit 1
    make -j || exit 1 
    cd $BUILDDIR || exit 1
    create-package.sh armv6 || exit 1
else
    echo "no arm compiler found skipping armv6"
fi

if [ -e $ARMHF_COMPILER ]; then
    export CC=$ARMHF_COMPILER
    cd $BUILDDIR/$ARMHF_TARGET || exit 1
    cmake -DCMAKE_BUILD_TYPE=Release -DAXIS_BUILDTYPE=$ARMHF_TARGET $DIR || exit 1
    make -j || exit 1
    cd $BUILDDIR || exit 1
    create-package.sh $ARMHF_TARGET || exit 1
else
    echo "No cris compiler found skipping crisv32"
fi

