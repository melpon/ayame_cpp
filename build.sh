#!/bin/bash

INSTALL_DIR="`pwd`/_install"
MODULE_PATH="`pwd`/cmake"

mkdir -p build
pushd build
  cmake .. \
    -DCLI11_ROOT_DIR="$INSTALL_DIR/CLI11" \
    -DJSON_ROOT_DIR="$INSTALL_DIR/json" \
    -DSPDLOG_ROOT_DIR="$INSTALL_DIR/spdlog" \
    -DCMAKE_PREFIX_PATH="$INSTALL_DIR/boost" \
    -DCMAKE_MODULE_PATH=$MODULE_PATH \
    "$@"
  make -j4
popd
