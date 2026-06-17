#### `v0.6.3`
- Feature: chordal hold support via `SMTD_CHORDAL_HOLD` (#60). Implements QMK's "opposite-hands rule": a tap-hold settles as HOLD only when an opposite-hand key is involved, so same-hand rolls stay taps while cross-hand chords hold. Disabled by default — `#define SMTD_CHORDAL_HOLD 1` to enable; it compiles out entirely (zero code/RAM) when off. Handedness comes from a user-supplied `const char chordal_hold_layout[MATRIX_ROWS][MATRIX_COLS]` (QMK's `'L'` / `'R'` / `'*'` convention) or from an overridable weak `smtd_chordal_handedness()` function
- Fix: custom / derived keycode taps now feed the QMK leader buffer — `smtd_tap_code16` calls `smtd_leader_consume` (guarded by `LEADER_ENABLE`) before the direct send, so Leader sequences see taps that take the direct-send path, not just native-keycode taps (fixes #29)

#### `v0.6.2`
- Fix: `SMTD_LT` now uses native QMK `layer_on` / `layer_off` instead of the old `LAYER_PUSH` / `LAYER_RESTORE` (`layer_move`) macros. Activating a layer is now additive, so it no longer wipes foreign layer bits on release (fixes #57: a `TG()` / `TO()` toggled layer held alongside an `SMTD_LT` stays on after release; unblocks #44: two `SMTD_LT` layers coexist, so a tri-layer `layer_state_set` hook can light up the third layer)
- Fix: a held key released while another key is stacked on top now schedules `timeout_hold_release`, so the state finalizes and its modifier is released instead of hanging forever (fixes #58)

#### `v0.6.1`
- Fix: guard against double-removal of a state from the active stack in `smtd_apply_stage` — prevents `smtd_active_states_size` underflow and out-of-bounds write when a stale timeout fires for an already-removed state
- Fix: `SMTD_TK` / `SMTD_TTO` macros incorrectly expanded to `_ON_MKEY` variants with duplicated `key` argument; now correctly delegate to the base macros
- New: `smtd_reset()` API for test harnesses to clear all sm_td runtime state between scenarios

#### `v0.6.0`
- Feature: dynamic release timeout (fixes #45). The window that decides hold-vs-tap for an overlapping `↓A ↓B ↑A ↑B` sequence is now derived from your actual typing rhythm as `min(p1, p2) / SMTD_GLOBAL_RELEASE_RATIO` (default ratio 5) instead of a fixed timeout. Quick rolls shrink the window, slow deliberate typing widens it. The previous fixed `SMTD_TIMEOUT_RELEASE` (per-key too) stays the upper bound
- Config: new `SMTD_GLOBAL_RELEASE_RATIO` (default 5); set to 0 to disable the dynamic window and keep the fixed `SMTD_GLOBAL_RELEASE_TERM` behavior of 0.5.x

#### `v0.5.6`
- Feature: sm_td taps now go through the full QMK `process_record()` pipeline, so Caps Word, Auto Shift, Key Overrides and other QMK features can see them (fixes #23). Controlled by `SMTD_GLOBAL_PIPELINE_TAPS` (default on) and per-key `SMTD_FEATURE_PIPELINE_TAPS`
- Feature: `SMTD_ENABLE_QMK_TAPHOLD` — standard QMK `MT()` / `LT()` keycodes are handled with sm_td timing, no `SMTD_MT` / `SMTD_LT` macros required
- Fix: derived keycodes (multi-tap alternates, `*_ON_MKEY` macros) get a proper Caps Word pass instead of a hardcoded shift; no more stuck shift when Caps Word turns off mid-hold
- Removed: `SMTD_GLOBAL_MODS_PROPAGATION_ENABLED` (was disabled by default; captured only real mods and could flicker a physically held modifier)

#### `v0.5.5`
- Fix: module installation uses process_record stage instead of pre_process_record
- Fix: Better place for avoid_unused_variable_on_compile

#### `v0.5.4`
- Feature: split sm_td into .h and .c file

#### `v0.5.3`
- Feature: add SMTD_MBTE5_ON_MKEY macro

#### `v0.5.2`
- Fix: smtd_current_keycode is now compatible with AVR (fixes issue #48)

#### `v0.5.1`
- [QMK Community Modules](https://docs.qmk.fm/features/community_modules) installation option

#### `v0.5.0`
- 3+ finger roll interpretation
- simplified configuration (no more custom keycodes are needed)
- no more SMTD_KEYCODES_BEGIN and SMTD_KEYCODES_END are needed
- simplified SMTD_MT, SMTD_LT and other macros
- bunch of useful macros
- some bug fixes

#### `v0.4.0`
- simplified installation process (no need to init each key with `SMTD()` macro)
- added useful `SMTD_MT()`, `SMTD_MTE()`, `SMTD_LT()` macros for easier customization
- added debugging utilities (see [Debugging Guide](https://github.com/stasmarkin/sm_td/blob/main/docs/040_debugging.md))
- fixed several bugs (especially with sticky modifiers)
- made some memory optimizations (for storing active states)

#### `v0.3.1`
- optional delay between simultaneous key presses (see SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS in [feature flags](https://github.com/stasmarkin/sm_td/blob/main/docs/080_customization_features.md))

#### `v0.3.0`
- bug fix on pressing same macro key on stage SMTD_STAGE_TOUCH_RELEASE
- reduce args number for get_smtd_timeout_default and smtd_feature_enabled_default functions (see Upgrade instructions)
- better stage naming
- comprehensive documentation

#### `v0.2.1`
- rename `SMTD_ACTION_INIT` → `SMTD_ACTION_TOUCH`
- remove obsolete `SMTD_ACTION_INIT_UNDO` (use that action within `SMTD_ACTION_TAP` instead)
- better naming for timeout definitions (see Upgrade instructions)
- better naming for global definitions (see Upgrade instructions)

#### `v0.2.0`
- public beta test
- API is not stable yet, but it is usable

#### `v0.1.0`
- initial release and testing some basic functionality
