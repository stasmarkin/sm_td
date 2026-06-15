/* sm_td reads keycodes LIVE from a real VIA/Vial dynamic keymap.
 *
 * keymap_key_to_keycode (smtd_hooks.c) routes through the genuine QMK dynamic
 * chain backed by the emulated EEPROM, and each test writes the keymap at runtime
 * with dynamic_keymap_set_keycode() — exactly what a VIA/Vial GUI remap does. The
 * harness's own set_keymap() is intentionally NOT used: KeymapKey::press() injects
 * straight into the matrix (press_key(col,row)), so a KeymapKey is just a position
 * holder here; its keycode field is ignored (the harness override is compiled out).
 *
 * sm_td resolves the keycode lazily, once per press, caching it for the state's
 * lifetime (sm_td.c: desired_keycode set on first action, cleared on reset). So a
 * remap written between presses changes the next press's behavior — the property
 * that makes GUI remaps work without a recompile. */

#include "keyboard_report_util.hpp"
#include "test_common.hpp"

extern "C" {
#include "dynamic_keymap.h"
void    smtd_reset(void);
uint8_t get_mods(void);
}

using testing::_;
using testing::AnyNumber;
using testing::InSequence;

class SmTdVia : public TestFixture {
  protected:
    void SetUp() override {
        smtd_reset();
        /* Zero the dynamic keymap so each test starts from KC_NO everywhere and
         * only the positions it writes are live. */
        for (uint8_t layer = 0; layer < DYNAMIC_KEYMAP_LAYER_COUNT; layer++) {
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                    dynamic_keymap_set_keycode(layer, row, col, KC_NO);
                }
            }
        }
    }

    /* KeymapKey(layer, col, row, kc): the kc is ignored by our override; row/col
     * must match the dynamic_keymap_set_keycode(layer, row, col, ...) below. */
    static KeymapKey at(uint8_t col, uint8_t row) { return KeymapKey(0, col, row, KC_NO); }
};

/* A keycode written into the dynamic keymap is what sm_td acts on (not keymaps[][]). */
TEST_F(SmTdVia, dynamic_keycode_is_resolved) {
    TestDriver driver;
    InSequence s;
    dynamic_keymap_set_keycode(0, 0, 0, KC_A); /* on_smtd_action: SMTD_MT(LCtrl) */
    KeymapKey k = at(0, 0);

    EXPECT_REPORT(driver, (KC_A));
    EXPECT_EMPTY_REPORT(driver);
    k.press();
    run_one_scan_loop();
    k.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* A plain (unhandled) keycode from the dynamic keymap passes straight through. */
TEST_F(SmTdVia, dynamic_plain_keycode_passthrough) {
    TestDriver driver;
    InSequence s;
    dynamic_keymap_set_keycode(0, 0, 0, KC_E); /* unhandled by on_smtd_action */
    KeymapKey k = at(0, 0);

    EXPECT_REPORT(driver, (KC_E));
    EXPECT_EMPTY_REPORT(driver);
    k.press();
    run_one_scan_loop();
    k.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* A live remap of the SAME position changes the next press's emitted keycode —
 * the core VIA/Vial guarantee. */
TEST_F(SmTdVia, live_remap_changes_emitted_keycode) {
    TestDriver driver;
    KeymapKey k = at(0, 0);

    {
        InSequence s;
        dynamic_keymap_set_keycode(0, 0, 0, KC_A); /* MT tap -> A */
        EXPECT_REPORT(driver, (KC_A));
        EXPECT_EMPTY_REPORT(driver);
        k.press();
        run_one_scan_loop();
        k.release();
        idle_for(TAPPING_TERM + 50);
        VERIFY_AND_CLEAR(driver);
    }
    {
        InSequence s;
        dynamic_keymap_set_keycode(0, 0, 0, KC_E); /* remap same cell -> plain E */
        EXPECT_REPORT(driver, (KC_E));
        EXPECT_EMPTY_REPORT(driver);
        k.press();
        run_one_scan_loop();
        k.release();
        idle_for(TAPPING_TERM + 50);
        VERIFY_AND_CLEAR(driver);
    }
}

/* A mod-tap from the dynamic keymap holds its mod on a following key, and the
 * following key is ALSO resolved from the dynamic keymap. */
TEST_F(SmTdVia, dynamic_mod_tap_holds_mod_on_following_key) {
    TestDriver driver;
    InSequence s;
    dynamic_keymap_set_keycode(0, 0, 0, KC_A); /* SMTD_MT(LCtrl) */
    dynamic_keymap_set_keycode(0, 0, 4, KC_B); /* plain follow key */
    KeymapKey mt = at(0, 0);
    KeymapKey b  = at(4, 0);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_B));
    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    EXPECT_EMPTY_REPORT(driver);
    mt.press();
    idle_for(TAPPING_TERM + 50);
    b.press();
    run_one_scan_loop();
    b.release();
    run_one_scan_loop();
    mt.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
    EXPECT_EQ(get_mods(), 0);
}

/* Remapping a mod-tap cell to a plain key removes the hold behavior on the next
 * press — the remap flows all the way into sm_td's hold resolution. */
TEST_F(SmTdVia, live_remap_mod_tap_to_plain_drops_hold) {
    TestDriver driver;
    KeymapKey k = at(0, 0);
    KeymapKey b = at(4, 0);
    dynamic_keymap_set_keycode(0, 0, 4, KC_B); /* plain follow key, constant */

    {
        InSequence s;
        dynamic_keymap_set_keycode(0, 0, 0, KC_A); /* MT(LCtrl): hold -> Ctrl+B */
        EXPECT_REPORT(driver, (KC_LEFT_CTRL));
        EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_B));
        EXPECT_REPORT(driver, (KC_LEFT_CTRL));
        EXPECT_EMPTY_REPORT(driver);
        k.press();
        idle_for(TAPPING_TERM + 50);
        b.press();
        run_one_scan_loop();
        b.release();
        run_one_scan_loop();
        k.release();
        idle_for(TAPPING_TERM + 50);
        VERIFY_AND_CLEAR(driver);
        EXPECT_EQ(get_mods(), 0);
    }
    {
        InSequence s;
        dynamic_keymap_set_keycode(0, 0, 0, KC_F); /* plain now: held, no mod */
        EXPECT_REPORT(driver, (KC_F));        /* press: plain key registers */
        EXPECT_REPORT(driver, (KC_F, KC_B));  /* follow key adds, no modifier */
        EXPECT_REPORT(driver, (KC_F));        /* follow released, KC_F still held */
        EXPECT_EMPTY_REPORT(driver);
        k.press();
        idle_for(TAPPING_TERM + 50);
        b.press();
        run_one_scan_loop();
        b.release();
        run_one_scan_loop();
        k.release();
        idle_for(TAPPING_TERM + 50);
        VERIFY_AND_CLEAR(driver);
        EXPECT_EQ(get_mods(), 0);
    }
}
