if [ -z "$1" ]; then

  rm -f ./build/CMakeCache.txt
  INSTALL_LOC=./install
  cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug \
    -Dglfw3_DIR=/home/zhangxinlong/app/glfw3/lib/cmake/glfw3 \
    -DGLAD=/home/zhangxinlong/app/glad \
    -Dglm_DIR=/home/zhangxinlong/app/glm/cmake/glm \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_LOC

elif [ $1 = "c" ]; then

  cmake --build build -j12

elif [ $1 = "i" ]; then

  rm -rf $INSTALL_LOC
  cmake --install build
elif [ $1 = "r" ]; then

  cmake --build build --target run
fi

#ctest -j 12

