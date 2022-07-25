#!/bin/bash
build_dir="./dist"

if [ -d "$build_dir" ] ; then
    pushd $build_dir
    find -type f | xargs -n1 sha256sum  > ./check_file.txt
    rm_line=`awk '{if ($2  == "./check_file.txt") print NR }' ./check_file.txt`
    sed -i "${rm_line}d" ./check_file.txt
    popd

else
    echo "no ./dist ,please run make!"
fi
