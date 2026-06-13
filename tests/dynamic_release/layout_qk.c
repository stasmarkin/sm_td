/* Layout configuration for sm_td tests: dynamic release term (issue #45)
 * applied to raw QMK MT()/LT() keycodes via SMTD_ENABLE_QMK_TAPHOLD */
#define SMTD_UNIT_TEST
#define SMTD_ENABLE_QMK_TAPHOLD 1

#define MATRIX_ROWS 5
#define MATRIX_COLS 9

#define TAPPING_TERM 200
/* defaults under test: RELEASE_TERM = 50, RELEASE_RATIO = 5 */

#include "../sm_td_bindings.c"

enum LAYERS { L0 = 0, L1 = 1 };

enum KEYCODES {
    L0_KC0 = 100, L0_KC1, L0_KC2, L0_KC3, L0_KC4, L0_KC5, L0_KC6, L0_KC7, L0_KC8,//
    L1_KC0 = 200, L1_KC1, L1_KC2, L1_KC3, L1_KC4, L1_KC5, L1_KC6, L1_KC7, L1_KC8,//
};

/* 5-bit mod mask as in QMK */
#define MOD_LCTL 0x01

/* Tap keycodes must fit in 8 bits for MT()/LT() packing */
#define TAP_KC_MT 104
#define TAP_KC_LT 105

uint16_t const keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [L0] = { L0_KC0, MT(MOD_LCTL, TAP_KC_MT), LT(L1, TAP_KC_LT), L0_KC3, L0_KC4, L0_KC5, L0_KC6, L0_KC7, L0_KC8, },
    [L1] = { L1_KC0, L1_KC1, L1_KC2, L1_KC3, L1_KC4, L1_KC5, L1_KC6, L1_KC7, L1_KC8, },
};

/* Everything is unhandled: raw MT()/LT() keycodes must be picked up by smtd_handle_qk_tap_hold */
smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    return SMTD_RESOLUTION_UNHANDLED;
}

uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
    return get_smtd_timeout_default(timeout);
}

bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature) {
    return smtd_feature_enabled_default(keycode, feature);
}

char* smtd_keycode_to_str_user(uint16_t keycode) {
    static char buffer[16];
    TEST_snprintf(buffer, sizeof(buffer), "KC_%d", keycode);
    return buffer;
}

// Post-function implementations (no special behavior in this suite)
void post_register_code16(uint16_t keycode) {
}

void post_unregister_code16(uint16_t keycode) {
}

void post_process_record(keyrecord_t *record) {
}
