#!/bin/sh

checkError()
{
  if [ "$1" -ne 0 ]
  then
    echo "*********************************************************************";
    echo "*********************SCRIPT FAIL DETAILS*****************************";
    echo "CI failure reason: $2"
    echo "Cause: $3"
    echo "Reproduction/How to fix: $4"
    echo "*********************************************************************";
    echo "*********************************************************************";
    exit 1
  fi
}

if [ "$TRAVIS_OS_NAME" = "linux" ]
then
    if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ]
    then
      echo "Ignoring install stage for $TRAVIS_EVENT_TYPE event";
      exit 0
    fi
fi

mkdir $TRAVIS_BUILD_DIR/logs
touch $TRAVIS_BUILD_DIR/logs/build_logs
BUILDLOGS=$TRAVIS_BUILD_DIR/logs/build_logs

if [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ;
then
  mkdir $TRAVIS_BUILD_DIR/logs/codecoverage
  checkError $? "unable to create codecoverage file" "could be permission issue" "Retry trigerring travis build"
  touch $TRAVIS_BUILD_DIR/logs/exec_logs
  checkError $? "unable to create exec logs file" "could be permission issue" "Retry trigerring travis build"
fi

if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ] ;
then
  mkdir $TRAVIS_BUILD_DIR/artifacts
  checkError $? "unable to create directory artifacts" "could be permission issue" "Retry trigerring travis build"
fi

echo "***************************** Building externals ****" > $BUILDLOGS
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/external
./build.sh>>$BUILDLOGS
checkError $? "building externals failed" "compilation error" "Need to build the externals directory locally in $TRAVIS_OS_NAME"
exit 0;
