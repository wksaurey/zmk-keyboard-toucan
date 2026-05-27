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

## BASE — RSHFT and combo

- Hold the new bottom-right RSHFT, type `a`. Should produce `A`.
- Press LSHFT + RSHFT simultaneously. Produces ESC keycode → toggles
  CAPS LOCK at the host (OS-level esc/caps swap). The combo has a
  2000 ms window after a 100 ms idle gate, so it does **not** fire
  while typing flow — pause briefly first, then chord.

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
- F-keys on the left hand: F1–F10 + F12 (11 keys). F11 was displaced
  from middle-row col 5 by `&tog GAME` (commit `dff378a`).
- Vol-down / mute / vol-up on the right home row.
- Confirm TAB on top-row col 0 of ADJ now actually emits Tab. (Fixed
  in `52895bc` — was `&mo TAB` which is invalid.)
- ADJ middle-row col 5 (G position) → `&tog GAME` — toggles into the
  GAME layer. See GAME section below.

## GAME — FPS-style gaming layer

Added in `dff378a` and polished in `0c30f4c`. ESDF replaces WASD,
with mods on the pinky column. Recovered game keys are remapped so
common FPS bindings (Q/W/E/R/F, M, R-as-reload) stay reachable.

### Enter / exit

- **Enter:** activate ADJ (hold both inner thumbs), tap middle-row
  col 5 (G position) → display shows "GAME".
- **Exit:** tap right inner thumb (single tap of `&tog GAME` bound
  directly inside GAME). Display returns to "BASE".
- The ADJ-G path **does not** exit GAME because layer 4 (GAME)
  outranks layer 3 (ADJ) and overrides pos 17 with `&kp F` — so the
  exit is on the inner thumb on purpose.

### Movement and mods

- Slide your hand one column right of WASD home. Index/middle/ring
  fingers should rest on F/D/S.
- Press E (top, middle finger) → host receives W.
- Press S (home, ring) → host receives A.
- Press D (home, middle) → host receives S.
- Press F (home, index) → host receives D.
- Press A (home, pinky) → host receives LSHIFT (sprint).
- Press Z (bottom, pinky) → host receives LCTRL (crouch).
- Press left-middle thumb → host receives SPACE (jump).

### Recovered game keys

These are mapped because the ESDF shift displaces the original
QWERTY positions:

- Q position → host receives TAB (scoreboard/inventory).
- W position → host receives Q.
- R position → host receives E.
- T position → host receives R (reload).
- G position → host receives F.
- B position → host receives M.

### Pass-through keys

These fall through to BASE and arrive unchanged at the host: TAB,
LCTRL/CAPS (left outer home), LSHFT (left outer bottom), X, C, V,
ALT (left outer thumb), the entire right half, and the right-outer
thumb (RGUI/Super).

### ESC

ESC is not directly bound on GAME. Tap the left outer home-row key
(physical Caps Lock position) → falls through to BASE's
`&mt LCTRL CAPS` → tap fires CAPS → OS-level Caps→Esc swap delivers
ESC. There is a ~200ms tap-vs-hold disambiguation latency; fine for
menu transitions, not snappy.

## NUMGAME — held weapon-switch sublayer

Held by the left inner thumb (`&mo NUMGAME`, defined in `0c30f4c`).
Numbers stay live only while the thumb is held; release returns to
GAME. The middle row's letter bindings (A=Shift, S/D/F=movement,
G=F) are overridden by numbers while held — weapon-switch is a
deliberate pause action so this is acceptable.

- Hold left inner thumb. Display does not change layer name (GAME
  is still shown because NUMGAME shares display).
- Top row left half: Q→1, W→2, E→3, R→4, T→5.
- Middle row left half: A→6, S→7, D→8, F→9, G→0.
- Bottom row left half: pass-through to GAME (LCTRL on Z, etc.).
- Right half: all pass-through to GAME (which mostly falls through
  to BASE).

## Touchpad

- Move cursor on the right-half touchpad. Speed scales at 2.25x
  (middle ground between the original 2.5x and the slower 2.0x
  tuning).
- On SYM only, slide on the touchpad → scroll. (NAV is cursor mode
  to support click-and-drag via NAV-T; only SYM stays scroll.)
- **Vertical:** slide DOWN, page scrolls DOWN (natural-scroll style).
- **Horizontal:** slide RIGHT, page scrolls RIGHT. (Y inverted, X not.)
- Scroll speed should feel ~30% slower than the upstream default.

## Sleep / wake

- Idle for ~60 minutes. Display switches to "Sleep" screen.
- Press any key. Wakes back up; display restores normal status.

## Sanity / regression

- Display layer name updates on each layer change.
- Battery icons appear on the display for both halves.
- No phantom modifiers when typing fast.
