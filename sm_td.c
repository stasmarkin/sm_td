/* Simple implementation file for sm_td to be used with tests */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAKE_KEYPOS(row, col) ((keypos_t){ (row), (col) })
#define MAKE_KEYEVENT(row, col, pressed) ((keyevent_t){ MAKE_KEYPOS((row), (col)), (pressed) })
#define INVALID_DEFERRED_TOKEN ((deferred_token)0)

#define MOD_BIT(mod) (1 << (mod))
#define LSFT(kc) ((kc) | 0x1000)
#define PROGMEM

#define MATRIX_ROWS 8
#define MATRIX_COLS 8

#define KC_A 0x04
#define KC_Z 0x1D
#define SAFE_RANGE 0xE000

#define TAPPING_TERM 200

#define MAX_RECORD_HISTORY 50

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

typedef uint32_t (*deferred_exec_callback)(uint32_t trigger_time, void *cb_arg);

typedef uint8_t deferred_token;


uint16_t keymaps[32][MATRIX_ROWS][MATRIX_COLS] = {0};
uint32_t layer_state = 0;
uint8_t current_mods = 0;
static keyrecord_t record_history[MAX_RECORD_HISTORY];
static uint8_t record_count = 0;


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
    layer_state = (1UL << layer);
}

bool is_caps_word_on(void) {
    return false;
}

void set_mods(uint8_t mods) {
    current_mods = mods;
}

uint8_t get_mods(void) {
    return current_mods;
}

void register_mods(uint8_t mods) {
    current_mods |= mods;
}

void unregister_mods(uint8_t mods) {
    current_mods &= ~mods;
}

void send_keyboard_report(void) {
    // No-op in mock
}

bool process_record(keyrecord_t *record) {
    // Save the record to history array if there's space
    if (record_count < MAX_RECORD_HISTORY) {
        record_history[record_count] = *record;
        record_count++;
    }
    return true;
}

deferred_token defer_exec(uint32_t delay_ms, deferred_exec_callback callback, void *cb_arg) {
    return 0;
}

void cancel_deferred_exec(deferred_token token) {
}

#include "sm_td.h"

smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t sequence_len) {
    return SMTD_RESOLUTION_UNHANDLED;
}

uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
    return get_smtd_timeout_default(timeout);
}

bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature) {
    return smtd_feature_enabled_default(keycode, feature);
}

void TEST_reset() {
    layer_state = 0;
    current_mods = 0;
    record_count = 0;
    for (uint8_t i = 0; i < MAX_RECORD_HISTORY; i++) {
        record_history[i] = (keyrecord_t){0};
    }
}

void TEST_set_smtd_bypass(const bool bypass) {
    smtd_bypass = bypass;
}

void TEST_get_record_history(keyrecord_t *out_records, uint8_t *out_count) {
    *out_count = record_count;
    for (uint8_t i = 0; i < record_count; i++) {
        out_records[i] = record_history[i];
    }
}
