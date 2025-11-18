/* Copyright 2025 Stanislav Markin (https://github.com/stasmarkin)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Version: 0.5.4
 * Date: 2025-09-25
 */
#pragma once

#ifndef SMTD_UNIT_TEST
#include QMK_KEYBOARD_H
#include "deferred_exec.h"
#endif

#ifdef SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS
#include "timer.h"
#endif

#ifdef SMTD_DEBUG_ENABLED
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "print.h"
#endif

#ifdef SMTD_UNIT_TEST
#define SMTD_DEBUG_ENABLED
#endif

/* ************************************* *
 *         GLOBAL CONFIGURATION          *
 * ************************************* */

#ifndef SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS
#define SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS 0
#endif

#if SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS > 0
#define SMTD_SIMULTANEOUS_PRESSES_DELAY wait_ms(SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS);
#else
#define SMTD_SIMULTANEOUS_PRESSES_DELAY
#endif

#ifndef SMTD_GLOBAL_TAP_TERM
#define SMTD_GLOBAL_TAP_TERM TAPPING_TERM
#endif

#ifndef SMTD_GLOBAL_SEQUENCE_TERM
#define SMTD_GLOBAL_SEQUENCE_TERM TAPPING_TERM / 2
#endif

#ifndef SMTD_GLOBAL_RELEASE_TERM
#define SMTD_GLOBAL_RELEASE_TERM TAPPING_TERM / 4
#endif

#ifndef SMTD_GLOBAL_AGGREGATE_TAPS
#define SMTD_GLOBAL_AGGREGATE_TAPS false
#endif

// SMTD_GLOBAL_MODS_PROPAGATION_ENABLED
// fixme-sm looks like this flag is not useful anymore.
//          I think, it should be removed in next versions
//          This flag was introduced to make sm_td usable with non-sm_td modifier (like generic shift)
//          But after X refactorings, it looks like sm_td behaves the same with and without that flag

#include <stdint.h>


/* ************************************* *
 *         BASE DEFINITIONS              *
 * ************************************* */

typedef enum {
    SMTD_STAGE_NONE,
    SMTD_STAGE_TOUCH,
    SMTD_STAGE_SEQUENCE,
    SMTD_STAGE_HOLD,
    SMTD_STAGE_TOUCH_RELEASE,
    SMTD_STAGE_HOLD_RELEASE,
} smtd_stage;

typedef enum {
    SMTD_ACTION_TOUCH,
    SMTD_ACTION_TAP,
    SMTD_ACTION_HOLD,
    SMTD_ACTION_RELEASE,
} smtd_action;

typedef enum {
    SMTD_RESOLUTION_UNCERTAIN,
    SMTD_RESOLUTION_UNHANDLED,
    SMTD_RESOLUTION_DETERMINED,
} smtd_resolution;

typedef enum {
    SMTD_TIMEOUT_TAP,
    SMTD_TIMEOUT_SEQUENCE,
    SMTD_TIMEOUT_RELEASE,
} smtd_timeout;

typedef enum {
    SMTD_FEATURE_AGGREGATE_TAPS,
} smtd_feature;


typedef struct {
    /** The position of a key that QMK thinks was pressed */
    keypos_t pressed_keyposition;

    /** The keycode of a key that QMK thinks was pressed */
    uint16_t pressed_keycode;

    #ifdef SMTD_GLOBAL_MODS_PROPAGATION_ENABLED
    /** The mods on key tap */
    uint8_t saved_mods;
    #endif

    /** The keycode that should be actually pressed (asked outside or determined by the tap action) */
    uint16_t desired_keycode;

    /** The length of the sequence of same key taps */
    uint8_t tap_count;

    /** The time when the key was pressed */
    uint32_t pressed_time;

    /** The time when the key was released */
    uint32_t released_time;

    /** The timeout of current stage */
    deferred_token timeout;

    /** The current stage of the state */
    smtd_stage stage;

    /** The level of certainty of the state */
    smtd_resolution resolution;

    /** The action that already performed */
    int8_t action_performed;

    /** The action that can be performed */
    int8_t action_required;

    /** The index of the state in the active states array */
    uint8_t idx;
} smtd_state;


#ifdef SMTD_GLOBAL_MODS_PROPAGATION_ENABLED
#define EMPTY_STATE {                               \
        .pressed_keyposition = MAKE_KEYPOS(0, 0),   \
        .pressed_keycode = 0,                       \
        .desired_keycode = 0,                       \
        .saved_mods = 0,                            \
        .tap_count = 0,                             \
        .pressed_time = 0,                          \
        .released_time = 0,                         \
        .timeout = INVALID_DEFERRED_TOKEN,          \
        .stage = SMTD_STAGE_NONE,                   \
        .resolution = SMTD_RESOLUTION_UNCERTAIN,    \
        .action_performed = -1,                     \
        .action_required = -1,                      \
        .idx = 0,                                   \
}
#else
#define EMPTY_STATE {                               \
        .pressed_keyposition = MAKE_KEYPOS(0, 0),   \
        .pressed_keycode = 0,                       \
        .desired_keycode = 0,                       \
        .tap_count = 0,                             \
        .pressed_time = 0,                          \
        .released_time = 0,                         \
        .timeout = INVALID_DEFERRED_TOKEN,          \
        .stage = SMTD_STAGE_NONE,                   \
        .resolution = SMTD_RESOLUTION_UNCERTAIN,    \
        .action_performed = -1,                     \
        .action_required = -1,                      \
        .idx = 0,                                   \
}
#endif


#define SMTD_POOL_SIZE 10

/* ************************************* *
 *           PUBLIC FUNCTIONS            *
 * ************************************* */

bool process_smtd(uint16_t keycode, keyrecord_t *record);

smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count);

__attribute__((weak)) uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout);

__attribute__((weak)) bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature);

extern const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS];


/* ************************************* *
 *           INTERNAL FUNCTIONS          *
 * ************************************* */

bool smtd_process_desired(uint16_t pressed_keycode, keyrecord_t *record, uint16_t desired_keycode);

void smtd_apply_to_stack(uint8_t starting_idx, uint16_t pressed_keycode, keyrecord_t *record, uint16_t desired_keycode);

void smtd_create_state(uint16_t pressed_keycode, keyrecord_t *record, uint16_t desired_keycode);

void smtd_apply_event(bool is_state_key, smtd_state *state, uint16_t pressed_keycode, keyrecord_t *record);

void smtd_apply_stage(smtd_state *state, smtd_stage next_stage);

void smtd_handle_action(smtd_state *state, smtd_action action);

void smtd_execute_action(smtd_state *state, smtd_action action);

void smtd_emulate_key(keypos_t *keypos, bool press);

#ifdef SMTD_GLOBAL_MODS_PROPAGATION_ENABLED
void smtd_propagate_mods(smtd_state *state, uint8_t mods_before_action, uint8_t mods_after_action);
#endif

smtd_resolution smtd_worst_resolution_before(smtd_state *state);

uint16_t smtd_current_keycode(keypos_t *key);

uint32_t get_smtd_timeout_default(smtd_timeout timeout);

uint32_t get_smtd_timeout_or_default(smtd_state *state, smtd_timeout timeout);

bool smtd_feature_enabled_default(uint16_t keycode, smtd_feature feature);

bool smtd_feature_enabled_or_default(smtd_state *state, smtd_feature feature);


/* ************************************* *
 *           DEBUG CONFIGURATION         *
 * ************************************* */

#ifndef SMTD_DEBUG_ENABLED

#define SMTD_DEBUG(...)
#define SMTD_DEBUG_INPUT(...)
#define SMTD_DEBUG_OFFSET_INC
#define SMTD_DEBUG_OFFSET_DEC
#define SMTD_DEBUG_FULL(...)

#else

uint32_t last_key_timer = 0;

#ifndef SMTD_PRINT
#define SMTD_PRINT(...) printf(__VA_ARGS__);
#endif

#ifndef SMTD_SNPRINT
#define SMTD_SNPRINT(bffr, bsize, ...) snprintf(bffr, bsize, __VA_ARGS__);
#endif

#ifndef SMTD_DEBUG_PRINT_OFFSETS
#define SMTD_DEBUG_PRINT_OFFSETS for (uint8_t __i__ = 0; __i__ < smtd_debug_offset; __i__++) { SMTD_PRINT("  "); }
#endif

#ifndef SMTD_DEBUG_OFFSET_INC
#define SMTD_DEBUG_OFFSET_INC smtd_debug_offset++;
#endif

#ifndef SMTD_DEBUG_OFFSET_DEC
#define SMTD_DEBUG_OFFSET_DEC smtd_debug_offset--;
#endif

#ifndef SMTD_DEBUG
#define SMTD_DEBUG(...) do { \
SMTD_PRINT("[%4d] ", __LINE__); \
SMTD_DEBUG_PRINT_OFFSETS; \
SMTD_PRINT(__VA_ARGS__); \
SMTD_PRINT("\n"); \
} while (0);
#endif

#ifndef SMTD_DEBUG_INPUT
#define SMTD_DEBUG_INPUT(...) \
SMTD_DEBUG("%s", ""); \
SMTD_DEBUG(">> +%lums", timer_elapsed32(last_key_timer)); \
SMTD_DEBUG(__VA_ARGS__); \
last_key_timer = timer_read32();
#endif

#ifndef SMTD_SNDEBUG
#define SMTD_SNDEBUG(bffr, bsize, ...) SMTD_SNPRINT(bffr, bsize, __VA_ARGS__);
#endif

#ifndef SMTD_DEBUG_FULL
#define SMTD_DEBUG_FULL() \
  SMTD_DEBUG("## active_statses %d", smtd_active_states_size); \
  if (smtd_active_states_size == 0) { SMTD_DEBUG("## %s", ""); SMTD_DEBUG("## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ##\n"); }; \
  for(int asdf=0; asdf<smtd_active_states_size; asdf++) { SMTD_DEBUG("## %s", smtd_state_to_str(smtd_active_states[asdf])); }
#endif

static uint8_t smtd_debug_offset = 0;

char *smtd_stage_to_str(smtd_stage stage);

char *smtd_action_to_str(smtd_action action);

char *smtd_resolution_to_str(smtd_resolution resolution);

__attribute__((weak)) char* smtd_keycode_to_str_user(uint16_t keycode);

char* smtd_keycode_to_str_uncertain(uint16_t keycode, bool uncertain);

char* smtd_keycode_to_str(uint16_t keycode);

char* smtd_state_to_str(smtd_state *state);

char* smtd_state_to_str2(smtd_state *state);

char* smtd_record_to_str(keyrecord_t *record);

#endif


/* ************************************* *
 *             TIMEOUTS                  *
 * ************************************* */

uint32_t timeout_reset_seq(uint32_t trigger_time, void *cb_arg);

uint32_t timeout_touch(uint32_t trigger_time, void *cb_arg);

uint32_t timeout_sequence(uint32_t trigger_time, void *cb_arg);

uint32_t timeout_touch_release(uint32_t trigger_time, void *cb_arg);

uint32_t timeout_hold_release(uint32_t trigger_time, void *cb_arg);


/* ************************************* *
 *             STATE PROCESSING          *
 * ************************************* */

bool process_smtd(uint16_t pressed_keycode, keyrecord_t *record);

bool smtd_process_desired(uint16_t pressed_keycode, keyrecord_t *record, uint16_t desired_keycode);

void
smtd_apply_to_stack(uint8_t starting_idx, uint16_t pressed_keycode, keyrecord_t *record, uint16_t desired_keycode);

void smtd_create_state(uint16_t pressed_keycode, keyrecord_t *record, uint16_t desired_keycode);

bool is_following_key(smtd_state *state, uint16_t pressed_keycode, keyrecord_t *record);

void smtd_apply_event(bool is_state_key, smtd_state *state, uint16_t pressed_keycode, keyrecord_t *record);

void reset_state(smtd_state *state);

void smtd_apply_stage(smtd_state *state, smtd_stage next_stage);

void smtd_handle_action(smtd_state *state, smtd_action action);

void smtd_execute_action(smtd_state *state, smtd_action action);

#ifdef SMTD_GLOBAL_MODS_PROPAGATION_ENABLED
void smtd_propagate_mods(smtd_state *state, uint8_t mods_before_action, uint8_t mods_after_action);
#endif

/* ************************************* *
 *      UTILITY FUNCTIONS                *
 * ************************************* */

void smtd_emulate_key(keypos_t *keypos, bool press);

smtd_resolution smtd_worst_resolution_before(smtd_state *state);

uint32_t get_smtd_timeout_or_default(smtd_state *state, smtd_timeout timeout);

uint32_t get_smtd_timeout_default(smtd_timeout timeout);

uint16_t smtd_current_keycode(keypos_t *key);

bool smtd_feature_enabled_or_default(smtd_state *state, smtd_feature feature);

bool smtd_feature_enabled_default(uint16_t keycode, smtd_feature feature);


/* ************************************* *
 *         CUSTOMIZATION MACROS          *
 * ************************************* */

#ifdef CAPS_WORD_ENABLE
#define SMTD_TAP_16(use_cl, key) tap_code16(use_cl && is_caps_word_on() ? LSFT(key) : key)
#define SMTD_REGISTER_16(use_cl, key) register_code16(use_cl && is_caps_word_on() ? LSFT(key) : key)
#define SMTD_UNREGISTER_16(use_cl, key) unregister_code16(use_cl && is_caps_word_on() ? LSFT(key) : key)
#else
#define SMTD_TAP_16(use_cl, key) tap_code16(key)
#define SMTD_REGISTER_16(use_cl, key) register_code16(key)
#define SMTD_UNREGISTER_16(use_cl, key) unregister_code16(key)
#endif

#ifndef NOTHING
#define NOTHING
#endif
#ifndef OVERLOAD6
#define OVERLOAD6(_1, _2, _3, _4, _5, _6, NAME, ...) NAME
#endif
#ifndef OVERLOAD5
#define OVERLOAD5(_1, _2, _3, _4, _5, NAME, ...) NAME
#endif
#ifndef OVERLOAD4
#define OVERLOAD4(_1, _2, _3, _4, NAME, ...) NAME
#endif
#ifndef OVERLOAD3
#define OVERLOAD3(_1, _2, _3, NAME, ...) NAME
#endif
#ifndef OVERLOAD2
#define OVERLOAD2(_1, _2, NAME, ...) NAME
#endif
#ifndef EXEC
#define EXEC(code) do { code } while(0)
#endif

#define SMTD_LIMIT(limit, if_under_limit, otherwise) \
    if (tap_count < limit) { if_under_limit; } else { otherwise; }

#define SMTD_DANCE(macro_key, touch_action, tap_action, hold_action, release_action)    \
    case macro_key: {                                                                   \
        switch (action) {                                                               \
            case SMTD_ACTION_TOUCH: touch_action; return SMTD_RESOLUTION_UNCERTAIN;     \
            case SMTD_ACTION_TAP: tap_action; return SMTD_RESOLUTION_DETERMINED;        \
            case SMTD_ACTION_HOLD: hold_action; return SMTD_RESOLUTION_DETERMINED;      \
            case SMTD_ACTION_RELEASE: release_action; return SMTD_RESOLUTION_DETERMINED;\
        }                                                                               \
        break;                                                                          \
    }

#define SMTD_MT(...) OVERLOAD4(__VA_ARGS__, SMTD_MT4, SMTD_MT3, SMTD_MT2)(__VA_ARGS__)
#define SMTD_MT2(key, mod) SMTD_MT3_ON_MKEY(key, key, mod)
#define SMTD_MT3(key, mod, threshold) SMTD_MT4_ON_MKEY(key, key, mod, threshold)
#define SMTD_MT4(key, mod, threshold, use_cl) SMTD_MT5_ON_MKEY(key, key, mod, threshold, use_cl)
#define SMTD_MT_ON_MKEY(...) OVERLOAD5(__VA_ARGS__, SMTD_MT5_ON_MKEY, SMTD_MT4_ON_MKEY, SMTD_MT3_ON_MKEY)(__VA_ARGS__)
#define SMTD_MT3_ON_MKEY(...) SMTD_MT4_ON_MKEY(__VA_ARGS__, 1)
#define SMTD_MT4_ON_MKEY(...) SMTD_MT5_ON_MKEY(__VA_ARGS__, true)
#define SMTD_MT5_ON_MKEY(macro_key, tap_key, mod, threshold, use_cl) \
    SMTD_DANCE(macro_key,                                    \
        NOTHING,                                             \
        SMTD_TAP_16(use_cl, tap_key),                        \
        SMTD_LIMIT(threshold,                                \
            register_mods(MOD_BIT(mod));                     \
            send_keyboard_report(),                          \
            SMTD_REGISTER_16(use_cl, tap_key)),              \
        SMTD_LIMIT(threshold,                                \
            unregister_mods(MOD_BIT(mod));                   \
            send_keyboard_report(),                          \
            SMTD_UNREGISTER_16(use_cl, tap_key));            \
            send_keyboard_report()                           \
    )

#define SMTD_MTE(...) OVERLOAD4(__VA_ARGS__, SMTD_MTE4, SMTD_MTE3, SMTD_MTE2)(__VA_ARGS__)
#define SMTD_MTE2(key, mod_key) SMTD_MTE3_ON_MKEY(key, key, mod_key)
#define SMTD_MTE3(key, mod_key, threshold) SMTD_MTE4_ON_MKEY(key, key, mod_key, threshold)
#define SMTD_MTE4(key, mod_key, threshold, use_cl) SMTD_MTE5_ON_MKEY(key, key, mod_key, threshold, use_cl)
#define SMTD_MTE_ON_MKEY(...) OVERLOAD5(__VA_ARGS__, SMTD_MTE5_ON_MKEY, SMTD_MTE4_ON_MKEY, SMTD_MTE3_ON_MKEY)(__VA_ARGS__)
#define SMTD_MTE3_ON_MKEY(...) SMTD_MTE4_ON_MKEY(__VA_ARGS__, 1)
#define SMTD_MTE4_ON_MKEY(...) SMTD_MTE5_ON_MKEY(__VA_ARGS__, true)
#define SMTD_MTE5_ON_MKEY(macro_key, tap_key, mod_key, threshold, use_cl) \
    SMTD_MBTE5_ON_MKEY(macro_key, tap_key, MOD_BIT(mod_key), threshold, use_cl)
#define SMTD_MBTE5_ON_MKEY(macro_key, tap_key, mods, threshold, use_cl) \
    SMTD_DANCE(macro_key,                                    \
        EXEC(                                                \
            register_mods(mods);                             \
            send_keyboard_report();                          \
        ),                                                   \
        EXEC(                                                \
            unregister_mods(mods);                           \
            SMTD_TAP_16(use_cl, tap_key);                    \
        ),                                                   \
        SMTD_LIMIT(threshold,                                \
            NOTHING,                                         \
            EXEC(                                            \
                unregister_mods(mods);                       \
                send_keyboard_report();                      \
                SMTD_REGISTER_16(use_cl, tap_key);           \
            )                                                \
        ),                                                   \
        SMTD_LIMIT(threshold,                                \
            EXEC(                                            \
                unregister_mods(mods);                       \
                send_keyboard_report();                      \
            ),                                               \
            SMTD_UNREGISTER_16(use_cl, tap_key)              \
        )                                                    \
    )

#define SMTD_LT(...) OVERLOAD4(__VA_ARGS__, SMTD_LT4, SMTD_LT3, SMTD_LT2)(__VA_ARGS__)
#define SMTD_LT2(key, layer) SMTD_LT3_ON_MKEY(key, key, layer)
#define SMTD_LT3(key, layer, threshold) SMTD_LT4_ON_MKEY(key, key, layer, threshold)
#define SMTD_LT4(key, layer, threshold, use_cl) SMTD_LT5_ON_MKEY(key, key, layer, threshold, use_cl)
#define SMTD_LT_ON_MKEY(...) OVERLOAD5(__VA_ARGS__, SMTD_LT5_ON_MKEY, SMTD_LT4_ON_MKEY, SMTD_LT3_ON_MKEY)(__VA_ARGS__)
#define SMTD_LT3_ON_MKEY(...) SMTD_LT4_ON_MKEY(__VA_ARGS__, 1)
#define SMTD_LT4_ON_MKEY(...) SMTD_LT5_ON_MKEY(__VA_ARGS__, true)
#define SMTD_LT5_ON_MKEY(macro_key, tap_key, layer, threshold, use_cl)\
    SMTD_DANCE(macro_key,                                     \
        NOTHING,                                              \
        SMTD_TAP_16(use_cl, tap_key),                         \
        SMTD_LIMIT(threshold,                                 \
            LAYER_PUSH(layer),                                \
            SMTD_REGISTER_16(use_cl, tap_key)),               \
        SMTD_LIMIT(threshold,                                 \
            LAYER_RESTORE(),                                  \
            SMTD_UNREGISTER_16(use_cl, tap_key));             \
    )

#define SMTD_TD(...) OVERLOAD4(__VA_ARGS__, SMTD_TD4, SMTD_TD3, SMTD_TD2)(__VA_ARGS__)
#define SMTD_TD2(key, tap_key) SMTD_TD3_ON_MKEY(key, key, tap_key)
#define SMTD_TD3(key, tap_key, threshold) SMTD_TD4_ON_MKEY(key, key, tap_key, threshold)
#define SMTD_TD4(key, tap_key, threshold, use_cl) SMTD_TD5_ON_MKEY(key, key, tap_key, threshold, use_cl)
#define SMTD_TD_ON_MKEY(...) OVERLOAD5(__VA_ARGS__, SMTD_TD5_ON_MKEY, SMTD_TD4_ON_MKEY, SMTD_TD3_ON_MKEY)(__VA_ARGS__)
#define SMTD_TD3_ON_MKEY(...) SMTD_TD4_ON_MKEY(__VA_ARGS__, 1)
#define SMTD_TD4_ON_MKEY(...) SMTD_TD5_ON_MKEY(__VA_ARGS__, true)
#define SMTD_TD5_ON_MKEY(macro_key, tap_key, hold_key, threshold, use_cl)\
    SMTD_DANCE(macro_key,                                        \
        NOTHING,                                                 \
        SMTD_TAP_16(use_cl, tap_key),                            \
        SMTD_LIMIT(threshold,                                    \
            SMTD_TAP_16(use_cl, hold_key),                       \
            SMTD_TAP_16(use_cl, tap_key)),                       \
        SMTD_LIMIT(threshold,                                    \
            SMTD_UNREGISTER_16(use_cl, hold_key),                \
            SMTD_UNREGISTER_16(use_cl, tap_key))                 \
    )

// multi-tap activated key
#define SMTD_TK(...) OVERLOAD4(__VA_ARGS__, SMTD_TK4, SMTD_TK3, SMTD_TK2)(__VA_ARGS__)
#define SMTD_TK2(key, tap_key) SMTD_TK3_ON_MKEY(key, key, tap_key)
#define SMTD_TK3(key, tap_key, threshold) SMTD_TK3_ON_MKEY(key, key, tap_key, threshold)
#define SMTD_TK4(key, tap_key, threshold, use_cl) SMTD_TK4_ON_MKEY(key, key, tap_key, threshold, use_cl)
#define SMTD_TK_ON_MKEY(...) OVERLOAD4(__VA_ARGS__, SMTD_TK4_ON_MKEY, SMTD_TK3_ON_MKEY, SMTD_TK2_ON_MKEY)(__VA_ARGS__)
#define SMTD_TK2_ON_MKEY(...) SMTD_TK3_ON_MKEY(__VA_ARGS__, 1)
#define SMTD_TK3_ON_MKEY(...) SMTD_TK4_ON_MKEY(__VA_ARGS__, true)
#define SMTD_TK4_ON_MKEY(macro_key, tap_key, threshold, use_cl) \
    SMTD_DANCE(macro_key,                              \
        SMTD_LIMIT(threshold,                          \
            NOTHING,                                   \
            SMTD_TAP_16(use_cl, tap_key)),             \
        NOTHING,                                       \
        NOTHING,                                       \
        NOTHING                                        \
    )

// multi-tap activated layer move
#define SMTD_TTO(...) OVERLOAD4(__VA_ARGS__, SMTD_TTO4, SMTD_TTO3, SMTD_TTO2)(__VA_ARGS__)
#define SMTD_TTO2(key, layer) SMTD_TTO3_ON_MKEY(key, key, layer)
#define SMTD_TTO3(key, layer, threshold) SMTD_TTO3_ON_MKEY(key, key, layer)
#define SMTD_TTO4(key, layer, threshold, use_cl) SMTD_TTO4_ON_MKEY(key, key, layer, threshold, use_cl)
#define SMTD_TTO_ON_MKEY(...) OVERLOAD4(__VA_ARGS__, SMTD_TTO4_ON_MKEY, SMTD_TTO3_ON_MKEY, SMTD_TTO2_ON_MKEY)(__VA_ARGS__)
#define SMTD_TTO2_ON_MKEY(...) SMTD_TTO3_ON_MKEY(__VA_ARGS__, 1)
#define SMTD_TTO3_ON_MKEY(...) SMTD_TTO4_ON_MKEY(__VA_ARGS__, true)
#define SMTD_TTO4_ON_MKEY(macro_key, layer, threshold, use_cl) \
    SMTD_DANCE(macro_key,                              \
        SMTD_LIMIT(threshold,                          \
            NOTHING,                                   \
            layer_move(layer)),                        \
        NOTHING,                                       \
        NOTHING,                                       \
        NOTHING                                        \
    )


/* ************************************* *
 *             LAYER UTILS               *
 * ************************************* */

#define RETURN_LAYER_NOT_SET 13

static uint8_t return_layer = RETURN_LAYER_NOT_SET;
static uint8_t return_layer_cnt = 0;

void avoid_unused_variable_on_compile(void *ptr);

#define LAYER_PUSH(layer)                              \
    return_layer_cnt++;                                \
    if (return_layer == RETURN_LAYER_NOT_SET) {        \
        return_layer = get_highest_layer(layer_state); \
    }                                                  \
    layer_move(layer);

#define LAYER_RESTORE()                          \
    if (return_layer_cnt > 0) {                  \
        return_layer_cnt--;                      \
        if (return_layer_cnt == 0) {             \
            layer_move(return_layer);            \
            return_layer = RETURN_LAYER_NOT_SET; \
        }                                        \
    }
