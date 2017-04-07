#!/bin/bash


DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

BUILDDIR=`pwd`

MIPS_TARGET=mipsisa32r2el-axis-linux-gnu
ARM_TARGET=armv6-axis-linux-gnueabi
ARMHF_TARGET=armv7-axis-linux-gnueabihf

rm -rf $BUILDDIR/$MIPS_TARGET
rm -rf $BUILDDIR/$ARM_TARGET
rm -rf $BUILDDIR/$ARMHF_TARGET
rm -f $DIR/*.eap $DIR/*.eap.old
