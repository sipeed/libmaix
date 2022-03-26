cmake_minimum_required(VERSION 3.5)

set(CMAKE_VERBOSE_MAKEFILE on)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../libmaix/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../r329/include/opencv4)
link_directories("../../../r329/opencv4")
link_directories("../../../r329/")

add_library(maix_image SHARED # STATIC
        src/libmaix_image.cpp
)

set(export_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../libmaix)

target_link_libraries(
        maix_image
        libmaix_utils.so
        libopencv_videoio.so
        libopencv_highgui.so
        libopencv_imgcodecs.so
        libopencv_imgproc.so
        libopencv_core.so
        libopencv_freetype.so
        # libopencv_aruco.so
        # libopencv_dnn.so
        # libopencv_hfs.so
        # libopencv_optflow.so
        # libopencv_shape.so
        # libopencv_bgsegm.so
        # libopencv_dpm.so
        # libopencv_phase_unwrapping.so
        # libopencv_stereo.so
        # libopencv_video.so
        # libopencv_bioinspired.so
        # libopencv_face.so
        # libopencv_photo.so
        # libopencv_stitching.so
        # libopencv_videostab.so
        # libopencv_calib3d.so
        # libopencv_features2d.so
        # libopencv_img_hash.so
        # libopencv_plot.so
        # libopencv_structured_light.so
        # libopencv_ccalib.so
        # libopencv_flann.so
        # libopencv_quality.so
        # libopencv_superres.so
        # libopencv_ximgproc.so
        # libopencv_line_descriptor.so
        # libopencv_reg.so
        # libopencv_surface_matching.so
        # libopencv_xobjdetect.so
        # libopencv_datasets.so
        # libopencv_fuzzy.so
        # libopencv_ml.so
        # libopencv_rgbd.so
        # libopencv_text.so
        # libopencv_xphoto.so
        # libopencv_dnn_objdetect.so
        # libopencv_objdetect.so
        # libopencv_saliency.so
        # libopencv_tracking.so
)

set_target_properties(maix_image PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${export_dir}/lib/arch/r329")

set_target_properties(maix_image PROPERTIES LINK_FLAGS "-Wl,-rpath,/usr/local/lib/python3.9/dist-packages/maix/")

