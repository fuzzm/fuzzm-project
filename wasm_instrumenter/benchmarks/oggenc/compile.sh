SCRIPT=`realpath $0`
DIR=`dirname $SCRIPT`
emcc -g -s EXIT_RUNTIME=1 -s TOTAL_MEMORY=2000MB -s NODERAWFS ${DIR}/oggenc.c -o ${DIR}/oggenc.js
