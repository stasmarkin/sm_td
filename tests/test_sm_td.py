import unittest
import os
import sys
import itertools

from tests.sm_td_bindings import *

# Add parent directory to path so we can import modules
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

# Import our bindings module
from tests import sm_td_bindings as smtd


class Key(Enum):
    A = (0, 0)  # no special behaviour
    S = (0, 1)  # no special behaviour
    D = (0, 2)  # SMTD_MT_ON_MKEY(L0_KC2, MACRO2, KC_LEFT_GUI)
    F = (0, 3)  # SMTD_MT(L0_KC3, KC_LEFT_ALT), SMTD_MT(L1_KC3, KC_LEFT_ALT)
    J = (0, 4)  # SMTD_MT(L0_KC4, KC_LEFT_CTRL), SMTD_MT(L1_KC4, KC_LEFT_CTRL)
    K = (0, 5)  # SMTD_LT(L0_KC5, L1)
    L = (0, 6)  # no special behaviour
    N = (0, 7)  # Ñ in Spanish layout, no special behaviour

    def __init__(self, row, col):
        self._rowcol = (row, col)
        self._keycode = None

    @classmethod
    def reset(cls):
        """Reset all keys to their initial state"""
        for key in cls:
            key._keycode = None

    def press(self):
        assert self._keycode is None
        self._keycode = Keycode.from_rowcol(self.rowcol())
        return self._keycode.press()

    def release(self):
        assert self._keycode is not None
        result = self._keycode.release()
        self._keycode = None
        return result

    def prolong(self):
        assert self._keycode is not None
        return self._keycode.prolong()

    def rowcol(self):
        return self._rowcol


class TestSmTd(unittest.TestCase):
    def setUp(self):
        """Method to run before each test"""
        reset()
        clear_debug_buffer()  # Clear debug output before each test
        Key.reset()

    def assertEvent(self, event, rowcol=(255, 255), keycode=65535, pressed=True, mods=0, layer_state=0,
                    smtd_bypass=False):
        self.assertEqual(event["row"], rowcol[0])
        self.assertEqual(event["col"], rowcol[1])
        self.assertEqual(event["keycode"], keycode)
        self.assertEqual(event["pressed"], pressed)
        self.assertEqual(event["mods"], mods)
        self.assertEqual(event["layer_state"], layer_state)
        self.assertEqual(event["smtd_bypass"], smtd_bypass)

    def assertRegister(self, event, keycode, mods=0, layer_state=0, smtd_bypass=False):
        self.assertEvent(event, keycode=keycode, pressed=True, mods=mods,
                         layer_state=layer_state, smtd_bypass=smtd_bypass)

    def assertUnregister(self, event, keycode, mods=0, layer_state=0, smtd_bypass=False):
        self.assertEvent(event, keycode=keycode, pressed=False, mods=mods,
                         layer_state=layer_state, smtd_bypass=smtd_bypass)

    def assertEmulatePress(self, event, key, mods=0, layer_state=0):
        self.assertEvent(event, key.rowcol(), pressed=True, mods=mods, layer_state=layer_state, smtd_bypass=True)

    def assertEmulateRelease(self, event, key, mods=0, layer_state=0):
        self.assertEvent(event, key.rowcol(), pressed=False, mods=mods, layer_state=layer_state, smtd_bypass=True)

    def test_process_smtd(self):
        """Test that process_smtd function from the actual library works"""
        self.assertFalse(Key.A.press(), "process_smtd should block future key events")

        records = get_record_history()
        self.assertEqual(len(records), 1)
        self.assertEmulatePress(records[0], Key.A)

    def test_bypass_mode(self):
        """Test the actual bypass mode in the library"""
        set_bypass(True)
        self.assertTrue(Key.A.press(), "sm_td should return true in bypass mode")

        self.assertEqual(len(get_record_history()), 0)

    def test_reset(self):
        """Test the reset function"""
        Key.A.press()
        records_before = get_record_history()

        reset()
        records_after = get_record_history()

        self.assertGreater(len(records_before), 0, "No records were created")
        self.assertEqual(len(records_after), 0, "Records were not cleared after reset")

    def test_generic_tap(self):
        """Test the generic tap function"""
        self.assertFalse(Key.S.press(), "press should block future key events")
        records = get_record_history()
        self.assertEqual(len(records), 1)
        self.assertEmulatePress(records[0], Key.S)

        self.assertFalse(Key.S.release(), "release should block future key events")
        records = get_record_history()
        self.assertEqual(len(records), 2)
        self.assertEmulateRelease(records[1], Key.S)

    def test_basic_MT_ON_MKEY_tap(self):
        """Test the basic MT function"""
        self.assertFalse(Key.D.press(), "press should block future key events")
        records = get_record_history()
        self.assertEqual(len(records), 0)

        self.assertFalse(Key.D.release(), "release should return true")
        records = get_record_history()
        self.assertEqual(len(records), 2, "tap should happer after release")
        self.assertRegister(records[0], Keycode.MACRO2.value)
        self.assertUnregister(records[1], Keycode.MACRO2.value)

    def test_basic_MT_ON_MKEY_hold(self):
        """Test the basic MT function"""
        self.assertFalse(Key.D.press(), "press should block future key events")
        self.assertEqual(len(get_record_history()), 0)
        self.assertEqual(get_mods(), 0)

        Key.D.prolong()
        self.assertEqual(len(get_record_history()), 0)
        self.assertEqual(get_mods(), 8)

        self.assertFalse(Key.D.release(), "release should return true")
        self.assertEqual(len(get_record_history()), 0)
        self.assertEqual(get_mods(), 0)

    def test_basic_MT_ON_MKEY_taphold(self):
        self.assertFalse(Key.D.press(), "press should block future key events")
        self.assertEqual(len(get_record_history()), 0)

        self.assertFalse(Key.D.release(), "release should return true")
        self.assertEqual(len(get_record_history()), 2)

        self.assertFalse(Key.D.press(), "press should block future key events")
        self.assertEqual(len(get_record_history()), 2)

        Key.D.prolong()
        self.assertEqual(len(get_record_history()), 3)

        self.assertFalse(Key.D.release(), "release should return true")
        self.assertEqual(len(get_record_history()), 4)

        records = get_record_history()
        self.assertRegister(records[0], Keycode.MACRO2.value)
        self.assertUnregister(records[1], Keycode.MACRO2.value)
        self.assertRegister(records[2], Keycode.MACRO2.value)
        self.assertUnregister(records[3], Keycode.MACRO2.value)

    def test_complex_hotkey(self):
        presses = [Keycode.L0_KC3, Keycode.L0_KC4, Keycode.L0_KC5]
        releases = [Keycode.L0_KC3, Keycode.L0_KC4, Keycode.L0_KC5, Keycode.L0_KC0]

        # sex, hehe
        seqs = []
        for press_seq in itertools.permutations(presses):
            press_seq = press_seq + (Keycode.L0_KC0,)  # L0_KC0 must be the last key pressed
            for release_seq in itertools.permutations(releases):
                seqs += [(press_seq, release_seq,)]

        for seq in seqs:
            reset()
            press_seq, release_seq = seq

            for key in press_seq: key.press()
            for key in release_seq: key.release()

            records = get_record_history()
            print(records)

    def test_LT_layer_switch(self):
        Keycode.L0_KC5.press()
        Keycode.L0_KC0.press()
        Keycode.L0_KC0.release()
        Keycode.L0_KC5.release()

        records = get_record_history()
        self.assertEqual(len(records), 2)
        self.assertEmulatePress(records[0], Key.A, layer_state=1)
        self.assertEmulateRelease(records[1], Key.A, layer_state=1)

    def test_instant_bypass(self):
        Keycode.L0_KC0.press()  # fixme вот тут можно было бы и отпускать процесс, а не стопорить и эмулировать нажатие
        Keycode.L0_KC0.release()
        # fixme

    def test_debug_output(self):
        """Test that debug output is working"""
        Keycode.L0_KC0.press()
        Keycode.L0_KC0.release()

        debug_output = get_debug_output()
        self.assertIn("GOT KEY", debug_output)
        self.assertIn("EMULATE", debug_output)

        print("\nDebug output from test_debug_output:")
        print(debug_output)


if __name__ == "__main__":
    unittest.main()
