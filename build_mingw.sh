#! /bin/sh
# MinGW クロスコンパイルビルドスクリプト (Conan 使用)

if [ "$(uname -m)" = "aarch64" ]; then
  export CXXFLAG="-O3 -mcpu=native"
  export CFLAG="-O3 -mcpu=native"
else
  export CXXFLAG="-O3 -march=native"
  export CFLAG="-O3 -march=native"
fi

export CONAN_LOG_RUN_TO_OUTPUT=1
export CONAN_LOGGING_LEVEL=10
export CONAN_PRINT_RUN_COMMANDS=1

# MinGW クロスコンパイラのパスを環境変数で渡す（cmake は CMAKE_C_COMPILER / CMAKE_CXX_COMPILER を参照する）
export CC=x86_64-w64-mingw32-gcc
export CXX=x86_64-w64-mingw32-g++

conan install -of build_mingw64 . --build=missing --profile=mingw64
RET=$?
if [ "${RET}" -ne 0 ]; then
  exit "${RET}"
fi

cmake -B build_mingw64 \
  -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER="${CC}" \
  -DCMAKE_CXX_COMPILER="${CXX}" \
  -S .
RET=$?
if [ "${RET}" -ne 0 ]; then
  exit "${RET}"
fi

cmake --build build_mingw64 --verbose --parallel "$(nproc 2>/dev/null || echo 4)"
RET=$?
if [ "${RET}" -ne 0 ]; then
  exit "${RET}"
fi
