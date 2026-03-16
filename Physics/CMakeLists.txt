# cmake 요구 버전
CMAKE_MINIMUM_REQUIRED(VERSION 3.15)

# 프로젝트 이름 설정
PROJECT(Physics)

MESSAGE(" ")
MESSAGE("======================================== ProjectName : ${CMAKE_PROJECT_NAME}")

# 빌드 타입
#set(CMAKE_BUILD_TYPE Release)
SET(CMAKE_BUILD_TYPE Debug)

MESSAGE("======================================== BuildType : ${CMAKE_BUILD_TYPE}")

SET (CMAKE_VERBOSE_MAKEFILE true )

MESSAGE("======================================== VerboseMakefile : ${CMAKE_VERBOSE_MAKEFILE}")


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

STRING(TIMESTAMP BUILDTIME  "%Y%m%d%H%M")

# 빌드 형상 옵션
SET(CMAKE_C_FLAGS 			"-std=gnu99 -g3 -O2 -Wall")
SET(CMAKE_C_FLAGS_DEBUG 	"-std=gnu99 -g3 -O0 -Wall")
SET(CMAKE_C_FLAGS_RELEASE 	"-std=gnu99 -g0 -O3 -Wall")
SET(CMAKE_CXX_FLAGS 		"-std=c++11 -g3 -O2 -Wall -DBUILDTIME=${BUILDTIME}")
SET(CMAKE_CXX_FLAGS_DEBUG 	"-std=c++11 -g3 -O0 -Wall -DBUILDTIME=${BUILDTIME}")
SET(CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -g0 -O3 -Wall -DBUILDTIME=${BUILDTIME}")

ADD_COMPILE_OPTIONS(-ldl)


MESSAGE("======================================== C Compiler : ${CMAKE_C_COMPILER}")
MESSAGE("======================================== C++ Compiler : ${CMAKE_CXX_COMPILER}")
MESSAGE("======================================== CMAKE_SOURCE_DIR : ${CMAKE_SOURCE_DIR}")
MESSAGE("======================================== CMAKE_BINARY_DIR : ${CMAKE_BINARY_DIR}")
MESSAGE("======================================== CMAKE_CURRENT_SOURCE_DIR : ${CMAKE_CURRENT_SOURCE_DIR}")
MESSAGE("======================================== CMAKE_INSTALL_PREFIX : ${CMAKE_INSTALL_PREFIX}")

FILE(GLOB_RECURSE PHYSICS_SRCS ${CMAKE_SOURCE_DIR}
"*.h"
"*.cpp"
)

ADD_LIBRARY(physics SHARED ${PHYSICS_SRCS})

SET_TARGET_PROPERTIES(physics PROPERTIES PREFIX "")

INSTALL( TARGETS physics DESTINATION ${CMAKE_SOURCE_DIR}/../InGameServer/bin/game/cservice/Resource/ )

MESSAGE(" ")