# vast-checker

This is the vast-checker project.

# Building

```sh
cmake -S . -B build --toolchain ./external/vast/cmake/lld.toolchain.cmake \
                    -D CMAKE_BUILD_TYPE=Release \
                    -D CMAKE_PREFIX_PATH=/usr/lib/llvm-15
cmake --build build
```

# Contributing

See the [CONTRIBUTING](CONTRIBUTING.md) document.

# Licensing

<!--
Please go to https://choosealicense.com/licenses/ and choose a license that
fits your needs. The recommended license for a project of this type is the
GNU AGPLv3.
-->
