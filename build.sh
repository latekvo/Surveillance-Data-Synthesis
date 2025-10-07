#!/usr/bin/env zsh
# TODO: Use CMake

OPENCV_INCLUDE=`pkg-config --cflags opencv4`
RAYLIB_LINK="-lraylib"

gcc main.cpp $RAYLIB_LINK $OPENCV_INCLUDE -o app
