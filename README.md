# vast-checker

This is the vast-checker project.

# Building on Ubuntu 22.04

```sh
sudo apt-get update && sudo apt-get install -y --no-install-recommends \
  clang-15 \
  clang-tools-15 \
  libclang-common-15-dev \
  libclang-15-dev \
  llvm-15 \
  llvm-15-dev \
  libllvm15 \
  libmlir-15-dev \
  mlir-15-tools \
  lld-15
```

```sh
git clone --recursive https://github.com/trailofbits/vast-checker.git
```

```sh
cmake -S vast-checker -B build --toolchain ./external/vast/cmake/lld.toolchain.cmake \
                      -D CMAKE_BUILD_TYPE=Release \
                      -D CMAKE_PREFIX_PATH=/usr/lib/llvm-15 \
                      -D CMAKE_C_COMPILER=clang-15 \
                      -D CMAKE_CXX_COMPILER=clang++-15
```

```sh
cmake --build build -j$(nproc)
```

# Usage

```sh
./build/external/vast/tools/vast-front/vast-front -vast-emit-mlir=hl vast-checker/sequoia/extract.c -o extract.hl.mlir
```

```sh
./build/vast-checker -sequoia extract.hl.mlir > /dev/null

```

# Contributing

See the [CONTRIBUTING](CONTRIBUTING.md) document.

# Licensing

<!--
Please go to https://choosealicense.com/licenses/ and choose a license that
fits your needs. The recommended license for a project of this type is the
GNU AGPLv3.
-->
