/* User-side C hooks for the QMK-native sm_td chordal-hold test.
 *
 * Same wiring as the smtd_qmk_taphold suite (route process_record into sm_td via
 * the weak process_record_user override), plus the chordal_hold_layout the
 * default smtd_chordal_handedness() reads. The test keyboard matrix is 4x10;
 * handedness is assigned by row so KeymapKey positions map cleanly:
 *   row 0 -> left hand  ('L')
 *   row 1 -> right hand ('R')
 *   row 2..3 -> thumbs / neutral ('*')
 */

#include "quantum.h"
#include "sm_td.h"

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    return process_smtd(keycode, record);
}

/* Raw MT()/LT() keycodes are resolved inside sm_td (smtd_handle_qk_tap_hold),
 * so the user handler claims nothing. */
smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    return SMTD_RESOLUTION_UNHANDLED;
}

/* These are weak in sm_td.h; macOS ld rejects undefined weak refs in an
 * executable, so provide the default delegation explicitly (as a real keymap does). */
uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
    return get_smtd_timeout_default(timeout);
}

bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature) {
    return smtd_feature_enabled_default(keycode, feature);
}

const char chordal_hold_layout[MATRIX_ROWS][MATRIX_COLS] = {
    {'L', 'L', 'L', 'L', 'L', 'L', 'L', 'L', 'L', 'L'},
    {'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R'},
    {'*', '*', '*', '*', '*', '*', '*', '*', '*', '*'},
    {'*', '*', '*', '*', '*', '*', '*', '*', '*', '*'},
};
