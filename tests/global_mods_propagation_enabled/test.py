import itertools
import random

try:
    from tests.sm_td_assertions import *
except ImportError:
    from sm_td_assertions import *

smtd = load_smtd_lib('tests/global_mods_propagation_enabled/layout.c')

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

        records = smtd.get_record_history()
        self.assertEqual(len(records), 1)
        self.assertEmulatePress(records[0], K2)

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
        records = smtd.get_record_history()
        self.assertEqual(len(records), 1)
        self.assertEmulatePress(records[0], K2)

        K2.prolong()
        self.assertEqual(len(records), 1)

        self.assertFalse(K2.release(), "release should block future key events")
        records = smtd.get_record_history()
        self.assertEqual(len(records), 2)
        self.assertEmulateRelease(records[1], K2)

    def test_MT_CTRL_tap(self):
        """Test the basic MT function"""
        self.assertFalse(MT_CTRL.press(), "press should block future key events")
        records = smtd.get_record_history()
        self.assertEqual(len(records), 0)

        self.assertFalse(MT_CTRL.release(), "release should return true")
        records = smtd.get_record_history()
        self.assertEqual(len(records), 2, "tap should happer after release")
        self.assertRegister(records[0], KC1_MT_CTRL)
        self.assertUnregister(records[1], KC1_MT_CTRL)

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
        records = smtd.get_record_history()
        self.assertEqual(len(records), 1)
        self.assertEqual(smtd.get_mods(), 2)

        SHIFT.prolong()
        records = smtd.get_record_history()
        self.assertEqual(len(records), 1)
        self.assertEqual(smtd.get_mods(), 2)

        SHIFT.release()
        records = smtd.get_record_history()
        self.assertEqual(len(records), 2)
        self.assertEqual(smtd.get_mods(), 0)
        self.assertEmulatePress(records[0], KC_SHIFT, mods = 0)
        self.assertEmulateRelease(records[1], KC_SHIFT, mods = 2)

    def test_SKKS(self):
        self.assertFalse(SHIFT.press())
        self.assertFalse(KC2.press())
        self.assertFalse(KC2.release())
        self.assertFalse(SHIFT.release())

        records = smtd.get_record_history()
        self.assertEqual(len(records), 4)
        self.assertEmulatePress(records[0], SHIFT, mods = 0)
        self.assertEmulatePress(records[1], K2, mods = 2)
        self.assertEmulateRelease(records[2], K2, mods = 2)
        self.assertEmulateRelease(records[3], SHIFT, mods = 2)

    def test_SKSK(self):
        self.assertFalse(SHIFT.press())
        self.assertFalse(KC2.press())
        self.assertFalse(SHIFT.release())
        self.assertFalse(KC2.release())

        records = smtd.get_record_history()
        self.assertEqual(len(records), 4)
        self.assertEmulatePress(records[0], SHIFT, mods = 0)
        self.assertEmulatePress(records[1], K2, mods = 2)
        self.assertEmulateRelease(records[2], SHIFT, mods = 2)
        self.assertEmulateRelease(records[3], K2, mods = 0)

    def test_KSKS(self):
        self.assertFalse(KC2.press())
        self.assertFalse(SHIFT.press())
        self.assertFalse(KC2.release())
        self.assertFalse(SHIFT.release())

        records = smtd.get_record_history()
        self.assertEqual(len(records), 4)
        self.assertEmulatePress(records[0], K2, mods = 0)
        self.assertEmulatePress(records[1], SHIFT, mods = 0)
        self.assertEmulateRelease(records[2], K2, mods = 2)
        self.assertEmulateRelease(records[3], SHIFT, mods = 2)

    def test_SCKKCS(self):



# Layers

L0 = 0

# Keycodes

KC_SHIFT = Keycode(smtd, 100, 0, 0, L0)
KC1_MT_CTRL = Keycode(smtd, 101, 0, 1, L0)
KC2 = Keycode(smtd, 102, 0, 2, L0)
KC3 = Keycode(smtd, 103, 0, 3, L0)

all_keycodes = [KC_SHIFT, KC1_MT_CTRL, KC2, KC3,]

# Keys

SHIFT = Key(smtd, 'SHIFT', 0, 0, "non-smtd-shift", all_keycodes)
MT_CTRL = Key(smtd, 'MT_CTRL', 0, 1, "smtd-ctrl", all_keycodes)
K2 = Key(smtd, 'K2', 0, 2, "K2", all_keycodes)
K3 = Key(smtd, 'K3', 0, 2, "K3", all_keycodes)

all_keys = [SHIFT, MT_CTRL, K2, K3]


def reset():
    for keycode in all_keycodes:
        keycode.reset()
    for key in all_keys:
        key.reset()
    smtd.reset()


if __name__ == "__main__":
    unittest.main()
