## Partial Combo support

`COMBO_ACTION` + `process_combo_event()` is only a supported API for QMK's combo feature.
See [official documentation](https://docs.qmk.fm/features/combo#examples)

Simple `COMBO()` are not supported yet


## Twice quantum key processing before users code

Then you press a key there are many layers involved in handling that press:
- quantum's process_record()
- keyboard's process_record_keyboard()
- users process_record_user() (the one we write in keymap.c)
- post user processing in keyboard layer
- post keyboard processing in quantum
- final decision to send a key press to OS

The problem is that sm_td suspends key handling execution in users layer and later emulates key pressing or releasing. The library uses quantum process_record() function to generate key presses and releases. So with single tap there will be such situation:

- user press a key
- quantum layer runs process_record() for press
- keyboard layer runs process_record_keyboard() for press
- user layer runs process_record_user() for press
- sm_td blocks further execution and nothing is sent to OS

- user release a key
- quantum layer runs process_record() for release
- keyboard layer runs process_record_keyboard() for release
- user layer runs process_record_user() for release
- sm_td interpret that as a tap, so it reruns process_record for press
  - quantum layer runs process_record() for press
  - keyboard layer runs process_record_keyboard() for press
  - user layer runs process_record_user() for press
  - sm_td passes exection since it recognized that as a tap
  - post user processing in keyboard layer for press
  - post keyboard processing in quantum for press
  - send a key press to OS
- sm_td passes release
- post user processing in keyboard layer for release
- post keyboard processing in quantum for release
- send a key release to OS

so as you see there is extra execution for process_record() and process_record_keyboard(). It's not a big deal if you don't use sm_td keys for anything else, I didn't notice any visible consequences of that behavior.

Since v0.5.6 this re-run through process_record() is actually a feature: it is how sm_td makes its taps visible to core QMK libraries (Caps Word, Auto Shift, Key Overrides, etc.), see `SMTD_GLOBAL_PIPELINE_TAPS` in [feature flags](https://github.com/stasmarkin/sm_td/blob/main/docs/080_customization_features.md). If the double processing causes side effects for a specific key, you can disable the pipeline for that key via `SMTD_FEATURE_PIPELINE_TAPS`. And if you find any bugs here, please create an issue on github


## Leader key support

QMK's Leader feature (`process_leader`) runs in the quantum chain *after*
`process_record_user`, so it only ever sees a tap that sm_td routes back through
`process_record()`. For a key whose keymap position resolves to the tap keycode
itself (the recommended `SMTD_MT(KC_A, MOD)` form), the pipeline taps above do
exactly that, so leader sequences work out of the box.

A tap on a custom/derived keycode (e.g. `SMTD_MT_ON_MKEY(CUSTOM_A, KC_A, MOD)`)
cannot take the pipeline path, so sm_td used to send it directly to the host and
the leader never saw it (issue #29). Such taps are now fed into the leader buffer
the same way `process_leader` would, so leader sequences work for custom keycodes
too.

Caveat: only the *tap* is captured. Holding a custom mod-tap key during a leader
sequence is not added to the buffer, since leader is a tap-sequence feature.
