if(CONFIG_COMPONENT_3RD_PARTY_ENABLE)

    list(APPEND ADD_INCLUDE "./"
                            # "lvgl"
                            "cJSON/include"
                            "sqlite3"
                            "libjpeg/include"
                            "libjpeg"
                            "apriltag/common"
                            "apriltag"
                            "zbar/include")
                            # "zbar/zbar"
                            # "zbar/zbar/decoder")

    # aux_source_directory("lvgl/lvgl/src/lv_core"    LVGL_SRCS)
    # aux_source_directory("lvgl/lvgl/src/lv_draw"    LVGL_SRCS)
    # aux_source_directory("lvgl/lvgl/src/lv_font"    LVGL_SRCS)
    # aux_source_directory("lvgl/lvgl/src/lv_hal"    LVGL_SRCS)
    # aux_source_directory("lvgl/lvgl/src/lv_misc"    LVGL_SRCS)
    # aux_source_directory("lvgl/lvgl/src/lv_objx"    LVGL_SRCS)
    # aux_source_directory("lvgl/lvgl/src/lv_widgets"    LVGL_SRCS)
    # aux_source_directory("lvgl/lvgl/src/lv_themes"    LVGL_SRCS)
    # aux_source_directory("lvgl/lv_examples"    LVGL_SRCS)
    # aux_source_directory("lvgl/lv_examples/src/assets/"    LVGL_SRCS)
    # aux_source_directory("lvgl/lv_examples/src/lv_demo_widgets/"    LVGL_SRCS)
    # aux_source_directory("lvgl/lv_examples/src/lv_demo_benchmark/"    LVGL_SRCS)
    # aux_source_directory("lvgl/lv_examples/src/lv_ex_get_started/"    LVGL_SRCS)
    # aux_source_directory("lvgl/lv_examples/src/lv_ex_style/"    LVGL_SRCS)
    # aux_source_directory("lvgl/lv_examples/src/lv_ex_widgets/lv_ex_img/"    LVGL_SRCS)
    # aux_source_directory("lvgl/lv_examples/src/lv_ex_widgets/lv_ex_canvas/"    LVGL_SRCS)
    # aux_source_directory("lvgl/lv_lib_gif"    LVGL_SRCS)
    # aux_source_directory("lvgl/lv_lib_png"    LVGL_SRCS)
    # aux_source_directory("lvgl/lv_lib_qrcode"   LVGL_SRCS)

    aux_source_directory("apriltag/common"  APRILTAG_SRCS)
    aux_source_directory("apriltag"  APRILTAG_SRCS)

    aux_source_directory("libjpeg/source"  LIBJPEG_SRCS)
    aux_source_directory("libjpeg"  LIBJPEG_SRCS)

    aux_source_directory("zbar"   ZBAR_SRCS)
    # aux_source_directory("zbar/zbar"   ZBAR_SRCS)
    # aux_source_directory("zbar/zbar/decoder"   ZBAR_SRCS)
    # aux_source_directory("zbar/zbar/qrcode"   ZBAR_SRCS)

    list(APPEND ADD_SRCS # ${LVGL_SRCS}
                         ${CJSON_SRCS}
                         "sqlite3/sqlite3.c"
                         ${APRILTAG_SRCS}
                         ${LIBJPEG_SRCS}
                         ${ZBAR_SRCS}
                        )



    list(REMOVE_ITEM ADD_SRCS "apriltag/apriltag_pywrap.c")

    # list(REMOVE_ITEM ADD_SRCS "zbar/zbar/svg.c")
    # list(REMOVE_ITEM ADD_SRCS "zbar/zbar/jpeg.c")
    # list(REMOVE_ITEM ADD_SRCS "zbar/zbar/decoder/databar.c")
    # list(REMOVE_ITEM ADD_SRCS "zbar/zbar/decoder/code93.c")
    # list(REMOVE_ITEM ADD_SRCS "zbar/zbar/decoder/codabar.c")
    # list(REMOVE_ITEM ADD_SRCS "zbar/zbar/decoder/pdf417.c")

    list(APPEND ADD_DEFINITIONS -fPIC) # for dynamic libs

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g")

    register_component()
endif()