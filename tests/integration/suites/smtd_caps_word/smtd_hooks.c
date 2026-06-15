/* Caps Word layout: an MT letter key plus a standard caps_word_press_user so the
 * real process_caps_word can shift sm_td's pipeline taps. */

#include "quantum.h"
#include "sm_td.h"

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    return process_smtd(keycode, record);
}

smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        SMTD_MT(KC_A, KC_LEFT_CTRL)            /* tap A / hold LCtrl, Caps-Word visible */
        SMTD_MT4(KC_X, KC_LEFT_CTRL, 1, false) /* use_cl=false: hidden from Caps Word */
    }
    return SMTD_RESOLUTION_UNHANDLED;
}

/* Standard Caps Word membership: letters (and a few symbols) keep Caps Word on and
 * get a weak shift; digits/backspace keep it on without shift; anything else ends it. */
bool caps_word_press_user(uint16_t keycode) {
    switch (keycode) {
        case KC_A ... KC_Z:
        case KC_MINS:
            add_weak_mods(MOD_BIT(KC_LSFT));
            return true;
        case KC_1 ... KC_0:
        case KC_BSPC:
        case KC_DEL:
        case KC_UNDS:
            return true;
        default:
            return false;
    }
}

uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
    return get_smtd_timeout_default(timeout);
}

bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature) {
    return smtd_feature_enabled_default(keycode, feature);
}
