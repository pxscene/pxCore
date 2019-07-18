#!/bin/bash
set -e
# Any subsequent(*) commands which fail will cause the shell script to exit immediately

#--------- CURL 
cd curl

if [ "$(uname)" == "Darwin" ]; then
./configure --with-darwinssl
else
./configure
fi

make all -j3
cd ..

#--------- PNG 
cd png
./configure
make all -j3
cd ..

#--------- FT 

cd ft
export LIBPNG_LIBS="-L../png/.libs -lpng16"
./configure
make all -j3
cd ..

#--------- JPG 
cd jpg
./configure
make all -j3
cd ..

#--------- GIF
cd gif
[ -d patches ] || mkdir -p patches
[ -d patches/series ] || echo 'giflib-5.1.9.patch' >patches/series
cp ../giflib-5.1.9.patch patches/

if [[ "$#" -eq "1" && "$1" == "--clean" ]]; then
quilt pop -afq || test $? = 2
rm -rf .libs/*
elif [[ "$#" -eq "1" && "$1" == "--force-clean" ]]; then
git clean -fdx .
git checkout .
rm -rf .libs/*
else
quilt push -aq || test $? = 2
fi

make
[ -d .libs ] || mkdir -p .libs
if [ -e libgif.7.dylib ]
then
cp libgif.7.dylib .libs/libgif.7.dylib
cp libutil.7.dylib .libs/libutil.7.dylib

elif [ -e libgif.so ]
then
cp libgif.so .libs/libgif.so
cp libutil.so .libs/libutil.so
cp libgif.so libgif.so.7
cp libutil.so libutil.so.7
cp libgif.so .libs/libgif.so.7
cp libutil.so .libs/libutil.so.7
fi

cd ..

#--------- ZLIB
cd zlib
./configure
make all -j3
cd ..


#--------- LIBNODE 
cd libnode-v6.9.0
./configure --shared
make -j3
ln -sf libnode.so.48 out/Release/obj.target/libnode.so
ln -sf libnode.48.dylib out/Release/libnode.dylib
cd ..

#-------- NANOSVG

cd nanosvg
quilt push -aq 
cd ..

