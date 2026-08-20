# Vendored Azoteq TPS43 driver (patched)

Vendored from `geeksville/zmk_driver_azoteq` at master SHA `66d51024`
(2026-07-31, tree identical to `c329f309` "Add three_finger_swipe throttle";
same content beekeeb's Toucan2 builds against via their `main` branch).
Vendored 2026-08-19 instead of west-pinned so we could carry a local patch;
license in `LICENSE-azoteq` (MIT). The DT bindings live in
`../dts/bindings/input/`, wired up via `zephyr/module.yml`
(`cmake`/`kconfig`/`dts_root`).

## Local patch: input reports use K_NO_WAIT, not K_FOREVER

`tps43.c` upstream makes all 20 `input_report_*` calls with `K_FOREVER` on
the shared system workqueue — byte-for-byte the failure class of the July
2026 Cirque lockup (see `../HARDWARE_DEBUG.md`): under BLE split congestion
the input queue fills and a `K_FOREVER` report wedges the peripheral's
sysworkq. Patched to `K_NO_WAIT` (drop-under-backpressure), mirroring the
cirque fix we already run (`geeksville/cirque-input-module` PR #4, pinned
in `config/west.yml`).

Known tradeoff: under backpressure a paired press/release event can drop,
momentarily sticking a button — self-corrects on the next tap, vs. the
K_FOREVER alternative of a permanent wedge. The `k_sem_take(&lock,
K_FOREVER)` at tps43.c:401 is a data-lock, not an input report — left as-is.

## Updating

Diff against upstream before pulling anything: the patch must be re-applied
to any refreshed copy (grep for `K_FOREVER` — only the one `k_sem_take`
should remain). If the K_NO_WAIT change ever lands upstream, drop the
vendored copy and west-pin the upstream SHA instead.
