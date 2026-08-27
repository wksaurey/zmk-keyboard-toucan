# Toucan firmware test plan

Walk through after reflashing both halves. Open a text editor and a
terminal (or `xev`) to dump raw keypresses for any key whose label is
ambiguous. Flag anything that doesn't behave as expected.

## Setup

- Both halves flashed with the latest firmware artifacts.
- Both halves paired to the host (default profile slot, or whichever you
  intend to test).
- The chosen profile slot connected — the display's profile area shows
  the active slot.

## BASE layer — typing

- Type a few sentences with normal letters, numbers (via shift on the
  top row), and punctuation. No phantom keys, no missed letters.
- Tap `'` repeatedly fast (don't, won't, it's). Should produce
  apostrophes, **not** phantom Ctrl. (`mt_tp` tap-preferred test.)
- Hold `'` ~250 ms, then press `c`. Should produce Ctrl+C.

## BASE — CAPS / CTRL home-row leftmost

- Tap once. Should act as Esc (OS-level caps→esc remap).
- Hold + press `c`. Should produce Ctrl+C.

## BASE — RSHFT and dual-shift caps (mod-morph)

Both outer pinky keys are `mod-morph` behaviors (not a combo): each sends
its shift immediately, but morphs to ESC if the *other* shift is already
held. ESC → CAPS LOCK at the host (OS-level esc/caps swap).

- Hold LSHFT, type `a` → `A`. Hold the bottom-right RSHFT, type `a` → `A`.
  Each shift must register on the **first** keypress, no delay.
- Hold LSHFT, then (still holding) press RSHFT → toggles CAPS LOCK.
  Then the reverse: hold RSHFT first, then press LSHFT → also toggles
  CAPS LOCK. Order-independent; whichever shift is pressed *second*
  produces the caps toggle.
- Hold LSHFT and shift-click / shift-drag with the mouse. The selection
  must extend on the **first** click — no missed/dropped shift. (This is
  the bug the mod-morph fixes: shift is never withheld, so a separate-USB
  mouse click can't be swallowed waiting on a combo window.)
- **Verify the morph emits a clean ESC:** when the second shift toggles
  caps, confirm CAPS LOCK actually flips at the host. If it doesn't, the
  first shift's modifier may be leaking through (host sees Shift+ESC, not
  ESC) and the ESC→Caps remap isn't firing — see the mod-morph note in
  `config/toucan.keymap`.

## BASE — cut / copy / paste combos

Bottom-row adjacent-key combos on the left hand. 50 ms window with
125 ms idle gate, so they only fire after a brief pause.

- Pause, then Z + X simultaneously → Ctrl+X (cut).
- Pause, then X + C simultaneously → Ctrl+C (copy).
- Pause, then C + V simultaneously → Ctrl+V (paste).
- While typing words like `vex` or `exact`, the combos must NOT fire.

## Thumbs

- Left-middle → Enter.
- Right-middle → Space.
- Left-outer (LALT) → plain Alt.
- Right-outer (RGUI) → Super/Cmd/Win.

## Layer keys — momentary (hold)

- Hold left-inner (NAV). Display shows "NAV". Release; back to "BASE".
  **Note any noticeable activation lag** — that's the tap-dance latency
  question.
- Same for right-inner (SYM).
- Hold both inner thumbs simultaneously → display shows "ADJ".

## Layer keys — toggle (double-tap)

- Quick double-tap left-inner. Display shows "NAV" and stays. Press
  the right-half numpad cluster → produces digits.
- Double-tap left-inner again → returns to BASE.
- Repeat with right-inner / SYM (test by typing `!` on top-row col 1).

## NAV — arrows on ESDF

- Hold NAV. E (top, middle finger) = up. S = left. D = down. F = right.

## NAV — numpad

Right-hand cluster — digits in their natural numpad positions, math
operators in the pinky columns (10 = pinky home, 11 = pinky reach),
`0` back on the right-outer thumb, `.` under the strongest aux finger:

```
            col6   col7  col8  col9  col10  col11
row 0:      MB1    7     8     9     ,      BSPC
row 1:      .      4     5     6     +      -
row 2:      =      1     2     3     *      /
right thumbs:                  trans  SPACE  0
```

- Hold NAV. Type 7,8,9 / 4,5,6 / 1,2,3 from the right hand. All
  produce digits regardless of host Num Lock state (bug fix carried
  over from `26d59a6` — `N0..N9` and `DOT` used instead of `KP_*`
  digit codes).
- Right-outer thumb → `0`.
- Home-row col 6 → `.` (regular DOT, NumLock-safe).
- Bottom-row col 6 → `=` (regular EQUAL, not KP_EQUAL — most hosts
  ignore the HID keypad-equal code).
- Top-row col 10 → `,` (comma — thousands separator).
- Math ops in pinky columns:
  - **Home row:** `+` (col 10, pinky home) / `-` (col 11, pinky reach)
  - **Bottom row:** `*` (col 10) / `/` (col 11)
- `MB1` (top col 6) — explicit left-click button for click-and-drag.
- Top-right (col 11) is `BSPC` on NAV — `DEL` only lives on SYM now,
  see the next section.

## NAV / SYM — trackpad clicks and drag

The `tap_to_rclick` input-processor was removed (commit `2924e69`)
because layer-conditional input-chain swapping was the leading
suspect for cursor lockups during fast layer-tap + drag. Right-click
is now exclusively via dedicated key bindings.

Current behavior:

- **Trackpad on BASE / NAV** — cursor mode. Slide moves the cursor.
  Tap fires MB1 (chip's BTN_TOUCH/BTN_0 default behavior).
- **Trackpad on SYM** — scroll mode. Slide scrolls the page (Y
  inverted: slide down → page down). Tap still fires MB1 but is
  rarely useful on SYM.
- **NAV / SYM top-row col 6 (Y position) → `&mkp MB2`.** Hold NAV or
  SYM and tap Y to right-click. Hold to keep MB2 down for right-drag
  with the trackpad.
- **NAV top-row col 5 (T position) → `&mkp MB1`** (commit `0fd0665`).
  Hold NAV + T, slide on the trackpad to drag. Works because NAV is
  no longer in the scroller's `layers` list — trackpad-on-NAV is
  cursor, not scroll, so motion + held MB1 = drag.

## SYM — Delete and Ctrl+Alt+Del

`DEL` now lives at top-row col 11 of **SYM only**. NAV top-right is
`BSPC` (numpad-context backspace) after the numpad redesign.

- Hold SYM, tap top-row col 11 → `Delete` (forward delete).
- Ctrl+Alt+Del: hold `LCTRL` (left pinky home-row), `LALT` (left outer
  thumb), `SYM` (right-inner thumb), then tap SYM top-row col 11
  (right pinky reach). Host should see Ctrl+Alt+Delete (Windows lock
  screen, Task Manager, etc.).

## NAV — Bluetooth + studio

- Hold NAV, press G (home-row col 5) → studio_unlock. With USB
  connected, ZMK Studio in a browser (https://zmk.studio) should
  become editable.
- Bottom row left: BT_SEL 0..4 on positions 25–29. Tap `BT_SEL 1` and
  confirm the display's profile area updates.
- `BT_CLR` is **no longer on NAV** (was position 24 / left-pinky
  bottom). Moved to ADJ at the same physical position so it can't be
  hit accidentally while holding LCTRL for Ctrl+arrow gestures.
- NAV pos 24 is now `&trans` → falls through to BASE `LSHFT`.

## ADJ — BT_CLR (relocated)

- Activate ADJ (hold both inner thumbs, or toggle one + hold the
  other), then press the bottom-row leftmost key (where `LSHFT` was on
  ADJ; same position as `BT_CLR` used to be on NAV).
- Tap once → clears bonded profile for the current BT slot. **Confirm
  intentional** — if you just did this and lost the host pairing, BT_CLR
  is exactly what cleared it. Re-pair on the host.

## SYM — special chars and right-thumb Enter

- Hold SYM. Top row: `!@#$%`. Right side top: `MB2  &*()` (col 6 is
  now right-click via `&mkp MB2`; `^` was displaced — still available
  via Shift+6 on BASE). Home row right: `-=[]\` and grave on
  rightmost. Bottom row right: `_+{}|~`.
- **Right-outer thumb on SYM = `Enter`** (replaces the BASE-layer
  `RGUI`). Lets you hit Enter one-handed on the right while the right
  hand is in trackpad territory — hold SYM with right-inner thumb,
  tap right-outer thumb, release SYM.

## ADJ — function and media

- Activate ADJ (hold both inner thumbs, or toggle one then hold the
  other).
- F-keys on the left hand, cols 1–4, reading order: F1–F4 top row,
  F5–F8 middle row, F9–F12 bottom row (all 12 — F11 regained when the
  block shifted left off col 5). Col 5 is `&trans` above and below
  `&tog GAME`.
- Vol-down / mute / vol-up on the right top row.
- Top-row col 10 (physical P position) → `&kp PSCRN` — Print Screen.
  Mnemonic: "P for print." Confirm it triggers the host screenshot
  action (Windows: clipboard capture; Linux: whatever the DE binds to
  PrtSc).
- Confirm TAB on top-row col 0 of ADJ now actually emits Tab. (Fixed
  in `52895bc` — was `&mo TAB` which is invalid.)
- ADJ middle-row col 5 (G position) → `&tog GAME` — toggles into the
  GAME layer. See GAME section below.

## GAME — FPS-style gaming layer

Added in `dff378a`, polished in `0c30f4c`, and reworked since for the
Z/X/C row, a bottom-row crouch, and an outer-pinky ESC. The alpha block
is shifted one column right of BASE, so the physical ESDF cluster
produces WASD movement. SHIFT (sprint) sits on the home row; CTRL
(crouch) is on the bottom-row pinky; ESC is on the outer home pinky.
All are plain `&kp` (not mod-taps — no hold-tap latency in-game).

This is a **left-half-only** layer: during play the split's right
half is physically detached for mouse room (see CLAUDE.md). Right-half
keys are `&trans`; the one live right-half key is the `&tog GAME` exit
on the inner right thumb.

### Enter / exit

- **Enter:** activate ADJ (hold both inner thumbs), tap middle-row
  col 5 (physical G) → display shows "GAME".
- **Exit:** re-dock the right half, tap the **inner right thumb**
  (`&tog GAME`, bound directly in GAME). Display returns to "BASE".
- The ADJ-G path **does not** exit GAME: layer 4 (GAME) outranks
  layer 3 (ADJ) and binds pos 17 to `&kp F`, so the exit lives on the
  inner thumb on purpose — and can't be hit mid-game because the right
  half is detached.

### Movement and mods

Rest your left hand one column right of WASD home (index/middle/ring
on F/D/S). Outputs below are what the host receives:

- Physical E (top, middle) → W (forward).
- Physical S (home, ring) → A (left).
- Physical D (home, middle) → S (back).
- Physical F (home, index) → D (right).
- Physical A (home, pinky) → LSHIFT (sprint).
- Physical Z (bottom, pinky) → LCTRL (crouch).
- Physical Ctrl/Caps key (home, outer pinky) → ESC.
- Left-outer thumb → LALT. Left-middle thumb → SPACE (jump).

### Direct + recovered game keys

The one-column shift means several physical keys send a different
keycode:

- Physical X → Z, C → X, V → C (the Z/X/C row, shifted one right).
- Physical Q → TAB (scoreboard/inventory).
- Physical W → Q.
- Physical R → E.
- Physical T → R (reload).
- Physical G → F (use/interact).
- Physical B → M.
- Physical TAB key (top, outer) → `` ` `` (GRAVE) — push-to-talk. Bound
  directly on GAME (pos0); no longer falls through to BASE TAB.
- Physical LShift key (bottom, outer) → RET (enter).

### ESC

ESC is bound **directly** on the outer home-row pinky (physical
Ctrl/Caps position) — plain `&kp ESC`, no fall-through, no Caps-swap
latency. (The top-outer key, which previously held ESC, now sends
GRAVE — push-to-talk, see below.)

### Push-to-talk

- Top-outer-left key (pos0, physical BASE-TAB position) → `` ` ``
  (GRAVE). This is Marvel Rivals push-to-talk. Hold it to transmit
  voice; release to stop. Confirm the host sees a backtick keypress and
  that Rivals' PTT (bound to grave in-game) keys the mic.

### Pass-through keys

These carry no GAME binding and fall through to BASE: the right-outer
thumb (→ RGUI/Super) and the entire right half. The left-outer thumb is
an explicit `&kp LALT`, and the top-outer-left key is now `&kp GRAVE`
(PTT) — neither falls through.

## GAMENUM — held weapon-switch sublayer

Held by the left inner thumb (`&mo GAMENUM`). Numbers stay live only
while the thumb is held; release returns to GAME. The home-row letters
(A→sprint, S/D/F→movement, G→use) are overridden by numbers while
held — weapon-switch is a deliberate pause action, so that's
acceptable.

- Hold left inner thumb. (Display: layer GAMENUM becomes active;
  confirm on-device whether the OLED shows "GAMENUM" or stays "GAME".)
- Top row left half: physical Q→1, W→2, E→3, R→4, T→5.
- Middle row left half: physical A→6, S→7, D→8, F→9, G→0.
- Bottom row: physical V → G and physical B → T. G was displaced from
  GAME by the Z/X/C shift; T was relocated within GAMENUM (it was never
  on GAME). The rest of the row falls through to GAME.
- Right half: pass-through to GAME (itself `&trans` → BASE).

## Touchpad

- Move cursor on the right-half touchpad. Speed scales at 2.25x
  (middle ground between the original 2.5x and the slower 2.0x
  tuning).
- On SYM only, slide on the touchpad → scroll. (NAV is cursor mode
  to support click-and-drag via NAV-T; only SYM stays scroll.)
- **Vertical:** slide DOWN, page scrolls DOWN (natural-scroll style).
- **Horizontal:** slide RIGHT, page scrolls RIGHT. (Y inverted, X not.)
- Scroll speed should feel ~30% slower than the upstream default.

## Touchpad — Azoteq TPS43 variant only (toucan_right_azoteq build)

Skip this section on the cirque right half. Gestures are decoded ON-CHIP
(IQS550) — enabled: tap, two-finger tap, press-and-hold, two-finger
scroll, pinch-zoom (added 2026-08-20); swipes/three-finger-swipe are
deliberately off (open decision, AZOTEQ_UPGRADE.md).

- Single tap anywhere → left click.
- Two-finger tap → right click.
- Press and hold, then slide → drag (left button held); release lifts it.
  A drag must never stick — if a button ever wedges pressed, tap once to
  clear it and note it (K_NO_WAIT drop-under-backpressure tradeoff,
  drivers/README.md).
- Two-finger slide → scroll, dominant axis only (no diagonal). Vertical
  direction must match the cirque's natural-scroll feel (invert-scroll-y).
- Slow one-finger movement still moves the cursor (no dead zone) — this
  guards the sensitivity=100 no-truncation decision.
- Cursor speed feels right (2026-08-20 retunes: on-chip x/y-resolution
  910x796, then 730x640 after "still a little fast"). Too fast/slow →
  adjust those two overlay values, nothing else.
- **Ballistics** (2026-08-20 pm): firmware is FLAT 1:1 — Windows
  "Enhance pointer precision" (kept ON for the desk mouse) is the only
  acceleration curve. Verify EPP is still on before judging feel
  (Pointer Options tab); a fast flick should cross the screen via EPP's
  gain, slow movement gets EPP's own fine-control damping. Firmware
  speed knob remains x/y-resolution in the azoteq overlay only.
- **Pinch-zoom** (Ctrl+Minus/Equal keypresses — the Ctrl+wheel macro
  experiment was reverted 2026-08-20, see toucan.dtsi comment): pinch
  out → zoom in, pinch in → zoom out, as discrete browser-zoom steps.
  Steps per pinch too many/few → zip_zoom_mapper `sensitivity` in
  toucan.dtsi (higher = more travel per step). For wheel-zoom apps,
  hold a physical Ctrl and two-finger-scroll instead.
- **Display (left half, after the toucan2 screen port):** style 2 shows
  a WPM chart + arc battery/profile/output widgets. Verify battery arcs
  for BOTH halves render, profile slot updates on BT profile switch,
  WPM chart moves while typing, and the deep-sleep "Sleep" screen still
  appears (~60 min idle).
- After the resolution retune, re-verify tap and scroll-start feel:
  chip distance thresholds are in resolution px and got ~2.25x bigger
  physically — sloppy taps or late scroll-start → set tap-distance /
  scroll-initial-distance in the overlay.
- Orientation sanity: slide right → cursor right, slide up → cursor up
  (switch-xy is set per beekeeb; if diagonals are mirrored, tune
  invert-x/invert-y in the overlay).
- SYM-layer scroller mode + NAV-T drag (sections above) still work
  (one-finger XY→scroll on SYM behaves identically to the cirque).
- **Known quirk — two-finger scroll WHILE HOLDING SYM:** the SYM scroller
  override bypasses the base listener's 1/20 wheel scaler, so the pad's
  native wheel events land ~3x faster AND direction-flipped (the child's
  Y-invert stacks on the driver's invert-scroll-y) vs. off-layer
  two-finger scroll. NOT an orientation bug — don't retune invert-x/y for
  it. Open decision in AZOTEQ_UPGRADE.md: document-and-live-with vs.
  retune the scroller child.
- Right-half **columns 3 and 5 still type** (physical cols on P0.04/P0.05
  — the pins the board's xiao_i2c would steal if its disable ever
  regresses; verify after any sleep/wake cycle too).
- **Boot timing:** azoteq init blocks ~610 ms normally and up to ~5.6 s
  if the pad doesn't answer (missing/miswired FPC) — a slow right-half
  boot on swap day is the driver retrying, not the June-style brick.
  If it boots slow EVERY time, reseat the FPC before firmware archaeology.

## Version stamp

- The left display shows the build's git short SHA in small text at the
  bottom edge. After ANY left flash, the SHA changing is the
  flash-took confirmation; it must match `git rev-parse --short` of the
  built commit.
- The right half logs `toucan firmware <sha>` at boot on the USB
  console — capture with the cable attached across a reset when a
  right-half flash needs verifying.

## Sleep / wake

- Idle for ~60 minutes. Display switches to "Sleep" screen.
- Press any key. Wakes back up; display restores normal status.

## Sanity / regression

- Display layer name updates on each layer change.
- Battery icons appear on the display for both halves.
- No phantom modifiers when typing fast.
