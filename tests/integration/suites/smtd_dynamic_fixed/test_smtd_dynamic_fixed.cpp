/* Fixed release term (percent 0): the touch-release window is a constant 50ms,
 * independent of the roll rhythm. */

#include "keyboard_report_util.hpp"
#include "test_common.hpp"

using testing::InSequence;

extern "C" void smtd_reset(void);

class SmTdDynamicFixed : public TestFixture {
  protected:
    void SetUp() override { smtd_reset(); }
};

/* Release 30ms after the mod release -> within the 50ms window -> hold-tap. */
TEST_F(SmTdDynamicFixed, release_within_fixed_window_is_hold_tap) {
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
    idle_for(50);
    b.press();
    idle_for(50);
    a.release();
    idle_for(30);
    b.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* Release 60ms after the mod release -> beyond the 50ms window -> tap-tap. */
TEST_F(SmTdDynamicFixed, release_after_fixed_window_is_tap_tap) {
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
    idle_for(50);
    a.release();
    idle_for(60);
    b.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}
