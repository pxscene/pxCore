#!/bin/bash

#minJS=cp        #don't minify
minJS=./jsMin.sh  #minify

externalDir=../external
bundle=pxscene.app
bundleBin=$bundle/Contents/MacOS

#bundleRes=$bundle/Contents/Resources
bundleRes=$bundle/Contents/MacOS
bundleCore=$bundleRes/rcvrcore
bundleUtils=$bundleCore/utils
bundleTools=$bundleCore/tools

bundleLib=$bundleBin/lib

rm -rf $bundle

# Make the bundle folder...
#
[ -d $bundleLib ] || mkdir -p $bundleLib

# Copy LIBS to Bundle...
#
cp $externalDir/png/.libs/libpng16.16.dylib $bundleLib
cp $externalDir/curl/lib/.libs/libcurl.4.dylib $bundleLib
#cp $externalDir/libnode/out/Release/libnode.dylib $bundleLib
cp $externalDir/libnode-v6.9.0/out/Release/libnode*.dylib $bundleLib
cp $externalDir/ft/objs/.libs/libfreetype.6.dylib $bundleLib
cp $externalDir/jpg/.libs/libjpeg.9.dylib $bundleLib

# Copy OTHER to Bundle...
#
cp macstuff/Info.plist $bundle/Contents

# Make the RES folders...
#
[ -d $bundleRes ] || mkdir -p $bundleRes
[ -d $bundleCore ] || mkdir -p $bundleCore
[ -d $bundleUtils ] || mkdir -p $bundleUtils
[ -d $bundleTools ] || mkdir -p $bundleTools

# Copy RESOURCES to Bundle...
#
cp -a browser $bundleRes
cp FreeSans.ttf $bundleRes

cp package.json $bundleRes
cp pxscene $bundleBin

cp macstuff/pxscene.sh $bundleBin
cp macstuff/EngineRunner $bundleBin

# Minify JS into Bundle...
#
# For Node
#./jsMinFolder.sh rcvrcore $bundleRes/rcvrcore
cp -a rcvrcore/* $bundleRes/rcvrcore


# NOTE" jsMin.sh will default to a 'min' name with 1 arg.  E.g.  "jsMin.sh INPUT.js"  >> INPUT.min.js
#
${minJS} init.js $bundleRes/init.js
${minJS} shell.js $bundleRes/shell.js
${minJS} browser.js $bundleRes/browser.js
${minJS} about.js $bundleRes/about.js
${minJS} browser/editbox.js $bundleRes/browser/editbox.js
#./jsMinFolder.sh browser $bundleRes/browser


# Copy duktape modules
cp -a duk_modules $bundleRes/duk_modules
# Copy node modules
cp -a node_modules $bundleRes/node_modules

# Copy OTHER to Resources...
#
cp -a macstuff/Resources $bundle/Contents/Resources
