/* sm_td chordal hold against the REAL QMK pipeline: SMTD_CHORDAL_HOLD on top of
 * SMTD_ENABLE_QMK_TAPHOLD with raw MT()/LT() keycodes.
 *
 * The chordal rule settles a tap-hold as HOLD only when an opposite-hand key is
 * involved; same-hand rolls stay taps. Handedness comes from chordal_hold_layout
 * (defined in smtd_hooks.c): row 0 = left, row 1 = right, rows 2..3 = neutral
 * (thumbs). KeymapKey positions are (col, row).
 *
 * Note on report timing: sm_td defers its decision to a key release, and in this
 * harness the resolved emulation flushes once the tap-hold key itself is
 * released. So every test drives the full chord, then asserts the whole report
 * sequence with a single VERIFY_AND_CLEAR at the end.
 */

#include "keyboard_report_util.hpp"
#include "test_common.hpp"

using testing::InSequence;

extern "C" void smtd_reset(void);

class SmTdChordalHold : public TestFixture {
  protected:
    void SetUp() override { smtd_reset(); }
};

/* Lone tap still produces the tap letter (regression with chordal compiled in). */
TEST_F(SmTdChordalHold, mt_tap_alone) {
    TestDriver driver;
    InSequence s;
    KeymapKey mt = KeymapKey(0, 0, 0, MT(MOD_LSFT, KC_A), KC_A);
    set_keymap({mt});

    EXPECT_REPORT(driver, (KC_A));
    EXPECT_EMPTY_REPORT(driver);
    mt.press();
    run_one_scan_loop();
    mt.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* Mod-tap + opposite-hand key resolves as HOLD: the mod applies to that key. */
TEST_F(SmTdChordalHold, cross_hand_mt_plain_holds) {
    TestDriver driver;
    InSequence s;
    KeymapKey mt    = KeymapKey(0, 0, 0, MT(MOD_LSFT, KC_A), KC_A);  /* left  */
    KeymapKey other = KeymapKey(0, 1, 1, KC_K);                      /* right */
    set_keymap({mt, other});

    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_K));
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);
    mt.press();
    run_one_scan_loop();
    other.press();
    run_one_scan_loop();
    other.release();
    run_one_scan_loop();
    mt.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* Mod-tap + same-hand key rolls to two taps, no modifier. */
TEST_F(SmTdChordalHold, same_hand_mt_plain_rolls_to_taps) {
    TestDriver driver;
    InSequence s;
    KeymapKey mt    = KeymapKey(0, 0, 0, MT(MOD_LSFT, KC_A), KC_A);  /* left */
    KeymapKey other = KeymapKey(0, 3, 0, KC_D);                      /* left */
    set_keymap({mt, other});

    EXPECT_REPORT(driver, (KC_A));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_D));
    EXPECT_EMPTY_REPORT(driver);
    mt.press();
    run_one_scan_loop();
    other.press();
    run_one_scan_loop();
    other.release();
    run_one_scan_loop();
    mt.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* Two same-hand mod-taps + opposite-hand key: both mods HOLD, key gets both. */
TEST_F(SmTdChordalHold, two_left_modtaps_plus_cross_hand_key) {
    TestDriver driver;
    InSequence s;
    KeymapKey mt1   = KeymapKey(0, 0, 0, MT(MOD_LSFT, KC_A), KC_A);  /* left */
    KeymapKey mt2   = KeymapKey(0, 1, 0, MT(MOD_LGUI, KC_S), KC_S);  /* left */
    KeymapKey other = KeymapKey(0, 1, 1, KC_K);                      /* right */
    set_keymap({mt1, mt2, other});

    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_LGUI));
    EXPECT_REPORT(driver, (KC_LSFT, KC_LGUI, KC_K));
    EXPECT_REPORT(driver, (KC_LSFT, KC_LGUI));
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);
    mt1.press();
    run_one_scan_loop();
    mt2.press();
    run_one_scan_loop();
    other.press();
    run_one_scan_loop();
    other.release();
    run_one_scan_loop();
    mt2.release();
    run_one_scan_loop();
    mt1.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* Layer-tap + opposite-hand key activates the layer (HOLD): the key resolves on
 * the held layer. */
TEST_F(SmTdChordalHold, cross_hand_lt_holds) {
    TestDriver driver;
    InSequence s;
    KeymapKey lt      = KeymapKey(0, 2, 0, LT(1, KC_C), KC_C);  /* left  */
    KeymapKey other_0 = KeymapKey(0, 1, 1, KC_K);              /* right, layer 0 */
    KeymapKey other_1 = KeymapKey(1, 1, 1, KC_M);              /* right, layer 1 */
    set_keymap({lt, other_0, other_1});

    EXPECT_REPORT(driver, (KC_M));
    EXPECT_EMPTY_REPORT(driver);
    lt.press();
    run_one_scan_loop();
    other_1.press();
    run_one_scan_loop();
    other_1.release();
    run_one_scan_loop();
    lt.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
    EXPECT_TRUE(layer_state_is(0));
}

/* Layer-tap + same-hand key rolls to two taps, no layer change. */
TEST_F(SmTdChordalHold, same_hand_lt_rolls_to_taps) {
    TestDriver driver;
    InSequence s;
    KeymapKey lt    = KeymapKey(0, 2, 0, LT(1, KC_C), KC_C);  /* left */
    KeymapKey other = KeymapKey(0, 3, 0, KC_D);              /* left */
    set_keymap({lt, other});

    EXPECT_REPORT(driver, (KC_C));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_D));
    EXPECT_EMPTY_REPORT(driver);
    lt.press();
    run_one_scan_loop();
    other.press();
    run_one_scan_loop();
    other.release();
    run_one_scan_loop();
    lt.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
    EXPECT_TRUE(layer_state_is(0));
}

/* Thumb (neutral) mod-tap holds even with a same-side key following it. */
TEST_F(SmTdChordalHold, thumb_modtap_holds) {
    TestDriver driver;
    InSequence s;
    KeymapKey thumb = KeymapKey(0, 0, 2, MT(MOD_LSFT, KC_SPC), KC_SPC);  /* neutral */
    KeymapKey other = KeymapKey(0, 3, 0, KC_D);                          /* left */
    set_keymap({thumb, other});

    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_D));
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);
    thumb.press();
    run_one_scan_loop();
    other.press();
    run_one_scan_loop();
    other.release();
    run_one_scan_loop();
    thumb.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}
