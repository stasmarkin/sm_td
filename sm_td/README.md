# SM Tap Dance (sm_td)

This module adds human-friendly Tap Dance and Home Row Mod support to your QMK build.

1. Add the following to the list of modules in your `keymap.json` to enable this module:

```json
{
    "modules": ["stasmarkin/sm_td"]
}
```


2. Include `sm_td.h` and add an `on_smtd_action` handler in your keymap file:

```c
#include "sm_td.h"

// the rest of your code ... 

smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        SMTD_MT(KC_A, KC_LEFT_GUI)
        SMTD_MT(KC_S, KC_LEFT_ALT)
        SMTD_MT(KC_D, KC_LEFT_CTRL)
        SMTD_MT(KC_F, KC_LSFT)
    }

    return SMTD_RESOLUTION_UNHANDLED;
}
```



Build and flash your firmware. Your keys will now support advanced tap, hold, and multi-tap behaviors with human-friendly timing.

For more details and customization, see the [documentation](https://github.com/stasmarkin/sm_td/tree/main/docs).