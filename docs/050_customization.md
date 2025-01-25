You need to put all the handlers of your sm_td keycodes into the `on_smtd_action()` function. It's called with the following arguments
- `uint16_t keycode` - keycode of the macro key you pressed
- smtd_action action` - result interpreted action (SMTD_ACTION_TOUCH, SMTD_ACTION_TAP, SMTD_ACTION_HOLD, SMTD_ACTION_RELEASE). tap, hold and release are self-explanatory. The touch action is fired on every key press (without knowing if it is a tap or a hold).
- uint8_t tap_count` - Number of consecutive taps before the current action. (will be reset after hold, pause or any other keypress)

There are only two execution flows for the `on_smtd_action` function:
- `SMTD_ACTION_TOUCH` → `SMTD_ACTION_TAP`.
- `SMTD_ACTION_TOUCH` → `SMTD_ACTION_HOLD` → `SMTD_ACTION_RELEASE`.

You will never get a tap between hold and release or miss a touch action. So you can be sure that once a touch is executed, it will be finished with a tap or hold+release.

One possible structure to describe macro behavior is nested switch-cases. Top level for macro key selection, second for action, third (optional) for tap_count. For example

```c
void on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        case CUSTOM_KEYCODE_1: {
            switch (action) {
                case SMTD_ACTION_TOUCH:
                    break; // touch action are not used in this example  
                    
                case SMTD_ACTION_TAP: 
                    // this is a tap action for CUSTOM_KEYCODE_1
                    tap_code(KC_SPACE);
                    break;
                    
                case SMTD_ACTION_HOLD:
                    // this is a hold action for CUSTOM_KEYCODE_1
                    switch (tap_count) {
                        case 0:
                        case 1:
                            layer_move(4);
                            break;
                        default:
                            // sending hold for OS
                            register_code(KC_SPACE); 
                            break;
                    }
                    break;
                    
                case SMTD_ACTION_RELEASE:
                    // this is a release action for CUSTOM_KEYCODE_1
                    switch (tap_count) {
                        case 0:
                        case 1:
                            layer_move(0);
                            break;
                        default:
                            // releasing hold for OS
                            unregister_code(KC_SPACE);
                            break;
                    }
                    break;
            } // end of switch (action)
            break;
        } // end of case CUSTOM_KEYCODE_1
            
        // put all your custom keycodes here
        
    } // end of switch (keycode)
} // end of on_smtd_action function
```

There are special macros to make your code more compact and easier:
- `SMTD_MT()`
- `SMTD_MTE()`
- `SMTD_LT()`

They handle custom keycodes in the same way as the standard QMK `MT()` or `LT()` functions.
So instead of writing a multiline handler for each sm_td keycode, you can write something like this

```
void on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) { }
    switch (keycode) {
        SMTD_MT(CKC_A, KC_A, KC_LEFT_GUI)
        SMTD_MT(CKC_S, KC_S, KC_LEFT_ALT)
        SMTD_MT(CKC_D, KC_D, KC_LEFT_CTRL)
        SMTD_MT(CKC_F, KC_F, KC_LSFT)

        SMTD_LT(CKC_SPACE, KC_SPACE, LAYER_NUM)
        SMTD_LT(CKC_ENTER, KC_ENTER, LAYER_FN)
    } // End of switch (keycode)
} // End of on_smtd_action function
```

See full documentation for this macro in the next chapter.