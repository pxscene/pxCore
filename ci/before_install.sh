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

if [ "$TRAVIS_OS_NAME" = "linux" ]
then
    if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ]
    then
      echo "Ignoring before install stage for $TRAVIS_EVENT_TYPE event";
      exit 0;
    fi
fi

#install necessary basic packages for linux and mac 
if [ "$TRAVIS_OS_NAME" = "linux" ] ; 
then 
  travis_retry sudo apt-get update
  travis_retry sudo apt-get install git libglew-dev freeglut3 freeglut3-dev libgcrypt11-dev zlib1g-dev g++ libssl-dev nasm autoconf valgrind libyaml-dev lcov cmake x11proto-input-dev lighttpd gdb
  cd /usr/include/X11/extensions && sudo ln -s XI.h XInput.h
fi

if [ "$TRAVIS_OS_NAME" = "osx" ] ;
then
  brew update;
  sudo /usr/sbin/DevToolsSecurity --enable
  lldb --version
  lldb --help
  man lldb
fi

#install lighttpd, code coverage binaries for mac
if [ "$TRAVIS_OS_NAME" = "osx" ] ; 
then
  if [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ]
  then
    brew install lighttpd
    brew install gcovr
    brew install lcov
    brew install --HEAD ccache
    ls -al $HOME/.ccache
  fi
fi

#setup lighttpd server
if [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ]
then
  if [ "$TRAVIS_OS_NAME" = "linux" ] ;
  then
    sudo cp $TRAVIS_BUILD_DIR/tests/pxScene2d/supportfiles/* /var/www/.
    ls -lrt /etc/lighttpd/lighttpd.conf
    sudo /etc/init.d/lighttpd stop
    sudo sed -i "s/server.modules = (/server.modules = (\n\t\"mod_setenv\"\,/g" /etc/lighttpd/lighttpd.conf
    echo "setenv.add-response-header += (\"Cache-Control\" => \"public, max-age=1000\")"|sudo tee -a /etc/lighttpd/lighttpd.conf
    cat /etc/lighttpd/lighttpd.conf
    sudo /etc/init.d/lighttpd start
  elif [ "$TRAVIS_OS_NAME" = "osx" ] ;
  then
    brew services stop lighttpd
    sudo mkdir -p /usr/local/var/www
    sudo mkdir -p /var
    sudo ln -s /usr/local/var/www /var/www
    sudo cp $TRAVIS_BUILD_DIR/tests/pxScene2d/supportfiles/* /var/www/.
    sudo sed -i -n "s/#  \"mod_setenv\"/   \"mod_setenv\"/g" /usr/local/etc/lighttpd/modules.conf
    echo "setenv.add-response-header += (\"Cache-Control\" => \"public, max-age=1000\")"|sudo tee -a /usr/local/etc/lighttpd/modules.conf
    find / -name lighttpd.conf|xargs cat
    cat /usr/local/etc/lighttpd/modules.conf
    brew services start lighttpd
    brew services reload lighttpd
    ps -aef|grep lighttpd
  fi
fi

#install codecov
if [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ]
then
if [ "$TRAVIS_OS_NAME" = "osx" ] ; 
then
  git clone https://github.com/pypa/pip 
  sudo easy_install pip
elif [ "$TRAVIS_OS_NAME" = "linux" ] ;
then
  sudo apt-get install python-pip
fi
  sudo pip install codecov
fi
