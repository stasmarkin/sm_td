/* sm_td against the REAL QMK pipeline: SMTD_ENABLE_QMK_TAPHOLD with raw MT()/LT().
 *
 * These tests assert the behavior sm_td is *designed* to produce (tap -> letter,
 * hold -> mod/layer, roll -> two letters). The open question they answer is
 * whether QMK's own action_tapping intercepts MT()/LT() before sm_td sees them
 * (see the SMTD_ENABLE_QMK_TAPHOLD caveat in AGENTS.md). With this suite's
 * default config (action_tapping enabled), divergence here is the finding;
 * the sibling no_action_tapping suite is the A/B control. */

#include "keyboard_report_util.hpp"
#include "test_common.hpp"

using testing::InSequence;

/* sm_td keeps global runtime state (state pool, active states, deferred tokens)
 * that the QMK test fixture does not reset between tests; clear it per test. */
extern "C" void smtd_reset(void);

class SmTdQmkTapHold : public TestFixture {
  protected:
    void SetUp() override { smtd_reset(); }
};

/* Plain (non tap-hold) keycode is emulated through the pipeline as-is. */
TEST_F(SmTdQmkTapHold, plain_key_passthrough) {
    TestDriver driver;
    InSequence s;
    KeymapKey plain = KeymapKey(0, 1, 0, KC_B);
    set_keymap({plain});

    EXPECT_REPORT(driver, (KC_B));
    EXPECT_EMPTY_REPORT(driver);
    plain.press();
    run_one_scan_loop();
    plain.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* Quick tap on MT() should yield the tap keycode, no modifier. */
TEST_F(SmTdQmkTapHold, mt_tap_produces_letter) {
    TestDriver driver;
    InSequence s;
    KeymapKey mt = KeymapKey(0, 1, 0, MT(MOD_LCTL, KC_A), KC_A);
    set_keymap({mt});

    EXPECT_REPORT(driver, (KC_A));
    EXPECT_EMPTY_REPORT(driver);
    mt.press();
    run_one_scan_loop();
    mt.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* Hold on MT() resolved by a following key applies the modifier to that key. */
TEST_F(SmTdQmkTapHold, mt_hold_with_following_key) {
    TestDriver driver;
    InSequence s;
    KeymapKey mt    = KeymapKey(0, 1, 0, MT(MOD_LCTL, KC_A), KC_A);
    KeymapKey other = KeymapKey(0, 2, 0, KC_B);
    set_keymap({mt, other});

    mt.press();
    idle_for(TAPPING_TERM + 50);   /* long enough to resolve as hold */

    /* sm_td registers the mod when the following key arrives, then taps it:
     * QMK emits the modifier in its own report before the key+mod report. */
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_B));
    EXPECT_REPORT(driver, (KC_LCTL));
    other.press();
    run_one_scan_loop();
    other.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_EMPTY_REPORT(driver);
    mt.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* Quick tap on LT() should yield the tap keycode and not switch layers. */
TEST_F(SmTdQmkTapHold, lt_tap_produces_letter) {
    TestDriver driver;
    InSequence s;
    KeymapKey lt = KeymapKey(0, 1, 0, LT(1, KC_C), KC_C);
    set_keymap({lt});

    EXPECT_REPORT(driver, (KC_C));
    EXPECT_EMPTY_REPORT(driver);
    lt.press();
    run_one_scan_loop();
    lt.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
    EXPECT_TRUE(layer_state_is(0));
}

/* Fast roll of two MT() keys should produce two letters, no modifiers. */
TEST_F(SmTdQmkTapHold, roll_two_mt_keys) {
    TestDriver driver;
    InSequence s;
    KeymapKey mt1 = KeymapKey(0, 1, 0, MT(MOD_LCTL, KC_A), KC_A);
    KeymapKey mt2 = KeymapKey(0, 2, 0, MT(MOD_LSFT, KC_S), KC_S);
    set_keymap({mt1, mt2});

    EXPECT_REPORT(driver, (KC_A));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_S));
    EXPECT_EMPTY_REPORT(driver);

    mt1.press();
    run_one_scan_loop();
    mt2.press();
    run_one_scan_loop();
    mt1.release();
    run_one_scan_loop();
    mt2.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}
