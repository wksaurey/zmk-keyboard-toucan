# APPLICATION_SOURCE_DIR (= zmk/app) include is the fix for the vendored
# accel processor: upstream's CMake never adds ZMK's include dir, so
# <drivers/input_processor.h> is unfindable — same one-liner the working
# zmk-input-zoom module uses.
zephyr_library()
zephyr_library_include_directories(${APPLICATION_SOURCE_DIR}/include)
zephyr_library_sources_ifdef(CONFIG_ZMK_INPUT_PROCESSOR_ACCELERATION input_processor_accel.c)
