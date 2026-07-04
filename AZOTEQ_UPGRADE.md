# Azoteq TPS43 upgrade — planning doc

Kolter ordered beekeeb's **Toucan Upgrade Kit** (2026-07-04, $48, ships
late-Jul/late-Aug 2026): swaps the right half's Cirque Pinnacle (SPI) for an
**Azoteq TPS43** multitouch pad (IQS550 controller, I2C, on-chip gestures).
This doc is the prep so the swap goes smoothly — researched 2026-07-04 from
beekeeb's driver + demo repos and the shop/blog pages, BEFORE the kit
arrived and BEFORE beekeeb published their conversion guide.

**Standing decision: this upgrade is a TWO-WAY DOOR.** Kolter may prefer the
Cirque and revert. Every step below preserves the revert path. Do not let a
step become one-way without an explicit OK.

## What's in the kit

TPS43 pad + custom glass cover, converter board + FPC cable, 7P connector
cable, right-side bottom case piece, acrylic cover, tape/feet/screws.
**No soldering either direction** — the Cirque comes out unplugged and
intact. Fits Thumb-Angled or Column-Angled cases, 36/42-key.
Source: shop.beekeeb.com/products/toucan-upgrade-kit

Revert kit to preserve on day 1: the removed Cirque + its cover + original
bottom case piece + original tape, bagged and labeled. The only likely
one-way step is adhesive (trackpad tape / glass mounting) — check the guide
when it exists, and prefer the removable route if there's a choice.

## The #1 firmware risk — the cirque bug, again

`beekeeb/zmk_driver_azoteq` (and its upstream `geeksville/zmk_driver_azoteq`)
reports input with **`K_FOREVER` on the shared system workqueue** — all 20
`input_report_*` calls, no dedicated thread, no queue Kconfig (verified in
`drivers/input/tps43.c` @ 2026-07-04). This is byte-for-byte the failure
class of the July 2026 lockup (see HARDWARE_DEBUG.md): under BLE split
congestion the input queue fills and a `K_FOREVER` report wedges the
peripheral's sysworkq — and multitouch gesture traffic is *heavier* than the
Cirque's.

**Plan: patch before first flash, not after the first hang.** Mirror the
cirque `K_NO_WAIT` fix (geeksville cirque PR #4) onto the azoteq driver —
swap the `K_FOREVER` timeouts to `K_NO_WAIT` in `tps43.c` (drop-under-
backpressure). Fork + pin our SHA in west.yml, and offer the patch upstream
to geeksville. The central-side protections from the lockup fix
(`INPUT_THREAD_STACK_SIZE=4096` etc. in toucan_left.conf) stay REQUIRED —
the relay fan-out on the central is the same regardless of which pad feeds
it.

## Module pinning gotcha

beekeeb's driver fork is **2 commits BEHIND** geeksville/master and their own
demo builds from `remote: geeksville`, not their fork. The missing delta is a
real fix (ignore scroll/zoom/move while 3 fingers active — originated as
beekeeb PR #1, merged upstream, never pulled back). **Pin
`geeksville/zmk_driver_azoteq` at an explicit SHA** (or our K_NO_WAIT fork of
it), never `beekeeb/zmk_driver_azoteq` and never a bare branch.

## ZMK version decision

- beekeeb's demo targets ZMK `main` ("slated for v0.4"); that's the only
  combo they test.
- BUT source inspection found **no main-only API in the driver**, and ZMK
  v0.3 already ships the full pointing subsystem the demo relies on
  (`input_split.c`, `input_listener`, `CONFIG_ZMK_POINTING` — Kconfig
  byte-identical to main). Both v0.3 and main ride Zephyr 3.5.
- **Recommendation: attempt on our pinned v0.3 first.** It keeps the revert
  trivial (same base as the cirque config), keeps nice_view_gem untouched
  (its upstream API rename only bites at ZMK ≥0.4 — see CLAUDE.md), and CI
  will say quickly whether it compiles. Fall back to the 0.4/main rebase
  only if the build or behavior forces it — and then the nice-view-gem
  rename fix becomes mandatory in the same change.
- Caveat: compile-verified ≠ behavior-verified; smooth-scroll/HID defaults
  evolved on main. Trial period will tell.

## Integration map (what changes in this repo)

Reversibility rule: ADD alongside, never replace. Target: both right-half
firmwares (cirque + azoteq) build from the same branch.

1. `config/west.yml`: ADD project `zmk_driver_azoteq` (geeksville remote or
   our fork, pinned SHA). KEEP the cirque-input-module pin.
2. New shield or overlay variant for the azoteq right half (e.g.
   `toucan_right_azoteq`), containing:
   - Enable `&xiao_i2c` (currently `status = "disabled"` in
     toucan_right.overlay:5 for the cirque build) with `bias-pull-up`.
   - `trackpad@74` node, compatible `azoteq,tps43`. **`reg = <0x74>` is
     BUILD_ASSERTed — not relocatable.** `rdy-gpios`/`rst-gpios` = the kit's
     converter-board pins — **UNKNOWN until beekeeb publishes** (demo used
     raw `&gpio1 12/13` on a non-Toucan board; do not copy blindly).
   - Gesture enables (DT booleans, all default-off): start with
     `single-tap`, `two-finger-tap`, `press-and-hold`, `scroll`; hold
     `swipes`/`zoom` for a second pass (they emit BTN_WEST/EAST/NORTH/SOUTH
     and REL_MISC and need explicit mapping to be useful).
   - Orientation: only `invert-x/y`, `switch-xy` exist (no rotation angle).
     Demo used `switch-xy` + `invert-scroll-y`; expect trial and error.
   - Split topology unchanged: `zmk,input-split` on the right with
     `device = <&trackpad>`, proxy + `zmk,input-listener` on the left —
     same shape as today's cirque nodes in toucan.dtsi.
3. Right-half conf (azoteq variant): `CONFIG_INPUT_TPS43=y` (+ keep
   `CONFIG_ZMK_POINTING=y`). Note `CONFIG_INPUT_TPS43` defaults `y` when
   I2C+INPUT are on — the cirque build stays clean only because its I2C bus
   is disabled; keep it disabled there.
4. `build.yaml`: ADD an azoteq right-half entry; KEEP the cirque entry.
   Sanity: `CONFIG_INPUT_PINNACLE` and the cirque node must not leak into
   the azoteq build (bus separation handles it: SPI pad vs I2C pad).
5. Archive the current known-good uf2 set (post-lockup-fix, cirque) before
   first azoteq flash — if reverting after a ZMK rebase, the binaries are
   the safety net, not the build.

## Driver tuning reference (DT props, condensed)

Sensitivity: `sensitivity` (cursor, %100 default), `scroll-sensitivity`
(**binding default 50 vs C fallback 100 — set it explicitly**),
`zoom-sensitivity`. Thresholds: `tap-time/-distance`, `hold-time`,
`scroll-initial-distance`, `scroll-angle`, `swipe-*`, `zoom-*-distance`.
Filters: `filter-settings` (default 0x0F; demo used 0x0B),
`filter-dynamic-*`. Power: `enable-power-management` (default ON),
`idle-sleep` (also sleep on ZMK IDLE — the cirque equivalent of this caused
the 300 ms wake lag we disabled; start OFF), `report-rate-*`/`timeout-*`
per power state. Resolution: `x-/y-resolution`.

Driver PM does NOT use Zephyr PM_DEVICE — it subscribes to ZMK activity
events (`tps43_idle_sleeper.c`); suspend behavior rides
`enable-power-management`.

## Known driver weaknesses (accepted going in)

- No palm rejection (PALM_DETECT bit defined, never read).
- Scroll is dominant-axis only (no diagonal two-axis scroll).
- Thin runtime error recovery: I2C read failure = log + skip; no re-init or
  bus recovery. Upstream dev log records intermittent nrfx TWIM -EIO tied
  to sleep/wake ("fixed", but watch for it).
- Machine-translated codebase (RU→EN), minor doc/code mismatches (SUSPEND
  bit comment vs code; Kconfig endif label).

## Multitouch gesture surface (what we get)

| Gesture | Emitted as | Note |
|---|---|---|
| Move | REL_X/REL_Y | on-chip filtered |
| Single tap | BTN_0 (left click) | |
| Two-finger tap | BTN_1 (right click) | |
| Press & hold | BTN_0 held (drag) | |
| Two-finger scroll | REL_WHEEL or REL_HWHEEL | dominant axis only |
| Pinch zoom | REL_MISC delta | needs mapping (see beekeeb/zmk-input-zoom) |
| Swipes 1-/3-finger | BTN_WEST/EAST/NORTH/SOUTH | need input-processor/behavior mapping to do anything |

beekeeb companion modules worth evaluating in pass 2: `zmk-input-zoom`
(REL_MISC→keypress) and `zmk-input-inertia` (inertial scroll; notes say
Zephyr 4.1-compatible — check against our base before adopting).

## Trial protocol (like-it-or-revert)

Evaluate over ~2 weeks before making the door one-way:
1. Stability under sustained cursor use — the capture rig
   (C:\Temp\toucan-logger-watchdog.ps1 → toucan-console.log) works
   unchanged on the azoteq build; watch for sysworkq wedges/0x08 drops.
2. Latency/feel over split BLE vs cirque (no community data exists —
   we're the measurement).
3. Gesture value vs accidental-trigger rate (esp. two-finger tap and
   palm contact, since there's no palm rejection).
4. Battery: right-half drain vs cirque baseline (~90%/day pattern).
5. Wake-from-idle lag if `idle-sleep`/PM enabled.

Revert = reflash archived cirque uf2 + reassemble bagged hardware.

## Blocked-on-beekeeb (re-check ~late July 2026, with the CLAUDE.md check)

- Converter-board pinout: which XIAO pins are SDA/SCL/RDY/RST on the kit.
- The promised conversion video/guide (docs.beekeeb.com).
- A toucan2/azoteq branch or overlay in beekeeb/zmk-keyboard-toucan (their
  real-world DT values: orientation flags, filter/threshold tuning, rates).
- The new display options (WPM graph / toucan icon) land in the same
  window — see CLAUDE.md watch item.
