
if(CONFIG_COMPONENT_MAIX_ENABLE)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g -Wno-strict-aliasing")

    list(APPEND ADD_INCLUDE "include")

    append_srcs_dir(ADD_SRCS "lib")
    append_srcs_dir(ADD_SRCS "src")

    if(CMAKE_COMPILER_IS_GNUCXX)
        add_compile_options(-std=c++11)
    endif(CMAKE_COMPILER_IS_GNUCXX)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g -Wno-strict-aliasing")

    list(APPEND ADD_INCLUDE "include")

    append_srcs_dir(ADD_SRCS "lib")
    append_srcs_dir(ADD_SRCS "src")

    ############ Add static libs ##################
    # list(APPEND ADD_STATIC_LIB "core/lib/libmaix_nn.a")
    ###############################################

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libmaix/include)

    if(CMAKE_COMPILER_IS_GNUCXX)
        # add_compile_options(-std=c++17)
        message(STATUS "optional:-std=c++17")
    endif(CMAKE_COMPILER_IS_GNUCXX)

register_component(DYNAMIC)
endif()
