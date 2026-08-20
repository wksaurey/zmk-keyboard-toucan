# Build-time version stamp: short SHA of this config repo, consumed by
# the display's version label and the boot log (version/). Fail-safe:
# never breaks the build — falls back to "unknown".
execute_process(
  COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR} describe --always --dirty
  OUTPUT_VARIABLE TOUCAN_FW_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
  RESULT_VARIABLE _toucan_git_rc)
if(NOT _toucan_git_rc EQUAL 0 OR "${TOUCAN_FW_VERSION}" STREQUAL "")
  set(TOUCAN_FW_VERSION "unknown")
endif()
message(STATUS "toucan firmware version: ${TOUCAN_FW_VERSION}")
configure_file(version/toucan_version.h.in
               ${CMAKE_BINARY_DIR}/toucan_generated/toucan_version.h @ONLY)
zephyr_include_directories(${CMAKE_BINARY_DIR}/toucan_generated)

add_subdirectory(drivers)
add_subdirectory(version)
