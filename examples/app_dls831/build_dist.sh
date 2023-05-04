#!/bin/bash
build_dir="./maix_dist"

if [ -d "$build_dir" ] ; then
    pushd $build_dir
    find -type f | xargs -n1 sha256sum  > ./maix_dist.sha256sum
    rm_line=`awk '{if ($2  == "./maix_dist.sha256sum") print NR }' ./maix_dist.sha256sum`
    sed -i "${rm_line}d" ./maix_dist.sha256sum
    popd

else
    echo "no ./dist ,please run make!"
fi
