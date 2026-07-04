#!/bin/bash

# Fail early
set -e

# Get number of available cores for the build.
NUM_CORES="$(nproc)"
echo "Number of available cores: ${NUM_CORES}"

# Locate the UE Linux toolchain clang. Checks in priority order:
#   1. LINUX_MULTIARCH_ROOT environment variable (set by UE build environment)
#   2. In-tree SDK: Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/<version>/x86_64-unknown-linux-gnu/
# Walk up from this script's location to find the workspace root (the directory containing Engine/).
# Script lives at Engine/Restricted/LimitedAccess/Plugins/MetaHumanDepthProcessing/Source/ThirdParty/dlib/
# so the workspace root is 8 levels up.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_ROOT="$(cd "${SCRIPT_DIR}/../../../../../../../.." && pwd)"

find_clang_in_dir() {
  local DIR="$1"
  local BIN="${DIR}/x86_64-unknown-linux-gnu/bin/clang++"
  if [ -f "${BIN}" ]; then echo "${BIN}"; return; fi
  BIN="${DIR}/bin/clang++"
  if [ -f "${BIN}" ]; then echo "${BIN}"; fi
}

if [ -n "${LINUX_MULTIARCH_ROOT}" ]; then
  TOOLCHAIN_VERSION_DIR="$(ls -d "${LINUX_MULTIARCH_ROOT}"/v*/ 2>/dev/null | head -1)"
  CLANG="$(find_clang_in_dir "${TOOLCHAIN_VERSION_DIR}")"
  TOOLCHAIN_SOURCE="LINUX_MULTIARCH_ROOT (${LINUX_MULTIARCH_ROOT})"
fi

if [ -z "${CLANG}" ]; then
  TOOLCHAIN_ROOT="${WORKSPACE_ROOT}/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64"
  if [ -d "${TOOLCHAIN_ROOT}" ]; then
    TOOLCHAIN_VERSION_DIR="$(ls -d "${TOOLCHAIN_ROOT}"/v*/ 2>/dev/null | head -1)"
    CLANG="$(find_clang_in_dir "${TOOLCHAIN_VERSION_DIR}")"
    TOOLCHAIN_SOURCE="in-tree SDK (${TOOLCHAIN_ROOT})"
  fi
fi

if [ -n "${CLANG}" ]; then
  echo "Using UE toolchain clang from ${TOOLCHAIN_SOURCE}: ${CLANG}"
  CMAKE_EXTRA="-DCMAKE_CXX_COMPILER=${CLANG} -DCMAKE_C_COMPILER=$(dirname ${CLANG})/clang"
else
  echo "UE toolchain not found (LINUX_MULTIARCH_ROOT not set and in-tree SDK missing), falling back to system clang."
  echo "To use the UE toolchain, sync Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/... or set LINUX_MULTIARCH_ROOT."
  CMAKE_EXTRA=""
  echo "Using system clang: $(clang++ --version | head -1)"
fi

if [ -d Build ]; then
  echo "Build directory exists, removing."
  rm -rf Build
fi

echo "Creating 'Build' directory."
mkdir Build
cd Build

# Setting DCMAKE_CXX_FLAGS to "-fvisibility=hidden" to match UE's symbol visibility settings.
echo "Running cmake."
cmake ${CMAKE_EXTRA} \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=Install \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DCMAKE_CXX_FLAGS="-fvisibility=hidden -stdlib=libc++ -Wno-missing-template-arg-list-after-template-kw" \
  -DCMAKE_EXE_LINKER_FLAGS="-stdlib=libc++" \
  -DDLIB_USE_MKL_FFT=0 \
  -DDLIB_USE_BLAS=OFF \
  -DDLIB_USE_LAPACK=OFF \
  -DDLIB_USE_CUDA=0 \
  -DDLIB_JPEG_SUPPORT=0 \
  -DDLIB_PNG_SUPPORT=0 \
  -DDLIB_GIF_SUPPORT=0 \
  -DDLIB_LINK_WITH_SQLITE3=0 \
  ../Source

echo "Compiling dlib."
make -j${NUM_CORES}

echo "Installing dlib."
make install
mkdir -p ../Lib/Linux/Release
cp -f Install/lib/libdlib.a ../Lib/Linux/Release/libdlib.a

if [ -d ../Include/dlib ]; then
  rm -rf ../Include/dlib
fi
mkdir -p ../Include/dlib
cp -rf Install/include/dlib ../Include

cd ..
