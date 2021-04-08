#!/bin/bash

PROG="base64"
PROGOPT="--decode"
INPUT_PATTERN="inputs/utmp-fuzzed-%s.b64"
INPUT_CLEAN="inputs/utmp.b64"

echo "Building buggy ${PROG}..."

(cd coreutils-8.24-lava-safe && ./compile.sh)
#make clean &> /dev/null
#export CFLAGS="-D__GNU_LIBRARY__ -DSLOW_BUT_NO_HACKS $CFLAGS"
#./configure --prefix=`pwd`/lava-install LIBS="-lacl" &> /dev/null
#make -j $(nproc) &> /dev/null
#make install &> /dev/null
#cd ..

echo "Checking if buggy ${PROG} succeeds on non-trigger input..."
#./coreutils-8.24-lava-safe/lava-install/bin/${PROG} ${PROGOPT} ${INPUT_CLEAN} &> /dev/null 
./coreutils-8.24-lava-safe/base64 ${PROGOPT} ${INPUT_CLEAN} &> /dev/null 
rv=$?
if [ $rv -lt 128 ]; then
    echo "Success: ${PROG} ${PROGOPT} ${INPUT_CLEAN} returned $rv"
else
    echo "ERROR: ${PROG} ${PROGOPT} ${INPUT_CLEAN} returned $rv"
fi

echo "Validating bugs..."
cat validated_bugs | while read line ; do
    INPUT_FUZZ=$(printf "$INPUT_PATTERN" $line)
    #{ ./coreutils-8.24-lava-safe/lava-install/bin/${PROG} ${PROGOPT} ${INPUT_FUZZ} ; } &> /dev/null
    { ./coreutils-8.24-lava-safe/base64 ${PROGOPT} ${INPUT_FUZZ} ; } &> /dev/null
    echo $line $?
done > validated.txt

awk 'BEGIN {valid = 0} $2 > 128 { valid += 1 } END { print "Validated", valid, "/", NR, "bugs" }' validated.txt

echo "You can see validated.txt for the exit code of each buggy version."
