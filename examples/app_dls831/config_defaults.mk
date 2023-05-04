
#
# Toolchain configuration
#
CONFIG_TOOLCHAIN_PATH="/opt/toolchain-sunxi-musl/toolchain/bin"
CONFIG_TOOLCHAIN_PREFIX="arm-openwrt-linux-muslgnueabi-"
# end of Toolchain configuration

#
# Target chip configuration
#
CONFIG_ARCH_V831=y
# CONFIG_ARCH_V833 is not set
# CONFIG_ARCH_R329 is not set

#
# upload dist configuration, only need if you use python3 project.py upload command
#
CONFIG_TARGET_IP="192.168.0.123"
CONFIG_TARGET_USER="root"
CONFIG_TARGET_DIST_DIR="/root/maix_dist"
CONFIG_TARGET_USER_PASSWD=""
# end of upload dist configuration, only need if you use python3 project.py upload command
# end of Target chip configuration

#
# Components configuration
#

#
# libmaix configuration
#
CONFIG_LIBMAIX_ENABLED=y
# end of libmaix configuration

CONFIG_COMPONENT_MAIX_DLS831_ENABLE=y

CONFIG_COMPONENT_3RD_PARTY_ENABLE=n

CONFIG_COMPONENT_IMAGE_ENABLE=n

#
# component libms configuration
#

# end of component libms configuration
# end of Components configuration
