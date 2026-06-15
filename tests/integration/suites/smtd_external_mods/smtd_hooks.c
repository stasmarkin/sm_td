/* External-mod layout: a plain QMK Shift (KC_LSFT, passed through) alongside an
 * sm_td mod-tap KC_A=LCtrl, plus plain letters. */

#include "quantum.h"
#include "sm_td.h"

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    return process_smtd(keycode, record);
}

smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        SMTD_MT(KC_A, KC_LEFT_CTRL)   /* tap A / hold LCtrl */
    }
    return SMTD_RESOLUTION_UNHANDLED;  /* KC_LSFT, KC_B, KC_C pass through */
}

uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
    return get_smtd_timeout_default(timeout);
}

bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature) {
    return smtd_feature_enabled_default(keycode, feature);
}
