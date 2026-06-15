/* sm_td interacting with an external (plain QMK) modifier. KC_LSFT is a real mod
 * (passed through); KC_A is an sm_td SMTD_MT(LCtrl). Verifies mods compose on a
 * following key and that every release order returns to a clean state. */

#include "keyboard_report_util.hpp"
#include "test_common.hpp"

using testing::InSequence;

extern "C" void smtd_reset(void);
extern "C" uint8_t get_mods(void);

class SmTdExternalMods : public TestFixture {
  protected:
    void SetUp() override { smtd_reset(); }
};

/* SKKS: external Shift held over a plain key. */
TEST_F(SmTdExternalMods, ext_shift_over_plain_key) {
    TestDriver driver;
    InSequence s;
    KeymapKey sh = KeymapKey(0, 0, 0, KC_LSFT);
    KeymapKey b  = KeymapKey(0, 2, 0, KC_B);
    set_keymap({sh, b});

    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_B));
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);
    sh.press();
    run_one_scan_loop();
    b.press();
    run_one_scan_loop();
    b.release();
    run_one_scan_loop();
    sh.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* sm_td MT(Ctrl) held over a plain key. */
TEST_F(SmTdExternalMods, mt_ctrl_hold_over_plain_key) {
    TestDriver driver;
    InSequence s;
    KeymapKey a = KeymapKey(0, 1, 0, KC_A);
    KeymapKey b = KeymapKey(0, 2, 0, KC_B);
    set_keymap({a, b});

    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_B));
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_EMPTY_REPORT(driver);
    a.press();
    idle_for(TAPPING_TERM + 50);
    b.press();
    run_one_scan_loop();
    b.release();
    run_one_scan_loop();
    a.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* External Shift + sm_td Ctrl both held over a key -> both mods composed.
 * The sm_td Ctrl commits when the following key arrives (its own mod-down report),
 * so the full sequence is shift, ctrl, key+both, key-up, ctrl-up, shift-up. */
TEST_F(SmTdExternalMods, ext_shift_and_mt_ctrl_compose_on_key) {
    TestDriver driver;
    InSequence s;
    KeymapKey sh = KeymapKey(0, 0, 0, KC_LSFT);
    KeymapKey a  = KeymapKey(0, 1, 0, KC_A);
    KeymapKey b  = KeymapKey(0, 2, 0, KC_B);
    set_keymap({sh, a, b});

    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_LCTL));
    EXPECT_REPORT(driver, (KC_LSFT, KC_LCTL, KC_B));
    EXPECT_REPORT(driver, (KC_LSFT, KC_LCTL));
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);
    sh.press();
    run_one_scan_loop();
    a.press();
    idle_for(TAPPING_TERM + 50);   /* A pending; commits to Ctrl on B */
    b.press();
    run_one_scan_loop();
    b.release();
    run_one_scan_loop();
    a.release();
    run_one_scan_loop();
    sh.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
    EXPECT_EQ(get_mods(), 0);
}

/* Every release order of {ext Shift, sm_td Ctrl, plain key} returns to clean mods. */
TEST_F(SmTdExternalMods, all_release_orders_return_clean) {
    KeymapKey sh = KeymapKey(0, 0, 0, KC_LSFT);
    KeymapKey a  = KeymapKey(0, 1, 0, KC_A);
    KeymapKey b  = KeymapKey(0, 2, 0, KC_B);

    const int perms[6][3] = {
        {0,1,2}, {0,2,1}, {1,0,2}, {1,2,0}, {2,0,1}, {2,1,0},
    };
    for (int p = 0; p < 6; p++) {
        TestDriver driver;            /* swallow reports; we assert final state */
        smtd_reset();
        set_keymap({sh, a, b});
        KeymapKey keys[3] = {sh, a, b};

        sh.press();
        run_one_scan_loop();
        a.press();
        idle_for(TAPPING_TERM + 50);  /* A resolves to Ctrl */
        b.press();
        run_one_scan_loop();

        for (int i = 0; i < 3; i++) {
            keys[perms[p][i]].release();
            run_one_scan_loop();
        }
        idle_for(TAPPING_TERM + 50);
        EXPECT_EQ(get_mods(), 0) << "stuck mods after release order "
                                 << perms[p][0] << perms[p][1] << perms[p][2];
    }
}
