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
 * Version: 0.3.1
 * Date: 2024-03-07
 */
#pragma once

#include QMK_KEYBOARD_H
#include "deferred_exec.h"
#ifdef SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS
#include "timer.h"
#endif


/* ************************************* *
 *         GLOBAL CONFIGURATION          *
 * ************************************* */

#ifndef SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS
#    define SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS 0
#endif

#if SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS > 0
    #define SMTD_SIMULTANEOUS_PRESSES_DELAY wait_ms(SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS)
#else
    #define SMTD_SIMULTANEOUS_PRESSES_DELAY
#endif

#ifndef SMTD_GLOBAL_TAP_TERM
#    define SMTD_GLOBAL_TAP_TERM TAPPING_TERM
#endif

#ifndef SMTD_GLOBAL_SEQUENCE_TERM
#    define SMTD_GLOBAL_SEQUENCE_TERM TAPPING_TERM / 2
#endif

#ifndef SMTD_GLOBAL_FOLLOWING_TAP_TERM
#    define SMTD_GLOBAL_FOLLOWING_TAP_TERM TAPPING_TERM
#endif

#ifndef SMTD_GLOBAL_RELEASE_TERM
#    define SMTD_GLOBAL_RELEASE_TERM 50
#endif

#ifndef SMTD_GLOBAL_MODS_RECALL
#    define SMTD_GLOBAL_MODS_RECALL true
#endif

#ifndef SMTD_GLOBAL_AGGREGATE_TAPS
#    define SMTD_GLOBAL_AGGREGATE_TAPS false
#endif

/* ************************************* *
 *      USER SMTD STATE DECLARATION      *
 * ************************************* */

#define SMTD(keycode)                                \
    {                                                \
        .freeze            = false,                  \
            .macro_keycode = keycode,                \
            .tap_mods      = 0,                      \
            .following_key = MAKE_KEYPOS(0, 0),      \
            .stage         = SMTD_STAGE_NONE,        \
            .timeout       = INVALID_DEFERRED_TOKEN, \
            .sequence_len  = 0,                      \
    }

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

uint32_t get_smtd_timeout_or_default(uint16_t keycode, smtd_timeout timeout) {
    if (get_smtd_timeout) {
        return get_smtd_timeout(keycode, timeout);
    }
    return get_smtd_timeout_default(timeout);
}

/* ************************************* *
 *    USER FEATURE FLAGS DEFINITIONS     *
 * ************************************* */

typedef enum {
    SMTD_FEATURE_MODS_RECALL,
    SMTD_FEATURE_AGGREGATE_TAPS,
} smtd_feature;

__attribute__((weak)) bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature);

bool smtd_feature_enabled_default(smtd_feature feature) {
    switch (feature) {
        case SMTD_FEATURE_MODS_RECALL:
            return SMTD_GLOBAL_MODS_RECALL;
        case SMTD_FEATURE_AGGREGATE_TAPS:
            return SMTD_GLOBAL_AGGREGATE_TAPS;
    }
    return false;
}

bool smtd_feature_enabled_or_default(uint16_t keycode, smtd_feature feature) {
    if (smtd_feature_enabled) {
        return smtd_feature_enabled(keycode, feature);
    }
    return smtd_feature_enabled_default(feature);
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

void on_smtd_action(uint16_t keycode, smtd_action action, uint8_t sequence_len);


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

typedef struct {
    uint16_t       macro_keycode;
    uint8_t        tap_mods;
    uint8_t        sequence_len;
    keypos_t       following_key;
    deferred_token timeout;
    smtd_stage     stage;
    bool           freeze;
} smtd_state;

extern smtd_state smtd_states[];
extern size_t     smtd_states_size;


/* ************************************* *
 *      CORE LOGIC IMPLEMENTATION        *
 * ************************************* */

smtd_state       *smtd_active_states[10]      = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
uint8_t           smtd_active_states_next_idx = 0;

#define DO_ACTION_TAP(state)                                                                                                  \
    uint8_t current_mods = get_mods();                                                                                        \
    if (smtd_feature_enabled_or_default(state->macro_keycode, SMTD_FEATURE_MODS_RECALL) && state->tap_mods != current_mods) { \
        set_mods(state->tap_mods);                                                                                            \
        send_keyboard_report();                                                                                               \
        SMTD_SIMULTANEOUS_PRESSES_DELAY;                                                                        \
    }                                                                                                                         \
    on_smtd_action(state->macro_keycode, SMTD_ACTION_TAP, state->sequence_len);                                               \
    if (smtd_feature_enabled_or_default(state->macro_keycode, SMTD_FEATURE_MODS_RECALL) && state->tap_mods != current_mods) { \
        SMTD_SIMULTANEOUS_PRESSES_DELAY;                                                                        \
        set_mods(current_mods);                                                                                               \
        send_keyboard_report();                                                                                               \
    }

#define PUSH_STATE(state)                                    \
    smtd_active_states[smtd_active_states_next_idx] = state; \
    smtd_active_states_next_idx++;

#define REMOVE_STATE(state)                                                 \
    for (uint8_t i = 0; i < smtd_active_states_next_idx; i++) {             \
        if (smtd_active_states[i] == state) {                               \
            for (uint8_t j = i; j < smtd_active_states_next_idx - 1; j++) { \
                smtd_active_states[j] = smtd_active_states[j + 1];          \
            }                                                               \
            smtd_active_states_next_idx--;                                  \
            smtd_active_states[smtd_active_states_next_idx] = NULL;         \
            break;                                                          \
        }                                                                   \
    }

void smtd_press_following_key(smtd_state *state, bool release) {
    state->freeze            = true;
    keyevent_t  event_press  = MAKE_KEYEVENT(state->following_key.row, state->following_key.col, true);
    keyrecord_t record_press = {.event = event_press};
    process_record(&record_press);
    if (release) {
        keyevent_t  event_release  = MAKE_KEYEVENT(state->following_key.row, state->following_key.col, false);
        keyrecord_t record_release = {.event = event_release};
        SMTD_SIMULTANEOUS_PRESSES_DELAY;
        process_record(&record_release);
    }
    state->freeze = false;
}

void smtd_next_stage(smtd_state *state, smtd_stage next_stage);

uint32_t timeout_reset_seq(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state   = (smtd_state *)cb_arg;
    state->sequence_len = 0;
    return 0;
}

uint32_t timeout_touch(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *)cb_arg;
    on_smtd_action(state->macro_keycode, SMTD_ACTION_HOLD, state->sequence_len);
    smtd_next_stage(state, SMTD_STAGE_HOLD);
    return 0;
}

uint32_t timeout_sequence(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *)cb_arg;
    if (smtd_feature_enabled_or_default(state->macro_keycode, SMTD_FEATURE_AGGREGATE_TAPS)) {
        DO_ACTION_TAP(state);
    }
    state->sequence_len = 0;
    smtd_next_stage(state, SMTD_STAGE_NONE);
    return 0;
}

uint32_t timeout_join(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *)cb_arg;
    on_smtd_action(state->macro_keycode, SMTD_ACTION_HOLD, state->sequence_len);
    smtd_next_stage(state, SMTD_STAGE_HOLD);
    SMTD_SIMULTANEOUS_PRESSES_DELAY;
    smtd_press_following_key(state, false);
    return 0;
}

uint32_t timeout_release(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *)cb_arg;
    DO_ACTION_TAP(state);
    SMTD_SIMULTANEOUS_PRESSES_DELAY;
    smtd_press_following_key(state, false);
    state->sequence_len = 0;
    smtd_next_stage(state, SMTD_STAGE_NONE);
    return 0;
}

void smtd_next_stage(smtd_state *state, smtd_stage next_stage) {
    if (state->stage == SMTD_STAGE_NONE) {
        state->tap_mods = get_mods();
        PUSH_STATE(state);
    }
    if (next_stage == SMTD_STAGE_NONE) {
        state->tap_mods = 0;
        REMOVE_STATE(state);
    }

    deferred_token prev_token = state->timeout;
    state->timeout            = INVALID_DEFERRED_TOKEN;
    state->stage              = next_stage;

    switch (state->stage) {
        case SMTD_STAGE_NONE:
            state->following_key = MAKE_KEYPOS(0, 0);
            if (state->sequence_len != 0) {
                state->timeout = defer_exec(get_smtd_timeout_or_default(state->macro_keycode, SMTD_TIMEOUT_SEQUENCE), timeout_reset_seq, state);
            }
            break;
        case SMTD_STAGE_TOUCH:
            state->timeout = defer_exec(get_smtd_timeout_or_default(state->macro_keycode, SMTD_TIMEOUT_TAP), timeout_touch, state);
            break;
        case SMTD_STAGE_SEQUENCE:
            state->timeout = defer_exec(get_smtd_timeout_or_default(state->macro_keycode, SMTD_TIMEOUT_SEQUENCE), timeout_sequence, state);
            break;
        case SMTD_STAGE_HOLD:
            break;
        case SMTD_STAGE_FOLLOWING_TOUCH:
            state->timeout = defer_exec(get_smtd_timeout_or_default(state->macro_keycode, SMTD_TIMEOUT_FOLLOWING_TAP), timeout_join, state);
            break;
        case SMTD_STAGE_RELEASE:
            state->timeout = defer_exec(get_smtd_timeout_or_default(state->macro_keycode, SMTD_TIMEOUT_RELEASE), timeout_release, state);
            break;
    }

    cancel_deferred_exec(prev_token);
}

bool process_smtd_state(uint16_t keycode, keyrecord_t *record, smtd_state *state) {
    if (state->freeze) {
        return true;
    }

    switch (state->stage) {
        case SMTD_STAGE_NONE:
            if (keycode == state->macro_keycode && record->event.pressed) {
                on_smtd_action(state->macro_keycode, SMTD_ACTION_TOUCH, state->sequence_len);
                smtd_next_stage(state, SMTD_STAGE_TOUCH);
                return false;
            }
            return true;

        case SMTD_STAGE_TOUCH:
            if (keycode == state->macro_keycode && !record->event.pressed) {
                smtd_next_stage(state, SMTD_STAGE_SEQUENCE);
                if (!smtd_feature_enabled_or_default(state->macro_keycode, SMTD_FEATURE_AGGREGATE_TAPS)) {
                    DO_ACTION_TAP(state);
                }
                return false;
            }
            if (record->event.pressed) {
                state->following_key = record->event.key;
                smtd_next_stage(state, SMTD_STAGE_FOLLOWING_TOUCH);
                return false;
            }
            return true;

        case SMTD_STAGE_SEQUENCE:
            if (keycode == state->macro_keycode && record->event.pressed) {
                state->sequence_len++;
                on_smtd_action(state->macro_keycode, SMTD_ACTION_TOUCH, state->sequence_len);
                smtd_next_stage(state, SMTD_STAGE_TOUCH);
                return false;
            }
            if (record->event.pressed) {
                if (smtd_feature_enabled_or_default(state->macro_keycode, SMTD_FEATURE_AGGREGATE_TAPS)) {
                    DO_ACTION_TAP(state);
                }
                state->sequence_len = 0;
                smtd_next_stage(state, SMTD_STAGE_NONE);
                return true;
            }
            return true;

        case SMTD_STAGE_FOLLOWING_TOUCH:
            if (keycode == state->macro_keycode && !record->event.pressed) {
                smtd_next_stage(state, SMTD_STAGE_RELEASE);
                return false;
            }
            if (state->following_key.row == record->event.key.row && state->following_key.col == record->event.key.col && !record->event.pressed) {
                on_smtd_action(state->macro_keycode, SMTD_ACTION_HOLD, state->sequence_len);
                smtd_next_stage(state, SMTD_STAGE_HOLD);
                SMTD_SIMULTANEOUS_PRESSES_DELAY;
                smtd_press_following_key(state, true);
                return false;
            }
            if (keycode != state->macro_keycode && (state->following_key.row != record->event.key.row || state->following_key.col != record->event.key.col) && record->event.pressed) {
                on_smtd_action(state->macro_keycode, SMTD_ACTION_HOLD, state->sequence_len);
                smtd_next_stage(state, SMTD_STAGE_HOLD);
                SMTD_SIMULTANEOUS_PRESSES_DELAY;
                smtd_press_following_key(state, false);
                return true;
            }
            return true;

        case SMTD_STAGE_HOLD:
            if (keycode == state->macro_keycode && !record->event.pressed) {
                on_smtd_action(state->macro_keycode, SMTD_ACTION_RELEASE, state->sequence_len);
                state->sequence_len = 0;
                smtd_next_stage(state, SMTD_STAGE_NONE);
                return false;
            }
            return true;

        case SMTD_STAGE_RELEASE:
            if (keycode == state->macro_keycode && record->event.pressed) {
                DO_ACTION_TAP(state);
                SMTD_SIMULTANEOUS_PRESSES_DELAY;
                smtd_press_following_key(state, false);
                SMTD_SIMULTANEOUS_PRESSES_DELAY;
                on_smtd_action(state->macro_keycode, SMTD_ACTION_TOUCH, state->sequence_len);
                state->sequence_len = 0;
                smtd_next_stage(state, SMTD_STAGE_TOUCH);
                return false;
            }
            if (state->following_key.row == record->event.key.row && state->following_key.col == record->event.key.col && !record->event.pressed) {
                on_smtd_action(state->macro_keycode, SMTD_ACTION_HOLD, state->sequence_len);
                SMTD_SIMULTANEOUS_PRESSES_DELAY;
                smtd_press_following_key(state, true);
                SMTD_SIMULTANEOUS_PRESSES_DELAY;
                on_smtd_action(state->macro_keycode, SMTD_ACTION_RELEASE, state->sequence_len);
                state->sequence_len = 0;
                smtd_next_stage(state, SMTD_STAGE_NONE);
                return true;
            }
            if (keycode != state->macro_keycode && (state->following_key.row != record->event.key.row || state->following_key.col != record->event.key.col) && record->event.pressed) {
                DO_ACTION_TAP(state);
                SMTD_SIMULTANEOUS_PRESSES_DELAY;
                smtd_press_following_key(state, false);
                state->sequence_len = 0;
                smtd_next_stage(state, SMTD_STAGE_NONE);
                return true;
            }
            return true;
    }

    return true;
}

/* ************************************* *
 *      ENTRY POINT IMPLEMENTATION       *
 * ************************************* */

bool process_smtd(uint16_t keycode, keyrecord_t *record) {
    for (uint8_t i = 0; i < smtd_active_states_next_idx; i++) {
        smtd_state *state = smtd_active_states[i];
        if (!process_smtd_state(keycode, record, state)) {
            return false;
        }
    }

    for (uint8_t i = 0; i < smtd_states_size; i++) {
        smtd_state *state = &smtd_states[i];
        if (state->stage == SMTD_STAGE_NONE && !process_smtd_state(keycode, record, state)) {
            return false;
        }
    }

    return true;
}
