# toucan keymap — project notes

ZMK config for the toucan split keyboard. The keymap lives at
`config/toucan.keymap`. Layers: `BASE(0) NAV(1) SYM(2) ADJ(3) GAME(4) GAMENUM(5)`.

## Displaying layers — two conventions

When showing or discussing a layer, use one of two views:

### Layout view (pretty / human-readable)

Default when showing a layer for review or discussion.

- One line per physical row, columns space-aligned.
- Left half ` | ` right half. **Omit a half entirely if it's all unused**, and
  note it in the header (e.g. `— right half unused`).
- Thumb cluster on its own indented line, prefixed `thumbs:`.
- Use the literal word `trans` for `&trans` keys (not `——` or em-dashes).
- Header line: `LAYERNAME (index)`.

Example:

```
GAME (4)            — right half unused
  ESC    TAB    Q      W   E   R
  LCTRL  LSHFT  A      S   D   F
  RET    Z      X      C   G   M
         thumbs:  trans   SPACE   MO(GAMENUM)
```

### Source view (code)

The literal ZMK `bindings = < … >` devicetree block from `config/toucan.keymap`.
Use when editing or showing an exact diff.

## GAME / GAMENUM usage

The **right half of the board is not used at all** while on the GAME or GAMENUM
layers — gaming is left-half-only. Layout view therefore drops the right half for
both layers. The right-half keys remain `&trans` in the keymap for now (switching
them to `&none` was considered and declined; revisit if stray presses become a
problem).
