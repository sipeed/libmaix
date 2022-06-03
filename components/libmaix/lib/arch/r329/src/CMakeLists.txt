cmake_minimum_required(VERSION 3.4.1)

project("export r329 dll")

set(CMAKE_C_COMPILER "/opt/r329_linux_x86_python3.9_toolchain/bin/aarch64-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "/opt/r329_linux_x86_python3.9_toolchain/bin/aarch64-linux-gnu-g++")
add_definitions( -DCONFIG_ARCH_R329=1)

set(CMAKE_VERBOSE_MAKEFILE on)

# 设置资源主目录 CMAKE_CURRENT_SOURCE_DIR 代表当前CmakeLists.txt 所在的目录
set(lib_src_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 设置Cmake编译后文件的存放的临时目录
set(lib_build_DIR ${lib_src_DIR}/build)

# 将生成的临时文件放在 lib_build_DIR 中
file(MAKE_DIRECTORY ${lib_build_DIR})

# 添加子项目
add_subdirectory(${lib_src_DIR}/libmaix_utils ${lib_build_DIR}/libmaix_utils)
add_subdirectory(${lib_src_DIR}/libmaix_image ${lib_build_DIR}/libmaix_image)
add_subdirectory(${lib_src_DIR}/libmaix_disp ${lib_build_DIR}/libmaix_disp)
add_subdirectory(${lib_src_DIR}/libmaix_cam ${lib_build_DIR}/libmaix_cam)
add_subdirectory(${lib_src_DIR}/libmaix_nn ${lib_build_DIR}/libmaix_nn)
add_subdirectory(${lib_src_DIR}/libmaix_nn_decoder ${lib_build_DIR}/libmaix_nn_decoder)
add_subdirectory(${lib_src_DIR}/libmaix_nn_app ${lib_build_DIR}/libmaix_nn_app)
