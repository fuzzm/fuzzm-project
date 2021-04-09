SCRIPT=`realpath $0`
DIR=`dirname $SCRIPT`
{
  rm ${DIR}/oggenc.js
  rm ${DIR}/oggenc.wasm
  rm ${DIR}/*.ogg
} &> /dev/null
exit 0
