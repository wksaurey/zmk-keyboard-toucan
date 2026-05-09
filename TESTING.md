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
- Press LSHFT + RSHFT simultaneously. Toggles Caps Lock. Press both
  again to toggle off.

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

- Hold NAV. Type 7,8,9 / 4,5,6 / 1,2,3 from the right hand. All produce
  digits regardless of host Num Lock state. (Was a bug fixed in commit
  `26d59a6` — KP_N* keycodes only emitted digits with Num Lock on.)
- Right-outer thumb → `0`.
- Math keys: top-row col 6 = `/`, top-row col 11 = `*`, home col 11 = `-`,
  home col 12 = `+`, bottom col 7 = `=`, home col 7 = `.`.

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

- Move cursor on the right-half touchpad. Speed scales at 2.0x (was
  2.5x — slower than the original feel).
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
