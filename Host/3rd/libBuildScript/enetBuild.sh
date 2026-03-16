#!/bin/sh

# 아래 경로는 서드파티의 버전 디렉토리 이름에 맞춰주세요. ex) enet-1.2.1 뭐 이런식으로
cd ../enet-1.3.13

BUILD_PATH=$(pwd)

echo "Build Path : "$BUILD_PATH

autoreconf -vfi

make clean

./configure --libdir=$BUILD_PATH/lib

make

make install

