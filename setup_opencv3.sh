#!/bin/bash
SCRIPT_DIR=$(cd $(dirname $0); pwd)
INSTALL_DIR=${SCRIPT_DIR}/third_party/opencv3
TMPDIR=$(mktemp -d)

curl -o ${TMPDIR}/3.0.0.zip -L https://github.com/Itseez/opencv/archive/3.0.0.zip
pushd ${TMPDIR}
unzip 3.0.0.zip
pushd opencv-3.0.0
mkdir release
pushd release
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=${INSTALL_DIR} ..
make -j 128
make install
popd
popd
popd
rm -fr ${TMPDIR}
