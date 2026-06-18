/* Layout configuration for sm_td tests: dynamic release term (issue #45),
 * SMTD_GLOBAL_RELEASE_PERCENT = 100 (upper clamp by RELEASE_TERM) */
#define SMTD_UNIT_TEST

#define MATRIX_ROWS 5
#define MATRIX_COLS 9

#define TAPPING_TERM 200
#define SMTD_GLOBAL_RELEASE_PERCENT 100

#include "../sm_td_bindings.c"

enum LAYERS { L0 = 0, L1 = 1 };

enum KEYCODES {
    L0_KC0 = 100, L0_KC1, L0_KC2, L0_KC3, L0_KC4, L0_KC5, L0_KC6, L0_KC7, L0_KC8,//
    L1_KC0 = 200, L1_KC1, L1_KC2, L1_KC3, L1_KC4, L1_KC5, L1_KC6, L1_KC7, L1_KC8,//
};

uint16_t const keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [L0] = { L0_KC0, L0_KC1, L0_KC2, L0_KC3, L0_KC4, L0_KC5, L0_KC6, L0_KC7, L0_KC8, },
    [L1] = { L1_KC0, L1_KC1, L1_KC2, L1_KC3, L1_KC4, L1_KC5, L1_KC6, L1_KC7, L1_KC8, },
};

smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        SMTD_MT(L0_KC1, KC_LSFT)
        SMTD_LT(L0_KC3, L1)
        SMTD_MT(L0_KC5, KC_LSFT)
    }
    return SMTD_RESOLUTION_UNHANDLED;
}

uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
    /* K5 has a per-key release term lower than the dynamic window it would
     * get from min(p1, p2) * percent / 100: the per-key value must stay the upper bound */
    if (keycode == L0_KC5 && timeout == SMTD_TIMEOUT_RELEASE) return 5;
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
