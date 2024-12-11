rm -rf build
rm -rf bin
mkdir build
cd build/
cmake .. -G "MinGW Makefiles"
mingw32-make -j

