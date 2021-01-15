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
    ###############################################

    ###### Add required/dependent components ######
    # list(APPEND ADD_REQUIREMENTS component1)
    ###############################################

    ############ Add static libs ##################
    # list(APPEND ADD_STATIC_LIB "core/lib/libmaix_nn.a")
    ###############################################

    ############ Add dynamic libs ##################
    list(APPEND ADD_DYNAMIC_LIB "lib/arch/v831/libmaix_nn.so"
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
                                "lib/arch/v831/libmpp_mini.so"
                                "lib/arch/v831/libmaix_cam.so"
                                "lib/arch/v831/libmaix_disp.so"
                                "lib/arch/v831/libmaix_image.so"
                                "lib/arch/v831/libmaix_utils.so"
    )
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
