#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "mock_qmk_types.h"

// Mock deferred execution functions
typedef uint32_t (*deferred_exec_callback)(uint32_t trigger_time, void *cb_arg);
typedef uint8_t deferred_token;

deferred_token defer_exec(uint32_t delay_ms, deferred_exec_callback callback, void *cb_arg) {
  return 0;
}
void cancel_deferred_exec(deferred_token token) {}

// Mock timer functions
uint32_t timer_read32(void);
uint32_t timer_elapsed32(uint32_t last);
void wait_ms(uint16_t ms);
