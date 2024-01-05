# sm_td
This is `SM Tap Dance` user library for QMK


## Introduction
This is a user library for QMK with custom implementations of Tap Dance functions with reliable tap+tap vs tap+hold decision mechanism. 
It offers smooth and fast tap dance multi-tap and hold interpretation with a very predictable behavior.
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
- when you press third key while pressing other two. The first key in that sequence will be interpreted as being held.

Please, see [wiki](https://github.com/stasmarkin/sm_td/wiki) for comprehensive documentation.

## Roadmap
#### `v0.1.0`
- initial release and testing some basic functionality
#### `v0.2.0`
- public beta test
- API is not stable yet, but it is usable
#### `v0.2.1`
- rename `SMTD_ACTION_INIT` → `SMTD_ACTION_TOUCH`
- remove obsolete `SMTD_ACTION_INIT_UNDO` (use that action within `SMTD_ACTION_TAP` instead)
- better naming for timeout definitions (see Upgrade instructions)
- better naming for global definitions (see Upgrade instructions)
#### `v0.3.0` ← we are here 
- bug fix on pressing same macro key on stage SMTD_STAGE_RELEASE 
- reduce args number for get_smtd_timeout_default and smtd_feature_enabled_default functions (see Upgrade instructions)
- better stage naming 
- comprehensive documentation
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

See [upgrade instructions](https://github.com/stasmarkin/sm_td/wiki/1.1:-Upgrade-instructions) if already using sm_td library.

## Installation
1. Clone `sm_td.h` repository into your `keymaps/your_keymap` folder (next to your keymap.c)
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
5. Declare variable `smtd_states` your custom keycodes and variable `smtd_states_size` for sm_td library like this:
```c
smtd_state smtd_states[] = {
    SMTD(CUSTOM_KEYCODE_1),
    SMTD(CUSTOM_KEYCODE_2),
    // put all your custom keycodes here
}

// this is the size of your custom keycodes array, it is used for internal purposes. Do not delete this
size_t smtd_states_size = sizeof(smtd_states) / sizeof(smtd_states[0]);
```
6. Describe your custom keycodes in `on_smtd_action` function. 
   See comprehensive documentation in [Customization guide](https://github.com/stasmarkin/sm_td/wiki/2.0:-Customization-guide) with cool [examples](https://github.com/stasmarkin/sm_td/wiki/2.1:-Customization-guide:-Examples)

7. (optional) Add global configuration parameters in your `config.h` file (see [timeouts](https://github.com/stasmarkin/sm_td/wiki/2.2:-Customization-guide:-Timeouts-per-key) and [feature flags](https://github.com/stasmarkin/sm_td/wiki/2.3:-Customization-guide:-Feature-flags))
8. (optional) Add configuration per key (see [timeouts](https://github.com/stasmarkin/sm_td/wiki/2.2:-Customization-guide:-Timeouts-per-key) and [feature flags](https://github.com/stasmarkin/sm_td/wiki/2.3:-Customization-guide:-Feature-flags))


## Basic usage

Once you hit a key assigned to sm_td, all the state machines stack starts to work.
Other keys you press after running sm_td state machine will be also processed by sm_td state machine.
That state machine could decide to postpone processing of the key you pressed, so it will be considered as a tap or hold later. 
You don't need to worry about that, sm_td will process all the keys you pressed in the right order and a very predictable way.
You also don't worry about that state machines stack implementation, but you need to know what output you will get from sm_td state machine.
Once you press keys assigned to sm_td, it will be calling `on_smtd_action` function with the following arguments:
- uint16_t keycode - keycode of the key you pressed
- smtd_action action - result interpreted action (`SMTD_ACTION_TOUCH`, `SMTD_ACTION_TAP`, `SMTD_ACTION_HOLD`, `SMTD_ACTION_RELEASE`). tap, hold and release are self-explanatory. Touch action fired on key press (without knowing if it is going to be a tap or hold).
- uint8_t tap_count - number of sequential taps before current action. (will reset after hold, pause or any other key press)

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

For deeper understanding of the execution flow, please check [state machine description](https://github.com/stasmarkin/sm_td/wiki/3.0:-Deep-explanation:-Stages) and further [one key explanation](https://github.com/stasmarkin/sm_td/wiki/3.1:-Deep-explanation:-One-key-stages), [two keys explanation](https://github.com/stasmarkin/sm_td/wiki/3.2:-Deep-explanation:-Two-keys-stages) and [state machine stack](https://github.com/stasmarkin/sm_td/wiki/3.3:-Deep-explanation:-Three-keys-and-states-stack).
