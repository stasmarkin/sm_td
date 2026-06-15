/* Full-engine layout for the QMK-native sm_td test.
 *
 * Routes the real pipeline into sm_td via process_record_user (post-action_tapping,
 * the manual-install position) and defines a rich on_smtd_action covering every
 * customization macro on real keycodes, so each action produces an observable HID
 * report (or layer change). */

#include "quantum.h"
#include "sm_td.h"

enum smtd_test_layers { L0 = 0, L1 = 1, L2 = 2 };

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    return process_smtd(keycode, record);
}

smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        SMTD_MT(KC_A, KC_LEFT_CTRL)              /* tap A / hold LCtrl */
        SMTD_MT(KC_J, KC_LEFT_ALT)               /* tap J / hold LAlt (second MT for rolls) */
        SMTD_MTE(KC_S, KC_LEFT_SHIFT)            /* eager: LShift on touch, A-like tap drops it */
        SMTD_LT(KC_D, L1)                        /* tap D / hold layer 1 */
        SMTD_TD(KC_F, KC_ESC)                    /* tap F / hold ESC */
        SMTD_TK(KC_G, KC_TAB, 2)                 /* TAB once tap_count reaches 2 (3rd press) */
        SMTD_TTO(KC_H, L2, 2)                    /* move to layer 2 once tap_count reaches 2 */
    }
    return SMTD_RESOLUTION_UNHANDLED;
}

/* Weak in sm_td.h; macOS ld rejects undefined-weak refs in an executable. */
uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
    return get_smtd_timeout_default(timeout);
}

bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature) {
    return smtd_feature_enabled_default(keycode, feature);
}
