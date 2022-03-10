
if(CONFIG_COMPONENT_IMAGE_ENABLE)

    list(APPEND ADD_INCLUDE "include")

    append_srcs_dir(ADD_SRCS "src")

    ############ Add static libs ##################
    # list(APPEND ADD_STATIC_LIB "core/lib/libmaix_nn.a")
    ###############################################

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libmaix/include)

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
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libmaix/lib/arch/v83x/include/opencv4)
        list(APPEND ADD_DYNAMIC_LIB
            "../libmaix/lib/arch/v831/libmaix_nn.so"
            "../libmaix/lib/arch/v831/libmaix_image.so"
            "../libmaix/lib/arch/v83x/lib/libjpeg.so"
            "../libmaix/lib/arch/v83x/lib/libpng12.so"
            "../libmaix/lib/arch/v83x/lib/libwebp.so"
            "../libmaix/lib/arch/v83x/lib/libfreetype.so"
            "../libmaix/lib/arch/v83x/lib/libharfbuzz.so"
            "../libmaix/lib/arch/v83x/lib/libglib-2.0.so"
            "../libmaix/lib/arch/v83x/lib/libpcre.so"
            "../libmaix/lib/arch/v83x/lib/libbz2.so"
            "../libmaix/lib/arch/v83x/opencv4/libopencv_videoio.so"
            "../libmaix/lib/arch/v83x/opencv4/libopencv_highgui.so"
            "../libmaix/lib/arch/v83x/opencv4/libopencv_imgcodecs.so"
            "../libmaix/lib/arch/v83x/opencv4/libopencv_imgproc.so"
            "../libmaix/lib/arch/v83x/opencv4/libopencv_core.so"
            "../libmaix/lib/arch/v83x/opencv4/libopencv_freetype.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_aruco.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_dnn.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_hfs.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_optflow.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_shape.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_bgsegm.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_dpm.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_phase_unwrapping.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_stereo.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_video.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_bioinspired.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_face.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_photo.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_stitching.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_videostab.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_calib3d.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_features2d.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_img_hash.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_plot.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_structured_light.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_ccalib.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_flann.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_quality.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_superres.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_ximgproc.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_line_descriptor.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_reg.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_surface_matching.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_xobjdetect.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_datasets.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_fuzzy.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_ml.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_rgbd.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_text.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_xphoto.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_dnn_objdetect.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_objdetect.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_saliency.so"
            # "../libmaix/lib/arch/v83x/opencv4/libopencv_tracking.so"
        )
    elseif(CONFIG_ARCH_V833)
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libmaix/lib/arch/v83x/include/opencv4)
        list(APPEND ADD_DYNAMIC_LIB
            "../libmaix/lib/arch/v831/libmaix_nn.so"
            "../libmaix/lib/arch/v831/libmaix_image.so"
            "../libmaix/lib/arch/v83x/lib/libjpeg.so"
            "../libmaix/lib/arch/v83x/lib/libpng12.so"
            "../libmaix/lib/arch/v83x/lib/libwebp.so"
            "../libmaix/lib/arch/v83x/lib/libfreetype.so"
            "../libmaix/lib/arch/v83x/lib/libharfbuzz.so"
            "../libmaix/lib/arch/v83x/lib/libglib-2.0.so"
            "../libmaix/lib/arch/v83x/lib/libpcre.so"
            "../libmaix/lib/arch/v83x/lib/libbz2.so"
            "../libmaix/lib/arch/v83x/opencv4/libopencv_videoio.so"
            "../libmaix/lib/arch/v83x/opencv4/libopencv_highgui.so"
            "../libmaix/lib/arch/v83x/opencv4/libopencv_imgcodecs.so"
            "../libmaix/lib/arch/v83x/opencv4/libopencv_imgproc.so"
            "../libmaix/lib/arch/v83x/opencv4/libopencv_core.so"
            "../libmaix/lib/arch/v83x/opencv4/libopencv_freetype.so"
        )
    elseif(CONFIG_ARCH_R329)
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libmaix/lib/arch/r329/include/opencv4)
        list(APPEND ADD_DYNAMIC_LIB
            "../libmaix/lib/arch/r329/libmaix_image.so"
            "../libmaix/lib/arch/r329/opencv4/libopencv_videoio.so"
            "../libmaix/lib/arch/r329/opencv4/libopencv_highgui.so"
            "../libmaix/lib/arch/r329/opencv4/libopencv_imgcodecs.so"
            "../libmaix/lib/arch/r329/opencv4/libopencv_imgproc.so"
            "../libmaix/lib/arch/r329/opencv4/libopencv_core.so"
            "../libmaix/lib/arch/r329/opencv4/libopencv_freetype.so"
        )
    else()

    endif()
    ###############################################

register_component(DYNAMIC)
endif()
