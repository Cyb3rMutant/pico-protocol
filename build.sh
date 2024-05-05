if [ $(basename $PWD) = "build" ]; then
  cd ..
fi
rm -rf build
mkdir build
cd build
cmake ..
make -j $(nproc)
