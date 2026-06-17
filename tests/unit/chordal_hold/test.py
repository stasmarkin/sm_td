try:
    from tests.unit.sm_td_assertions import *
except ImportError:
    from sm_td_assertions import *

smtd = load_smtd_lib('tests/unit/chordal_hold/layout.c')


class TestSmTdChordalHold(SmTdAssertions):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.smtd = smtd

    def setUp(self):
        super().setUp()
        reset()

    # ---- Regression: basic tap/hold still work with chordal compiled in ----

    def test_mt_tap_alone(self):
        """A lone mod-tap tap still sends the tap keycode"""
        MT_LSFT.press()
        MT_LSFT.release()
        self.assertHistory(
            Register(TAP_A),
            Unregister(TAP_A),
        )
        self.assertEqual(smtd.get_mods(), 0)

    def test_mt_hold_alone_via_timeout(self):
        """A lone mod-tap with no following key still holds on timeout"""
        MT_LSFT.press()
        self.assertEqual(smtd.get_mods(), 0)

        MT_LSFT.prolong()
        self.assertEqual(smtd.get_mods(), MOD_LSFT, "lone hold must still register the mod")

        MT_LSFT.release()
        self.assertEqual(smtd.get_mods(), 0)
        self.assertHistory()

    # ---- Cross-hand: opposite hands settle as HOLD ----

    def test_cross_hand_mt_plain_holds(self):
        """Mod-tap + opposite-hand key resolves as HOLD (mod applied)"""
        MT_LSFT.press()
        R_PLAIN.press()
        R_PLAIN.release()
        MT_LSFT.release()

        self.assertHistory(
            EmulatePress(R_PLAIN, mods=MOD_LSFT),
            EmulateRelease(R_PLAIN, mods=MOD_LSFT),
        )
        self.assertEqual(smtd.get_mods(), 0)

    def test_cross_hand_timeout_not_cancelled(self):
        """Opposite-hand following press leaves the hold timeout armed"""
        MT_LSFT.press()
        R_PLAIN.press()

        # timeout still pending -> firing it must hold the mod
        MT_LSFT.prolong()
        self.assertEqual(smtd.get_mods(), MOD_LSFT, "cross-hand timeout must still fire a HOLD")

        R_PLAIN.release()
        MT_LSFT.release()
        self.assertEqual(smtd.get_mods(), 0)

    def test_two_left_modtaps_plus_cross_hand_key(self):
        """Two same-hand mod-taps + opposite-hand key: both mods HOLD, key tapped with both"""
        MT_LSFT.press()
        MT_LGUI.press()
        R_PLAIN.press()
        R_PLAIN.release()
        MT_LGUI.release()
        MT_LSFT.release()

        self.assertHistory(
            EmulatePress(R_PLAIN, mods=MOD_LSFT | MOD_LGUI),
            EmulateRelease(R_PLAIN, mods=MOD_LSFT | MOD_LGUI),
        )
        self.assertEqual(smtd.get_mods(), 0)

    # ---- Same-hand: same hand settles as TAP ----

    def test_same_hand_mt_plain_rolls_to_taps(self):
        """Mod-tap + same-hand key resolves both as TAP (no accidental mod)"""
        MT_LSFT.press()
        L_PLAIN.press()
        L_PLAIN.release()
        MT_LSFT.release()

        self.assertHistory(
            Register(TAP_A),
            Unregister(TAP_A),
            EmulatePress(L_PLAIN, mods=0),
            EmulateRelease(L_PLAIN, mods=0),
        )
        self.assertEqual(smtd.get_mods(), 0)

    def test_same_hand_press_cancels_timeout(self):
        """Same-hand following press cancels the hold timeout (no accidental HOLD)"""
        MT_LSFT.press()
        L_PLAIN.press()

        # timeout must be cancelled -> firing it (try) must NOT register a mod
        MT_LSFT.try_prolong()
        self.assertEqual(smtd.get_mods(), 0, "same-hand roll must not arm a HOLD")

        L_PLAIN.release()
        MT_LSFT.release()
        self.assertEqual(smtd.get_mods(), 0)

    def test_two_left_modtaps_only_tap(self):
        """Two same-hand mod-taps with no cross-hand key resolve all as TAP"""
        MT_LSFT.press()
        MT_LGUI.press()
        MT_LGUI.release()
        MT_LSFT.release()

        self.assertHistory(
            Register(TAP_A),
            Unregister(TAP_A),
            Register(TAP_B),
            Unregister(TAP_B),
        )
        self.assertEqual(smtd.get_mods(), 0)

    # ---- Layer-tap behaves like mod-tap under the chordal rule ----

    def test_lt_cross_hand_holds(self):
        """Layer-tap + opposite-hand key activates the layer (HOLD)"""
        LT_L1.press()
        R_PLAIN.press()
        self.assertEqual(smtd.get_layer_state(), 0)

        LT_L1.prolong()
        self.assertEqual(smtd.get_layer_state(), 1, "cross-hand layer-tap must push the layer")

        R_PLAIN.release()
        LT_L1.release()
        self.assertEqual(smtd.get_layer_state(), 0)

    def test_lt_same_hand_rolls_to_taps(self):
        """Layer-tap + same-hand key resolves both as TAP (no layer change)"""
        LT_L1.press()
        L_PLAIN.press()
        L_PLAIN.release()
        LT_L1.release()

        self.assertEqual(smtd.get_layer_state(), 0)
        self.assertHistory(
            Register(TAP_C),
            Unregister(TAP_C),
            EmulatePress(L_PLAIN, mods=0),
            EmulateRelease(L_PLAIN, mods=0),
        )

    # ---- Thumbs are neutral: they keep the default hold-capable behavior ----

    def test_thumb_modtap_same_side_holds(self):
        """Thumb (neutral) mod-tap holds even with a same-side key following"""
        MT_THUMB.press()
        L_PLAIN.press()
        L_PLAIN.release()
        MT_THUMB.release()

        self.assertHistory(
            EmulatePress(L_PLAIN, mods=MOD_LSFT),
            EmulateRelease(L_PLAIN, mods=MOD_LSFT),
        )
        self.assertEqual(smtd.get_mods(), 0)

    def test_thumb_modtap_cross_hand_holds(self):
        """Thumb (neutral) mod-tap holds with an opposite-hand key too"""
        MT_THUMB.press()
        R_PLAIN.press()
        R_PLAIN.release()
        MT_THUMB.release()

        self.assertHistory(
            EmulatePress(R_PLAIN, mods=MOD_LSFT),
            EmulateRelease(R_PLAIN, mods=MOD_LSFT),
        )
        self.assertEqual(smtd.get_mods(), 0)

    # ---- Neutral following key: a '*' key signals an intentional chord -> HOLD ----

    def test_mt_plus_neutral_following_holds(self):
        """Mod-tap + following neutral ('*') key resolves as HOLD (mod applied)"""
        MT_LSFT.press()
        T_PLAIN.press()
        T_PLAIN.release()
        MT_LSFT.release()

        self.assertHistory(
            EmulatePress(T_PLAIN, mods=MOD_LSFT),
            EmulateRelease(T_PLAIN, mods=MOD_LSFT),
        )
        self.assertEqual(smtd.get_mods(), 0)

    def test_mt_plus_neutral_timeout_holds(self):
        """Neutral following press leaves the hold timeout armed (consistency check)"""
        MT_LSFT.press()
        T_PLAIN.press()

        # timeout still pending -> firing it must hold the mod
        MT_LSFT.prolong()
        self.assertEqual(smtd.get_mods(), MOD_LSFT, "neutral-key timeout must still fire a HOLD")

        T_PLAIN.release()
        MT_LSFT.release()
        self.assertEqual(smtd.get_mods(), 0)


# Layers

L0 = 0
L1 = 1

# Raw QMK tap-hold keycodes (must match layout.c packing)

def qmk_mt(mod, kc):
    return 0x2000 | ((mod & 0x1F) << 8) | (kc & 0xFF)

def qmk_lt(layer, kc):
    return 0x4000 | ((layer & 0xF) << 8) | (kc & 0xFF)

MOD_LSFT = 0x02
MOD_LGUI = 0x08
MOD_RSFT = 0x20  # right-hand mod unpacked to the 8-bit representation

# Keycodes

KC_MT_LSFT = Keycode(smtd, qmk_mt(0x02, 104), 0, 0, L0)
KC_MT_LGUI = Keycode(smtd, qmk_mt(0x08, 105), 0, 1, L0)
KC_LT_L1 = Keycode(smtd, qmk_lt(L1, 106), 0, 2, L0)
KC_L_PLAIN = Keycode(smtd, 110, 0, 3, L0)

KC_MT_RSFT = Keycode(smtd, qmk_mt(0x12, 107), 1, 0, L0)
KC_R_PLAIN = Keycode(smtd, 111, 1, 1, L0)
KC_R_PLAIN2 = Keycode(smtd, 111, 1, 2, L0)

KC_MT_THUMB = Keycode(smtd, qmk_mt(0x02, 108), 2, 0, L0)
KC_T_PLAIN = Keycode(smtd, 112, 2, 1, L0)

# Layer-1 keycodes for positions pressed while a layer-tap is held
KC_R_PLAIN_L1 = Keycode(smtd, 221, 1, 1, L1)
KC_L_PLAIN_L1 = Keycode(smtd, 213, 0, 3, L1)

# Tap keycodes extracted from MT()/LT() (only .value is used in assertions)
TAP_A = Keycode(smtd, 104, 255, 255, -100)
TAP_B = Keycode(smtd, 105, 255, 255, -100)
TAP_C = Keycode(smtd, 106, 255, 255, -100)
TAP_D = Keycode(smtd, 107, 255, 255, -100)
TAP_T = Keycode(smtd, 108, 255, 255, -100)

all_keycodes = [
    KC_MT_LSFT, KC_MT_LGUI, KC_LT_L1, KC_L_PLAIN,
    KC_MT_RSFT, KC_R_PLAIN, KC_R_PLAIN2,
    KC_MT_THUMB, KC_T_PLAIN,
    KC_R_PLAIN_L1, KC_L_PLAIN_L1,
]

# Keys

MT_LSFT = Key(smtd, 'MT_LSFT', 0, 0, "MT(MOD_LSFT, TAP_A) left", all_keycodes)
MT_LGUI = Key(smtd, 'MT_LGUI', 0, 1, "MT(MOD_LGUI, TAP_B) left", all_keycodes)
LT_L1 = Key(smtd, 'LT_L1', 0, 2, "LT(L1, TAP_C) left", all_keycodes)
L_PLAIN = Key(smtd, 'L_PLAIN', 0, 3, "plain left", all_keycodes)

MT_RSFT = Key(smtd, 'MT_RSFT', 1, 0, "MT(MOD_RSFT, TAP_D) right", all_keycodes)
R_PLAIN = Key(smtd, 'R_PLAIN', 1, 1, "plain right", all_keycodes)
R_PLAIN2 = Key(smtd, 'R_PLAIN2', 1, 2, "plain right", all_keycodes)

MT_THUMB = Key(smtd, 'MT_THUMB', 2, 0, "MT(MOD_LSFT, TAP_T) thumb", all_keycodes)
T_PLAIN = Key(smtd, 'T_PLAIN', 2, 1, "plain thumb", all_keycodes)

all_keys = [
    MT_LSFT, MT_LGUI, LT_L1, L_PLAIN,
    MT_RSFT, R_PLAIN, R_PLAIN2,
    MT_THUMB, T_PLAIN,
]


def reset():
    for keycode in all_keycodes:
        keycode.reset()
    for key in all_keys:
        key.reset()
    smtd.reset()


if __name__ == "__main__":
    unittest.main()
