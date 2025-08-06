import itertools
import random

try:
    from tests.sm_td_assertions import *
except ImportError:
    from sm_td_assertions import *

smtd = load_smtd_lib('tests/global_mods_propagation_disabled/layout.c')


class TestSmTdWithGlobalModsPropagationEnabled(SmTdAssertions):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.smtd = smtd

    def setUp(self):
        """Method to run before each test"""
        super().setUp()
        reset()

    def test_process_smtd(self):
        """Test that process_smtd function from the actual library works"""
        self.assertFalse(K2.press(), "process_smtd should block future key events")

        self.assertHistory(
            pressed(K2)
        )

    def test_bypass_mode(self):
        """Test the actual bypass mode in the library"""
        smtd.set_bypass(True)
        self.assertTrue(K2.press(), "sm_td should return true in bypass mode")
        self.assertEqual(len(smtd.get_record_history()), 0)

    def test_reset(self):
        """Test the reset function"""
        K2.press()
        records_before = smtd.get_record_history()

        reset()
        records_after = smtd.get_record_history()

        self.assertGreater(len(records_before), 0, "No records were created")
        self.assertEqual(len(records_after), 0, "Records were not cleared after reset")

    def test_generic_tap(self):
        """Test the generic tap function"""
        self.assertFalse(K2.press(), "press should block future key events")
        K2.prolong()
        self.assertFalse(K2.release(), "release should block future key events")

        self.assertHistory(
            pressed(K2),
            released(K2)
        )

    def test_MT_CTRL_tap(self):
        """Test the basic MT function"""
        self.assertFalse(MT_CTRL.press(), "press should block future key events")
        self.assertFalse(MT_CTRL.release(), "release should return true")

        self.assertHistory(
            registered(KC1_MT_CTRL),
            unregistered(KC1_MT_CTRL)
        )

    def test_MT_CTRL_hold(self):
        """Test the basic MT function"""
        self.assertFalse(MT_CTRL.press(), "press should block future key events")
        self.assertEqual(len(smtd.get_record_history()), 0)
        self.assertEqual(smtd.get_mods(), 0)

        MT_CTRL.prolong()
        self.assertEqual(len(smtd.get_record_history()), 0)
        self.assertEqual(smtd.get_mods(), 1)

        self.assertFalse(MT_CTRL.release(), "release should return true")
        self.assertEqual(len(smtd.get_record_history()), 0)
        self.assertEqual(smtd.get_mods(), 0)

    def test_shift_key(self):
        """Test the basic MT function"""
        self.assertFalse(SHIFT.press())
        self.assertEqual(smtd.get_mods(), 2)

        SHIFT.prolong()
        self.assertEqual(smtd.get_mods(), 2)

        SHIFT.release()
        self.assertEqual(smtd.get_mods(), 0)

        self.assertHistory(
            pressed(KC_SHIFT, mods=0),
            released(KC_SHIFT, mods=2)
        )

    def test_SKKS(self):
        self.assertFalse(SHIFT.press())
        self.assertFalse(KC2.press())
        self.assertFalse(KC2.release())
        self.assertFalse(SHIFT.release())

        self.assertHistory(
            pressed(KC_SHIFT, mods=0),
            pressed(K2, mods=2),
            released(K2, mods=2),
            released(KC_SHIFT, mods=2)
        )

    def test_CKCK(self):
        self.assertFalse(KC1_MT_CTRL.press())
        self.assertFalse(KC2.press())
        self.assertFalse(KC1_MT_CTRL.release())
        self.assertFalse(KC2.release())

        self.assertHistory(
            pressed(K2, mods=1),
            released(K2, mods=1),
        )

    def test_SKSK(self):
        self.assertFalse(SHIFT.press())
        self.assertFalse(KC2.press())
        self.assertFalse(SHIFT.release())
        self.assertFalse(KC2.release())

        self.assertHistory(
            pressed(KC_SHIFT, mods=0),
            pressed(K2, mods=2),
            released(K2, mods=2),
            released(KC_SHIFT, mods=2)
        )

    def test_SSKK(self):
        self.assertFalse(SHIFT.press())
        self.assertFalse(SHIFT.release())
        self.assertFalse(KC2.press())
        self.assertFalse(KC2.release())

        self.assertHistory(
            pressed(KC_SHIFT, mods=0),
            released(KC_SHIFT, mods=2),
            pressed(K2, mods=0),
            released(K2, mods=0),
        )

    def test_KSKS(self):
        self.assertFalse(KC2.press())
        self.assertFalse(SHIFT.press())
        self.assertFalse(KC2.release())
        self.assertFalse(SHIFT.release())

        self.assertHistory(
            pressed(K2, mods=0),
            pressed(KC_SHIFT, mods=0),
            released(KC_SHIFT, mods=2),
            released(K2, mods=0)
        )

    def test_SCKKCS(self):
        self.assertFalse(SHIFT.press())
        self.assertFalse(MT_CTRL.press())
        self.assertFalse(K2.press())
        self.assertFalse(K2.release())
        self.assertFalse(MT_CTRL.release())
        self.assertFalse(SHIFT.release())

        self.assertHistory(
            pressed(KC_SHIFT, mods=0),
            pressed(K2, mods=3),
            released(K2, mods=3),
            released(KC_SHIFT, mods=2)
        )

    def test_SCK_permutations(self):
        releases = [SHIFT, MT_CTRL, K2]
        for press_seq in itertools.permutations(releases):
            reset()

            self.assertFalse(SHIFT.press())
            self.assertFalse(MT_CTRL.press())
            self.assertFalse(K2.press())

            for key in press_seq: key.release()

            self.assertHistory(
                pressed(KC_SHIFT, mods=0),
                pressed(K2, mods=3),
                released(K2, mods=3),
                released(KC_SHIFT, mods=2)
            )

    def test_CSK_permutations(self):
        releases = [SHIFT, MT_CTRL, K2]
        for press_seq in itertools.permutations(releases):
            reset()

            self.assertFalse(MT_CTRL.press())
            self.assertFalse(SHIFT.press())
            self.assertFalse(K2.press())

            for key in press_seq: key.release()

            self.assertHistory(
                pressed(KC_SHIFT, mods=1),
                pressed(K2, mods=3),
                released(K2, mods=3),
                released(KC_SHIFT, mods=3)
            )

    def test_CKSKCKSK(self):
        self.assertFalse(MT_CTRL.press())
        self.assertFalse(K2.press())
        self.assertFalse(SHIFT.press())
        self.assertFalse(K2.release())
        self.assertFalse(MT_CTRL.release())
        self.assertFalse(K3.press())
        self.assertFalse(SHIFT.release())
        self.assertFalse(K3.release())

        # with global mod propagation this should happen:
        # self.assertHistory(
        #     pressed(K2, mods=1),
        #     pressed(KC_SHIFT, mods=1),
        #     released(K2, mods=0),
        #     pressed(K3, mods=2),
        #     released(K3, mods=2),
        #     released(KC_SHIFT, mods=2),
        # )

        # but without it we have:
        self.assertHistory(
            pressed(K2, mods=1),
            pressed(KC_SHIFT, mods=1),
            released(K2, mods=2),
            pressed(K3, mods=2),
            released(K3, mods=2),
            released(KC_SHIFT, mods=2),
        )

    def test_CS2C23S2(self):
        self.assertFalse(MT_CTRL.press())
        self.assertFalse(SHIFT.press())
        self.assertFalse(K2.press())
        self.assertFalse(MT_CTRL.release())
        self.assertFalse(K2.release())
        self.assertFalse(K3.press())
        self.assertFalse(SHIFT.release())
        self.assertFalse(K3.release())

        self.assertHistory(
            pressed(SHIFT, mods=1),
            pressed(K2, mods=3),
            released(K2, mods=3),
            pressed(K3, mods=2),
            released(K3, mods=2),
            released(SHIFT, mods=2),
        )

# Layers

L0 = 0

# Keycodes

KC_SHIFT = Keycode(smtd, 100, 0, 0, L0)
KC1_MT_CTRL = Keycode(smtd, 101, 0, 1, L0)
KC2 = Keycode(smtd, 102, 0, 2, L0)
KC3 = Keycode(smtd, 103, 0, 3, L0)

all_keycodes = [KC_SHIFT, KC1_MT_CTRL, KC2, KC3, ]

# Keys

SHIFT = Key(smtd, 'SHIFT', 0, 0, "non-smtd-shift", all_keycodes)
MT_CTRL = Key(smtd, 'MT_CTRL', 0, 1, "smtd-ctrl", all_keycodes)
K2 = Key(smtd, 'K2', 0, 2, "K2", all_keycodes)
K3 = Key(smtd, 'K3', 0, 3, "K3", all_keycodes)

all_keys = [SHIFT, MT_CTRL, K2, K3]


def reset():
    for keycode in all_keycodes:
        keycode.reset()
    for key in all_keys:
        key.reset()
    smtd.reset()


if __name__ == "__main__":
    unittest.main()
