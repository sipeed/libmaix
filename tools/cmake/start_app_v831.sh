#!/bin/sh

curr_dir=$(cd `dirname $0`; pwd)

export LD_LIBRARY_PATH=$curr_dir/maix_dist/lib:./:./lib:LD_LIBRARY_PATH:/usr/lib:/opt/lib:/usr/lib/eyesee-mpp

# echo $LD_LIBRARY_PATH
