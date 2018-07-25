#!/bin/bash


function mkUpdate() {

INFILE=macstuff/software_update.plist
OUTFILE=deploy/mac/software_update.plist
DMGFILE=deploy/mac/pxscene.dmg

SIZE=`stat -f "%z" deploy/mac/pxscene.dmg`
HASH=`openssl sha1 -binary "${DMGFILE}" | openssl base64`
VERS=$1

printf "Updating ${FILE} for ${1}\n"
m4 -D__HASH__="${HASH}" -D__SIZE__="${SIZE}" -D__VERSION__="${VERS}" ${INFILE} > ${OUTFILE}

}


DEPLOY_DIR=deploy/mac/.stage
PX_SCENE_VERSION=$1
mkdir -p $DEPLOY_DIR
rm -r $DEPLOY_DIR/pxscene.app
if [ "$TRAVIS_EVENT_TYPE" == "cron" ]
then
cp -a pxsceneEdge.app $DEPLOY_DIR
echo $PX_SCENE_VERSION > $DEPLOY_DIR/pxsceneEdge.app/Contents/MacOS/version
else
cp -a pxscene.app $DEPLOY_DIR
echo $PX_SCENE_VERSION > $DEPLOY_DIR/pxscene.app/Contents/MacOS/version
fi

#build dmg
./mkdmg.sh
mkUpdate $PX_SCENE_VERSION
