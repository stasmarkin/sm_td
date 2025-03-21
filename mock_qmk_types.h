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
#define MAKE_KEYPOS(row, col) ((keypos_t){ (row), (col) })

// Define MAKE_KEYEVENT macro
#define MAKE_KEYEVENT(row, col, pressed) ((keyevent_t){ MAKE_KEYPOS((row), (col)), (pressed) })

// Define INVALID_DEFERRED_TOKEN for deferred execution
#define INVALID_DEFERRED_TOKEN ((deferred_token)0)

// Define any other QMK-specific macros that might be needed
#define MOD_BIT(mod) (1 << (mod))
#define LSFT(kc) ((kc) | 0x1000)
#define PROGMEM

// Matrix dimensions
#define MATRIX_ROWS 8
#define MATRIX_COLS 8

// Layer state type
extern uint32_t layer_state;

// Keymaps definition
extern uint16_t keymaps[32][MATRIX_ROWS][MATRIX_COLS];

// Forward declarations of QMK functions used in sm_td.h
uint8_t get_highest_layer(uint32_t state);
void layer_move(uint8_t layer);
bool is_caps_word_on(void);
void set_mods(uint8_t mods);
uint8_t get_mods(void);
void register_mods(uint8_t mods);
void unregister_mods(uint8_t mods);
void send_keyboard_report(void);

// Process record function 
bool process_record(keyrecord_t *record);
