
set (OpenCV_DIR /usr/lib/arm-linux-gnueabihf/cmake/opencv4)
find_package(OpenCV REQUIRED)
############### Add include ###################
list(APPEND ADD_INCLUDE "include"
    )
list(APPEND ADD_PRIVATE_INCLUDE ${OpenCV_INCLUDE_DIRS})
###############################################

############ Add source files #################
# list(APPEND ADD_SRCS  "src/main.c"
#     )
# aux_source_directory(src ADD_SRCS)
append_srcs_dir(ADD_SRCS "src")
# list(REMOVE_ITEM COMPONENT_SRCS "src/test2.c")
###############################################

###### Add required/dependent components ######
list(APPEND ADD_REQUIREMENTS maix_cv_image libmaix )
###############################################

############ Add static libs ##################
# list(APPEND ADD_STATIC_LIB ${OpenCV_LIBS})
list(APPEND ADD_REQUIREMENTS ${OpenCV_LIBS})
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

