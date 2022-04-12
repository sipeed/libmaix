if(CONFIG_COMPONENT_3RD_PARTY_ENABLE)

    list(APPEND ADD_INCLUDE "./include/")

    list(APPEND ADD_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    add_definitions(-DIMLIB_CONFIG_H_FILE="costom_imlib_config.h")

    aux_source_directory("apriltag/common"  APRILTAG_SRCS)
    aux_source_directory("apriltag"  APRILTAG_SRCS)

    aux_source_directory("libjpeg/source"  LIBJPEG_SRCS)
    aux_source_directory("libjpeg"  LIBJPEG_SRCS)

    aux_source_directory("zbar"   ZBAR_SRCS)

    aux_source_directory("imlib/src"   IMLIB_SRCS)

    if(CONFIG_LVGL_ENABLE)
        list(APPEND ADD_INCLUDE
            "lvgl"
            "lvgl/include/"
        )
        aux_source_directory("lvgl/lvgl/src/lv_core"    LVGL_SRCS)
        aux_source_directory("lvgl/lvgl/src/lv_draw"    LVGL_SRCS)
        aux_source_directory("lvgl/lvgl/src/lv_font"    LVGL_SRCS)
        aux_source_directory("lvgl/lvgl/src/lv_hal"    LVGL_SRCS)
        aux_source_directory("lvgl/lvgl/src/lv_misc"    LVGL_SRCS)
        aux_source_directory("lvgl/lvgl/src/lv_objx"    LVGL_SRCS)
        aux_source_directory("lvgl/lvgl/src/lv_widgets"    LVGL_SRCS)
        aux_source_directory("lvgl/lvgl/src/lv_themes"    LVGL_SRCS)
        aux_source_directory("lvgl/lv_examples"    LVGL_SRCS)
        aux_source_directory("lvgl/lv_examples/src/assets/"    LVGL_SRCS)
        aux_source_directory("lvgl/lv_examples/src/lv_demo_widgets/"    LVGL_SRCS)
        aux_source_directory("lvgl/lv_examples/src/lv_demo_benchmark/"    LVGL_SRCS)
        aux_source_directory("lvgl/lv_examples/src/lv_ex_get_started/"    LVGL_SRCS)
        aux_source_directory("lvgl/lv_examples/src/lv_ex_style/"    LVGL_SRCS)
        aux_source_directory("lvgl/lv_examples/src/lv_ex_widgets/lv_ex_img/"    LVGL_SRCS)
        aux_source_directory("lvgl/lv_examples/src/lv_ex_widgets/lv_ex_canvas/"    LVGL_SRCS)
        aux_source_directory("lvgl/lv_lib_gif"    LVGL_SRCS)
        aux_source_directory("lvgl/lv_lib_png"    LVGL_SRCS)
        aux_source_directory("lvgl/lv_lib_qrcode"   LVGL_SRCS)
        aux_source_directory("lvgl/src"   LVGL_SRCS)
        list(APPEND ADD_SRCS ${LVGL_SRCS})
    endif()

    if(CONFIG_LIBJPEG_ENABLE)
        list(APPEND ADD_INCLUDE
            "libjpeg/include"
            "libjpeg"
        )
        list(APPEND ADD_SRCS
            ${LIBJPEG_SRCS}
        )
    endif()

    if(CONFIG_CJSON_ENABLE)
        list(APPEND ADD_INCLUDE "cJSON/include")
        list(APPEND ADD_SRCS ${CJSON_SRCS})
    endif()

    if(CONFIG_ZBAR_ENABLE)
        list(APPEND ADD_INCLUDE "zbar/include")
        list(APPEND ADD_SRCS ${ZBAR_SRCS})
    endif()

    if(CONFIG_SQLITE3_ENABLE)
        list(APPEND ADD_INCLUDE "sqlite3")
        list(APPEND ADD_SRCS "sqlite3/sqlite3.c")
    endif()

    if(CONFIG_IMLIB_ENABLE)
        list(APPEND ADD_INCLUDE
                "imlib/include"
            )
        list(APPEND ADD_SRCS
                ${IMLIB_SRCS}
            )
        list(REMOVE_ITEM ADD_SRCS "apriltag/apriltag.c")
    endif()

    if(CONFIG_APRILTAG_ENABLE)
        list(APPEND ADD_INCLUDE
                "apriltag/common"
                "apriltag"
            )
        list(APPEND ADD_SRCS
                ${APRILTAG_SRCS}
            )
        # list(REMOVE_ITEM ADD_SRCS "imlib/src/apriltag.c")
        list(REMOVE_ITEM ADD_SRCS "apriltag/apriltag_pywrap.c")
    endif()

    list(APPEND ADD_DEFINITIONS -fPIC) # for dynamic libs

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g -Wno-strict-aliasing")

    register_component()
endif()