SCRIPT=`realpath $0`
DIR=`dirname $SCRIPT`
emcc -g -s EXIT_RUNTIME=1 -s NODERAWFS ${DIR}/gzip.c -o ${DIR}/gzip.js
echo AAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBCCCCCCCCCCCCCCCC > ${DIR}/file.txt
