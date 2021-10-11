if(CONFIG_COMPONENT_IMAGE_ENABLE)
	
    list(APPEND ADD_INCLUDE "include")
    append_srcs_dir(ADD_SRCS "src")

    register_component(DYNAMIC)
endif()
