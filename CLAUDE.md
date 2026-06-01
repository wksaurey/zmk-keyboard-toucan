# toucan keymap — project notes

ZMK config for the toucan split keyboard. The keymap lives at
`config/toucan.keymap`. Layers: `BASE(0) NAV(1) SYM(2) ADJ(3) GAME(4) GAMENUM(5)`.

## Which keymap is authoritative

There are **two** keymap files — edit only the first:

- `config/toucan.keymap` — **authoritative.** This is what actually builds: the
  `build.yaml` matrix compiles the `toucan_left`/`toucan_right` shields and ZMK
  searches `ZMK_CONFIG` (= `config/`) before the shield dir, so this file wins.
- `boards/shields/toucan/toucan.keymap` — **do not edit.** This is beekeeb's
  upstream default keymap (BASE/NAV/SYM/ADJ only), kept as the shield module's
  standalone fallback. This repo is a fork of `beekeeb/zmk-keyboard-toucan`;
  leaving it intact preserves upstream provenance. It is never compiled here.

## Displaying layers — two conventions

When showing or discussing a layer, use one of two views:

### Layout view (pretty / human-readable)

Default when showing a layer for review or discussion.

- One line per physical row, columns space-aligned.
- Left half ` | ` right half. **Omit a half if its alpha/number columns are all
  unused**, and note it in the header (e.g. `— left-half-only`). If the omitted
  half still holds a live key (e.g. the `tog GAME` exit on a thumb), call it out
  in an `exit:`/note line below the grid rather than silently dropping it.
- Thumb cluster on its own indented line, prefixed `thumbs:`.
- Use the literal word `trans` for `&trans` keys (not `——` or em-dashes).
- Header line: `LAYERNAME (index)`.

Example:

```
GAME (4)            — left-half-only (right half detached during play)
  trans  TAB    Q      W   E   R
  ESC    LSHFT  A      S   D   F
  RET    LCTRL  Z      X   C   M
         thumbs:  LALT   SPACE   MO(GAMENUM)
  exit:  TOG(GAME) on the inner right thumb — re-dock right half to return to work
```

### Source view (code)

The literal ZMK `bindings = < … >` devicetree block from `config/toucan.keymap`.
Use when editing or showing an exact diff.

## Keep TESTING.md in sync

`TESTING.md` is a manual, on-device test checklist that maps physical keys to
host output per layer. **After every keymap change, review TESTING.md and update
it if the change affects what any documented key does.** Keymap edits drift the
test steps silently (a renamed layer, a moved keycode, a changed mod) — stale
test steps are worse than none. Treat updating TESTING.md as part of the same
change, not a follow-up.

## GAME / GAMENUM usage

These layers are designed for **left-half-only** play. The toucan is a split
keyboard, and during gaming the right half is physically **detached and moved
aside** to free up desk space for the mouse — so only the left-hand keys and the
left thumb cluster are reachable in play.

The one key that lives on the right half is `&tog GAME` on the inner right thumb
(position 39), and it is the **exit back to work mode**. The workflow: detach the
right half and game on the left half; when done, re-dock the right half and press
its inner thumb key to toggle GAME off. Placing the exit there is deliberate — it
can't be fumbled mid-game because the right half isn't even present. (This is also
why `tog GAME` is the *only* way out of the layer: ADJ, the other place it appears,
is unreachable from GAME.)

The right-half alpha/number columns are unused on these layers and remain `&trans`
in the keymap (switching them to `&none` was considered and declined). Layout view
drops those columns but keeps the `exit:` note for the right-thumb toggle.

### Marvel Rivals key coverage

GAME is tuned for Marvel Rivals (hero shooter). Reachable on the left half in
play: movement (WASD via the shifted ESDF cluster), jump (SPACE thumb), crouch
(LCTRL), abilities E/LSHIFT/F, ultimate (Q), reload (R), scoreboard (TAB), text
chat (RET), menu (ESC).

- **Team-Up abilities are Z / X / C** (Team-Up Ability 1/2/3 in Rivals). All three
  are on the GAME bottom row — physical X→Z, C→X, V→C. This is the whole reason
  for the Z/X/C row; do not treat them as generic/unused keys. Rivals caps team-ups
  at exactly **3 slots** (Z/X/C), so no team-up combo can ever appear on a key the
  GAME layer can't reach — full coverage is guaranteed by the game's own cap.
- **M = change-hero**, rebound from the default H, and also used in other games —
  keep it on GAME (physical B). Default H is unused.
- **LALT = hero profile**, rebound from the default F1 — this is why LALT is
  explicitly bound on the GAME outer-left thumb (pos36). Not vestigial; keep it.
- **Push-to-talk is T**, currently on GAMENUM (held), which is awkward for a
  hold-to-talk key. **Mouse-bound (not on the keyboard): melee (V), ping (G / middle
  mouse), and the show-breakable-objects key** — the GAME layer intentionally omits
  these; don't flag them as missing in coverage reviews.

**Future idea (not now):** consider moving the talk control (push-to-talk, T) to
the top-left button (pos0), which currently just duplicates TAB (TAB is already on
physical Q). The pinky can hold pos0 while playing. Deferred.
