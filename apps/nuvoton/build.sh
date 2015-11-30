#!/bin/bash
#
# KEIL UV4 commands see http://www.keil.com/support/man/docs/uv4/uv4_commandline.htm
#

KEIL_EXE="/cygdrive/c/Keil_v5/UV4/UV4.exe"
OUTPUT_FILE="KEIL/obj/*.bin"

echo -e "Building project using KEIL UV4...\n"

if [ ! -f $KEIL_EXE ]; then
  echo "Invalid path to KEIL UV4 executable"
  exit 1
fi

rm -rf KEIL/obj
$KEIL_EXE -b KEIL/NuMaker_TRIO.uvproj -j0 -o output.txt
cat KEIL/output.txt

echo -e "\n"

if [ -f $OUTPUT_FILE ]; then
  echo "BUILD SUCCEEDED"
else
  echo "BUILD FAILED: Missing output file"
  exit 1
fi
