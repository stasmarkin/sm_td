#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "mock_qmk_types.h"

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
