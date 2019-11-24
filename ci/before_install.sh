#!/bin/sh

travis_retry() {
  local result=0
  local count=1
  while [ $count -le 3 ]; do
    [ $result -ne 0 ] && {
      echo -e "\n$The command \"$@\" failed *******************. Retrying, $count of 3.\n" >&2
    }
    "$@"
    result=$?
    [ $result -eq 0 ] && break
    count=$(($count + 1))
    sleep 1
  done

  [ $count -gt 3 ] && {
    echo -e "\n$The command \"$@\" failed 3 times *******************.\n" >&2
  }

  return $result
}

#start the monitor
$TRAVIS_BUILD_DIR/ci/monitor.sh &

if [ "$TRAVIS_OS_NAME" = "linux" ]
then
    if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ] || [ ! -z "${TRAVIS_TAG}" ]
    then
      sudo apt-get install jq
      sudo apt-get install wget
      exit 0;
    fi
fi

#do the license check
if [ "$TRAVIS_OS_NAME" = "linux" ] ;
then
  $TRAVIS_BUILD_DIR/ci/licenseScanner.sh
  if [ "$?" != "0" ] 
  then
    printf "\n!*!*!* licenseScanner.sh detected files without proper license. Please refer to the logs above. !*!*!*\n"
    exit 1;
  fi
fi

#install necessary basic packages for linux and mac 
if [ "$TRAVIS_OS_NAME" = "linux" ] ; 
then 
  travis_retry sudo apt-get update
  travis_retry sudo apt-get install git libglew-dev freeglut3 freeglut3-dev libgcrypt11-dev zlib1g-dev g++ libssl-dev nasm autoconf valgrind libyaml-dev lcov cmake gdb quilt libuv-dev xmlto
fi

if [ "$TRAVIS_OS_NAME" = "osx" ] ;
then
  brew update;
  #brew upgrade cmake;
  brew install yasm bison flex 
  ln -sf /usr/local/opt/bison/bin/bison /usr/local/bin/bison
  brew install quilt
  brew install libuv
  brew install xmlto
  brew install pkg-config glfw3 glew
  echo "About to install openssl ............."
  ln -s /usr/local/opt/openssl/lib/libcrypto.1.0.0.dylib /usr/local/lib/
  ln -s /usr/local/opt/openssl/lib/libssl.1.0.0.dylib /usr/local/lib/
  #brew uninstall --ignore-dependencies python
  #brew install openssl
  #brew link openssl --force
  #brew uninstall python
  #brew install python --with-brewed-openssl
  sudo /usr/sbin/DevToolsSecurity --enable
  lldb --version
  lldb --help
  cmake --version
  man lldb
fi

#install lighttpd, code coverage binaries for mac
if [ "$TRAVIS_OS_NAME" = "osx" ] ; 
then
  if ( [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ) && [ -z "${TRAVIS_TAG}" ]
  then
#    brew install lighttpd
    brew install gcovr
    brew install lcov
  fi
fi

#setup lighttpd server
#if [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ]
#then
#  if [ "$TRAVIS_OS_NAME" = "linux" ] ;
#  then
#    sudo cp $TRAVIS_BUILD_DIR/tests/pxScene2d/supportfiles/* /var/www/.
#    sudo chmod -R 777 $TRAVIS_BUILD_DIR/tests/pxScene2d/supportfiles/
#    sudo chmod -R 777 /var/www
#    ls -lrt /etc/lighttpd/lighttpd.conf
#    sudo /etc/init.d/lighttpd stop
#    sudo sed -i "s/server.modules = (/server.modules = (\n\t\"mod_setenv\"\,/g" /etc/lighttpd/lighttpd.conf
#    echo "setenv.add-response-header += (\"Cache-Control\" => \"public, max-age=1000\")"|sudo tee -a /etc/lighttpd/lighttpd.conf
#    cat /etc/lighttpd/lighttpd.conf
#    sudo /etc/init.d/lighttpd start
#  elif [ "$TRAVIS_OS_NAME" = "osx" ] ;
#  then
#    brew services stop lighttpd
#    sudo mkdir -p /usr/local/var/www
#    sudo mkdir -p /var
#    sudo ln -s /usr/local/var/www /var/www
#    sudo cp $TRAVIS_BUILD_DIR/tests/pxScene2d/supportfiles/* /var/www/.
#    sudo chmod -R 777 $TRAVIS_BUILD_DIR/tests/pxScene2d/supportfiles/
#    sudo chmod -R 777 /var/www
#    sudo sed -i -n "s/server.port = 8080/server.port = 80/g" /usr/local/etc/lighttpd/lighttpd.conf
#    sudo sed -i -n "s/#  \"mod_setenv\"/   \"mod_setenv\"/g" /usr/local/etc/lighttpd/modules.conf
#    echo "setenv.add-response-header += (\"Cache-Control\" => \"public, max-age=1000\")"|sudo tee -a /usr/local/etc/lighttpd/modules.conf
#    echo "Displaying lighttpd file ***************************"
#    cat /usr/local/etc/lighttpd/lighttpd.conf
#    echo "Displaying modules.conf file ***************************"
#    cat /usr/local/etc/lighttpd/modules.conf
#    echo "Displaying modules.conf file completed ***************************"
#    sudo chmod -R 777 /usr/local
#    ls -lrt /usr/local/var/
#    sudo lighttpd -f /usr/local/etc/lighttpd/lighttpd.conf &
#    ps -aef|grep lighttpd
#    sudo netstat -a
#  fi
#fi


#install codecov
if ( [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ) && [ -z "${TRAVIS_TAG}" ]
then
	if [ "$TRAVIS_OS_NAME" = "osx" ] ; 
	then
		git clone https://github.com/pypa/pip 
		sudo easy_install pip
	elif [ "$TRAVIS_OS_NAME" = "linux" ] ;
	then
		sudo apt-get install python-pip
	fi
	sudo LD_LIBRARY_PATH=/usr/local/opt/openssl/lib pip install codecov
  #rm -rf /usr/local/opt/openssl/lib/libcrypto.1.0.0.dylib
  #rm -rf /usr/local/opt/openssl/lib/libssl.1.0.0.dylib
  #brew uninstall openssl
  codecov
fi

#setup spark externals repo
SRC_REPO_USER_NAME='pxscene'
DEST_REPO_USER_NAME=`echo $TRAVIS_REPO_SLUG | cut -d'/' -f 1`
cd $TRAVIS_BUILD_DIR
cd ../
mkdir rlExternals
cd rlExternals
git clone --branch=master https://github.com/$SRC_REPO_USER_NAME/Spark-Externals.git
cd Spark-Externals
if [ "$TRAVIS_OS_NAME" = "osx" ]; then
  sed -i -n "s/$SRC_REPO_USER_NAME/$DEST_REPO_USER_NAME/g" artifacts/$TRAVIS_OS_NAME/lib/pkgconfig/*
else
  sed -i "s/$SRC_REPO_USER_NAME/$DEST_REPO_USER_NAME/g" artifacts/$TRAVIS_OS_NAME/lib/pkgconfig/*
fi
ln -sf artifacts/$TRAVIS_OS_NAME extlibs
cd extlibs
mkdir lib_orig
cp -R lib/* lib_orig/.
if [ "$TRAVIS_OS_NAME" = "osx" ]; then
rm -rf lib_orig/libgif.7.dylib
rm -rf lib_orig/libpng.dylib
rm -rf lib_orig/libsqlite3.dylib
rm -rf lib_orig/libjpeg.dylib
fi
cd $TRAVIS_BUILD_DIR
cd ..
ln -sf rlExternals/Spark-Externals Spark-Externals
cd $TRAVIS_BUILD_DIR
