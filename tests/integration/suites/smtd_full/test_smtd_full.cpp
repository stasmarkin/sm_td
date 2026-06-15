/* Full sm_td engine against the real QMK pipeline. on_smtd_action (in smtd_hooks.c)
 * maps real keycodes to every customization macro:
 *   KC_A -> SMTD_MT(LCtrl)   KC_J -> SMTD_MT(LAlt)   KC_S -> SMTD_MTE(LShift)
 *   KC_D -> SMTD_LT(layer 1) KC_F -> SMTD_TD(ESC)    KC_G -> SMTD_TK(TAB, 3)
 *   KC_H -> SMTD_TTO(layer 2, 3)
 * Plain keys (KC_B, layer-1 KC_M) are passed through.
 * Release term is fixed (ratio 0) so tap/hold/roll depend only on the press pattern. */

#include "keyboard_report_util.hpp"
#include "test_common.hpp"

using testing::_;
using testing::AnyNumber;
using testing::InSequence;

extern "C" void smtd_reset(void);
extern "C" uint8_t get_mods(void);

class SmTdFull : public TestFixture {
  protected:
    void SetUp() override { smtd_reset(); }
};

/* ---- plain passthrough ---- */
TEST_F(SmTdFull, plain_key_passthrough) {
    TestDriver driver;
    InSequence s;
    KeymapKey b = KeymapKey(0, 4, 0, KC_B);
    set_keymap({b});

    EXPECT_REPORT(driver, (KC_B));
    EXPECT_EMPTY_REPORT(driver);
    b.press();
    run_one_scan_loop();
    b.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* ---- SMTD_MT ---- */
TEST_F(SmTdFull, mt_tap_sends_letter) {
    TestDriver driver;
    InSequence s;
    KeymapKey a = KeymapKey(0, 0, 0, KC_A);
    set_keymap({a});

    EXPECT_REPORT(driver, (KC_A));
    EXPECT_EMPTY_REPORT(driver);
    a.press();
    run_one_scan_loop();
    a.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

TEST_F(SmTdFull, mt_hold_registers_mod_on_following_key) {
    TestDriver driver;
    InSequence s;
    KeymapKey a = KeymapKey(0, 0, 0, KC_A);
    KeymapKey b = KeymapKey(0, 4, 0, KC_B);
    set_keymap({a, b});

    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_B));
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_EMPTY_REPORT(driver);
    a.press();
    idle_for(TAPPING_TERM + 50);   /* HOLD -> LCtrl */
    b.press();
    run_one_scan_loop();
    b.release();
    run_one_scan_loop();
    a.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

TEST_F(SmTdFull, mt_tap_then_hold_holds_letter) {
    /* threshold defaults to 1: on the 2nd press (tap_count>=1) HOLD registers the
     * tap key itself instead of the mod. */
    TestDriver driver;
    InSequence s;
    KeymapKey a = KeymapKey(0, 0, 0, KC_A);
    set_keymap({a});

    EXPECT_REPORT(driver, (KC_A));   /* first tap */
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_A));   /* second press resolves to held letter */
    EXPECT_EMPTY_REPORT(driver);
    a.press();
    run_one_scan_loop();
    a.release();
    run_one_scan_loop();
    a.press();
    idle_for(TAPPING_TERM + 50);
    a.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);
}

/* ---- SMTD_MTE (eager mod) ---- */
TEST_F(SmTdFull, mte_hold_with_following_key_applies_shift) {
    /* Eager mod registers LShift on touch; a following key is typed under it. */
    TestDriver driver;
    InSequence s;
    KeymapKey ss = KeymapKey(0, 1, 0, KC_S);
    KeymapKey b  = KeymapKey(0, 4, 0, KC_B);
    set_keymap({ss, b});

    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_B));
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);
    ss.press();
    idle_for(TAPPING_TERM + 50);
    b.press();
    run_one_scan_loop();
    b.release();
    run_one_scan_loop();
    ss.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

TEST_F(SmTdFull, mte_tap_sends_letter) {
    /* Eager mod is registered on touch, then dropped and the letter tapped on a
     * quick release: LShift down, LShift up, then KC_S tap. */
    TestDriver driver;
    InSequence s;
    KeymapKey ss = KeymapKey(0, 1, 0, KC_S);
    set_keymap({ss});

    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_S));
    EXPECT_EMPTY_REPORT(driver);
    ss.press();
    run_one_scan_loop();
    ss.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* ---- SMTD_LT ---- */
TEST_F(SmTdFull, lt_tap_sends_letter_no_layer) {
    TestDriver driver;
    InSequence s;
    KeymapKey d = KeymapKey(0, 2, 0, KC_D);
    set_keymap({d});

    EXPECT_REPORT(driver, (KC_D));
    EXPECT_EMPTY_REPORT(driver);
    d.press();
    run_one_scan_loop();
    d.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
    EXPECT_TRUE(layer_state_is(0));
}

TEST_F(SmTdFull, lt_hold_pushes_layer_then_restores) {
    /* Observable layer check: the same matrix cell (4,0) resolves to KC_M while the
     * LT layer is held and back to KC_N after release. */
    TestDriver driver;
    KeymapKey d0  = KeymapKey(0, 2, 0, KC_D);
    KeymapKey cell0 = KeymapKey(0, 4, 0, KC_N);   /* layer 0 at (4,0) */
    KeymapKey cell1 = KeymapKey(1, 4, 0, KC_M);   /* layer 1 at (4,0) */
    set_keymap({d0, cell0, cell1});

    d0.press();
    idle_for(TAPPING_TERM + 50);   /* HOLD -> push layer 1 */

    {
        InSequence s;
        EXPECT_REPORT(driver, (KC_M));
        EXPECT_EMPTY_REPORT(driver);
        cell0.press();             /* position (4,0); resolves on active layer */
        run_one_scan_loop();
        cell0.release();
        run_one_scan_loop();
        VERIFY_AND_CLEAR(driver);
    }

    d0.release();
    idle_for(TAPPING_TERM + 50);   /* RELEASE -> restore layer 0 */

    {
        InSequence s;
        EXPECT_REPORT(driver, (KC_N));
        EXPECT_EMPTY_REPORT(driver);
        cell0.press();
        run_one_scan_loop();
        cell0.release();
        idle_for(TAPPING_TERM + 50);
        VERIFY_AND_CLEAR(driver);
    }
}

/* ---- SMTD_TD ---- */
TEST_F(SmTdFull, td_tap_sends_tap_key) {
    TestDriver driver;
    InSequence s;
    KeymapKey f = KeymapKey(0, 3, 0, KC_F);
    set_keymap({f});

    EXPECT_REPORT(driver, (KC_F));
    EXPECT_EMPTY_REPORT(driver);
    f.press();
    run_one_scan_loop();
    f.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

TEST_F(SmTdFull, td_hold_alone_sends_tap_key) {
    /* A lone long press of an SMTD_TD key resolves to the tap key (the hold key is
     * produced only when interrupted by another key). Document the observed output. */
    TestDriver driver;
    InSequence s;
    KeymapKey f = KeymapKey(0, 3, 0, KC_F);
    set_keymap({f});

    EXPECT_REPORT(driver, (KC_F));
    EXPECT_EMPTY_REPORT(driver);
    f.press();
    idle_for(TAPPING_TERM + 50);
    f.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* ---- roll: two MT keys with a FIXED release term (ratio 0) -> hold-tap ----
 * With dynamic release disabled, a fast overlapping roll resolves the first key as
 * a hold: KC_A becomes LCtrl and KC_J is typed under it. The tap-tap resolution of
 * rolls is a dynamic-release feature, covered in the smtd_dynamic suite. */
TEST_F(SmTdFull, roll_two_mt_keys_fixed_release_is_hold_tap) {
    TestDriver driver;
    InSequence s;
    KeymapKey a = KeymapKey(0, 0, 0, KC_A);   /* MT LCtrl */
    KeymapKey j = KeymapKey(0, 5, 0, KC_J);   /* MT LAlt  */
    set_keymap({a, j});

    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_J));
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_EMPTY_REPORT(driver);
    a.press();
    run_one_scan_loop();
    j.press();
    run_one_scan_loop();
    a.release();
    run_one_scan_loop();
    j.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* ---- multi-tap: three quick taps on an MT key send three letters ---- */
TEST_F(SmTdFull, mt_three_taps_send_three_letters) {
    TestDriver driver;
    InSequence s;
    KeymapKey a = KeymapKey(0, 0, 0, KC_A);
    set_keymap({a});

    for (int i = 0; i < 3; i++) {
        EXPECT_REPORT(driver, (KC_A));
        EXPECT_EMPTY_REPORT(driver);
    }
    for (int i = 0; i < 3; i++) {
        a.press();
        run_one_scan_loop();
        a.release();
        run_one_scan_loop();
    }
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* ---- external (non-sm_td) modifier interacts with an sm_td tap ----
 * KC_LSFT is a plain QMK mod (passed through), held while an MT key is tapped. */
TEST_F(SmTdFull, external_shift_held_over_mt_tap) {
    TestDriver driver;
    InSequence s;
    KeymapKey sh = KeymapKey(0, 6, 0, KC_LSFT);   /* real modifier, not an sm_td key */
    KeymapKey a  = KeymapKey(0, 0, 0, KC_A);       /* SMTD_MT */
    set_keymap({sh, a});

    EXPECT_REPORT(driver, (KC_LSFT));            /* external shift down */
    EXPECT_REPORT(driver, (KC_LSFT, KC_A));      /* MT tap under shift */
    EXPECT_REPORT(driver, (KC_LSFT));            /* tap released, shift still held */
    EXPECT_EMPTY_REPORT(driver);                 /* shift up */
    sh.press();
    run_one_scan_loop();
    a.press();
    run_one_scan_loop();
    a.release();
    run_one_scan_loop();
    sh.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* ---- SMTD_TK: multi-tap-activated key (TAB after tap_count reaches 2) ---- */
TEST_F(SmTdFull, tk_fires_tap_key_after_threshold) {
    TestDriver driver;
    InSequence s;
    KeymapKey g = KeymapKey(0, 7, 0, KC_G);
    set_keymap({g});

    /* Only the 3rd press (tap_count == 2 == threshold) emits TAB; earlier presses
     * are silent. */
    EXPECT_REPORT(driver, (KC_TAB));
    EXPECT_EMPTY_REPORT(driver);
    for (int i = 0; i < 3; i++) {
        g.press();
        run_one_scan_loop();
        g.release();
        run_one_scan_loop();
    }
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* ---- SMTD_TTO: multi-tap-to-layer (move to layer 2 after tap_count reaches 2) ---- */
TEST_F(SmTdFull, tto_moves_layer_after_threshold) {
    TestDriver driver;
    KeymapKey h     = KeymapKey(0, 7, 0, KC_H);
    KeymapKey cell0 = KeymapKey(0, 4, 0, KC_N);   /* layer 0 at (4,0) */
    KeymapKey cell2 = KeymapKey(2, 4, 0, KC_O);   /* layer 2 at (4,0) */
    set_keymap({h, cell0, cell2});

    for (int i = 0; i < 3; i++) {
        h.press();
        run_one_scan_loop();
        h.release();
        run_one_scan_loop();
    }
    idle_for(TAPPING_TERM + 50);   /* 3rd press moved to layer 2 */

    InSequence s;
    EXPECT_REPORT(driver, (KC_O));   /* (4,0) now resolves on layer 2 */
    EXPECT_EMPTY_REPORT(driver);
    cell0.press();                   /* position (4,0) */
    run_one_scan_loop();
    cell0.release();
    idle_for(TAPPING_TERM + 50);
    VERIFY_AND_CLEAR(driver);
}

/* ---- LT layer + MT mod composed on a following key ---- */
TEST_F(SmTdFull, lt_layer_and_mt_mod_compose) {
    TestDriver driver;
    KeymapKey d     = KeymapKey(0, 2, 0, KC_D);   /* SMTD_LT layer 1 */
    KeymapKey a0    = KeymapKey(0, 0, 0, KC_A);   /* SMTD_MT LCtrl on layer 0 */
    KeymapKey a1    = KeymapKey(1, 0, 0, KC_A);   /* same on layer 1 (A pressed after push) */
    KeymapKey a     = a0;                          /* press position (0,0) */
    KeymapKey cell0 = KeymapKey(0, 4, 0, KC_N);   /* layer 0 at (4,0) */
    KeymapKey cell1 = KeymapKey(1, 4, 0, KC_M);   /* layer 1 at (4,0) */
    set_keymap({d, a0, a1, cell0, cell1});

    d.press();
    idle_for(TAPPING_TERM + 50);   /* push layer 1 */
    a.press();
    idle_for(TAPPING_TERM + 50);   /* A pending; commits to Ctrl on the cell */

    InSequence s;
    EXPECT_REPORT(driver, (KC_LCTL));
    EXPECT_REPORT(driver, (KC_LCTL, KC_M));   /* layer-1 key under Ctrl */
    EXPECT_REPORT(driver, (KC_LCTL));
    cell0.press();
    run_one_scan_loop();
    cell0.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    a.release();
    run_one_scan_loop();
    d.release();
    idle_for(TAPPING_TERM + 50);
    EXPECT_EQ(get_mods(), 0);
    EXPECT_TRUE(layer_state_is(0));
}

/* Faceroll regression #1: an SMTD_MT key overlapped by an SMTD_MTE (eager) key.
 * Earlier captured as a "segfault", but the crash was a harness artifact, NOT an
 * engine bug: these tests declared no `TestDriver driver;`, so the host driver
 * pointer (set from a TestDriver in SetUpTestCase, then destroyed) dangled and
 * led_task()->host_keyboard_leds() dereferenced freed stack every scan. With a
 * live driver the sequence runs cleanly and ends with no stuck mods. */
TEST_F(SmTdFull, mt_overlapped_by_mte) {
    TestDriver driver;
    EXPECT_ANY_REPORT(driver).Times(AnyNumber());
    KeymapKey a  = KeymapKey(0, 0, 0, KC_A);   /* SMTD_MT  */
    KeymapKey ss = KeymapKey(0, 1, 0, KC_S);   /* SMTD_MTE */
    set_keymap({a, ss});

    a.press();    run_one_scan_loop();
    ss.press();   run_one_scan_loop();
    a.release();  run_one_scan_loop();
    ss.release(); idle_for(TAPPING_TERM + 50);

    EXPECT_EQ(get_mods(), 0);
}

/* Faceroll regression #2: a deeper interleave of two SMTD_MT keys with plain keys,
 * releasing inner keys while others are still held. (See #1 on the former
 * "segfault" — a missing TestDriver, not an engine bug.) */
TEST_F(SmTdFull, mt_plain_interleave) {
    TestDriver driver;
    EXPECT_ANY_REPORT(driver).Times(AnyNumber());
    KeymapKey a = KeymapKey(0, 0, 0, KC_A);   /* MT */
    KeymapKey j = KeymapKey(0, 5, 0, KC_J);   /* MT */
    KeymapKey b = KeymapKey(0, 4, 0, KC_B);   /* plain */
    KeymapKey c = KeymapKey(0, 6, 0, KC_C);   /* plain */
    set_keymap({a, j, b, c});

    a.press();   run_one_scan_loop();
    j.press();   run_one_scan_loop();
    b.press();   run_one_scan_loop();
    a.release(); run_one_scan_loop();
    c.press();   run_one_scan_loop();
    b.release(); run_one_scan_loop();
    j.release(); run_one_scan_loop();
    c.release(); idle_for(TAPPING_TERM + 50);

    EXPECT_EQ(get_mods(), 0);
}

/* Faceroll regression #3: holding an LT layer then pressing a key that maps to
 * KC_NO on the pushed layer (a "dead" cell, as a real keymap would have rather
 * than the fixture's hard-FAIL on a fully unmapped cell). (See #1 on the former
 * "segfault" — a missing TestDriver, not an engine bug.) */
TEST_F(SmTdFull, lt_hold_with_unmapped_follow_key) {
    TestDriver driver;
    EXPECT_ANY_REPORT(driver).Times(AnyNumber());
    KeymapKey d   = KeymapKey(0, 2, 0, KC_D);   /* SMTD_LT layer 1 */
    KeymapKey k0  = KeymapKey(0, 4, 0, KC_B);   /* mapped on layer 0 */
    KeymapKey k1  = KeymapKey(1, 4, 0, KC_NO);  /* dead cell on the pushed layer */
    set_keymap({d, k0, k1});

    d.press();
    idle_for(TAPPING_TERM + 50);   /* push layer 1 */
    k0.press();                    /* (4,0) resolves to KC_NO on layer 1 */
    run_one_scan_loop();
    k0.release();
    run_one_scan_loop();
    d.release();
    idle_for(TAPPING_TERM + 50);
    EXPECT_TRUE(layer_state_is(0));
}
