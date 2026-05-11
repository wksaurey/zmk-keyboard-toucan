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
