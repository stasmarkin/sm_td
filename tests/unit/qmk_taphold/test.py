try:
    from tests.unit.sm_td_assertions import *
except ImportError:
    from sm_td_assertions import *

smtd = load_smtd_lib('tests/unit/qmk_taphold/layout.c')


class TestSmTdQmkTapHold(SmTdAssertions):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.smtd = smtd

    def setUp(self):
        """Method to run before each test"""
        super().setUp()
        reset()

    def test_mt_tap(self):
        """Tap on a raw MT() keycode sends the tap keycode"""
        self.assertFalse(MT_CTRL.press(), "press should block future key events")
        self.assertEqual(len(smtd.get_record_history()), 0, "tap should happen after release")

        self.assertFalse(MT_CTRL.release())
        self.assertHistory(
            Register(TAP_KC1),
            Unregister(TAP_KC1),
        )

    def test_mt_tap_sequence(self):
        """Every tap in a sequence on a raw MT() keycode sends the tap keycode"""
        MT_CTRL.press()
        MT_CTRL.release()
        MT_CTRL.press()
        MT_CTRL.release()

        self.assertHistory(
            Register(TAP_KC1),
            Unregister(TAP_KC1),
            Register(TAP_KC1),
            Unregister(TAP_KC1),
        )

    def test_mt_hold(self):
        """Hold on a raw MT() keycode registers the mod, no key output"""
        self.assertFalse(MT_CTRL.press())
        self.assertEqual(smtd.get_mods(), 0)

        MT_CTRL.prolong()
        self.assertEqual(smtd.get_mods(), 1, "MOD_LCTL must be registered on hold")
        self.assertHistory()

        MT_CTRL.release()
        self.assertEqual(smtd.get_mods(), 0, "mod must be released")
        self.assertHistory()

    def test_mt_right_mod_hold(self):
        """5-bit right-hand mods are unpacked to the 8-bit representation"""
        self.assertFalse(MT_RCTRL.press())

        MT_RCTRL.prolong()
        self.assertEqual(smtd.get_mods(), 0x10, "MOD_RCTL must occupy the right-hand bit")

        MT_RCTRL.release()
        self.assertEqual(smtd.get_mods(), 0)
        self.assertHistory()

    def test_mt_hold_with_following_key(self):
        """MT() hold resolved by a following key press applies the mod to that key"""
        MT_CTRL.press()
        K4.press()
        K4.release()
        MT_CTRL.release()

        self.assertHistory(
            EmulatePress(K4, mods=1),
            EmulateRelease(K4, mods=1),
        )
        self.assertEqual(smtd.get_mods(), 0)

    def test_lt_tap(self):
        """Tap on a raw LT() keycode sends the tap keycode"""
        self.assertFalse(LT_L1.press())
        self.assertFalse(LT_L1.release())

        self.assertHistory(
            Register(TAP_KC2),
            Unregister(TAP_KC2),
        )
        self.assertEqual(smtd.get_layer_state(), 0)

    def test_lt_hold(self):
        """Hold on a raw LT() keycode moves to the layer and restores it on release"""
        self.assertFalse(LT_L1.press())
        self.assertEqual(smtd.get_layer_state(), 0)

        LT_L1.prolong()
        self.assertEqual(smtd.get_layer_state(), 1, "layer must be pushed on hold")

        K4.press()
        K4.release()

        LT_L1.release()
        self.assertEqual(smtd.get_layer_state(), 0, "layer must be restored on release")

        self.assertHistory(
            EmulatePress(K4, layer=1),
            EmulateRelease(K4, layer=1),
        )

    def test_plain_key_passthrough(self):
        """Non-tap-hold keycodes are still emulated as-is"""
        K0.press()
        K0.release()

        self.assertHistory(
            EmulatePress(K0),
            EmulateRelease(K0),
        )


# Layers

L0 = 0
L1 = 1

# Raw QMK tap-hold keycodes (must match layout.c packing)

def qmk_mt(mod, kc):
    return 0x2000 | ((mod & 0x1F) << 8) | (kc & 0xFF)

def qmk_lt(layer, kc):
    return 0x4000 | ((layer & 0xF) << 8) | (kc & 0xFF)

MOD_LCTL = 0x01
MOD_RCTL = 0x11

# Keycodes

L0_KC0 = Keycode(smtd, 100, 0, 0, L0)
KC_MT_CTRL = Keycode(smtd, qmk_mt(MOD_LCTL, 104), 0, 1, L0)
KC_LT_L1 = Keycode(smtd, qmk_lt(L1, 105), 0, 2, L0)
KC_MT_RCTRL = Keycode(smtd, qmk_mt(MOD_RCTL, 106), 0, 3, L0)
L0_KC4 = Keycode(smtd, 104, 0, 4, L0)

L1_KC0 = Keycode(smtd, 200, 0, 0, L1)
L1_KC1 = Keycode(smtd, 201, 0, 1, L1)
L1_KC2 = Keycode(smtd, 202, 0, 2, L1)
L1_KC3 = Keycode(smtd, 203, 0, 3, L1)
L1_KC4 = Keycode(smtd, 204, 0, 4, L1)

# Tap keycodes extracted from MT()/LT() (only .value is used in assertions)
TAP_KC1 = Keycode(smtd, 104, 255, 255, -100)
TAP_KC2 = Keycode(smtd, 105, 255, 255, -100)
TAP_KC3 = Keycode(smtd, 106, 255, 255, -100)

all_keycodes = [
    L0_KC0, KC_MT_CTRL, KC_LT_L1, KC_MT_RCTRL, L0_KC4,
    L1_KC0, L1_KC1, L1_KC2, L1_KC3, L1_KC4,
]

# Keys

K0 = Key(smtd, 'K0', 0, 0, "no special behavior", all_keycodes)
MT_CTRL = Key(smtd, 'MT_CTRL', 0, 1, "MT(MOD_LCTL, TAP_KC1)", all_keycodes)
LT_L1 = Key(smtd, 'LT_L1', 0, 2, "LT(L1, TAP_KC2)", all_keycodes)
MT_RCTRL = Key(smtd, 'MT_RCTRL', 0, 3, "MT(MOD_RCTL, TAP_KC3)", all_keycodes)
K4 = Key(smtd, 'K4', 0, 4, "no special behavior", all_keycodes)

all_keys = [K0, MT_CTRL, LT_L1, MT_RCTRL, K4]


def reset():
    for keycode in all_keycodes:
        keycode.reset()
    for key in all_keys:
        key.reset()
    smtd.reset()


if __name__ == "__main__":
    unittest.main()
