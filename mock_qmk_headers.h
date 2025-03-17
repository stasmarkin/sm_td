#pragma once

// Mock QMK headers for testing purposes

// Define basic types needed by sm_td.h
#include <stdint.h>
#include <stdbool.h>

// Mock keycode ranges
#define KC_A 0x04
#define KC_Z 0x1D
#define SAFE_RANGE 0xE000


// Mock deferred execution functions
typedef void (*deferred_token)(void);
typedef void (*deferred_exec_callback)(uint32_t trigger_time, void *cb_arg);
typedef bool (*deferred_exec_ready_callback)(uint32_t trigger_time, void *cb_arg);

deferred_token defer_exec(uint32_t delay_ms, deferred_exec_callback callback, void *cb_arg);
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
uint8_t get_highest_layer(uint32_t state);
void layer_move(uint8_t layer);
extern uint32_t layer_state;
