cmake_minimum_required(VERSION 3.10)

project(lccl VERSION 1.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 构建类型默认为Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# 源文件
file(GLOB_RECURSE srcs "src/*.cpp")

# 创建动态库文件
add_library(${PROJECT_NAME} SHARED ${srcs})

# 设定库版本
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION 1.0 SOVERSION 1)

# 指定目标的包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# 链接库目录
target_link_directories(${PROJECT_NAME} PRIVATE dep/lib)

# 设置输出目录
if(MSVC)
    set(bin_dir ${CMAKE_CURRENT_SOURCE_DIR}/bin)
else()
    set(bin_dir ${CMAKE_CURRENT_BINARY_DIR}/bin)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${bin_dir})
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${bin_dir})
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${bin_dir})
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${bin_dir})
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${bin_dir})
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG ${bin_dir})
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE ${bin_dir})
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${bin_dir})
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${bin_dir})

set(os_name "debian")
set(os_version "12")

# 获取系统和架构类型
if(WIN32)
    set(os_name "windows")
    set(os_version "msvc")
else()
    if(EXISTS "/etc/os-release")
        file(READ "/etc/os-release" os_release_content)
        string(REGEX MATCH "ID=\"([^\"]*)\"" _ ${os_release_content})
        set(os_name ${CMAKE_MATCH_1})
        string(REGEX MATCH "VERSION_ID=\"([^\"]*)\"" _ ${os_release_content})
        set(os_version ${CMAKE_MATCH_1})
    endif()
endif()
message(STATUS "os name=${os_name}, os version=${os_version}, os arch=${CMAKE_SYSTEM_PROCESSOR}, build type=${CMAKE_BUILD_TYPE}")

set(prebuild_lib_base_path "../../version/lib")
set(prebuild_libs "")

# 系统自定义参数
if(os_name MATCHES "Windows")
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
    target_compile_definitions(${PROJECT_NAME} PRIVATE LCCL_API_EXPORT=1)
    set(prebuild_libs
        ${prebuild_libs}
        "zlib 1.3.1"
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE zlib)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE z)
endif()

# 编译前操作
add_custom_target(
    ${PROJECT_NAME}_prebuild
    COMMAND ${CMAKE_COMMAND} -E env python3 ${CMAKE_CURRENT_SOURCE_DIR}/prebuild.py ${os_name} ${os_version} ${CMAKE_SYSTEM_PROCESSOR} ${CMAKE_BUILD_TYPE} ${prebuild_lib_base_path} ${prebuild_libs}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_prebuild)

set(postbuild_base_path "../../version/lib")

# 编译后操作
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env python3 ${CMAKE_CURRENT_SOURCE_DIR}/postbuild.py ${os_name} ${os_version} ${CMAKE_SYSTEM_PROCESSOR} ${CMAKE_BUILD_TYPE} ${postbuild_base_path} ${PROJECT_NAME} ${PROJECT_VERSION}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
