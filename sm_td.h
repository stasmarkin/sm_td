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
 * Version: 0.5.0-RC1
 * Date: 2024-12-26
 */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include QMK_KEYBOARD_H
#include "deferred_exec.h"

#ifdef SMTD_DEBUG_ENABLED
#include "print.h"
#endif

#ifdef SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS
#include "timer.h"
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

#ifndef SMTD_GLOBAL_FOLLOWING_TAP_TERM
#define SMTD_GLOBAL_FOLLOWING_TAP_TERM TAPPING_TERM
#endif

#ifndef SMTD_GLOBAL_RELEASE_TERM
#define SMTD_GLOBAL_RELEASE_TERM TAPPING_TERM / 4
#endif

#ifndef SMTD_GLOBAL_AGGREGATE_TAPS
#define SMTD_GLOBAL_AGGREGATE_TAPS false
#endif


/* ************************************* *
 *         BASE DEFINITIONS              *
 * ************************************* */

typedef enum {
    SMTD_STAGE_NONE,
    SMTD_STAGE_TOUCH,
    SMTD_STAGE_SEQUENCE,
    SMTD_STAGE_FOLLOWING_TOUCH,
    SMTD_STAGE_HOLD,
    SMTD_STAGE_RELEASE,
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
    SMTD_TIMEOUT_FOLLOWING_TAP,
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

    /** The mods that were active on last key action */
    uint8_t saved_mods;

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
        .saved_mods = 0,                            \
        .sequence_len = 0,                          \
        .pressed_time = 0,                          \
        .released_time = 0,                         \
        .timeout = INVALID_DEFERRED_TOKEN,          \
        .stage = SMTD_STAGE_NONE,                   \
        .resolution = SMTD_RESOLUTION_UNCERTAIN,    \
        .next_action = SMTD_ACTION_TOUCH,           \
        .need_next_action = false,                  \
        .idx = 0                                    \
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


/* ************************************* *
 *           INTERNAL FUNCTIONS          *
 * ************************************* */

bool smtd_process_desired(uint16_t pressed_keycode, keyrecord_t *record, uint16_t desired_keycode);

void smtd_apply_to_stack(uint8_t starting_idx, uint16_t pressed_keycode, keyrecord_t *record, uint16_t desired_keycode);

void smtd_create_state(uint16_t pressed_keycode, keyrecord_t *record, uint16_t desired_keycode);

bool smtd_apply_event(bool is_state_key, smtd_state *state, uint16_t pressed_keycode, keyrecord_t *record,
                      uint16_t desired_keycode);

void smtd_apply_stage(smtd_state *state, smtd_stage next_stage);

void smtd_handle_action(smtd_state *state, smtd_action action);

void smtd_execute_action(smtd_state *state, smtd_action action);

void smtd_emulate_press(keypos_t *keypos, bool press);

void smtd_propagate_mods(smtd_state *state, uint8_t mods_before_action, uint8_t mods_after_action);

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

#define SMTD_DEBUG(...) printf(__VA_ARGS__)

char *smtd_stage_to_str(smtd_stage stage) {
    switch (stage) {
        case SMTD_STAGE_NONE:
            return "S_NON";
        case SMTD_STAGE_TOUCH:
            return "S_TCH";
        case SMTD_STAGE_SEQUENCE:
            return "S_SEQ";
        case SMTD_STAGE_FOLLOWING_TOUCH:
            return "S_FOL";
        case SMTD_STAGE_HOLD:
            return "S_HLD";
        case SMTD_STAGE_RELEASE:
            return "S_RLS";
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
            snprintf(buffer_keycode, sizeof(buffer_keycode), uncertain ? "?%s" : "%s", result);
            return result;
        }
    }

    snprintf(buffer_keycode, sizeof(buffer_keycode), uncertain ? "?KC_%d" : "KC_%d", keycode);
    return buffer_keycode;
}

char* smtd_keycode_to_str(uint16_t keycode) {
    return smtd_keycode_to_str_uncertain(keycode, false);
}

char* smtd_state_to_str(smtd_state *state) {
    static char buffer_state[64];

    snprintf(buffer_state, sizeof(buffer_state), "S[%d](@%d.%d#%s->%s){%s/%s,m=%x}",
             state->idx,
             state->pressed_keyposition.row,
             state->pressed_keyposition.col,
             smtd_keycode_to_str(state->pressed_keycode),
             smtd_keycode_to_str(state->desired_keycode),
             smtd_stage_to_str(state->stage),
             smtd_resolution_to_str(state->resolution),
             state->saved_mods);

    return buffer_state;
}

char* smtd_state_to_str2(smtd_state *state) {
    static char buffer_state2[64];

    snprintf(buffer_state2, sizeof(buffer_state2), "S[%d](@%d.%d#%s->%s){%s/%s,m=%x}",
             state->idx,
             state->pressed_keyposition.row,
             state->pressed_keyposition.col,
             smtd_keycode_to_str(state->pressed_keycode),
             smtd_keycode_to_str(state->desired_keycode),
             smtd_stage_to_str(state->stage),
             smtd_resolution_to_str(state->resolution),
             state->saved_mods);

    return buffer_state2;
}

char* smtd_record_to_str(keyrecord_t *record) {
    static char buffer_record[16];

    snprintf(buffer_record, sizeof(buffer_record), "R(@%d#%d %s)",
             record->event.key.row,
             record->event.key.col,
             record->event.pressed ? "|*|" : "|O|");

    return buffer_record;
}
#else

#define SMTD_DEBUG(...)

#endif


/* ************************************* *
 *             TIMEOUTS                  *
 * ************************************* */

uint32_t timeout_reset_seq(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    SMTD_DEBUG("\n      %s timeout_reset_seq\n", smtd_state_to_str(state));
    state->sequence_len = 0;
    return 0;
}

uint32_t timeout_touch(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    SMTD_DEBUG("\n      %s timeout_touch\n", smtd_state_to_str(state));
    smtd_apply_stage(state, SMTD_STAGE_HOLD);
    smtd_handle_action(state, SMTD_ACTION_HOLD);
    return 0;
}

uint32_t timeout_sequence(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    SMTD_DEBUG("\n      %s timeout_sequence\n", smtd_state_to_str(state));
    if (smtd_feature_enabled_or_default(state, SMTD_FEATURE_AGGREGATE_TAPS)) {
        smtd_handle_action(state, SMTD_ACTION_TAP);
    }
    smtd_apply_stage(state, SMTD_STAGE_NONE);
    return 0;
}

uint32_t timeout_following_touch(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    SMTD_DEBUG("\n      %s timeout_following_touch\n", smtd_state_to_str(state));
    smtd_apply_stage(state, SMTD_STAGE_HOLD);
    smtd_handle_action(state, SMTD_ACTION_HOLD);
    return 0;
}

uint32_t timeout_release(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    SMTD_DEBUG("\n      %s timeout_release\n", smtd_state_to_str(state));
    smtd_handle_action(state, SMTD_ACTION_TAP);
    smtd_apply_stage(state, SMTD_STAGE_NONE);
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
        SMTD_DEBUG("            %s GLOBAL BYPASS KEY %s\n",
                   smtd_record_to_str(record),
                   smtd_keycode_to_str_uncertain(pressed_keycode, desired_keycode == 0));
        return true;
    }

#ifdef SMTD_DEBUG_ENABLED
    SMTD_DEBUG("\n>> +%lums %s GOT KEY %s\n",
               timer_elapsed32(last_key_timer),
               smtd_record_to_str(record),
               smtd_keycode_to_str_uncertain(pressed_keycode, desired_keycode == 0));
    last_key_timer = timer_read32();
#endif

    smtd_apply_to_stack(0, pressed_keycode, record, desired_keycode);
    return false;
}

void
smtd_apply_to_stack(uint8_t starting_idx, uint16_t pressed_keycode, keyrecord_t *record, uint16_t desired_keycode) {
    SMTD_DEBUG("    %s apply_to_stack starting idx=%d\n",
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

        bool can_process_next = smtd_apply_event(is_state_key, state, pressed_keycode, record, desired_keycode);

        if (!can_process_next) {
            SMTD_DEBUG("<< %s TERM STATE with %s\n",
                       smtd_record_to_str(record),
                       smtd_state_to_str(state));
            break;
        }
    }

    if (processed_state) {
        SMTD_DEBUG("<< %s STATE PROCESSED\n",
                   smtd_record_to_str(record));
        return;
    }

    smtd_create_state(pressed_keycode, record, desired_keycode);
}

void smtd_create_state(uint16_t pressed_keycode, keyrecord_t *record, uint16_t desired_keycode) {
    // may be start a new state? A key must be just pressed
    if (!record->event.pressed) {
        SMTD_DEBUG("<< %s BYPASS KEY RELEASE\n", smtd_record_to_str(record));
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
        SMTD_DEBUG("<< %s NO FREE STATES\n",
                   smtd_record_to_str(record));
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

    smtd_apply_event(true, state, pressed_keycode, record, desired_keycode);
    SMTD_DEBUG("<< %s CREATE STATE %s\n",
               smtd_record_to_str(record),
               smtd_state_to_str(state));
}

bool smtd_apply_event(bool is_state_key, smtd_state *state, uint16_t pressed_keycode,
                      keyrecord_t *record, uint16_t desired_keycode) {
    SMTD_DEBUG("   -- %s apply_event with %s, is_state_key=%d\n",
               smtd_state_to_str(state),
               smtd_record_to_str(record),
               is_state_key);

    switch (state->stage) {
        case SMTD_STAGE_NONE:
            if (is_state_key && record->event.pressed) {
                state->saved_mods = get_mods();
                smtd_apply_stage(state, SMTD_STAGE_TOUCH);
                smtd_handle_action(state, SMTD_ACTION_TOUCH);
                break;
            }
            break;

        case SMTD_STAGE_TOUCH:
            if (is_state_key && !record->event.pressed) {
                if (!smtd_feature_enabled_or_default(state, SMTD_FEATURE_AGGREGATE_TAPS)) {
                    smtd_handle_action(state, SMTD_ACTION_TAP);
                }

                smtd_apply_stage(state, SMTD_STAGE_SEQUENCE);
                break;
            }

            if (!is_state_key && record->event.pressed) {
                smtd_apply_stage(state, SMTD_STAGE_FOLLOWING_TOUCH);
                break;
            }
            break;

        case SMTD_STAGE_SEQUENCE:
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

        case SMTD_STAGE_FOLLOWING_TOUCH:
            // At this stage, we have already pressed the macro key and the following key
            // none of them is assumed to be held yet

            if (is_state_key && !record->event.pressed) {
                // Macro key is released, moving to the next stage
                smtd_apply_stage(state, SMTD_STAGE_RELEASE);
                break;
            }

            if (!is_state_key && !record->event.pressed) {
                // Another key has been released
                smtd_state *following_key_state = NULL;
                for (uint8_t i = state->idx + 1; i < smtd_active_states_size; i++) {
                    bool is_following_state_key =
                            (record->event.key.row == smtd_active_states[i]->pressed_keyposition.row &&
                             record->event.key.col == smtd_active_states[i]->pressed_keyposition.col) &&
                            (pressed_keycode == smtd_active_states[i]->pressed_keycode ||
                             pressed_keycode == smtd_active_states[i]->desired_keycode);
                    if (is_following_state_key) {
                        following_key_state = smtd_active_states[i];
                        break;
                    }
                }

                if (following_key_state == NULL) {
                    // Some previously pressed key has been released
                    // We don't need to do anything here
                    break;
                }

                // Following key is released. Now we definitely know that macro key is held
                // we need to execute hold the macro key and let following state handle the key release
                smtd_apply_stage(state, SMTD_STAGE_HOLD);
                smtd_handle_action(state, SMTD_ACTION_HOLD);
                SMTD_SIMULTANEOUS_PRESSES_DELAY
                break;
            }
            break;

        case SMTD_STAGE_HOLD:
            if (is_state_key && !record->event.pressed) {
                smtd_handle_action(state, SMTD_ACTION_RELEASE);
                smtd_apply_stage(state, SMTD_STAGE_NONE);
                break;
            }
            break;

        case SMTD_STAGE_RELEASE:
            // At this stage we have just released the macro key and still holding the following key

            if (is_state_key) {
                if (!record->event.pressed) {
                    // Macro key is released again. We should not be here, since we have already release state
                    SMTD_DEBUG("        %s how could that happen? %s, is_state_key=%d\n",
                               smtd_state_to_str(state),
                               smtd_record_to_str(record),
                               is_state_key);
                    break;
                }

                // Same key has just pressed again. We consider that we are in a sequence of taps
                // So current state is interpreted as tap action. And next tap should be handled in another state.
                smtd_handle_action(state, SMTD_ACTION_TAP);

                uint8_t idx = state->idx;

                smtd_apply_stage(state, SMTD_STAGE_NONE);
                SMTD_SIMULTANEOUS_PRESSES_DELAY

                // let that new press event be processed by the next state
                smtd_apply_to_stack(idx, pressed_keycode, record, desired_keycode);
                return false;
            }

            // if (!is_state_key && record->event.pressed) { -- is_state_key == false always here
            if (record->event.pressed) {
                break;
            }

            if (timer_elapsed32(state->released_time) >= get_smtd_timeout_or_default(state, SMTD_TIMEOUT_RELEASE)) {
                // Timeout has been reached, but timeout_release has not been executed yet
                SMTD_DEBUG("        %s timeout_release has not been executed yet\n",
                           smtd_state_to_str(state));

                uint8_t idx = state->idx;

                smtd_handle_action(state, SMTD_ACTION_TAP);
                SMTD_SIMULTANEOUS_PRESSES_DELAY
                smtd_apply_stage(state, SMTD_STAGE_NONE);

                smtd_apply_to_stack(idx, pressed_keycode, record, desired_keycode);
                return false;
            }

            smtd_state *following_key_state = NULL;
            for (uint8_t i = state->idx + 1; i < smtd_active_states_size; i++) {
                bool is_following_state_key =
                        (record->event.key.row == smtd_active_states[i]->pressed_keyposition.row &&
                         record->event.key.col == smtd_active_states[i]->pressed_keyposition.col) &&
                        (pressed_keycode == smtd_active_states[i]->pressed_keycode ||
                         pressed_keycode == smtd_active_states[i]->desired_keycode);
                if (is_following_state_key) {
                    following_key_state = smtd_active_states[i];
                    break;
                }
            }

            if (following_key_state != NULL) {
                // Following key is released. Now we definitely know that macro key is held
                // we need to execute hold the macro key and let following state handle the key release

                smtd_apply_stage(state, SMTD_STAGE_HOLD);
                smtd_handle_action(state, SMTD_ACTION_HOLD);
                SMTD_SIMULTANEOUS_PRESSES_DELAY

                smtd_apply_to_stack(state->idx + 1, pressed_keycode, record, desired_keycode);
                SMTD_SIMULTANEOUS_PRESSES_DELAY

                smtd_handle_action(state, SMTD_ACTION_RELEASE);
                smtd_apply_stage(state, SMTD_STAGE_NONE);
                return false;
            }
            break;
    }

    return true;
}

void smtd_apply_stage(smtd_state *state, smtd_stage next_stage) {
    SMTD_DEBUG("      %s stage -> %s\n",
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

            state->pressed_keyposition = MAKE_KEYPOS(0, 0);
            state->pressed_keycode = 0;
            state->desired_keycode = 0;
            state->saved_mods = 0;
            state->sequence_len = 0;
            state->pressed_time = 0;
            state->released_time = 0;
            state->timeout = INVALID_DEFERRED_TOKEN;
            state->resolution = SMTD_RESOLUTION_UNCERTAIN;
            state->idx = 0;
            state->next_action = SMTD_ACTION_TOUCH;
            state->need_next_action = false;
            break;

        case SMTD_STAGE_TOUCH:
            state->pressed_time = timer_read32();
            state->timeout = defer_exec(get_smtd_timeout_or_default(state, SMTD_TIMEOUT_TAP),
                                        timeout_touch, state);
            SMTD_DEBUG("      %s timeout_touch in %lums\n", smtd_state_to_str(state),
                       get_smtd_timeout_or_default(state, SMTD_TIMEOUT_TAP));
            break;

        case SMTD_STAGE_SEQUENCE:
            state->released_time = timer_read32();
            state->resolution = SMTD_RESOLUTION_UNCERTAIN;
            state->saved_mods = get_mods();
            state->timeout = defer_exec(get_smtd_timeout_or_default(state, SMTD_TIMEOUT_SEQUENCE),
                                        timeout_sequence, state);
            SMTD_DEBUG("      %s timeout_sequence in %lums\n", smtd_state_to_str(state),
                       get_smtd_timeout_or_default(state, SMTD_TIMEOUT_SEQUENCE));
            break;

        case SMTD_STAGE_HOLD:
            break;

        case SMTD_STAGE_FOLLOWING_TOUCH:
            state->timeout = defer_exec(get_smtd_timeout_or_default(state, SMTD_TIMEOUT_FOLLOWING_TAP),
                                        timeout_following_touch, state);
            SMTD_DEBUG("      %s timeout_following_touch in %lums\n", smtd_state_to_str(state),
                       get_smtd_timeout_or_default(state, SMTD_TIMEOUT_FOLLOWING_TAP));
            break;

        case SMTD_STAGE_RELEASE:
            state->released_time = timer_read32();
            state->timeout = defer_exec(get_smtd_timeout_or_default(state, SMTD_TIMEOUT_RELEASE),
                                        timeout_release, state);
            SMTD_DEBUG("      %s timeout_release in %lums\n", smtd_state_to_str(state),
                       get_smtd_timeout_or_default(state, SMTD_TIMEOUT_RELEASE));
            break;
    }

    // need to cancel after creating new timeout. There is a bug in QMK scheduling
    cancel_deferred_exec(prev_token);
}

void smtd_handle_action(smtd_state *state, smtd_action action) {
    SMTD_DEBUG("        %s action processing with %s\n",
               smtd_state_to_str(state),
               smtd_action_to_str(action));

    if (smtd_worst_resolution_before(state) < SMTD_RESOLUTION_DETERMINED) {
        state->need_next_action = true;
        state->next_action = action;
        SMTD_DEBUG("        %s action is deffered with %s\n",
                   smtd_state_to_str(state),
                   smtd_action_to_str(action));
        return;
    }

    smtd_resolution resolution_before_action = state->resolution;
    smtd_execute_action(state, action);
    smtd_resolution resolution_after_action = state->resolution;

    if (!(resolution_before_action < SMTD_RESOLUTION_DETERMINED &&
          SMTD_RESOLUTION_DETERMINED == resolution_after_action)) {
        SMTD_DEBUG("        %s action is complete by not just got determined with %s\n",
                   smtd_state_to_str(state),
                   smtd_action_to_str(action));
        return;
    }

    smtd_state *next_state = NULL;
    for (int i = state->idx + 1; i < smtd_active_states_size; i++) {
        if (smtd_active_states[i]->need_next_action) {
            next_state = smtd_active_states[i];
            break;
        }
    }

    if (next_state == NULL) {
        SMTD_DEBUG("        %s action is complete by last with %s\n",
                   smtd_state_to_str(state),
                   smtd_action_to_str(action));
        return;
    }

    SMTD_DEBUG("        %s action with %s runs deferred %s\n",
               smtd_state_to_str(state),
               smtd_action_to_str(action),
               smtd_state_to_str2(next_state));

    next_state->need_next_action = false;

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

    SMTD_DEBUG("        %s action is complete with %s\n",
               smtd_state_to_str(state),
               smtd_action_to_str(action));
}

void smtd_execute_action(smtd_state *state, smtd_action action) {
    if (state->desired_keycode == 0) {
        state->desired_keycode = smtd_current_keycode(&state->pressed_keyposition);
    }

    SMTD_DEBUG("          %s exec in progress with %s\n",
               smtd_state_to_str(state),
               smtd_action_to_str(action));

    uint8_t mods_on_start = get_mods();

    if (state->saved_mods != mods_on_start) {
        set_mods(state->saved_mods);
        send_keyboard_report();
        SMTD_SIMULTANEOUS_PRESSES_DELAY
    }

    uint8_t mods_on_restore = state->saved_mods;

    smtd_resolution new_resolution = on_smtd_action(state->desired_keycode, action, state->sequence_len);
    if (new_resolution > state->resolution) {
        state->resolution = new_resolution;
    }

    if (new_resolution == SMTD_RESOLUTION_UNHANDLED) {
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
    }

    uint8_t mods_after_action = get_mods();

    if (mods_on_restore != mods_after_action) {
        state->saved_mods = mods_after_action;
        smtd_propagate_mods(state, mods_on_restore, mods_after_action);
    }

    if (mods_on_start != mods_on_restore) {
        uint8_t changed_mods = mods_on_start ^ mods_on_restore;
        uint8_t enabled_mods = mods_on_start & changed_mods;
        uint8_t disabled_mods = mods_on_restore & changed_mods;

        uint8_t current_mods = get_mods();
        current_mods |= enabled_mods;
        current_mods &= ~disabled_mods;

        set_mods(current_mods);
        send_keyboard_report();
        SMTD_SIMULTANEOUS_PRESSES_DELAY
    }

    SMTD_DEBUG("          %s exec done with %s\n",
               smtd_state_to_str(state),
               smtd_action_to_str(action));
}


void smtd_propagate_mods(smtd_state *state, uint8_t mods_before_action, uint8_t mods_after_action) {
    uint8_t changed_mods = mods_after_action ^ mods_before_action;
    uint8_t enabled_mods = mods_after_action & changed_mods;
    uint8_t disabled_mods = mods_before_action & changed_mods;

    SMTD_DEBUG("            %s mods: before:%x  after:%x  ^^:%x  ++:%x  --:%x\n",
               smtd_state_to_str(state), mods_before_action, mods_after_action,
               changed_mods, enabled_mods, disabled_mods);

    for (uint8_t i = state->idx + 1; i < smtd_active_states_size; i++) {
        SMTD_DEBUG("            %s mods upd %s by +%x -%x\n",
                   smtd_state_to_str(state),
                   smtd_state_to_str2(smtd_active_states[i]),
                   enabled_mods,
                   disabled_mods);

        smtd_active_states[i]->saved_mods |= enabled_mods;
        smtd_active_states[i]->saved_mods &= ~disabled_mods;

        SMTD_DEBUG("            %s changed mods %s\n",
                   smtd_state_to_str(state),
                   smtd_state_to_str2(smtd_active_states[i]));
    }
}


/* ************************************* *
 *      UTILITY FUNCTIONS                *
 * ************************************* */

void smtd_emulate_press(keypos_t *keypos, bool press) {
    SMTD_DEBUG("            EMULATE %s %s\n", press ? "PRESS" : "RELEASE",
               smtd_keycode_to_str(smtd_current_keycode(keypos)));
    smtd_bypass = true;
    //fixme-sm how to emulate keypresses with row,col = (0,0) // like combos for example
    keyevent_t event_press = MAKE_KEYEVENT(keypos->row, keypos->col, press);
    keyrecord_t record_press = {.event = event_press};
    process_record(&record_press);
    smtd_bypass = false;
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

    SMTD_DEBUG("        worst_resolution_before: %s result %d\n",
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
        case SMTD_TIMEOUT_FOLLOWING_TAP:
            return SMTD_GLOBAL_FOLLOWING_TAP_TERM;
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

#define SMTD_GET_MACRO(_1, _2, _3, _4, _5, NAME, ...) NAME
#define SMTD_MT(...) SMTD_GET_MACRO(__VA_ARGS__, SMTD_MT5, SMTD_MT4, SMTD_MT3)(__VA_ARGS__)
#define SMTD_MTE(...) SMTD_GET_MACRO(__VA_ARGS__, SMTD_MTE5, SMTD_MTE4, SMTD_MTE3)(__VA_ARGS__)
#define SMTD_LT(...) SMTD_GET_MACRO(__VA_ARGS__, SMTD_LT5, SMTD_LT4, SMTD_LT3)(__VA_ARGS__)

#define SMTD_MT3(macro_kc, tap_key, mod) SMTD_MT4(macro_kc, tap_key, mod, 1000)
#define SMTD_MTE3(macro_kc, tap_key, mod) SMTD_MTE4(macro_kc, tap_key, mod, 1000)
#define SMTD_LT3(macro_kc, tap_key, layer) SMTD_LT4(macro_kc, tap_key, layer, 1000)

#define SMTD_MT4(macro_kc, tap_key, mod, threshold) SMTD_MT5(macro_kc, tap_key, mod, threshold, true)
#define SMTD_MTE4(macro_kc, tap_key, mod, threshold) SMTD_MTE5(macro_kc, tap_key, mod, threshold, true)
#define SMTD_LT4(macro_kc, tap_key, layer, threshold) SMTD_LT5(macro_kc, tap_key, layer, threshold, true)

#define SMTD_MT5(macro_kc, tap_key, mod, threshold, use_cl)   \
    case macro_kc: {                                          \
        switch (action) {                                     \
            case SMTD_ACTION_TOUCH:                           \
                return SMTD_RESOLUTION_UNCERTAIN;             \
            case SMTD_ACTION_TAP:                             \
                SMTD_TAP_16(use_cl, tap_key);                 \
                return SMTD_RESOLUTION_DETERMINED;            \
            case SMTD_ACTION_HOLD:                            \
                if (tap_count < threshold) {                  \
                    register_mods(MOD_BIT(mod));              \
                } else {                                      \
                    SMTD_REGISTER_16(use_cl, tap_key);        \
                }                                             \
                return SMTD_RESOLUTION_DETERMINED;            \
            case SMTD_ACTION_RELEASE:                         \
                if (tap_count < threshold) {                  \
                    unregister_mods(MOD_BIT(mod));            \
                } else {                                      \
                    SMTD_UNREGISTER_16(use_cl, tap_key);      \
                    send_keyboard_report();                   \
                }                                             \
                return SMTD_RESOLUTION_DETERMINED;            \
        }                                                     \
        break;                                                \
    }

#define SMTD_MTE5(macro_kc, tap_key, mod, threshold, use_cl)  \
    case macro_kc: {                                          \
        switch (action) {                                     \
            case SMTD_ACTION_TOUCH:                           \
                register_mods(MOD_BIT(mod));                  \
                return SMTD_RESOLUTION_UNCERTAIN;             \
            case SMTD_ACTION_TAP:                             \
                unregister_mods(MOD_BIT(mod));                \
                SMTD_TAP_16(use_cl, tap_key);                 \
                return SMTD_RESOLUTION_DETERMINED;            \
            case SMTD_ACTION_HOLD:                            \
                if (!(tap_count < threshold)) {               \
                    unregister_mods(MOD_BIT(mod));            \
                    SMTD_REGISTER_16(use_cl, tap_key);        \
                }                                             \
                return SMTD_RESOLUTION_DETERMINED;            \
            case SMTD_ACTION_RELEASE:                         \
                if (tap_count < threshold) {                  \
                    unregister_mods(MOD_BIT(mod));            \
                    send_keyboard_report();                   \
                } else {                                      \
                    SMTD_UNREGISTER_16(use_cl, tap_key);      \
                }                                             \
                return SMTD_RESOLUTION_DETERMINED;            \
        }                                                     \
        break;                                                \
    }

#define SMTD_LT5(macro_kc, tap_key, layer, threshold, use_cl) \
    case macro_kc: {                                          \
        switch (action) {                                     \
            case SMTD_ACTION_TOUCH:                           \
                return SMTD_RESOLUTION_UNCERTAIN;             \
            case SMTD_ACTION_TAP:                             \
                SMTD_TAP_16(use_cl, tap_key);                 \
                return SMTD_RESOLUTION_DETERMINED;            \
            case SMTD_ACTION_HOLD:                            \
                if (tap_count < threshold) {                  \
                    LAYER_PUSH(layer);                        \
                } else {                                      \
                    SMTD_REGISTER_16(use_cl, tap_key);        \
                }                                             \
                return SMTD_RESOLUTION_DETERMINED;            \
            case SMTD_ACTION_RELEASE:                         \
                if (tap_count < threshold) {                  \
                    LAYER_RESTORE();                          \
                }                                             \
                SMTD_UNREGISTER_16(use_cl, tap_key);          \
                return SMTD_RESOLUTION_DETERMINED;            \
        }                                                     \
        break;                                                \
    }


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
