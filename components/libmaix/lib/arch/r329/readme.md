
## create r329 *.so 

sudo apt install gcc-aarch64-linux-gnu

mkdir build

cd build

cmake ..

make

## libmaix 

cd examples/hello-world

python project.py --toolchain /usr/bin --toolchain-prefix aarch64-linux-gnu- config

python project.py menuconfig

**check is Chip architecture (compile for arch R329)**

python project.py build
