# cmake 요구 버전
CMAKE_MINIMUM_REQUIRED(VERSION 3.15)

# 프로젝트 이름 설정
PROJECT(F4Host)
MESSAGE("======================================== ProjectName : ${CMAKE_PROJECT_NAME}")

# 빌드 타입
#set(CMAKE_BUILD_TYPE Release)
SET(CMAKE_BUILD_TYPE Debug)
MESSAGE("======================================== BuildType : ${CMAKE_BUILD_TYPE}")

SET (CMAKE_VERBOSE_MAKEFILE true )
MESSAGE("======================================== VerboseMakefile : ${CMAKE_VERBOSE_MAKEFILE}")

# proto generate
#FIND_PACKAGE(Protobuf 3.00.0.0 REQUIRED HINTS ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libprotobuf)
#PROTOBUF_GENERATE(LANGUAGE cpp OUT_VAR PROTO_SRCS PROTOS ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/Message.proto)

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#SET(PROTOBUF_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/3rd/protobuf-3.10.1/src)
#INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_PATH})
SET(GAMECORE_PATH ${CMAKE_SOURCE_DIR}/../GameCore)

#openssl
SET(OPENSSL_PATH ${CMAKE_SOURCE_DIR}/3rd/openssl-3.2.0)

# 써드파티 헤더파일 포함
INCLUDE_DIRECTORIES(${GAMECORE_PATH})
INCLUDE_DIRECTORIES(${GAMECORE_PATH}/includes)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../Physics)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/host)

#openssl
INCLUDE_DIRECTORIES(${OPENSSL_PATH})
INCLUDE_DIRECTORIES(${OPENSSL_PATH}/include/openssl)

# 써드파티 링크
#LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/3rd/enet-1.3.13/lib)
#LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/3rd/protobuf-3.10.1/lib)
#INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../Physics/build)
LINK_DIRECTORIES(${GAMECORE_PATH}/build) # library file path

#openssl
LINK_DIRECTORIES(${OPENSSL_PATH}) # library file path

#if(Protobuf_FOUND)
#MESSAGE("======================================== protobuf FOUND")
#else()
#MESSAGE("======================================== protobuf NOT_FOUND")
#endif()


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

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
	ADD_DEFINITIONS(-D_DEBUG)
ENDIF()

STRING(TIMESTAMP BUILDTIME  "%Y%m%d%H%M")

# 빌드 형상 옵션
SET(CMAKE_C_FLAGS 			"-std=gnu99 -g3 -O2 -Wall")
SET(CMAKE_C_FLAGS_DEBUG 	"-std=gnu99 -g3 -O0 -Wall")
SET(CMAKE_C_FLAGS_RELEASE 	"-std=gnu99 -g0 -O0 -Wall")
SET(CMAKE_CXX_FLAGS 		"-std=c++11 -g3 -O2 -Wall -DBUILDTIME=${BUILDTIME}")
SET(CMAKE_CXX_FLAGS_DEBUG 	"-std=c++11 -g3 -O0 -Wall -DBUILDTIME=${BUILDTIME}")
SET(CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -g0 -O0 -Wall -DBUILDTIME=${BUILDTIME}")

set(CMAKE_INSTALL_RPATH "$ORIGIN")#library path

#ADD_COMPILE_OPTIONS(-Wall -g -c -fPIC -fvisibility=hidden)

MESSAGE("======================================== C Compiler : ${CMAKE_C_COMPILER}")
MESSAGE("======================================== C++ Compiler : ${CMAKE_CXX_COMPILER}")
MESSAGE("======================================== CMAKE_SOURCE_DIR : ${CMAKE_SOURCE_DIR}")
MESSAGE("======================================== CMAKE_BINARY_DIR : ${CMAKE_BINARY_DIR}")
MESSAGE("======================================== CMAKE_CURRENT_SOURCE_DIR : ${CMAKE_CURRENT_SOURCE_DIR}")
MESSAGE("======================================== CMAKE_BINARY_DIR : ${CMAKE_BINARY_DIR}")
MESSAGE("======================================== GAMECORE_BINARY_DIR : ${GAMECORE_PATH}/build")

FILE(GLOB_RECURSE HOST_SRCS ${CMAKE_SOURCE_DIR}
"../Data/*.h"
"host/*.h"
"host/*.cpp"
)

# FILE(GLOB_RECURSE PHYSICS_SRCS ${CMAKE_SOURCE_DIR}
# "../Physics/*.h"
# "../Physics/*.cpp"
# )
# #
# FILE(GLOB_RECURSE GAMECORE_SRCS ${CMAKE_SOURCE_DIR}
# "../GameCore/includes/*.h"
# "../GameCore/includes/*.cpp"
# "../GameCore/*.h"
# "../GameCore/*.cpp"
# )

#TARGET_LINK_LIBRARIES(GameCore Physics)..

# ADD_LIBRARY(GameCore SHARED ${GAMECORE_SRCS})
# ADD_LIBRARY(Physics SHARED ${PHYSICS_SRCS})
ADD_LIBRARY(host SHARED ${HOST_SRCS} ${PROTO_SRCS})
#TARGET_COMPILE_DEFINITIONS(GameCore PUBLIC GameCore)
#TARGET_COMPILE_DEFINITIONS(Physics PUBLIC Physics)


#TARGET_LINK_LIBRARIES(GameCore Physics "")
#TARGET_LINK_LIBRARIES(host protobuf::libprotobuf)
TARGET_LINK_LIBRARIES(host gamecore physics crypto ssl "")
#라이브러리가 있는 위치 ""
#ADD_LIBRARY(GameCore SHARED ${GAMECORE_SRCS})
#SET_TARGET_PROPERTIES(Physics PROPERTIES IMPORTED_LOCATION .)
#SET_TARGET_PROPERTIES(GameCore PROPERTIES IMPORTED_LOCATION .)

#TARGET_COMPILE_OPTIONS(Physics PUBLIC -Wall -g -c -fPIC  -Wl -rpath .)
#TARGET_COMPILE_OPTIONS(GameCore PUBLIC -Wall -g -c -fPIC  -Wl -rpath .)
TARGET_COMPILE_OPTIONS(host PUBLIC -Wall -g -c -fPIC -fvisibility=hidden -Wl -rpath .)

#LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/../InGameServer/bin/game/cservice/Resource)
#LINK_LIBRARIES(host GameCore Physics)
#LINK_LIBRARIES(GameCore Physics)
LINK_LIBRARIES(gamecore)
LINK_LIBRARIES(physics)
LINK_LIBRARIES(crypto)
LINK_LIBRARIES(ssl)
#SET_TARGET_PROPERTIES(host PROPERTIES CXX_VISIBILITY_PRESET hidden)
#SET_TARGET_PROPERTIES(Physics PROPERTIES PREFIX "")
#SET_TARGET_PROPERTIES(host PROPERTIES PREFIX "") 
#SET_TARGET_PROPERTIES(GameCore PROPERTIES PREFIX "")

SET_TARGET_PROPERTIES(host PROPERTIES LINK_FLAGS "-Wl,-rpath,./game/cservice/Resource") #Rpath Set

#INSTALL( TARGETS Physics LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/../InGameServer/bin/game/cservice/Resource/ )
INSTALL( TARGETS host LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/../InGameServer/bin/game/cservice/Resource/ )
#INSTALL( TARGETS GameCore LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/../InGameServer/bin/game/cservice/Resource/ )
#configure_file(${CMAKE_SOURCE_DIR}/../GameCore/Res/AnimData.bin ${CMAKE_SOURCE_DIR}/../InGameServer/bin/game/cservice/Resource/ COPYONLY)

