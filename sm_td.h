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
 * Version: 0.4.0
 * Date: 2024-03-07
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
    /** The position of key that was pressed */
    keypos_t macro_pos;

    /** The keycode of the key that was pressed (assigned on tap) */
    uint16_t tap_keycode;

    /** The mods that were active on last key action */
    uint8_t saved_mods;

    /** The length of the sequence of same key taps */
    uint8_t sequence_len;

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

#define EMPTY_STATE {                       \
        .macro_pos = MAKE_KEYPOS(0, 0),     \
        .tap_keycode = 0,                   \
        .saved_mods = 0,             \
        .sequence_len = 0,                  \
        .timeout = INVALID_DEFERRED_TOKEN,  \
        .stage = SMTD_STAGE_NONE,           \
        .resolution = SMTD_RESOLUTION_UNCERTAIN,        \
        .next_action = SMTD_ACTION_TOUCH,   \
        .need_next_action = false,          \
        .idx = 0                            \
}

#define SMTD_POOL_SIZE 10
static smtd_state *smtd_active_states[SMTD_POOL_SIZE] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static smtd_state smtd_states_pool[SMTD_POOL_SIZE] = {EMPTY_STATE, EMPTY_STATE, EMPTY_STATE, EMPTY_STATE, EMPTY_STATE,
                                                      EMPTY_STATE, EMPTY_STATE, EMPTY_STATE, EMPTY_STATE, EMPTY_STATE};
static uint8_t smtd_active_states_size = 0;
static bool smtd_bypass = false;



/* ************************************* *
 *           PUBLIC FUNCTIONS            *
 * ************************************* */

bool process_smtd(uint16_t keycode, keyrecord_t *record);

smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t sequence_len);

uint16_t current_keycode(keypos_t *key);

__attribute__((weak)) uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout);

uint32_t get_smtd_timeout_default(smtd_timeout timeout);

uint32_t get_smtd_timeout_or_default(smtd_state *state, smtd_timeout timeout);

__attribute__((weak)) bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature);

bool smtd_feature_enabled_default(uint16_t keycode, smtd_feature feature);

bool smtd_feature_enabled_or_default(smtd_state *state, smtd_feature feature);



/* ************************************* *
 *           INTERNAL FUNCTIONS          *
 * ************************************* */

#define IS_STATE_KEY(state, key) (state->macro_pos.row == key.row && state->macro_pos.col == key.col)

bool smtd_process_desired(uint16_t pressed_keycode, keyrecord_t *record, uint16_t desired_keycode);

void smtd_apply_to_stack(uint8_t starting_idx, keyrecord_t *record, uint16_t desired_keycode);

void smtd_create_state(keyrecord_t *record, uint16_t desired_keycode);

bool smtd_apply_event(smtd_state *state, keyrecord_t *record, bool is_state_key);

void smtd_apply_stage(smtd_state *state, smtd_stage next_stage);

void smtd_handle_action(smtd_state *state, smtd_action action);

void smtd_execute_action(smtd_state *state, smtd_action action);

void smtd_emulate_press(keypos_t *keypos, bool press);

void smtd_propagate_mods(smtd_state *state, uint8_t mods_before_action, uint8_t mods_after_action);

smtd_resolution worst_resolution_before(smtd_state *state);



/* ************************************* *
 *           DEBUG CONFIGURATION         *
 * ************************************* */

#ifdef SMTD_DEBUG_ENABLED

#define SMTD_DEBUG(...) printf(__VA_ARGS__)

char *smtd_stage_to_string(smtd_stage stage) {
    switch (stage) {
        case SMTD_STAGE_NONE:
            return "STAGE_NONE";
        case SMTD_STAGE_TOUCH:
            return "STAGE_TOUCH";
        case SMTD_STAGE_SEQUENCE:
            return "STAGE_SEQUENCE";
        case SMTD_STAGE_FOLLOWING_TOUCH:
            return "STAGE_FOL_TOUCH";
        case SMTD_STAGE_HOLD:
            return "STAGE_HOLD";
        case SMTD_STAGE_RELEASE:
            return "STAGE_RELEASE";
    }
    return "STAGE_UNKNOWN";
}

char *action_to_string(smtd_action action) {
    switch (action) {
        case SMTD_ACTION_TOUCH:
            return "TOUCH";
        case SMTD_ACTION_TAP:
            return "TAP";
        case SMTD_ACTION_HOLD:
            return "HOLD";
        case SMTD_ACTION_RELEASE:
            return "RELEASE";
    }
    return "UNKNOWN";
}

__attribute__((weak)) char* keycode_to_string_user(uint16_t keycode);

char* keycode_to_string_uncertain(uint16_t keycode, bool uncertain) {
    static char buffer[16];

    if (keycode_to_string_user) {
        char* result = keycode_to_string_user(keycode);
        if (result) {
            snprintf(buffer, sizeof(buffer), uncertain ? "?%s" : "%s", result);
            return result;
        }
    }

    snprintf(buffer, sizeof(buffer), uncertain ? "?KC_%d" : "KC_%d", keycode);
    return buffer;
}

char* keycode_to_string(uint16_t keycode) {
    return keycode_to_string_uncertain(keycode, false);
}

char* state_to_string(smtd_state *state) {
    uint16_t keycode = state->tap_keycode;
    if (keycode == 0) {
        keycode = current_keycode(&state->macro_pos);
        return keycode_to_string_uncertain(keycode, true);
    }
    return keycode_to_string_uncertain(keycode, false);
}
#else

#define SMTD_DEBUG(...)

#endif


/* ************************************* *
 *             TIMEOUTS                  *
 * ************************************* */

uint32_t timeout_reset_seq(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    state->sequence_len = 0;
    return 0;
}

uint32_t timeout_touch(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    smtd_handle_action(state, SMTD_ACTION_HOLD);
    smtd_apply_stage(state, SMTD_STAGE_HOLD);
    return 0;
}

uint32_t timeout_sequence(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    if (smtd_feature_enabled_or_default(state, SMTD_FEATURE_AGGREGATE_TAPS)) {
        smtd_handle_action(state, SMTD_ACTION_TAP);
    }
    smtd_apply_stage(state, SMTD_STAGE_NONE);
    return 0;
}

uint32_t timeout_following_touch(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    smtd_handle_action(state, SMTD_ACTION_HOLD);
    smtd_apply_stage(state, SMTD_STAGE_HOLD);
    return 0;
}

uint32_t timeout_release(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    smtd_handle_action(state, SMTD_ACTION_TAP);
    smtd_apply_stage(state, SMTD_STAGE_NONE);
    return 0;
}



/* ************************************* *
 *             STATE PROCESSING          *
 * ************************************* */

bool process_smtd(uint16_t keycode, keyrecord_t *record) {
    return smtd_process_desired(keycode, record, 0);
}

bool smtd_process_desired(uint16_t pressed_keycode, keyrecord_t *record, uint16_t desired_keycode) {
    if (smtd_bypass) {
        SMTD_DEBUG("   GLOBAL BYPASS KEY %s %s\n", keycode_to_string_uncertain(pressed_keycode, desired_keycode == 0), record->event.pressed ? "PRESSED" : "RELEASED");
        return true;
    }

    SMTD_DEBUG("\n>> GOT KEY %s %s\n", keycode_to_string_uncertain(pressed_keycode, desired_keycode == 0), record->event.pressed ? "PRESSED" : "RELEASED");
    smtd_apply_to_stack(0, record, desired_keycode);
    return false;
}

void smtd_apply_to_stack(uint8_t starting_idx, keyrecord_t *record, uint16_t desired_keycode) {
    bool processed_state = false;

    for (uint8_t i = starting_idx; i < smtd_active_states_size; i++) {
        smtd_state *state = smtd_active_states[i];

        bool is_state_key = IS_STATE_KEY(state, record->event.key);
        processed_state = processed_state | is_state_key;

        SMTD_DEBUG("   processing state %s by %s, is_state_key=%d\n", record->event.pressed ? "PRESSED" : "RELEASED",
               state_to_string(state), is_state_key);
        bool can_process_next = smtd_apply_event(state, record, is_state_key);

        if (!can_process_next) {
            SMTD_DEBUG("<< TERM STATE %s by %s\n",
                   record->event.pressed ? "PRESSED" : "RELEASED", state_to_string(state));
            break;
        }
    }

    if (processed_state) {
        SMTD_DEBUG("<< GOT RELEVANT STATE\n");
        return;
    }

    smtd_create_state(record, desired_keycode);
}

void smtd_create_state(keyrecord_t *record, uint16_t desired_keycode) {
    // may be start a new state? A key must be just pressed
    if (!record->event.pressed) {
        SMTD_DEBUG("<< BYPASS KEY RELEASE\n");
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
        SMTD_DEBUG("<< NO FREE STATES\n");
        return;
    }

    smtd_active_states[smtd_active_states_size] = state;
    state->idx = smtd_active_states_size;
    state->macro_pos = record->event.key;
    if (desired_keycode > 0) {
        state->tap_keycode = desired_keycode;
    }
    smtd_active_states_size++;

    SMTD_DEBUG("<< CREATE STATE %s %s\n", state_to_string(state),
               record->event.pressed ? "PRESSED" : "RELEASED");
    smtd_apply_event(state, record, true);
    return;
}

bool smtd_apply_event(smtd_state *state, keyrecord_t *record, bool is_state_key) {
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
                    if (IS_STATE_KEY(smtd_active_states[i], record->event.key)) {
                        following_key_state = smtd_active_states[i];
                        break;
                    }
                }

                if (following_key_state == NULL) {
                    // Some previously pressed key has been released
                    // We don't need to do anything here
                    return true;
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

            if (is_state_key && record->event.pressed) {
                // Same key has just pressed again. We consider that we are in a sequence of taps
                // So current state is interpreted as tap action. And next tap should be handled in another state.
                smtd_handle_action(state, SMTD_ACTION_TAP);

                uint8_t idx = state->idx;

                smtd_apply_stage(state, SMTD_STAGE_NONE);
                SMTD_SIMULTANEOUS_PRESSES_DELAY

                // let that new press event be processed by the next state
                smtd_apply_to_stack(idx, record, false);
                return false;
            }

            if (!is_state_key && record->event.pressed) {
                return true;
            }

            smtd_state *following_key_state = NULL;
            for (uint8_t i = state->idx + 1; i < smtd_active_states_size; i++) {
                if (IS_STATE_KEY(smtd_active_states[i], record->event.key)) {
                    following_key_state = smtd_active_states[i];
                    break;
                }
            }

            if (following_key_state != NULL) {
                // Following key is released. Now we definitely know that macro key is held
                // we need to execute hold the macro key and let following state handle the key release

                smtd_handle_action(state, SMTD_ACTION_HOLD);
                SMTD_SIMULTANEOUS_PRESSES_DELAY

                smtd_apply_to_stack(state->idx + 1, record, false);
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
    SMTD_DEBUG("STAGE by %s, %s -> %s\n", state_to_string(state),
           smtd_stage_to_string(state->stage),smtd_stage_to_string(next_stage));

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

            state->macro_pos = MAKE_KEYPOS(0, 0);
            state->tap_keycode = 0;
            state->saved_mods = 0;
            state->sequence_len = 0;
            state->timeout = INVALID_DEFERRED_TOKEN;
            state->resolution = SMTD_RESOLUTION_UNCERTAIN;
            state->idx = 0;
            state->next_action = SMTD_ACTION_TOUCH;
            state->need_next_action = false;
            break;

        case SMTD_STAGE_TOUCH:
            state->timeout = defer_exec(get_smtd_timeout_or_default(state, SMTD_TIMEOUT_TAP),
                                        timeout_touch, state);
            break;

        case SMTD_STAGE_SEQUENCE:
            state->resolution = SMTD_RESOLUTION_UNCERTAIN;
            state->saved_mods = get_mods();
            state->timeout = defer_exec(get_smtd_timeout_or_default(state, SMTD_TIMEOUT_SEQUENCE),
                                        timeout_sequence, state);
            break;

        case SMTD_STAGE_HOLD:
            break;

        case SMTD_STAGE_FOLLOWING_TOUCH:
            state->timeout = defer_exec(get_smtd_timeout_or_default(state, SMTD_TIMEOUT_FOLLOWING_TAP),
                                        timeout_following_touch, state);
            break;

        case SMTD_STAGE_RELEASE:
            state->timeout = defer_exec(get_smtd_timeout_or_default(state, SMTD_TIMEOUT_RELEASE),
                                        timeout_release, state);
            break;
    }

    // need to cancel after creating new timeout. There is a bug in QMK scheduling
    cancel_deferred_exec(prev_token);
}

void smtd_handle_action(smtd_state *state, smtd_action action) {
    SMTD_DEBUG("..action %s by %s in %s\n",
           action_to_string(action), state_to_string(state), smtd_stage_to_string(state->stage));

    if (worst_resolution_before(state) < SMTD_RESOLUTION_DETERMINED) {
        state->need_next_action = true;
        state->next_action = action;
        SMTD_DEBUG("..action %s by %s in %s deferred\n",
           action_to_string(action), state_to_string(state), smtd_stage_to_string(state->stage));
        return;
    }

    smtd_resolution resolution_before_action = state->resolution;
    smtd_execute_action(state, action);
    smtd_resolution resolution_after_action = state->resolution;

    smtd_state *next_state = NULL;
    for (int i = state->idx + 1; i < smtd_active_states_size; i++) {
        if (smtd_active_states[i]->need_next_action) {
            next_state = smtd_active_states[i];
            break;
        }
    }

    if (next_state == NULL) {
        SMTD_DEBUG("..action %s by %s in %s is complete by last item in stack\n",
           action_to_string(action), state_to_string(state), smtd_stage_to_string(state->stage));
        return;
    }

    if (!(resolution_before_action < SMTD_RESOLUTION_DETERMINED && SMTD_RESOLUTION_DETERMINED == resolution_after_action)) {
        SMTD_DEBUG("..action %s by %s in %s is complete by not certain\n",
           action_to_string(action), state_to_string(state), smtd_stage_to_string(state->stage));
        return;
    }

    SMTD_DEBUG("..action defer %s by %s in %s\n",
           action_to_string(next_state->next_action), state_to_string(next_state), smtd_stage_to_string(next_state->stage));

    next_state->need_next_action = false;

    switch (next_state->next_action) {
        case SMTD_ACTION_TOUCH:
            smtd_execute_action(next_state, SMTD_ACTION_TOUCH);
            break;
        case SMTD_ACTION_TAP:
            smtd_execute_action(next_state, SMTD_ACTION_TOUCH);
            smtd_execute_action(next_state, SMTD_ACTION_TAP);
            break;
        case SMTD_ACTION_HOLD:
            smtd_execute_action(next_state, SMTD_ACTION_TOUCH);
            smtd_execute_action(next_state, SMTD_ACTION_HOLD);
            break;
        case SMTD_ACTION_RELEASE:
            smtd_execute_action(next_state, SMTD_ACTION_TOUCH);
            smtd_execute_action(next_state, SMTD_ACTION_HOLD);
            smtd_execute_action(next_state, SMTD_ACTION_RELEASE);
            break;
    }

    SMTD_DEBUG("..action %s by %s in %s is complete\n",
           action_to_string(action), state_to_string(state), smtd_stage_to_string(state->stage));
}

void smtd_execute_action(smtd_state *state, smtd_action action) {
    if (state->tap_keycode == 0) {
        state->tap_keycode = current_keycode(&state->macro_pos);
    }

    uint8_t mods_before_action = get_mods();

    if (state->saved_mods != mods_before_action) {
        set_mods(state->saved_mods);
        send_keyboard_report();
        SMTD_SIMULTANEOUS_PRESSES_DELAY
    }

    smtd_resolution new_resolution = on_smtd_action(state->tap_keycode, action, state->sequence_len);
    if (new_resolution > state->resolution) {
        state->resolution = new_resolution;
    }

    if (new_resolution == SMTD_RESOLUTION_UNHANDLED) {
        switch (action) {
            case SMTD_ACTION_TOUCH:
                smtd_emulate_press(&state->macro_pos, true);
                state->resolution = SMTD_RESOLUTION_DETERMINED;
                break;
            case SMTD_ACTION_TAP:
                smtd_emulate_press(&state->macro_pos, false);
                break;
            case SMTD_ACTION_HOLD:
                break;
            case SMTD_ACTION_RELEASE:
                smtd_emulate_press(&state->macro_pos, false);
                break;
        }
    }

    uint8_t mods_after_action = get_mods();
    if (mods_before_action != mods_after_action) {
        state->saved_mods = mods_after_action;
        smtd_propagate_mods(state, mods_before_action, mods_after_action);
    }
}


void smtd_propagate_mods(smtd_state *state, uint8_t mods_before_action, uint8_t mods_after_action) {
    uint8_t changed_mods = mods_after_action ^ mods_before_action;
    uint8_t enabled_mods = mods_after_action & changed_mods;
    uint8_t disabled_mods = mods_before_action & changed_mods;

    SMTD_DEBUG("  state[%s]: mods_before_action:%x  mods_after_action:%x"
           "  changed_mods:%x  enabled_mods:%x  disabled_mods:%x\n",
               state_to_string(state), mods_before_action, mods_after_action,
               changed_mods, enabled_mods, disabled_mods);

    for (uint8_t i = state->idx + 1; i < smtd_active_states_size; i++) {
        SMTD_DEBUG("  state[%s] upd state[%s].saved_mods %x by +%x -%x\n",
               state_to_string(state), state_to_string(smtd_active_states[i]),
               smtd_active_states[i]->saved_mods, enabled_mods, disabled_mods);

        smtd_active_states[i]->saved_mods |= enabled_mods;
        smtd_active_states[i]->saved_mods &= ~disabled_mods;

        SMTD_DEBUG("  state[%s] upd state[%s].saved_mods result %x\n",
               state_to_string(state), state_to_string(smtd_active_states[i]),
               smtd_active_states[i]->saved_mods);
    }
}


/* ************************************* *
 *      UTILLITY FUNCTIONS               *
 * ************************************* */

void smtd_emulate_press(keypos_t *keypos, bool press) {
    SMTD_DEBUG("\nEMULATE %s %s\n", press ? "PRESS" : "RELEASE",
           keycode_to_string(current_keycode(keypos)));
    smtd_bypass = true;
    keyevent_t event_press = MAKE_KEYEVENT(keypos->row, keypos->col, press);
    keyrecord_t record_press = {.event = event_press};
    process_record(&record_press);
    smtd_bypass = false;
}

smtd_resolution worst_resolution_before(smtd_state *state) {
    smtd_resolution result = SMTD_RESOLUTION_DETERMINED;
    for (uint8_t i = 0; i < state->idx; i++) {
        if (smtd_active_states[i]->stage == SMTD_STAGE_SEQUENCE) {
            continue;
        }

        if (smtd_active_states[i]->resolution < result) {
            result = smtd_active_states[i]->resolution;
        }
    }

    SMTD_DEBUG("  worst_resolution_before: state[%s] result %d\n",
           state_to_string(state), result);
    return result;
}

uint32_t get_smtd_timeout_or_default(smtd_state *state, smtd_timeout timeout) {
    if (get_smtd_timeout) {
        return get_smtd_timeout(state->tap_keycode, timeout);
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

uint16_t current_keycode(keypos_t *key) {
    uint8_t current_layer = get_highest_layer(layer_state);
    return keymaps[current_layer][key->row][key->col];
}

bool smtd_feature_enabled_or_default(smtd_state *state, smtd_feature feature) {
    if (smtd_feature_enabled) {
        return smtd_feature_enabled(state->tap_keycode, feature);
    }
    return smtd_feature_enabled_default(state->tap_keycode, feature);
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
                return SMTD_RESOLUTION_UNCERTAIN;                 \
            case SMTD_ACTION_TAP:                             \
                SMTD_TAP_16(use_cl, tap_key);                 \
                return SMTD_RESOLUTION_DETERMINED;                   \
            case SMTD_ACTION_HOLD:                            \
                if (tap_count < threshold) {                  \
                    register_mods(MOD_BIT(mod));              \
                } else {                                      \
                    SMTD_REGISTER_16(use_cl, tap_key);        \
                }                                             \
                return SMTD_RESOLUTION_DETERMINED;                   \
            case SMTD_ACTION_RELEASE:                         \
                if (tap_count < threshold) {                  \
                    unregister_mods(MOD_BIT(mod));            \
                } else {                                      \
                    SMTD_UNREGISTER_16(use_cl, tap_key);      \
                    send_keyboard_report();                   \
                }                                             \
                return SMTD_RESOLUTION_DETERMINED;                   \
        }                                                     \
        break;                                                \
    }

#define SMTD_MTE5(macro_kc, tap_key, mod, threshold, use_cl)  \
    case macro_kc: {                                          \
        switch (action) {                                     \
            case SMTD_ACTION_TOUCH:                           \
                register_mods(MOD_BIT(mod));                  \
                return SMTD_RESOLUTION_UNCERTAIN;                 \
            case SMTD_ACTION_TAP:                             \
                unregister_mods(MOD_BIT(mod));                \
                SMTD_TAP_16(use_cl, tap_key);                 \
                return SMTD_RESOLUTION_DETERMINED;                   \
            case SMTD_ACTION_HOLD:                            \
                if (!(tap_count < threshold)) {               \
                    unregister_mods(MOD_BIT(mod));            \
                    SMTD_REGISTER_16(use_cl, tap_key);        \
                }                                             \
                return SMTD_RESOLUTION_DETERMINED;                   \
            case SMTD_ACTION_RELEASE:                         \
                if (tap_count < threshold) {                  \
                    unregister_mods(MOD_BIT(mod));            \
                    send_keyboard_report();                   \
                } else {                                      \
                    SMTD_UNREGISTER_16(use_cl, tap_key);      \
                }                                             \
                return SMTD_RESOLUTION_DETERMINED;                   \
        }                                                     \
        break;                                                \
    }

#define SMTD_LT5(macro_kc, tap_key, layer, threshold, use_cl) \
    case macro_kc: {                                          \
        switch (action) {                                     \
            case SMTD_ACTION_TOUCH:                           \
                return SMTD_RESOLUTION_UNCERTAIN;               \
            case SMTD_ACTION_TAP:                             \
                SMTD_TAP_16(use_cl, tap_key);                 \
                return SMTD_RESOLUTION_DETERMINED;                   \
            case SMTD_ACTION_HOLD:                            \
                if (tap_count < threshold) {                  \
                    LAYER_PUSH(layer);                        \
                } else {                                      \
                    SMTD_REGISTER_16(use_cl, tap_key);        \
                }                                             \
                return SMTD_RESOLUTION_DETERMINED;                   \
            case SMTD_ACTION_RELEASE:                         \
                if (tap_count < threshold) {                  \
                    LAYER_RESTORE();                          \
                }                                             \
                SMTD_UNREGISTER_16(use_cl, tap_key);          \
                return SMTD_RESOLUTION_DETERMINED;                   \
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
