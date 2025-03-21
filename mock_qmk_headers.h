#pragma once

// Mock QMK headers for testing purposes

// Define basic types needed by sm_td.h
#include <stdint.h>
#include <stdbool.h>
#include "mock_qmk_types.h"

// Mock keycode ranges
#define KC_A 0x04
#define KC_Z 0x1D
#define SAFE_RANGE 0xE000

// Mock constants
#define TAPPING_TERM 200

// Mock deferred execution functions
typedef void (*deferred_token)(void);
typedef void (*deferred_exec_callback)(uint32_t trigger_time, void *cb_arg);
typedef bool (*deferred_exec_ready_callback)(uint32_t trigger_time, void *cb_arg);

deferred_token defer_exec(uint32_t delay_ms, deferred_exec_callback callback, void *cb_arg) {
    // In the test environment, this could just return a dummy token
    return 0;
}

deferred_token defer_exec_advanced(uint32_t delay_ms, deferred_exec_callback callback, deferred_exec_ready_callback ready_callback, void *cb_arg);
void cancel_deferred_exec(deferred_token token);

// Mock timer functions
uint32_t timer_read32(void);
uint32_t timer_elapsed32(uint32_t last);
void wait_ms(uint16_t ms);

// Mock keyboard functions
void tap_code16(uint16_t keycode);
void register_code16(uint16_t keycode);
void unregister_code16(uint16_t keycode);
void register_mods(uint8_t mods);
void unregister_mods(uint8_t mods);
uint8_t get_mods(void);
void set_mods(uint8_t mods);
void send_keyboard_report(void);

// Mock layer functions
uint8_t get_highest_layer(uint32_t state) {
    // Find the highest bit that is set
    uint8_t highest = 0;
    for (uint8_t i = 0; i < 32; i++) {
        if ((state >> i) & 1) {
            highest = i;
        }
    }
    return highest;
}

void layer_move(uint8_t layer) {
    // Simply sets the layer_state to have only the specified layer bit set
    layer_state = (1UL << layer);
}

extern uint32_t layer_state;
uint32_t layer_state = 0;

bool is_caps_word_on(void) {
    return false;
}

// Mock keymap
uint16_t keymaps[32][MATRIX_ROWS][MATRIX_COLS] = {0};

// Mock mod state
static uint8_t current_mods = 0;

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
    // No-op in mock
    return true;
}
