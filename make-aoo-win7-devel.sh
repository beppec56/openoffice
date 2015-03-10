#!/bin/bash

#set -x

DATESET=`date +%Y%m%d-%H%M%S`
LOGFILE="make-aoo-$DATESET.log"

date > ../../$LOGFILE

echo 'Grab Environment'
. ./*.Set.sh

echo "$SOLARENV"

cd $SOLARSRC/instsetoo_native || exit 1;
echo "Starting build"

echo "log file: "
echo "grep \"Building\" ../$LOGFILE ; tail -f ../$LOGFILE | grep \"Building\""
echo ""

perl $SOLARENV/bin/build.pl --html -P1 --all -- -P1 >> ../../$LOGFILE 2>&1 &

echo "see file at:  $SOLARSRC/../log/unxlngx6.pro.build.html"
