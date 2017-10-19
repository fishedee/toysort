#rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake
cmake -DCMAKE_BUILD_TYPE=Release .
make
./toysort
