#!/bin/bash

PROG="md5sum"
PROGOPT="-c"
INPUT_PATTERN="inputs/bin-ls-md5s-fuzzed-%s"
INPUT_CLEAN="inputs/bin-ls-md5s"

#echo "Building buggy ${PROG}..."

(cd coreutils-8.24-lava-safe && ./compile.sh)
#make clean &> /dev/null
#./configure --prefix=`pwd`/lava-install LIBS="-lacl" &> /dev/null
#make -j $(nproc) &> /dev/null
#make install &> /dev/null
#cd ..

echo "Checking if buggy ${PROG} succeeds on non-trigger input..."
./coreutils-8.24-lava-safe/${PROG} ${PROGOPT} ${INPUT_CLEAN} &> /dev/null 
rv=$?
if [ $rv -lt 128 ]; then
    echo "Success: ${PROG} ${PROGOPT} ${INPUT_CLEAN} returned $rv"
else
    echo "ERROR: ${PROG} ${PROGOPT} ${INPUT_CLEAN} returned $rv"
fi

echo "Validating bugs..."
cat validated_bugs | while read line ; do
    INPUT_FUZZ=$(printf "$INPUT_PATTERN" $line)
    { ./coreutils-8.24-lava-safe/${PROG} ${PROGOPT} ${INPUT_FUZZ} ; } &> /dev/null
    echo $line $?
done > validated.txt

awk 'BEGIN {valid = 0} $2 > 128 { valid += 1 } END { print "Validated", valid, "/", NR, "bugs" }' validated.txt

echo "You can see validated.txt for the exit code of each buggy version."
