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

#include QMK_KEYBOARD_H
#include "deferred_exec.h"

#ifdef SMTD_DEBUG_ENABLED
#include "print.h"
#endif

#ifdef SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS
#include "timer.h"
#endif


/* ************************************* *
 *       USER STATES DEFINITIONS         *
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
    SMTD_STATUS_UNSET,
    SMTD_STATUS_UNHANDLED,
    SMTD_STATUS_UNDEF_LAYER,
    SMTD_STATUS_UNDEF_MOD,
    SMTD_STATUS_CERTAIN,
} smtd_status;

#ifdef SMTD_DEBUG_ENABLED
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
#endif

typedef struct {
    /** The position of key that was pressed */
    keypos_t macro_pos;

    /** The keycode of the key that was pressed (assigned on tap) */
    uint16_t tap_keycode;

    /** The mods after the touch action performed */
    uint8_t modes_after_touch;

    /** The length of the sequence of same key taps */
    uint8_t sequence_len;

    /** The timeout of current stage */
    deferred_token timeout;

    /** The current stage of the state */
    smtd_stage stage;

    /** The level of certainty of the state */
    smtd_status status;
} smtd_state;

#define EMPTY_STATE {                       \
        .macro_pos = MAKE_KEYPOS(0, 0),     \
        .tap_keycode = 0,                   \
        .modes_after_touch = 0,              \
        .sequence_len = 0,                  \
        .timeout = INVALID_DEFERRED_TOKEN,  \
        .stage = SMTD_STAGE_NONE,           \
        .status = SMTD_STATUS_UNSET         \
}

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


uint16_t current_keycode(keypos_t *key) {
    uint8_t current_layer = get_highest_layer(layer_state);
    return keymaps[current_layer][key->row][key->col];
}

#define IS_STATE_KEY(state, key) \
    (state->macro_pos.row == key.row && state->macro_pos.col == key.col)

/* ************************************* *
 *          DEBUG CONFIGURATION          *
 * ************************************* */

#ifdef SMTD_DEBUG_ENABLED
__attribute__((weak)) char* keycode_to_string_user(uint16_t keycode);

char* keycode_to_string(uint16_t keycode) {
    if (keycode_to_string_user) {
        char* result = keycode_to_string_user(keycode);
        if (result) {
            return result;
        }
    }

    static char buffer[16];
    snprintf(buffer, sizeof(buffer), "KC_%d", keycode);
    return buffer;
}

char* state_to_string(smtd_state *state) {
    uint16_t keycode = state->tap_keycode;
    if (keycode == 0) {
        keycode = current_keycode(&state->macro_pos);
    }
    return keycode_to_string(keycode);
}
#endif

/* ************************************* *
 *       USER TIMEOUT DEFINITIONS        *
 * ************************************* */

typedef enum {
    SMTD_TIMEOUT_TAP,
    SMTD_TIMEOUT_SEQUENCE,
    SMTD_TIMEOUT_FOLLOWING_TAP,
    SMTD_TIMEOUT_RELEASE,
} smtd_timeout;

__attribute__((weak)) uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout);

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

uint32_t get_smtd_timeout_or_default(smtd_state *state, smtd_timeout timeout) {
    if (get_smtd_timeout) {
        return get_smtd_timeout(state->tap_keycode, timeout);
    }
    return get_smtd_timeout_default(timeout);
}

/* ************************************* *
 *    USER FEATURE FLAGS DEFINITIONS     *
 * ************************************* */

typedef enum {
    SMTD_FEATURE_AGGREGATE_TAPS,
} smtd_feature;

__attribute__((weak)) bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature);

bool smtd_feature_enabled_default(uint16_t keycode, smtd_feature feature) {
    switch (feature) {
        case SMTD_FEATURE_AGGREGATE_TAPS:
            return SMTD_GLOBAL_AGGREGATE_TAPS;
    }
    return false;
}

bool smtd_feature_enabled_or_default(smtd_state *state, smtd_feature feature) {
    if (smtd_feature_enabled) {
        return smtd_feature_enabled(state->tap_keycode, feature);
    }
    return smtd_feature_enabled_default(state->tap_keycode, feature);
}

/* ************************************* *
 *       USER ACTION DEFINITIONS         *
 * ************************************* */

typedef enum {
    SMTD_ACTION_TOUCH,
    SMTD_ACTION_TAP,
    SMTD_ACTION_HOLD,
    SMTD_ACTION_RELEASE,
} smtd_action;

#ifdef SMTD_DEBUG_ENABLED
char *action_to_string(smtd_action action) {
    switch (action) {
        case SMTD_ACTION_TOUCH:
            return "ACT_TOUCH";
        case SMTD_ACTION_TAP:
            return "ACT_TAP";
        case SMTD_ACTION_HOLD:
            return "ACT_HOLD";
        case SMTD_ACTION_RELEASE:
            return "ACT_RELEASE";
    }
    return "ACT_UNKNOWN";
}
#endif

smtd_status on_smtd_action(uint16_t keycode, smtd_action action, uint8_t sequence_len);

/* ************************************* *
 *             LAYER UTILS               *
 * ************************************* */

#define RETURN_LAYER_NOT_SET 15

static uint8_t return_layer = RETURN_LAYER_NOT_SET;
static uint8_t return_layer_cnt = 0;

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

/* ************************************* *
 *      CORE LOGIC IMPLEMENTATION        *
 * ************************************* */

static smtd_state smtd_states_pool[10] = {EMPTY_STATE, EMPTY_STATE, EMPTY_STATE, EMPTY_STATE, EMPTY_STATE,
                                          EMPTY_STATE, EMPTY_STATE, EMPTY_STATE, EMPTY_STATE, EMPTY_STATE};
static smtd_state *smtd_active_states[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static uint8_t smtd_active_states_size = 0;
static bool smtd_bypass = false;
static bool smtd_uncertain_layer = false;

smtd_status worst_status_before(smtd_state *state) {
    smtd_status result = SMTD_STATUS_CERTAIN;
    for (uint8_t i = 0; i < smtd_active_states_size; i++) {
        if (smtd_active_states[i] == state) {
            break;
        }
        if (smtd_active_states[i]->status < result) {
            result = smtd_active_states[i]->status;
        }
    }
    return result;
}

void avoid_unused_variable_on_compile(void *ptr) {
    // just touch them, so compiler won't throw "defined but not used" error
    // that variables are used in macros that user may not use
    if (return_layer == RETURN_LAYER_NOT_SET) return_layer = RETURN_LAYER_NOT_SET;
    if (return_layer_cnt == 0) return_layer_cnt = 0;
    if (smtd_uncertain_layer) smtd_uncertain_layer = true;
}

void do_smtd_action(smtd_action action, smtd_state *state);

void emulate_press(keypos_t *keypos, bool press) {
    #ifdef SMTD_DEBUG_ENABLED
    printf("\nEMULATE %s %s\n", press ? "PRESS" : "RELEASE",
           keycode_to_string(current_keycode(keypos)));
    #endif
    smtd_bypass = true;
    keyevent_t event_press = MAKE_KEYEVENT(keypos->row, keypos->col, press);
    keyrecord_t record_press = {.event = event_press};
    process_record(&record_press);
    smtd_bypass = false;
}

void smtd_next_stage(smtd_state *state, smtd_stage next_stage);

uint32_t timeout_reset_seq(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    state->sequence_len = 0;
    return 0;
}

uint32_t timeout_touch(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    do_smtd_action(SMTD_ACTION_HOLD, state);
    smtd_next_stage(state, SMTD_STAGE_HOLD);
    return 0;
}

uint32_t timeout_sequence(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    if (smtd_feature_enabled_or_default(state, SMTD_FEATURE_AGGREGATE_TAPS)) {
        do_smtd_action(SMTD_ACTION_TAP, state);
    }
    smtd_next_stage(state, SMTD_STAGE_NONE);
    return 0;
}

uint32_t timeout_following_touch(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    do_smtd_action(SMTD_ACTION_HOLD, state);
    smtd_next_stage(state, SMTD_STAGE_HOLD);
    return 0;
}

uint32_t timeout_release(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *) cb_arg;
    do_smtd_action(SMTD_ACTION_TAP, state);
    smtd_next_stage(state, SMTD_STAGE_NONE);
    return 0;
}

void smtd_next_stage(smtd_state *state, smtd_stage next_stage) {
    #ifdef SMTD_DEBUG_ENABLED
    printf("STAGE by %s, %s -> %s\n", state_to_string(state),
           smtd_stage_to_string(state->stage),smtd_stage_to_string(next_stage));
    #endif

    deferred_token prev_token = state->timeout;
    state->timeout = INVALID_DEFERRED_TOKEN;
    state->stage = next_stage;

    switch (state->stage) {
        case SMTD_STAGE_NONE:
            for (uint8_t i = 0; i < smtd_active_states_size; i++) {
                if (smtd_active_states[i] != state) continue;

                for (uint8_t j = i; j < smtd_active_states_size - 1; j++) {
                    smtd_active_states[j] = smtd_active_states[j + 1];
                }

                smtd_active_states_size--;
                smtd_active_states[smtd_active_states_size] = NULL;
                break;
            }

            state->macro_pos = MAKE_KEYPOS(0, 0);
            state->tap_keycode = 0;
            state->modes_after_touch = 0;
            state->sequence_len = 0;
            state->timeout = INVALID_DEFERRED_TOKEN;
            state->status = SMTD_STATUS_UNSET;
            break;

        case SMTD_STAGE_TOUCH:
            if (state->tap_keycode == 0) {
                state->tap_keycode = current_keycode(&state->macro_pos);
            }
            state->timeout = defer_exec(get_smtd_timeout_or_default(state, SMTD_TIMEOUT_TAP),
                                        timeout_touch, state);
            break;

        case SMTD_STAGE_SEQUENCE:
            state->status = SMTD_STATUS_UNSET;
            state->modes_after_touch = 0;
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

bool process_smtd_state(keyrecord_t *record, smtd_state *state, uint8_t idx);

smtd_state *process_smtd_active_states(keyrecord_t *record, uint8_t starting_idx);

bool process_smtd_state(keyrecord_t *record, smtd_state *state, uint8_t idx) {
    switch (state->stage) {
        case SMTD_STAGE_NONE:
            if (IS_STATE_KEY(state, record->event.key) && record->event.pressed) {
                smtd_next_stage(state, SMTD_STAGE_TOUCH);
                do_smtd_action(SMTD_ACTION_TOUCH, state);
                return false;
            }

            return true;

        case SMTD_STAGE_TOUCH:
            if (IS_STATE_KEY(state, record->event.key) && !record->event.pressed) {
                if (!smtd_feature_enabled_or_default(state, SMTD_FEATURE_AGGREGATE_TAPS)) {
                    do_smtd_action(SMTD_ACTION_TAP, state);
                }

                smtd_next_stage(state, SMTD_STAGE_SEQUENCE);

                return false;
            }

            if (!IS_STATE_KEY(state, record->event.key) && record->event.pressed) {
                smtd_next_stage(state, SMTD_STAGE_FOLLOWING_TOUCH);
                return true;
            }

            return true;

        case SMTD_STAGE_SEQUENCE:
            if (IS_STATE_KEY(state, record->event.key) && record->event.pressed) {
                state->sequence_len++;
                do_smtd_action(SMTD_ACTION_TOUCH, state);
                smtd_next_stage(state, SMTD_STAGE_TOUCH);
                return false;
            }

            if (record->event.pressed) {
                if (smtd_feature_enabled_or_default(state, SMTD_FEATURE_AGGREGATE_TAPS)) {
                    do_smtd_action(SMTD_ACTION_TAP, state);
                }
                smtd_next_stage(state, SMTD_STAGE_NONE);
                return true;
            }

            return true;

        case SMTD_STAGE_FOLLOWING_TOUCH:
            // At this stage, we have already pressed the macro key and the following key
            // none of them is assumed to be held yet

            if (IS_STATE_KEY(state, record->event.key) && !record->event.pressed) {
                // Macro key is released, moving to the next stage
                smtd_next_stage(state, SMTD_STAGE_RELEASE);
                return false;
            }

            if (!IS_STATE_KEY(state, record->event.key) && !record->event.pressed) {
                // Another key has been released
                smtd_state *following_key_state = NULL;
                bool reached_curr_state = false;
                for (uint8_t i = 0; i < smtd_active_states_size; i++) {
                    if (smtd_active_states[i] == state) {
                        reached_curr_state = true;
                        continue;
                    }

                    if (!reached_curr_state) {
                        continue;
                    }

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
                smtd_next_stage(state, SMTD_STAGE_HOLD);
                do_smtd_action(SMTD_ACTION_HOLD, state);

                SMTD_SIMULTANEOUS_PRESSES_DELAY
                return process_smtd_active_states(record, idx + 1);
            }

            return true;

        case SMTD_STAGE_HOLD:
            if (IS_STATE_KEY(state, record->event.key) && !record->event.pressed) {
                do_smtd_action(SMTD_ACTION_RELEASE, state);
                smtd_next_stage(state, SMTD_STAGE_NONE);
                return false;
            }

            return true;

        case SMTD_STAGE_RELEASE:
            // At this stage we have just released the macro key and still holding the following key

            if (IS_STATE_KEY(state, record->event.key) && record->event.pressed) {
                // Same key has just pressed again. We consider that we are in a sequence of taps
                // So current state is interpreted as tap action. And next tap should be handled in another state.
                do_smtd_action(SMTD_ACTION_TAP, state);

                //todo need to go to NONE stage and from NONE jump to TOUCH stage
                smtd_next_stage(state, SMTD_STAGE_NONE);

                SMTD_SIMULTANEOUS_PRESSES_DELAY
                // let that new press event be processed by the next state
                return true;
            }

            smtd_state *following_key_state = NULL;
            bool reached_curr_state = false;
            for (uint8_t i = 0; i < smtd_active_states_size; i++) {
                if (smtd_active_states[i] == state) {
                    reached_curr_state = true;
                    continue;
                }

                if (!reached_curr_state) {
                    continue;
                }

                if (IS_STATE_KEY(smtd_active_states[i], record->event.key)) {
                    following_key_state = smtd_active_states[i];
                    break;
                }
            }

            if (following_key_state != NULL && !record->event.pressed) {
                // Following key is released. Now we definitely know that macro key is held
                // we need to execute hold the macro key and let following state handle the key release

                do_smtd_action(SMTD_ACTION_HOLD, state);

                SMTD_SIMULTANEOUS_PRESSES_DELAY
                bool result = process_smtd_active_states(record, idx + 1);

                SMTD_SIMULTANEOUS_PRESSES_DELAY
                do_smtd_action(SMTD_ACTION_RELEASE, state);

                smtd_next_stage(state, SMTD_STAGE_NONE);

                return result;
            }

            return true;
    }

    return true;
}

smtd_state *process_smtd_active_states(keyrecord_t *record, uint8_t starting_idx) {
    for (uint8_t i = starting_idx; i < smtd_active_states_size; i++) {
        smtd_state *state = smtd_active_states[i];
        if (!process_smtd_state(record, state, i)) {
            return state;
        }
    }

    return NULL;
}

/* ************************************* *
 *      ENTRY POINT IMPLEMENTATION       *
 * ************************************* */

bool process_smtd_fixed(uint16_t keycode, keyrecord_t *record, bool fixed) {
    #ifdef SMTD_DEBUG_ENABLED
    printf("\n>> GOT KEY %s %s\n", keycode_to_string(keycode), record->event.pressed ? "PRESSED" : "RELEASED");
    #endif

    if (smtd_bypass) {
        #ifdef SMTD_DEBUG_ENABLED
        printf("<< GLOBAL BYPASS KEY %s %s\n", keycode_to_string(keycode), record->event.pressed ? "PRESSED" : "RELEASED");
        #endif
        return true;
    }

    // check if any active state may process an event
    smtd_state *processed_state = process_smtd_active_states(record, 0);
    if (processed_state != NULL) {
        #ifdef SMTD_DEBUG_ENABLED
        printf("<< HANDLE KEY %s %s by %s\n", keycode_to_string(keycode),
                   record->event.pressed ? "PRESSED" : "RELEASED", state_to_string(processed_state));
        #endif
        return false;
    }

    // may be start a new state? A key must be just pressed
    if (!record->event.pressed) {
        #ifdef SMTD_DEBUG_ENABLED
        printf("<< BYPASS KEY %s %s\n", keycode_to_string(keycode), record->event.pressed ? "PRESSED" : "RELEASED");
        #endif
        return true;
    }

    // check if the key is already handled
    for (uint8_t i = 0; i < smtd_active_states_size; i++) {
        if (IS_STATE_KEY(smtd_active_states[i], record->event.key)) {
            #ifdef SMTD_DEBUG_ENABLED
            printf("<< ALREADY HANDELED KEY %s %s\n", keycode_to_string(keycode), record->event.pressed ? "PRESSED" : "RELEASED");
            #endif
            return true;
        }
    }

    // create a new state and process the event
    smtd_state *state = NULL;
    for (uint8_t i = 0; i < 10; i++) {
        if (smtd_states_pool[i].stage == SMTD_STAGE_NONE) {
            state = &smtd_states_pool[i];
            break;
        }
    }

    if (!state || state == NULL) {
        #ifdef SMTD_DEBUG_ENABLED
        printf("<< NO FREE STATES\n");
        #endif
        return true;
    }

    smtd_active_states[smtd_active_states_size] = state;
    state->macro_pos = record->event.key;
    if (fixed) {
        state->tap_keycode = keycode;
    }
    smtd_active_states_size++;

    #ifdef SMTD_DEBUG_ENABLED
    printf("<< CREATE STATE %s %s\n", keycode_to_string(keycode), record->event.pressed ? "PRESSED" : "RELEASED");
    #endif
    return process_smtd_state(record, state, smtd_active_states_size - 1);
}

bool process_smtd(uint16_t keycode, keyrecord_t *record) {
    return process_smtd_fixed(keycode, record, false);
}

void do_smtd_action(smtd_action action, smtd_state *state) {
    #ifdef SMTD_DEBUG_ENABLED
    printf("%s by %s in %s\n",
           action_to_string(action), state_to_string(state), smtd_stage_to_string(state->stage));
    #endif

    uint16_t keycode = state->tap_keycode;
    uint8_t mods_before_action = get_mods();
    smtd_status status_before_action = state->status;

    if ((action == SMTD_ACTION_TAP || action == SMTD_ACTION_HOLD)
        && state->modes_after_touch != mods_before_action) {
        set_mods(state->modes_after_touch);
        send_keyboard_report();
        SMTD_SIMULTANEOUS_PRESSES_DELAY
    }

    smtd_status new_status = on_smtd_action(keycode, action, state->sequence_len);
    if (new_status > state->status) {
        state->status = new_status;
    }

    if (new_status == SMTD_STATUS_UNHANDLED) {
        switch (action) {
            case SMTD_ACTION_TOUCH:
                if (worst_status_before(state) == SMTD_STATUS_CERTAIN) {
                    emulate_press(&state->macro_pos, true);
                    state->status = SMTD_STATUS_CERTAIN;
                }
                break;
            case SMTD_ACTION_TAP:
                if (state->status == SMTD_STATUS_UNHANDLED) {
                    emulate_press(&state->macro_pos, true);
                    state->status = SMTD_STATUS_CERTAIN;
                }
                emulate_press(&state->macro_pos, false);
                break;
            case SMTD_ACTION_HOLD:
                if (state->status == SMTD_STATUS_UNHANDLED) {
                    emulate_press(&state->macro_pos, true);
                    state->status = SMTD_STATUS_CERTAIN;
                }
                break;
            case SMTD_ACTION_RELEASE:
                emulate_press(&state->macro_pos, false);
                break;
        }
    }

    uint8_t mods_after_action = get_mods();
    if (action == SMTD_ACTION_TOUCH) {
        state->modes_after_touch = mods_after_action;
    }

    smtd_status status_after_action = state->status;
    if (status_after_action <= SMTD_STATUS_UNDEF_MOD || status_before_action > SMTD_STATUS_UNDEF_MOD) {
        return;
    }

    uint8_t changed_mods = mods_after_action ^ state->modes_after_touch;
    uint8_t enabled_mods = mods_after_action & changed_mods;
    uint8_t disabled_mods = mods_before_action & changed_mods;

    uint8_t result_mods = (mods_before_action | enabled_mods) & ~disabled_mods;
    set_mods(result_mods);
    send_keyboard_report();

    #ifdef SMTD_DEBUG_ENABLED
    printf("  state[%s]: state->modes_after_touch:%x  mods_before_action:%x  mods_after_action:%x  changed_mods:%x  enabled_mods:%x  disabled_mods:%x  result_mods:%x\n",
               state_to_string(state), state->modes_after_touch, mods_before_action, mods_after_action, changed_mods, enabled_mods, disabled_mods, result_mods);
    #endif

    if (result_mods == mods_before_action) {
        #ifdef SMTD_DEBUG_ENABLED
        printf("  state[%s] no changed modes after %s\n",
               state_to_string(state), action_to_string(action));
        #endif
        return;
    }

    bool reached_curr_state = false;
    for (uint8_t i = 0; i < smtd_active_states_size; i++) {
        #ifdef SMTD_DEBUG_ENABLED
        printf("  state[%s] examine state[%s]\n",
               state_to_string(state), state_to_string(smtd_active_states[i]));
        #endif

        if (smtd_active_states[i] == state) {
            reached_curr_state = true;
            #ifdef SMTD_DEBUG_ENABLED
            printf("  state[%s] reached self\n",
               state_to_string(state));
            #endif
            continue;
        }

        if (!reached_curr_state) {
            #ifdef SMTD_DEBUG_ENABLED
            printf("  state[%s] skip state[%s]\n",
               state_to_string(state), state_to_string(smtd_active_states[i]));
            #endif
            continue;
        }

        #ifdef SMTD_DEBUG_ENABLED
        printf("  state[%s] upd state[%s].modes_after_touch %x by +%x -%x\n",
               state_to_string(state), state_to_string(smtd_active_states[i]),
               smtd_active_states[i]->modes_after_touch, enabled_mods, disabled_mods);
        #endif

        smtd_active_states[i]->modes_after_touch |= enabled_mods;
        smtd_active_states[i]->modes_after_touch &= ~disabled_mods;

        #ifdef SMTD_DEBUG_ENABLED
        printf("  state[%s] upd state[%s].modes_after_touch result %x\n",
               state_to_string(state), state_to_string(smtd_active_states[i]),
               smtd_active_states[i]->modes_after_touch);
        #endif
    }

    state->modes_after_touch = 0;
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
                return SMTD_STATUS_UNDEF_MOD;                 \
            case SMTD_ACTION_TAP:                             \
                SMTD_TAP_16(use_cl, tap_key);                 \
                return SMTD_STATUS_CERTAIN;                   \
            case SMTD_ACTION_HOLD:                            \
                if (tap_count < threshold) {                  \
                    register_mods(MOD_BIT(mod));              \
                } else {                                      \
                    SMTD_REGISTER_16(use_cl, tap_key);        \
                }                                             \
                return SMTD_STATUS_CERTAIN;                   \
            case SMTD_ACTION_RELEASE:                         \
                if (tap_count < threshold) {                  \
                    unregister_mods(MOD_BIT(mod));            \
                } else {                                      \
                    SMTD_UNREGISTER_16(use_cl, tap_key);      \
                    send_keyboard_report();                   \
                }                                             \
                return SMTD_STATUS_CERTAIN;                   \
        }                                                     \
        break;                                                \
    }

#define SMTD_MTE5(macro_kc, tap_key, mod, threshold, use_cl)  \
    case macro_kc: {                                          \
        switch (action) {                                     \
            case SMTD_ACTION_TOUCH:                           \
                register_mods(MOD_BIT(mod));                  \
                return SMTD_STATUS_UNDEF_MOD;                 \
            case SMTD_ACTION_TAP:                             \
                unregister_mods(MOD_BIT(mod));                \
                SMTD_TAP_16(use_cl, tap_key);                 \
                return SMTD_STATUS_CERTAIN;                   \
            case SMTD_ACTION_HOLD:                            \
                if (!(tap_count < threshold)) {               \
                    unregister_mods(MOD_BIT(mod));            \
                    SMTD_REGISTER_16(use_cl, tap_key);        \
                }                                             \
                return SMTD_STATUS_CERTAIN;                   \
            case SMTD_ACTION_RELEASE:                         \
                if (tap_count < threshold) {                  \
                    unregister_mods(MOD_BIT(mod));            \
                    send_keyboard_report();                   \
                } else {                                      \
                    SMTD_UNREGISTER_16(use_cl, tap_key);      \
                }                                             \
                return SMTD_STATUS_CERTAIN;                   \
        }                                                     \
        break;                                                \
    }

#define SMTD_LT5(macro_kc, tap_key, layer, threshold, use_cl) \
    case macro_kc: {                                          \
        switch (action) {                                     \
            case SMTD_ACTION_TOUCH:                           \
                return SMTD_STATUS_UNDEF_LAYER;               \
            case SMTD_ACTION_TAP:                             \
                SMTD_TAP_16(use_cl, tap_key);                 \
                return SMTD_STATUS_CERTAIN;                   \
            case SMTD_ACTION_HOLD:                            \
                if (tap_count < threshold) {                  \
                    LAYER_PUSH(layer);                        \
                } else {                                      \
                    SMTD_REGISTER_16(use_cl, tap_key);        \
                }                                             \
                return SMTD_STATUS_CERTAIN;                   \
            case SMTD_ACTION_RELEASE:                         \
                if (tap_count < threshold) {                  \
                    LAYER_RESTORE();                          \
                }                                             \
                SMTD_UNREGISTER_16(use_cl, tap_key);          \
                return SMTD_STATUS_CERTAIN;                   \
        }                                                     \
        break;                                                \
    }

