# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(vast-checkerTests LANGUAGES CXX)

# ---- Tests ----

add_executable(vast-checker_test source/vast-checker_test.cpp)
target_link_libraries(vast-checker_test PRIVATE vast-checker_lib)
target_compile_features(vast-checker_test PRIVATE cxx_std_20)

add_test(NAME vast-checker_test COMMAND vast-checker_test)

# ---- End-of-file commands ----

add_folders(Test)
