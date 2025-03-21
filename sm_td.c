/* Simple implementation file for sm_td to be used with tests */

#include <stdint.h>
#include <stdbool.h>
#include "mock_qmk_headers.h"
#include "mock_qmk_types.h"
#include "mock_qmk_deferred_exec.h"
#include "sm_td.h"

uint16_t keymaps[32][MATRIX_ROWS][MATRIX_COLS] = {0};

smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t sequence_len) {
    return SMTD_RESOLUTION_UNHANDLED;
}
