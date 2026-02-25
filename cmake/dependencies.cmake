if(NOT GNUPLOTPP_ENABLE_CPM)
  return()
endif()

set(CPM_DOWNLOAD_VERSION 0.40.3)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if(NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
  message(STATUS "Downloading CPM.cmake v${CPM_DOWNLOAD_VERSION}")
  file(
    DOWNLOAD
    "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
    ${CPM_DOWNLOAD_LOCATION}
    EXPECTED_HASH SHA256=302f3ee95d731d94def0707260f6f8c9d7079682933c5d5c6d4218f292362766
  )
endif()

include(${CPM_DOWNLOAD_LOCATION})

CPMAddPackage(
  NAME nlohmann_json
  GITHUB_REPOSITORY nlohmann/json
  VERSION 3.11.3
  OPTIONS "JSON_BuildTests OFF"
)
