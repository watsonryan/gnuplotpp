cmake_minimum_required(VERSION 3.20)
project(gnuplotpp VERSION 0.1.0 LANGUAGES CXX)

option(ENABLE_PLOTTING "Enable gnuplot rendering support" ON)
option(BUILD_TESTING "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(gnuplotpp
  src/figure.cpp
  src/gnuplot_backend.cpp
  src/presets.cpp
)

add_library(gnuplotpp::gnuplotpp ALIAS gnuplotpp)

target_include_directories(gnuplotpp
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(gnuplotpp PUBLIC cxx_std_20)

if(BUILD_EXAMPLES)
  add_executable(two_window_example examples/two_window_example.cpp)
  target_link_libraries(two_window_example PRIVATE gnuplotpp::gnuplotpp)
endif()

if(BUILD_TESTING)
  enable_testing()

  add_executable(gnuplotpp_api_test tests/api_test.cpp)
  target_link_libraries(gnuplotpp_api_test PRIVATE gnuplotpp::gnuplotpp)
  add_test(NAME gnuplotpp_api_test COMMAND gnuplotpp_api_test)

  add_executable(gnuplotpp_gnuplot_backend_test tests/gnuplot_backend_test.cpp)
  target_link_libraries(gnuplotpp_gnuplot_backend_test PRIVATE gnuplotpp::gnuplotpp)
  add_test(NAME gnuplotpp_gnuplot_backend_test COMMAND gnuplotpp_gnuplot_backend_test)
endif()
