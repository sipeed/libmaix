# 此 Makefile 是为了方便 libmaix 用户调试程序而编写的一个制作脚本。
# 功能一: make , make clean , make menuconfig
# 		用于编译工程,清理工程，打开编译菜单
#
# 功能二： make push
#		用于上传工程到用户目录
#
# 功能三： make test, make run
#		用于进行程序测试，该指令可以加参数. 列如 make test adb="-d2 -f one"
#
# 功能四： make forward
#		建立 V831 和设备之间的tcp链接通道
#
# 功能五： make ssh_clean
#		清理 ssh 连接的旧数据。当make push 提示 sshpass 失败时使用。
#
# 功能六： make shell
#		进入开发板的终端
#
# 功能七： make pc_init
# 		自动配置开发环境

.PHONY: target

adb ?=

# please run after this
# sudo apt install sshpass expect rsync -y

# rsync -arvlz -e 'ssh -p 15630' /home/nihao/work/work_space/libmaix/examples/smart/dist root@127.0.0.1:/root


define func1
	@ echo "/usr/bin/expect << EOF" >> $(1)
	@ echo "set time 30" >> $(1)
	@ echo "spawn ssh root@127.0.0.1 -p 15630" >> $(1)
	@ echo "expect {" >> $(1)
	@ echo "\"*yes/no\" { send \"yes\\n\"; exp_continue }" >> $(1)
	@ echo "\"*password:\" { send \"root\\n\"; exp_continue}" >> $(1)
	@ echo "\"*~#\" {send \"exit\\n\"}" >> $(1)
	@ echo "}" >> $(1)
	@ echo "expect eof" >> $(1)
	@ echo "EOF" >> $(1)
	@ bash $(1); rm $(1)
endef

ssh_port=$(shell lsof -i:15630 | grep adb | awk '{print $$1}')

ifeq ("$(ssh_port)", "")
	push_s=adb push ./dist /root
else
	push_s=sshpass -p "root" rsync -rvlzt -e 'ssh -p 15630' ./dist root@127.0.0.1:/root
endif

target:
	python3 project.py build
	@if [ -f "./adb_push_dist.py" ] ; then python3 ./adb_push_dist.py ; fi
	# @if [ -f "./build_dist.sh" ] ; then ./build_dist.sh ; fi >> /dev/null
	adb shell "cd /root/dist; ./start_app.sh \"$(adb)\" "

clean:
	$(RM) -r build/ dist

menuconfig:
	python3 project.py menuconfig

# # 上传程序到开发板
# push:
# 	adb push dist /root

# 安装依赖选项
pc_init:
	sudo apt install cmake git sshpass expect make rsync adb -y
	wget http://mirrors.kernel.org/ubuntu/pool/main/libf/libffi/libffi6_3.2.1-8_amd64.deb
	sudo dpkg -i ./libffi6_3.2.1-8_amd64.deb
	sudo apt-get install uuid-dev
	wget https://github.com/sipeed/MaixPy3/releases/download/20210613/v83x_linux_x86_python3.8_toolchain.zip
	unzip unzip v83x_linux_x86_python3.8_toolchain.zip
	sudo mkdir /opt/v83x_linux_x86_python3.8_toolchain
	cp -r ./v83x_linux_x86_python3.8_toolchain/* /opt/v83x_linux_x86_python3.8_toolchain
	sed -i "7c \    \       default\ \"/opt/v831/toolchain-sunxi-musl/toolchain/bin/\"" ../../Kconfig
	sed -i "11c \	\	default\ \"arm-openwrt-linux-muslgnueabi-\"" ../../Kconfig

# 运行程序
run:
	adb shell "cd /root/dist; ./start_app.sh"

# 进入开发板 shell 环境
shell:
	adb shell $(adb)

# 清除 ssh 残留密钥
ssh_clean:
	adb forward tcp:15630 tcp:22
	ssh-keygen -R [127.0.0.1]:15630 -f /home/`whoami`/.ssh/known_hosts
	$(call func1, $(shell mktemp))

# 建立开发板 tcp 通道
forward:
	bash -c "if [ -f \"../../tools/rsync\" ] ; then adb push ../../tools/rsync /bin ; fi"
	adb forward tcp:15630 tcp:22

# 上传程序到开发板
push:
	$(push_s)

# 自定义功能测试
test:
	make
	make push
	adb shell "cd /root/dist; ./start_app.sh \"$(adb)\" "

# 运行 self_test 功能测试
self_test:
	make
	make push
	adb shell "cd /root/dist; ./start_app.sh \"-d -t\""

# 设定程序循环执行
loop:
	make
	make push
	adb shell "cd /root/dist; ./start_app.sh \"-d -l 5\""
