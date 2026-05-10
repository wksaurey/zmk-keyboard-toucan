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

- Hold left-inner (SYM). Display shows "SYM". Release; back to "BASE".
  **Note any noticeable activation lag** — that's the tap-dance latency
  question.
- Same for right-inner (NAV).
- Hold both inner thumbs simultaneously → display shows "ADJ".

## Layer keys — toggle (double-tap)

- Quick double-tap left-inner. Display shows "SYM" and stays. Type `!`
  (top-row leftmost on SYM) — produces EXCL.
- Double-tap left-inner again → returns to BASE.
- Repeat with right-inner / NAV.

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
  bottom col 10 = `=`, bottom col 11 = `+`.

## NAV — right click

- Hold NAV, tap top-row col 6 (was `/` previously, now `&mkp MB2`) over
  any window. Context menu opens (right-click).

## NAV — Delete and Ctrl+Alt+Del

- Hold NAV, tap top-row col 11 → `Delete` (deletes character forward).
- Ctrl+Alt+Del: hold left-pinky home (`LCTRL`), left-outer thumb
  (`LALT`), right-inner thumb (`NAV`), then tap top-row col 11. The
  host should see Ctrl+Alt+Delete (Windows lock screen, Task Manager,
  etc.).

## NAV — Bluetooth + studio

- Hold NAV, press G (home-row col 5) → studio_unlock. With USB
  connected, ZMK Studio in a browser (https://zmk.studio) should
  become editable.
- Bottom row left: BT_CLR + BT_SEL 0..4. Tap `BT_SEL 1` and confirm
  the display's profile area updates.

## SYM — special chars

- Hold SYM. Top row: `!@#$%`. Right side top: `^&*()`. Home row right:
  `-=[]\` and grave on rightmost. Bottom row right: `_+{}|~`.

## ADJ — function and media

- Activate ADJ (hold both inner thumbs, or toggle one then hold the
  other).
- F1–F12 on the left hand.
- Vol-down / mute / vol-up on the right home row.
- Confirm TAB on top-row col 0 of ADJ now actually emits Tab. (Fixed
  in `52895bc` — was `&mo TAB` which is invalid.)

## Touchpad

The chip is now in **absolute mode**, with the
`halfdane/zmk-input-gestures` processor in front of the existing scaler
chain. Speed/feel may differ from prior tuning — re-tune
`zip_xy_scaler` and the `sensitivity = "2x"` setting if needed.

- Move cursor on the right-half touchpad. Cursor tracks; verify X and
  Y directions still match (`x-invert;` carried over from the prior
  config — confirm it still feels right after the absolute-mode swap).
- On NAV or SYM, slide on the touchpad → scroll.
- **Vertical:** slide DOWN, page scrolls DOWN (natural-scroll style).
- **Horizontal:** slide RIGHT, page scrolls RIGHT. (Y inverted, X not.)

### Tap-to-click

In absolute mode the chip's hardware tap-to-click no longer fires;
`zip_gestures` re-implements it with `tap-detection` (default 120 ms).

- Quick tap on the touchpad → left click.
- Hold > 120 ms then drag → no click, just cursor movement.

### Inertial cursor

`zip_gestures` `inertial-cursor` (defaults: 2 px/ms threshold, 30%
decay). **Cursor only — the inertia handler emits cursor movement
directly, bypassing the input-processor chain that maps to scroll on
NAV/SYM.**

- Flick a fast swipe on BASE and lift. Cursor should keep coasting
  briefly, then decelerate and stop.
- Slow drag and lift. Cursor should NOT coast (below threshold).
- **Known UX hazard:** flick fast on NAV/SYM. While the finger is
  down you get scroll; after lift the inertia kicks in and produces a
  cursor jolt (because inertia bypasses the scroll mapper). If this
  is intolerable, raise `inertial-cursor-velocity-threshold` so
  scroll-flicks fall below the inertia trigger, or remove
  `inertial-cursor;` from `&zip_gestures`.

### Circular scroll

`zip_gestures` `circular-scroll` (default: outer 10% rim, requires
absolute mode).

- On any layer, place a finger on the outer ~10% of the touchpad and
  trace a clockwise arc. Page scrolls down.
- Counter-clockwise arc → scroll up.
- Touch starting in the inner ~90% should NOT trigger circular
  scroll — must move the cursor / scroll-layer-scroll as before.
- On NAV/SYM, confirm circular scroll and the existing `xy_to_scroll`
  mapping don't fight each other (a circular gesture should produce
  smooth scroll, not double-step or cancel).

## Sleep / wake

- Idle for ~60 minutes. Display switches to "Sleep" screen.
- Press any key. Wakes back up; display restores normal status.

## Sanity / regression

- Display layer name updates on each layer change.
- Battery icons appear on the display for both halves.
- No phantom modifiers when typing fast.
