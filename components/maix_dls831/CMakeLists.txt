
if(CONFIG_COMPONENT_MAIX_DLS831_ENABLE)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1 -g -Wno-strict-aliasing")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1 -g -Wno-strict-aliasing -fpermissive")

    list(APPEND ADD_INCLUDE "include")
    add_definitions(-DIMLIB_CONFIG_H_FILE="costom_imlib_config.h")

    append_srcs_dir(ADD_SRCS "lib")
    append_srcs_dir(ADD_SRCS "src")

    # include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libcedarc)
    # include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libcedarc/include)
    # include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libcedarc/include/base)
    # link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libcedarc/library)
    ############ Add static libs ##################
    # list(APPEND ADD_STATIC_LIB "core/lib/libmaix_nn.a")
    ###############################################

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libmaix/include)

    if(CMAKE_COMPILER_IS_GNUCXX)
        # add_compile_options(-std=c++11)
        message(STATUS "optional:-std=c++11")
    endif(CMAKE_COMPILER_IS_GNUCXX)
    ############ Add dynamic libs ##################
    if(CONFIG_ARCH_V831)

        # list(APPEND ADD_INCLUDE "../third_party/quirc")
        # append_srcs_dir(ADD_SRCS "../third_party/quirc")

        # list(APPEND ADD_INCLUDE "../third_party/zbar/include/")
        # append_srcs_dir(ADD_SRCS "../third_party/zbar")

        # list(APPEND ADD_INCLUDE "../third_party/apriltag/common")
        # append_srcs_dir(ADD_SRCS "../third_party/apriltag/common")
        # list(APPEND ADD_INCLUDE "../third_party/apriltag")
        # append_srcs_dir(ADD_SRCS "../third_party/apriltag")
        # list(REMOVE_ITEM ADD_SRCS "../third_party/apriltag/apriltag_pywrap.c")

        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imlib/include)
        list(APPEND ADD_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include")
        add_definitions(-DIMLIB_CONFIG_H_FILE="costom_imlib_config.h")
        append_srcs_dir(ADD_SRCS "../third_party/imlib/src")

        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libmaix/lib/arch/v83x/include/opencv4)

        # list(APPEND ADD_REQUIREMENTS maix_speech)

        list(APPEND ADD_DYNAMIC_LIB
            # "lib/libzbar.so"
            # "lib/libzbar.so.0"
            # "lib/libapriltag.so"
            # "lib/libapriltag.so.3"
            "../libmaix/lib/arch/v831/libmaix_nn.so"
            "../libmaix/lib/arch/v831/libmaix_image.so"
            "../libmaix/lib/arch/v83x/lib/libfreetype.so"
            "../libmaix/lib/arch/v83x/lib/libbz2.so"
            "../libmaix/lib/arch/v83x/lib/libjpeg.so"
            "../libmaix/lib/arch/v83x/lib/libpng12.so"
            "../libmaix/lib/arch/v83x/lib/libwebp.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_videoio.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_highgui.so"
            "../libmaix/lib/arch/v83x/opencv4/libopencv_imgcodecs.so"
            "../libmaix/lib/arch/v83x/opencv4/libopencv_imgproc.so"
            "../libmaix/lib/arch/v83x/opencv4/libopencv_core.so"
        )
    else()

    endif()
    ###############################################

register_component(DYNAMIC)
endif()
