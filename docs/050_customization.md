```
This documentation is written for version 0.4.1.
It is a bit outdated for later versions of SM_TD.
```


## What is `on_smtd_action()` function?

When you press a key, all state machines in the stack start working.
Other keys you press after the sm_td state machine has run will also be processed by the sm_td state machine.
This state machine might decide to postpone the processing of the key you pressed, so that it will be considered a tap or hold later.
You don't have to worry about this, sm_td will process all keys you press in the correct order and in a very predictable way.
You don't need to understand the internal implementation of the state machine stack, but you do need to know what output you will get from sm_td's state machine.
As soon as you press keys assigned to sm_td, it will call the `on_smtd_action()` function with the following arguments
- uint16_t keycode - keycode of the key you pressed
- smtd_action action - result interpreted action (`SMTD_ACTION_TOUCH`, `SMTD_ACTION_TAP`, `SMTD_ACTION_HOLD`, `SMTD_ACTION_RELEASE`). tap, hold and release are self-explanatory. Touch action fired on key press (without knowing if it is a tap or hold).
- uint8_t tap_count - number of consecutive taps before the current action. (will be reset after hold, pause or any other keypress)

There are only two execution flows for the `on_smtd_action` function:
- `SMTD_ACTION_TOUCH` → `SMTD_ACTION_TAP`.
- `SMTD_ACTION_TOUCH` → `SMTD_ACTION_HOLD` → `SMTD_ACTION_RELEASE`.

Consider the following example to understand the execution flow.
Let's say you want to tap, tap, hold and tap again a custom key `KC`. Here are your finger movements:

- `↓KC` 50ms `↑KC` (first tap finished) 50ms
- `↓KC` 50ms `↑KC` (second tap finished) 50ms
- `↓KC` 200ms (holding long enough for hold action) `↑KC` 50ms
- `↓KC` 50ms `↑KC` (third tap finished)

For this example, you will get the following `on_smtd_action()` calls:
- `on_smtd_action(KC, SMTD_ACTION_TOUCH, 0)` right after pressing `↓KC`
- `on_smtd_action(KC, SMTD_ACTION_TAP, 0)` right after releasing `↑KC` (first tap finished)
- `on_smtd_action(KC, SMTD_ACTION_TOUCH, 1)` right after pressing `↓KC` second time
- `on_smtd_action(KC, SMTD_ACTION_TAP, 1)` right after releasing `↑KC` second time (second tap finished)
- `on_smtd_action(KC, SMTD_ACTION_TOUCH, 2)` right after pressing `↓KC` third time
- `on_smtd_action(KC, SMTD_ACTION_HOLD, 2)` after holding `KC` long enough
- `on_smtd_action(KC, SMTD_ACTION_RELEASE, 2)` right after releasing `↑KC` (hold)
- `on_smtd_action(KC, SMTD_ACTION_TOUCH, 0)` right after pressing `↓KC` fourth time
- `on_smtd_action(KC, SMTD_ACTION_TAP, 0)` right after releasing `↑KC` (third tap finished)



## How to add custom behavior to sm_td keycodes?

You need to put all the handlers of your sm_td keycodes into the `on_smtd_action()` function. It's called with the following arguments
- `uint16_t keycode` - keycode of the macro key you pressed
- `smtd_action action` - result interpreted action (SMTD_ACTION_TOUCH, SMTD_ACTION_TAP, SMTD_ACTION_HOLD, SMTD_ACTION_RELEASE). tap, hold and release are self-explanatory. The touch action is fired on every key press (without knowing if it is a tap or a hold).
- `uint8_t tap_count` - Number of consecutive taps before the current action. (will be reset after hold, pause or any other keypress)

### `smtd_resolution` return value

The function should return a `smtd_resolution` value to indicate how the action was handled:
- `SMTD_RESOLUTION_UNHANDLED` - The action was not handled by some custom code. `sm_td` will handle it as normal key action
- `SMTD_RESOLUTION_UNCERTAIN` - The action is not yet determined (for example, tapping MT key may result in mod or key, so on tap resolution is uncertain)
- `SMTD_RESOLUTION_DETERMINED` - The action was handled and determined

The rule of thumb is to return `SMTD_RESOLUTION_UNCERTAIN` on `SMTD_ACTION_TOUCH` and `SMTD_RESOLUTION_DETERMINED` on all other actions.
This will be enough for 99.8% of cases. In the other 0.2% contact me, I'm really curious what are you trying to achieve.

### `on_smtd_action()` execution flow

There are only two execution flows for the `on_smtd_action` function:
- `SMTD_ACTION_TOUCH` → `SMTD_ACTION_TAP`.
- `SMTD_ACTION_TOUCH` → `SMTD_ACTION_HOLD` → `SMTD_ACTION_RELEASE`.

You will never get a tap between hold and release or miss a touch action. So you can be sure that once a touch is executed, it will be finished with a tap or hold+release.

One possible structure to describe macro behavior is nested switch-cases. Top level for macro key selection, second for action, third (optional) for tap_count. For example


--- EVERYTHING BELOW IS A BIT OUTDATED ---

```c
smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
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
            return SMTD_RESOLUTION_DETERMINED;
        } // end of case CUSTOM_KEYCODE_1
            
        // put all your custom keycodes here
        
    } // end of switch (keycode)
    return SMTD_RESOLUTION_UNHANDLED;
} // end of on_smtd_action function
```

There are special macros to make your code more compact and easier:
- `SMTD_MT()`
- `SMTD_MTE()`
- `SMTD_LT()`

They handle custom keycodes in the same way as the standard QMK `MT()` or `LT()` functions.
So instead of writing a multiline handler for each sm_td keycode, you can write something like this

```c
smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        SMTD_MT(CKC_A, KC_A, KC_LEFT_GUI)
        SMTD_MT(CKC_S, KC_S, KC_LEFT_ALT)
        SMTD_MT(CKC_D, KC_D, KC_LEFT_CTRL)
        SMTD_MT(CKC_F, KC_F, KC_LSFT)

        SMTD_LT(CKC_SPACE, KC_SPACE, LAYER_NUM)
        SMTD_LT(CKC_ENTER, KC_ENTER, LAYER_FN)
    } // End of switch (keycode)
    
    return SMTD_RESOLUTION_UNHANDLED;
} // End of on_smtd_action function
```

See full documentation for this macro in the next chapter.