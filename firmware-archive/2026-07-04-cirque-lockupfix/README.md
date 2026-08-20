# Known-good CIRQUE firmware set — 2026-07-04 (post-lockup-fix)

The deployed, soak-tested cirque build from the July 2026 lockup fix
(cirque `K_NO_WAIT` pin `00a4a28` + central stack fixes). Archived
2026-08-19, immediately before the first Azoteq TPS43 flash, per
AZOTEQ_UPGRADE.md step 5: **these binaries — not a rebuild — are the
revert path** if we go back to the Cirque after a ZMK/module drift.

Source: `Downloads/toucan-fw-lockupfix/firmware/` (md5-verified copy).

Revert = double-tap-reset each half into DFU, copy the matching uf2,
reassemble the bagged cirque hardware (pad + cover + bottom case).
