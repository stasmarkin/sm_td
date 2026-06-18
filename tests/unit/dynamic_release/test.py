try:
    from tests.unit.sm_td_assertions import *
except ImportError:
    from sm_td_assertions import *

smtd_dyn = load_smtd_lib('tests/unit/dynamic_release/layout.c')
smtd_fixed = load_smtd_lib('tests/unit/dynamic_release/layout_fixed.c')
smtd_clamp = load_smtd_lib('tests/unit/dynamic_release/layout_clamp.c')
smtd_qk = load_smtd_lib('tests/unit/dynamic_release/layout_qk.c')
smtd_pct = load_smtd_lib('tests/unit/dynamic_release/layout_percent.c')


def build_keys(smtd, l0_values=None):
    """Two layers at row 0: keycodes 100..108 on L0 (overridable per column for
    packed MT()/LT() values) and 200..208 on L1.
    In layout.c K1 is SMTD_MT(L0_KC1, KC_LSFT) and K3 is SMTD_LT(L0_KC3, L1)."""
    keycodes = [Keycode(smtd, (l0_values or {}).get(col, 100 + col), 0, col, 0) for col in range(9)] + \
               [Keycode(smtd, 200 + col, 0, col, 1) for col in range(9)]
    keys = [Key(smtd, f'K{col}', 0, col, "row 0", keycodes) for col in range(9)]
    return keycodes, keys


def qmk_mt(mod, kc):
    return 0x2000 | ((mod & 0x1F) << 8) | (kc & 0xFF)


def qmk_lt(layer, kc):
    return 0x4000 | ((layer & 0xF) << 8) | (kc & 0xFF)


def reset(smtd, keycodes, keys):
    for keycode in keycodes:
        keycode.reset()
    for key in keys:
        key.reset()
    smtd.reset()


MOD_LSFT = 0x02  # MOD_BIT(KC_LSFT)


class TestDynamicReleaseTerm(SmTdAssertions):
    """SMTD_GLOBAL_RELEASE_PERCENT = 20: the ↑MOD..↑B decision window
    is min(p1, p2) * 20/100 where p1 = ↓MOD..↓B and p2 = ↓B..↑MOD"""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.smtd = smtd_dyn

    def setUp(self):
        super().setUp()
        reset(smtd_dyn, KEYCODES_DYN, KEYS_DYN)

    def test_steady_roll_is_tap_tap(self):
        """p1 = p2 = p3: releases follow the press rhythm, so it is a roll"""
        MOD_DYN.press()
        smtd_dyn.wait(50)
        B_DYN.press()
        smtd_dyn.wait(50)
        MOD_DYN.release()
        smtd_dyn.wait(50)  # window is min(50, 50) * 20/100 = 10ms, expires mid-wait
        B_DYN.release()

        self.assertHistory(
            pressed(MOD_DYN),
            released(MOD_DYN),
            pressed(B_DYN),
            released(B_DYN),
        )

    def test_fast_releases_is_hold_tap(self):
        """p3 much smaller than both p1 and p2: sloppy mod release, still a hold"""
        MOD_DYN.press()
        smtd_dyn.wait(60)
        B_DYN.press()
        smtd_dyn.wait(60)
        MOD_DYN.release()
        smtd_dyn.wait(5)  # window is min(60, 60) * 20/100 = 12ms, ↑B beats it
        B_DYN.release()

        self.assertHistory(
            pressed(B_DYN, mods=MOD_LSFT),
            released(B_DYN, mods=MOD_LSFT),
        )
        self.assertEqual(smtd_dyn.get_mods(), 0)

    def test_combo_like_release_is_tap_tap(self):
        """p1 ~= p3 << p2: long overlap, but releases mirror the press rhythm.
        The min(p1, p2) rule resolves this combo-like pattern as a roll"""
        MOD_DYN.press()
        smtd_dyn.wait(50)
        B_DYN.press()
        smtd_dyn.wait(140)
        MOD_DYN.release()
        smtd_dyn.wait(45)  # window is min(50, 140) * 20/100 = 10ms, expires mid-wait
        B_DYN.release()

        self.assertHistory(
            pressed(MOD_DYN),
            released(MOD_DYN),
            pressed(B_DYN),
            released(B_DYN),
        )

    def test_tiny_p1_requires_tinier_p3_for_hold(self):
        """Hold is still reachable with a tiny p1 when p3 is even smaller"""
        MOD_DYN.press()
        smtd_dyn.wait(20)
        B_DYN.press()
        smtd_dyn.wait(150)
        MOD_DYN.release()
        smtd_dyn.wait(3)  # window is min(20, 150) * 20/100 = 4ms, ↑B beats it
        B_DYN.release()

        self.assertHistory(
            pressed(B_DYN, mods=MOD_LSFT),
            released(B_DYN, mods=MOD_LSFT),
        )
        self.assertEqual(smtd_dyn.get_mods(), 0)

    def test_instant_overlap_is_hold_tap(self):
        """All events in the same millisecond: window clamps to 1ms, ↑B at 0ms wins"""
        MOD_DYN.press()
        B_DYN.press()
        MOD_DYN.release()
        B_DYN.release()

        self.assertHistory(
            pressed(B_DYN, mods=MOD_LSFT),
            released(B_DYN, mods=MOD_LSFT),
        )
        self.assertEqual(smtd_dyn.get_mods(), 0)

    def test_tap_term_hold_via_clock(self):
        """The virtual clock fires the tap-term timeout, no prolong() involved"""
        MOD_DYN.press()
        self.assertEqual(smtd_dyn.get_mods(), 0)

        smtd_dyn.wait(200)  # TAPPING_TERM
        self.assertEqual(smtd_dyn.get_mods(), MOD_LSFT, "hold must fire on tap term")

        MOD_DYN.release()
        self.assertEqual(smtd_dyn.get_mods(), 0)
        self.assertHistory()

    def test_smtd_lt_steady_roll_is_tap_tap(self):
        """SMTD_LT roll: layer key taps, following key stays on layer 0"""
        LT_DYN.press()
        smtd_dyn.wait(50)
        B_DYN.press()
        smtd_dyn.wait(50)
        LT_DYN.release()
        smtd_dyn.wait(50)  # window is min(50, 50) * 20/100 = 10ms, expires mid-wait
        B_DYN.release()

        self.assertHistory(
            pressed(LT_DYN),
            released(LT_DYN),
            pressed(B_DYN),
            released(B_DYN),
        )
        self.assertEqual(smtd_dyn.get_layer_state(), 0)

    def test_smtd_lt_fast_releases_is_hold_tap(self):
        """SMTD_LT hold: following key is emulated on the pushed layer"""
        LT_DYN.press()
        smtd_dyn.wait(60)
        B_DYN.press()
        smtd_dyn.wait(60)
        LT_DYN.release()
        smtd_dyn.wait(5)  # window is min(60, 60) * 20/100 = 12ms, ↑B beats it
        B_DYN.release()

        self.assertHistory(
            pressed(B_DYN, layer=1),
            released(B_DYN, layer=1),
        )
        self.assertEqual(smtd_dyn.get_layer_state(), 0, "layer must be restored")

    def test_per_key_release_term_caps_dynamic_window(self):
        """K5 has a per-key SMTD_TIMEOUT_RELEASE of 5ms: it must cap the dynamic
        window (min(60, 60) * 20/100 = 12ms would otherwise make this a hold)"""
        MODPK_DYN.press()
        smtd_dyn.wait(60)
        B_DYN.press()
        smtd_dyn.wait(60)
        MODPK_DYN.release()
        smtd_dyn.wait(8)  # beyond the 5ms per-key cap, within the unclamped 12ms
        B_DYN.release()

        self.assertHistory(
            pressed(MODPK_DYN),
            released(MODPK_DYN),
            pressed(B_DYN),
            released(B_DYN),
        )

    def test_per_key_release_within_capped_window_is_hold_tap(self):
        MODPK_DYN.press()
        smtd_dyn.wait(60)
        B_DYN.press()
        smtd_dyn.wait(60)
        MODPK_DYN.release()
        smtd_dyn.wait(3)  # within the 5ms per-key cap
        B_DYN.release()

        self.assertHistory(
            pressed(B_DYN, mods=MOD_LSFT),
            released(B_DYN, mods=MOD_LSFT),
        )
        self.assertEqual(smtd_dyn.get_mods(), 0)

    def test_three_key_roll_is_tap_tap_tap(self):
        """Steady three-key roll: every state gets its own window from its
        immediate follower, presses and releases keep the typing order"""
        MOD_DYN.press()
        smtd_dyn.wait(40)
        B_DYN.press()
        smtd_dyn.wait(40)
        C_DYN.press()
        smtd_dyn.wait(40)
        MOD_DYN.release()  # window from B: min(40, 80) * 20/100 = 8ms
        smtd_dyn.wait(40)
        B_DYN.release()    # window from C: min(40, 80) * 20/100 = 8ms
        smtd_dyn.wait(40)
        C_DYN.release()

        self.assertHistory(
            pressed(MOD_DYN),
            released(MOD_DYN),
            pressed(B_DYN),
            pressed(C_DYN),
            released(B_DYN),
            released(C_DYN),
        )

    def test_three_key_fast_releases_is_hold(self):
        """All releases beat their windows: mod holds, B and C are interpreted
        as nested under it (B release is emitted after C, the 3-finger-roll rule)"""
        MOD_DYN.press()
        smtd_dyn.wait(40)
        B_DYN.press()
        smtd_dyn.wait(40)
        C_DYN.press()
        smtd_dyn.wait(40)
        MOD_DYN.release()
        smtd_dyn.wait(3)  # window from B is 8ms, ↑B beats it
        B_DYN.release()
        smtd_dyn.wait(3)  # window from C is 8ms, ↑C beats it
        C_DYN.release()

        self.assertHistory(
            pressed(B_DYN, mods=MOD_LSFT),
            pressed(C_DYN, mods=MOD_LSFT),
            released(C_DYN, mods=MOD_LSFT),
            released(B_DYN, mods=MOD_LSFT),
        )
        self.assertEqual(smtd_dyn.get_mods(), 0)

    def test_multi_tap_p1_measured_from_last_touch(self):
        """In a tap sequence p1 must come from the latest press of the macro key
        (30ms), not from the first one (90ms): the 8ms release gap then falls
        outside the min(30, 60) * 20/100 = 6ms window and resolves as a roll"""
        MOD_DYN.press()
        smtd_dyn.wait(20)
        MOD_DYN.release()  # first tap
        smtd_dyn.wait(40)  # within SEQUENCE_TERM (100ms)
        MOD_DYN.press()    # tap_count = 1
        smtd_dyn.wait(30)
        B_DYN.press()
        smtd_dyn.wait(60)
        MOD_DYN.release()
        smtd_dyn.wait(8)   # outside 6ms; a stale p1 of 90ms would give 12ms = hold
        B_DYN.release()

        self.assertHistory(
            pressed(MOD_DYN),
            released(MOD_DYN),
            pressed(MOD_DYN),
            released(MOD_DYN),
            pressed(B_DYN),
            released(B_DYN),
        )

    def test_sequence_expiry_resets_tap_count_before_hold(self):
        """Sequence timeout fired by the clock resets tap_count, so the next
        long press is a fresh hold (mods), not a tap_count>=1 key repeat"""
        MOD_DYN.press()
        smtd_dyn.wait(30)
        MOD_DYN.release()
        smtd_dyn.wait(120)  # beyond SEQUENCE_TERM (100ms), sequence expires

        MOD_DYN.press()
        smtd_dyn.wait(200)  # TAPPING_TERM
        self.assertEqual(smtd_dyn.get_mods(), MOD_LSFT, "must be a fresh hold")
        MOD_DYN.release()
        self.assertEqual(smtd_dyn.get_mods(), 0)

        self.assertHistory(
            pressed(MOD_DYN),
            released(MOD_DYN),
        )


class TestFixedReleaseTermFallback(SmTdAssertions):
    """SMTD_GLOBAL_RELEASE_PERCENT = 0: the fixed SMTD_TIMEOUT_RELEASE (50ms) is used"""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.smtd = smtd_fixed

    def setUp(self):
        super().setUp()
        reset(smtd_fixed, KEYCODES_FIXED, KEYS_FIXED)

    def test_release_within_fixed_window_is_hold_tap(self):
        """Same timings resolve as tap-tap with the dynamic window (see the dynamic
        suite test_steady_roll_is_tap_tap with p3=50), but as hold here"""
        MOD_FIXED.press()
        smtd_fixed.wait(50)
        B_FIXED.press()
        smtd_fixed.wait(50)
        MOD_FIXED.release()
        smtd_fixed.wait(30)  # within the fixed 50ms window
        B_FIXED.release()

        self.assertHistory(
            pressed(B_FIXED, mods=MOD_LSFT),
            released(B_FIXED, mods=MOD_LSFT),
        )
        self.assertEqual(smtd_fixed.get_mods(), 0)

    def test_release_after_fixed_window_is_tap_tap(self):
        MOD_FIXED.press()
        smtd_fixed.wait(50)
        B_FIXED.press()
        smtd_fixed.wait(50)
        MOD_FIXED.release()
        smtd_fixed.wait(60)  # beyond the fixed 50ms window
        B_FIXED.release()

        self.assertHistory(
            pressed(MOD_FIXED),
            released(MOD_FIXED),
            pressed(B_FIXED),
            released(B_FIXED),
        )


class TestReleaseTermUpperClamp(SmTdAssertions):
    """SMTD_GLOBAL_RELEASE_PERCENT = 100: min(p1, p2) may exceed the fixed
    SMTD_TIMEOUT_RELEASE (50ms), which must stay the upper bound"""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.smtd = smtd_clamp

    def setUp(self):
        super().setUp()
        reset(smtd_clamp, KEYCODES_CLAMP, KEYS_CLAMP)

    def test_window_never_exceeds_fixed_term(self):
        """min(80, 80) * 100/100 = 80ms, but the window is clamped to 50ms"""
        MOD_CLAMP.press()
        smtd_clamp.wait(80)
        B_CLAMP.press()
        smtd_clamp.wait(80)
        MOD_CLAMP.release()
        smtd_clamp.wait(60)  # beyond the 50ms clamp, within the unclamped 80ms
        B_CLAMP.release()

        self.assertHistory(
            pressed(MOD_CLAMP),
            released(MOD_CLAMP),
            pressed(B_CLAMP),
            released(B_CLAMP),
        )

    def test_release_within_clamped_window_is_hold_tap(self):
        MOD_CLAMP.press()
        smtd_clamp.wait(80)
        B_CLAMP.press()
        smtd_clamp.wait(80)
        MOD_CLAMP.release()
        smtd_clamp.wait(40)  # within the 50ms clamp
        B_CLAMP.release()

        self.assertHistory(
            pressed(B_CLAMP, mods=MOD_LSFT),
            released(B_CLAMP, mods=MOD_LSFT),
        )
        self.assertEqual(smtd_clamp.get_mods(), 0)


class TestQmkTapHoldDynamicRelease(SmTdAssertions):
    """SMTD_ENABLE_QMK_TAPHOLD: the dynamic window (SMTD_GLOBAL_RELEASE_PERCENT 20) applies to
    raw QMK MT()/LT() keycodes resolved by smtd_handle_qk_tap_hold"""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.smtd = smtd_qk

    def setUp(self):
        super().setUp()
        reset(smtd_qk, KEYCODES_QK, KEYS_QK)

    def test_mt_steady_roll_is_tap_tap(self):
        """MT() in a roll sends its tap keycode, no mods on the following key"""
        MT_QK.press()
        smtd_qk.wait(50)
        B_QK.press()
        smtd_qk.wait(50)
        MT_QK.release()
        smtd_qk.wait(50)  # window is min(50, 50) * 20/100 = 10ms, expires mid-wait
        B_QK.release()

        self.assertHistory(
            registered(TAP_MT),
            unregistered(TAP_MT),
            pressed(B_QK),
            released(B_QK),
        )
        self.assertEqual(smtd_qk.get_mods(), 0)

    def test_mt_fast_releases_is_hold_tap(self):
        """MT() with near-simultaneous releases applies the mod to the following key"""
        MT_QK.press()
        smtd_qk.wait(60)
        B_QK.press()
        smtd_qk.wait(60)
        MT_QK.release()
        smtd_qk.wait(5)  # window is min(60, 60) * 20/100 = 12ms, ↑B beats it
        B_QK.release()

        self.assertHistory(
            pressed(B_QK, mods=MOD_LCTL_8BIT),
            released(B_QK, mods=MOD_LCTL_8BIT),
        )
        self.assertEqual(smtd_qk.get_mods(), 0)

    def test_lt_steady_roll_is_tap_tap(self):
        """LT() in a roll sends its tap keycode, the following key stays on layer 0"""
        LT_QK.press()
        smtd_qk.wait(50)
        B_QK.press()
        smtd_qk.wait(50)
        LT_QK.release()
        smtd_qk.wait(50)  # window is min(50, 50) * 20/100 = 10ms, expires mid-wait
        B_QK.release()

        self.assertHistory(
            registered(TAP_LT),
            unregistered(TAP_LT),
            pressed(B_QK),
            released(B_QK),
        )
        self.assertEqual(smtd_qk.get_layer_state(), 0)

    def test_lt_fast_releases_is_hold_tap(self):
        """LT() with near-simultaneous releases emulates the following key on the pushed layer"""
        LT_QK.press()
        smtd_qk.wait(60)
        B_QK.press()
        smtd_qk.wait(60)
        LT_QK.release()
        smtd_qk.wait(5)  # window is min(60, 60) * 20/100 = 12ms, ↑B beats it
        B_QK.release()

        self.assertHistory(
            pressed(B_QK, layer=1),
            released(B_QK, layer=1),
        )
        self.assertEqual(smtd_qk.get_layer_state(), 0, "layer must be restored")


class TestFractionalReleasePercent(SmTdAssertions):
    """SMTD_GLOBAL_RELEASE_PERCENT = 40: the ↑MOD..↑B window is min(p1, p2) * 40 / 100.
    With p1 = p2 = 50 the window is 20ms — a fractional width the coarse integer divisor
    form cannot produce (its neighboring steps give 25ms and 16ms), so these tests pin
    the window strictly between those two neighbors."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.smtd = smtd_pct

    def setUp(self):
        super().setUp()
        reset(smtd_pct, KEYCODES_PCT, KEYS_PCT)

    def test_within_window_is_hold_tap(self):
        """release gap 15ms < 20ms window -> hold"""
        MOD_PCT.press()
        smtd_pct.wait(50)
        B_PCT.press()
        smtd_pct.wait(50)
        MOD_PCT.release()
        smtd_pct.wait(15)  # window is min(50, 50) * 40 / 100 = 20ms, ↑B beats it
        B_PCT.release()

        self.assertHistory(
            pressed(B_PCT, mods=MOD_LSFT),
            released(B_PCT, mods=MOD_LSFT),
        )
        self.assertEqual(smtd_pct.get_mods(), 0)

    def test_after_window_is_tap_tap(self):
        """release gap 30ms > 20ms window -> roll"""
        MOD_PCT.press()
        smtd_pct.wait(50)
        B_PCT.press()
        smtd_pct.wait(50)
        MOD_PCT.release()
        smtd_pct.wait(30)  # window is 20ms, expires mid-wait
        B_PCT.release()

        self.assertHistory(
            pressed(MOD_PCT),
            released(MOD_PCT),
            pressed(B_PCT),
            released(B_PCT),
        )

    def test_window_wider_than_16ms_step(self):
        """gap 18ms: hold here (20ms window) but would be a roll at a 16ms window
        — proves the window is finer than a coarse integer-divisor step"""
        MOD_PCT.press()
        smtd_pct.wait(50)
        B_PCT.press()
        smtd_pct.wait(50)
        MOD_PCT.release()
        smtd_pct.wait(18)  # 16ms step < 18 < 20ms (percent 40)
        B_PCT.release()

        self.assertHistory(
            pressed(B_PCT, mods=MOD_LSFT),
            released(B_PCT, mods=MOD_LSFT),
        )
        self.assertEqual(smtd_pct.get_mods(), 0)

    def test_window_narrower_than_25ms_step(self):
        """gap 22ms: roll here (20ms window) but would be a hold at a 25ms window
        — pins the window strictly below the next coarser step"""
        MOD_PCT.press()
        smtd_pct.wait(50)
        B_PCT.press()
        smtd_pct.wait(50)
        MOD_PCT.release()
        smtd_pct.wait(22)  # 20ms (percent 40) < 22 < 25ms step
        B_PCT.release()

        self.assertHistory(
            pressed(MOD_PCT),
            released(MOD_PCT),
            pressed(B_PCT),
            released(B_PCT),
        )


KEYCODES_DYN, KEYS_DYN = build_keys(smtd_dyn)
MOD_DYN = KEYS_DYN[1]
B_DYN = KEYS_DYN[2]
LT_DYN = KEYS_DYN[3]
C_DYN = KEYS_DYN[4]
MODPK_DYN = KEYS_DYN[5]  # SMTD_MT with per-key SMTD_TIMEOUT_RELEASE = 5ms

KEYCODES_FIXED, KEYS_FIXED = build_keys(smtd_fixed)
MOD_FIXED = KEYS_FIXED[1]
B_FIXED = KEYS_FIXED[2]

KEYCODES_CLAMP, KEYS_CLAMP = build_keys(smtd_clamp)
MOD_CLAMP = KEYS_CLAMP[1]
B_CLAMP = KEYS_CLAMP[2]

KEYCODES_PCT, KEYS_PCT = build_keys(smtd_pct)
MOD_PCT = KEYS_PCT[1]
B_PCT = KEYS_PCT[2]

# Raw QMK tap-hold keycodes (must match layout_qk.c packing)
MOD_LCTL = 0x01       # 5-bit mask in the MT() keycode
MOD_LCTL_8BIT = 0x01  # MOD_BIT(KC_LCTL)
KEYCODES_QK, KEYS_QK = build_keys(smtd_qk, l0_values={1: qmk_mt(MOD_LCTL, 104), 2: qmk_lt(1, 105)})
MT_QK = KEYS_QK[1]
LT_QK = KEYS_QK[2]
B_QK = KEYS_QK[3]

# Tap keycodes extracted from MT()/LT() (only .value is used in assertions)
TAP_MT = Keycode(smtd_qk, 104, 255, 255, -100)
TAP_LT = Keycode(smtd_qk, 105, 255, 255, -100)


if __name__ == "__main__":
    unittest.main()
