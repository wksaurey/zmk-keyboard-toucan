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

## Right-half lockup — firmware confirmed alive; cause is the split BLE link

The right half occasionally hangs during continuous trackpad use (whole
half: keys + trackpad stop, requires power-cycle to recover). Prior
mitigations in `24da3be` (cirque SHA pin, `CONFIG_ZMK_IDLE_TIMEOUT=0`,
stack bumps to 4096) did not eliminate it — those remain in place
permanently. Next failure observed 2026-05-27 12:53:00 during cursor
use, battery ~50% (brown-out ruled out). Diagnostics plan below; the
config-side bits live in `boards/shields/toucan/toucan_right.conf`,
and the console plumbing is wired via a snippet in `build.yaml`.

> **STATUS 2026-06-04 — caught it live; firmware is NOT the problem.**
> Captured the lockup over the USB CDC console while it was happening:
> the thread-analyzer dump kept advancing for 4+ minutes during the hang
> (idle thread + others ticking up), with no panic and no `STACK_SENTINEL`
> trip. The right-half firmware core was **alive the whole time** — keys +
> trackpad were simply not reaching the host. That **confirms the "Firmware
> alive, BLE link dropped" branch** of §2 below and **rules out** firmware
> crash, stack overflow, and a cirque-SPI-only hang. The dead segment is the
> right↔left split BLE link; the hang did **not** self-recover without a
> power cycle. Open question — *why* the link drops (clean supervision
> timeout vs. a wedged BLE thread) — is what the follow-up logging (named
> threads + `BT_HCI_CORE`/`BT_CONN` debug logging, added to
> `toucan_right.conf` on 2026-06-04) is meant to answer on the next capture.

> **STATUS 2026-06-09 — the `c205b8c` diag build BRICKS the right half's
> boot. Do NOT flash it.** Flashing the main-branch CI artifact (run
> `27004601648`, the only build containing the `CONFIG_THREAD_NAME` +
> `BT_HCI_CORE`/`BT_CONN` debug flags) left the right half dead: USB
> enumeration fails ("device descriptor request failed" on every reset),
> no split link, appears off on battery — only the dim CH charge LED.
> The app crashes/hangs during early boot, almost certainly the BT host's
> debug-log flood at init; even per-module DBG was too much. Recovered via
> double-tap DFU (bootloader survives — pulsing red USR LED + `XIAO-SENSE`
> drive) and reflashing the known-good branch build.
>
> **FIXED 2026-06-16.** The two boot-breaking flags
> (`CONFIG_BT_HCI_CORE_LOG_LEVEL_DBG` + `CONFIG_BT_CONN_LOG_LEVEL_DBG`) were
> removed from `toucan_right.conf` — *not* via a full `git revert c205b8c`,
> which would also have dropped `CONFIG_THREAD_NAME`. `THREAD_NAME` is kept:
> it isn't the boot-breaker (small RAM, no behavior change) and it labels
> threads in the analyzer dump. Main CI artifacts boot the right half again.
>
> **The flags were also unnecessary.** ZMK v0.3's split peripheral
> (`app/src/split/bluetooth/peripheral.c`) already logs
> `Disconnected from <addr> (reason 0x%02x)` at `LOG_DBG` in the `zmk`
> log module — which the `zmk-usb-logging` snippet already enables
> (every capture shows `<dbg> zmk:` lines). The reason byte prints on the
> **current known-good firmware**; no rebuild needed. What the 2026-06-04
> and 2026-06-09 captures were missing was not logging but *attachment*:
> both connected after the drop, and the line had scrolled out of the 8 KB
> ringbuf. Capture plan: PuTTY pre-attached and logging from boot, then
> grep the log for `reason 0x`. (2026-06-09 capture re-confirmed
> firmware-alive: ~20 analyzer blocks advancing, no panic, uptime 00:14 at
> capture start.)

> **STATUS 2026-07-03 — GOT THE REASON BYTE: `0x08` connection supervision
> timeout.** Captured live with the console attached before the drop (lockup
> at 14:47:11 during continuous trackpad use, no host-switching involved).
> Full sequence from the right half's log:
>
> 1. `<wrn> bt_att: Not connected` — the peripheral was mid-send (cursor
>    traffic) when the ATT layer found the link already dead.
> 2. `Disconnected from ED:D4:95:87:0F:D1 (reason 0x08)` — supervision
>    timeout: the **central (left half) went silent on the radio**; nobody
>    deliberately disconnected (that would be 0x13/0x16).
> 3. **26 ms later: full automatic reconnect** — `Peripheral connected`,
>    security level 2 restored, both split CCCs re-subscribed, physical
>    layout re-selected via GATT write. The BLE link healed itself
>    essentially instantly.
> 4. Post-reconnect, the right half kept scanning and sending key events
>    normally (test presses visible in the log).
>
> Additional finding, an accident of USB power: the user's recovery
> "power-cycle" of the right half **did not actually reset it** — firmware
> uptime ran continuously through the whole episode, because **the slide
> switch only disconnects the battery; USB VBUS keeps the MCU running**.
> Function returned anyway. So the right half needed no reset at all,
> which — combined with the instant reconnect and reason 0x08 — shifts
> prime suspicion to the **left half (split central)**: it stops
> responding on the radio long enough to kill the link, its BT stack
> recovers the connection immediately, and whether right-half input
> reaches the host afterward is the open question (perceived deadness vs.
> log-confirmed sends is unresolved — see next steps).
>
> Capture rig (all on the Windows side, C:\Temp): `toucan-serial-logger.ps1`
> (auto-reconnecting CDC reader → timestamped `toucan-console.log`) managed
> by `toucan-logger-watchdog.ps1` (force-restarts the reader if the log goes
> stale while the port is enumerated — a blocked serial read once silently
> ate 40 min of stream, so the watchdog is not optional). Reader gotchas
> fixed along the way: .NET regex `.` doesn't match newlines (multi-line
> chunks jammed the line splitter), and `ReadExisting()` can block forever
> on a wedged/suspended usbser handle (gate on `BytesToRead`).
>
> (2026-07-03, later:) recovery that day turned out to be a bundle — right
> switch flip (no-op under USB power), LEFT half power-cycle (a real reset),
> and a host move — so which action restored function is confounded, and
> whether the auto-reconnect alone had already restored it is unknown.
>
> **NEXT-LOCKUP PROTOCOL (in order, touching nothing else):**
> 1. Hands off. Note the time. The disconnect line is already captured.
> 2. Wait ~15 s (give the auto-reconnect time), then test: move the
>    trackpad, type a few right-half keys somewhere visible. If input
>    works → the link self-heals and no reset is needed; prior right-half
>    power-cycle fixes were placebo. Done.
> 3. If still dead: power-cycle ONLY the left half. Re-test the right
>    half. If input returns → central-side wedge confirmed; instrument
>    the left half next.
> 4. Only if still dead: reset the right half (unplug its USB first —
>    the slide switch alone doesn't reset it while the cable is in;
>    double-tap reset also works).
> Report which step restored function.
>
> **STATUS 2026-07-04 — VERDICT: the LEFT half (split central) is the
> patient. Protocol ran to completion on a live lockup; right half is
> exonerated.** Full sequence, all from the right half's console:
>
> - Lockup struck during cursor use (~10:15). This time the split link
>   did NOT drop — no disconnect line, no bt_att errors. The right half
>   kept scanning and sending key events into a connection the left half
>   was accepting at the radio level and ignoring at the app level.
> - ~10:17, the failure CASCADED: the right half's kscan went completely
>   silent (10 test H-presses at ~10:22 produced zero scan events) while
>   its firmware stayed alive (analyzer + battery logging normal).
>   Interpretation: the peripheral's split-TX event queue jammed against
>   the zombie connection and back-pressured the scan pipeline.
> - User power-cycled ONLY the left half. The right half held the zombie
>   connection until supervision timeout (`reason 0x08`, 10:26:23) — the
>   same byte as 2026-07-03, now understood as "central went silent."
> - The teardown unjammed the peripheral: 26 ms later it reconnected to
>   the freshly-booted left half, and 9 s later the user's H-presses were
>   back in the scan log AND on screen. **Right half was never rebooted**
>   (uptime continuous through the entire episode).
>
> This also reinterprets 2026-07-03: that day's instant auto-reconnect
> went to a *still-wedged* central, so input stayed dead. Reconnection
> isn't the cure — a REBOOTED central is. The left half's BT host stack
> survives the wedge (reconnects + re-subscribes CCCs within ~1 s every
> time); what hangs is ZMK's split-central input processing above it.
> Trigger correlates with sustained pointer traffic (high event rate)
> in every observed instance; keys-only use has never triggered it.
>
> Recovery shortcut until fixed: flip the LEFT half's power switch.
> ~2 s outage, no bond loss, right half recovers on its own.
>
> Next steps: (a) research upstream — ZMK issues/PRs for split-central
> input-relay wedges under pointer load (we pin zmk v0.3 branch +
> cirque-input-module effec100); (b) instrument the LEFT half the same
> way as the right (zmk-usb-logging snippet + THREAD_NAME + analyzer on
> its build entry — the boot-brick flags were the BT DBG pair, NOT these;
> beware ZMK prefers USB for HID output when a cable is in, so add
> `&out` toggles to ADJ or unplug after capture) and catch the wedge
> from the central's side: which thread/queue stalls; (c) fix per
> findings — queue sizing, ZMK update/patch, or upstream report with
> the trace; (d) optional stopgap while root-causing: task watchdog on
> the central to auto-reset on input-thread stall. Superseded plan for
> reference: (was) if central-side wedge is confirmed,
> instrument the LEFT half (zmk-usb-logging snippet on its build entry —
> beware ZMK output routing prefers USB when a cable is attached; force
> `&out OUT_BLE` or unpair expectations accordingly); (c) correlate lockup
> frequency with heavy pointer traffic on the split link (matches every
> observed trigger to date).

> **RESEARCH 2026-07-04 — upstream already found it. Three research
> streams (beekeeb fork, cirque module + community, ZMK core source),
> all converging:**
>
> 1. **beekeeb PR #19 (MERGED upstream 2026-06-28) — the primary fix.**
>    Zephyr's input thread defaults to a 512 B stack; on the CENTRAL the
>    forwarded-trackpad fan-out (split input handler → listeners →
>    xy-scaler → mouse HID → scroll mapper) peaks at 536 B. Measured on
>    identical hardware (kalbasit): free space 88→0 in ~90 s of trackpad
>    use, corruption wedges the relay, eventually kills the split link
>    with BT_HCI_ERR_CONN_TIMEOUT — our `reason 0x08`, verbatim.
>    Fix: `CONFIG_INPUT_THREAD_STACK_SIZE` ≥2048 on the central. **Our
>    fork bumped this on the RIGHT half only (May diagnostics) — the
>    wrong half.** Explains a permanent wedge (corrupted memory doesn't
>    self-heal) and why the central's BT stack survives (separate threads).
> 2. **cirque-input-module PR #4 (`00a4a28`, OPEN, parent = our pin
>    `effec100`) — the peripheral-side fix.** The Pinnacle driver calls
>    `input_report_*(…, K_FOREVER)` on the sysworkq; when the split link
>    congests and the 16-deep input queue (`CONFIG_INPUT_QUEUE_MAX_MSGS`)
>    fills, the sysworkq blocks forever — kscan, battery, split TX all
>    stall, firmware alive, no panic. Matches our 2026-07-04 kscan-silence
>    cascade AND the original May right-half hangs. Fix: `K_NO_WAIT`
>    (drop under backpressure).
> 3. **ZMK v0.3 source facts (verified from the tree):** peripheral
>    pointer events bypass ZMK's queues entirely — `zmk_split_bt_report_input()`
>    → synchronous `bt_gatt_notify` on the input thread, no backpressure
>    valve (unchanged on main). Central relay funnels ALL peripheral
>    events through one 5-deep msgq (`ZMK_SPLIT_BLE_CENTRAL_POSITION_QUEUE_SIZE`,
>    silent drops). Below ZMK: Zephyr 3.5 BT host has known
>    notify-TX-credit exhaustion + buffer-leak-on-disconnect bugs
>    (zephyr#47649, #16803, #28248); ZMK main moved to Zephyr 4.1.
>    Secondary knobs if needed: `CONFIG_ZMK_SPLIT_BLE_CENTRAL_SPLIT_RUN_STACK_SIZE=4096`
>    (beekeeb PR #20, measurements disputed), central position-queue bump,
>    `BT_L2CAP_TX_BUF_COUNT`/`BT_ATT_TX_COUNT`, badjeff's
>    `zip_report_rate_limit` input processor to throttle at the source.
>
> **Fix plan:** (1) `CONFIG_INPUT_THREAD_STACK_SIZE=4096` +
> `CONFIG_ZMK_SPLIT_BLE_CENTRAL_SPLIT_RUN_STACK_SIZE=4096` + diag block
> (STACK_SENTINEL/analyzer/THREAD_NAME — currently right-half-only) in
> `boards/shields/toucan/toucan_left.conf`; (2) move cirque pin
> `effec100` → `00a4a28` in `config/west.yml`; (3) repair the dangling
> `config/toucan_{left,right}.conf` symlinks (point at nonexistent
> `toucan.conf`; contribute nothing to builds). Escalation if lockups
> persist: Zephyr BT buffer bumps above, then ZMK 0.4/Zephyr-4.1 rebase.
> Our capture record (0x08 + 26 ms reconnect + live-link variant +
> teardown-unjam) is an unusually complete repro — worth filing upstream
> if anything survives the fix. Related, not ours: beekeeb issue #23
> (`GPIO_PULL_UP` on the Cirque CS line kills the pad outright on some
> builds — our overlay carries the same line but our pad works).

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

(~~Not adding `CONFIG_THREAD_ANALYZER`: it only emits output when
something calls `thread_analyzer_run()` — ZMK doesn't, and we have no
`CONFIG_THREAD_ANALYZER_AUTO` interval set. It's compiled-in
dead-weight without one of those.~~ **Reversed:** `THREAD_ANALYZER` +
`THREAD_ANALYZER_AUTO` (5 s interval) + `THREAD_ANALYZER_USE_PRINTK`
*were* added — the periodic auto-dump is precisely the heartbeat that
proved the firmware stays alive during the hang. See `toucan_right.conf`.)

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
  *(2026-06-04: ruled out — see STATUS above.)*
- **Firmware alive, BLE link dropped** — CDC console enumerates and
  prints normally. Diagnostic attention moves to the radio side
  (BLE state machine, peripheral-central pairing); the firmware
  isn't the problem. **← CONFIRMED 2026-06-04.** The live tell is the
  thread-analyzer counters still advancing during the hang.
- **SPI bus to cirque is stuck** — CDC enumerates but logs show the
  cirque driver thread last-alive timestamp frozen. (Cirque on
  Toucan is on `spi0` per `toucan_right.overlay`, not I²C; the XIAO
  I²C controller is disabled on this shield.)
  *(2026-06-04: ruled out — keys died too, not just the trackpad, so
  the whole right→host path is down, not just the cirque bus.)*

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
2. On the next failure: follow the "Capturing logs after a lockup"
   procedure below.
3. Based on what the capture shows, pick the right recovery path
   from #3 above.
4. If still unsolved after that: file upstream against
   `geeksville/cirque-input-module` with the trace + reproduction
   pattern.

### Capturing logs after a lockup

When the right half locks up, the goal is to read the kernel log
(and panic dump, if any) over the USB CDC console wired by the
`zmk-usb-logging` snippet.

**Critical — do NOT power-cycle the right half** between the lockup
and the capture. The log buffer lives in RAM. Toggling the slide
switch off (or letting the battery die) wipes it.

**Better still — connect the console BEFORE it locks up.** Now that the
firmware is confirmed alive during the hang (STATUS above), the prize is
the *disconnect event itself* — ZMK's own
`Disconnected from <addr> (reason 0x..)` line, logged at `zmk` DBG level
by the split peripheral on the current firmware (no extra Kconfig
needed; the `BT_HCI_CORE`/`BT_CONN` debug flags turned out to brick boot
— see STATUS 2026-06-09). It only prints at the instant the link drops. On
2026-06-04 we connected *after* the lockup and the event had already
scrolled out of the small ringbuf — so we proved firmware-alive but
missed *why* the link dropped. Going forward, work/game with USB-C in the
right half and PuTTY already logging to a file, and let it run until the
hang reproduces. The post-lockup procedure below is the fallback for when
you weren't already connected.

#### Procedure

1. Lockup happens → note the time and what you were doing. Don't
   touch the right half.
2. Within ~2 minutes (see "Time constraints" below): plug USB-C
   into the right half. Slide switch stays ON.
3. Windows enumerates a new serial port. Device Manager →
   "Ports (COM & LPT)" → "USB Serial Device (COMx)". Note the COM
   number.
4. Open PuTTY (or any serial terminal) on COMx. Settings: 115200
   8N1, no flow control. CDC ACM is speed-independent so the baud
   is informational, but 115200 is the convention.
5. Whatever was queued in the ringbuf streams out when PuTTY opens
   the port and asserts DTR.
6. Save the capture (PuTTY: Session → Logging → "Printable output"
   → pick a file path *before* opening the connection).
7. Power-cycle the right half to recover.

#### Time constraints

The log buffer is small:

- `CONFIG_LOG_BUFFER_SIZE=8192` — formatted log strings
- `CONFIG_USB_CDC_ACM_RINGBUF_SIZE=1024` — CDC TX ringbuf to host

When the host isn't connected, the ringbuf fills and older entries
are dropped, not held.

- **Panic case (kernel halted):** the panic dump is the *last* thing
  emitted. Nothing displaces it after. Hours later is fine — even
  overnight, as long as power stays on.
- **Live case (firmware alive, BLE link dropped):** firmware keeps
  logging. Ring churns continuously. Plug in within ~2-5 minutes to
  catch disconnect-time messages before they scroll out.

You won't know up front which case you're in. Default to "plug in
within a couple minutes" and you cover both.

#### Interpreting the output

| What you see                                                          | Failure mode                                                                              |
| --------------------------------------------------------------------- | ----------------------------------------------------------------------------------------- |
| `E: ***** USAGE FAULT *****` / `>>> ZEPHYR FATAL ERROR 2`             | Stack overflow (FATAL ERROR 2). "Current thread" names the failing stack.                 |
| Other `>>> ZEPHYR FATAL ERROR N`                                      | Other kernel panic — hardfault, mutex deadlock detector, etc. Code identifies the class.  |
| Thread-analyzer counters still advancing during the hang, no panic    | **Firmware alive — confirmed case (2026-06-04, re-confirmed 2026-06-09).** The hang is the split BLE link, not the firmware. Look for ZMK's `<dbg> zmk: ... Disconnected from <addr> (reason 0x..)` line for the cause — present on current firmware, but only if the console was attached when the link dropped. |
| CDC enumerates but port is silent                                     | Firmware halted before logging, or HardFault took down output before flush.               |
| No enumeration at all                                                 | USB stack itself is dead. HardFault, MPU fault, or hardware-level issue.                  |

The interpretation maps directly onto the recovery options in
section #3 above.

### Capturing a Bluetooth HCI snoop log

Useful when the lockup might be on the BLE side (link supervision
timeout, controller wedge, host stack stall) rather than firmware-side.
`btmon` taps the kernel's HCI monitor socket and writes every HCI
packet to a `.btsnoop` file readable by Wireshark.

```
sudo btmon -w toucan-hang.btsnoop
```

Run it before reproducing the hang; Ctrl-C after the keyboard locks up.

**Watch the file size.** `btmon -w` writes unbounded — no built-in
size cap exists. A long capture on a busy adapter (multiple devices,
audio streaming) can grow into the GBs and contribute to OOM.
Mitigations:

- bound the run by time: `sudo timeout 1h btmon -w toucan-hang.btsnoop`
  (stops cleanly after 1 hour),
- check periodically: `watch -n 30 'ls -lh ~/toucan-hang.btsnoop'` and
  Ctrl-C the capture before it gets out of hand,
- start the capture only once you sense the hang is imminent,
- disconnect other BT devices first (especially A2DP audio sinks) to
  keep the log focused on the keyboard.

Do **not** commit `.btsnoop` captures to the repo — they're large and
contain raw traffic from every connected BT device.

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

## Flashing — bootloader enumerates but no `XIAO-BOOT` mount appears

Symptom: double-tap reset, the desktop shows a "device connected"
toast, `lsusb` lists the bootloader (`2886:0064 XIAO nRF52840 Plus`
for the Plus, or similar for the base XIAO), but no FAT volume mounts
and `lsblk` shows no new block device. Drag-and-drop flashing is
impossible until that volume appears.

Quick triage:

```
lsmod | grep usb_storage
```

- **Empty** → the kernel can't bind the bootloader's mass-storage
  interface. The bootloader is fine; Linux just has no driver.
- **Present** → the module loaded but the FS isn't auto-mounting.
  Find the device with `lsblk`, then
  `udisksctl mount -b /dev/sdX1` (or install `udisks2` if missing).

If `usb_storage` is empty, try `sudo modprobe usb_storage`. On an
**Arch / EndeavourOS host that has pending kernel updates**, expect:

```
modprobe: FATAL: Module usb_storage not found in directory
/lib/modules/<running-kernel>
```

That means `pacman` updated the `linux` package, wiped the old
kernel's `/lib/modules/<old>` tree, and the still-running old kernel
now has *no* dynamically-loadable modules at all (not just
`usb_storage`). Confirm with:

```
uname -r                   # running kernel
ls /lib/modules/           # installed module trees
pacman -Q linux            # installed linux package version
```

If `uname -r` doesn't match a directory under `/lib/modules/`, **reboot**.
That alone fixes the flash flow — `usb_storage` autoloads on the
bootloader's next enumeration and the `XIAO-BOOT` volume mounts.

Prevention: install `kernel-modules-hook` from the AUR. It preserves
the old kernel's module tree across `pacman` upgrades until you reboot,
so on-demand module loads keep working in the running session.
