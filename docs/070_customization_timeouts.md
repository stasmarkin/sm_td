There are 4 crucial timeouts for sm_td:

- `SMTD_TIMEOUT_TAP`

  This is the time in ms between macro key press and release to consider it as a tap action.
  So if you press and release a key within this time, it will be considered as a tap action. If you hold a key longer than this time, it will be considered as a hold action.


- `SMTD_TIMEOUT_FOLLOWING_TAP`

  This is the time in ms between following key press (second after macro key pressed) and release to consider macro is still might be a tap action.
  In other words, if you hold both macro and following keys longer than this time, macro key will be considered as a hold action.


- `SMTD_TIMEOUT_SEQUENCE`

  This is the time in ms between two macro taps to consider them as a sequence of taps.


- `SMTD_TIMEOUT_RELEASE`

  This is the time in ms to consider two keys released within that period as a hold action for the first key and a tap action for second.
  If two keys has bigger time between their releases, they will be considered as a tap action for both keys.


Each of them has coresponding default global value:
- `SMTD_GLOBAL_TAP_TERM` (default is TAPPING_TERM)
- `SMTD_GLOBAL_FOLLOWING_TAP_TERM` (default is TAPPING_TERM)
- `SMTD_GLOBAL_SEQUENCE_TERM` (default is TAPPING_TERM / 2)
- `SMTD_GLOBAL_RELEASE_TERM` (default is TAPPING_TERM / 4)


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
- if you notice, that in quick typing you sometimes get false hold interpretations, try to decrease SMTD_TIMEOUT_RELEASE.
- if you don't have enough time to make a tap sequence and it resets too early, try to increase SMTD_TIMEOUT_SEQUENCE
