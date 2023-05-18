# vast-checker

VAST-checker is a [VAST](https://github.com/trailofbits/vast)-based tool that
scans C code for variants of the
[Sequoia](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2021-33909) bug. It
is intended to be run after translating C code to
[](https://github.com/trailofbits/vast/blob/master/docs/HighLevel/HighLevel.md)VAST's
`hl` MLIR dialect via the `vast-front` tool that ships with VAST. The main
purpose of the project is to demonstrate the capabilities of VAST and MLIR in
bug hunting and program analysis use cases.

# Building on Ubuntu 22.04

```sh

wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo add-apt-repository 'deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main'

sudo apt-get update && sudo apt-get install -y --no-install-recommends \
  clang-16 \
  clang-tools-16 \
  libclang-common-16-dev \
  libclang-16-dev \
  llvm-16 \
  llvm-16-dev \
  libllvm16 \
  libmlir-16-dev \
  mlir-16-tools \
  lld-16
```

```sh
git clone --recursive https://github.com/trailofbits/vast-checker.git
```

```sh
cmake -S vast-checker -B build --toolchain ./external/vast/cmake/lld.toolchain.cmake \
                      -D CMAKE_BUILD_TYPE=Release \
                      -D CMAKE_PREFIX_PATH=/usr/lib/llvm-16 \
                      -D CMAKE_C_COMPILER=clang-16 \
                      -D CMAKE_CXX_COMPILER=clang++-16
```

```sh
cmake --build build -j$(nproc)
```

# Usage

```sh
./build/external/vast/tools/vast-front/vast-front \
                      -vast-emit-mlir=hl \
                      -o extract.hl.mlir \
                      vast-checker/sequoia/extract.c
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
