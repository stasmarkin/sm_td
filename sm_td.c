/* Simple implementation file for sm_td to be used with tests */

#include "sm_td.h"

// Simple implementation of key functions for testing

bool process_smtd(uint16_t keycode, keyrecord_t *record) {
    return smtd_process_desired(keycode, record, 0);
}

smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t sequence_len) {
    // This is a weak function that should be overridden by the user
    // By default, we'll just return unhandled
    return SMTD_RESOLUTION_UNHANDLED;
}

uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
    // This is a weak function that should be overridden by the user
    // By default, just use the default timeout values
    return get_smtd_timeout_default(timeout);
}

bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature) {
    // This is a weak function that should be overridden by the user
    // By default, just use the default feature settings
    return smtd_feature_enabled_default(keycode, feature);
}
