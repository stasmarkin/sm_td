```
This documentation is up to date for version 0.6.0.
```


There are 3 crucial timeouts for sm_td:

- `SMTD_TIMEOUT_TAP`

  This is the time in ms between macro key press and release to consider it as a tap action.
  So if you press and release a key within this time, it will be considered as a tap action. If you hold a key longer than this time, it will be considered as a hold action.


- `SMTD_TIMEOUT_SEQUENCE`

  This is the time in ms between two macro taps to consider them as a sequence of taps.


- `SMTD_TIMEOUT_RELEASE`

  This is the time in ms to consider two keys released within that period as a hold action for the first key and a tap action for second.
  If two keys has bigger time between their releases, they will be considered as a tap action for both keys.

  Since 0.5.7 this timeout is dynamic by default: it is derived from your actual typing rhythm on every keystroke.
  For an overlapping sequence `↓A ↓B ↑A ↑B` the decision window for `↑B` is computed as
  `min(p1, p2) / SMTD_GLOBAL_RELEASE_RATIO`, where `p1` is the pause between `↓A` and `↓B`,
  and `p2` is the pause between `↓B` and `↑A`. In other words, `↓A` is interpreted as a hold
  only when both keys are released much faster than they were pressed — quick rolls
  naturally shrink the window, slow deliberate typing widens it.
  `SMTD_TIMEOUT_RELEASE` still acts as the upper bound for the computed window
  (per-key too, via `get_smtd_timeout`), so the dynamic window may only shrink it.


Each of them has coresponding default global value:
- `SMTD_GLOBAL_TAP_TERM` (default is TAPPING_TERM)
- `SMTD_GLOBAL_SEQUENCE_TERM` (default is TAPPING_TERM / 2)
- `SMTD_GLOBAL_RELEASE_TERM` (default is TAPPING_TERM / 4)
- `SMTD_GLOBAL_RELEASE_RATIO` (default is 5; set to 0 to disable the dynamic release window and use the fixed `SMTD_GLOBAL_RELEASE_TERM` as before)


You may override that global terms in your `config.h` file, eg `#define SMTD_GLOBAL_RELEASE_TERM 75`.

There is also a handy function `uint32_t get_smtd_timeout_default(smtd_timeout timeout)` to get current default value for given smtd_timeout.

And you may override timeouts for a single key with function `uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout)`

Just define it in your `keymap.c` like this:

```c
uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
    switch (keycode) {
        case MACRO_KEY_CODE:
            if (timeout == SMTD_TIMEOUT_TAP) return 300;
    }

    return get_smtd_timeout_default(timeout);
}
```

Main advices for tweaking timeouts:
- if you have a weak finger, that gets stuck on a key press, so it counts as being held, try to increase SMTD_TIMEOUT_TAP.
- if you notice, that in quick typing you sometimes get false hold interpretations, try to increase SMTD_GLOBAL_RELEASE_RATIO (or decrease SMTD_TIMEOUT_RELEASE).
- if you get false tap interpretations instead of holds, try to decrease SMTD_GLOBAL_RELEASE_RATIO.
- if you don't have enough time to make a tap sequence and it resets too early, try to increase SMTD_TIMEOUT_SEQUENCE
