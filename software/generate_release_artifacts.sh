#!/bin/bash

FULL_PATH=$(readlink -f $0)
EXEC_NAME=$(basename $FULL_PATH)
SH_DIR=${FULL_PATH%"$EXEC_NAME"}

TAG_VERSION="undefined"
SRC_FOLDER=$SH_DIR
BUILD_FOLDER=$SH_DIR/build
ARTIFACTS=$SH_DIR/artifacts
ENV_FILE="env_vars"
IMAGE_BASE_NAME="AnyKeyFW"

usage()
{
  cat <<USAGE
Usage:
$EXEC_NAME [options]
  Option       Help                       Default
  --tag        Tag for artifact build     [ $TAG_VERSION ]
USAGE
}

build_and_copy()
{
  APPEND=$1
  echo "Building target ${IMAGE_BASE_NAME}_${APPEND} with the following environment:"
  cat $ENV_FILE
  echo " "
  make clean
  make all -j 8
  mv $BUILD_FOLDER/${IMAGE_BASE_NAME}.bin $ARTIFACTS/${IMAGE_BASE_NAME}_${APPEND}_${TAG_VERSION}.bin
  mv $BUILD_FOLDER/${IMAGE_BASE_NAME}.elf $ARTIFACTS/${IMAGE_BASE_NAME}_${APPEND}_${TAG_VERSION}.elf
}

while [[ $# -gt 0 ]] ; do
key="$1"
case $key in
  --tag)
  TAG_VERSION=$2
  shift ; shift
  ;;
  -h|--help)
  usage
  exit
  shift
  ;;
  *)
  BAD_OPT="$BAD_OPT $key"
  shift
  ;;
esac
done

[ "$BAD_OPT" ] && { echo "Unkown options: $BAD_OPT" ;  usage ; exit ; }
[ -d $ARTIFACTS ] || mkdir -p $ARTIFACTS

cd $SRC_FOLDER
cp ${ENV_FILE} ${ENV_FILE}_orig
sed -i 's/USE_DEBUG_BUILD=1/USE_DEBUG_BUILD=0/g' ${ENV_FILE}
sed -i 's/USE_MAPLEMINI_BOOTLOADER=1/USE_MAPLEMINI_BOOTLOADER=0/g' ${ENV_FILE}
sed -i 's/USE_CMD_SHELL=1/USE_CMD_SHELL=0/g' ${ENV_FILE}
build_and_copy "wo_bl_wo_cmd"
sed -i 's/USE_MAPLEMINI_BOOTLOADER=0/USE_MAPLEMINI_BOOTLOADER=1/g' ${ENV_FILE}
build_and_copy "w_bl_wo_cmd"
sed -i 's/USE_CMD_SHELL=0/USE_CMD_SHELL=1/g' ${ENV_FILE}
build_and_copy "w_bl_w_cmd"
sed -i 's/USE_MAPLEMINI_BOOTLOADER=1/USE_MAPLEMINI_BOOTLOADER=0/g' ${ENV_FILE}
build_and_copy "wo_bl_w_cmd"
mv ${ENV_FILE}_orig ${ENV_FILE} 
cd -

