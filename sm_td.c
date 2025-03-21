/* Simple implementation file for sm_td to be used with tests */

#include <stdint.h>
#include <stdbool.h>
#include "mock_qmk_headers.h"
#include "mock_qmk_types.h"
#include "mock_qmk_deferred_exec.h"
#include "sm_td.h"

uint16_t keymaps[32][MATRIX_ROWS][MATRIX_COLS] = {0};

// Implementation of on_smtd_action function
smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t sequence_len) {
    return SMTD_RESOLUTION_UNHANDLED;
}

// Implementation of get_smtd_timeout function
uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
    return get_smtd_timeout_default(timeout);
}

// Implementation of smtd_feature_enabled function
bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature) {
    return smtd_feature_enabled_default(keycode, feature);
}
