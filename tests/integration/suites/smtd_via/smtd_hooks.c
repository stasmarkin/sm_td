/* VIA/Vial dynamic-keymap layout for the QMK-native sm_td test.
 *
 * Routes the real pipeline into sm_td (process_record_user -> process_smtd) with
 * a small on_smtd_action, and — crucially — provides the keymap_key_to_keycode
 * the harness normally owns. The harness override is compiled out here via
 * SMTD_USE_DYNAMIC_KEYMAP, so sm_td's keycode resolution flows through the real
 * QMK dynamic-keymap chain (keycode_at_keymap_location -> dynamic_keymap_get_keycode
 * -> emulated EEPROM) instead of the in-memory set_keymap() vector. */

#include "quantum.h"
#include "sm_td.h"
#include "keymap_introspection.h"

enum smtd_via_layers { L0 = 0, L1 = 1 };

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    return process_smtd(keycode, record);
}

smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        SMTD_MT(KC_A, KC_LEFT_CTRL)   /* tap A / hold LCtrl */
        SMTD_MTE(KC_S, KC_LEFT_SHIFT) /* eager: LShift on touch */
        SMTD_LT(KC_D, L1)             /* tap D / hold layer 1 */
    }
    return SMTD_RESOLUTION_UNHANDLED; /* plain keycodes (KC_B, KC_E, ...) pass through */
}

/* The dynamic-keymap override: keycode_at_keymap_location is the non-weak override
 * dynamic_keymap.c installs, which reads from the runtime dynamic keymap in EEPROM.
 * keypos_t is {.col, .row}; the dynamic API is (layer, row, column). */
uint16_t keymap_key_to_keycode(uint8_t layer, keypos_t key) {
    return keycode_at_keymap_location(layer, key.row, key.col);
}

/* Weak in sm_td.h; macOS ld rejects undefined-weak refs in an executable. */
uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
    return get_smtd_timeout_default(timeout);
}

bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature) {
    return smtd_feature_enabled_default(keycode, feature);
}
