if(CONFIG_LIBMAIX_ENABLED)
    ################# Add include #################
    list(APPEND ADD_INCLUDE "include"
        )
    # list(APPEND ADD_PRIVATE_INCLUDE "include_private")
    ###############################################

    ############## Add source files ###############
    list(APPEND ADD_SRCS  "src/libmaix.c"
        )
    # aux_source_directory(src ADD_SRCS)
    # list(REMOVE_ITEM COMPONENT_SRCS "src/test.c")
    append_srcs_dir(ADD_SRCS "src/decoder")
    ###############################################
    append_srcs_dir(ADD_SRCS "src/mud")
    ###### Add required/dependent components ######
    # list(APPEND ADD_REQUIREMENTS component1)
    ###############################################

    ############ Add static libs ##################
    # list(APPEND ADD_STATIC_LIB "core/lib/libmaix_nn.a")
    ###############################################

    ############ Add dynamic libs ##################
    if(CONFIG_ARCH_Desktop)
        include_directories(/usr/include/opencv4)
        include_directories(/usr/local/include/opencv4)
        append_srcs_dir(ADD_SRCS "lib/arch/desktop/libmaix_utils")
        append_srcs_dir(ADD_SRCS "lib/arch/desktop/libmaix_image")
        append_srcs_dir(ADD_SRCS "lib/arch/desktop/libmaix_cam")
        append_srcs_dir(ADD_SRCS "lib/arch/desktop/libmaix_disp")
        list(APPEND ADD_DYNAMIC_LIB
            "/usr/lib/x86_64-linux-gnu/libopencv_videoio.so"
            "/usr/lib/x86_64-linux-gnu/libopencv_highgui.so"
            "/usr/lib/x86_64-linux-gnu/libopencv_imgcodecs.so"
            "/usr/lib/x86_64-linux-gnu/libopencv_imgproc.so"
            "/usr/lib/x86_64-linux-gnu/libopencv_core.so"
            "/usr/lib/x86_64-linux-gnu/libopencv_freetype.so"
        )
    elseif(CONFIG_ARCH_V831)
        list(APPEND ADD_DYNAMIC_LIB "lib/arch/v831/libmaix_nn.so"
            "lib/arch/v831/libz.so"
            "lib/arch/v831/libISP.so"
            "lib/arch/v831/libcdc_base.so"
            "lib/arch/v831/libion.so"
            "lib/arch/v831/libglog.so"
            "lib/arch/v831/liblog.so"
            "lib/arch/v831/libVE.so"
            "lib/arch/v831/libisp_ini.so"
            "lib/arch/v831/libmedia_utils.so"
            "lib/arch/v831/libMemAdapter.so"
            "lib/arch/v831/libmpp_isp.so"
            "lib/arch/v831/libmpp_vi.so"
            "lib/arch/v831/libmaix_cam.so"
            "lib/arch/v831/libmaix_disp.so"
            "lib/arch/v831/libmaix_image.so"
            "lib/arch/v831/libmaix_utils.so"
            "lib/arch/v831/libcdx_base.so"
            "lib/arch/v831/libvideoengine.so"
            "lib/arch/v831/libcdx_stream.so"
            "lib/arch/v831/libcdx_parser.so"
            "lib/arch/v831/libvdecoder.so"
            "lib/arch/v831/libvenc_codec.so"
            "lib/arch/v831/libvenc_base.so"
            "lib/arch/v831/libmpp_vo.so"
            "lib/arch/v831/libmedia_mpp.so"
            "lib/arch/v831/libcedarxrender.so"
            "lib/arch/v831/libcedarxstream.so"
            "lib/arch/v831/libcdx_common.so"
            "lib/arch/v831/libadecoder.so"
            "lib/arch/v831/libhwdisplay.so"
            "lib/arch/v831/libcutils.so"
            "lib/arch/v831/libcedarx_aencoder.so"
            "lib/arch/v831/libasound.so"
            "lib/arch/v831/libmpp_component.so"
            "lib/arch/v831/libvencoder.so"
        )
        set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group libmaix/liblibmaix.a -lmaix_nn -Wl,--end-group" PARENT_SCOPE)
    elseif(CONFIG_ARCH_V833)
        list(APPEND ADD_DYNAMIC_LIB "lib/arch/v833/libmaix_nn.so"
            "lib/arch/v833/libz.so"
            "lib/arch/v833/libISP.so"
            "lib/arch/v833/libcdc_base.so"
            "lib/arch/v833/libion.so"
            "lib/arch/v833/libglog.so"
            "lib/arch/v833/liblog.so"
            "lib/arch/v833/libVE.so"
            "lib/arch/v833/libisp_ini.so"
            "lib/arch/v833/libmedia_utils.so"
            "lib/arch/v833/libMemAdapter.so"
            "lib/arch/v833/libmpp_isp.so"
            "lib/arch/v833/libmpp_vi.so"
            "lib/arch/v833/libmaix_cam.so"
            "lib/arch/v833/libmaix_disp.so"
            "lib/arch/v833/libmaix_image.so"
            "lib/arch/v833/libmaix_utils.so"
            "lib/arch/v833/libcdx_base.so"
            "lib/arch/v833/libvideoengine.so"
            "lib/arch/v833/libcdx_stream.so"
            "lib/arch/v833/libcdx_parser.so"
            "lib/arch/v833/libvdecoder.so"
            "lib/arch/v833/libvenc_codec.so"
            "lib/arch/v833/libvenc_base.so"
            "lib/arch/v833/libmpp_vo.so"
            "lib/arch/v833/libmedia_mpp.so"
            "lib/arch/v833/libcedarxrender.so"
            "lib/arch/v833/libcedarxstream.so"
            "lib/arch/v833/libcdx_common.so"
            "lib/arch/v833/libadecoder.so"
            "lib/arch/v833/libhwdisplay.so"
            "lib/arch/v833/libcutils.so"
            "lib/arch/v833/libcedarx_aencoder.so"
            "lib/arch/v833/libasound.so"
            "lib/arch/v833/libmpp_component.so"
            "lib/arch/v833/libvencoder.so"
        )
      set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group libmaix/liblibmaix.a -lmaix_nn -Wl,--end-group" PARENT_SCOPE)
    elseif(CONFIG_ARCH_R329)
      list(APPEND ADD_DYNAMIC_LIB
          "lib/arch/r329/libmaix_utils.so"
          "lib/arch/r329/libmaix_image.so"
          "lib/arch/r329/libmaix_disp.so"
          "lib/arch/r329/libmaix_cam.so"
          "lib/arch/r329/libmaix_nn.so"
          "lib/arch/r329/libmaix_nn_decoder.so"
          "lib/arch/r329/libmaix_nn_app.so"
          "lib/arch/r329/libaipudrv.so.4"
          "lib/arch/r329/libaipudrv.so"

      )
      set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -fuse-ld=gold -Wl,--start-group libmaix/liblibmaix.a -lm -Wl,--end-group" PARENT_SCOPE)

      list(APPEND ADD_DEFINITIONS -DCONFIG_ARCH_R329=1)

    else()

    endif()
    ###############################################

    #### Add compile option for this component ####
    #### Just for this component, won't affect other
    #### modules, including component that depend
    #### on this component
    # list(APPEND ADD_DEFINITIONS_PRIVATE -DAAAAA=1)

    #### Add compile option for this component
    #### and components denpend on this component
    # list(APPEND ADD_DEFINITIONS -DAAAAA222=1
    #                             -DAAAAA333=1)
    ###############################################

    register_component()
endif()
