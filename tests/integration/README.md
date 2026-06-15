# QMK-native tests for sm_td

These suites compile `sm_td.c` against a **real, unmodified `qmk/qmk_firmware`**
checkout and run it through QMK's own googletest harness (`tests/test_common`:
`TestFixture`, `TestDriver`, `KeymapKey`, `EXPECT_REPORT`). Unlike the Python
`ctypes` suites — which drive `process_smtd` against hand-written mocks
(`tests/sm_td_bindings.c`) — these exercise the genuine quantum pipeline:
`action.c`, `action_tapping.c`, `deferred_exec.c`, layer state, etc., compiled
into one native host executable. No QMK fork or patch is required.

## Run

```sh
# default: tag 0.33.5, suite smtd_qmk_taphold
sh run.sh

# explicit ref and/or suites
sh run.sh 0.33.5 smtd_qmk_taphold smtd_qmk_taphold_no_action_tapping

# a different QMK version (own checkout dir)
sh run.sh 0.32.16 smtd_qmk_taphold
```

`fetch.sh <ref>` shallow-clones qmk_firmware into `checkouts/<ref>/` (gitignored).
`run.sh` copies the overlay into `checkouts/<ref>/tests/<suite>/`, symlinks the
sm_td sources as `smtd_src/`, and runs `make test:<suite>`. The built binary is
`checkouts/<ref>/.build/test/<suite>.elf` (debuggable under gdb/lldb).

A **version matrix** is just several `run.sh <ref>` invocations (or a CI matrix);
each ref gets its own `checkouts/<ref>/`.

## Layout

```
overlay/<suite>/
  test.mk              # DEFERRED_EXEC_ENABLE, -DQMK_KEYBOARD_H="quantum.h", SRC += sm_td.c smtd_hooks.c
  config.h             # #include "test_common.h" + SMTD_* config (per-suite knobs)
  smtd_hooks.c         # process_record_user -> process_smtd; on_smtd_action; weak-hook defaults
  test_*.cpp           # TEST_F over TestFixture; set_keymap + EXPECT_REPORT
```

### How sm_td is wired in

- The test build has no community-module codegen (`build_test.mk` lacks it), so we
  route the real pipeline into sm_td via the weak `process_record_user` override
  (`smtd_hooks.c`) calling `process_smtd` — the same pipeline stage as the
  `process_record_sm_td` module hook.
- `smtd_hooks.c` must define `get_smtd_timeout` / `smtd_feature_enabled` (weak in
  `sm_td.h`): macOS `ld` rejects undefined-weak references in an executable.
- Each fixture calls `smtd_reset()` in `SetUp()` — sm_td keeps global runtime state
  that the QMK fixture does not clear between tests.

### Checkout patches (for the `smtd_via` suite)

`run.sh` applies two idempotent, upstream-safe guards to the gitignored checkout so
the `smtd_via` suite can reach the REAL VIA/Vial dynamic-keymap chain. Both are inert
for every other suite (they only change behavior under defines `smtd_via` sets):

1. `tests/test_common/test_fixture.cpp` — its own `keymap_key_to_keycode` override
   (non-weak, normally wins over QMK's weak one) is wrapped in
   `#ifndef SMTD_USE_DYNAMIC_KEYMAP`, so a suite can compile it out and supply its
   own dynamic-keymap-backed override instead.
2. `platforms/eeprom.h` — an `#ifndef` guard around the test-harness
   `TOTAL_EEPROM_BYTE_COUNT` (default 32, far too small for a real dynamic keymap),
   so a suite can raise it.

No QMK fork: the patches live in `run.sh`, target the throwaway checkout, and re-apply
after a re-fetch.

## Suites

- `smtd_qmk_taphold` — `SMTD_ENABLE_QMK_TAPHOLD=1`, action_tapping enabled (QMK default).
- `smtd_qmk_taphold_no_action_tapping` — same, plus `#define NO_ACTION_TAPPING`
  (QMK then calls `process_record` per event instead of routing MT()/LT() through
  action_tapping first). A/B control for the action_tapping caveat.
- `smtd_full` — the full engine with a rich `on_smtd_action` (in its `smtd_hooks.c`)
  mapping real keycodes to every customization macro: `SMTD_MT` (KC_A=LCtrl,
  KC_J=LAlt), `SMTD_MTE` (KC_S=LShift, eager), `SMTD_LT` (KC_D=layer 1),
  `SMTD_TD` (KC_F/ESC), `SMTD_TK` (KC_G→TAB after 3 taps), `SMTD_TTO` (KC_H→layer 2
  after 3 taps). Covers tap / hold+following-key / tap-then-hold / multi-tap /
  eager-mod / layer push+restore / external (non-sm_td) modifier / roll /
  multi-tap-to-key / multi-tap-to-layer. Release term is fixed
  (`SMTD_GLOBAL_RELEASE_RATIO 0`) so resolution depends only on the press pattern.
- `smtd_dynamic` — dynamic release term (`SMTD_GLOBAL_RELEASE_RATIO 5`). The decisive
  timing tests: the same MT/LT roll resolves **tap-tap** when the release gap mirrors
  the press gap, and **hold-tap** when releases are near-simultaneous.
- `smtd_caps_word` — `CAPS_WORD_ENABLE = yes` with a standard `caps_word_press_user`.
  Verifies sm_td's pipeline taps are visible to the real `process_caps_word`: an MT
  letter tap and a plain letter are shifted and keep Caps Word on; a space ends it; a
  `use_cl=false` mod-tap is hidden (unshifted, stays on); holding a non-shift mod-tap
  breaks Caps Word (same as native QMK).
- `smtd_external_mods` — an external plain `KC_LSFT` composed with an sm_td
  `SMTD_MT(LCtrl)`: mods stack on a following key, and all six release orders of
  {ext-Shift, sm_td-Ctrl, key} return to a clean mod state (`get_mods()==0`).
- `smtd_dynamic_fixed` / `smtd_dynamic_clamp` — release-term edges: ratio 0 (fixed
  50ms window: within=hold-tap, after=tap-tap) and ratio 1 (upper-clamped at 50ms).
- `smtd_via` — sm_td against a REAL VIA/Vial dynamic keymap (`DYNAMIC_KEYMAP_ENABLE`).
  Reaches the genuine QMK chain `keymap_key_to_keycode` → `keycode_at_keymap_location`
  → `dynamic_keymap_get_keycode` → emulated EEPROM (see "Checkout patches" above), and
  writes the keymap at runtime via `dynamic_keymap_set_keycode()` — exactly what a GUI
  remap does. `set_keymap()` is NOT used (`KeymapKey::press()` injects straight into the
  matrix); `smtd_hooks.c` supplies the dynamic-backed `keymap_key_to_keycode`. Proves
  sm_td resolves keycodes LIVE per press from the dynamic keymap (so GUI remaps are
  picked up without recompile): a dynamic keycode is resolved, a plain one passes
  through, a live remap of the same cell changes the next press's emitted keycode, a
  dynamic mod-tap holds its mod on a following key (also dynamic), and remapping a
  mod-tap cell to a plain key drops the hold.

## Status / findings

- Integration works: nine suites, 56 active tests green against qmk_firmware 0.33.5.
- The faceroll "crashes" once captured as `DISABLED_*_segfaults` were a TEST-HARNESS
  artifact, NOT engine bugs: those tests declared no `TestDriver driver;`, so QMK's
  host-driver pointer dangled (its only setter was the throwaway `TestDriver` in
  `SetUpTestCase`, long destroyed) and `led_task → host_keyboard_leds()` jumped
  through freed stack every scan. Every test that touches the keyboard MUST declare
  a `TestDriver driver;` (live for the whole body). Fixed and re-enabled as
  `mt_overlapped_by_mte`, `mt_plain_interleave`, `lt_hold_with_unmapped_follow_key`
  — sm_td handles all three multi-key sequences cleanly. (See AGENTS.md for the
  full root-cause writeup.)
- `smtd_full` exercises the engine's customization macros end-to-end through the
  real pipeline; `smtd_dynamic` covers release-rhythm (tap-tap vs hold-tap).
- The basic tap/hold/roll scenarios behave **identically** with action_tapping ON and
  OFF — they do not by themselves reproduce the `SMTD_ENABLE_QMK_TAPHOLD` action_tapping
  caveat (which is about hold latency / rhythm).
- Open finding: a **lone** long press of a raw `MT()` under `SMTD_ENABLE_QMK_TAPHOLD`
  resolves to a TAP, not a held mod (the hold is committed by an overlapping following
  key, not the touch timeout alone). This diverges from the Python mock suite, where
  `prolong()` forces the timeout into a hold. Worth investigating against hardware.
- The Python suites remain the fast, detailed unit layer; this contour is the
  real-pipeline integration layer.
