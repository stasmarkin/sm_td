import unittest
import os
import sys
import itertools
from dataclasses import dataclass
import random
from time import sleep

from tests.sm_td_bindings import *

# Add parent directory to path so we can import modules
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

# Import our bindings module
from tests import sm_td_bindings as smtd


class Key(Enum):
    K1 = (0, 0, "no special behaviour")
    K2 = (0, 1, "no special behaviour")
    MMT = (0, 2, "SMTD_MT_ON_MKEY(L0_KC2, MACRO2, KC_LEFT_GUI, 2)")
    MT1 = (0, 3, "SMTD_MT(L*_KC3, KC_LEFT_ALT)")
    MT2 = (0, 4, "SMTD_MT(L*_KC4, KC_LEFT_CTRL)")
    LT1 = (0, 5, "SMTD_LT(L0_KC5, L1), SMTD_LT(L2_KC5, L3)")
    LT2 = (0, 6, "SMTD_LT(L0_KC5, L2), SMTD_LT(L1_KC5, L3)")
    MTE = (0, 7, "SMTD_MTE(L*_KC3, KC_LEFT_SHIFT)")  # Ñ in Spanish layout,

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

    def try_prolong(self):
        if self._pressed: return self._pressed.try_prolong()
        if self._released: return self._released.try_prolong()
        raise "both _pressed and _released are None"

    def rowcol(self):
        return self._rowcol

    def __repr__(self):
        return self.__str__()

    def __str__(self):
        return f"Key.{self.name} # {self._comment}"


@dataclass
class Register:
    keycode: Keycode
    bypass: bool = False
    mods: int = 0
    layer: int = 0


@dataclass
class Unregister:
    keycode: Keycode
    bypass: bool = False
    mods: int = 0
    layer: int = 0


@dataclass
class EmulatePress:
    key: Key
    mods: int = 0
    layer: int = 0


@dataclass
class EmulateRelease:
    key: Key
    mods: int = 0
    layer: int = 0


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

    def assertHistory(self, *args):
        history = get_record_history()
        assert len(history) == len(args)

        for i, a in enumerate(args):
            if isinstance(a, EmulatePress):
                self.assertEmulatePress(history[i], a.key, a.mods, a.layer)
            elif isinstance(a, EmulateRelease):
                self.assertEmulateRelease(history[i], a.key, a.mods, a.layer)
            elif isinstance(a, Register):
                self.assertRegister(history[i], a.keycode, a.mods, a.layer, a.bypass)
            elif isinstance(a, Unregister):
                self.assertUnregister(history[i], a.keycode, a.mods, a.layer, a.bypass)
            else:
                raise ValueError(f"Unknown type in assertHistory {a}")

    def assertEvent(self, event, rowcol=(255, 255), keycodeValue=65535, pressed=True, mods=0, layer_state=0,
                    smtd_bypass=False):
        self.assertEqual(event["row"], rowcol[0], f"{event} doesn't match rowcol={rowcol}")
        self.assertEqual(event["col"], rowcol[1], f"{event} doesn't match rowcol={rowcol}")
        self.assertEqual(event["keycode"], keycodeValue, f"{event} doesn't match keycodeValue={keycodeValue}")
        self.assertEqual(event["pressed"], pressed, f"{event} doesn't match pressed={pressed}")
        self.assertEqual(event["mods"], mods, f"{event} doesn't match mods={mods}")
        self.assertEqual(event["layer_state"], layer_state, f"{event} doesn't match layer_state={layer_state}")
        self.assertEqual(event["smtd_bypass"], smtd_bypass, f"{event} doesn't match smtd_bypass={smtd_bypass}")

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
        self.assertFalse(Key.K1.press(), "process_smtd should block future key events")

        records = get_record_history()
        self.assertEqual(len(records), 1)
        self.assertEmulatePress(records[0], Key.K1)

    def test_bypass_mode(self):
        """Test the actual bypass mode in the library"""
        set_bypass(True)
        self.assertTrue(Key.K1.press(), "sm_td should return true in bypass mode")
        self.assertEqual(len(get_record_history()), 0)

    def test_reset(self):
        """Test the reset function"""
        Key.K1.press()
        records_before = get_record_history()

        reset()
        records_after = get_record_history()

        self.assertGreater(len(records_before), 0, "No records were created")
        self.assertEqual(len(records_after), 0, "Records were not cleared after reset")

    def test_generic_tap(self):
        """Test the generic tap function"""
        self.assertFalse(Key.K2.press(), "press should block future key events")
        records = get_record_history()
        self.assertEqual(len(records), 1)
        self.assertEmulatePress(records[0], Key.K2)

        self.assertFalse(Key.K2.release(), "release should block future key events")
        records = get_record_history()
        self.assertEqual(len(records), 2)
        self.assertEmulateRelease(records[1], Key.K2)

    def test_basic_MT_ON_MKEY_tap(self):
        """Test the basic MT function"""
        self.assertFalse(Key.MMT.press(), "press should block future key events")
        records = get_record_history()
        self.assertEqual(len(records), 0)

        self.assertFalse(Key.MMT.release(), "release should return true")
        records = get_record_history()
        self.assertEqual(len(records), 2, "tap should happer after release")
        self.assertRegister(records[0], Keycode.MACRO2)
        self.assertUnregister(records[1], Keycode.MACRO2)

    def test_basic_MT_ON_MKEY_hold(self):
        """Test the basic MT function"""
        self.assertFalse(Key.MMT.press(), "press should block future key events")
        self.assertEqual(len(get_record_history()), 0)
        self.assertEqual(get_mods(), 0)

        Key.MMT.prolong()
        self.assertEqual(len(get_record_history()), 0)
        self.assertEqual(get_mods(), 8)

        self.assertFalse(Key.MMT.release(), "release should return true")
        self.assertEqual(len(get_record_history()), 0)
        self.assertEqual(get_mods(), 0)

    def test_basic_MT_taphold(self):
        self.assertFalse(Key.MMT.press(), "press should block future key events")
        self.assertEqual(len(get_record_history()), 0)

        self.assertFalse(Key.MMT.release(), "release should return true")
        self.assertEqual(len(get_record_history()), 2)

        self.assertFalse(Key.MMT.press(), "press should block future key events")
        self.assertEqual(len(get_record_history()), 2)

        self.assertEqual(get_mods(), 0)
        Key.MMT.prolong()
        self.assertEqual(get_mods(), 8)
        self.assertFalse(Key.MMT.release(), "release should return true")
        self.assertEqual(get_mods(), 0)

        print("\nevents:")
        records = get_record_history()
        for r in records: print(f"{r}")

        self.assertEqual(len(records), 2)
        self.assertRegister(records[0], Keycode.MACRO2)
        self.assertUnregister(records[1], Keycode.MACRO2)

    def test_basic_MT_taptaptap(self):
        self.assertFalse(Key.MMT.press(), "press should block future key events")
        self.assertEqual(len(get_record_history()), 0)

        self.assertFalse(Key.MMT.release(), "release should return true")
        self.assertEqual(len(get_record_history()), 2)

        self.assertFalse(Key.MMT.press(), "press should block future key events")
        self.assertEqual(len(get_record_history()), 2)

        self.assertFalse(Key.MMT.release(), "release should return true")
        self.assertEqual(len(get_record_history()), 4)

        self.assertFalse(Key.MMT.press(), "press should block future key events")
        self.assertEqual(len(get_record_history()), 4)

        Key.MMT.prolong()
        self.assertEqual(len(get_record_history()), 5)

        self.assertFalse(Key.MMT.release(), "release should return true")
        self.assertEqual(len(get_record_history()), 6)

        records = get_record_history()
        self.assertRegister(records[0], Keycode.MACRO2)
        self.assertUnregister(records[1], Keycode.MACRO2)
        self.assertRegister(records[2], Keycode.MACRO2)
        self.assertUnregister(records[3], Keycode.MACRO2)
        self.assertRegister(records[4], Keycode.MACRO2)
        self.assertUnregister(records[5], Keycode.MACRO2)

    def test_MTE_hold(self):
        self.assertFalse(Key.MTE.press())
        self.assertHistory()
        self.assertEqual(get_mods(), 2)

        Key.MTE.prolong()
        self.assertHistory()
        self.assertEqual(get_mods(), 2)

        Key.MTE.release()
        self.assertHistory()
        self.assertEqual(get_mods(), 0)

    def test_MTE_tap(self):
        self.assertFalse(Key.MTE.press())
        self.assertHistory()
        self.assertEqual(get_mods(), 2)

        Key.MTE.release()
        self.assertEqual(get_mods(), 0)
        self.assertHistory(
            Register(Keycode.L0_KC7),
            Unregister(Keycode.L0_KC7),
        )

    def test_LT_MT_KEY_DOWN__MT_LT_KEY_UP(self):
        presses = [Key.LT1, Key.MT1, Key.K1]
        releases = [Key.MT1, Key.LT1, Key.K1]

        for p in presses: p.press()
        for r in releases: r.release()

        print("\n\npresses:")
        for p in presses: print(f" -- {p}")
        print("\nreleases:")
        for r in releases: print(f" -- {r}")

        print("\nevents:")
        records = get_record_history()
        for r in records: print(f"{r}")

        self.assertEmulatePress(records[0], Key.K1, layer_state=1, mods=4)
        self.assertEmulateRelease(records[1], Key.K1, layer_state=1, mods=4)
        print("\n\n----------------------------------------------------\n\n")

        self.assertEqual(get_mods(), 0)  # fixme every test should test it
        self.assertEqual(get_layer_state(), 0)

    def test_LT_MT_KEY_DOWN__LT_MT_KEY_UP(self):
        presses = [Key.LT1, Key.MT1, Key.K1]
        releases = [Key.LT1, Key.MT1, Key.K1]

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
        self.assertEmulatePress(records[0], Key.K1, layer_state=1, mods=4)
        self.assertEmulateRelease(records[1], Key.K1, layer_state=1, mods=4)
        print("\n\n----------------------------------------------------\n\n")

    def test_MT_TAP_MT_KEY(self):
        Key.MMT.press()
        Key.MMT.release()
        Key.MMT.press()
        Key.K1.press()
        Key.MMT.release()
        Key.K1.release()

        print('\n\n')
        records = get_record_history()
        for r in records: print(f"{r}")
        self.assertEqual(len(records), 4)
        self.assertRegister(records[0], Keycode.MACRO2)
        self.assertUnregister(records[1], Keycode.MACRO2)
        self.assertEmulatePress(records[2], Key.K1, mods=8)
        self.assertEmulateRelease(records[3], Key.K1, mods=8)

    def test_LT_MT_permutations(self):
        presses = [Key.MT1, Key.LT1]
        releases = [Key.MT1, Key.LT1, Key.K1]

        # sex, hehe
        seqs = []
        for press_seq in itertools.permutations(presses):
            press_seq = press_seq + (Key.K1,)  # Key.A must be the last key pressed
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

            self.assertEmulatePress(records[0], Key.K1, layer_state=1, mods=4)
            self.assertEmulateRelease(records[1], Key.K1, layer_state=1, mods=4)
            print("\n\n----------------------------------------------------\n\n")

    def test_LT_layer_switch(self):
        Key.LT1.press()
        Key.K1.press()
        Key.K1.release()
        Key.LT1.release()

        records = get_record_history()
        self.assertEqual(len(records), 2)
        self.assertEmulatePress(records[0], Key.K1, layer_state=1)
        self.assertEmulateRelease(records[1], Key.K1, layer_state=1)

    def test_instant_bypass(self):
        Key.K1.press()
        # fixme вот тут можно было бы и отпускать процесс, а не стопорить и эмулировать нажатие
        # то есть не включать байпас, а сделать так, чтобы process_smtd просто отпускал нажатие клавиши
        Key.K1.release()

    def test_MT_LT_key_MT_key_ordered(self):
        Key.LT1.press()
        Key.MT1.press()
        Key.K1.press()
        Key.K1.release()
        Key.MT1.release()
        Key.MTE.press()
        Key.K2.press()
        Key.K2.release()
        Key.MTE.release()
        Key.LT1.release()

        print("\n\n\n------------------------------------------")
        print("\n\nevents:")
        for r in get_record_history(): print(f"{r}")
        print("\n\n------------------------------------------\n\n\n")

        self.assertHistory(
            EmulatePress(Key.K1, layer=1, mods=4),
            EmulateRelease(Key.K1, layer=1, mods=4),
            EmulatePress(Key.K2, layer=1, mods=2),
            EmulateRelease(Key.K2, layer=1, mods=2),
        )


    def test_MT_LT_key_MT_key_shuffled(self):
        Key.MT1.press()
        Key.LT1.press()
        Key.K1.press()
        Key.MTE.press()
        Key.MT1.release()
        Key.K2.press()
        Key.K1.release()

        print("\n\n\n------------------------------------------")
        print("\n\nevents:")
        for r in get_record_history(): print(f"{r}")
        print("\n\n------------------------------------------\n\n\n")
        Key.K2.release()

        print("\n\n\n------------------------------------------")
        print("\n\nevents:")
        for r in get_record_history(): print(f"{r}")
        print("\n\n------------------------------------------\n\n\n")

        Key.LT1.release()
        Key.MTE.release()


        print("\n\n\n------------------------------------------")
        print("\n\nevents:")
        for r in get_record_history(): print(f"{r}")
        print("\n\n------------------------------------------\n\n\n")

        self.assertHistory(
            EmulatePress(Key.K1, layer=1, mods=4),
            EmulateRelease(Key.K1, layer=1, mods=4),
            EmulatePress(Key.K2, layer=1, mods=2),
            EmulateRelease(Key.K2, layer=1, mods=2),
        )


    def test_faceroll_must_return_to_default_state(self):
        """
        Test that the state of the SMTD is reset after a faceroll.

        This test generates random combinations of key presses and releases
        but in the end all pressed keys must be released and the state of the SMTD must be reset.
        Since number of combinations is huge, we just sample a few of them.
        """
        sample_size = 1_000
        faceroll_keys = [k for k in Key]

        def fn_prolong(key, *args):
            return (
                lambda: key.try_prolong(),
                [_a(key) for _a in args],
                f"prolong {key.name}",
            )

        def fn_release(key):
            return (
                lambda: key.release(),
                [fn_prolong(key)],
                f"release {key.name}",
            )

        def fn_press(key):
            return (
                lambda: key.press(),
                [fn_release(key), fn_prolong(key)],
                f"press {key.name}",
            )

        def permutations_generator():
            while True:
                subset = random.sample(faceroll_keys, random.randint(1, len(faceroll_keys)))
                fn_left = [fn_press(k) for k in subset]

                result_desc = "/reset/"
                result_actions = []

                while fn_left:
                    fn = random.choice(fn_left)
                    fn_left.remove(fn)
                    action, next_fns, desc = fn
                    result_actions.append(action)
                    for next_fn in next_fns:
                        fn_left.append(next_fn)
                    result_desc += f" -> {desc}"

                yield (result_desc, result_actions)

        generator = permutations_generator()

        for _ in range(sample_size):
            reset()
            desc, actions = next(generator)
            for action in actions: action()

            # Check that the state is reset
            assert get_mods() == 0, f"Mods should be 0 after {desc}"
            assert get_layer_state() == 0, f"Layer state should be 0 after {desc}"


if __name__ == "__main__":
    unittest.main()
