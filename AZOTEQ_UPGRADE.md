# Azoteq TPS43 upgrade — planning doc

> **STATUS 2026-08-19 — kit ARRIVED, integration IMPLEMENTED (branch
> `kolter/azoteq-tps43`), not yet flashed.** beekeeb published
> `beekeeb/zmk-keyboard-toucan2` on 2026-07-28, answering every item in
> the blocked-on-beekeeb list (see that section for the answers). What
> was built here, and where the plan below was overtaken:
>
> - New `toucan_right_azoteq` shield variant alongside the cirque one;
>   one left firmware serves both (the azoteq pad feeds the same
>   `&glidepoint_split` endpoint). **Build matrix trimmed 2026-08-20
>   (Kolter): CI builds only left + azoteq right + settings_reset.**
>   The cirque SHIELD stays in the repo and buildable — revert uses the
>   archived uf2s in `firmware-archive/` (always the real revert path);
>   a cirque rebuild = re-add one build.yaml entry (template in its
>   comment).
> - Driver is **VENDORED into this repo** (`drivers/`, bindings in
>   `dts/bindings/`) at geeksville master `66d51024`, with the K_NO_WAIT
>   patch applied — decided 2026-08-19 over the fork-and-pin plan (gh on
>   this host can't create personal repos; vendoring matches the
>   nice_view_gem precedent). Provenance + update rules:
>   `drivers/README.md`. No west.yml change was needed.
> - ZMK stays on our pinned v0.3 — confirmed correct: beekeeb's toucan2
>   itself builds on v0.3, not main.
> - Gestures: tap / two-finger tap / press-and-hold / scroll from day 1;
>   **pinch-zoom added 2026-08-20** (first trial-day feedback) via
>   zmk-input-zoom (west-pinned) + zip_zoom_mapper on the central,
>   Windows Ctrl bindings. Same day: cursor retuned to ~beekeeb feel via
>   on-chip x/y-resolution 910x796 (the only azoteq-only knob without a
>   truncation dead zone — driver sensitivity <100 and v0.3
>   peripheral-side split processors both stateless-truncate).
>   Still deferred: three-finger swipes + the touch-hold mouse layer
>   (beekeeb's mappings live in their toucan2 `toucan.dtsi`; their MOU
>   layer sits at index 4 = our GAME, so re-index when porting).
>   Momentum/inertial scroll: NOT in this setup; beekeeb's
>   zmk-input-inertia exists but self-describes as Zephyr 4.1 — needs a
>   compat check against our v0.3/Zephyr 3.5 base before adopting.
> - Known-good cirque uf2 set archived (md5-verified) at
>   `firmware-archive/2026-07-04-cirque-lockupfix/` — step 5 done.
> - Trial protocol below is unchanged and is the next step after CI
>   builds green and the hardware swap happens.
>
> **Open decisions from the 2026-08-19 review** (in-context + fresh review,
> no correctness bugs; three free fixes landed same day):
> - **SYM-layer two-finger scroll mismatch:** the scroller child (layers
>   <2>) bypasses the base 1/20 wheel scaler (ZMK applies the first
>   matching child INSTEAD of the base list), so azoteq wheel on SYM is
>   ~3x fast + direction-flipped. Cirque /7 and azoteq /20 genuinely
>   disagree — retuning the child changes cirque feel too. Documented in
>   TESTING.md for now; decide after the trial.
> - **RDY-race upstream defect (tps43.c ~1404 vs 1420):** the RDY GPIO
>   interrupt is armed before k_work_init/k_sem_init — a microsecond
>   boot-time window for a NULL-handler hard fault. 2-line fix, but a
>   SECOND local divergence from upstream; candidate for an upstream PR
>   instead. Decide before calling the vendored copy stable.
> - **K_NO_WAIT drop observability:** input_report_* returns are
>   discarded, so a dropped press/release (the documented tradeoff) is
>   invisible in the USB capture — "stuck button" and "split link died"
>   look identical. A rate-limited LOG_WRN on nonzero return would
>   disambiguate; same second-divergence tradeoff as above.
> - **Runtime display-style toggle (Kolter ask, 2026-08-20):** the
>   toucan2 screen styles are compile-time (`#if` in screen.c selects
>   which widget code exists), so a key shortcut can't switch them
>   today. Interim: CI builds BOTH left images (style 2 default +
>   `toucan_left_screen1_toucan_icon`); swap = reflash. Real feature =
>   fork the vendored display: compile all styles, runtime state var +
>   redraw, custom `zmk,behavior-*` to cycle, optional NVS persistence —
>   a real C project (~few hundred lines + on-device debug) and a heavy
>   divergence from beekeeb's copy; also an upstream-PR candidate.
>   Decide after living with both styles.
> - **Trial-day-1/2 gesture + feel iterations (2026-08-20):** resolution
>   910x796 → 730x640 → 620x544 → 515x450 (quantization floor reached —
>   next cut = scaler consolidation, see overlay comment); zoom added
>   (Ctrl+± keypresses; Ctrl+wheel macro attempt failed — &msc is
>   timer-driven, see toucan.dtsi comment); velocity acceleration added
>   (pointer_accel, vendored after upstream module broke CI); MAV filter
>   off (cured steady-state float, no jitter); LP ladder tuned
>   (timeout-lp1=5, lp2 320ms) for the after-sit wake lag.
> - **WATCH: transient float under sustained fast flicking** (v10,
>   2026-08-20, one occurrence, ~a minute, self-recovered). Candidates:
>   split-link backlog draining late vs on-chip reATI recalibration
>   after heavy rubbing. If reproducible, run the instrumented round:
>   K_NO_WAIT drop-counter patch + the USB capture rig — drops logged
>   during the float = link congestion; silence = on-chip.

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

## Blocked-on-beekeeb — RESOLVED 2026-08-19 (answers from beekeeb/zmk-keyboard-toucan2, published 2026-07-28)

- **Converter-board pinout:** the kit reuses the cirque's old SPI pins for
  I2C — SDA=P1.13, SCL=P1.15, RST=P1.14 (`&gpio1 14`), RDY=P0.02
  (`&gpio0 2`), trackpad at `0x74`, I2C fast mode, pinctrl pull-ups.
- **Real-world DT values:** `switch-xy` + `invert-scroll-y`,
  `scroll-angle 30` ("default ~47° too big on toucan2"),
  `filter-settings 0x0B`, `hold-time 500`, sensitivity 100/100,
  `report-rate-lp2 640` (55 µA vs 174 µA), power management on,
  `idle-sleep` commented out as "not recommended" (the 300 ms wake lag).
  All adopted in `toucan_right_azoteq.overlay`.
- **Their module pin:** `beekeeb/zmk_driver_azoteq` at `revision: main` —
  a bare branch (the exact thing this doc said to avoid). Their `main`
  (c329f309) is content-identical to geeksville master (66d51024, only
  the merge commit differs); their default branch `master` is the stale
  one. Moot for us: driver vendored at that content.
- **Driver movement since the 2026-07-04 baseline:** `three_finger_swipe`
  gesture + `three-finger-swipe-throttle-ms` (beekeeb PR #2, merged
  upstream 2026-07-31) and per-board build gating
  (`DT_HAS_AZOTEQ_TPS43_ENABLED` — the driver can't leak into cirque/left
  builds). **K_FOREVER is still unpatched upstream** — our vendored copy
  carries the K_NO_WAIT fix.
- **Display options:** landed in the same toucan2 repo —
  `boards/shields/nice_view_gem/` there has 10 new widget files (chart /
  WPM graph, layer_logo toucan icon, arc + vertical battery variants,
  `CONFIG_TOUCAN_STATUS_SCREEN=0/1/2`). Separate change; see the CLAUDE.md
  watch item.
- Conversion video/guide (docs.beekeeb.com): not re-checked — the repo
  answered the firmware side; check when doing the physical swap.
