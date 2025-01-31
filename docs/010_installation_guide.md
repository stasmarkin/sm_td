

1. Add `DEFERRED_EXEC_ENABLE = yes` to your `rules.mk` file.
2. Add `#define MAX_DEFERRED_EXECUTORS 10` (or add 10 if you already use it) to your `config.h` file.
3. Clone the `sm_td.h` repository into your `keymaps/your_keymap` folder (next to your `keymap.c`)
4. Add `#include "sm_td.h"` to your `keymap.c` file. !!! WARNING !!! There is a bug in v0.4.0 and the library would compile with a "'SMTD_KEYCODES_BEGIN' undeclared" error. You need to put this #include "sm_td.h" right after you define your custom keycodes enum (described on p.6).
5. Check `!process_smtd` first in your `process_record_user` function like this
   ```c
   bool process_record_user(uint16_t keycode, keyrecord_t *record) {
       if (!process_smtd(keycode, record)) {
           return false;
       }
   
       // your rest code here
       return true;
   }
   ```

6. Add `SMTD_KEYCODES_BEGIN` and `SMTD_KEYCODES_END` to you custom keycodes, and put each new custom keycode you want to use with `sm_td` between them.
   For example, if you want to use `A`, `S`, `D` and `F` for HRM, you need to create a custom keycode for them like this
   ```c
   enum custom_keycodes {
       SMTD_KEYCODES_BEGIN = SAFE_RANGE,
       CKC_A, // reads as C(ustom) + KC_A, but you may give any name here
       CKC_S,
       CKC_D,
       CKC_F,
       SMTD_KEYCODES_END,
   };
   ```
   Please don't forget to put ; at the end of the enum definition. Normally it's not necessary, but if you put #include "sm_td.h" it will break compilation.
   Note that `SAFE_RANGE` is a predefined constant in QMK, and is used [to define custom keycodes](https://docs.qmk.fm/custom_quantum_functions).
   You need to put it on the beginning of your custom keycodes enum.
   Some keyboards may have their own `SAFE_RANGE` constant, so you need to check your firmware for this constant.

7. Place all your custom keycodes on the desired key positions in your `keymaps`.
8. Create a `void on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count)` function that would handle all the actions of the custom keycodes you defined in the previous step.
   For example, if you want to use `CKC_A`, `CKC_S`, `CKC_D` and `CKC_F` for HRM, your `on_smtd_action()` function will look like this
   ```c
   void on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
       switch (keycode) {
           SMTD_MT(CKC_A, KC_A, KC_LEFT_GUI)
           SMTD_MT(CKC_S, KC_S, KC_LEFT_ALT)
           SMTD_MT(CKC_D, KC_D, KC_LEFT_CTRL)
           SMTD_MT(CKC_F, KC_F, KC_LSFT)
       }
   }
   ```
   See extensive documentation in the [Customization Guide](https://github.com/stasmarkin/sm_td/blob/main/docs/050_customization.md) with cool [Examples](https://github.com/stasmarkin/sm_td/blob/main/docs/60_customization_examples.md)

9. (optional) Add global configuration parameters to your `config.h` file (see [timeouts](https://github.com/stasmarkin/sm_td/blob/main/docs/070_customization_timeouts.md) and [feature flags](https://github.com/stasmarkin/sm_td/blob/main/docs/080_customization_features.md)).
10. (optional) Add per-key configuration (see [timeouts](https://github.com/stasmarkin/sm_td/blob/main/docs/070_customization_timeouts.md) and [feature flags](https://github.com/stasmarkin/sm_td/blob/main/docs/080_customization_features.md)).