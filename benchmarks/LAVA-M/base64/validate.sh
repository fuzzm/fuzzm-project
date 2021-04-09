#!/bin/bash

PROG="base64"
PROGOPT="--decode"
INPUT_PATTERN="inputs/utmp-fuzzed-%s.b64"
INPUT_CLEAN="inputs/utmp.b64"

#echo "Building buggy ${PROG}..."

(cd coreutils-8.24-lava-safe && ./wasm-compile-and-instr)
#make clean &> /dev/null
#./configure --prefix=`pwd`/lava-install LIBS="-lacl" &> /dev/null
#make -j $(nproc) &> /dev/null
#make install &> /dev/null
#cd ..

echo "Checking if buggy ${PROG} succeeds on non-trigger input..."
#./coreutils-8.24-lava-safe/lava-install/bin/${PROG} ${PROGOPT} ${INPUT_CLEAN} &> /dev/null 
wasmtime ./coreutils-8.24-lava-safe/base64.wasm --dir=. -- ${PROGOPT} ${INPUT_CLEAN} &> /dev/null 
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
    { wasmtime ./coreutils-8.24-lava-safe/base64.wasm --dir=. -- ${PROGOPT} ${INPUT_FUZZ} ; } &> /dev/null
    echo $line $?
done > validated.txt

awk 'BEGIN {valid = 0} $2 > 128 { valid += 1 } END { print "Validated", valid, "/", NR, "bugs" }' validated.txt

echo "Validating bugs in instrumented program"
cat validated_bugs | while read line ; do
    INPUT_FUZZ=$(printf "$INPUT_PATTERN" $line)
    #{ ./coreutils-8.24-lava-safe/lava-install/bin/${PROG} ${PROGOPT} ${INPUT_FUZZ} ; } &> /dev/null
    { wasmtime ./coreutils-8.24-lava-safe/base64.instr.wasm --dir=. -- ${PROGOPT} ${INPUT_FUZZ} ; } &> /dev/null
    echo $line $?
done > validated-instr.txt

awk 'BEGIN {valid = 0} $2 > 128 { valid += 1 } END { print "Validated", valid, "/", NR, "bugs" }' validated-instr.txt

echo "You can see validated.txt and validated-instr.txt for the exit code of each buggy version."
