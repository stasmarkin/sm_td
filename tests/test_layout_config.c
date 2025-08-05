/* Layout configuration for sm_td tests */

#include "sm_td_bindings.c"

// Forward declarations for post-functions
void post_register_code16(uint16_t keycode);
void post_unregister_code16(uint16_t keycode);
void post_process_record(keyrecord_t *record);

enum LAYERS { L0 = 0, L1 = 1, L2 = 2, L3 = 3 };

enum KEYCODES {
    L0_KC0 = 100, L0_KC1, L0_KC2, L0_KC3, L0_KC4, L0_KC5, L0_KC6, L0_KC7, L0_KC8,//
    L1_KC0 = 200, L1_KC1, L1_KC2, L1_KC3, L1_KC4, L1_KC5, L1_KC6, L1_KC7, L1_KC8,//
    L2_KC0 = 300, L2_KC1, L2_KC2, L2_KC3, L2_KC4, L2_KC5, L2_KC6, L2_KC7, L2_KC8,//
    L3_KC0 = 400, L3_KC1, L3_KC2, L3_KC3, L3_KC4, L3_KC5, L3_KC6, L3_KC7, L3_KC8,//
    MACRO0 = 500, MACRO1, MACRO2, MACRO3, MACRO4, MACRO5, MACRO6, MACRO7, MACRO8,//
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
    [L0] = { L0_KC0, L0_KC1, L0_KC2, L0_KC3, L0_KC4, L0_KC5, L0_KC6, L0_KC7, L0_KC8, },
    [L1] = { L1_KC0, L1_KC1, L1_KC2, L1_KC3, L1_KC4, L1_KC5, L1_KC6, L1_KC7, L1_KC8, },
    [L2] = { L2_KC0, L2_KC1, L2_KC2, L2_KC3, L2_KC4, L2_KC5, L2_KC6, L2_KC7, L2_KC8, },
    [L3] = { L3_KC0, L3_KC1, L3_KC2, L3_KC3, L3_KC4, L3_KC5, L3_KC6, L3_KC7, L3_KC8, },
};

smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        SMTD_MT_ON_MKEY(L0_KC2, MACRO2, KC_LEFT_GUI, 2)

        SMTD_MT(L0_KC3, KC_LEFT_ALT)
        SMTD_MT(L1_KC3, KC_LEFT_ALT)
        SMTD_MT(L2_KC3, KC_LEFT_ALT)

        SMTD_MT(L0_KC4, KC_LEFT_CTRL)
        SMTD_MT(L1_KC4, KC_LEFT_CTRL)
        SMTD_MT(L2_KC4, KC_LEFT_CTRL)

        SMTD_LT(L0_KC5, L1)
        SMTD_LT(L2_KC5, L3)

        SMTD_LT(L0_KC6, L2)
        SMTD_LT(L1_KC6, L3)

        SMTD_MTE(L0_KC7, KC_LEFT_SHIFT)
        SMTD_MTE(L1_KC7, KC_LEFT_SHIFT)
        SMTD_MTE(L2_KC7, KC_LEFT_SHIFT)
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
        case L0_KC0: return "L0_KC0";
        case L0_KC1: return "L0_KC1";
        case L0_KC2: return "L0_KC2";
        case L0_KC3: return "L0_KC3";
        case L0_KC4: return "L0_KC4";
        case L0_KC5: return "L0_KC5";
        case L0_KC6: return "L0_KC6";
        case L0_KC7: return "L0_KC7";
        case L0_KC8: return "L0_KC8";
        case L1_KC0: return "L1_KC0";
        case L1_KC1: return "L1_KC1";
        case L1_KC2: return "L1_KC2";
        case L1_KC3: return "L1_KC3";
        case L1_KC4: return "L1_KC4";
        case L1_KC5: return "L1_KC5";
        case L1_KC6: return "L1_KC6";
        case L1_KC7: return "L1_KC7";
        case L1_KC8: return "L1_KC8";
        case L2_KC0: return "L2_KC0";
        case L2_KC1: return "L2_KC1";
        case L2_KC2: return "L2_KC2";
        case L2_KC3: return "L2_KC3";
        case L2_KC4: return "L2_KC4";
        case L2_KC5: return "L2_KC5";
        case L2_KC6: return "L2_KC6";
        case L2_KC7: return "L2_KC7";
        case L2_KC8: return "L2_KC8";
        case L3_KC0: return "L3_KC0";
        case L3_KC1: return "L3_KC1";
        case L3_KC2: return "L3_KC2";
        case L3_KC3: return "L3_KC3";
        case L3_KC4: return "L3_KC4";
        case L3_KC5: return "L3_KC5";
        case L3_KC6: return "L3_KC6";
        case L3_KC7: return "L3_KC7";
        case L3_KC8: return "L3_KC8";
        case MACRO0: return "MACRO0";
        case MACRO1: return "MACRO1";
        case MACRO2: return "MACRO2";
        case MACRO3: return "MACRO3";
        case MACRO4: return "MACRO4";
        case MACRO5: return "MACRO5";
        case MACRO6: return "MACRO6";
        case MACRO7: return "MACRO7";
        case MACRO8: return "MACRO8";
        default:     return "UNKNWN";
    }
}

// Post-function implementations
void post_register_code16(uint16_t keycode) {
    if (keycode == L0_KC8) register_mods(MOD_BIT(KC_LEFT_CTRL));
    if (keycode == L1_KC8) register_mods(MOD_BIT(KC_LEFT_CTRL));
    if (keycode == L2_KC8) register_mods(MOD_BIT(KC_LEFT_CTRL));
}

void post_unregister_code16(uint16_t keycode) {
    if (keycode == L0_KC8) unregister_mods(MOD_BIT(KC_LEFT_CTRL));
    if (keycode == L1_KC8) unregister_mods(MOD_BIT(KC_LEFT_CTRL));
    if (keycode == L2_KC8) unregister_mods(MOD_BIT(KC_LEFT_CTRL));
}

void post_process_record(keyrecord_t *record) {
    if (record->event.key.col == 8 && record->event.pressed) register_mods(MOD_BIT(KC_LEFT_CTRL));
    if (record->event.key.col == 8 && !record->event.pressed) unregister_mods(MOD_BIT(KC_LEFT_CTRL));
}