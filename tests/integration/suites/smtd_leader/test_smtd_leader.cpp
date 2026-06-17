/* sm_td + real QMK Leader Key (issue #29).
 *
 * A leader sequence is started, then an sm_td mod-tap key is TAPPED, followed by
 * a plain key, then the leader times out. leader_end_user emits KC_1 iff the
 * sequence (KC_A, KC_C) was captured by the leader buffer.
 *
 * native: keymap position is the bare KC_A. The resolved tap goes through the
 *         pipeline (process_record), so process_leader (which runs after
 *         process_record_user in the quantum chain) sees it. Leader matches and
 *         emits KC_1. -> FIXED by pipeline taps (SMTD_GLOBAL_PIPELINE_TAPS).
 *
 * custom: keymap position is a custom keycode CKC_A mapped to KC_A in
 *         on_smtd_action. The resolved tap cannot be pipelined (the position
 *         does not resolve to KC_A), so sm_td sends it directly. Without the #29
 *         fix this bypassed process_record and leaked KC_A to the host; the
 *         smtd_leader_consume shim now feeds the tap into the leader buffer, so
 *         the sequence is captured exactly like the native case. */

#include "keyboard_report_util.hpp"
#include "test_common.hpp"

using testing::InSequence;

extern "C" void smtd_reset(void);

enum { CKC_A = 0x7E40 /* SAFE_RANGE (QK_USER) on this QMK */ };

class SmTdLeader : public TestFixture {
  protected:
    void SetUp() override { smtd_reset(); }
};

/* Tap an sm_td mod-tap key cleanly: press, release, idle long enough for the
 * release term to expire so sm_td resolves and emits the TAP. */
#define SMTD_TAP(k)          \
    do {                     \
        (k).press();         \
        run_one_scan_loop(); \
        (k).release();       \
        idle_for(80);        \
    } while (0)

TEST_F(SmTdLeader, native_keycode_tap_is_captured_by_leader) {
    TestDriver driver;
    InSequence s;

    KeymapKey key_leader = KeymapKey(0, 0, 0, QK_LEADER);
    KeymapKey key_a      = KeymapKey(0, 1, 0, KC_A);
    KeymapKey key_c      = KeymapKey(0, 2, 0, KC_C);
    set_keymap({key_leader, key_a, key_c});

    tap_key(key_leader);
    EXPECT_EQ(leader_sequence_active(), true);

    /* The sequence marker KC_1 is the only thing that should reach the host:
     * both the mod-tap KC_A and the plain KC_C are swallowed by the leader. */
    EXPECT_REPORT(driver, (KC_1));
    EXPECT_EMPTY_REPORT(driver);

    SMTD_TAP(key_a); /* mod-tap KC_A, resolved as a tap, pipelined into leader */
    tap_key(key_c);  /* plain KC_C, captured by leader directly */

    idle_for(LEADER_TIMEOUT + 50); /* leader times out -> leader_end_user */
    VERIFY_AND_CLEAR(driver);
}

TEST_F(SmTdLeader, custom_keycode_tap_is_captured_by_leader) {
    TestDriver driver;
    InSequence s;

    KeymapKey key_leader = KeymapKey(0, 0, 0, QK_LEADER);
    KeymapKey key_a      = KeymapKey(0, 1, 0, (uint16_t)CKC_A);
    KeymapKey key_c      = KeymapKey(0, 2, 0, KC_C);
    set_keymap({key_leader, key_a, key_c});

    tap_key(key_leader);
    EXPECT_EQ(leader_sequence_active(), true);

    /* With the #29 fix, the custom-keycode tap is fed into the leader buffer by
     * smtd_leader_consume instead of leaking KC_A to the host, so the (A,C)
     * sequence matches and only the KC_1 marker reaches the host. */
    EXPECT_REPORT(driver, (KC_1));
    EXPECT_EMPTY_REPORT(driver);

    SMTD_TAP(key_a);
    tap_key(key_c);

    idle_for(LEADER_TIMEOUT + 50);
    VERIFY_AND_CLEAR(driver);
}
