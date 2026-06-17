/* Leader-key layout for the QMK-native sm_td test (issue #29).
 *
 * Two flavours of sm_td mod-tap are exercised:
 *   - native keycode KC_A: keymap position holds KC_A directly, so the resolved
 *     tap goes through the pipeline (process_record) and Leader can see it.
 *   - custom keycode CKC_A: keymap position holds a custom keycode mapped to KC_A
 *     in on_smtd_action, so the resolved tap is sent directly (tap_code16),
 *     bypassing process_record -> Leader never sees it. This reproduces #29.
 *
 * leader_end_user emits an observable HID report per recognised sequence. */

#include "quantum.h"
#include "sm_td.h"

enum custom_keycodes {
    CKC_A = SAFE_RANGE,
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    return process_smtd(keycode, record);
}

smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        SMTD_MT(KC_A, KC_LEFT_CTRL)                /* native: tap KC_A / hold LCtrl */
        SMTD_MT_ON_MKEY(CKC_A, KC_A, KC_LEFT_CTRL) /* custom: tap KC_A / hold LCtrl */
    }
    return SMTD_RESOLUTION_UNHANDLED;
}

void leader_end_user(void) {
    if (leader_sequence_two_keys(KC_A, KC_C)) {
        tap_code(KC_1); /* observable marker: sequence A,C was captured */
    }
}

/* Weak in sm_td.h; macOS ld rejects undefined-weak refs in an executable. */
uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
    return get_smtd_timeout_default(timeout);
}

bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature) {
    return smtd_feature_enabled_default(keycode, feature);
}
