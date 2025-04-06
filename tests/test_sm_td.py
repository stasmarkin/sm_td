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
    A = (0, 0, "no special behaviour")
    S = (0, 1, "no special behaviour")
    D = (0, 2, "SMTD_MT_ON_MKEY(L0_KC2, MACRO2, KC_LEFT_GUI, 2)")
    F = (0, 3, "SMTD_MT(L*_KC3, KC_LEFT_ALT)")
    J = (0, 4, "SMTD_MT(L*_KC4, KC_LEFT_CTRL)")
    K = (0, 5, "SMTD_LT(L0_KC5, L1), SMTD_LT(L2_KC5, L3)")
    L = (0, 6, "SMTD_LT(L0_KC5, L2), SMTD_LT(L1_KC5, L3)")
    N = (0, 7, "no special behaviour") # Ñ in Spanish layout,

    def __init__(self, row, col, comment):
        self._rowcol = (row, col)
        self._comment = comment
        self._pressed = None
        self._released = None

    @classmethod
    def reset(cls):
        """Reset all keys to their initial state"""
        for key in cls:
            key._pressed = None
            key._released = None

    def press(self):
        assert self._pressed is None
        self._released = None
        self._pressed = Keycode.from_rowcol(self.rowcol())
        return self._pressed.press()

    def release(self):
        assert self._pressed is not None
        assert self._released is None
        result = self._pressed.release()
        self._released = self._pressed
        self._pressed = None
        return result

    def prolong(self):
        if self._pressed: return self._pressed.prolong()
        if self._released: return self._released.prolong()
        raise "both _pressed and _released are None"

    def rowcol(self):
        return self._rowcol

    def __repr__(self):
        return self.__str__()

    def __str__(self):
        return f"Key.{self.name} # {self._comment}"


class TestSmTd(unittest.TestCase):
    def setUp(self):
        """Method to run before each test"""
        reset()
        Key.reset()

    def tearDown(self):
        """Method to run after each test"""
        assert get_mods() == 0
        assert get_layer_state() == 0

        for d in get_deferred_execs():
            if d["active"]: execute_deferred(d["idx"])
        for d in get_deferred_execs():
            assert not d["active"]

        assert get_mods() == 0
        assert get_layer_state() == 0


    def assertEvent(self, event, rowcol=(255, 255), keycodeValue=65535, pressed=True, mods=0, layer_state=0,
                    smtd_bypass=False):
        self.assertEqual(event["row"], rowcol[0])
        self.assertEqual(event["col"], rowcol[1])
        self.assertEqual(event["keycode"], keycodeValue)
        self.assertEqual(event["pressed"], pressed)
        self.assertEqual(event["mods"], mods)
        self.assertEqual(event["layer_state"], layer_state)
        self.assertEqual(event["smtd_bypass"], smtd_bypass)

    def assertRegister(self, event, keycode, mods=0, layer_state=0, smtd_bypass=False):
        self.assertEvent(event, keycodeValue=keycode.value, pressed=True, mods=mods,
                         layer_state=layer_state, smtd_bypass=smtd_bypass)

    def assertUnregister(self, event, keycode, mods=0, layer_state=0, smtd_bypass=False):
        self.assertEvent(event, keycodeValue=keycode.value, pressed=False, mods=mods,
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
        self.assertRegister(records[0], Keycode.MACRO2)
        self.assertUnregister(records[1], Keycode.MACRO2)


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


    def test_basic_MT_taphold(self):
        self.assertFalse(Key.D.press(), "press should block future key events")
        self.assertEqual(len(get_record_history()), 0)

        self.assertFalse(Key.D.release(), "release should return true")
        self.assertEqual(len(get_record_history()), 2)

        self.assertFalse(Key.D.press(), "press should block future key events")
        self.assertEqual(len(get_record_history()), 2)

        self.assertEqual(get_mods(), 0)
        Key.D.prolong()
        self.assertEqual(get_mods(), 8)
        self.assertFalse(Key.D.release(), "release should return true")
        self.assertEqual(get_mods(), 0)

        print("\nevents:")
        records = get_record_history()
        for r in records: print(f"{r}")

        self.assertEqual(len(records), 2)
        self.assertRegister(records[0], Keycode.MACRO2)
        self.assertUnregister(records[1], Keycode.MACRO2)


    def test_basic_MT_taptaptap(self):
        self.assertFalse(Key.D.press(), "press should block future key events")
        self.assertEqual(len(get_record_history()), 0)

        self.assertFalse(Key.D.release(), "release should return true")
        self.assertEqual(len(get_record_history()), 2)

        self.assertFalse(Key.D.press(), "press should block future key events")
        self.assertEqual(len(get_record_history()), 2)

        self.assertFalse(Key.D.release(), "release should return true")
        self.assertEqual(len(get_record_history()), 4)

        self.assertFalse(Key.D.press(), "press should block future key events")
        self.assertEqual(len(get_record_history()), 4)

        Key.D.prolong()
        self.assertEqual(len(get_record_history()), 5)

        self.assertFalse(Key.D.release(), "release should return true")
        self.assertEqual(len(get_record_history()), 6)

        records = get_record_history()
        self.assertRegister(records[0], Keycode.MACRO2)
        self.assertUnregister(records[1], Keycode.MACRO2)
        self.assertRegister(records[2], Keycode.MACRO2)
        self.assertUnregister(records[3], Keycode.MACRO2)
        self.assertRegister(records[4], Keycode.MACRO2)
        self.assertUnregister(records[5], Keycode.MACRO2)


    def test_LT_MT_KEY_DOWN__MT_LT_KEY_UP(self):
        presses = [Key.K, Key.F, Key.A]
        releases = [Key.F, Key.K, Key.A]

        for p in presses: p.press()
        for r in releases: r.release()

        print("\n\npresses:")
        for p in presses: print(f" -- {p}")
        print("\nreleases:")
        for r in releases: print(f" -- {r}")

        print("\nevents:")
        records = get_record_history()
        for r in records: print(f"{r}")

        self.assertEmulatePress(records[0], Key.A, layer_state=1, mods=4)
        self.assertEmulateRelease(records[1], Key.A, layer_state=1, mods=4)
        print("\n\n----------------------------------------------------\n\n")

        self.assertEqual(get_mods(), 0)  # fixme every test should test it
        self.assertEqual(get_layer_state(), 0)


    def test_LT_MT_KEY_DOWN__LT_MT_KEY_UP(self):
        presses = [Key.K, Key.F, Key.A]
        releases = [Key.K, Key.F, Key.A]

        for p in presses: p.press()
        for r in releases: r.release()

        print("\n\npresses:")
        for p in presses: print(f" -- {p}")
        print("\nreleases:")
        for r in releases: print(f" -- {r}")

        print("\nevents:")
        records = get_record_history()
        for r in records: print(f"{r}")

        self.assertEqual(len(records), 2)
        self.assertEmulatePress(records[0], Key.A, layer_state=1, mods=4)
        self.assertEmulateRelease(records[1], Key.A, layer_state=1, mods=4)
        print("\n\n----------------------------------------------------\n\n")


    def test_MT_TAP_MT_KEY(self):
        Key.D.press()
        Key.D.release()
        Key.D.press()
        Key.A.press()
        Key.D.release()
        Key.A.release()

        print('\n\n')
        records = get_record_history()
        for r in records: print(f"{r}")
        self.assertEqual(len(records), 4)
        self.assertRegister(records[0], Keycode.MACRO2)
        self.assertUnregister(records[1], Keycode.MACRO2)
        self.assertEmulatePress(records[2], Key.A, mods=8)
        self.assertEmulateRelease(records[3], Key.A, mods=8)


    def test_LT_MT_permutations(self):
        presses = [Key.F, Key.K]
        releases = [Key.F, Key.K, Key.A]

        # sex, hehe
        seqs = []
        for press_seq in itertools.permutations(presses):
            press_seq = press_seq + (Key.A,)  # Key.A must be the last key pressed
            for release_seq in itertools.permutations(releases):
                seqs += [(press_seq, release_seq,)]

        for seq in seqs:
            reset()
            press_seq, release_seq = seq

            for key in press_seq: key.press()
            for key in release_seq: key.release()

            print("\n\npresses:")
            for p in press_seq: print(f" -- {p}")
            print("\nreleases:")
            for r in release_seq: print(f" -- {r}")

            print("\nevents:")
            records = get_record_history()
            for r in records: print(f"{r}")

            self.assertEmulatePress(records[0], Key.A, layer_state=1, mods=4)
            self.assertEmulateRelease(records[1], Key.A, layer_state=1, mods=4)
            print("\n\n----------------------------------------------------\n\n")


    def test_LT_layer_switch(self):
        Key.K.press()
        Key.A.press()
        Key.A.release()
        Key.K.release()

        records = get_record_history()
        self.assertEqual(len(records), 2)
        self.assertEmulatePress(records[0], Key.A, layer_state=1)
        self.assertEmulateRelease(records[1], Key.A, layer_state=1)


    def test_instant_bypass(self):
        Key.A.press()
        # fixme вот тут можно было бы и отпускать процесс, а не стопорить и эмулировать нажатие
        # то есть не включать байпас, а сделать так, чтобы process_smtd просто отпускал нажатие клавиши
        Key.A.release()


if __name__ == "__main__":
    unittest.main()
