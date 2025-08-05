import unittest
import sys
import itertools
from dataclasses import dataclass
import random

from sm_td_bindings import *

# Add parent directory to path so we can import modules
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

smtd = load_smtd_lib()

# Layers

L0 = 0
L1 = 1
L2 = 2

# Keycodes

L0_KC0 = Keycode(smtd, 100, 0, 0, L0)
L0_KC1 = Keycode(smtd, 101, 0, 1, L0)
L0_KC2 = Keycode(smtd, 102, 0, 2, L0)
L0_KC3 = Keycode(smtd, 103, 0, 3, L0)
L0_KC4 = Keycode(smtd, 104, 0, 4, L0)
L0_KC5 = Keycode(smtd, 105, 0, 5, L0)
L0_KC6 = Keycode(smtd, 106, 0, 6, L0)
L0_KC7 = Keycode(smtd, 107, 0, 7, L0)
L0_KC8 = Keycode(smtd, 108, 0, 8, L0)

L1_KC0 = Keycode(smtd, 200, 0, 0, L1)
L1_KC1 = Keycode(smtd, 201, 0, 1, L1)
L1_KC2 = Keycode(smtd, 202, 0, 2, L1)
L1_KC3 = Keycode(smtd, 203, 0, 3, L1)
L1_KC4 = Keycode(smtd, 204, 0, 4, L1)
L1_KC5 = Keycode(smtd, 205, 0, 5, L1)
L1_KC6 = Keycode(smtd, 206, 0, 6, L1)
L1_KC7 = Keycode(smtd, 207, 0, 7, L1)
L1_KC8 = Keycode(smtd, 208, 0, 8, L1)

L2_KC0 = Keycode(smtd, 300, 0, 0, L2)
L2_KC1 = Keycode(smtd, 301, 0, 1, L2)
L2_KC2 = Keycode(smtd, 302, 0, 2, L2)
L2_KC3 = Keycode(smtd, 303, 0, 3, L2)
L2_KC4 = Keycode(smtd, 304, 0, 4, L2)
L2_KC5 = Keycode(smtd, 305, 0, 5, L2)
L2_KC6 = Keycode(smtd, 306, 0, 6, L2)
L2_KC7 = Keycode(smtd, 307, 0, 7, L2)
L2_KC8 = Keycode(smtd, 308, 0, 8, L2)

MACRO0 = Keycode(smtd, 500, 0, 0, -100)
MACRO1 = Keycode(smtd, 501, 0, 1, -100)
MACRO2 = Keycode(smtd, 502, 0, 2, -100)
MACRO3 = Keycode(smtd, 503, 0, 3, -100)
MACRO4 = Keycode(smtd, 504, 0, 4, -100)
MACRO5 = Keycode(smtd, 505, 0, 5, -100)
MACRO6 = Keycode(smtd, 506, 0, 6, -100)
MACRO7 = Keycode(smtd, 507, 0, 7, -100)
MACRO8 = Keycode(smtd, 508, 0, 8, -100)

all_keycodes = [
    L0_KC0, L0_KC1, L0_KC2, L0_KC3, L0_KC4, L0_KC5, L0_KC6, L0_KC7, L0_KC8,
    L1_KC0, L1_KC1, L1_KC2, L1_KC3, L1_KC4, L1_KC5, L1_KC6, L1_KC7, L1_KC8,
    L2_KC0, L2_KC1, L2_KC2, L2_KC3, L2_KC4, L2_KC5, L2_KC6, L2_KC7, L2_KC8,
    MACRO0, MACRO1, MACRO2, MACRO3, MACRO4, MACRO5, MACRO6, MACRO7, MACRO8,
]

# Keys

K1 = Key(smtd, 'K1', 0, 0, "no special behaviour", all_keycodes)
K2 = Key(smtd, 'K2', 0, 1, "no special behaviour", all_keycodes)
MMT = Key(smtd, 'MMT', 0, 2, "SMTD_MT_ON_MKEY(L0_KC2, MACRO2, KC_LEFT_GUI, 2)", all_keycodes)
MT1 = Key(smtd, 'MT1', 0, 3, "SMTD_MT(L*_KC3, KC_LEFT_ALT)", all_keycodes)
MT2 = Key(smtd, 'MT2', 0, 4, "SMTD_MT(L*_KC4, KC_LEFT_CTRL)", all_keycodes)
LT1 = Key(smtd, 'LT1', 0, 5, "SMTD_LT(L0_KC5, L1), SMTD_LT(L2_KC5, L3)", all_keycodes)
LT2 = Key(smtd, 'LT2', 0, 6, "SMTD_LT(L0_KC5, L2), SMTD_LT(L1_KC5, L3)", all_keycodes)
MTE = Key(smtd, 'MTE', 0, 7, "SMTD_MTE(L*_KC3, KC_LEFT_SHIFT)", all_keycodes)  # Ñ in Spanish layout,
CTRL = Key(smtd, 'CTRL', 0, 8, "KC_LEFT_CTRL", all_keycodes)

all_keys = [K1, K2, MMT, MT1, MT2, LT1, LT2, MTE, CTRL]


# Tests

class TestSmTd(unittest.TestCase):
    def setUp(self):
        """Method to run before each test"""
        reset()

    def tearDown(self):
        """Method to run after each test"""
        assert smtd.get_mods() == 0
        assert smtd.get_layer_state() == 0

        for d in smtd.get_deferred_execs():
            if d["active"]: smtd.execute_deferred(d["idx"])
        for d in smtd.get_deferred_execs():
            assert not d["active"]

        assert smtd.get_mods() == 0
        assert smtd.get_layer_state() == 0

    def assertHistory(self, *args):
        history = smtd.get_record_history()
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
        self.assertFalse(K1.press(), "process_smtd should block future key events")

        records = smtd.get_record_history()
        self.assertEqual(len(records), 1)
        self.assertEmulatePress(records[0], K1)

    def test_bypass_mode(self):
        """Test the actual bypass mode in the library"""
        smtd.set_bypass(True)
        self.assertTrue(K1.press(), "sm_td should return true in bypass mode")
        self.assertEqual(len(smtd.get_record_history()), 0)

    def test_reset(self):
        """Test the reset function"""
        K1.press()
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

        self.assertFalse(K2.release(), "release should block future key events")
        records = smtd.get_record_history()
        self.assertEqual(len(records), 2)
        self.assertEmulateRelease(records[1], K2)

    def test_basic_MT_ON_MKEY_tap(self):
        """Test the basic MT function"""
        self.assertFalse(MMT.press(), "press should block future key events")
        records = smtd.get_record_history()
        self.assertEqual(len(records), 0)

        self.assertFalse(MMT.release(), "release should return true")
        records = smtd.get_record_history()
        self.assertEqual(len(records), 2, "tap should happer after release")
        self.assertRegister(records[0], MACRO2)
        self.assertUnregister(records[1], MACRO2)

    def test_basic_MT_ON_MKEY_hold(self):
        """Test the basic MT function"""
        self.assertFalse(MMT.press(), "press should block future key events")
        self.assertEqual(len(smtd.get_record_history()), 0)
        self.assertEqual(smtd.get_mods(), 0)

        MMT.prolong()
        self.assertEqual(len(smtd.get_record_history()), 0)
        self.assertEqual(smtd.get_mods(), 8)

        self.assertFalse(MMT.release(), "release should return true")
        self.assertEqual(len(smtd.get_record_history()), 0)
        self.assertEqual(smtd.get_mods(), 0)

    def test_basic_MT_taphold(self):
        self.assertFalse(MMT.press(), "press should block future key events")
        self.assertEqual(len(smtd.get_record_history()), 0)

        self.assertFalse(MMT.release(), "release should return true")
        self.assertEqual(len(smtd.get_record_history()), 2)

        self.assertFalse(MMT.press(), "press should block future key events")
        self.assertEqual(len(smtd.get_record_history()), 2)

        self.assertEqual(smtd.get_mods(), 0)
        MMT.prolong()
        self.assertEqual(smtd.get_mods(), 8)
        self.assertFalse(MMT.release(), "release should return true")
        self.assertEqual(smtd.get_mods(), 0)

        print("\nevents:")
        records = smtd.get_record_history()
        for r in records: print(f"{r}")

        self.assertEqual(len(records), 2)
        self.assertRegister(records[0], MACRO2)
        self.assertUnregister(records[1], MACRO2)

    def test_basic_MT_taptaptap(self):
        self.assertFalse(MMT.press(), "press should block future key events")
        self.assertEqual(len(smtd.get_record_history()), 0)

        self.assertFalse(MMT.release(), "release should return true")
        self.assertEqual(len(smtd.get_record_history()), 2)

        self.assertFalse(MMT.press(), "press should block future key events")
        self.assertEqual(len(smtd.get_record_history()), 2)

        self.assertFalse(MMT.release(), "release should return true")
        self.assertEqual(len(smtd.get_record_history()), 4)

        self.assertFalse(MMT.press(), "press should block future key events")
        self.assertEqual(len(smtd.get_record_history()), 4)

        MMT.prolong()
        self.assertEqual(len(smtd.get_record_history()), 5)

        self.assertFalse(MMT.release(), "release should return true")
        self.assertEqual(len(smtd.get_record_history()), 6)

        records = smtd.get_record_history()
        self.assertRegister(records[0], MACRO2)
        self.assertUnregister(records[1], MACRO2)
        self.assertRegister(records[2], MACRO2)
        self.assertUnregister(records[3], MACRO2)
        self.assertRegister(records[4], MACRO2)
        self.assertUnregister(records[5], MACRO2)

    def test_MTE_hold(self):
        self.assertFalse(MTE.press())
        self.assertHistory()
        self.assertEqual(smtd.get_mods(), 2)

        MTE.prolong()
        self.assertHistory()
        self.assertEqual(smtd.get_mods(), 2)

        MTE.release()
        self.assertHistory()
        self.assertEqual(smtd.get_mods(), 0)

    def test_MTE_tap(self):
        self.assertFalse(MTE.press())
        self.assertHistory()
        self.assertEqual(smtd.get_mods(), 2)

        MTE.release()
        self.assertEqual(smtd.get_mods(), 0)
        self.assertHistory(
            Register(L0_KC7),
            Unregister(L0_KC7),
        )

    def test_LT_MT_KEY_DOWN__MT_LT_KEY_UP(self):
        presses = [LT1, MT1, K1]
        releases = [MT1, LT1, K1]

        for p in presses: p.press()
        for r in releases: r.release()

        print("\n\npresses:")
        for p in presses: print(f" -- {p}")
        print("\nreleases:")
        for r in releases: print(f" -- {r}")

        print("\nevents:")
        records = smtd.get_record_history()
        for r in records: print(f"{r}")

        self.assertEmulatePress(records[0], K1, layer_state=1, mods=4)
        self.assertEmulateRelease(records[1], K1, layer_state=1, mods=4)

    def test_LT_MT_KEY_DOWN__LT_MT_KEY_UP(self):
        presses = [LT1, MT1, K1]
        releases = [LT1, MT1, K1]

        for p in presses: p.press()
        for r in releases: r.release()

        print("\n\npresses:")
        for p in presses: print(f" -- {p}")
        print("\nreleases:")
        for r in releases: print(f" -- {r}")

        print("\nevents:")
        records = smtd.get_record_history()
        for r in records: print(f"{r}")

        self.assertEqual(len(records), 2)
        self.assertEmulatePress(records[0], K1, layer_state=1, mods=4)
        self.assertEmulateRelease(records[1], K1, layer_state=1, mods=4)
        print("\n\n----------------------------------------------------\n\n")

    def test_MT_TAP_MT_KEY(self):
        MMT.press()
        MMT.release()
        MMT.press()
        K1.press()
        MMT.release()
        K1.release()

        print('\n\n')
        records = smtd.get_record_history()
        for r in records: print(f"{r}")
        self.assertEqual(len(records), 4)
        self.assertRegister(records[0], MACRO2)
        self.assertUnregister(records[1], MACRO2)
        self.assertEmulatePress(records[2], K1, mods=8)
        self.assertEmulateRelease(records[3], K1, mods=8)

    def test_LT_MT_permutations(self):
        presses = [MT1, LT1]
        releases = [MT1, LT1, K1]

        # sex, hehe
        seqs = []
        for press_seq in itertools.permutations(presses):
            press_seq = press_seq + (K1,)  # A must be the last key pressed
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
            records = smtd.get_record_history()
            for r in records: print(f"{r}")

            self.assertEmulatePress(records[0], K1, layer_state=1, mods=4)
            self.assertEmulateRelease(records[1], K1, layer_state=1, mods=4)
            print("\n\n----------------------------------------------------\n\n")

    def test_LT_layer_switch(self):
        LT1.press()
        K1.press()
        K1.release()
        LT1.release()

        records = smtd.get_record_history()
        self.assertEqual(len(records), 2)
        self.assertEmulatePress(records[0], K1, layer_state=1)
        self.assertEmulateRelease(records[1], K1, layer_state=1)

    def test_instant_bypass(self):
        K1.press()
        # fixme вот тут можно было бы и отпускать процесс, а не стопорить и эмулировать нажатие
        # то есть не включать байпас, а сделать так, чтобы process_smtd просто отпускал нажатие клавиши
        K1.release()

    def test_MT_LT_key_MT_key_ordered(self):
        LT1.press()
        MT1.press()
        K1.press()
        K1.release()
        MT1.release()
        MTE.press()
        K2.press()
        K2.release()
        MTE.release()
        LT1.release()

        print("\n\n\n------------------------------------------")
        print("\n\nevents:")
        for r in smtd.get_record_history(): print(f"{r}")
        print("\n\n------------------------------------------\n\n\n")

        self.assertHistory(
            EmulatePress(K1, layer=1, mods=4),
            EmulateRelease(K1, layer=1, mods=4),
            EmulatePress(K2, layer=1, mods=2),
            EmulateRelease(K2, layer=1, mods=2),
        )

    def test_MT_LT_key_MT_key_shuffled(self):
        MT1.press()
        LT1.press()
        K1.press()
        MTE.press()
        MT1.release()
        K1.release()
        K2.press()
        K2.release()
        LT1.release()
        MTE.release()

        print("\n\n\n------------------------------------------")
        print("\n\nevents:")
        for r in smtd.get_record_history(): print(f"{r}")
        print("\n\n------------------------------------------\n\n\n")

        self.assertHistory(
            EmulatePress(K1, layer=1, mods=4),
            EmulateRelease(K1, layer=1, mods=-1),
            EmulatePress(K2, layer=1, mods=2),
            EmulateRelease(K2, layer=1, mods=2),
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
            reset()
            desc, actions = next(generator)
            for action in actions: action()

            # Check that the state is reset
            assert smtd.get_mods() == 0, f"Mods should be 0 after {desc}"
            assert smtd.get_layer_state() == 0, f"Layer state should be 0 after {desc}"

    def test_upright_mod_press(self):
        """Test that the normal mod press works"""
        CTRL.press()
        K1.press()
        K1.release()
        CTRL.release()

        self.assertHistory(
            EmulatePress(CTRL, mods=0),
            EmulatePress(K1, mods=1),
            EmulateRelease(K1, mods=1),
            EmulateRelease(CTRL, mods=1),
        )

    def test_stirred_mod_press(self):
        CTRL.press()
        K1.press()
        CTRL.release()
        K1.release()

        # since CTRL and K1 and both quickly pressed and released, K1 is assumed to be TAPPED
        # and since K1 is tapped, the virtual key press and release appears to be simultaneous
        # so that tap "rearranges" actual press-release sequence
        self.assertHistory(
            EmulatePress(CTRL, mods=0),
            EmulatePress(K1, mods=1),
            EmulateRelease(K1, mods=1),
            EmulateRelease(CTRL, mods=1),
        )

    def test_stirred_long_mod_press1(self):
        CTRL.press()
        CTRL.prolong()
        K1.press()
        CTRL.release()
        K1.release()

        self.assertHistory(
            EmulatePress(CTRL, mods=0),
            EmulatePress(K1, mods=1),
            EmulateRelease(K1, mods=1),
            EmulateRelease(CTRL, mods=1),
        )

    def test_stirred_long_mod_press2(self):
        CTRL.press()
        K1.press()
        K1.prolong()
        CTRL.release()
        K1.release()

        self.assertHistory(
            EmulatePress(CTRL, mods=0),
            EmulatePress(K1, mods=1),
            EmulateRelease(K1, mods=1),
            EmulateRelease(CTRL, mods=1),
        )

    def test_stirred_long_mod_press3(self):
        CTRL.press()
        K1.press()
        CTRL.release()
        CTRL.prolong()
        K1.prolong()
        K1.release()

        self.assertHistory(
            EmulatePress(CTRL, mods=0),
            EmulatePress(K1, mods=1),
            EmulateRelease(CTRL, mods=1),
            EmulateRelease(K1, mods=0),
        )

    def test_upright_mod_smtd_press(self):
        CTRL.press()
        MT1.press()
        MT1.release()
        CTRL.release()

        self.assertHistory(
            EmulatePress(CTRL, mods=0),
            Register(L0_KC3, mods=1),
            Unregister(L0_KC3, mods=1),
            EmulateRelease(CTRL, mods=1),
        )

    def test_stirred_mod_smtd_press(self):
        CTRL.press()
        MT1.press()
        CTRL.release()
        MT1.release()

        self.assertHistory(
            EmulatePress(CTRL, mods=0),
            Register(L0_KC3, mods=1),
            Unregister(L0_KC3, mods=1),
            EmulateRelease(CTRL, mods=1),
        )

    def test_stirred_long_mod_smtd_press1(self):
        CTRL.press()
        CTRL.prolong()
        MT1.press()
        CTRL.release()
        MT1.release()

        self.assertHistory(
            EmulatePress(CTRL, mods=0),
            Register(L0_KC3, mods=1),
            Unregister(L0_KC3, mods=1),
            EmulateRelease(CTRL, mods=1),
        )

    def test_stirred_long_mod_smtd_press2_bugged(self):
        CTRL.press()
        MT1.press()
        MT1.prolong()  # well, MT1 comes a modifier here
        CTRL.release()
        # fixme-sm now CTRL stuck in THL state, so it doesn't sent actual release right now
        # sm_td need to understand that case and skip THL state straight to TAP action and NONE state
        MT1.release()

        self.assertHistory(
            EmulatePress(CTRL, mods=0),
            EmulateRelease(CTRL, mods=1),
        )

    def test_stirred_long_mod_smtd_press2_fixed(self):
        CTRL.press()
        CTRL.prolong()
        MT1.press()
        MT1.prolong()  # well, MT1 comes a modifier here
        CTRL.release()
        CTRL.prolong()
        MT1.release()

        self.assertHistory(
            EmulatePress(CTRL, mods=0),
            EmulateRelease(CTRL, mods=-1),
        )

    def test_stirred_mod_smtd_press3(self):
        CTRL.press()
        MT1.press()
        CTRL.release()
        MT1.release()

        self.assertHistory(
            EmulatePress(CTRL, mods=0),
            Register(L0_KC3, mods=1),
            Unregister(L0_KC3, mods=-1),
            EmulateRelease(CTRL, mods=-1),
        )

    def test_stirred_long_mod_smtd_press3(self):
        CTRL.press()
        CTRL.prolong()
        MT1.press()
        CTRL.release()
        MT1.release()

        self.assertHistory(
            EmulatePress(CTRL, mods=0),
            Register(L0_KC3, mods=1),
            Unregister(L0_KC3, mods=-1),
            EmulateRelease(CTRL, mods=-1),
        )

    def test_stirred_long_mod_smtd_press4(self):
        CTRL.press()
        MT1.press()
        CTRL.prolong()
        CTRL.release()
        MT1.release()

        self.assertHistory(
            EmulatePress(CTRL, mods=0),
            Register(L0_KC3, mods=1),
            Unregister(L0_KC3, mods=-1),
            EmulateRelease(CTRL, mods=-1),
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


def reset():
    for keycode in all_keycodes:
        keycode.reset()
    for key in all_keys:
        key.reset()
    smtd.reset()


if __name__ == "__main__":
    unittest.main()
