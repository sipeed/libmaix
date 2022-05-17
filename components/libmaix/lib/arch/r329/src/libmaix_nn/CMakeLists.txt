
cmake_minimum_required(VERSION 3.5)
# message("_____________________________________________________________")
# message(STATUS "cmake_source_dir ->" ${CMAKE_SOURCE_DIR})
# message(STATUS "project_source_dir ->"  ${PROJECT_SOURCE_DIR})
# message(STATUS "executable_output_dir - > "${EXECUTABLE_OUTPUT_PATH})
# message(STATUS "current_source_path ->."${CMAKE_CURRENT_SOURCE_DIR})
# message("_____________________________________________________________")


set(CMAKE_VERBOSE_MAKEFILE on)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../libmaix/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../r329/src/libmaix_nn/lib/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../r329/src)

link_directories("./lib/")
link_directories("../../../r329/")

add_library(maix_nn SHARED # STATIC
        src/libmaix_nn.cpp
)

set(export_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../libmaix)

target_link_libraries(
    maix_nn
    aipudrv
)


set_target_properties(maix_nn PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${export_dir}/lib/arch/r329")

set_target_properties(maix_nn PROPERTIES LINK_FLAGS "-Wl,-rpath,/usr/local/lib/python3.9/dist-packages/maix/")


# add_custom_command(
#         # POST_BUILD 处 有三个值可选
#         # 分别是:
#         # PRE_BUILD：在 hello 运行其他规则前执行
#         # PRE_LINK：在编译源文件之后但在 链接其他二进制文件 或 运行静态库的库管理器 或 归档工具 之前执行
#         # POST_BUILD：最后执行
#         TARGET maix_nn POST_BUILD
#         COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_SOURCE_DIR}/src/libmaix_nn.h" "${export_dir}/include/libmaix_nn.h"
# )
