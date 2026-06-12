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
