# sm_td

## Introduction

This is the `SM Tap Dance` (`sm_td` or `smtd` for short) user library for QMK.
The main goal of this library is to ultimately fix the Home Row Mods (HRM) and Tap Dance problems in QMK.
The basic features of this library are:
- Human-friendly tap+tap vs. hold+tap interpretation. Especially useful for `LT()` and `MT()` macros.
- Better multi-tap and hold (and then tap again) interpretation of the same key
- Reactive response to multiple taps (and holds)

This library uses the natural way of human typing when we have a small overlap between key taps.
For example, when a person types `hi` quickly, he does not release `h` before pressing `i`, in other words, the finger movements are: `↓h` `↓i` `↑h` `↑i`.
The main problem with QMK tap dance is that it does not consider this natural way of typing and tries to interpret all keys pressed and released in the straight order.
So in the example above, if you put a tap-hold action on the `h` key (e.g. `LT(1, KC_H)`), QMK will interpret it as `layer_move(1)` + `tap(KC_I)`.

There are many other ways to fix this problem in QMK, but all of them are not perfect and require some changes in your typing habits.
The core principle of this library is respecting human typing habits and not try to change them.
The main idea is to pay attention to the time between key releases (instead of key presses) and interpret them in a more human-friendly way.
So, `↓h` `↓i` `↑h` (tiny pause) `↑i` will be interpreted as `layer_move(1)` + `tap(KC_I)` because as humans we release combo keys almost simultaneously.
On the other hand, `↓h` `↓i` `↑h` (long pause) `↑i` will be interpreted as `tap(KC_H)` + `tap(KC_I)` because as humans we release sequential keys with a long pause in between.

Please see the [wiki](https://github.com/stasmarkin/sm_td/wiki) for extensive documentation.

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
#### `v0.3.0` 
- bug fix on pressing same macro key on stage SMTD_STAGE_RELEASE 
- reduce args number for get_smtd_timeout_default and smtd_feature_enabled_default functions (see Upgrade instructions)
- better stage naming 
- comprehensive documentation
#### `v0.3.1`
- optional delay between simultaneous key presses (see SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS in [feature flags](https://github.com/stasmarkin/sm_td/wiki/2.3:-Customization-guide:-Feature-flags)) 
#### `v0.4.0` ← we are here
- simplified installation process (no need to init each key with `SMTD()` macro)
- added useful `SMTD_MT()`, `SMTD_MTE()`, `SMTD_LT()` macros for easier customization
- added debugging utilities (see [Debugging Guide](https://github.com/stasmarkin/sm_td/wiki/1.3:-Debugging-guide))
- fixed several bugs (especially with sticky modifiers)
- made some memory optimizations (for storing active states)
#### `v0.4.1`
- fix 'SMTD_KEYCODES_BEGIN' undeclared error on compilation
#### `v0.5.0` and further `v0.x`
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
1. Add `DEFERRED_EXEC_ENABLE = yes` to your `rules.mk` file.
2. Add `#define MAX_DEFERRED_EXECUTORS 10` (or add 10 if you already use it) to your `config.h` file.
3. Clone the `sm_td.h` repository into your `keymaps/your_keymap` folder (next to your `keymap.c`)
4. Add `#include "sm_td.h"` to your `keymap.c` file. !!! WARNING !!! There is a bug in v0.4.0 and the library would compile with a "'SMTD_KEYCODES_BEGIN' undeclared" error. You need to put this `#include "sm_td.h"` right after you define your custom keycodes enum (described on p.6).
5. Check `!process_smtd` first in your `process_record_user` function like this
   ```c
   bool process_record_user(uint16_t keycode, keyrecord_t *record) {
       if (!process_smtd(keycode, record)) {
           return false;
       }
       // your code here
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
   Please don't forget to put `;` at the end of the enum definition. Normally it's not necessary, but if you put `#include "sm_td.h"` it will break compilation.   Note that `SAFE_RANGE` is a predefined constant in QMK, and is used [to define custom keycodes](https://docs.qmk.fm/custom_quantum_functions).
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
   See extensive documentation in the [Customization Guide](https://github.com/stasmarkin/sm_td/wiki/2.0:-Customization-guide) with cool [Examples](https://github.com/stasmarkin/sm_td/wiki/2.1:-Customization-guide:-Examples)

9. (optional) Add global configuration parameters to your `config.h` file (see [timeouts](https://github.com/stasmarkin/sm_td/wiki/2.2:-Customization-guide:-Timeouts-per-key) and [feature flags](https://github.com/stasmarkin/sm_td/wiki/2.3:-Customization-guide:-Feature-flags)).
10. (optional) Add per-key configuration (see [timeouts](https://github.com/stasmarkin/sm_td/wiki/2.2:-Customization-guide:-Timeouts-per-key) and [feature flags](https://github.com/stasmarkin/sm_td/wiki/2.3:-Customization-guide:-Feature-flags))


## What is `on_smtd_action()` function?

As soon as you press a key assigned between `SMTD_KEYCODES_BEGIN` and `SMTD_KEYCODES_END`, all state machines in the stack start working.
Other keys you press after the sm_td state machine has run will also be processed by the sm_td state machine.
This state machine might decide to postpone the processing of the key you pressed, so that it will be considered a tap or hold later.
You don't have to worry about this, sm_td will process all keys you press in the correct order and in a very predictable way.
You also don't need to worry about the implementation of the state machine stack, but you do need to know what output you will get from sm_td's state machine.
As soon as you press keys assigned to sm_td, it will call the `on_smtd_action()` function with the following arguments
- uint16_t keycode - keycode of the key you pressed
- smtd_action action - result interpreted action (`SMTD_ACTION_TOUCH`, `SMTD_ACTION_TAP`, `SMTD_ACTION_HOLD`, `SMTD_ACTION_RELEASE`). tap, hold and release are self-explanatory. Touch action fired on key press (without knowing if it is a tap or hold).
- uint8_t tap_count - number of consecutive taps before the current action. (will be reset after hold, pause or any other keypress)

There are only two execution flows for the `on_smtd_action' function:
- `SMTD_ACTION_TOUCH` → `SMTD_ACTION_TAP`.
- `SMTD_ACTION_TOUCH` → `SMTD_ACTION_HOLD` → `SMTD_ACTION_RELEASE`.

For a better understanding of the execution flow, please check the following example.
Let's say you want to tap, tap, hold and tap again a custom key `CKC`. Here are your finger movements:

- `↓CKC` 50ms `↑CKC` (first tap finished) 50ms 
- `↓CKC` 50ms `↑CKC` (second tap finished) 50ms 
- `↓CKC` 200ms (holding long enough for hold action) `↑CKC` 50ms 
- `↓CKC` 50ms `↑CKC` (third tap finished)

For this example, you will get the following `on_smtd_action()` calls:
- `on_smtd_action(CKC, SMTD_ACTION_TOUCH, 0)` right after pressing `↓CKC`
- `on_smtd_action(CKC, SMTD_ACTION_TAP, 0)` right after releasing `↑CKC` (first tap finished)
- `on_smtd_action(CKC, SMTD_ACTION_TOUCH, 1)` right after pressing `↓CKC` second time
- `on_smtd_action(CKC, SMTD_ACTION_TAP, 1)` right after releasing `↑CKC` second time (second tap finished)
- `on_smtd_action(CKC, SMTD_ACTION_TOUCH, 2)` right after pressing `↓CKC` third time
- `on_smtd_action(CKC, SMTD_ACTION_HOLD, 2)` after holding `CKC` long enough
- `on_smtd_action(CKC, SMTD_ACTION_RELEASE, 2)` right after releasing `↑CKC` (hold)
- `on_smtd_action(CKC, SMTD_ACTION_TOUCH, 0)` right after pressing `↓CKC` fourth time
- `on_smtd_action(CKC, SMTD_ACTION_TAP, 0)` right after releasing `↑CKC` (third tap finished)

If you need, there is a deeper documentation on execution flow, please see [state machine description](https://github.com/stasmarkin/sm_td/wiki/3.0:-Deep-explanation:-Stages) and further [one key explanation](https://github.com/stasmarkin/sm_td/wiki/3.1:-Deep-explanation:-One-key-stages), [two key explanation](https://github.com/stasmarkin/sm_td/wiki/3.2:-Deep-explanation:-Two-keys-stages) and [state machine stack](https://github.com/stasmarkin/sm_td/wiki/3.3:-Deep-explanation:-Three-keys-and-states-stack).
