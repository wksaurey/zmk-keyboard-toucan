# toucan keymap — project notes

ZMK config for the toucan split keyboard. The keymap lives at
`config/toucan.keymap`. Layers: `BASE(0) NAV(1) SYM(2) ADJ(3) GAME(4) GAMENUM(5)`.

## Session start — offer the upstream check

At the start of each session in this repo, ask Kolter whether to run the
upstream check: search `beekeeb/zmk-keyboard-toucan`,
`geeksville/cirque-input-module`, and `zmkfirmware/zmk` for issues / PRs /
commits updated since the last check that touch split lockups, input relay,
central stacks, or the cirque driver. Public repos — `gh`/web search, no auth.

Baseline as of 2026-07-04: beekeeb PR #19 merged (central
`INPUT_THREAD_STACK_SIZE` fix — deployed here), PR #20/#24 closed-unmerged,
`cirque-input-module` PR #4 (`K_NO_WAIT`) and #5 OPEN — **our `west.yml` pins
PR #4's head SHA `00a4a28`, so its fate matters** (merge/rebase/supersede →
repin), beekeeb issue #23 open (CS-line `GPIO_PULL_UP`, not our bug). Report
only movement against this baseline, and update this section's baseline after
each check.

Display note: the screen UI (`boards/shields/nice_view_gem/`) is a VENDORED,
beekeeb-modified copy of `M165437/nice-view-gem` — it never updates via west.
Our copy predates upstream's 2026-02-16 API rename
(`zmk_endpoints_selected` → `zmk_endpoint_get_selected`), which targets ZMK
main and would break our v0.3 build if pulled today — but becomes REQUIRED
whenever we rebase to ZMK ≥0.4. Check that repo only when planning the rebase.

Watch item: Kolter ordered beekeeb's Azoteq TPS43 upgrade kit (ships
late-Jul/Aug 2026) — full prep, integration map, and the
blocked-on-beekeeb re-check list live in `AZOTEQ_UPGRADE.md`. The upgrade
is explicitly reversible (he may prefer the Cirque); keep both trackpad
configs buildable.

Watch item: beekeeb announced two NEW Toucan display options (WPM pixel
graph + 1-bit pixel-art toucan icon, compile-time selectable) in the
2026-06-04 "Introducing Toucan2" blog post — Kolter wants these. As of
2026-07-04 no code is public (their `zmk-keyboard-displaydemo` repo is a
generic sprite demo, not the Toucan screens); expected around the Toucan2
ship date, late July 2026. When it lands, evaluate vendoring into
`boards/shields/nice_view_gem/` (same XIAO + nice!view stack, likely
portable to Toucan1).

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

- **Team-Up abilities are Z / X / C** — confirmed in Kolter's in-game keybind menu
  (Settings → Keyboard). All three are on the GAME bottom row (physical X→Z, C→X,
  V→C); that is the reason the row exists. Rivals' team-up slots are 1/2/3 = Z/X/C,
  so the team-up combos are fully covered on the left half. (Note: **T is not a
  team-up** — earlier confusion; the `T` on GAMENUM is vestigial in this config.)
- **M = change-hero**, rebound from the default H, and also used in other games —
  keep it on GAME (physical B). Default H is unused.
- **LALT = hero profile**, rebound from the default F1 — this is why LALT is
  explicitly bound on the GAME outer-left thumb (pos36). Not vestigial; keep it.
- **Push-to-talk = grave/backtick (`)** — confirmed in-game. It is **not bound on
  GAME or GAMENUM**, and Kolter currently plays without keyboard PTT in left-half
  mode (voice handled off-layer / mouse). Revisit only if keyboard PTT is wanted.
- **Mouse-bound (not on the keyboard): melee (V), ping (G / middle mouse), and the
  show-breakable-objects key** — the GAME layer intentionally omits these; don't
  flag them as missing in coverage reviews.

**Future idea (not now):** the top-left key (pos0) currently just duplicates TAB
(TAB is already on physical Q), so it's a spare reachable key — a candidate home if
a combat key ever needs one (e.g. keyboard PTT). Deferred.
