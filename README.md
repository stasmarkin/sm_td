# sm_td
This is `SM Tap Dance` user library for QMK


## Introduction
This is a user library for QMK with custom implementations of Tap Dance functions. 
It offers smooth, fast and reliable tap dance functions for your keyboard.
Base functions are:
- better tap+tap vs hold+tap interpretation with two different keys
- better multi-tap and hold (and tap again then) interpretation of the same key
- more reactive response on multiple taps (and holds)

This library utilizes natural way of typing when you have a small overlap between the keys you are tapping.
For example, when you are fast typing `hi` you are not releasing `h` before pressing `i`, in other words, your finger movements are: `↓h` `↓i` `↑h` (substantial pause here) `↑i`.
On the other hand, if you put some hold action on `h` key (e.g. MOD_RSFT(KC_H)) you will have a change to miss a hold action, because it's natural in fast typing your fingers tends to release keys in same sequence as they were pressed: `↓h` `↓i` `↑h` (a tiny pause here) `↑i`.
This library takes this in consideration, and not trying to change your taping habits. 
Core concept of this library is to interpret hold action in different situations:
- when you are holding a key for a long time, it is a hold action (same as in QMK tap dance)
- when you are pressing and releasing a key while holding another key, it is a hold action for another key (also same as in QMK tap dance)
- when you release two keys almost simultaneously, it is a hold action for a key that was pressed first (this is the main difference from QMK tap dance)


## Roadmap
#### `v0.1.0`
- initial release and testing some basic functionality
#### `v0.2.0 `
- public beta test
- API is not stable yet, but it is usable
#### `v0.2.1` ← we are here
- rename `SMTD_ACTION_INIT` → `SMTD_ACTION_TOUCH`
- remove obsolete `SMTD_ACTION_INIT_UNDO` (use that action within `SMTD_ACTION_TAP` instead)
- better naming for timeout definitions (see Upgrade instructions)
- better naming for global definitions (see Upgrade instructions)
#### `v0.3.0` 
- comprehensive documentation
- reddit post
#### `v0.4.0` and further `v0.x`
- feature requests
- bug fixes
#### `v1.0.0`
- stable API
- debug utilities
- memory optimizations (on storing active states)
- memory optimizations (on state machine stack size)
- bunch of useful macros
- split into header and source files
#### `v1.1.0`
- better 3 finger roll interpretation


## Installation
1. Clone `sm_td.h` repository into your `keymaps/your_keymap` folder
2. Add `#include "sm_td.h"` to your `keymap.c` file
3. Check `!process_smtd` first in your `process_record_user` function like this
```c
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!process_smtd(keycode, record)) {
        return false;
    }
    // your code here
}
```
4. Add some custom keycodes to your `keymaps`, so sm_td library can use them without conflicts
5. Declare variable `smtd_states` your custom keycodes for sm_td library like this:
```c
smtd_state smtd_states[] = {
    SMTD(CUSTOM_KEYCODE_1),
    SMTD(CUSTOM_KEYCODE_2),
    // put all your custom keycodes here
}

// this is the size of your custom keycodes array, it is used for internal purposes. Do not delete this
size_t smtd_states_size = sizeof(smtd_states) / sizeof(smtd_states[0]);
```
6. Describe your custom keycodes in `on_smtd_action` function like this:
```c
void on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        case CKC_SPACE: {
            switch (CUSTOM_KEYCODE_1) {
                case SMTD_ACTION_TOUCH:
                    break; // touch action are not used in this example  
                case SMTD_ACTION_TAP:
                    // this is a tap action for CUSTOM_KEYCODE_1
                    tap_code(KC_SPACE);
                    break;
                case SMTD_ACTION_HOLD:
                    // this is a hold action for CUSTOM_KEYCODE_1
                    if (tap_count == 0 || tap_count == 1) {
                        layer_move(4);
                    } else {
                        // sending hold for OS
                        register_code(KC_SPACE); 
                    }
                    break;
                case SMTD_ACTION_RELEASE:
                    // this is a release action for CUSTOM_KEYCODE_1
                    if (tap_count == 0 || tap_count == 1) {
                        layer_move(0);
                    } else {
                        // releasing hold for OS
                        unregister_code(KC_SPACE);
                    }
                    break;
            }
            break;
        } // end of case CKC_SPACE
            
        // put all your custom keycodes here
        
    } // end of switch (keycode)
} // end of on_smtd_action function
```
7. (optional) Add global configuration 
  There are some global definitions you can use to change the behavior of sm_td library:

- `SMTD_GLOBAL_TAP_TERM` (default is TAPPING_TERM)

  This is the time in ms between macro key press and release to consider it as a tap action.
  So if you press and release a key within this time, it will be considered as a tap action. If you hold a key longer than this time, it will be considered as a hold action.


- `SMTD_GLOBAL_FOLLOWING_TAP_TERM` (default is TAPPING_TERM) 

  This is the time in ms between following key press (second after macro key pressed) and release to consider macro is still might be a tap action.
  In other words, if you hold both macro and following keys longer than this time, macro key will be considered as a hold action. 


- `SMTD_GLOBAL_SEQUENCE_TERM` (default is TAPPING_TERM / 2)

  This is the time in ms between two macro taps to consider them as a sequence of taps.


- `SMTD_GLOBAL_RELEASE_TERM` (default is 50)

  This is the time in ms to consider two keys released within that period as a hold action for the first key and a tap action for second.
  If two keys has bigger time between their releases, they will be considered as a tap action for both keys.


- `SMTD_GLOBAL_MODS_RECALL` (default is true) 

  Since tap action may be executed after a small delay (not immediately after key press), modifiers might be changed in that period. This option saves modifiers on key press and restores in on tap action.


- `SMTD_GLOBAL_AGGREGATE_TAPS` (default is false) 

  Default behavior of sm_td library is to call tap action every time it's considered as a tap. This option allows to aggregate taps and call tap action only once after all taps are finished (same as original QMK Tap Dance).


If you want to tweak some of these options, you can make such definitions in your `config.h` file, eg `#define SMTD_GLOBAL_RELEASE_TERM 75`. 

8. (optional) Add configuration per key #TODO


## Basic usage

Once you hit a key assigned to sm_td, all the state machines stack starts to work.
Other keys you press after running sm_td state machine will be also processed by sm_td state machine.
That state machine could decide to postpone processing of the key you pressed, so it will be considered as a tap or hold later. 
You don't need to worry about that, sm_td will process all the keys you pressed in the right order and a very predictable way.
You also don't worry about that state machines stack implementation, but you need to know what output you will get from sm_td state machine.
Once you press keys assigned to sm_td, it will be calling `on_smtd_action` function with the following arguments:
- uint16_t keycode - keycode of the key you pressed
- smtd_action action - result interpreted action (touch, tap, hold, release). tap, hold and release are self-explanatory. Touch action fired on key press (without knowing if it is a tap or hold).
- uint8_t tap_count - number of sequential taps before current action. (will reset after hold or any other key press)

There are only two execution flow for `on_smtd_action` function:
- touch → tap 
- touch → hold → release

For better understanding of the execution flow, please check the following example.
Let's say you want to tap, tap, hold, and tap again some custom key `CKC`. Here is your finger movements:

- `↓CKC` 50ms `↑CKC` (first tap finished) 50ms 
- `↓CKC` 50ms `↑CKC` (second tap finished) 50ms 
- `↓CKC` 200ms (holding long enough for hold action) `↑CKC` 50ms 
- `↓CKC` 50ms `↑CKC` (third tap finished)

For this example, you will get the following `on_smtd_action` calls:
- `on_smtd_action(CKC, SMTD_ACTION_TOUCH, 0)` right after pressing `↓CKC`
- `on_smtd_action(CKC, SMTD_ACTION_TAP, 0)` right after releasing `↑CKC` (first tap)
- `on_smtd_action(CKC, SMTD_ACTION_TOUCH, 1)` right after pressing `↓CKC` second time
- `on_smtd_action(CKC, SMTD_ACTION_TAP, 1)` right after releasing `↑CKC` second time (second tap)
- `on_smtd_action(CKC, SMTD_ACTION_TOUCH, 2)` right after pressing `↓CKC` third time
- `on_smtd_action(CKC, SMTD_ACTION_HOLD, 2)` after holding `CKC` long enough
- `on_smtd_action(CKC, SMTD_ACTION_RELEASE, 2)` right after releasing `↑CKC` (hold)
- `on_smtd_action(CKC, SMTD_ACTION_TOUCH, 0)` right after pressing `↓CKC` fourth time
- `on_smtd_action(CKC, SMTD_ACTION_TAP, 0)` right after releasing `↑CKC` (third tap)

## Upgrade instructions
#### `v0.2.0` → `v0.2.1`
- remove every SMTD_ACTION_INIT_UNDO. If you need any UNDO action, put in SMTD_ACTION_TAP instead
- rename `SMTD_ACTION_INIT` → `SMTD_ACTION_TOUCH`
- rename `SMTD_TIMEOUT_JOIN` → `SMTD_TIMEOUT_FOLLOWING_TAP`
- rename `SMTD_TIMEOUT_TAP` → `SMTD_TIMEOUT_SEQUENCE`
- rename `SMTD_TIMEOUT_INIT` → `SMTD_TIMEOUT_TAP`
- rename `SMTD_INIT_TERM` → `SMTD_GLOBAL_TAP_TERM`
- rename `SMTD_TAP_TERM` → `SMTD_GLOBAL_SEQUENCE_TERM`
- rename `SMTD_JOIN_TERM` → `SMTD_GLOBAL_FOLLOWING_TAP_TERM`
- rename `SMTD_RELEASE_TERM` → `SMTD_GLOBAL_RELEASE_TERM`