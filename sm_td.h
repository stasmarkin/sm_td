/* Copyright 2024 Stanislav Markin (https://github.com/stasmarkin)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Version: 0.5.0-RC5
 * Date: 2025-04-12
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

    /** The keycode that should be actually pressed (asked outside or determined by the tap action) */
    uint16_t desired_keycode;

    /** The length of the sequence of same key taps */
    uint8_t sequence_len;

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

    /** Whether the next action should be performed */
    bool need_next_action;

    /** The action that should be performed next */
    smtd_action next_action;

    /** The index of the state in the active states array */
    uint8_t idx;
} smtd_state;

#define EMPTY_STATE {                               \
        .pressed_keyposition = MAKE_KEYPOS(0, 0),   \
        .pressed_keycode = 0,                       \
        .desired_keycode = 0,                       \
        .sequence_len = 0,                          \
        .pressed_time = 0,                          \
        .released_time = 0,                         \
        .timeout = INVALID_DEFERRED_TOKEN,          \
        .stage = SMTD_STAGE_NONE,                   \
        .resolution = SMTD_RESOLUTION_UNCERTAIN,    \
        .next_action = SMTD_ACTION_TOUCH,           \
        .need_next_action = false,                  \
        .idx = 0,                                   \
}

#define SMTD_POOL_SIZE 10
static smtd_state *smtd_active_states[SMTD_POOL_SIZE] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static smtd_state smtd_states_pool[SMTD_POOL_SIZE] = {
    EMPTY_STATE, EMPTY_STATE, EMPTY_STATE, EMPTY_STATE, EMPTY_STATE,
    EMPTY_STATE, EMPTY_STATE, EMPTY_STATE, EMPTY_STATE, EMPTY_STATE
};
static uint8_t smtd_active_states_size = 0;
static bool smtd_bypass = false;

/* ************************************* *
 *           PUBLIC FUNCTIONS            *
 * ************************************* */

bool process_smtd(uint16_t keycode, keyrecord_t *record);

smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t sequence_len);

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

void smtd_emulate_press(keypos_t *keypos, bool press);

smtd_resolution smtd_worst_resolution_before(smtd_state *state);

uint16_t smtd_current_keycode(keypos_t *key);

uint32_t get_smtd_timeout_default(smtd_timeout timeout);

uint32_t get_smtd_timeout_or_default(smtd_state *state, smtd_timeout timeout);

bool smtd_feature_enabled_default(uint16_t keycode, smtd_feature feature);

bool smtd_feature_enabled_or_default(smtd_state *state, smtd_feature feature);


/* ************************************* *
 *           DEBUG CONFIGURATION         *
 * ************************************* */

#ifdef SMTD_DEBUG_ENABLED

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

char *smtd_stage_to_str(smtd_stage stage) {
    switch (stage) {
        case SMTD_STAGE_NONE:
            return "S_NON";
        case SMTD_STAGE_TOUCH:
            return "S_TCH";
        case SMTD_STAGE_SEQUENCE:
            return "S_SEQ";
        case SMTD_STAGE_HOLD:
            return "S_HLD";
        case SMTD_STAGE_TOUCH_RELEASE:
            return "S_TRL";
        case SMTD_STAGE_HOLD_RELEASE:
            return "S_HRL";
    }
    return "S_???";
}

char *smtd_action_to_str(smtd_action action) {
    switch (action) {
        case SMTD_ACTION_TOUCH:
            return "TCH";
        case SMTD_ACTION_TAP:
            return "TAP";
        case SMTD_ACTION_HOLD:
            return "HLD";
        case SMTD_ACTION_RELEASE:
            return "RLS";
    }
    return "?A?";
}

char *smtd_resolution_to_str(smtd_resolution resolution) {
    switch (resolution) {
        case SMTD_RESOLUTION_UNCERTAIN:
            return "xx";
        case SMTD_RESOLUTION_UNHANDLED:
            return "--";
        case SMTD_RESOLUTION_DETERMINED:
            return "!!";
    }
    return "??";
}

__attribute__((weak)) char* smtd_keycode_to_str_user(uint16_t keycode);

char* smtd_keycode_to_str_uncertain(uint16_t keycode, bool uncertain) {
    static char buffer_keycode[16];

    if (smtd_keycode_to_str_user) {
        char* result = smtd_keycode_to_str_user(keycode);
        if (result) {
            SMTD_SNDEBUG(buffer_keycode, sizeof(buffer_keycode), uncertain ? "?%s" : "%s", result);
            return result;
        }
    }

    SMTD_SNDEBUG(buffer_keycode, sizeof(buffer_keycode), uncertain ? "?KC_%d" : "KC_%d", keycode);
    return buffer_keycode;
}

char* smtd_keycode_to_str(uint16_t keycode) {
    return smtd_keycode_to_str_uncertain(keycode, false);
}

char* smtd_state_to_str(smtd_state *state) {
    static char buffer_state[64];

    SMTD_SNDEBUG(buffer_state, sizeof(buffer_state), "S[%d](@%d.%d#%s->%s){%s/%s}",
             state->idx,
             state->pressed_keyposition.row,
             state->pressed_keyposition.col,
             smtd_keycode_to_str(state->pressed_keycode),
             smtd_keycode_to_str(state->desired_keycode),
             smtd_stage_to_str(state->stage),
             smtd_resolution_to_str(state->resolution));

    return buffer_state;
}

char* smtd_state_to_str2(smtd_state *state) {
    static char buffer_state2[64];

    SMTD_SNDEBUG(buffer_state2, sizeof(buffer_state2), "S[%d](@%d.%d#%s->%s){%s/%s}",
             state->idx,
             state->pressed_keyposition.row,
             state->pressed_keyposition.col,
             smtd_keycode_to_str(state->pressed_keycode),
             smtd_keycode_to_str(state->desired_keycode),
             smtd_stage_to_str(state->stage),
             smtd_resolution_to_str(state->resolution));

    return buffer_state2;
}

char* smtd_record_to_str(keyrecord_t *record) {
    static char buffer_record[32];

    SMTD_SNDEBUG(buffer_record, sizeof(buffer_record), "R(@%d.%d %s)", record->event.key.row, record->event.key.col, record->event.pressed ? "|*|" : "|O|");

    return buffer_record;
}
#else

#define SMTD_DEBUG(...)
#define SMTD_DEBUG_INPUT(...)
#define SMTD_DEBUG_OFFSET_INC
#define SMTD_DEBUG_OFFSET_DEC
#define SMTD_DEBUG_FULL(...)

#endif


/* ************************************* *
 *             TIMEOUTS                  *
 * ************************************* */

uint32_t timeout_reset_seq(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    SMTD_DEBUG_INPUT(">> %s timeout_reset_seq", smtd_state_to_str(state));
    state->sequence_len = 0;
    SMTD_DEBUG("<< %s timeout_reset_seq", smtd_state_to_str(state));
    SMTD_DEBUG_FULL();
    return 0;
}

uint32_t timeout_touch(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    SMTD_DEBUG_INPUT(">> %s timeout_touch", smtd_state_to_str(state));
    SMTD_DEBUG_OFFSET_INC;
    smtd_apply_stage(state, SMTD_STAGE_HOLD);
    smtd_handle_action(state, SMTD_ACTION_HOLD);
    SMTD_DEBUG_OFFSET_DEC;
    SMTD_DEBUG("<< %s timeout_touch", smtd_state_to_str(state));
    SMTD_DEBUG_FULL();
    return 0;
}

uint32_t timeout_sequence(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    SMTD_DEBUG_INPUT(">> %s timeout_sequence", smtd_state_to_str(state));
    SMTD_DEBUG_OFFSET_INC;
    if (smtd_feature_enabled_or_default(state, SMTD_FEATURE_AGGREGATE_TAPS)) {
        smtd_handle_action(state, SMTD_ACTION_TAP);
    }
    smtd_apply_stage(state, SMTD_STAGE_NONE);
    SMTD_DEBUG_OFFSET_DEC;
    SMTD_DEBUG("<< %s timeout_sequence", smtd_state_to_str(state));
    SMTD_DEBUG_FULL();
    return 0;
}

uint32_t timeout_touch_release(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    SMTD_DEBUG_INPUT(">> %s timeout_touch_release", smtd_state_to_str(state));
    SMTD_DEBUG_OFFSET_INC;
    smtd_handle_action(state, SMTD_ACTION_TAP);
    smtd_apply_stage(state, SMTD_STAGE_NONE);
    SMTD_DEBUG_OFFSET_DEC;
    SMTD_DEBUG("<< %s timeout_touch_release", smtd_state_to_str(state));
    SMTD_DEBUG_FULL();
    return 0;
}

uint32_t timeout_hold_release(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    SMTD_DEBUG_INPUT(">> %s timeout_hold_release", smtd_state_to_str(state));
    SMTD_DEBUG_OFFSET_INC;
    smtd_handle_action(state, SMTD_ACTION_RELEASE);
    smtd_apply_stage(state, SMTD_STAGE_NONE);
    SMTD_DEBUG_OFFSET_DEC;
    SMTD_DEBUG("<< %s timeout_hold_release", smtd_state_to_str(state));
    SMTD_DEBUG_FULL();
    return 0;
}


/* ************************************* *
 *             STATE PROCESSING          *
 * ************************************* */

bool process_smtd(uint16_t pressed_keycode, keyrecord_t *record) {
    return smtd_process_desired(pressed_keycode, record, 0);
}

bool smtd_process_desired(uint16_t pressed_keycode, keyrecord_t *record, uint16_t desired_keycode) {
    if (smtd_bypass) {
        SMTD_DEBUG("%s GLOBAL BYPASS KEY %s",
                   smtd_record_to_str(record),
                   smtd_keycode_to_str_uncertain(pressed_keycode, desired_keycode == 0));
        return true;
    }

    SMTD_DEBUG_INPUT(">> %s GOT KEY %s",
               smtd_record_to_str(record),
               smtd_keycode_to_str_uncertain(pressed_keycode, desired_keycode == 0));

    smtd_apply_to_stack(0, pressed_keycode, record, desired_keycode);
    return false;
}

void
smtd_apply_to_stack(uint8_t starting_idx, uint16_t pressed_keycode, keyrecord_t *record, uint16_t desired_keycode) {
    SMTD_DEBUG("%s apply_to_stack starting idx=%d",
               smtd_record_to_str(record),
               starting_idx);

    bool processed_state = false;

    for (uint8_t i = starting_idx; i < smtd_active_states_size; i++) {
        smtd_state *state = smtd_active_states[i];

        bool is_state_key = (record->event.key.row == state->pressed_keyposition.row &&
                             record->event.key.col == state->pressed_keyposition.col) &&
                            (pressed_keycode == state->pressed_keycode ||
                             pressed_keycode == state->desired_keycode);
        processed_state = processed_state | is_state_key;

        SMTD_DEBUG_OFFSET_INC;
        smtd_apply_event(is_state_key, state, pressed_keycode, record);
        if (state->stage == SMTD_STAGE_NONE) {
            // stack has been moved with one element left
            // fixme-sm maybe move to cleanup stage?
            i--;
        }

        SMTD_DEBUG_OFFSET_DEC;
    }

    SMTD_DEBUG_OFFSET_INC;
    uint8_t idx = smtd_active_states_size;
    while (idx > 0) {
        smtd_state *state = smtd_active_states[idx - 1];
        if (state->stage == SMTD_STAGE_TOUCH_RELEASE) {
            SMTD_DEBUG("%s clean up", smtd_state_to_str(state));
            smtd_handle_action(state, SMTD_ACTION_TAP);
            smtd_apply_stage(state, SMTD_STAGE_NONE);
            idx = smtd_active_states_size;
            continue;
        }

        if (state->stage == SMTD_STAGE_HOLD_RELEASE) {
            SMTD_DEBUG("%s clean up", smtd_state_to_str(state));
            smtd_handle_action(state, SMTD_ACTION_RELEASE);
            smtd_apply_stage(state, SMTD_STAGE_NONE);
            idx = smtd_active_states_size;
            continue;
        }

        if (state->stage == SMTD_STAGE_SEQUENCE) {
            idx--;
            continue;
        }

        SMTD_DEBUG("%s clean up is not required", smtd_state_to_str(state));
        break;
    }
    SMTD_DEBUG_OFFSET_DEC;

    if (processed_state) {
        SMTD_DEBUG("<< %s STATE PROCESSED", smtd_record_to_str(record));
        SMTD_DEBUG_FULL();
        return;
    }

    smtd_create_state(pressed_keycode, record, desired_keycode);
}

void smtd_create_state(uint16_t pressed_keycode, keyrecord_t *record, uint16_t desired_keycode) {
    // may be start a new state? A key must be just pressed
    if (!record->event.pressed) {
        SMTD_DEBUG("<< %s BYPASS KEY RELEASE", smtd_record_to_str(record));
        SMTD_DEBUG_FULL();
        return;
    }

    // create a new state and process the event
    smtd_state *state = NULL;
    for (uint8_t i = 0; i < SMTD_POOL_SIZE; i++) {
        if (smtd_states_pool[i].stage == SMTD_STAGE_NONE) {
            state = &smtd_states_pool[i];
            break;
        }
    }

    if (!state || state == NULL) {
        SMTD_DEBUG("<< %s NO FREE STATES",
                   smtd_record_to_str(record));
        SMTD_DEBUG_FULL();
        return;
    }

    smtd_active_states[smtd_active_states_size] = state;
    state->idx = smtd_active_states_size;
    state->pressed_keycode = pressed_keycode;
    state->pressed_keyposition = record->event.key;
    if (desired_keycode > 0) {
        state->desired_keycode = desired_keycode;
    }
    smtd_active_states_size++;

    SMTD_DEBUG_OFFSET_INC;
    smtd_apply_event(true, state, pressed_keycode, record);
    SMTD_DEBUG_OFFSET_DEC;

    SMTD_DEBUG("<< %s CREATE STATE %s",
               smtd_record_to_str(record),
               smtd_state_to_str(state));
    SMTD_DEBUG_FULL();
}

bool is_following_key(smtd_state *state, uint16_t pressed_keycode, keyrecord_t *record) {
    for (uint8_t i = state->idx + 1; i < smtd_active_states_size; i++) {
        bool is_following_state_key =
                (record->event.key.row == smtd_active_states[i]->pressed_keyposition.row &&
                 record->event.key.col == smtd_active_states[i]->pressed_keyposition.col) &&
                (pressed_keycode == smtd_active_states[i]->pressed_keycode ||
                 pressed_keycode == smtd_active_states[i]->desired_keycode);
        if (is_following_state_key) {
            return true;
        }
    }

    return false;
}

void smtd_apply_event(bool is_state_key, smtd_state *state, uint16_t pressed_keycode,
                      keyrecord_t *record) {
    SMTD_DEBUG("--%s apply_event with %s, is_state_key=%d",
               smtd_state_to_str(state),
               smtd_record_to_str(record),
               is_state_key);

    SMTD_DEBUG_OFFSET_INC;

    switch (state->stage) {
        // -----------------------------------------------------------------------------------------
        case SMTD_STAGE_NONE: {
            if (is_state_key && record->event.pressed) {
                smtd_apply_stage(state, SMTD_STAGE_TOUCH);
                smtd_handle_action(state, SMTD_ACTION_TOUCH);
                break;
            }
            break;
        } // case SMTD_STAGE_NONE

        // -----------------------------------------------------------------------------------------
        case SMTD_STAGE_TOUCH: {
            if (state->idx + 1 == smtd_active_states_size) {
                // last state in stack
                if (is_state_key && !record->event.pressed) {
                    if (!smtd_feature_enabled_or_default(state, SMTD_FEATURE_AGGREGATE_TAPS)) {
                        smtd_handle_action(state, SMTD_ACTION_TAP);
                    }

                    smtd_apply_stage(state, SMTD_STAGE_SEQUENCE);
                    break;
                }

                break;
            }

            if (is_state_key && !record->event.pressed) {
                // Macro key is released, moving to the next stage
                smtd_apply_stage(state, SMTD_STAGE_TOUCH_RELEASE);
                break;
            }

            if (!is_following_key(state, pressed_keycode, record)) {
                // Some previously pressed key has been released
                // We don't need to do anything here
                break;
            }

            if (!is_state_key && !record->event.pressed) {
                // Following key is released. Now we definitely know that macro key is held
                // we need to execute hold the macro key and let following state handle the key release
                smtd_apply_stage(state, SMTD_STAGE_HOLD);
                smtd_handle_action(state, SMTD_ACTION_HOLD);
                break;
            }

            break;
        } // case SMTD_STAGE_TOUCH

        // -----------------------------------------------------------------------------------------
        case SMTD_STAGE_SEQUENCE: {
            if (is_state_key && record->event.pressed) {
                //fixme move to the end of states? or drop if not the last?
                state->sequence_len++;
                smtd_handle_action(state, SMTD_ACTION_TOUCH);
                smtd_apply_stage(state, SMTD_STAGE_TOUCH);
                break;
            }

            if (!is_state_key && record->event.pressed) {
                state->resolution = SMTD_RESOLUTION_DETERMINED;
                if (smtd_feature_enabled_or_default(state, SMTD_FEATURE_AGGREGATE_TAPS)) {
                    smtd_handle_action(state, SMTD_ACTION_TAP);
                }
                smtd_apply_stage(state, SMTD_STAGE_NONE);
                break;
            }
            break;
        } // case SMTD_STAGE_SEQUENCE

        // -----------------------------------------------------------------------------------------
        case SMTD_STAGE_HOLD: {
            if (is_state_key && !record->event.pressed) {
                if (state->idx == smtd_active_states_size - 1) {
                    smtd_handle_action(state, SMTD_ACTION_RELEASE);
                    smtd_apply_stage(state, SMTD_STAGE_NONE);
                    break;
                }

                smtd_apply_stage(state, SMTD_STAGE_HOLD_RELEASE);
                break;
            }

            break;
        } // case SMTD_STAGE_HOLD

        // -----------------------------------------------------------------------------------------
        case SMTD_STAGE_TOUCH_RELEASE: {
            // At this stage we have just released the macro key and still holding the following key

            if (record->event.pressed) {
                // Another or Same key has just pressed again. We consider that we are in a sequence of taps
                // So current state is interpreted as tap action. And next tap should be handled in another state.
                // fixme test this case
                smtd_handle_action(state, SMTD_ACTION_TAP);
                smtd_apply_stage(state, SMTD_STAGE_NONE);
                break;
            }

            if (timer_elapsed32(state->released_time) >= get_smtd_timeout_or_default(state, SMTD_TIMEOUT_RELEASE)) {
                // Timeout has been reached, but timeout_touch_release has not been executed yet
                SMTD_DEBUG("%s timeout_touch_release has not been executed yet",
                           smtd_state_to_str(state));
                smtd_handle_action(state, SMTD_ACTION_TAP);
                smtd_apply_stage(state, SMTD_STAGE_NONE);
                break;
            }

            if (!is_following_key(state, pressed_keycode, record)) {
                // Some previously pressed key has been released
                // We don't need to do anything here
                break;
            }

            // Following key is released. Now we definitely know that macro key is held
            // we need to execute hold the macro key
            smtd_apply_stage(state, SMTD_STAGE_HOLD_RELEASE);
            smtd_handle_action(state, SMTD_ACTION_HOLD);

            break;
        } // case SMTD_STAGE_TOUCH_RELEASE

        // -----------------------------------------------------------------------------------------
        case SMTD_STAGE_HOLD_RELEASE: {
            // At this stage we have just released the macro key (which was held)
            // and still holding the following key (or keys)
            if (!record->event.pressed && state->idx != smtd_active_states_size - 1) {
                break;
            }

            // A key was pressed. Can't hold current state anymore
            smtd_handle_action(state, SMTD_ACTION_RELEASE);
            smtd_apply_stage(state, SMTD_STAGE_NONE);
            break;
        } // case SMTD_STAGE_HOLD_RELEASE
    }

    SMTD_DEBUG_OFFSET_DEC;
}

void reset_state(smtd_state *state) {
    state->stage = SMTD_STAGE_NONE;
    state->pressed_keyposition = MAKE_KEYPOS(0, 0);
    state->pressed_keycode = 0;
    state->desired_keycode = 0;
    state->sequence_len = 0;
    state->pressed_time = 0;
    state->released_time = 0;
    state->timeout = INVALID_DEFERRED_TOKEN;
    state->resolution = SMTD_RESOLUTION_UNCERTAIN;
    state->idx = 0;
    state->next_action = SMTD_ACTION_TOUCH;
    state->need_next_action = false;
}

void smtd_apply_stage(smtd_state *state, smtd_stage next_stage) {
    SMTD_DEBUG("%s stage -> %s",
               smtd_state_to_str(state),
               smtd_stage_to_str(next_stage));

    deferred_token prev_token = state->timeout;
    state->timeout = INVALID_DEFERRED_TOKEN;
    state->stage = next_stage;

    switch (state->stage) {
        case SMTD_STAGE_NONE:
            for (uint8_t j = state->idx; j < smtd_active_states_size - 1; j++) {
                smtd_active_states[j] = smtd_active_states[j + 1];
                smtd_active_states[j]->idx--;
            }

            smtd_active_states_size--;
            smtd_active_states[smtd_active_states_size] = NULL;
            reset_state(state);
            break;

        case SMTD_STAGE_TOUCH:
            state->pressed_time = timer_read32();
            state->timeout = defer_exec(get_smtd_timeout_or_default(state, SMTD_TIMEOUT_TAP),
                                        timeout_touch, state);
            SMTD_DEBUG("%s timeout_touch in %lums", smtd_state_to_str(state),
                       get_smtd_timeout_or_default(state, SMTD_TIMEOUT_TAP));
            break;

        case SMTD_STAGE_SEQUENCE:
            state->released_time = timer_read32();
            state->resolution = SMTD_RESOLUTION_UNCERTAIN;
            state->timeout = defer_exec(get_smtd_timeout_or_default(state, SMTD_TIMEOUT_SEQUENCE),
                                        timeout_sequence, state);
            SMTD_DEBUG("%s timeout_sequence in %lums", smtd_state_to_str(state),
                       get_smtd_timeout_or_default(state, SMTD_TIMEOUT_SEQUENCE));
            break;

        case SMTD_STAGE_HOLD:
            break;

        case SMTD_STAGE_TOUCH_RELEASE:
            state->released_time = timer_read32();
            state->timeout = defer_exec(get_smtd_timeout_or_default(state, SMTD_TIMEOUT_RELEASE),
                                        timeout_touch_release, state);
            SMTD_DEBUG("%s timeout_touch_release in %lums", smtd_state_to_str(state),
                       get_smtd_timeout_or_default(state, SMTD_TIMEOUT_RELEASE));
            break;

        case SMTD_STAGE_HOLD_RELEASE:
            state->released_time = timer_read32();
            state->timeout = defer_exec(get_smtd_timeout_or_default(state, SMTD_TIMEOUT_RELEASE),
                                        timeout_hold_release, state);
            SMTD_DEBUG("%s timeout_hold_release in %lums", smtd_state_to_str(state),
                       get_smtd_timeout_or_default(state, SMTD_TIMEOUT_RELEASE));
            break;
    }

    // need to cancel after creating new timeout. There is a bug in QMK scheduling
    cancel_deferred_exec(prev_token);
}

void smtd_handle_action(smtd_state *state, smtd_action action) {
    if (smtd_worst_resolution_before(state) < SMTD_RESOLUTION_DETERMINED) {
        state->need_next_action = true;
        state->next_action = action;
        SMTD_DEBUG("%s %s is deffered",
                   smtd_state_to_str(state),
                   smtd_action_to_str(action));
        return;
    }

    SMTD_DEBUG("%s %s processing",
               smtd_state_to_str(state),
               smtd_action_to_str(action));

    smtd_resolution resolution_before_action = state->resolution;
    SMTD_DEBUG_OFFSET_INC;
    smtd_execute_action(state, action);
    SMTD_DEBUG_OFFSET_DEC;
    smtd_resolution resolution_after_action = state->resolution;

    if (resolution_before_action == SMTD_RESOLUTION_DETERMINED) {
        SMTD_DEBUG("%s %s was already determined before",
                   smtd_state_to_str(state),
                   smtd_action_to_str(action));
        return;
    }

    if (resolution_after_action != SMTD_RESOLUTION_DETERMINED) {
        SMTD_DEBUG("%s %s is not yet determined",
                   smtd_state_to_str(state),
                   smtd_action_to_str(action));
        return;
    }

    for (int i = state->idx + 1; i < smtd_active_states_size; i++) {
        if (!(smtd_active_states[i]->need_next_action)) {
            break;
        }

        smtd_state *next_state = smtd_active_states[i];

        SMTD_DEBUG("%s %s will run deferred %s",
                   smtd_state_to_str(state),
                   smtd_action_to_str(action),
                   smtd_state_to_str2(next_state));

        next_state->need_next_action = false;

        SMTD_DEBUG_OFFSET_INC;
        switch (next_state->next_action) {
            case SMTD_ACTION_TOUCH:
                smtd_handle_action(next_state, SMTD_ACTION_TOUCH);
                break;
            case SMTD_ACTION_TAP:
                smtd_handle_action(next_state, SMTD_ACTION_TOUCH);
                smtd_handle_action(next_state, SMTD_ACTION_TAP);
                break;
            case SMTD_ACTION_HOLD:
                smtd_handle_action(next_state, SMTD_ACTION_TOUCH);
                smtd_handle_action(next_state, SMTD_ACTION_HOLD);
                break;
            case SMTD_ACTION_RELEASE:
                smtd_handle_action(next_state, SMTD_ACTION_TOUCH);
                smtd_handle_action(next_state, SMTD_ACTION_HOLD);
                smtd_handle_action(next_state, SMTD_ACTION_RELEASE);
                break;
        }
        SMTD_DEBUG_OFFSET_DEC;

        SMTD_DEBUG("%s %s is complete",
                   smtd_state_to_str(state),
                   smtd_action_to_str(action));
    }
}

void smtd_execute_action(smtd_state *state, smtd_action action) {
    if (state->desired_keycode == 0) {
        state->desired_keycode = smtd_current_keycode(&state->pressed_keyposition);
    }

    SMTD_DEBUG("%s exec in progress with %s",
               smtd_state_to_str(state),
               smtd_action_to_str(action));

    smtd_bypass = true;
    smtd_resolution new_resolution = on_smtd_action(state->desired_keycode, action, state->sequence_len);
    smtd_bypass = false;

    SMTD_SIMULTANEOUS_PRESSES_DELAY
    if (new_resolution > state->resolution) {
        state->resolution = new_resolution;
    }

    if (new_resolution == SMTD_RESOLUTION_UNHANDLED) {
        SMTD_DEBUG_OFFSET_INC;
        switch (action) {
            case SMTD_ACTION_TOUCH:
                smtd_emulate_press(&state->pressed_keyposition, true);
                state->resolution = SMTD_RESOLUTION_DETERMINED;
                break;
            case SMTD_ACTION_TAP:
                smtd_emulate_press(&state->pressed_keyposition, false);
                break;
            case SMTD_ACTION_HOLD:
                break;
            case SMTD_ACTION_RELEASE:
                smtd_emulate_press(&state->pressed_keyposition, false);
                break;
        }
        SMTD_DEBUG_OFFSET_DEC;
    }

    SMTD_DEBUG("%s exec done with %s",
               smtd_state_to_str(state),
               smtd_action_to_str(action));
}


/* ************************************* *
 *      UTILITY FUNCTIONS                *
 * ************************************* */

void smtd_emulate_press(keypos_t *keypos, bool press) {
    SMTD_DEBUG("EMULATE %s %s", press ? "PRESS" : "RELEASE",
               smtd_keycode_to_str(smtd_current_keycode(keypos)));
    smtd_bypass = true;
    //fixme-sm how to emulate keypresses with row,col = (0,0) // like combos for example
    keyevent_t event_press = MAKE_KEYEVENT(keypos->row, keypos->col, press);
    keyrecord_t record_press = {.event = event_press};
    SMTD_DEBUG_OFFSET_INC;
    process_record(&record_press);
    SMTD_DEBUG_OFFSET_DEC;
    smtd_bypass = false;
    SMTD_SIMULTANEOUS_PRESSES_DELAY
}

smtd_resolution smtd_worst_resolution_before(smtd_state *state) {
    smtd_resolution result = SMTD_RESOLUTION_DETERMINED;
    for (uint8_t i = 0; i < state->idx; i++) {
        if (smtd_active_states[i]->stage == SMTD_STAGE_SEQUENCE) {
            continue;
        }

        if (smtd_active_states[i]->resolution < result) {
            result = smtd_active_states[i]->resolution;
        }
    }

    SMTD_DEBUG("worst_resolution_before: %s result %d",
               smtd_state_to_str(state), result);
    return result;
}

uint32_t get_smtd_timeout_or_default(smtd_state *state, smtd_timeout timeout) {
    if (get_smtd_timeout) {
        return get_smtd_timeout(state->desired_keycode, timeout);
    }
    return get_smtd_timeout_default(timeout);
}

uint32_t get_smtd_timeout_default(smtd_timeout timeout) {
    switch (timeout) {
        case SMTD_TIMEOUT_TAP:
            return SMTD_GLOBAL_TAP_TERM;
        case SMTD_TIMEOUT_SEQUENCE:
            return SMTD_GLOBAL_SEQUENCE_TERM;
        case SMTD_TIMEOUT_RELEASE:
            return SMTD_GLOBAL_RELEASE_TERM;
    }
    return 0;
}

uint16_t smtd_current_keycode(keypos_t *key) {
    uint8_t current_layer = get_highest_layer(layer_state);
    return keymaps[current_layer][key->row][key->col];
}

bool smtd_feature_enabled_or_default(smtd_state *state, smtd_feature feature) {
    if (smtd_feature_enabled) {
        return smtd_feature_enabled(state->desired_keycode, feature);
    }
    return smtd_feature_enabled_default(state->desired_keycode, feature);
}

bool smtd_feature_enabled_default(uint16_t keycode, smtd_feature feature) {
    switch (feature) {
        case SMTD_FEATURE_AGGREGATE_TAPS:
            return SMTD_GLOBAL_AGGREGATE_TAPS;
    }
    return false;
}


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

#define SMTD_LIMIT(limit, then, otherwise) \
    if (tap_count < limit) { then; } else { otherwise; }

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
#define SMTD_MTE2(key, mod) SMTD_MTE3_ON_MKEY(key, key, mod)
#define SMTD_MTE3(key, mod, threshold) SMTD_MTE4_ON_MKEY(key, key, mod, threshold)
#define SMTD_MTE4(key, mod, threshold, use_cl) SMTD_MTE5_ON_MKEY(key, key, mod, threshold, use_cl)
#define SMTD_MTE_ON_MKEY(...) OVERLOAD5(__VA_ARGS__, SMTD_MTE5_ON_MKEY, SMTD_MTE4_ON_MKEY, SMTD_MTE3_ON_MKEY)(__VA_ARGS__)
#define SMTD_MTE3_ON_MKEY(...) SMTD_MTE4_ON_MKEY(__VA_ARGS__, 1)
#define SMTD_MTE4_ON_MKEY(...) SMTD_MTE5_ON_MKEY(__VA_ARGS__, true)
#define SMTD_MTE5_ON_MKEY(macro_key, tap_key, mod, threshold, use_cl)\
    SMTD_DANCE(macro_key,                                    \
        EXEC(                                                \
            register_mods(MOD_BIT(mod));                     \
            send_keyboard_report();                          \
        ),                                                   \
        EXEC(                                                \
            unregister_mods(MOD_BIT(mod));                   \
            SMTD_TAP_16(use_cl, tap_key);                    \
        ),                                                   \
        SMTD_LIMIT(threshold,                                \
            NOTHING,                                         \
            EXEC(                                            \
                unregister_mods(MOD_BIT(mod));               \
                send_keyboard_report();                      \
                SMTD_REGISTER_16(use_cl, tap_key);           \
            )                                                \
        ),                                                   \
        SMTD_LIMIT(threshold,                                \
            EXEC(                                            \
                unregister_mods(MOD_BIT(mod));               \
                send_keyboard_report();                      \
            ),                                               \
            SMTD_UNREGISTER_16(use_cl, tap_key)              \
        )                                                    \
    )

//fixme попробовать отказаться от самописной смены слоя
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

// fixme-sm SMTD_TK and others without ON_MKEY
// multi-tap activated key
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

void avoid_unused_variable_on_compile(void *ptr) {
    // just touch them, so compiler won't throw "defined but not used" error
    // that variables are used in macros that user may not use
    if (return_layer == RETURN_LAYER_NOT_SET) return_layer = RETURN_LAYER_NOT_SET;
    if (return_layer_cnt == 0) return_layer_cnt = 0;
}

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
