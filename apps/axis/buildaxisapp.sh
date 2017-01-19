#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

BUILDDIR=`pwd`

AXIS_BUILDTYPE=$1

if [ $VERSION_MINOR != "" ]; then
    sed -ie 's/APPMINORVERSION.*/APPMINORVERSION=\"$VERSION_MINOR\"/g' $DIR/package.conf
fi

cd /opt/axis/emb-app-sdk_1_4
source ./init_env

mkdir -p $BUILDDIR/mipsisa32r2el-axis-linux-gnu
mkdir -p $BUILDDIR/armv6-axis-linux-gnueabi
mkdir -p $BUILDDIR/crisv32-axis-linux-gnu

if [ -e /usr/local/mipsisa32r2el/r12/bin/mipsisa32r2el-axis-linux-gnu-gcc ]; then

    export CC=/usr/local/mipsisa32r2el/r12/bin/mipsisa32r2el-axis-linux-gnu-gcc
    cd $BUILDDIR/mipsisa32r2el-axis-linux-gnu 
    cmake -DCMAKE_BUILD_TYPE=Release -DAXIS_BUILDTYPE=mipsisa32r2el-axis-linux-gnu $DIR
    make -j
    cd $BUILDDIR
    create-package.sh mipsisa32r2el
else
    echo "no mips compiler found skipping mips"
fi

if [ -e /usr/local/arm/r11/bin/arm-axis-linux-gnueabi-gcc ]; then
    export CC=/usr/local/arm/r11/bin/arm-axis-linux-gnueabi-gcc
    cd $BUILDDIR/armv6-axis-linux-gnueabi 
    cmake -DCMAKE_BUILD_TYPE=Release -DAXIS_BUILDTYPE=armv6-axis-linux-gnueabi $DIR
    make -j
    cd $BUILDDIR
    create-package.sh armv6
else
    echo "no arm compiler found skipping armv6"
fi

if [ -e /usr/local/crisv32/r94/bin/crisv32-axis-linux-gnu-gcc ]; then
    export CC=/usr/local/crisv32/r94/bin/crisv32-axis-linux-gnu-gcc
    cd $BUILDDIR/crisv32-axis-linux-gnu
    cmake -DCMAKE_BUILD_TYPE=Release -DAXIS_BUILDTYPE=crisv32-axis-linux-gnu $DIR
    make -j
    
    cd $BUILDDIR
    create-package.sh crisv32
else
    echo "No cris compiler found skipping crisv32"
fi

