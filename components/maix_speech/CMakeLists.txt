
if(CONFIG_COMPONENT_LIBMS_ENABLE)

    list(APPEND ADD_INCLUDE "include")

    list(APPEND ADD_REQUIREMENTS pthread m)

    append_srcs_dir(ADD_SRCS "src")

    if(CONFIG_ASR_ENABLE)

        list(APPEND ADD_INCLUDE "Maix-Speech/components/asr_lib/include")
        list(APPEND ADD_INCLUDE "Maix-Speech/components/utils/include")

        ############ Add static libs ##################
        if(CONFIG_ARCH_X86)
            list(APPEND ADD_STATIC_LIB "Maix-Speech/components/asr_lib/lib/x86/libms_asr_x86.a")
        elseif(CONFIG_ARCH_R329)
            list(APPEND ADD_LINK_SEARCH_PATH "${CONFIG_TOOLCHAIN_PATH}/../lib")
            list(APPEND ADD_STATIC_LIB "Maix-Speech/components/asr_lib/lib/r329/libms_asr_r329.a")
            list(APPEND ADD_DYNAMIC_LIB "Maix-Speech/components/asr_lib/lib/r329/libaipudrv.so")
            list(APPEND ADD_DEFINITIONS -march=armv8-a)
        elseif(CONFIG_ARCH_V831 OR CONFIG_ARCH_V833)
            list(APPEND ADD_LINK_SEARCH_PATH "Maix-Speech/components/asr_lib/lib/v83x")
            list(APPEND ADD_STATIC_LIB "Maix-Speech/components/asr_lib/lib/v83x/libms_asr_v83x.a")
            # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-unused-variable -fPIC -c -s -ffunction-sections -fdata-sections -march=armv7-a  -mtune=cortex-a7" PARENT_SCOPE)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare -Wno-unused-variable -fPIC -fexceptions -s -ffunction-sections -fdata-sections -fpermissive -march=armv7-a  -mtune=cortex-a7" PARENT_SCOPE)
            list(APPEND ADD_DEFINITIONS -D__ARM__ -D__ARMV7__ -DCONF_KERNEL_IOMMU -DCONF_KERNEL_VERSION_4_9)
        elseif(CONFIG_ARCH_ARMV7)
            list(APPEND ADD_LINK_SEARCH_PATH "Maix-Speech/components/asr_lib/lib/armv7")
            list(APPEND ADD_STATIC_LIB "Maix-Speech/components/asr_lib/lib/armv7/libms_asr_armv7.a")
            list(APPEND ADD_DEFINITIONS -march=armv7-a)
            # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive" PARENT_SCOPE)
        elseif(CONFIG_ARCH_AARCH64)
            list(APPEND ADD_LINK_SEARCH_PATH "Maix-Speech/components/asr_lib/lib/aarch64" "${CONFIG_TOOLCHAIN_PATH}/../lib")
            list(APPEND ADD_STATIC_LIB "Maix-Speech/components/asr_lib/lib/aarch64/libms_asr_aarch64.a")
            list(APPEND ADD_DEFINITIONS -march=armv8-a)
            # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive" PARENT_SCOPE)
        elseif(CONFIG_ARCH_ARMV7MUSL)
            list(APPEND ADD_LINK_SEARCH_PATH "Maix-Speech/components/asr_lib/lib/armv7musl" "${CONFIG_TOOLCHAIN_PATH}/../lib")
            list(APPEND ADD_STATIC_LIB "Maix-Speech/components/asr_lib/lib/armv7musl/libms_asr_armv7musl.a")
            list(APPEND ADD_DEFINITIONS -march=armv7-a -g)
            # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive" PARENT_SCOPE)
        elseif(CONFIG_ARCH_RISCV64)
            list(APPEND ADD_LINK_SEARCH_PATH "Maix-Speech/components/asr_lib/lib/riscv64" "${CONFIG_TOOLCHAIN_PATH}/../lib")
            list(APPEND ADD_STATIC_LIB "Maix-Speech/components/asr_lib/lib/riscv64/libms_asr_riscv64.a")
            # list(APPEND ADD_DEFINITIONS -march=rv64gcvxtheadc)
            # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive" PARENT_SCOPE)
        endif()
        ###############################################

    endif()

    if(CONFIG_MFCC_ENABLE)

        # aux_source_directory("mfcc-with-vad/main"   MFCC_SRCS)
        aux_source_directory("mfcc-with-vad/src"   MFCC_SRCS)
        aux_source_directory("mfcc-with-vad/src/vad"   MFCC_SRCS)
        aux_source_directory("mfcc-with-vad/src/signal_processing"   MFCC_SRCS)

        list(APPEND ADD_INCLUDE
                "mfcc-with-vad"
                "mfcc-with-vad/include"
            )
        list(APPEND ADD_SRCS
                ${MFCC_SRCS}
            )
    endif()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g -Wno-strict-aliasing")

register_component(DYNAMIC)
endif()
