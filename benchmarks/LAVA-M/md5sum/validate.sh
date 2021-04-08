#!/bin/bash

PROG="md5sum"
PROGOPT="-c"
INPUT_PATTERN="inputs/bin-ls-md5s-fuzzed-%s"
INPUT_CLEAN="inputs/bin-ls-md5s"

(cd coreutils-8.24-lava-safe && ./wasm-compile-and-instr)

#cd coreutils-8.24-lava-safe
#make clean &> /dev/null
#./configure --prefix=`pwd`/lava-install LIBS="-lacl" &> /dev/null
#make -j $(nproc) &> /dev/null
#make install &> /dev/null
#cd ..

echo "Checking if buggy ${PROG} succeeds on non-trigger input..."
#./coreutils-8.24-lava-safe/lava-install/bin/${PROG} ${PROGOPT} ${INPUT_CLEAN} &> /dev/null 
wasmtime ./coreutils-8.24-lava-safe/md5sum.wasm --dir=. -- ${PROGOPT} ${INPUT_CLEAN} &> /dev/null 
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
    { wasmtime ./coreutils-8.24-lava-safe/md5sum.wasm --dir=/bin --dir=. -- ${PROGOPT} ${INPUT_FUZZ} ; } &> /dev/null
    echo $line $?
done > validated.txt

awk 'BEGIN {valid = 0} $2 > 128 { valid += 1 } END { print "Validated", valid, "/", NR, "bugs" }' validated.txt

echo "Validating bugs in instrumented program..."
cat validated_bugs | while read line ; do
    INPUT_FUZZ=$(printf "$INPUT_PATTERN" $line)
    #{ ./coreutils-8.24-lava-safe/lava-install/bin/${PROG} ${PROGOPT} ${INPUT_FUZZ} ; } &> /dev/null
    { wasmtime ./coreutils-8.24-lava-safe/md5sum.instr.wasm --dir=/bin --dir=. -- ${PROGOPT} ${INPUT_FUZZ} ; } &> /dev/null
    echo $line $?
done > validated-instr.txt

awk 'BEGIN {valid = 0} $2 > 128 { valid += 1 } END { print "Validated", valid, "/", NR, "bugs" }' validated-instr.txt


echo "You can see validated.txt validated-instr.txt for the exit code of each buggy version."
