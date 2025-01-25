#### `v0.4.0`
- simplified installation process (no need to init each key with `SMTD()` macro)
- added useful `SMTD_MT()`, `SMTD_MTE()`, `SMTD_LT()` macros for easier customization
- added debugging utilities (see [Debugging Guide](https://github.com/stasmarkin/sm_td/blob/main/docs/040_debugging.md))
- fixed several bugs (especially with sticky modifiers)
- made some memory optimizations (for storing active states)

#### `v0.3.1`
- optional delay between simultaneous key presses (see SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS in [feature flags](https://github.com/stasmarkin/sm_td/blob/main/docs/080_customization_features.md))

#### `v0.3.0`
- bug fix on pressing same macro key on stage SMTD_STAGE_RELEASE
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
