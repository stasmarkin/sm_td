/* Simple implementation file for sm_td to be used with tests */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define SMTD_PRINT(...) TEST_print(__VA_ARGS__);
#define SMTD_SNPRINT(bffr, bsize, ...) TEST_snprintf(bffr, bsize, __VA_ARGS__);

#define MAKE_KEYPOS(row, col) ((keypos_t){ (row), (col) })
#define MAKE_KEYEVENT(row, col, pressed) ((keyevent_t){ MAKE_KEYPOS((row), (col)), (pressed) })
#define INVALID_DEFERRED_TOKEN ((deferred_token)0)

#define MOD_BIT(code) (1 << ((code) & 0x07))
#define LSFT(kc) ((kc) | 0x1000)
#define PROGMEM

#define MATRIX_ROWS 1
#define MATRIX_COLS 9

#define TAPPING_TERM 200

#define MAX_RECORD_HISTORY 100
#define MAX_DEFERRED_EXECS 100

#define DEBUG_BUFFER_SIZE 65535


typedef struct {
    uint8_t row;
    uint8_t col;
} keypos_t;

typedef struct {
    keypos_t key;
    bool pressed;
} keyevent_t;

typedef struct {
    keyevent_t event;
} keyrecord_t;

// Flatten structure to store history events
typedef struct {
    uint8_t row;
    uint8_t col;
    uint16_t keycode;
    bool pressed;         // Whether key was pressed or released
    uint8_t mods;         // Modifier state at the time of the event
    uint32_t layer_state; // Layer state at the time of the event
    bool smtd_bypass;
} history_t;

typedef uint32_t (*deferred_exec_callback)(uint32_t trigger_time, void *cb_arg);

typedef uint8_t deferred_token;

typedef struct {
    uint32_t delay_ms;
    deferred_exec_callback callback;
    void *cb_arg;
    bool active;
} deferred_exec_info_t;

enum LAYERS { L0 = 0, L1 = 1, L2 = 2, L3 = 3 };

enum KEYCODES {
    L0_KC0 = 100, L0_KC1, L0_KC2, L0_KC3, L0_KC4, L0_KC5, L0_KC6, L0_KC7, L0_KC8,//
    L1_KC0 = 200, L1_KC1, L1_KC2, L1_KC3, L1_KC4, L1_KC5, L1_KC6, L1_KC7, L1_KC8,//
    L2_KC0 = 300, L2_KC1, L2_KC2, L2_KC3, L2_KC4, L2_KC5, L2_KC6, L2_KC7, L2_KC8,//
    L3_KC0 = 400, L3_KC1, L3_KC2, L3_KC3, L3_KC4, L3_KC5, L3_KC6, L3_KC7, L3_KC8,//
    MACRO0 = 500, MACRO1, MACRO2, MACRO3, MACRO4, MACRO5, MACRO6, MACRO7, MACRO8,//
};

enum MODIFIERS {
    KC_LEFT_CTRL = 0x00E0,
    KC_LEFT_SHIFT = 0x00E1,
    KC_LEFT_ALT = 0x00E2,
    KC_LEFT_GUI = 0x00E3,
    KC_RIGHT_CTRL = 0x00E4,
    KC_RIGHT_SHIFT = 0x00E5,
    KC_RIGHT_ALT = 0x00E6,
    KC_RIGHT_GUI = 0x00E7,
};

uint32_t layer_state = 0;
uint8_t current_mods = 0;
static history_t record_history[MAX_RECORD_HISTORY];
static uint8_t record_count = 0;
static deferred_exec_info_t deferred_execs[MAX_DEFERRED_EXECS] = {0};
static uint8_t deferred_exec_count = 0;
uint16_t const keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [L0] = { L0_KC0, L0_KC1, L0_KC2, L0_KC3, L0_KC4, L0_KC5, L0_KC6, L0_KC7, L0_KC8, },
    [L1] = { L1_KC0, L1_KC1, L1_KC2, L1_KC3, L1_KC4, L1_KC5, L1_KC6, L1_KC7, L1_KC8, },
    [L2] = { L2_KC0, L2_KC1, L2_KC2, L2_KC3, L2_KC4, L2_KC5, L2_KC6, L2_KC7, L2_KC8, },
};

void TEST_print(const char* format, ...);
void TEST_snprintf(char* buffer, size_t bsize, const char* format, ...);

uint32_t timer_read32(void) {
    return 0;
}

uint32_t timer_elapsed32(uint32_t last) {
    return 0;
}

void wait_ms(uint16_t ms) {
}

uint8_t get_highest_layer(uint32_t state) {
    uint8_t highest = 0;
    for (uint8_t i = 0; i < 32; i++) {
        if ((state >> i) & 1) {
            highest = i;
        }
    }
    return highest;
}

void layer_move(uint8_t layer) {
    TEST_print(" --> Layer moved to %d", layer);
    layer_state = layer;
}

bool is_caps_word_on(void) {
    return false;
}

void set_mods(uint8_t mods) {
  TEST_print(" --> Set mods: %d", mods);
    current_mods = mods;
}

uint8_t get_mods(void) {
    return current_mods;
}

void register_mods(uint8_t mods) {
    TEST_print(" --> Register mods: %d", mods);
    current_mods |= mods;
}

void add_mods(uint8_t mods) {
    TEST_print(" --> Add mods: %d", mods);
    current_mods |= mods;
}

void unregister_mods(uint8_t mods) {
    TEST_print(" --> Unregister mods: %d", mods);
    current_mods &= ~mods;
}

void del_mods(uint8_t mods) {
    TEST_print(" --> Delete mods: %d", mods);
    current_mods &= ~mods;
}

void send_keyboard_report(void) {
    // No-op in mock
}

bool get_smtd_bypass();

void unregister_code16(uint16_t keycode) {
    TEST_print(" --> Unregister code: %d", keycode);
    record_history[record_count] = (history_t) {
        .row = 255,
        .col = 255,
        .keycode = keycode,
        .pressed = false,
        .mods = current_mods,
        .layer_state = layer_state,
        .smtd_bypass = get_smtd_bypass(),
    };
    record_count++;

    if (keycode == L0_KC8) unregister_mods(MOD_BIT(KC_LEFT_CTRL));
    if (keycode == L1_KC8) unregister_mods(MOD_BIT(KC_LEFT_CTRL));
    if (keycode == L2_KC8) unregister_mods(MOD_BIT(KC_LEFT_CTRL));
}

void register_code16(uint16_t keycode) {
    TEST_print(" --> Register code: %d", keycode);
    record_history[record_count] = (history_t) {
        .row = 255,
        .col = 255,
        .keycode = keycode,
        .pressed = true,
        .mods = current_mods,
        .layer_state = layer_state,
        .smtd_bypass = get_smtd_bypass(),
    };
    record_count++;

    if (keycode == L0_KC8) register_mods(MOD_BIT(KC_LEFT_CTRL));
    if (keycode == L1_KC8) register_mods(MOD_BIT(KC_LEFT_CTRL));
    if (keycode == L2_KC8) register_mods(MOD_BIT(KC_LEFT_CTRL));
}

void tap_code16(uint16_t keycode) {
    register_code16(keycode);
    unregister_code16(keycode);
}

bool process_record(keyrecord_t *record) {
    record_history[record_count] = (history_t) {
        .row = record->event.key.row,
        .col = record->event.key.col,
        .keycode = 65535,
        .pressed = record->event.pressed,
        .mods = current_mods,
        .layer_state = layer_state,
        .smtd_bypass = get_smtd_bypass(),
    };
    record_count++;

    if (record->event.key.col == 8 && record->event.pressed) register_mods(MOD_BIT(KC_LEFT_CTRL));
    if (record->event.key.col == 8 && !record->event.pressed) unregister_mods(MOD_BIT(KC_LEFT_CTRL));

    return true;
}

deferred_token defer_exec(uint32_t delay_ms, deferred_exec_callback callback, void *cb_arg) {
    deferred_exec_count++;
    deferred_execs[deferred_exec_count-1].delay_ms = delay_ms;
    deferred_execs[deferred_exec_count-1].callback = callback;
    deferred_execs[deferred_exec_count-1].cb_arg = cb_arg;
    deferred_execs[deferred_exec_count-1].active = true;
    return deferred_exec_count;
}

void cancel_deferred_exec(deferred_token token) {
    if (token > 0 && token <= deferred_exec_count) {
        deferred_execs[token-1].active = false;
    }
}


void TEST_print(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void TEST_snprintf(char* buffer, size_t bsize, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, bsize, format, args);
    va_end(args);
}

#include "../sm_td.h"

smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        SMTD_MT_ON_MKEY(L0_KC2, MACRO2, KC_LEFT_GUI, 2)

        SMTD_MT(L0_KC3, KC_LEFT_ALT)
        SMTD_MT(L1_KC3, KC_LEFT_ALT)
        SMTD_MT(L2_KC3, KC_LEFT_ALT)
        SMTD_MT(L3_KC3, KC_LEFT_ALT)

        SMTD_MT(L0_KC4, KC_LEFT_CTRL)
        SMTD_MT(L1_KC4, KC_LEFT_CTRL)
        SMTD_MT(L2_KC4, KC_LEFT_CTRL)
        SMTD_MT(L3_KC4, KC_LEFT_CTRL)

        SMTD_LT(L0_KC5, L1)
        SMTD_LT(L2_KC5, L3)

        SMTD_LT(L0_KC6, L2)
        SMTD_LT(L1_KC6, L3)

        SMTD_MTE(L0_KC7, KC_LEFT_SHIFT)
        SMTD_MTE(L1_KC7, KC_LEFT_SHIFT)
        SMTD_MTE(L2_KC7, KC_LEFT_SHIFT)
        SMTD_MTE(L3_KC7, KC_LEFT_SHIFT)
    }
    return SMTD_RESOLUTION_UNHANDLED;
}

uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
    return get_smtd_timeout_default(timeout);
}

bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature) {
    return smtd_feature_enabled_default(keycode, feature);
}

char* smtd_keycode_to_str_user(uint16_t keycode) {
    switch (keycode) {
        case L0_KC0: return "L0_KC0";
        case L0_KC1: return "L0_KC1";
        case L0_KC2: return "L0_KC2";
        case L0_KC3: return "L0_KC3";
        case L0_KC4: return "L0_KC4";
        case L0_KC5: return "L0_KC5";
        case L0_KC6: return "L0_KC6";
        case L0_KC7: return "L0_KC7";
        case L0_KC8: return "L0_KC8";
        case L1_KC0: return "L1_KC0";
        case L1_KC1: return "L1_KC1";
        case L1_KC2: return "L1_KC2";
        case L1_KC3: return "L1_KC3";
        case L1_KC4: return "L1_KC4";
        case L1_KC5: return "L1_KC5";
        case L1_KC6: return "L1_KC6";
        case L1_KC7: return "L1_KC7";
        case L1_KC8: return "L1_KC8";
        case L2_KC0: return "L2_KC0";
        case L2_KC1: return "L2_KC1";
        case L2_KC2: return "L2_KC2";
        case L2_KC3: return "L2_KC3";
        case L2_KC4: return "L2_KC4";
        case L2_KC5: return "L2_KC5";
        case L2_KC6: return "L2_KC6";
        case L2_KC7: return "L2_KC7";
        case L2_KC8: return "L2_KC8";
        case MACRO0: return "MACRO0";
        case MACRO1: return "MACRO1";
        case MACRO2: return "MACRO2";
        case MACRO3: return "MACRO3";
        case MACRO4: return "MACRO4";
        case MACRO5: return "MACRO5";
        case MACRO6: return "MACRO6";
        case MACRO7: return "MACRO7";
        case MACRO8: return "MACRO8";
        default:     return "UNKNWN";
    }
}

void TEST_reset() {
    layer_state = 0;
    current_mods = 0;
    record_count = 0;
    deferred_exec_count = 0;
    return_layer = RETURN_LAYER_NOT_SET;
    return_layer_cnt = 0;
    for (uint8_t i = 0; i < MAX_RECORD_HISTORY; i++) {
        record_history[i] = (history_t){0};
    }
    for (uint8_t i = 0; i < MAX_DEFERRED_EXECS; i++) {
        deferred_execs[i] = (deferred_exec_info_t){0};
    }
    for (uint8_t i = 0; i < SMTD_POOL_SIZE; i++) {
        smtd_active_states[i] = NULL;
        reset_state(&smtd_states_pool[i]);
    }
    smtd_active_states_size = 0;
    smtd_bypass = false;
}

bool get_smtd_bypass() {
    return smtd_bypass;
}

void TEST_set_smtd_bypass(const bool bypass) {
    smtd_bypass = bypass;
}

bool TEST_get_layer_state() {
    return layer_state;
}

void TEST_get_record_history(history_t *out_records, uint8_t *out_count) {
    *out_count = record_count;
    for (uint8_t i = 0; i < record_count; i++) {
        out_records[i] = record_history[i];
    }
}

void TEST_get_deferred_execs(deferred_exec_info_t *out_execs, uint8_t *out_count) {
    *out_count = deferred_exec_count;
    for (uint8_t i = 0; i < deferred_exec_count; i++) {
        out_execs[i] = deferred_execs[i];
    }
}

void TEST_execute_deferred(deferred_token token) {
    if (token > 0 && token <= deferred_exec_count && deferred_execs[token-1].active) {
        deferred_exec_callback callback = deferred_execs[token-1].callback;
        void *cb_arg = deferred_execs[token-1].cb_arg;
        if (callback != NULL) {
            callback(0, cb_arg);
        }
        deferred_execs[token-1].active = false;
    }
}