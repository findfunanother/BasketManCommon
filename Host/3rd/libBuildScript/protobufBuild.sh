#!/bin/sh

# 아래 경로는 서드파티의 버전 디렉토리 이름에 맞춰주세요. ex)
cd ../

mkdir libprotobuf

cd libprotobuf

cmake -DCMAKE_POSITION_INDEPENDENT_CODE=ON -Dprotobuf_BUILD_TESTS=OFF ../protobuf-3.10.1/cmake  && make -j8 


