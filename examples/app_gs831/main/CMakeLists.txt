# set(CMAKE_VERBOSE_MAKEFILE on)
# add_definitions("-g")
add_compile_options(-O3)
############### Add include ###################
list(APPEND ADD_INCLUDE "include"
    )
###############################################

############ Add source files #################
# list(APPEND ADD_SRCS  "src/main.c")
# list(APPEND ADD_SRCS  "src/fun_class.c")
# list(APPEND ADD_SRCS  "src/defaults.c")
# list(APPEND ADD_SRCS  "src/card.c")
# list(APPEND ADD_SRCS  "src/face.c")
# list(APPEND ADD_SRCS  "src/self_test.c")


# aux_source_directory(src ADD_SRCS)
append_srcs_dir(ADD_SRCS "src")
append_srcs_dir(ADD_SRCS "lib")
# list(REMOVE_ITEM COMPONENT_SRCS "src/test2.c")
###############################################

###### Add required/dependent components ######
list(APPEND ADD_REQUIREMENTS libmaix maix_gs831)

###############################################

############ Add static libs ##################
# list(APPEND ADD_STATIC_LIB "lib/libtest.a")
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

set_target_properties(app_gs831 PROPERTIES LINK_FLAGS "-T${CMAKE_CURRENT_SOURCE_DIR}/app_gs831.lds -Wl,--whole-archive main/libmain.a -Wl,--no-whole-archive")

register_component()
