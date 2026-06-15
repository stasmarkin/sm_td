/* Dynamic release-term (ratio 5) against the real pipeline. The release rhythm
 * decides tap-tap vs hold-tap: a roll whose release gap mirrors the press gap is
 * tap-tap; a near-simultaneous release is hold-tap. idle_for(ms) is the clock. */

#include "keyboard_report_util.hpp"
#include "test_common.hpp"

using testing::InSequence;

extern "C" void smtd_reset(void);

class SmTdDynamic : public TestFixture {
  protected:
    void SetUp() override { smtd_reset(); }
};

/* Steady roll: press gap ~= release gap -> both keys are taps, no shift. */
TEST_F(SmTdDynamic, mt_steady_roll_is_tap_tap) {
    TestDriver driver;
    InSequence s;
    KeymapKey a = KeymapKey(0, 0, 0, KC_A);   /* SMTD_MT shift */
    KeymapKey b = KeymapKey(0, 4, 0, KC_B);   /* plain */
    set_keymap({a, b});

    EXPECT_REPORT(driver, (KC_A));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_B));
    EXPECT_EMPTY_REPORT(driver);
    a.press();
    idle_for(50);
    b.press();
    idle_for(50);
    a.release();
    idle_for(50);
    b.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* Fast release: B released almost together with A -> A holds shift, B under it. */
TEST_F(SmTdDynamic, mt_fast_release_is_hold_tap) {
    TestDriver driver;
    InSequence s;
    KeymapKey a = KeymapKey(0, 0, 0, KC_A);
    KeymapKey b = KeymapKey(0, 4, 0, KC_B);
    set_keymap({a, b});

    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_B));
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);
    a.press();
    idle_for(60);
    b.press();
    idle_for(60);
    a.release();
    idle_for(5);
    b.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* LT steady roll: tap-tap, no layer change. */
TEST_F(SmTdDynamic, lt_steady_roll_is_tap_tap) {
    TestDriver driver;
    InSequence s;
    KeymapKey d = KeymapKey(0, 2, 0, KC_D);   /* SMTD_LT layer 1 */
    KeymapKey b = KeymapKey(0, 4, 0, KC_B);
    set_keymap({d, b});

    EXPECT_REPORT(driver, (KC_D));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_B));
    EXPECT_EMPTY_REPORT(driver);
    d.press();
    idle_for(50);
    b.press();
    idle_for(50);
    d.release();
    idle_for(50);
    b.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
    EXPECT_TRUE(layer_state_is(0));
}

/* LT fast release: layer pushed, following key resolves on layer 1. */
TEST_F(SmTdDynamic, lt_fast_release_is_hold_tap) {
    TestDriver driver;
    InSequence s;
    KeymapKey d     = KeymapKey(0, 2, 0, KC_D);
    KeymapKey cell0 = KeymapKey(0, 4, 0, KC_B);   /* layer 0 at (4,0) */
    KeymapKey cell1 = KeymapKey(1, 4, 0, KC_M);   /* layer 1 at (4,0) */
    set_keymap({d, cell0, cell1});

    EXPECT_REPORT(driver, (KC_M));
    EXPECT_EMPTY_REPORT(driver);
    d.press();
    idle_for(60);
    cell0.press();
    idle_for(60);
    d.release();
    idle_for(5);
    cell0.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* Combo-like: long overlap but the release gap (45ms) far exceeds the window
 * (min(50,140)/5 = 10ms) -> resolves tap-tap. */
TEST_F(SmTdDynamic, combo_like_release_is_tap_tap) {
    TestDriver driver;
    InSequence s;
    KeymapKey a = KeymapKey(0, 0, 0, KC_A);
    KeymapKey b = KeymapKey(0, 4, 0, KC_B);
    set_keymap({a, b});

    EXPECT_REPORT(driver, (KC_A));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_B));
    EXPECT_EMPTY_REPORT(driver);
    a.press();
    idle_for(50);
    b.press();
    idle_for(140);
    a.release();
    idle_for(45);
    b.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* Instant overlap: everything in one tick -> window clamps to 1ms, simultaneous
 * release beats it -> hold-tap. */
TEST_F(SmTdDynamic, instant_overlap_is_hold_tap) {
    TestDriver driver;
    InSequence s;
    KeymapKey a = KeymapKey(0, 0, 0, KC_A);
    KeymapKey b = KeymapKey(0, 4, 0, KC_B);
    set_keymap({a, b});

    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_B));
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);
    a.press();
    b.press();
    run_one_scan_loop();
    a.release();
    b.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* Per-key release cap: KC_F caps the window at 5ms; an 8ms release gap exceeds it
 * even though the dynamic value (min(60,60)/5 = 12ms) would not -> tap-tap. */
TEST_F(SmTdDynamic, per_key_cap_release_is_tap_tap) {
    TestDriver driver;
    InSequence s;
    KeymapKey f = KeymapKey(0, 1, 0, KC_F);
    KeymapKey b = KeymapKey(0, 4, 0, KC_B);
    set_keymap({f, b});

    EXPECT_REPORT(driver, (KC_F));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_B));
    EXPECT_EMPTY_REPORT(driver);
    f.press();
    idle_for(60);
    b.press();
    idle_for(60);
    f.release();
    idle_for(8);
    b.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* Within the per-key cap (3ms < 5ms) -> hold-tap. */
TEST_F(SmTdDynamic, per_key_cap_within_is_hold_tap) {
    TestDriver driver;
    InSequence s;
    KeymapKey f = KeymapKey(0, 1, 0, KC_F);
    KeymapKey b = KeymapKey(0, 4, 0, KC_B);
    set_keymap({f, b});

    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_B));
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);
    f.press();
    idle_for(60);
    b.press();
    idle_for(60);
    f.release();
    idle_for(3);
    b.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* Three-key steady roll of MT keys -> three clean taps (each MT resolves on its
 * own release rhythm). */
TEST_F(SmTdDynamic, three_key_steady_roll_is_tap_tap_tap) {
    TestDriver driver;
    InSequence s;
    KeymapKey a = KeymapKey(0, 0, 0, KC_A);   /* MT LShift */
    KeymapKey ss = KeymapKey(0, 1, 0, KC_S);  /* MT LGui  */
    KeymapKey g = KeymapKey(0, 5, 0, KC_G);   /* MT LAlt  */
    set_keymap({a, ss, g});

    EXPECT_REPORT(driver, (KC_A));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_S));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_G));
    EXPECT_EMPTY_REPORT(driver);
    a.press();
    idle_for(40);
    ss.press();
    idle_for(40);
    g.press();
    idle_for(40);
    a.release();
    idle_for(40);
    ss.release();
    idle_for(40);
    g.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}
