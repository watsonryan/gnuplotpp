if(NOT GNUPLOTPP_ENABLE_CPM)
  return()
endif()

set(CPM_DOWNLOAD_VERSION 0.40.3)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if(NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
  message(STATUS "Downloading CPM.cmake v${CPM_DOWNLOAD_VERSION}")
  file(DOWNLOAD
    "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
    ${CPM_DOWNLOAD_LOCATION}
    EXPECTED_HASH SHA256=302f3ee95d731d94def0707260f6f8c9d7079682933c5d5c6d4218f292362766
    STATUS cpm_download_status
    LOG cpm_download_log
  )
  list(GET cpm_download_status 0 cpm_download_code)
  if(NOT cpm_download_code EQUAL 0)
    list(GET cpm_download_status 1 cpm_download_reason)
    message(WARNING "CPM download failed (${cpm_download_reason}); continuing without CPM packages.")
    return()
  endif()
endif()

include(${CPM_DOWNLOAD_LOCATION})
if(NOT COMMAND CPMAddPackage)
  message(WARNING "CPMAddPackage is unavailable; continuing without CPM packages.")
  return()
endif()

CPMAddPackage(
  NAME nlohmann_json
  GITHUB_REPOSITORY nlohmann/json
  VERSION 3.11.3
  OPTIONS "JSON_BuildTests OFF"
)

CPMAddPackage(
  NAME fmt
  GITHUB_REPOSITORY fmtlib/fmt
  VERSION 11.0.2
)

CPMAddPackage(
  NAME spdlog
  GITHUB_REPOSITORY gabime/spdlog
  VERSION 1.14.1
  OPTIONS "SPDLOG_FMT_EXTERNAL ON"
)

CPMAddPackage(
  NAME yaml-cpp
  GITHUB_REPOSITORY jbeder/yaml-cpp
  VERSION 0.8.0
  OPTIONS "YAML_CPP_BUILD_TESTS OFF" "YAML_CPP_BUILD_TOOLS OFF" "YAML_CPP_BUILD_CONTRIB OFF"
)
