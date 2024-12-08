# the minimum version of CMake.
cmake_minimum_required(VERSION 3.5.0)
project(jsvm_demo)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# 日志打印配置
add_definitions( "-DLOG_DOMAIN=0xd0d0" )
add_definitions( "-DLOG_TAG=\"jscore\"" )

if(DEFINED PACKAGE_FIND_FILE)
    include(${PACKAGE_FIND_FILE})
endif()

include_directories(${NATIVERENDER_ROOT_PATH}
                    ${NATIVERENDER_ROOT_PATH}/include)

add_library(jscore SHARED napi_init.cpp jsvm_utils.cpp)
target_link_libraries(jscore PUBLIC libace_napi.z.so libjsvm.so libhilog_ndk.z.so)