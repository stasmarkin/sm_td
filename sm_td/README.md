# SM Tap Dance (sm_td)

This module adds human-friendly Tap Dance and Home Row Mod support to your QMK build.

Add the following to the list of modules in your `keymap.json` to enable this module:

```json
{
    "modules": ["stasmarkin/sm_td"]
}
```

In your keymap, add an `on_smtd_action` handler:

```c
void on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    // Handle tap, hold, and release actions for your custom keycodes here
}
```

Define your custom keycodes between `SMTD_KEYCODES_BEGIN` and `SMTD_KEYCODES_END`:

```c
enum custom_keycodes {
    SMTD_KEYCODES_BEGIN = SAFE_RANGE,
    CKC_A,
    CKC_S,
    CKC_D,
    CKC_F,
    SMTD_KEYCODES_END,
};
```

Include `sm_td.h` in your keymap file:

```c
#include "sm_td.h"
```

In your `process_record_user` function, call `process_smtd`:

```c
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!process_smtd(keycode, record)) {
        return false;
    }
    // Your code here
    return true;
}
```

Build and flash your firmware. Your custom keys will now support advanced tap, hold, and multi-tap behaviors with human-friendly timing.

For more details and customization,