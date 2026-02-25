cmake_minimum_required(VERSION 3.20)
project(gnuplotpp VERSION 0.1.0 LANGUAGES CXX)

option(ENABLE_PLOTTING "Enable gnuplot rendering support" ON)
option(BUILD_TESTING "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(GNUPLOTPP_ENABLE_CPM "Enable CPM-managed external dependencies" OFF)
option(GNUPLOTPP_ENABLE_LOGGING "Enable fmt/spdlog logging integration when available" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/dependencies.cmake)

if(GNUPLOTPP_ENABLE_LOGGING AND NOT GNUPLOTPP_ENABLE_CPM)
  find_package(fmt CONFIG QUIET)
  find_package(spdlog CONFIG QUIET)
endif()

add_library(gnuplotpp
  src/figure.cpp
  src/gnuplot_backend.cpp
  src/presets.cpp
  src/svg_backend.cpp
  src/png_backend.cpp
)

add_library(gnuplotpp::gnuplotpp ALIAS gnuplotpp)

target_include_directories(gnuplotpp
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(gnuplotpp PUBLIC cxx_std_20)

if(TARGET nlohmann_json::nlohmann_json)
  target_link_libraries(gnuplotpp PUBLIC nlohmann_json::nlohmann_json)
endif()

if(GNUPLOTPP_ENABLE_LOGGING)
  if(TARGET fmt::fmt)
    target_link_libraries(gnuplotpp PUBLIC fmt::fmt)
    target_compile_definitions(gnuplotpp PUBLIC GNUPLOTPP_HAS_FMT=1)
  elseif(TARGET fmt::fmt-header-only)
    target_link_libraries(gnuplotpp PUBLIC fmt::fmt-header-only)
    target_compile_definitions(gnuplotpp PUBLIC GNUPLOTPP_HAS_FMT=1)
  endif()

  if(TARGET spdlog::spdlog)
    target_link_libraries(gnuplotpp PUBLIC spdlog::spdlog)
    target_compile_definitions(gnuplotpp PUBLIC GNUPLOTPP_HAS_SPDLOG=1)
  elseif(TARGET spdlog::spdlog_header_only)
    target_link_libraries(gnuplotpp PUBLIC spdlog::spdlog_header_only)
    target_compile_definitions(gnuplotpp PUBLIC GNUPLOTPP_HAS_SPDLOG=1)
  endif()
endif()

if(BUILD_EXAMPLES)
  add_executable(two_window_example examples/two_window_example.cpp)
  target_link_libraries(two_window_example PRIVATE gnuplotpp::gnuplotpp)

  add_executable(layout_2x2_example examples/layout_2x2_example.cpp)
  target_link_libraries(layout_2x2_example PRIVATE gnuplotpp::gnuplotpp)
endif()

if(BUILD_TESTING)
  enable_testing()

  add_executable(gnuplotpp_api_test tests/api_test.cpp)
  target_link_libraries(gnuplotpp_api_test PRIVATE gnuplotpp::gnuplotpp)
  add_test(NAME gnuplotpp_api_test COMMAND gnuplotpp_api_test)

  add_executable(gnuplotpp_gnuplot_backend_test tests/gnuplot_backend_test.cpp)
  target_link_libraries(gnuplotpp_gnuplot_backend_test PRIVATE gnuplotpp::gnuplotpp)
  add_test(NAME gnuplotpp_gnuplot_backend_test COMMAND gnuplotpp_gnuplot_backend_test)

  add_executable(gnuplotpp_svg_backend_test tests/svg_backend_test.cpp)
  target_link_libraries(gnuplotpp_svg_backend_test PRIVATE gnuplotpp::gnuplotpp)
  add_test(NAME gnuplotpp_svg_backend_test COMMAND gnuplotpp_svg_backend_test)

  add_executable(gnuplotpp_png_backend_test tests/png_backend_test.cpp)
  target_link_libraries(gnuplotpp_png_backend_test PRIVATE gnuplotpp::gnuplotpp)
  add_test(NAME gnuplotpp_png_backend_test COMMAND gnuplotpp_png_backend_test)
endif()
