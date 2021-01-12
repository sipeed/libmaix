
# set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <OBJECTS> -o <TARGET>.elf <LINK_LIBRARIES>")
# set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <OBJECTS> -o <TARGET>.elf <LINK_LIBRARIES>")

set(start_app_sh ${SDK_PATH}/tools/cmake/start_app_v831.sh)
if(CONFIG_ARCH STREQUAL "ARCH_V831")
    set(start_app_sh ${SDK_PATH}/tools/cmake/start_app_v831.sh)
endif()    


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND mkdir -p ${PROJECT_DIST_DIR}
    COMMAND mkdir -p ${PROJECT_DIST_DIR}/lib
    COMMAND cp ${g_dynamic_libs} ${PROJECT_DIST_DIR}/lib/
    COMMAND cp ${PROJECT_BINARY_DIR}/${PROJECT_NAME} ${PROJECT_DIST_DIR}
    COMMAND cp ${start_app_sh} ${PROJECT_DIST_DIR}/start_app.sh
    COMMAND chmod +x ${PROJECT_DIST_DIR}/start_app.sh && echo "$curr_dir/${PROJECT_NAME}" >> ${PROJECT_DIST_DIR}/start_app.sh VERBATIM
    DEPENDS ${PROJECT_NAME}
    COMMENT "-- copy binary files to dist dir ..."
    )

