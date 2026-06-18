## `v0.6.3` → `v0.6.4`
- replace both `sm_td.h` and `sm_td.c` with newer versions (or just update the community module)
- behavior change: the dynamic release window default is now `SMTD_GLOBAL_RELEASE_PERCENT 30` (a slightly wider window — fewer hold→tap-tap misfires, especially on the pinky). To restore the previous behavior, add `#define SMTD_GLOBAL_RELEASE_PERCENT 20` to your `config.h`
- the window is now configured by `SMTD_GLOBAL_RELEASE_PERCENT` (window = `min(p1, p2) * percent / 100`): raise it for easier holds, lower it for fewer false holds, set `0` to disable the dynamic window

## `v0.5.6` → `v0.6.0`
- replace both `sm_td.h` and `sm_td.c` with newer versions (or just update the community module)
- behavior change: `SMTD_TIMEOUT_RELEASE` is now a dynamic window derived from your typing rhythm (a fraction of `min(p1, p2)`), clamped above by your existing `SMTD_TIMEOUT_RELEASE`. You will likely get fewer false holds during fast rolls out of the box
- if you want the exact 0.5.x behavior back, add `#define SMTD_GLOBAL_RELEASE_PERCENT 0` to your `config.h` — the release window then falls back to the fixed `SMTD_GLOBAL_RELEASE_TERM`
- if you tuned `SMTD_GLOBAL_RELEASE_TERM` to fight false holds, consider lowering `SMTD_GLOBAL_RELEASE_PERCENT` instead now

## `v0.5.5` → `v0.5.6`
- replace both `sm_td.h` and `sm_td.c` with newer versions (or just update the community module)
- behavior change: sm_td now sends taps through the full QMK pipeline, so QMK features like Caps Word see them. If this causes problems for a specific key (e.g. a key that is part of a Combo), disable it per key via `SMTD_FEATURE_PIPELINE_TAPS` in `smtd_feature_enabled`, or globally with `#define SMTD_GLOBAL_PIPELINE_TAPS false`
- if you defined `SMTD_GLOBAL_MODS_PROPAGATION_ENABLED` in your `config.h`, remove it — the flag is gone and the define has no effect anymore

## `v0.5.4` → `v0.5.5`
- replace `sm_td.h` with newer version
- no need to upgrade if sm_td 0.5.0 or latter is good for you

## `v0.5.3` → `v0.5.4`
- replace `sm_td.h` with newer version
- no need to upgrade if sm_td 0.5.0 or latter is good for you

## `v0.5.2` → `v0.5.3`
- replace `sm_td.h` with newer version
- no need to upgrade if sm_td 0.5.0 or latter is good for you

## `v0.5.1` → `v0.5.2`
- replace `sm_td.h` with newer version
- no need to upgrade if sm_td 0.5.0 or latter compiles for you

## `v0.5.0` → `v0.5.1`
- nothing to do, we have just added another installation option. If you have already installed it, you're good to go.
- no need to upgrade if sm_td 0.5.0 compiles for you

## `v0.4.0` → `v0.5.0`
- replace `sm_td.h` with newer version
- remove SMTD_KEYCODES_BEGIN and SMTD_KEYCODES_END (if you want to)
- change function `void on_smtd_action(...)` to `smtd_resolution on_smtd_action(...)`
- add `return SMTD_RESOLUTION_UNHANDLED;` as the last line of `on_smtd_action(...)` function
- next you have to upgrade SMTD_* macros. There are two ways of doing this:
  - **Option 1**: (recommended, but long) for every SMTD_MT (or _LT) macro remove the first argument with custom keycode. So, instead of `SMTD_MT(CKC_A, KC_A, KC_LEFT_GUI, 2)` just leave `SMTD_MT(KC_A, KC_LEFT_GUI, 2)`. You also need to replace every occurrence of CKC_A to KC_A (so, put it in your keycodes matrix, combos and so on). And remove CKC_A from custom_keycodes entirely. 
  - **Option 2**: (fast, but ugly) replace SMTD_MT with SMTD_MT_ON_MKEY.  So, instead of `SMTD_MT(CKC_A, KC_A, KC_LEFT_GUI, 2)` you just write `SMTD_MT_ON_MKEY(CKC_A, KC_A, KC_LEFT_GUI, 2)`. That's it. 

## `v0.3.1` → `v0.4.0`
- replace `sm_td.h` with newer version
- remove `smtd_state smtd_states[]` and `size_t smtd_states_size`
- move all sm_td keycodes close together in your enum, and wrap them with `SMTD_KEYCODES_BEGIN` and `SMTD_KEYCODES_END`, so that every sm_td keycode will be inside BEGIN and END

## `v0.3.0` → `v0.3.1`
- replace `sm_td.h` with newer version

## `v0.2.1` → `v0.3.0`
- replace `sm_td.h` with newer version
- replace `get_smtd_timeout_default(keycode, timeout)` calls with `get_smtd_timeout_default(timeout)`
- replace `smtd_feature_enabled_default(keycode, feature)` calls with `smtd_feature_enabled_default(feature)`

## `v0.2.0` → `v0.2.1`
- replace `sm_td.h` with newer version
- remove every SMTD_ACTION_INIT_UNDO. If you need any UNDO action, put in SMTD_ACTION_TAP instead
- rename `SMTD_ACTION_INIT` → `SMTD_ACTION_TOUCH`
- rename `SMTD_TIMEOUT_JOIN` → `SMTD_TIMEOUT_FOLLOWING_TAP`
- rename `SMTD_TIMEOUT_TAP` → `SMTD_TIMEOUT_SEQUENCE`
- rename `SMTD_TIMEOUT_INIT` → `SMTD_TIMEOUT_TAP`
- rename `SMTD_INIT_TERM` → `SMTD_GLOBAL_TAP_TERM`
- rename `SMTD_TAP_TERM` → `SMTD_GLOBAL_SEQUENCE_TERM`
- rename `SMTD_JOIN_TERM` → `SMTD_GLOBAL_FOLLOWING_TAP_TERM`
- rename `SMTD_RELEASE_TERM` → `SMTD_GLOBAL_RELEASE_TERM`
