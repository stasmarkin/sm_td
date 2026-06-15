/* Dynamic release-term layout: a shift mod-tap and a layer-tap, plus plain keys. */

#include "quantum.h"
#include "sm_td.h"

enum smtd_dyn_layers { L0 = 0, L1 = 1 };

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    return process_smtd(keycode, record);
}

smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        SMTD_MT(KC_A, KC_LEFT_SHIFT)   /* tap A / hold LShift */
        SMTD_MT(KC_S, KC_LEFT_GUI)     /* tap S / hold LGui (for 3-key MT roll) */
        SMTD_MT(KC_G, KC_LEFT_ALT)     /* tap G / hold LAlt (for 3-key MT roll) */
        SMTD_MT(KC_F, KC_LEFT_SHIFT)   /* same, but with a per-key release cap below */
        SMTD_LT(KC_D, L1)              /* tap D / hold layer 1 */
    }
    return SMTD_RESOLUTION_UNHANDLED;
}

/* KC_F caps its touch-release window at 5ms, so the dynamic window (min(p1,p2)/5)
 * can never exceed 5ms for it. */
uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
    if (keycode == KC_F && timeout == SMTD_TIMEOUT_RELEASE) {
        return 5;
    }
    return get_smtd_timeout_default(timeout);
}

bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature) {
    return smtd_feature_enabled_default(keycode, feature);
}
