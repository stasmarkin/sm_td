/* Layout for sm_td chordal-hold tests: SMTD_CHORDAL_HOLD with raw QMK MT()/LT().
 *
 * Matrix is split by hand so the chordal layout is trivial to reason about:
 *   row 0 -> left hand  ('L')
 *   row 1 -> right hand ('R')
 *   row 2 -> thumbs     ('*', neutral)
 */
#define SMTD_UNIT_TEST
#define SMTD_ENABLE_QMK_TAPHOLD 1
#define SMTD_CHORDAL_HOLD 1

#define MATRIX_ROWS 3
#define MATRIX_COLS 4

#define TAPPING_TERM 200


#include "../sm_td_bindings.c"

enum LAYERS { L0 = 0, L1 = 1 };

/* 5-bit mod masks as in QMK (bit 4 set => right-hand mod) */
#define MOD_LSFT 0x02
#define MOD_LGUI 0x08
#define MOD_RSFT 0x12

/* Tap keycodes must fit in 8 bits for MT()/LT() packing */
#define TAP_A 104
#define TAP_B 105
#define TAP_C 106
#define TAP_D 107
#define TAP_T 108

/* Plain keycodes */
#define L_PLAIN_KC 110
#define R_PLAIN_KC 111
#define T_PLAIN_KC 112

uint16_t const keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [L0] = {
        /* left  */ { MT(MOD_LSFT, TAP_A), MT(MOD_LGUI, TAP_B), LT(L1, TAP_C), L_PLAIN_KC },
        /* right */ { MT(MOD_RSFT, TAP_D), R_PLAIN_KC,          R_PLAIN_KC,    R_PLAIN_KC },
        /* thumb */ { MT(MOD_LSFT, TAP_T), T_PLAIN_KC,          T_PLAIN_KC,    T_PLAIN_KC },
    },
    [L1] = {
        { 210, 211, 212, 213 },
        { 220, 221, 222, 223 },
        { 230, 231, 232, 233 },
    },
};

const char chordal_hold_layout[MATRIX_ROWS][MATRIX_COLS] = {
    { 'L', 'L', 'L', 'L' },
    { 'R', 'R', 'R', 'R' },
    { '*', '*', '*', '*' },
};

/* Everything is unhandled: raw MT()/LT() keycodes are picked up by smtd_handle_qk_tap_hold */
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

void post_register_code16(uint16_t keycode) {
}

void post_unregister_code16(uint16_t keycode) {
}

void post_process_record(keyrecord_t *record) {
}
