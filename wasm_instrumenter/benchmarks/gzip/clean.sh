SCRIPT=`realpath $0`
DIR=`dirname $SCRIPT`
{
  rm ${DIR}/gzip.js
  rm ${DIR}/gzip.wasm
  rm ${DIR}/file*
} &> /dev/null
exit 0
