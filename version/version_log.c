/* Logs the config-repo version once at boot, on BOTH halves — the right
 * half has no display, so this line over the USB console (zmk-usb-logging
 * snippet) is its flash-took check. Early-boot lines can be lost before
 * USB enumerates; capture with the cable attached and a terminal open
 * across a reset (the toucan-logger rig does this). */
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <toucan_version.h>

LOG_MODULE_REGISTER(toucan_version, LOG_LEVEL_INF);

static int toucan_version_log_init(void) {
    LOG_INF("toucan firmware " TOUCAN_FW_VERSION);
    return 0;
}

SYS_INIT(toucan_version_log_init, APPLICATION, 99);
