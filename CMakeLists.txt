cmake_minimum_required(VERSION 3.7)
project(tlhttp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenSSL REQUIRED)

set(SOURCE_FILES src/Connection.cpp src/Connection.h src/Request.cpp src/Request.h src/Server.cpp src/Server.h)
add_library(tlhttp SHARED ${SOURCE_FILES})

target_link_libraries(tlhttp ${OPENSSL_LIBRARIES})
include_directories(${OPENSSL_INCLUDE_DIR})

find_package(GTest REQUIRED)
if(NOT NO_TESTS)
	add_executable(tlhttp_test
			test/test.cpp)

	target_link_libraries(tlhttp_test ${GTEST_BOTH_LIBRARIES} tlhttp)
	target_include_directories(tlhttp_test PUBLIC ${GTEST_INCLUDE_DIR})
endif()
