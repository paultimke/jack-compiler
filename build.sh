#!/bin/bash

BUILD_DIR=build

function buildDebug() {
    CURR_DIR=$(pwd)
    cmake -S . -B ${BUILD_DIR} -DCMAKE_BUILD_TYPE=Debug || return $?
    cmake --build ${BUILD_DIR} || return $?
    cd ${CURR_DIR}
}

if [[ $# == 0 ]]; then
    buildDebug
elif [[ $# == 1 ]]; then
    if [[ $1 == "clean" ]]; then
        rm -rf ${BUILD_DIR}

    elif [[ $1 == "release" ]]; then
        cmake -S . -B ${BUILD_DIR} -DCMAKE_BUILD_TYPE=Release || exit $?
        cmake --build ${BUILD_DIR} || exit $?
        cd ..
    else
        echo "Unrecognized command $1"
        exit 1
    fi
else
    echo "run as ./build.sh or ./build.sh <arg>"
    exit 1
fi