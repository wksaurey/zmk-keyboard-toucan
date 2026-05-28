# Toucan hardware debug

Procedures for diagnosing physical / electrical faults on the keyboard.
Different from `TESTING.md` (which covers firmware behavior).

## Dead-key diagnosis with a multimeter

Use this when a single key is unresponsive while the rest of the
keyboard works. The fault is almost always one of: the switch itself,
the diode at that key, or a cold solder joint somewhere along the
column/row path between the MCU and the switch.

### Background — what the diode does

Toucan uses a **col2row** matrix
(`boards/shields/toucan/toucan.dtsi:16` — `diode-direction = "col2row"`).
Firmware drives one column GPIO high at a time and reads all rows. If
a switch on that column is pressed, current flows
**column → switch → diode → row**. Without diodes, pressing three
keys in an L-shape would create a back-path through the third key and
register a phantom fourth. The diode at every switch blocks the
back-path by allowing current only in the forward direction.

For col2row, the diode is oriented:
- **Anode → column side** (where current enters)
- **Cathode → row side** (where current exits)

The **cathode is the end with the stripe / band** on the diode body
(matches the line in the schematic symbol `▷|`). On the PCB the band
should point toward the row trace.

### Multimeter modes

- **Diode mode** — dial position marked with `▷|`. Outputs a small
  current and reads the forward voltage drop.
  - Healthy silicon diode: ~0.5–0.7 V in forward direction, `OL`
    (open) in reverse.
- **Continuity mode** — usually marked with a speaker / sound-wave
  icon. Beeps when resistance is near zero. Good for confirming
  switch closures and trace integrity.
- Red probe = positive (current source out of the meter). Black
  probe = ground.

### Step 1 — Test the diode in isolation

Find the diode next to the dead key. SMD diodes are small black
rectangles with a tiny white stripe at one end.

1. Multimeter → **diode mode**.
2. Touch one probe to each end of the diode.
3. Reading interpretation:
   - **~0.5–0.7 V** → forward direction. The end touching the **red
     probe is the anode**; the **black probe is on the cathode**.
   - **OL / no reading** → reverse direction. Swap probes and you
     should now see ~0.5–0.7 V.
   - **OL in both directions** → diode is open. Either dead, or a cold
     joint on one leg. Reflow first; replace if reflow doesn't help.
   - **Low voltage or beep in both directions** → diode is shorted.
     Replace.
4. Confirm the **cathode end (black probe when forward)** lines up
   with the **printed stripe** on the diode body. If they don't match,
   the diode is mounted backwards. Desolder, flip, resolder.

### Step 2 — Test the switch

1. Multimeter → **continuity mode**.
2. Probe across the two switch contact pads (the two pads going
   directly into the switch body — **not** the diode pads).
3. Press the switch.
   - **Pressed → beep / ~0 Ω.**
   - **Released → silent / OL.**
4. No beep when pressed → bad switch or cold joint on a switch pad.
   Reflow the switch pads, retest. If still dead, swap with a
   known-good switch.

### Step 3 — Test the full MCU-to-MCU path through the closed switch

If the diode and switch both pass in isolation, the fault is somewhere
between the MCU pads and the switch — a hairline trace crack, cold
joint at the MCU pad, or cold joint at the diode/switch pad.

1. Multimeter → **diode mode**.
2. Identify the column and row GPIO pads on the XIAO MCU for the dead
   key. Mapping (right half, after `col-offset = 6`):
   - Right-shield columns 0–5 = `gpio1_12 / gpio1_11 / gpio0_5 /
     gpio0_10 / gpio0_4 / gpio0_9`
   - Rows 0–3 = `gpio0_19 / gpio0_28 / gpio1_1 / gpio0_29`
   - Example: J is keymap (row 1, col 7). col_7 - 6 = right-shield
     col 1 = `gpio1_11`. Row 1 = `gpio0_28`. So J's MCU path is
     **P1.11 ↔ P0.28**.
3. Red probe → column GPIO pad on the XIAO. Black probe → row GPIO
   pad. Multimeter still in diode mode.
4. **Press the dead key.**
   - **Reads ~0.5–0.7 V** → entire chain conducts. Fault must be
     intermittent — try wiggling / flexing the PCB while probing.
   - **Reads OL** → there's a break in the path. Move to step 4.

### Step 4 — Narrow down the break

Probe pair-wise along the path. Each probe pair should show
continuity (beep) in continuity mode, except the diode itself which
shows forward voltage in diode mode.

Sequence (column → row direction):
1. **XIAO column pad ↔ diode anode pad** — should beep.
2. **Diode anode ↔ cathode** — in diode mode with switch pressed,
   ~0.5–0.7 V; OL with switch released (or always 0.5–0.7 V depending
   on whether the diode is in-circuit and the path is otherwise broken
   downstream; the cleaner test is when the switch is pressed
   completing the circuit).
3. **Diode cathode pad ↔ one switch pad** — should beep.
4. **Switch pad ↔ other switch pad, switch pressed** — should beep.
5. **Other switch pad ↔ XIAO row pad** — should beep.

The pair that doesn't beep contains the broken segment. Reflow the
joints at both ends; if a trace itself is fractured, jumper a fine
wire from pad to pad.

### Common failure modes

- **Cold / insufficient joint** at the diode anode (column side) or
  cathode (row side) — most common single-key failure. Solder looks
  dull, grainy, or pulled back from the pad. A reflow with a touch of
  fresh flux usually fixes it.
- **Cold joint at a switch pad** — second most common.
- **Reversed diode** — would manifest from first power-on; if the key
  used to work and just died, this is unlikely. But confirm
  orientation if the PCB was hand-soldered.
- **Cracked trace** — rare without obvious physical trauma. Look for
  pinches near the case mounting points.

## Right-half lockup — pending diagnostics

The right half occasionally hangs during continuous trackpad use (whole
half: keys + trackpad stop, requires power-cycle to recover). Prior
mitigations in `24da3be` (cirque SHA pin, `CONFIG_ZMK_IDLE_TIMEOUT=0`,
stack bumps to 4096) did not eliminate it — those remain in place
permanently. Next failure observed 2026-05-27 12:53:00 during cursor
use, battery ~50% (brown-out ruled out). Diagnostics plan below; the
config-side bits live in `boards/shields/toucan/toucan_right.conf`,
and the console plumbing is wired via a snippet in `build.yaml`.

**Diagnose first, then aim.** The watchdog approach in an earlier
draft of this plan (`CONFIG_WDT=y`, `CONFIG_ZMK_BEHAVIORS_WATCHDOG=y`)
turned out to be wrong on two counts: the parent symbol is
`WATCHDOG`, not `WDT`; and `ZMK_BEHAVIORS_WATCHDOG` doesn't exist in
this ZMK version. Even with the symbol names fixed, the watchdog is
*recovery*, not *diagnosis* — and ZMK doesn't ship a WDT kicker, so
enabling the driver alone is a no-op. Recovery decisions wait until
we know what's actually failing.

### 1. Stack overflow detection — rule in/out stack as cause

If the cirque driver hot path overflows the stack, we get a kernel
panic with a traceback instead of a silent deadlock.

```
CONFIG_STACK_SENTINEL=y
CONFIG_THREAD_STACK_INFO=y
```

Only useful if the cause IS stack overflow. With stacks already at
4096 this is *less* likely than before — adding it cleanly rules it
in or out. **Useless without #2** (a console to read the panic);
without it, a sentinel-triggered halt looks identical to a hang.

(Not adding `CONFIG_THREAD_ANALYZER`: it only emits output when
something calls `thread_analyzer_run()` — ZMK doesn't, and we have no
`CONFIG_THREAD_ANALYZER_AUTO` interval set. It's compiled-in
dead-weight without one of those.)

### 2. USB CDC console — read what the right half is doing

Console output over USB lets us plug the right half in after a
failure and read the last log lines / panic traceback. Wired via the
ZMK `zmk-usb-logging` snippet on the right-half entry in `build.yaml`:

```yaml
- board: seeeduino_xiao_ble
  shield: toucan_right rgbled_adapter
  snippet: zmk-usb-logging
```

The snippet pulls in both the Kconfig selects (`CONFIG_ZMK_USB_LOGGING`
→ `USB_DEVICE_STACK`, `USB_CDC_ACM`, `UART_CONSOLE`, `USB_UART_CONSOLE`,
`UART_LINE_CTRL`, `LOG`, etc.) **and** the devicetree `chosen`
redirect that points `zephyr,console` at the CDC ACM uart. Adding
the raw Kconfig symbols by hand without the DTS override produces an
enumerated-but-mute port — see the dead-end documented in commit
`e80a432`.

Cost: active USB device stack on the right half full-time (small
power overhead, mild risk of BLE/USB stack interaction). Worth it
because it's the only way to distinguish three very different
failure modes that all *look* identical from the host's perspective:

- **Firmware crashed** (stack overflow, mutex deadlock, ISR storm) —
  CDC console either doesn't enumerate, or prints a panic on connect.
- **Firmware alive, BLE link dropped** — CDC console enumerates and
  prints normally. Diagnostic attention moves to the radio side
  (BLE state machine, peripheral-central pairing); the firmware
  isn't the problem.
- **SPI bus to cirque is stuck** — CDC enumerates but logs show the
  cirque driver thread last-alive timestamp frozen. (Cirque on
  Toucan is on `spi0` per `toucan_right.overlay`, not I²C; the XIAO
  I²C controller is disabled on this shield.)

**Heisenbug caveat:** always-on USB changes scheduler load, ISR
cadence, and idle/sleep behavior. If the lockup *stops* reproducing
under diag firmware, that's itself a data point — suspect a sleep
or timing-sensitive path and consider keeping the diag build as a
separate `build.yaml` entry so you can A/B-flip.

### 3. Recovery (deferred — needs code, not config)

Once #1+#2 tell us what's failing, recovery becomes a real decision:

- **If firmware crashes:** add a hardware watchdog. Needs *both*
  `CONFIG_WATCHDOG=y` + `CONFIG_WDT_NRFX=y` *and* a kicker. Two
  paths: a custom thread (~20 LOC: register a channel, `wdt_feed()`
  on a kernel timer) or `CONFIG_TASK_WDT` + `CONFIG_TASK_WDT_HW_FALLBACK`
  with per-thread channels. Recommended default: TASK_WDT — it
  integrates cleanly with Zephyr's existing thread layer instead of
  reinventing the kicker. The single-thread custom kicker is the
  fallback if the trace points at a kernel-level hang rather than
  one specific thread.
- **If BLE link drops:** watchdog is irrelevant; pursue radio-side
  fixes.
- **If SPI stuck:** consider a bus-recovery sequence on driver-side
  timeout. Upstream against `geeksville/cirque-input-module`.

### Suggested order

1. Apply #1 + the `zmk-usb-logging` snippet to the right-half build
   entry. Flash right half. Live with the bug for a few days.
2. On the next failure: plug the right half into USB. Open a serial
   monitor (PuTTY / `picocom`) on the CDC port. Capture whatever
   prints — panic traceback, last log lines, or nothing.
3. Based on what we see, pick the right recovery path from #3 above.
4. If still unsolved after that: file upstream against
   `geeksville/cirque-input-module` with the trace + reproduction
   pattern.

## Charging fault diagnosis

Use this when USB-C is plugged in but the on-screen battery percentage
never moves, the CHG LED on the XIAO never lights, and (typically)
**both halves** show the same behavior.

### Step −1 — Verify power switches are ON (do this first, always)

The Toucan slide switch is in series with **both** the load *and* the
charger. If a switch is OFF, USB-C still powers the board through
VBUS (the keyboard appears to work), but the charger never sees the
battery — no CH LED, no charging, no % climb. This produces symptoms
**identical** to a cold BAT+ joint and is the more common cause.

beekeeb's assembly guide states this explicitly: *"Toggle the keyboard
switches to 'on' is required for charging."*

Procedure:

1. On each half, confirm the slide power switch is in the **ON**
   position (typically toward the USB-C end of the case — verify
   against your specific build, the orientation isn't universal).
2. Plug in USB-C.
3. Within 1–2 seconds, the **CH LED** (green, labeled `CH` on the
   XIAO top side, near the USB-C connector) should light. It's dim
   compared to the USR LED — that's normal.
4. If the CH LED lights on both halves, you're done. Skip the rest
   of this section. The original "not charging" symptom was just an
   OFF switch.
5. If a switch is already ON and the CH LED still doesn't light on
   that half, continue to Step 0 below.

**This sanity check exists because a prior session (2026-05-14)
skipped it, fixated on a cold-joint theory, and very nearly had the
user reflow both halves to fix a problem that was actually two
flipped switches. The 2026-05-15 retro entry documents the lesson.**

### Background — how Toucan charging works

Toucan uses the **Seeed XIAO nRF52840 Plus**. The charge chip
(BQ25101) lives on the XIAO module itself and is wired to:

- **VBUS:** the USB-C 5 V rail. Drives the charger when USB is
  plugged in.
- **BAT pin / BAT+ pad:** the dedicated battery connection on the
  back of the XIAO, opposite the USB-C end. The Toucan PCB has a
  matching pad/via on its **bottom** side that you solder through to
  bridge the XIAO's BAT+ to the PCB's battery line.
- **CHG LED:** a small green LED on the top of the XIAO, labeled
  **`CH`** on the silkscreen (right next to the **`USR`** LED). The
  BQ25101 drives it low (lit) while charging, high-Z (off) when
  complete or when no valid cell is detected. The CH LED is much
  dimmer than the USR LED by design — that's the current-limiting
  resistor, not a fault. Steady-dim is fine; flickering would be
  the cold-joint warning sign.

The PCB-side battery line then runs: **JST/Molex connector →
slide power switch → BAT+ pad under the XIAO**. The switch is in
series, so OFF disconnects the cell from both the load and the
charger.

beekeeb's soldering guide specifically calls out the BAT+ pad as a
common failure point — *"You might need to use more flux than you
thought."* The pad sits between the XIAO and the PCB, which makes it
hard to heat through and easy to leave as a cold joint.

The PCB also has an **ALT battery-connector footprint** for builds
that swap trackpad/display sides. The default Toucan build (display
left, trackpad right) must use the non-ALT footprint. If the JST was
soldered to ALT on a default build, the battery is on an unrouted
trace and nothing works.

### Symptoms that point to BAT+ cold joint

- Board runs on battery (cold joints can still pass low-current load
  through high resistance).
- USB-C plugged in powers the board (independent VBUS path) but no
  CHG LED, ever.
- Displayed battery % is well under 100% and doesn't move after an
  hour+ of charging.
- Both halves behave identically (same assembly step was done the
  same way on both).
- The original soldering used "lots of solder" with limited flux.
  Big solder ball + insufficient heat = textbook cold joint.

### Step 0 — Safety prep

1. Slide power switch to **OFF**.
2. Unplug USB-C.
3. Disconnect the battery from its JST/Molex housing (gently, by the
   housing, never by the wires).

Probe a de-energized board. A shorted probe across a live LiPo can
vent or ignite the cell.

### Step 1 — Visual inspection of the BAT+ joint

Flip the half so the **bottom of the PCB** faces you. Locate the
solder joint aligned with the back-side BAT+ pad of the XIAO
(opposite the USB-C end of the controller).

- **Good joint:** smooth, shiny, concave fillet, fully wetted to
  both pads. Mirror-like surface.
- **Cold joint:** dull, grainy, ball-shaped, or pulled back from one
  of the two pads. Solder sitting on top of the via without flowing
  in. "Lots of solder" with a rounded blob shape is a strong tell —
  heat flows into the work *through* the solder, so a big blob with
  insufficient iron temp insulates the joint while looking full.
- **Open joint:** visible gap, or solder is only attached to one
  side.

If it looks suspect on visual alone, jump to Step 3 (reflow).
Otherwise continue to Step 2 to confirm electrically.

### Step 2 — Continuity test along the battery path

Multimeter → **continuity mode** (speaker icon).

The path you're testing is: **JST `+` pin → top of switch → bottom
of switch → BAT+ joint at the XIAO**.

1. Slide power switch to **ON** (the switch is in series with the
   path you're testing; OFF would break the path itself).
2. Probe 1 on the JST/Molex connector's `+` (red wire) pin on the
   PCB. Easiest from the top side, on the metal contact inside the
   housing.
3. Probe 2 on the BAT+ joint on the bottom of the PCB.

Interpretation:
- **Beep / ~0 Ω** → full path is electrically continuous. Cold joint
  unlikely; rerun after a reflow as a sanity check, or move to "if
  this all passes" below.
- **No beep / OL** → there's an open somewhere along the path. Most
  likely the BAT+ joint. Reflow it (Step 3).
- **Intermittent beep when you wiggle the probe** → textbook cold
  joint. Reflow even if it sometimes beeps — probe pressure can
  bridge a gap that real load can't.

If the full-path test fails, isolate which segment:
- JST `+` pin ↔ top side of power switch — should beep.
- Bottom side of power switch ↔ BAT+ joint — should beep with switch
  ON, OL with switch OFF (this also tells you the switch is working).

The segment that doesn't beep contains the open joint.

### Step 3 — Reflow the BAT+ joint

Technique matters more than solder quantity:

1. Battery disconnected (Step 0).
2. Add a small dot of **fresh no-clean flux** directly on the
   existing joint. Flux is what makes the solder wet both surfaces;
   the original joint almost certainly failed for lack of flux.
3. Set the iron hot: **350–370 °C / 660–700 °F**. Cool iron is the
   #1 cause of cold joints on this pad.
4. Touch the iron tip to **both** the PCB pad and the XIAO's pad
   simultaneously — slide the tip into the via at an angle so it
   contacts both edges. A larger tip (chisel / D-shape) carries more
   heat than a fine cone.
5. Feed a small amount of fresh solder *into the iron tip where it
   meets the joint*, not onto the tip alone. Hold for 1–2 seconds
   after the solder visibly flows and the joint goes shiny.
6. Remove the solder first, then the iron. Don't move the joint
   until it solidifies (1–2 seconds).
7. Clean residue with isopropyl alcohol + a soft brush.

Result should be a smooth, shiny fillet, not a big silver ball. If
it still looks like a ball, more flux + more heat + longer dwell.

### Step 4 — Confirm charging works

1. Reconnect battery to JST. Slide power switch ON.
2. Plug in USB-C.
3. **CH LED** on the XIAO (green, labeled `CH`, near USB-C, top
   side) should light within a second or two. It's dim — that's
   normal.
4. **DC volts** across the battery wires (red to JST `+`, black to
   JST `-`): start at the cell's resting voltage (~3.7–3.9 V for a
   half-empty cell), should climb visibly over ~10–30 minutes. CHG
   LED turns off when the cell reaches ~4.2 V (charge complete).

Repeat Steps 0–4 on the other half.

### If Step 2 passes but charging still doesn't work after reflow

Less common, but worth checking:

- **BAT- / GND return joint** — same kind of cold-joint failure on
  the negative side. Run the same continuity test against GND
  instead of BAT+.
- **VBUS not reaching the charger:** measure DC volts between the
  XIAO's `5V` pin and `GND` with USB plugged in. Should be 4.7–5.1
  V. If lower, suspect cable or USB source even though others were
  tried.
- **ALT footprint mistake:** confirm the JST is soldered to the
  non-ALT footprint (silkscreen). ALT is only for swapped-side
  builds with custom firmware.
- **Charge-current select pin (P0.13):** drives the BQ25101's
  current setting (50 mA vs 100 mA). Default is fine; only worth
  checking if you've modified firmware that touches GPIO13.
- **Dead BQ25101 on the XIAO:** unlikely on both halves at once.
  Swap the XIAO with the `settings_reset` build target XIAO if you
  have one, as a known-good control.

### Common failure modes

- **Cold joint at BAT+ on the back of the XIAO** — by far the most
  common Toucan charging failure. Big-blob + low-heat soldering
  technique is the usual cause. Reflow with extra flux fixes it.
- **JST soldered to ALT footprint on a default build** — battery
  isn't actually on the charger trace.
- **Bad USB-C cable / sketchy port** — VBUS under ~4.5 V can prevent
  the BQ25101 from entering charge mode. Rule out with a wall
  adapter + known-good cable.
- **Cell at 100%** — CHG LED off and % not climbing is the *correct*
  behavior at full charge. Only a concern when % is well below 100.
