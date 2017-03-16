#!/bin/sh
BUILDLOGS=$TRAVIS_BUILD_DIR/logs/build_logs
cd $TRAVIS_BUILD_DIR/src
cd $TRAVIS_BUILD_DIR;
if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
echo "***************************** Building pxcore ****" >> $BUILDLOGS
xcodebuild clean
xcodebuild -scheme "pxCore Static Library">>$BUILDLOGS 2>&1;
else
echo "***************************** Building pxcore ****"
xcodebuild clean
xcodebuild -scheme "pxCore Static Library" 1>>$BUILDLOGS;
fi
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src;

if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
echo "***************************** Building pxscene app ***" >> $BUILDLOGS
make clean;
make -j>>$BUILDLOGS 2>&1
else
echo "***************************** Building pxscene app ***"
make clean;
make -j 1>>$BUILDLOGS
fi
