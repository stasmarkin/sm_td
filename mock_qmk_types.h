#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

// Define keypos_t structure (used in QMK for key position)
typedef struct {
    uint8_t row;
    uint8_t col;
} keypos_t;

// Define keyevent_t structure (used in QMK for key events)
typedef struct {
    keypos_t key;
    bool     pressed;
} keyevent_t;

// Define keyrecord_t structure (used in QMK for key records)
typedef struct {
    keyevent_t event;
} keyrecord_t;

// Define the MAKE_KEYPOS macro
#define MAKE_KEYPOS(row, col) ((keypos_t){ .row = (row), .col = (col) })

// Define MAKE_KEYEVENT macro
#define MAKE_KEYEVENT(row, col, pressed) ((keyevent_t){ .key = MAKE_KEYPOS((row), (col)), .pressed = (pressed) })

// Define INVALID_DEFERRED_TOKEN for deferred execution
#define INVALID_DEFERRED_TOKEN ((deferred_token)0)

// Define any other QMK-specific macros that might be needed
#define MOD_BIT(mod) (1 << (mod))
#define LSFT(kc) ((kc) | 0x1000)

// Layer state type
extern uint32_t layer_state;

// Forward declarations of QMK functions used in sm_td.h
uint8_t get_highest_layer(uint32_t state);
void layer_move(uint8_t layer);
bool is_caps_word_on(void);
