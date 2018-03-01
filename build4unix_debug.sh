rm -rf unix_build_debug
mkdir unix_build_debug
cd unix_build_debug
cmake -DCMAKE_BUILD_TYPE=debug ..
make
cd ..
