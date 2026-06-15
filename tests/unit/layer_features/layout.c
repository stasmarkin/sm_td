/* Layout for layer-interaction regression tests.
 *
 * Models the QMK pieces that sm_td has to coexist with but does not implement
 * itself: a TG()-style toggle key (issue #57) and tri-layer auto-activation
 * (issue #44). The thing under test is sm_td's own layer handling: since 0.6.2
 * SMTD_LT uses layer_on/layer_off (additive) instead of layer_move (replace),
 * so foreign layer bits set by these QMK features must survive an SMTD_LT hold.
 *
 * Layer map:
 *   #44 tri-layer uses L1 + L2 -> L3
 *   #57 toggle    uses L4 (held via SMTD_LT) + L5 (toggled via TG_L5)
 * The two groups don't overlap, so the tri-layer hook never interferes with the
 * toggle test.
 */
#define SMTD_UNIT_TEST

#define MATRIX_ROWS 1
#define MATRIX_COLS 5

#define TAPPING_TERM 200

/* This layout provides its own layer_state_set_user (tri-layer model below),
 * so suppress the identity default in sm_td_bindings.c. */
#define SMTD_LAYOUT_DEFINES_LAYER_HOOK

#include "../sm_td_bindings.c"

enum LAYERS { L0 = 0, L1 = 1, L2 = 2, L3 = 3, L4 = 4, L5 = 5 };

enum KEYCODES {
    /* col 0: SMTD_LT(L0 -> L1) for the tri-layer test */
    LT_A = 100,
    /* col 1: SMTD_LT(L1 -> L2) for the tri-layer test, pressed while on L1 */
    LT_B,
    /* col 2: SMTD_LT(L0 -> L4) for the toggle test */
    LT_C,
    /* col 3: plain (non-sm_td) key that toggles L5, mimics QMK's TG(L5) */
    TG_L5,
    /* col 4: filler plain keys, one per layer, never pressed */
    KC_FILL_L0 = 200, KC_FILL_L1, KC_FILL_L2, KC_FILL_L3, KC_FILL_L4, KC_FILL_L5,
};

uint16_t const keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [L0] = { LT_A,  LT_B,  LT_C,  TG_L5, KC_FILL_L0 },
    [L1] = { LT_A,  LT_B,  LT_C,  TG_L5, KC_FILL_L1 },
    [L2] = { LT_A,  LT_B,  LT_C,  TG_L5, KC_FILL_L2 },
    [L3] = { LT_A,  LT_B,  LT_C,  TG_L5, KC_FILL_L3 },
    [L4] = { LT_A,  LT_B,  LT_C,  TG_L5, KC_FILL_L4 },
    [L5] = { LT_A,  LT_B,  LT_C,  TG_L5, KC_FILL_L5 },
};

smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        SMTD_LT(LT_A, L1)
        SMTD_LT(LT_B, L2)
        SMTD_LT(LT_C, L4)
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
        case LT_A:  return "LT_A";
        case LT_B:  return "LT_B";
        case LT_C:  return "LT_C";
        case TG_L5: return "TG_L5";
        default:    return "FILL";
    }
}

/* QMK runs tri-layer adjustment inside layer_state_set; model L1 + L2 -> L3. */
uint32_t layer_state_set_user(uint32_t state) {
    if ((state & (1UL << L1)) && (state & (1UL << L2))) {
        state |= (1UL << L3);
    } else {
        state &= ~(1UL << L3);
    }
    return state;
}

void post_register_code16(uint16_t keycode) {}

void post_unregister_code16(uint16_t keycode) {}

/* TG_L5 is a plain key handled by "QMK": toggle L5 on its press, ignore release. */
void post_process_record(keyrecord_t *record) {
    uint16_t keycode = keymap_key_to_keycode(get_highest_layer(layer_state), record->event.key);
    if (keycode == TG_L5 && record->event.pressed) {
        if ((layer_state >> L5) & 1) {
            layer_off(L5);
        } else {
            layer_on(L5);
        }
    }
}
