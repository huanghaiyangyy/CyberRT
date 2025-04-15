# protobuf
pushd "$CURRENT_PATH/../third_party/protobuf/"
git checkout v3.14.0
mkdir -p cmake/build && cd cmake/build

export ANDROID_NDK=/home/huang/Documents/android-ndk-r21e
cmake .. \
-DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK}/build/cmake/android.toolchain.cmake \
-DANDROID_ABI=arm64-v8a \
-DANDROID_PLATFORM=android-26 \
-Dprotobuf_BUILD_SHARED_LIBS=ON \
-Dprotobuf_BUILD_TESTS=OFF \
-DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX

make -j$(nproc)
sudo make install
popd