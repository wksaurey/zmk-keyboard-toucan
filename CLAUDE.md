# toucan keymap — project notes

ZMK config for the toucan split keyboard. The keymap lives at
`config/toucan.keymap`. Layers: `BASE(0) NAV(1) SYM(2) ADJ(3) GAME(4) GAMENUM(5)`.

## Session start — offer the upstream check

At the start of each session in this repo, ask Kolter whether to run the
upstream check: search `beekeeb/zmk-keyboard-toucan`,
`geeksville/cirque-input-module`, and `zmkfirmware/zmk` for issues / PRs /
commits updated since the last check that touch split lockups, input relay,
central stacks, or the cirque driver. Public repos — `gh`/web search, no auth.

Baseline as of 2026-08-19 (previous: 2026-07-04): `cirque-input-module`
PR #4 (`K_NO_WAIT`) still OPEN, head still `00a4a28` — **our `west.yml` pins
that SHA, so its fate matters** (merge/rebase/supersede → repin); beekeeb
issue #23 still open (CS-line `GPIO_PULL_UP`, not our bug); beekeeb PR #19
merged / #20/#24 closed-unmerged (unchanged). NEW since 07-04:
`beekeeb/zmk-keyboard-toucan2` published 2026-07-28 (full azoteq reference
config, builds on ZMK v0.3); `geeksville/zmk_driver_azoteq` gained
three_finger_swipe + throttle + DT-gated build (66d51024) — we vendored that
content K_NO_WAIT-patched into `drivers/` (add `zmk_driver_azoteq` repos to
the upstream check list; K_FOREVER still unpatched upstream, so check
whether our patch can be retired). Report only movement against this
baseline, and update this section's baseline after each check.

Display note: the screen UI (`boards/shields/nice_view_gem/`) is a VENDORED,
beekeeb-modified copy of `M165437/nice-view-gem` — it never updates via west.
Our copy predates upstream's 2026-02-16 API rename
(`zmk_endpoints_selected` → `zmk_endpoint_get_selected`), which targets ZMK
main and would break our v0.3 build if pulled today — but becomes REQUIRED
whenever we rebase to ZMK ≥0.4. Check that repo only when planning the rebase.

Watch item (kit ARRIVED 2026-08-19, firmware implemented on branch
`kolter/azoteq-tps43`): the Azoteq TPS43 upgrade — status header, resolved
blocked-on-beekeeb answers, and the trial protocol live in
`AZOTEQ_UPGRADE.md`; the vendored/patched driver rules in
`drivers/README.md`. The upgrade is explicitly reversible (he may prefer
the Cirque); the cirque SHIELD stays in the repo, but the CI matrix
builds only left + azoteq right + settings_reset (Kolter, 2026-08-20) —
cirque revert = archived uf2s in `firmware-archive/`, or re-add the
build.yaml entry (template in its comment).

Watch item: the new Toucan display options Kolter wants (WPM pixel graph +
toucan icon, `CONFIG_TOUCAN_STATUS_SCREEN=0/1/2`) are now PUBLIC in
`beekeeb/zmk-keyboard-toucan2` `boards/shields/nice_view_gem/` (10 new
widget files vs our vendored copy: chart, layer_logo, arc/vertical battery
variants). Same XIAO + nice!view stack, ZMK v0.3 — vendoring into our
`boards/shields/nice_view_gem/` looks straightforward; do it as its OWN
change (not mixed into trackpad work).

Why this exists: the July-2026 lockup root cause sat in an upstream PR for a
month while we instrumented from scratch — a one-shot "upstream has nothing"
check went stale (see `.claude/retro.md` 2026-07-04). Retire this section
once the fix has soaked clean for a month or two AND cirque #4 has landed
somewhere permanent.

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

### Naming ONE key — lead with its BASE legend

When pointing Kolter at a single physical key, say **what that key is on BASE**
first: "the right-shift key", "the B key". That is the label his fingers and his
eyes actually use. Position indices (`pos35`), grid coordinates
("bottom-row col 11"), and half-relative descriptions ("bottom-right corner") are
all ambiguous to a reader who is not looking at the keymap source, and
"bottom-right corner" in particular reads as *the right half's corner* to us and
as *somewhere near B* to someone picturing the whole board.

Cost of getting this wrong: two extra round-trips confirming which key was meant
(2026-08-27, binding `&bootloader` — he read "bottom-right corner" as the B key,
which is on the opposite half and would have flashed the wrong controller).

Format: **BASE legend first, then the position index in parentheses** for the
edit — `the right-shift key (pos35, ADJ bottom row col 11)`. And when the half
matters — anything source-local like `&bootloader` or `&sys_reset` — say which
half the key lives on explicitly, because that is what the behavior acts on.

## Keep TESTING.md in sync

`TESTING.md` is a manual, on-device test checklist that maps physical keys to
host output per layer. **After every keymap change, review TESTING.md and update
it if the change affects what any documented key does.** Keymap edits drift the
test steps silently (a renamed layer, a moved keycode, a changed mod) — stale
test steps are worse than none. Treat updating TESTING.md as part of the same
change, not a follow-up.

## Which half needs flashing, and how to prove it

**Left is the central** (`boards/shields/toucan/Kconfig.defconfig`:
`ZMK_SPLIT_ROLE_CENTRAL` under `if SHIELD_TOUCAN_LEFT`). The keymap, combos,
behaviors and layer logic all live there; the right half only reports key
positions. So **any keymap-only change is a LEFT-half flash** — including adding
a key bound to a right-half position, and including `&bootloader` (verified
source-local on ZMK v0.3, 2026-08-27: the central relays execution to the half
the key sits on, so the peripheral gains the key without being reflashed).

**The version stamp makes every build byte-different, so never conclude "the
right half changed" from a checksum.** `dc9698d`/`a79dbf9` bake `GITHUB_SHA` into
the display and boot log, so two builds of identical source produce different
`.uf2` files. To decide whether a half actually needs flashing, diff at block
level and count:

```python
a=open(old,'rb').read(); b=open(new,'rb').read()
diff=[i for i in range(min(len(a),len(b))//512) if a[i*512:(i+1)*512]!=b[i*512:(i+1)*512]]
```

One differing block out of ~944 = the stamp only, functionally identical
(measured 2026-08-27). Many differing blocks = real change.

**Cheap pre-CI sanity check on the keymap:** count `&`-bindings per layer; every
layer must be exactly **42** (12+12+12+6). A miscount is the classic silent
devicetree break. ZMK source is not vendored locally and `west` is not installed,
so CI is the only real compile check — the binding count is what catches the
common error before burning a CI round.

**Downloaded firmware goes to `Downloads/toucan-fw-<sha>/`** (matches the
`firmware-archive/` README convention). When a newer set supersedes an older one,
say which folder to use or delete the stale one — two lookalike folders is a
flashing hazard.

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

- **Team-Up abilities are Z / X / C** — confirmed in Kolter's in-game keybind menu
  (Settings → Keyboard). All three are on the GAME bottom row (physical X→Z, C→X,
  V→C); that is the reason the row exists. Rivals' team-up slots are 1/2/3 = Z/X/C,
  so the team-up combos are fully covered on the left half. (Note: **T is not a
  team-up** — earlier confusion; the `T` on GAMENUM is vestigial in this config.)
- **M = change-hero**, rebound from the default H, and also used in other games —
  keep it on GAME (physical B). Default H is unused.
- **LALT = hero profile**, rebound from the default F1 — this is why LALT is
  explicitly bound on the GAME outer-left thumb (pos36). Not vestigial; keep it.
- **Push-to-talk = grave/backtick (`)** — confirmed in-game. Bound on GAME at
  the **top-outer-left key (pos0)** as `&kp GRAVE`. Hold to transmit. This is the
  spare reachable key that previously fell through to TAB; it now hosts PTT.
- **Mouse-bound (not on the keyboard): melee (V), ping (G / middle mouse), and the
  show-breakable-objects key** — the GAME layer intentionally omits these; don't
  flag them as missing in coverage reviews.

The top-left key (pos0) now holds push-to-talk (`&kp GRAVE`); TAB remains on
physical Q. (This was the deferred spare-key slot — now spent on PTT.)
