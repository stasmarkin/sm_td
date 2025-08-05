import unittest
import sys
import itertools
from dataclasses import dataclass
import random
from enum import Enum

from sm_td_bindings import *

# Add parent directory to path so we can import modules
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))


class Layer(Enum):
    L0 = 0
    L1 = 1
    L2 = 2


class Keycodes:
    def __init__(self, smtd):
        self.L0_KC0 = Keycode(smtd, 100, 0, 0, Layer.L0)
        self.L0_KC1 = Keycode(smtd, 101, 0, 1, Layer.L0)
        self.L0_KC2 = Keycode(smtd, 102, 0, 2, Layer.L0)
        self.L0_KC3 = Keycode(smtd, 103, 0, 3, Layer.L0)
        self.L0_KC4 = Keycode(smtd, 104, 0, 4, Layer.L0)
        self.L0_KC5 = Keycode(smtd, 105, 0, 5, Layer.L0)
        self.L0_KC6 = Keycode(smtd, 106, 0, 6, Layer.L0)
        self.L0_KC7 = Keycode(smtd, 107, 0, 7, Layer.L0)
        self.L0_KC8 = Keycode(smtd, 108, 0, 8, Layer.L0)

        self.L1_KC0 = Keycode(smtd, 200, 0, 0, Layer.L1)
        self.L1_KC1 = Keycode(smtd, 201, 0, 1, Layer.L1)
        self.L1_KC2 = Keycode(smtd, 202, 0, 2, Layer.L1)
        self.L1_KC3 = Keycode(smtd, 203, 0, 3, Layer.L1)
        self.L1_KC4 = Keycode(smtd, 204, 0, 4, Layer.L1)
        self.L1_KC5 = Keycode(smtd, 205, 0, 5, Layer.L1)
        self.L1_KC6 = Keycode(smtd, 206, 0, 6, Layer.L1)
        self.L1_KC7 = Keycode(smtd, 207, 0, 7, Layer.L1)
        self.L1_KC8 = Keycode(smtd, 208, 0, 8, Layer.L1)

        self.L2_KC0 = Keycode(smtd, 300, 0, 0, Layer.L2)
        self.L2_KC1 = Keycode(smtd, 301, 0, 1, Layer.L2)
        self.L2_KC2 = Keycode(smtd, 302, 0, 2, Layer.L2)
        self.L2_KC3 = Keycode(smtd, 303, 0, 3, Layer.L2)
        self.L2_KC4 = Keycode(smtd, 304, 0, 4, Layer.L2)
        self.L2_KC5 = Keycode(smtd, 305, 0, 5, Layer.L2)
        self.L2_KC6 = Keycode(smtd, 306, 0, 6, Layer.L2)
        self.L2_KC7 = Keycode(smtd, 307, 0, 7, Layer.L2)
        self.L2_KC8 = Keycode(smtd, 308, 0, 8, Layer.L2)

        self.MACRO0 = Keycode(smtd, 500, 0, 0, -100)
        self.MACRO1 = Keycode(smtd, 501, 0, 1, -100)
        self.MACRO2 = Keycode(smtd, 502, 0, 2, -100)
        self.MACRO3 = Keycode(smtd, 503, 0, 3, -100)
        self.MACRO4 = Keycode(smtd, 504, 0, 4, -100)
        self.MACRO5 = Keycode(smtd, 505, 0, 5, -100)
        self.MACRO6 = Keycode(smtd, 506, 0, 6, -100)
        self.MACRO7 = Keycode(smtd, 507, 0, 7, -100)
        self.MACRO8 = Keycode(smtd, 508, 0, 8, -100)

    def from_rowcol(self, smtd, rowcol):
        return self.from_layer_rowcol(smtd.get_layer_state(), rowcol)

    def from_layer_rowcol(self, layer, rowcol):
        return self.from_value(100 + layer * 100 + rowcol[0] * 10 + rowcol[1])

    def from_value(self, value):
        """Get the enum member from its value"""
        for member in self:  # fixme-sm
            if member.value == value:
                return member
        raise ValueError(f"No enum member with value {value}")

    def reset(self):
        for member in self:  # fixme-sm
            member.__init__(member._value)


class Keys:
    def __init__(self, smtd):
        self.smtd = smtd
        self.K1 = Key(smtd, 'K1', 0, 0, "no special behaviour")
        self.K2 = Key(smtd, 'K2', 0, 1, "no special behaviour")
        self.MMT = Key(smtd, 'MMT', 0, 2, "SMTD_MT_ON_MKEY(L0_KC2, MACRO2, KC_LEFT_GUI, 2)")
        self.MT1 = Key(smtd, 'MT1', 0, 3, "SMTD_MT(L*_KC3, KC_LEFT_ALT)")
        self.MT2 = Key(smtd, 'MT2', 0, 4, "SMTD_MT(L*_KC4, KC_LEFT_CTRL)")
        self.LT1 = Key(smtd, 'LT1', 0, 5, "SMTD_LT(L0_KC5, L1), SMTD_LT(L2_KC5, L3)")
        self.LT2 = Key(smtd, 'LT2', 0, 6, "SMTD_LT(L0_KC5, L2), SMTD_LT(L1_KC5, L3)")
        self.MTE = Key(smtd, 'MTE', 0, 7, "SMTD_MTE(L*_KC3, KC_LEFT_SHIFT)")  # Ñ in Spanish layout,
        self.CTRL = Key(smtd, 'CTRL', 0, 8, "KC_LEFT_CTRL")

    def reset(self):
        """Reset all keys to their initial state"""
        for key in self:  # fixme-sm
            key.pressed = None
            key.released = None



class TestSmTd(unittest.TestCase):
    def __init__(self):
        super().__init__()
        self.smtd = load_smtd_lib()
        self.keycodes = Keycodes(self.smtd)
        self.keys = Keys(self.smtd)

    def setUp(self):
        """Method to run before each test"""
        self.smtd.reset()
        Key.reset()

    def tearDown(self):
        """Method to run after each test"""
        assert self.smtd.get_mods() == 0
        assert self.smtd.get_layer_state() == 0

        for d in self.smtd.get_deferred_execs():
            if d["active"]: self.smtd.execute_deferred(d["idx"])
        for d in self.smtd.get_deferred_execs():
            assert not d["active"]

        assert self.smtd.get_mods() == 0
        assert self.smtd.get_layer_state() == 0

    def assertHistory(self, *args):
        history = self.smtd.get_record_history()
        print("\n\nHistory:")
        for h in history: print(f" -- {h}")
        print("\n")

        assert len(history) == len(args)

        for i, a in enumerate(args):
            if isinstance(a, EmulatePress):
                self.assertEmulatePress(history[i], a.key, a.mods, a.layer)
            elif isinstance(a, EmulateRelease):
                self.assertEmulateRelease(history[i], a.key, a.mods, a.layer)
            elif isinstance(a, Register):
                self.assertRegister(history[i], a.keycode, a.mods, a.layer)
            elif isinstance(a, Unregister):
                self.assertUnregister(history[i], a.keycode, a.mods, a.layer)
            else:
                raise ValueError(f"Unknown type in assertHistory {a}")

    def assertEvent(self, event, rowcol=(255, 255), keycodeValue=65535, pressed=True, mods=0, layer_state=0,
                    smtd_bypass=True):
        self.assertEqual(event["row"], rowcol[0], f"{event} doesn't match rowcol={rowcol}")
        self.assertEqual(event["col"], rowcol[1], f"{event} doesn't match rowcol={rowcol}")
        self.assertEqual(event["keycode"], keycodeValue, f"{event} doesn't match keycodeValue={keycodeValue}")
        self.assertEqual(event["pressed"], pressed, f"{event} doesn't match pressed={pressed}")
        if (mods >= 0): self.assertEqual(event["mods"], mods, f"{event} doesn't match mods={mods}")
        if (layer_state >= 0): self.assertEqual(event["layer_state"], layer_state,
                                                f"{event} doesn't match layer_state={layer_state}")
        self.assertEqual(event["smtd_bypass"], smtd_bypass, f"{event} doesn't match smtd_bypass={smtd_bypass}")

    def assertRegister(self, event, keycode, mods=0, layer_state=0, smtd_bypass=True):
        self.assertEvent(event, keycodeValue=keycode.value, pressed=True, mods=mods,
                         layer_state=layer_state, smtd_bypass=smtd_bypass)

    def assertUnregister(self, event, keycode, mods=0, layer_state=0, smtd_bypass=True):
        self.assertEvent(event, keycodeValue=keycode.value, pressed=False, mods=mods,
                         layer_state=layer_state, smtd_bypass=smtd_bypass)

    def assertEmulatePress(self, event, key, mods=0, layer_state=0):
        self.assertEvent(event, key.rowcol(), pressed=True, mods=mods, layer_state=layer_state, smtd_bypass=True)

    def assertEmulateRelease(self, event, key, mods=0, layer_state=0):
        self.assertEvent(event, key.rowcol(), pressed=False, mods=mods, layer_state=layer_state, smtd_bypass=True)

    def test_process_smtd(self):
        """Test that process_smtd function from the actual library works"""
        self.assertFalse(Key.K1.press(), "process_smtd should block future key events")

        records = self.smtd.get_record_history()
        self.assertEqual(len(records), 1)
        self.assertEmulatePress(records[0], Key.K1)

    def test_bypass_mode(self):
        """Test the actual bypass mode in the library"""
        self.smtd.set_bypass(True)
        self.assertTrue(Key.K1.press(), "sm_td should return true in bypass mode")
        self.assertEqual(len(self.smtd.get_record_history()), 0)

    def test_reset(self):
        """Test the reset function"""
        Key.K1.press()
        records_before = self.smtd.get_record_history()

        self.smtd.reset()
        records_after = self.smtd.get_record_history()

        self.assertGreater(len(records_before), 0, "No records were created")
        self.assertEqual(len(records_after), 0, "Records were not cleared after reset")

    def test_generic_tap(self):
        """Test the generic tap function"""
        self.assertFalse(Key.K2.press(), "press should block future key events")
        records = self.smtd.get_record_history()
        self.assertEqual(len(records), 1)
        self.assertEmulatePress(records[0], Key.K2)

        self.assertFalse(Key.K2.release(), "release should block future key events")
        records = self.smtd.get_record_history()
        self.assertEqual(len(records), 2)
        self.assertEmulateRelease(records[1], Key.K2)

    def test_basic_MT_ON_MKEY_tap(self):
        """Test the basic MT function"""
        self.assertFalse(Key.MMT.press(), "press should block future key events")
        records = self.smtd.get_record_history()
        self.assertEqual(len(records), 0)

        self.assertFalse(Key.MMT.release(), "release should return true")
        records = self.smtd.get_record_history()
        self.assertEqual(len(records), 2, "tap should happer after release")
        self.assertRegister(records[0], Keycode.MACRO2)
        self.assertUnregister(records[1], Keycode.MACRO2)

    def test_basic_MT_ON_MKEY_hold(self):
        """Test the basic MT function"""
        self.assertFalse(Key.MMT.press(), "press should block future key events")
        self.assertEqual(len(self.smtd.get_record_history()), 0)
        self.assertEqual(self.smtd.get_mods(), 0)

        Key.MMT.prolong()
        self.assertEqual(len(self.smtd.get_record_history()), 0)
        self.assertEqual(self.smtd.get_mods(), 8)

        self.assertFalse(Key.MMT.release(), "release should return true")
        self.assertEqual(len(self.smtd.get_record_history()), 0)
        self.assertEqual(self.smtd.get_mods(), 0)

    def test_basic_MT_taphold(self):
        self.assertFalse(Key.MMT.press(), "press should block future key events")
        self.assertEqual(len(self.smtd.get_record_history()), 0)

        self.assertFalse(Key.MMT.release(), "release should return true")
        self.assertEqual(len(self.smtd.get_record_history()), 2)

        self.assertFalse(Key.MMT.press(), "press should block future key events")
        self.assertEqual(len(self.smtd.get_record_history()), 2)

        self.assertEqual(self.smtd.get_mods(), 0)
        Key.MMT.prolong()
        self.assertEqual(self.smtd.get_mods(), 8)
        self.assertFalse(Key.MMT.release(), "release should return true")
        self.assertEqual(self.smtd.get_mods(), 0)

        print("\nevents:")
        records = self.smtd.get_record_history()
        for r in records: print(f"{r}")

        self.assertEqual(len(records), 2)
        self.assertRegister(records[0], Keycode.MACRO2)
        self.assertUnregister(records[1], Keycode.MACRO2)

    def test_basic_MT_taptaptap(self):
        self.assertFalse(Key.MMT.press(), "press should block future key events")
        self.assertEqual(len(self.smtd.get_record_history()), 0)

        self.assertFalse(Key.MMT.release(), "release should return true")
        self.assertEqual(len(self.smtd.get_record_history()), 2)

        self.assertFalse(Key.MMT.press(), "press should block future key events")
        self.assertEqual(len(self.smtd.get_record_history()), 2)

        self.assertFalse(Key.MMT.release(), "release should return true")
        self.assertEqual(len(self.smtd.get_record_history()), 4)

        self.assertFalse(Key.MMT.press(), "press should block future key events")
        self.assertEqual(len(self.smtd.get_record_history()), 4)

        Key.MMT.prolong()
        self.assertEqual(len(self.smtd.get_record_history()), 5)

        self.assertFalse(Key.MMT.release(), "release should return true")
        self.assertEqual(len(self.smtd.get_record_history()), 6)

        records = self.smtd.get_record_history()
        self.assertRegister(records[0], Keycode.MACRO2)
        self.assertUnregister(records[1], Keycode.MACRO2)
        self.assertRegister(records[2], Keycode.MACRO2)
        self.assertUnregister(records[3], Keycode.MACRO2)
        self.assertRegister(records[4], Keycode.MACRO2)
        self.assertUnregister(records[5], Keycode.MACRO2)

    def test_MTE_hold(self):
        self.assertFalse(Key.MTE.press())
        self.assertHistory()
        self.assertEqual(self.smtd.get_mods(), 2)

        Key.MTE.prolong()
        self.assertHistory()
        self.assertEqual(self.smtd.get_mods(), 2)

        Key.MTE.release()
        self.assertHistory()
        self.assertEqual(self.smtd.get_mods(), 0)

    def test_MTE_tap(self):
        self.assertFalse(Key.MTE.press())
        self.assertHistory()
        self.assertEqual(self.smtd.get_mods(), 2)

        Key.MTE.release()
        self.assertEqual(self.smtd.get_mods(), 0)
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
        records = self.smtd.get_record_history()
        for r in records: print(f"{r}")

        self.assertEmulatePress(records[0], Key.K1, layer_state=1, mods=4)
        self.assertEmulateRelease(records[1], Key.K1, layer_state=1, mods=4)

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
        records = self.smtd.get_record_history()
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
        records = self.smtd.get_record_history()
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
            self.smtd.reset()
            press_seq, release_seq = seq

            for key in press_seq: key.press()
            for key in release_seq: key.release()

            print("\n\npresses:")
            for p in press_seq: print(f" -- {p}")
            print("\nreleases:")
            for r in release_seq: print(f" -- {r}")

            print("\nevents:")
            records = self.smtd.get_record_history()
            for r in records: print(f"{r}")

            self.assertEmulatePress(records[0], Key.K1, layer_state=1, mods=4)
            self.assertEmulateRelease(records[1], Key.K1, layer_state=1, mods=4)
            print("\n\n----------------------------------------------------\n\n")

    def test_LT_layer_switch(self):
        Key.LT1.press()
        Key.K1.press()
        Key.K1.release()
        Key.LT1.release()

        records = self.smtd.get_record_history()
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
        for r in self.smtd.get_record_history(): print(f"{r}")
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
        Key.K1.release()
        Key.K2.press()
        Key.K2.release()
        Key.LT1.release()
        Key.MTE.release()

        print("\n\n\n------------------------------------------")
        print("\n\nevents:")
        for r in self.smtd.get_record_history(): print(f"{r}")
        print("\n\n------------------------------------------\n\n\n")

        self.assertHistory(
            EmulatePress(Key.K1, layer=1, mods=4),
            EmulateRelease(Key.K1, layer=1, mods=-1),
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
        sample_size = 100
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
            self.smtd.reset()
            desc, actions = next(generator)
            for action in actions: action()

            # Check that the state is reset
            assert self.smtd.get_mods() == 0, f"Mods should be 0 after {desc}"
            assert self.smtd.get_layer_state() == 0, f"Layer state should be 0 after {desc}"

    def test_upright_mod_press(self):
        """Test that the normal mod press works"""
        Key.CTRL.press()
        Key.K1.press()
        Key.K1.release()
        Key.CTRL.release()

        self.assertHistory(
            EmulatePress(Key.CTRL, mods=0),
            EmulatePress(Key.K1, mods=1),
            EmulateRelease(Key.K1, mods=1),
            EmulateRelease(Key.CTRL, mods=1),
        )

    def test_stirred_mod_press(self):
        Key.CTRL.press()
        Key.K1.press()
        Key.CTRL.release()
        Key.K1.release()

        # since CTRL and K1 and both quickly pressed and released, K1 is assumed to be TAPPED
        # and since K1 is tapped, the virtual key press and release appears to be simultaneous
        # so that tap "rearranges" actual press-release sequence
        self.assertHistory(
            EmulatePress(Key.CTRL, mods=0),
            EmulatePress(Key.K1, mods=1),
            EmulateRelease(Key.K1, mods=1),
            EmulateRelease(Key.CTRL, mods=1),
        )

    def test_stirred_long_mod_press1(self):
        Key.CTRL.press()
        Key.CTRL.prolong()
        Key.K1.press()
        Key.CTRL.release()
        Key.K1.release()

        self.assertHistory(
            EmulatePress(Key.CTRL, mods=0),
            EmulatePress(Key.K1, mods=1),
            EmulateRelease(Key.K1, mods=1),
            EmulateRelease(Key.CTRL, mods=1),
        )

    def test_stirred_long_mod_press2(self):
        Key.CTRL.press()
        Key.K1.press()
        Key.K1.prolong()
        Key.CTRL.release()
        Key.K1.release()

        self.assertHistory(
            EmulatePress(Key.CTRL, mods=0),
            EmulatePress(Key.K1, mods=1),
            EmulateRelease(Key.K1, mods=1),
            EmulateRelease(Key.CTRL, mods=1),
        )

    def test_stirred_long_mod_press3(self):
        Key.CTRL.press()
        Key.K1.press()
        Key.CTRL.release()
        Key.CTRL.prolong()
        Key.K1.prolong()
        Key.K1.release()

        self.assertHistory(
            EmulatePress(Key.CTRL, mods=0),
            EmulatePress(Key.K1, mods=1),
            EmulateRelease(Key.CTRL, mods=1),
            EmulateRelease(Key.K1, mods=0),
        )

    def test_upright_mod_smtd_press(self):
        Key.CTRL.press()
        Key.MT1.press()
        Key.MT1.release()
        Key.CTRL.release()

        self.assertHistory(
            EmulatePress(Key.CTRL, mods=0),
            Register(Keycode.L0_KC3, mods=1),
            Unregister(Keycode.L0_KC3, mods=1),
            EmulateRelease(Key.CTRL, mods=1),
        )

    def test_stirred_mod_smtd_press(self):
        Key.CTRL.press()
        Key.MT1.press()
        Key.CTRL.release()
        Key.MT1.release()

        self.assertHistory(
            EmulatePress(Key.CTRL, mods=0),
            Register(Keycode.L0_KC3, mods=1),
            Unregister(Keycode.L0_KC3, mods=1),
            EmulateRelease(Key.CTRL, mods=1),
        )

    def test_stirred_long_mod_smtd_press1(self):
        Key.CTRL.press()
        Key.CTRL.prolong()
        Key.MT1.press()
        Key.CTRL.release()
        Key.MT1.release()

        self.assertHistory(
            EmulatePress(Key.CTRL, mods=0),
            Register(Keycode.L0_KC3, mods=1),
            Unregister(Keycode.L0_KC3, mods=1),
            EmulateRelease(Key.CTRL, mods=1),
        )

    def test_stirred_long_mod_smtd_press2_bugged(self):
        Key.CTRL.press()
        Key.MT1.press()
        Key.MT1.prolong()  # well, MT1 comes a modifier here
        Key.CTRL.release()
        # fixme-sm now CTRL stuck in THL state, so it doesn't sent actual release right now
        # sm_td need to understand that case and skip THL state straight to TAP action and NONE state
        Key.MT1.release()

        self.assertHistory(
            EmulatePress(Key.CTRL, mods=0),
            EmulateRelease(Key.CTRL, mods=1),
        )

    def test_stirred_long_mod_smtd_press2_fixed(self):
        Key.CTRL.press()
        Key.CTRL.prolong()
        Key.MT1.press()
        Key.MT1.prolong()  # well, MT1 comes a modifier here
        Key.CTRL.release()
        Key.CTRL.prolong()
        Key.MT1.release()

        self.assertHistory(
            EmulatePress(Key.CTRL, mods=0),
            EmulateRelease(Key.CTRL, mods=-1),
        )

    def test_stirred_mod_smtd_press3(self):
        Key.CTRL.press()
        Key.MT1.press()
        Key.CTRL.release()
        Key.MT1.release()

        self.assertHistory(
            EmulatePress(Key.CTRL, mods=0),
            Register(Keycode.L0_KC3, mods=1),
            Unregister(Keycode.L0_KC3, mods=-1),
            EmulateRelease(Key.CTRL, mods=-1),
        )

    def test_stirred_long_mod_smtd_press3(self):
        Key.CTRL.press()
        Key.CTRL.prolong()
        Key.MT1.press()
        Key.CTRL.release()
        Key.MT1.release()

        self.assertHistory(
            EmulatePress(Key.CTRL, mods=0),
            Register(Keycode.L0_KC3, mods=1),
            Unregister(Keycode.L0_KC3, mods=-1),
            EmulateRelease(Key.CTRL, mods=-1),
        )

    def test_stirred_long_mod_smtd_press4(self):
        Key.CTRL.press()
        Key.MT1.press()
        Key.CTRL.prolong()
        Key.CTRL.release()
        Key.MT1.release()

        self.assertHistory(
            EmulatePress(Key.CTRL, mods=0),
            Register(Keycode.L0_KC3, mods=1),
            Unregister(Keycode.L0_KC3, mods=-1),
            EmulateRelease(Key.CTRL, mods=-1),
        )


@dataclass
class Register:
    keycode: Keycode
    mods: int = -1
    layer: int = -1


@dataclass
class Unregister:
    keycode: Keycode
    mods: int = -1
    layer: int = -1


@dataclass
class EmulatePress:
    key: Key
    mods: int = -1
    layer: int = -1


@dataclass
class EmulateRelease:
    key: Key
    mods: int = -1
    layer: int = -1


if __name__ == "__main__":
    unittest.main()
