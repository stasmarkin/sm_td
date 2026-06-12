```
This documentation is up to date for version 0.5.6.
```


There are global feature flags that may help you with customizing sm_td behavior:

- `SMTD_GLOBAL_AGGREGATE_TAPS` (default is false)

  Default behavior of sm_td library is to call tap action every time it's considered as a tap. This option allows to aggregate taps and call tap action only once after tap sequence is finished (same as original QMK Tap Dance).


- `SMTD_GLOBAL_PIPELINE_TAPS` (default is true)

  When enabled, sm_td sends resolved tap keys through the full QMK pipeline (`process_record`) instead of raw `tap_code16` / `register_code16` calls. This way other QMK features (Caps Word, Auto Shift, Key Overrides, Repeat Key, etc.) can see the keys sm_td sends: for example, Caps Word properly shifts letters, turns itself off after a space on an `SMTD_LT` key and calls your `caps_word_press_user`.

  This only works when the key being sent matches the keycode in your keymap at the pressed position (the common case for `SMTD_MT` / `SMTD_LT` macros without custom keycodes). Derived keycodes — alternate multi-tap keys, `*_ON_MKEY` macro keycodes — are still sent directly, but with a manual Caps Word pass, so Caps Word stays correct for them too.

  Keys with `use_cl = false` are always sent directly and stay invisible to Caps Word.

  If a key conflicts with another `process_record`-based feature (e.g. it is part of a Combo), you can disable the pipeline for that key via `SMTD_FEATURE_PIPELINE_TAPS` in `smtd_feature_enabled` (see below).


- `SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS` (default is 0)

  One some stages sm_td may generate several events that would be sent immediately to OS. For example, by releasing following key sm_td may decide to unset modifier, send first key press, then set modifier and send following key press and release — everything one by one as soon as possible. In some cases corresponding keyboard driver or app may not register that events correctly. So, that SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS will help you with that case. If you sent SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS bigger than 0, sm_td will make a small pauses between sending events to OS.


- `SMTD_ENABLE_QMK_TAPHOLD` (default is 0)

  When set to 1, standard QMK `MT()` / `LT()` keycodes that are left unhandled by your `on_smtd_action` are processed by sm_td with its timing: tap sends the tap keycode, hold registers the mods / pushes the layer. This lets you keep plain `MT()` / `LT()` in your keymap without `SMTD_MT` / `SMTD_LT` macros. Advanced features like tap-count thresholds still require the macros.


- `SMTD_QMK_TAPHOLD_USE_CAPS_WORD` (default is true)

  Whether taps produced by `SMTD_ENABLE_QMK_TAPHOLD` handling respect Caps Word. Set to false to hide those taps from Caps Word.


You make redefine any of this global flags in your config.h.


Or you may want to redefine some flags for single keys.
You can override per-key feature flags by adding `bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature)` to your keymap.c:

```c
bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature) {
    switch (keycode) {
        case MACRO_KEY_CODE:
            if (feature == SMTD_FEATURE_AGGREGATE_TAPS) return false;
            break;
    }

    return smtd_feature_enabled_default(keycode, feature);
}
```

Note: `smtd_feature` currently includes `SMTD_FEATURE_AGGREGATE_TAPS` and `SMTD_FEATURE_PIPELINE_TAPS`. Simultaneous presses delay cannot be overridden per key.



Let's examine each of this features more closely.

## SMTD_FEATURE_AGGREGATE_TAPS

That emulates QMK's Tap Dance. If SMTD_FEATURE_AGGREGATE_TAPS = true, `on_smtd_action(CKC, SMTD_ACTION_TAP, XXX)` will be called once in the end of sequence of taps. So sm_td need a little pause after last tap to ensure, that sequence has finished, and sm_td may send final SMTD_ACTION_TAP event. See a table to see the difference:

```								 
                     | SMTD_FEATURE_AGGREGATE_TAPS = false         | SMTD_FEATURE_AGGREGATE_TAPS = true          |
                     |                                             |                                             |
0ms  - - - ┌—————┐ - | - - - - - - - - - - - - - - - - - - - - - - | - - - - - - - - - - - - - - - - - - - - - - |
           │macro│   | on_smtd_action(macro, SMTD_ACTION_TOUCH, 0) | on_smtd_action(macro, SMTD_ACTION_TOUCH, 0) |
           │ key │   |                                             |                                             |
           │     │   |                                             |                                             |
10ms - - - └—————┘ - | - - - - - - - - - - - - - - - - - - - - - - | - - - - - - - - - - - - - - - - - - - - - - |
                     | on_smtd_action(macro, SMTD_ACTION_TAP, 0)   |                                             |
                     |                                             |                                             |
20ms - - - ┌—————┐ - | - - - - - - - - - - - - - - - - - - - - - - | - - - - - - - - - - - - - - - - - - - - - - |
           │macro│   | on_smtd_action(macro, SMTD_ACTION_TOUCH, 1) | on_smtd_action(macro, SMTD_ACTION_TOUCH, 1) |
           │ key │   |                                             |                                             |
           │     │   |                                             |                                             |
30ms - - - └—————┘ - | - - - - - - - - - - - - - - - - - - - - - - | - - - - - - - - - - - - - - - - - - - - - - |
                     | on_smtd_action(macro, SMTD_ACTION_TAP, 1)   |                                             |
                     |                                             |                                             |
                     |                                             |                                             |
40ms - - - ┌—————┐ - | - - - - - - - - - - - - - - - - - - - - - - | - - - - - - - - - - - - - - - - - - - - - - |
           │macro│   | on_smtd_action(macro, SMTD_ACTION_TOUCH, 2) | on_smtd_action(macro, SMTD_ACTION_TOUCH, 2) |
           │ key │   |                                             |                                             |
           │     │   |                                             |                                             |
50ms - - - └—————┘ - | - - - - - - - - - - - - - - - - - - - - - - | - - - - - - - - - - - - - - - - - - - - - - |
                     | on_smtd_action(macro, SMTD_ACTION_TAP, 2)   |                                             |
                     |                                             |                                             |
                     |                                             |                                             |
50ms + SEQUENCE_TERM | - - - - - - - - - - - - - - - - - - - - - - | - - - - - - - - - - - - - - - - - - - - - - |
                     |                                             | on_smtd_action(macro, SMTD_ACTION_TAP, 2)   |
```

