# cmake 요구 버전
CMAKE_MINIMUM_REQUIRED(VERSION 3.15)

# 프로젝트 이름 설정
PROJECT(F4ObserverHost)
MESSAGE("======================================== ProjectName : ${CMAKE_PROJECT_NAME}")

# 빌드 타입
#set(CMAKE_BUILD_TYPE Release)
SET(CMAKE_BUILD_TYPE Debug)
MESSAGE("======================================== BuildType : ${CMAKE_BUILD_TYPE}")

SET (CMAKE_VERBOSE_MAKEFILE true )
MESSAGE("======================================== VerboseMakefile : ${CMAKE_VERBOSE_MAKEFILE}")
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../Host/protobuf DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
# proto generate
FIND_PACKAGE(Protobuf 3.00.0.0 REQUIRED HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../Host/3rd/libprotobuf)
PROTOBUF_GENERATE(LANGUAGE cpp OUT_VAR PROTO_SRCS PROTOS ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/Message.proto)

#INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/../host)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(PROTOBUF_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/../Host/3rd/protobuf-3.10.1/src)
SET(HOST_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../Host/host)
INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${HOST_SOURCE_DIR})



# 컴파일러
if (WIN32)
	MESSAGE("======================================== Platform : Windows")
elseif (UNIX AND NOT APPLE)
	MESSAGE("======================================== Platform : Linux")
	SET(CMAKE_C_COMPILER clang)
	SET(CMAKE_CXX_COMPILER clang++)
else()
	MESSAGE("======================================== Platform : Unix or Apple")
	SET(CMAKE_C_COMPILER gcc)
	SET(CMAKE_CXX_COMPILER g++)
endif()

if(Protobuf_FOUND)
MESSAGE("======================================== protobuf FOUND")
else()
MESSAGE("======================================== protobuf NOT_FOUND")
endif()

STRING(TIMESTAMP BUILDTIME  "%Y%m%d%H%M")

# 빌드 형상 옵션
SET(CMAKE_C_FLAGS 			"-std=gnu99 -g3 -O2 -Wall")
SET(CMAKE_C_FLAGS_DEBUG 	"-std=gnu99 -g3 -O0 -Wall")
SET(CMAKE_C_FLAGS_RELEASE 	"-std=gnu99 -g0 -O3 -Wall")
SET(CMAKE_CXX_FLAGS 		"-std=c++11 -g3 -O2 -Wall -DBUILDTIME=${BUILDTIME}")
SET(CMAKE_CXX_FLAGS_DEBUG 	"-std=c++11 -g3 -O0 -Wall -DBUILDTIME=${BUILDTIME}")
SET(CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -g0 -O3 -Wall -DBUILDTIME=${BUILDTIME}")

set(CMAKE_INSTALL_RPATH "$ORIGIN")#library path

#ADD_COMPILE_OPTIONS(-Wall -g -c -fPIC -fvisibility=hidden)

MESSAGE("======================================== C Compiler : ${CMAKE_C_COMPILER}")
MESSAGE("======================================== C++ Compiler : ${CMAKE_CXX_COMPILER}")
MESSAGE("======================================== CMAKE_SOURCE_DIR : ${CMAKE_SOURCE_DIR}")
MESSAGE("======================================== CMAKE_BINARY_DIR : ${CMAKE_BINARY_DIR}")
MESSAGE("======================================== CMAKE_CURRENT_SOURCE_DIR : ${CMAKE_CURRENT_SOURCE_DIR}")
MESSAGE("======================================== CMAKE_BINARY_DIR : ${CMAKE_BINARY_DIR}")
MESSAGE("======================================== HOST_DIR : ${HOST_SOURCE_DIR}")

FILE(GLOB_RECURSE OBHOST_SRCS ${CMAKE_SOURCE_DIR}
"*.h"
"*.cpp"
"../Host/host/ProtoBufProtocol.h"
"../Host/host/ProtoBufProtocol.cpp"
"../Host/host/ProtoBufPacketImplement.h"
"../Host/host/ProtoBufPacketImplement.cpp"
)

ADD_LIBRARY(ob_host SHARED ${OBHOST_SRCS} ${PROTO_SRCS} )

TARGET_LINK_LIBRARIES(ob_host protobuf::libprotobuf)

SET_TARGET_PROPERTIES(ob_host PROPERTIES PREFIX "") 


INSTALL( TARGETS ob_host LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/../ObserverServer/bin/game/cservice/Resource/ )
				 

