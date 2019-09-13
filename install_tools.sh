#!/bin/bash

BUILD_DIR="`pwd`/_build"
INSTALL_DIR="`pwd`/_install"

set -ex

mkdir -p $BUILD_DIR
mkdir -p $INSTALL_DIR

BOOST_VERSION="1.71.0"
BOOST_VERSION_FILE="$INSTALL_DIR/boost.version"
BOOST_CHANGED=0
if [ ! -e $BOOST_VERSION_FILE -o "$BOOST_VERSION" != "`cat $BOOST_VERSION_FILE`" ]; then
  BOOST_CHANGED=1
fi

JSON_VERSION="3.6.1"
JSON_VERSION_FILE="$INSTALL_DIR/json.version"
JSON_CHANGED=0
if [ ! -e $JSON_VERSION_FILE -o "$JSON_VERSION" != "`cat $JSON_VERSION_FILE`" ]; then
  JSON_CHANGED=1
fi

CLI11_VERSION="1.8.0"
CLI11_VERSION_FILE="$INSTALL_DIR/cli11.version"
CLI11_CHANGED=0
if [ ! -e $CLI11_VERSION_FILE -o "$CLI11_VERSION" != "`cat $CLI11_VERSION_FILE`" ]; then
  CLI11_CHANGED=1
fi

SPDLOG_VERSION="1.3.1"
SPDLOG_VERSION_FILE="$INSTALL_DIR/spdlog.version"
SPDLOG_CHANGED=0
if [ ! -e $SPDLOG_VERSION_FILE -o "$SPDLOG_VERSION" != "`cat $SPDLOG_VERSION_FILE`" ]; then
  SPDLOG_CHANGED=1
fi


# boost
if [ $BOOST_CHANGED -eq 1 -o ! -e $INSTALL_DIR/boost/lib/libboost_system.a ]; then
  _VERSION_UNDERSCORE=${BOOST_VERSION//./_}
  _URL=https://dl.bintray.com/boostorg/release/${BOOST_VERSION}/source/boost_${_VERSION_UNDERSCORE}.tar.gz
  _FILE=$BUILD_DIR/boost_${_VERSION_UNDERSCORE}.tar.gz
  if [ ! -e $_FILE ]; then
    echo "file(DOWNLOAD $_URL $_FILE)" > $BUILD_DIR/tmp.cmake
    cmake -P $BUILD_DIR/tmp.cmake
    rm $BUILD_DIR/tmp.cmake
  fi
  pushd $BUILD_DIR
    rm -rf boost_${_VERSION_UNDERSCORE}
    cmake -E tar xf $_FILE

    pushd boost_${_VERSION_UNDERSCORE}
      ./bootstrap.sh
      ./b2 install --prefix=$INSTALL_DIR/boost --build-dir=build link=static --with-system
    popd
  popd
fi
echo $BOOST_VERSION > $BOOST_VERSION_FILE

# json
if [ $JSON_CHANGED -eq 1 -o ! -e $INSTALL_DIR/json/include/nlohmann/json.hpp ]; then
  _URL=https://github.com/nlohmann/json/releases/download/v${JSON_VERSION}/include.zip
  _FILE=$BUILD_DIR/json.zip
  if [ ! -e $_FILE ]; then
    echo "file(DOWNLOAD $_URL $_FILE)" > $BUILD_DIR/tmp.cmake
    cmake -P $BUILD_DIR/tmp.cmake
    rm $BUILD_DIR/tmp.cmake
  fi
  pushd $INSTALL_DIR
    rm -rf json include
    mkdir json
    cmake -E tar xf $_FILE
    mv include/ json/include/
  popd
fi
echo $JSON_VERSION > $JSON_VERSION_FILE

# CLI11
if [ $CLI11_CHANGED -eq 1 -o  ! -e $INSTALL_DIR/CLI11/include ]; then
  rm -rf $INSTALL_DIR/CLI11
  git clone --branch v$CLI11_VERSION --depth 1 https://github.com/CLIUtils/CLI11.git $INSTALL_DIR/CLI11
fi
echo $CLI11_VERSION > $CLI11_VERSION_FILE

# spdlog
if [ $SPDLOG_CHANGED -eq 1 -o  ! -e $INSTALL_DIR/spdlog/include ]; then
  rm -rf $INSTALL_DIR/spdlog
  git clone --branch v$SPDLOG_VERSION --depth 1 https://github.com/gabime/spdlog.git $INSTALL_DIR/spdlog
fi
echo $SPDLOG_VERSION > $SPDLOG_VERSION_FILE
