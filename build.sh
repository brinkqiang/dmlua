rm -rf unix_build
mkdir unix_build
cd unix_build
cmake -DCMAKE_BUILD_TYPE=relwithdebinfo ..
make
cd ..
