```
This documentation is written for version 0.4.1.
It is a bit outdated for later versions of SM_TD.
```


There are global feature flags that may help you with customizing sm_td behavior:

- `SMTD_GLOBAL_AGGREGATE_TAPS` (default is false)

  Default behavior of sm_td library is to call tap action every time it's considered as a tap. This option allows to aggregate taps and call tap action only once after tap sequence is finished (same as original QMK Tap Dance).


- `SMTD_GLOBAL_MODS_RECALL` (default is true)

  Since tap action may be executed after a small delay (not immediately after key press), modifiers might be changed in that period. This option saves modifiers on key press and restores in on tap action.


- `SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS` (default is 0)

  One some stages sm_td may generate several events that would be sent immediately to OS. For example, by releasing following key sm_td may decide to unset modifier, send first key press, then set modifier and send following key press and release — everything one by one as soon as possible. In some cases corresponding keyboard driver or app may not register that events correctly. So, that SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS will help you with that case. If you sent SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS bigger than 0, sm_td will make a small pauses between sending events to OS.


You make redefine any of this global flags in your config.h.


Or you may want to redefine that flags for single keys.
And you may also override feature flags per key with adding `bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature)` function to your keymap.c:

```c
bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature);
    switch (keycode) {
        case MACRO_KEY_CODE:
            if (feature == SMTD_FEATURE_MODS_RECALL) return false;
    }

    return smtd_feature_enabled_default(feature);
}
```

Note, that smtd_feature type has only two possible values: SMTD_FEATURE_MODS_RECALL and SMTD_FEATURE_AGGREGATE_TAPS. You may not redefine simultaneous presses delay per key basis.



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

## SMTD_FEATURE_MODS_RECALL

Since sm_td send key press a bit later after real physical key press has occurred, that may lead to unexpected behaviour with mod keys. So, if you you hit `↓shift` (that is not a part of sm_td), `↓smtd_macro`, `↑shift` and `↑smtd_macro`. Actual tap of smtd_macro wouldn't be sent to OS until you physically release that key. So OS will receive that sequence: `↓shift`, `↑shift` (from physical pressing and releasing shift key), then `↓↑smtd_macro_tap_action` (after macro key release). The feature SMTD_FEATURE_MODS_RECALL (it is ON by default) is implemented just to make that macro taps more predictable while holding modifiers. Mods recall feature will re-register actual on physical pressing modifiers right before sending SMTD_ACTION_TAP. See a chart below for better explanation

```
                             | SMTD_FEATURE_MODS_RECALL = false          | SMTD_FEATURE_MODS_RECALL = true           |
                             |                                           |                                           |
0ms  - - - ┌—————┐ - - - - - | - - - - - - - - - - - - - - - - - - - - - | - - - - - - - - - - - - - - - - - - - - - |
           │shift│           | press(shift)                              | press(shift)                              |
           │ key │           |                                           |                                           |
           │     │           |                                           |                                           |
10ms - - - │ - - │ ┌—————┐ - | - - - - - - - - - - - - - - - - - - - - - | - - - - - - - - - - - - - - - - - - - - - |
           │     │ │macro│   |                                           |                                           |
           │     │ │ key │   |                                           |                                           |
           │     │ │     │   |                                           |                                           |
20ms - - - └—————┘ │ - - │ - | - - - - - - - - - - - - - - - - - - - - - | - - - - - - - - - - - - - - - - - - - - - |
                   │     │   | release(shift)                            | release(shift)                            |
                   │     │   |                                           |                                           |
30ms - - - - - - - └—————┘ - | - - - - - - - - - - - - - - - - - - - - - | - - - - - - - - - - - - - - - - - - - - - |
                             | on_smtd_action(macro, SMTD_ACTION_TAP, 0) | press(shift)                              |
                             |                                           | on_smtd_action(macro, SMTD_ACTION_TAP, 0) |
                             |                                           | release(shift)                            |
```

