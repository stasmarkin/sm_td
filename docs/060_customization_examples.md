```
This documentation is written for version 0.4.1.
It is a bit outdated for later versions of SM_TD.
```


## Emulate LT(LAYER, KEY)
There is `SMTD_LT` macro for that. You can use it in 3 different forms:
* `SMTD_LT(macro_key, tap_key, layer)`
* `SMTD_LT(macro_key, tap_key, layer, threshold)`
* `SMTD_LT(macro_key, tap_key, layer, threshold, use_cl)`

Parameters are:

- `macro_key` - smtd keycode that would be monitored
- `tap_key` - the key that would be pressed if the sm_td library detects a key tap
- `layer` - number of the layer to move when the sm_td library detects a key hold
- `threshold` - number of consecutive taps to hold `tap_key` instead of moving to the layer. For example, with `SMTD_LT(CKC_SPACE, KC_SPACE, SOME_LAYER, 2)` you can tap CKC_SPACE twice and then hold it, and OS will get double KC_SPACE and hold KC_SPACE.
- `use_cl' - a boolean that indicates whether you want to follow the caps_word settings or not. Since sm_td pauses and emulates keystrokes, it's not very friendly with other QMK features. So it has to manually check if caps_word is enabled and implicitly init shift modifiers. In some cases, you may not need to press shift even if caps_word is on (e.g. for numbers). So you should set this to `false` for such cases.

With all that you can easily emulate `LT(LAYER, KEY)` like that:
```c
smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        SMTD_LT(CKC_SPACE, KC_SPACE, LAYER_NUM)
        SMTD_LT(CKC_ENTER, KC_ENTER, LAYER_SYMBOLS, 2)
        SMTD_LT(CKC_1, KC_1, LAYER_SYS, 2, false)
    } // end of switch (keycode)
    
    return SMTD_RESOLUTION_UNHANDLED;
} // end of on_smtd_action function
```


Or you can manually write it as a custom handler
```c
smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        case emulate_lt_macro_key: {
            switch (action) {
                case SMTD_ACTION_TOUCH:
                    break;

                case SMTD_ACTION_TAP:
                    tap_code(KEY);
                    break;
                    
                case SMTD_ACTION_HOLD:
                    switch (tap_count) {
                        case 0:
                        case 1:
                            layer_move(LAYER);
                            break;
                        default:
                            register_code(KEY); 
                            break;
                    }
                    break;
                    
                case SMTD_ACTION_RELEASE:
                    switch (tap_count) {
                        case 0:
                        case 1:
                            layer_move(0);
                            break;
                        default:
                            unregister_code(KEY);
                            break;
                    }
                    break;
            } // end of switch (action)
            return SMTD_RESOLUTION_DETERMINED;
        }
                    
    } // end of switch (keycode)
    return SMTD_RESOLUTION_UNHANDLED;
} // end of on_smtd_action function
```

So, if sm_td register a tap, it will send KEY tap.
If sm_td registers a hold, it will switch to LAYER.
But if it's a hold after two sequential taps, it will send KEY press and will be pressing until a macro key will be physically released.




## Emulate MT(MOD, KEY)
There is `SMTD_MT` macro for that. You can use it in 3 different forms:
- `SMTD_MT(macro_key, tap_key, mod)`
- `SMTD_MT(macro_key, tap_key, mod, threshold)`
- `SMTD_MT(macro_key, tap_key, mod, threshold, use_cl)`

Parameters are:

- `macro_key` - smtd keycode that would be monitored
- `tap_key` - the key that would be pressed if the sm_td library detects a key tap
- `mod` - modifier to be pressed if the sm_td library detects a key hold
- `threshold` - number of consecutive taps to hold `tap_key` instead of holding a mod. For example, with `SMTD_MTE(CKC_A, KC_A, KC_LEFT_GUI, 2)` you can tap CKC_A twice and then hold it, and OS will get double KC_A and hold KC_A (instead of holding KC_LEFT_GUI).
- `use_cl' - a boolean that indicates whether you want to follow the caps_word settings or not. Since sm_td pauses and emulates keystrokes, it's not very friendly with other QMK features. So it has to manually check if caps_word is enabled and implicitly init shift modifiers. In some cases, you may not need to press shift even if caps_word is on (e.g. for numbers). So you should set this to `false` for such cases.

There is also a `SMTD_MTE` macro in the same 3 forms. The only difference is `E` - eager.
So when you press a macro key, the modifier is immediately turned on. But if you release a key fast enough to be interpreted as a tap, the modifier is turned off and a normal tap is sent to the OS.

With all that you can easily emulate `MT(MOD, KEY)` like that:
```c
smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        SMTD_MT(CKC_A, KC_A, KC_LEFT_GUI)
        SMTD_MT(CKC_S, KC_S, KC_LEFT_ALT, 3)
        SMTD_MTE(CKC_D, KC_D, KC_LEFT_CTRL, 3, false)
        SMTD_MTE(CKC_F, KC_F, KC_LEFT_SHIFT)
    } // end of switch (keycode)
    
    return SMTD_RESOLUTION_UNHANDLED;
} // end of on_smtd_action function
```


Or you can manually write it as a custom handler
```c
smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        case emulate_mt_macro_key: {                                           
            switch (action) {                                     
                case SMTD_ACTION_TOUCH:                           
                    break;    
                                                        
                case SMTD_ACTION_TAP:                             
                    tap_code16(KEY);                          
                    break;

                case SMTD_ACTION_HOLD:
                    switch (tap_count) {
                        case 0:
                        case 1:
                            register_mods(MOD_BIT(MOD)); 
                            break;
                        default:
                            register_code16(KEY);                 
                            break;
                    }                            
                    break;

                case SMTD_ACTION_RELEASE:                         
                    switch (tap_count) {
                        case 0:
                        case 1:
                            unregister_mods(MOD_BIT(MOD));
                            break;
                        default:
                            unregister_code16(KEY);
                            break;
                    }             
                    break;                                        
            } // end of switch (action)
            return SMTD_RESOLUTION_DETERMINED;
        } // end of case emulate_mt_macro_key
    } // end of switch (keycode)
    return SMTD_RESOLUTION_UNHANDLED;
} // end of on_smtd_action function
```



## Responsive MT(MOD, KEY)

Previous paragraph describes `SMTD_MTE` macro for that.
But you can implement that manually:
```c
smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        case resposive_mt_macro_key: {                                           
            switch (action) {                                     
                case SMTD_ACTION_TOUCH:                           
                    switch (tap_count) {
                        case 0:
                        case 1:
                            register_mods(MOD_BIT(MOD)); 
                            break;
                        default:
                            register_code16(KEY);
                            break;
                    }
                    break;    

                case SMTD_ACTION_TAP: 
                    unregister_mods(MOD_BIT(MOD));
                    tap_code16(KEY);           
                    break;

                case SMTD_ACTION_HOLD:
                    // no need to register anything since it was already registered in SMTD_ACTION_TOUCH
                    break;

                case SMTD_ACTION_RELEASE:                         
                    switch (tap_count) {
                        case 0:
                        case 1:
                            unregister_mods(MOD_BIT(MOD));
                            break;
                        default:
                            unregister_code16(KEY);
                            break;
                    }             
                    break;                                        
            } // end of switch (action)
            return SMTD_RESOLUTION_DETERMINED;
        } // end of case resposive_mt_macro_key
    } // end of switch (keycode)
    return SMTD_RESOLUTION_UNHANDLED;
} // end of on_smtd_action function
```

In this case MOD is registered as soon as key was pressed. That might be handy, if you MOD with mouse click, so you don't need before keyboard will interpret that key is held.
So, on touch action sm_td will register MOD, and it should be unregistered on tap action.



## Responsive symbol change on sequential tapping

For example, you to alternate between `:`, `;` and `#` on each key press.
```c
smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        case resposive_seq_macro_key: {                                           
            switch (action) {                                     
                case SMTD_ACTION_TOUCH: 
                    if (tap_count > 0) {
                        tap_code16(KC_BSPACE);
                    }                         

                    switch (tap_count % 3) {
                        case 0: register_code16(KC_COLON); break;
                        case 1: register_code16(KC_SEMICOLON); break;
                        case 2: register_code16(KC_HASH); break;
                        default: break;
                    }
                    break;    
            } // end of switch (action)
            return SMTD_RESOLUTION_DETERMINED;
        } // end of case resposive_seq_macro_key
    } // end of switch (keycode)
    return SMTD_RESOLUTION_UNHANDLED;
} // end of on_smtd_action function
```

So, the first press will send `:` and sequential presses will delete just entered symbol and send next


## Clumsy symbol pick after sequential tapping (same as QMK Tap Dance)

```c
smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        case clumsy_seq_macro_key: {                                           
            switch (action) {                                     
                case SMTD_ACTION_TAP: 
                    switch (tap_count % 3) {
                        case 0: register_code16(KC_COLON); break;
                        case 1: register_code16(KC_SEMICOLON); break;
                        case 2: register_code16(KC_HASH); break;
                        default: break;
                    }
                    break;
            } // end of switch (action)
            return SMTD_RESOLUTION_DETERMINED;
        } // end of case clumsy_seq_macro_key
    } // end of switch (keycode)
    return SMTD_RESOLUTION_UNHANDLED;
} // end of on_smtd_action function

bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature) {
    if (keycode == clumsy_seq_macro_key && feature == SMTD_FEATURE_AGGREGATE_TAPS) return true;
    
    return smtd_feature_enabled_default(keycode, feature); 
}
```
In this case SMTD_ACTION_TAP wouldn't be sent until sm_td is 100% sure that you have finished your tap sequence. So SMTD_ACTION_TAP will be executed exactly once for all sequence.