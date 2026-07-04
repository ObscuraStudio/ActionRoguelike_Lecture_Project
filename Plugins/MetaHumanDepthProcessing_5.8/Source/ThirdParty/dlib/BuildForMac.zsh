#!/bin/zsh

# Fail early
set -e

# Get number of available cores for the build.
NUM_CORES="$(sysctl -n hw.ncpu)"
echo "Number of available cores: ${NUM_CORES}"

if [ -d Build ]; then
  echo "Build directory exists, removing."
  rm -rf Build
fi

echo "Creating 'Build' directory."
mkdir Build
cd Build

# Setting DCMAKE_OSX_ARCHITECTURES to "x86_64;arm64" produces a 'fat binary' that can be linked on both Intel and ARM based systems.
# CMAKE_OSX_DEPLOYMENT_TARGET is set to macOS 11 as this is the version currently targetted when building UE on macOS (Jan 8th 2024).
# Setting DCMAKE_CXX_FLAGS to "-fvisibility=hidden" in order to match UE's symbol visibility settings.
# Xcode should already be installed before running this script

echo "Using the following Xcode installation:"
echo "$(xcode-select -p)"

echo "Running cmake."
cmake -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" -DCMAKE_OSX_DEPLOYMENT_TARGET="11.0" -DCMAKE_INSTALL_PREFIX=Install -DCMAKE_CXX_FLAGS="-fvisibility=hidden" -DDLIB_USE_MKL_FFT=0 -DDLIB_USE_BLAS=OFF -DDLIB_USE_LAPACK=OFF -DDLIB_USE_CUDA=0 -DDLIB_JPEG_SUPPORT=0 -DDLIB_PNG_SUPPORT=0 -DDLIB_GIF_SUPPORT=0 -DDLIB_LINK_WITH_SQLITE3=0 ../Source

echo "Compiling dlib."
make -j${NUM_CORES}

echo "Installing dlib."
make install
mkdir -p ../Lib/Mac/Release
cp -f Install/lib/libdlib.a ../Lib/Mac/Release/libdlib.a

if [ -d ../Include/dlib ]; then
  rm -rf ../Include/dlib
fi
mkdir -p ../Include/dlib
cp -rf Install/include/dlib ../Include

cd ..
