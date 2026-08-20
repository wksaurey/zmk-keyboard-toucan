# Build-time version stamp: short SHA of this config repo, consumed by
# the display's version label and the boot log (version/). In CI the
# SHA comes from GITHUB_SHA (git describe fails in the build container —
# dubious-ownership refusal, not overridable per-invocation; verified
# "unknown" in run of 2026-08-20); git describe covers local builds.
# Fail-safe: never breaks the build — falls back to "unknown".
if(DEFINED ENV{GITHUB_SHA} AND NOT "$ENV{GITHUB_SHA}" STREQUAL "")
  string(SUBSTRING "$ENV{GITHUB_SHA}" 0 7 TOUCAN_FW_VERSION)
else()
  execute_process(
    COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR} describe --always --dirty
    OUTPUT_VARIABLE TOUCAN_FW_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _toucan_git_rc)
  if(NOT _toucan_git_rc EQUAL 0 OR "${TOUCAN_FW_VERSION}" STREQUAL "")
    set(TOUCAN_FW_VERSION "unknown")
  endif()
endif()
message(STATUS "toucan firmware version: ${TOUCAN_FW_VERSION}")
configure_file(version/toucan_version.h.in
               ${CMAKE_BINARY_DIR}/toucan_generated/toucan_version.h @ONLY)
zephyr_include_directories(${CMAKE_BINARY_DIR}/toucan_generated)

add_subdirectory(drivers)
add_subdirectory(version)
