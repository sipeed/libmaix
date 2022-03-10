cmake_minimum_required(VERSION 3.5)

set(CMAKE_VERBOSE_MAKEFILE on)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../libmaix/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../r329/include/opencv4)
link_directories("../../../r329/opencv4")

add_library(maix_utils SHARED # STATIC
        src/libmaix_err.c
        src/libmaix_log.c
)

set(export_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../libmaix)

target_link_libraries(
        maix_utils
)

set_target_properties(
        maix_utils
        PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${export_dir}/lib/arch/r329")

set_target_properties(maix_utils PROPERTIES LINK_FLAGS "-Wl,-rpath,/usr/local/lib/python3.9/dist-packages/maix/")

add_custom_command(
        # POST_BUILD 处 有三个值可选
        # 分别是:
        # PRE_BUILD：在 hello 运行其他规则前执行
        # PRE_LINK：在编译源文件之后但在 链接其他二进制文件 或 运行静态库的库管理器 或 归档工具 之前执行
        # POST_BUILD：最后执行
        TARGET maix_utils POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_SOURCE_DIR}/src/libmaix_err.h" "${export_dir}/include/libmaix_err.h"
)
