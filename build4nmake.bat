rem need run vcvarsall.bat
rmdir /S /Q nmake_build
mkdir nmake_build
cd nmake_build
cmake -G "NMake Makefiles" ..
nmake -f makefile
cd ..
