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

Right-hand cluster after the rework — digits in their numpad positions,
math operators stacked in cols 10/11, `0` and `.` on the home/bottom
rows of col 6 (just left of the digit grid):

```
            col6   col7  col8  col9  col10  col11
row 0:      MB2    7     8     9     /      DEL
row 1:      0      4     5     6     *      -
row 2:      .      1     2     3     =      +
right thumbs:                  trans  SPACE  KP_ENTER
```

- Hold NAV. Type 7,8,9 / 4,5,6 / 1,2,3 from the right hand and `0` from
  col 6 row 1. All produce digits regardless of host Num Lock state.
  (Bug fix carried over from commit `26d59a6` — `N0..N9` and `DOT`
  used instead of `KP_*` digit codes.)
- Right-outer thumb → `KP_ENTER` (numpad Enter; identical to Enter for
  most apps, distinct in some — calculators, Excel cell-edit).
- Bottom-row col 6 → `.` (regular DOT, NumLock-safe).
- Math keys: top col 10 = `/`, mid col 10 = `*`, mid col 11 = `-`,
  bottom col 10 = `=` (regular EQUAL, not KP_EQUAL — most hosts
  ignore the HID keypad-equal code), bottom col 11 = `+`.

## NAV / SYM — trackpad clicks

Two ways to click while NAV or SYM is held:

- **Trackpad tap → right-click on NAV/SYM.** On layers 1 and 2 the
  `tap_to_rclick` mapper suppresses the chip's `INPUT_BTN_TOUCH`
  presence event (so non-tap touches don't hold MB2 during scroll)
  and remaps `INPUT_BTN_0` (the chip's tap pulse) to `INPUT_BTN_1`
  (MB2). Tap the trackpad while holding NAV → context menu opens.
  Slide without tapping → just scroll, no button held. On BASE the
  same tap stays as a left-click.
- **NAV top-row col 6 → explicit left-click (`&mkp MB1`).** Useful
  for click-and-drag: hold NAV, hold the col-6 key to keep the left
  button down, drag with the trackpad, release the key to drop. Was
  `MB2` before — moved to `MB1` once trackpad tap covered right-click
  on NAV.
- Side effect to be aware of: the chip's secondary-tap (tap-and-drag)
  is still globally on, so a deliberate double-tap-then-drag on
  NAV/SYM produces a *right-click* drag (because BTN_0 stays held and
  is remapped to MB2). Harmless if unused; use the explicit `MB1`
  button for left-click drag on NAV.

## NAV / SYM — Delete and Ctrl+Alt+Del

`DEL` lives at top-row col 11 on **both** NAV and SYM (mirrored), so
either layer-thumb works.

- Hold NAV (or SYM), tap top-row col 11 → `Delete` (forward delete).
- Ctrl+Alt+Del: hold `LCTRL` (left pinky home-row), `LALT` (left outer
  thumb), the layer-thumb (left-inner for NAV, right-inner for SYM),
  then tap top-row col 11. Host should see Ctrl+Alt+Delete (Windows
  lock screen, Task Manager, etc.).

## NAV — Bluetooth + studio

- Hold NAV, press G (home-row col 5) → studio_unlock. With USB
  connected, ZMK Studio in a browser (https://zmk.studio) should
  become editable.
- Bottom row left: BT_CLR + BT_SEL 0..4. Tap `BT_SEL 1` and confirm
  the display's profile area updates.

## SYM — special chars and right-thumb Enter

- Hold SYM. Top row: `!@#$%`. Right side top: `^&*()`. Home row right:
  `-=[]\` and grave on rightmost. Bottom row right: `_+{}|~`.
- **Right-outer thumb on SYM = `Enter`** (replaces the BASE-layer
  `RGUI`). Lets you hit Enter one-handed on the right while the right
  hand is in trackpad territory — hold SYM with right-inner thumb,
  tap right-outer thumb, release SYM.

## ADJ — function and media

- Activate ADJ (hold both inner thumbs, or toggle one then hold the
  other).
- F1–F12 on the left hand.
- Vol-down / mute / vol-up on the right home row.
- Confirm TAB on top-row col 0 of ADJ now actually emits Tab. (Fixed
  in `52895bc` — was `&mo TAB` which is invalid.)

## Touchpad

- Move cursor on the right-half touchpad. Speed scales at 2.25x
  (middle ground between the original 2.5x and the slower 2.0x
  tuning).
- On NAV or SYM, slide on the touchpad → scroll.
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
