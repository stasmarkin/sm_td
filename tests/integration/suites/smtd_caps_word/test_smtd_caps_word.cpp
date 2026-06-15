/* sm_td against the REAL QMK Caps Word. sm_td's pipeline taps go through
 * process_record, so process_caps_word sees them and applies the weak shift; a
 * non-member key ends Caps Word. */

#include "keyboard_report_util.hpp"
#include "test_common.hpp"

using testing::InSequence;

extern "C" void smtd_reset(void);
extern "C" void caps_word_on(void);
extern "C" void caps_word_off(void);
extern "C" bool is_caps_word_on(void);

class SmTdCapsWord : public TestFixture {
  protected:
    void SetUp() override { smtd_reset(); caps_word_off(); }
};

/* An MT() letter tapped under Caps Word is shifted. */
TEST_F(SmTdCapsWord, mt_tap_is_shifted_under_caps_word) {
    TestDriver driver;
    InSequence s;
    KeymapKey a = KeymapKey(0, 0, 0, KC_A);
    set_keymap({a});
    caps_word_on();

    /* Caps Word applies the weak shift in its own report, then the key; the weak
     * shift lingers in the report until the next key (cleared only at teardown). */
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_A));
    EXPECT_REPORT(driver, (KC_LSFT));
    a.press();
    run_one_scan_loop();
    a.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
    EXPECT_TRUE(is_caps_word_on());
}

/* A plain letter (emulated through the pipeline) is also shifted and keeps Caps Word on. */
TEST_F(SmTdCapsWord, plain_letter_is_shifted_under_caps_word) {
    TestDriver driver;
    InSequence s;
    KeymapKey b = KeymapKey(0, 4, 0, KC_B);
    set_keymap({b});
    caps_word_on();

    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_B));
    EXPECT_REPORT(driver, (KC_LSFT));
    b.press();
    run_one_scan_loop();
    b.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
    EXPECT_TRUE(is_caps_word_on());
}

/* A word-breaking key (space) ends Caps Word and is sent unshifted. */
TEST_F(SmTdCapsWord, space_breaks_caps_word) {
    TestDriver driver;
    InSequence s;
    KeymapKey spc = KeymapKey(0, 5, 0, KC_SPC);
    set_keymap({spc});
    caps_word_on();

    EXPECT_REPORT(driver, (KC_SPC));
    EXPECT_EMPTY_REPORT(driver);
    spc.press();
    run_one_scan_loop();
    spc.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
    EXPECT_FALSE(is_caps_word_on());
}

/* A use_cl=false mod-tap is hidden from Caps Word: its tap is NOT shifted and Caps
 * Word stays on (the key is invisible to it). */
TEST_F(SmTdCapsWord, use_cl_false_tap_is_not_shifted) {
    TestDriver driver;
    InSequence s;
    KeymapKey x = KeymapKey(0, 6, 0, KC_X);
    set_keymap({x});
    caps_word_on();

    EXPECT_REPORT(driver, (KC_X));
    EXPECT_EMPTY_REPORT(driver);
    x.press();
    run_one_scan_loop();
    x.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
    EXPECT_TRUE(is_caps_word_on());
}

/* Holding a non-shift sm_td mod-tap (Ctrl) and typing under it turns Caps Word off
 * and the key is sent unshifted -- same as native QMK (a non-shift mod breaks Caps
 * Word). Verified through the real pipeline. */
TEST_F(SmTdCapsWord, mt_hold_nonshift_mod_breaks_caps_word) {
    TestDriver driver;
    InSequence s;
    KeymapKey a = KeymapKey(0, 0, 0, KC_A);
    KeymapKey b = KeymapKey(0, 4, 0, KC_B);
    set_keymap({a, b});
    caps_word_on();

    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_B));   /* B unshifted: Caps Word broke */
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_EMPTY_REPORT(driver);
    a.press();
    idle_for(TAPPING_TERM + 50);   /* A -> LCtrl (hold) */
    b.press();
    run_one_scan_loop();
    b.release();
    run_one_scan_loop();
    a.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
    EXPECT_FALSE(is_caps_word_on());
}
