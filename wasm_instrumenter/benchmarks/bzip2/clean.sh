SCRIPT=`realpath $0`
DIR=`dirname $SCRIPT`
{
  rm ${DIR}/bzip2.js
  rm ${DIR}/bzip2.wasm
  rm ${DIR}/file.*
} &> /dev/null
exit 0
