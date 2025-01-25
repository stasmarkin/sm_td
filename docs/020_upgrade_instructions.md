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
