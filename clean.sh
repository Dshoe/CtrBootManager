#!/bin/sh
BUILD="build/"
RELEASE="release/"
if [ -d "$BUILD" ]; then
  rm -rf build/
fi
if [ -d "$RELEASE" ]; then
  rm -rf release/
fi
