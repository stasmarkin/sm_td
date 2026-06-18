/* Upper-clamp release term (percent 100): the window is min(p1, p2) but is never
 * allowed above SMTD_GLOBAL_RELEASE_TERM (50ms). */

#include "keyboard_report_util.hpp"
#include "test_common.hpp"

using testing::InSequence;

extern "C" void smtd_reset(void);

class SmTdDynamicClamp : public TestFixture {
  protected:
    void SetUp() override { smtd_reset(); }
};

/* p1=p2=80 -> min would be 80, clamped to 50; a 60ms release gap exceeds it -> tap-tap. */
TEST_F(SmTdDynamicClamp, window_never_exceeds_fixed_term) {
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
    idle_for(80);
    b.press();
    idle_for(80);
    a.release();
    idle_for(60);
    b.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* Within the clamped window (40ms < 50ms) -> hold-tap. */
TEST_F(SmTdDynamicClamp, release_within_clamped_window_is_hold_tap) {
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
    idle_for(80);
    b.press();
    idle_for(80);
    a.release();
    idle_for(40);
    b.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}
