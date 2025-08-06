/* Layout configuration for sm_td tests */
#define SMTD_UNIT_TEST
#define SMTD_GLOBAL_MODS_PROPAGATION_ENABLED

#define MATRIX_ROWS 1
#define MATRIX_COLS 4

#define TAPPING_TERM 200

#include "../sm_td_bindings.c"

enum LAYERS { L0 = 0 };

enum KEYCODES {
    KC_SHIFT = 100, KC1_MT_CTRL, KC2, KC3,
};

enum MODIFIERS {
    KC_LEFT_CTRL = 0x00E0,
    KC_LEFT_SHIFT = 0x00E1,
    KC_LEFT_ALT = 0x00E2,
    KC_LEFT_GUI = 0x00E3,
    KC_RIGHT_CTRL = 0x00E4,
    KC_RIGHT_SHIFT = 0x00E5,
    KC_RIGHT_ALT = 0x00E6,
    KC_RIGHT_GUI = 0x00E7,
};

uint16_t const keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [L0] = { KC_SHIFT, KC1_MT_CTRL, KC2, KC3, }
};

smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        SMTD_MT(KC1_MT_CTRL, KC_LEFT_CTRL)
    }
    return SMTD_RESOLUTION_UNHANDLED;
}

uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
    return get_smtd_timeout_default(timeout);
}

bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature) {
    return smtd_feature_enabled_default(keycode, feature);
}

char* smtd_keycode_to_str_user(uint16_t keycode) {
    switch (keycode) {
        case KC_SHIFT: return "KC_SHIFT";
        case KC1_MT_CTRL: return "KC1_MT_CTRL";
        case KC2: return "KC2";
        default:     return "UNKNWN";
    }
}

// Post-function implementations
void post_register_code16(uint16_t keycode) {
    if (keycode == KC_SHIFT) register_mods(MOD_BIT(KC_LEFT_SHIFT));
}

void post_unregister_code16(uint16_t keycode) {
    if (keycode == KC_SHIFT) unregister_mods(MOD_BIT(KC_LEFT_SHIFT));
}

void post_process_record(keyrecord_t *record) {
    if (record->event.key.col == 0 && record->event.pressed) register_mods(MOD_BIT(KC_LEFT_SHIFT));
    if (record->event.key.col == 0 && !record->event.pressed) unregister_mods(MOD_BIT(KC_LEFT_SHIFT));
}