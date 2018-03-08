# client maintainer: simon.rit@creatis.insa-lyon.fr
set(CTEST_SITE "linux15.dg.creatis.insa-lyon.fr")
set(CTEST_BUILD_NAME "Linux-64bit-gcc-cuda-system-itk4-cov")
set(CTEST_BUILD_CONFIGURATION Debug)
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
set(CTEST_DASHBOARD_ROOT "/tmp/RTK_dashboard")
set(dashboard_binary_name "RTK_lin64_gcc_cuda_system_itk4_cov")
set(ENV{ITK_DIR} "/home/srit/src/itk/lin64-dg-debug")
set(ENV{CUDA_INC_PATH} "/usr/include/cuda")
set(CTEST_BUILD_FLAGS -j12)

# Coverage
SET(CTEST_COVERAGE_COMMAND "/usr/bin/gcov")
SET(ENV{CXXFLAGS} "-g -O0 -fprofile-arcs -ftest-coverage -Wno-deprecated -Wno-unused-local-typedefs -Wall")
SET(ENV{CFLAGS} "-g -O0 -fprofile-arcs -ftest-coverage -Wall")
SET(ENV{LDFLAGS} "-fprofile-arcs -ftest-coverage")
set(CTEST_CUSTOM_COVERAGE_EXCLUDE
    ${CTEST_CUSTOM_COVERAGE_EXCLUDE} # keep current exclude expressions
    "/cmake/"
    "/utilities/"
    ".ggo"
    )
set(CTEST_EXTRA_COVERAGE_GLOB "/code/*.h" "/code/*.cxx" "/code/*.hxx")
SET(CONFIGURE_OPTIONS ${CONFIGURE_OPTIONS}
                      -DFAST_TESTS_NO_CHECKS=TRUE)
set(dashboard_do_coverage true)

include("${CTEST_SCRIPT_DIRECTORY}/rtk_common.cmake")

