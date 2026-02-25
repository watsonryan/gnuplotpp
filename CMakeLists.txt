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
  src/data.cpp
  src/facet.cpp
  src/figure.cpp
  src/gnuplot_backend.cpp
  src/gnuplot_backend_emit.cpp
  src/presets.cpp
  src/spec_yaml.cpp
  src/statistics.cpp
  src/templates.cpp
  src/theme.cpp
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

  add_executable(three_line_ieee_example examples/three_line_ieee_example.cpp)
  target_link_libraries(three_line_ieee_example PRIVATE gnuplotpp::gnuplotpp)

  add_executable(monte_carlo_alpha_example examples/monte_carlo_alpha_example.cpp)
  target_link_libraries(monte_carlo_alpha_example PRIVATE gnuplotpp::gnuplotpp)

  add_executable(feature_rich_showcase examples/feature_rich_showcase.cpp)
  target_link_libraries(feature_rich_showcase PRIVATE gnuplotpp::gnuplotpp)

  add_executable(interactive_facet_example examples/interactive_facet_example.cpp)
  target_link_libraries(interactive_facet_example PRIVATE gnuplotpp::gnuplotpp)

  add_executable(yaml_spec_example examples/yaml_spec_example.cpp)
  target_link_libraries(yaml_spec_example PRIVATE gnuplotpp::gnuplotpp)

  add_executable(stats_plot_examples examples/stats_plot_examples.cpp)
  target_link_libraries(stats_plot_examples PRIVATE gnuplotpp::gnuplotpp)

  add_executable(tufte_minimal_example examples/tufte_minimal_example.cpp)
  target_link_libraries(tufte_minimal_example PRIVATE gnuplotpp::gnuplotpp)

  if(NOT WIN32)
    add_custom_target(refresh-readme-assets
      COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/refresh_readme_assets.sh ${CMAKE_BINARY_DIR}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Regenerating README-linked figure assets from examples")
  endif()
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
