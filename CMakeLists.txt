cmake_minimum_required(VERSION 3.10)

project(lccl VERSION 1.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# Windows设置UTF8的flag
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif()

# 宏定义
if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE LCCL_API_EXPORT=1)
endif()

# 链接库
target_link_directories(${PROJECT_NAME} PRIVATE dep/lib)

if(MSVC)
    target_link_libraries(${PROJECT_NAME} PRIVATE zlib)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE z)
endif()


# 编译前操作
add_custom_target(
    ${PROJECT_NAME}_prebuild
    COMMAND ${CMAKE_COMMAND} -E env python3 ${CMAKE_CURRENT_SOURCE_DIR}/prebuild.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_prebuild)
