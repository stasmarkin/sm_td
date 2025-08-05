import unittest
try:
    from sm_td_bindings import *
except ImportError:
    from tests.sm_td_bindings import *
from dataclasses import dataclass


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


class SmTdAssertions(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.smtd = None

    def setUp(self):
        """Method to run before each test"""
        super().setUp()

    def tearDown(self):
        """Method to run after each test"""
        super().tearDown()

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

    def assertEvent(self, event, row=255, col=255, keycode_value=65535, pressed=True, mods=0, layer_state=0,
                    smtd_bypass=True):
        self.assertEqual(event["row"], row, f"{event} doesn't match row={row}")
        self.assertEqual(event["col"], col, f"{event} doesn't match col={col}")
        self.assertEqual(event["keycode"], keycode_value, f"{event} doesn't match keycode_value={keycode_value}")
        self.assertEqual(event["pressed"], pressed, f"{event} doesn't match pressed={pressed}")
        if (mods >= 0): self.assertEqual(event["mods"], mods, f"{event} doesn't match mods={mods}")
        if (layer_state >= 0): self.assertEqual(event["layer_state"], layer_state,
                                                f"{event} doesn't match layer_state={layer_state}")
        self.assertEqual(event["smtd_bypass"], smtd_bypass, f"{event} doesn't match smtd_bypass={smtd_bypass}")

    def assertRegister(self, event, keycode, mods=0, layer_state=0, smtd_bypass=True):
        self.assertEvent(event, keycode_value=keycode.value, pressed=True, mods=mods,
                         layer_state=layer_state, smtd_bypass=smtd_bypass)

    def assertUnregister(self, event, keycode, mods=0, layer_state=0, smtd_bypass=True):
        self.assertEvent(event, keycode_value=keycode.value, pressed=False, mods=mods,
                         layer_state=layer_state, smtd_bypass=smtd_bypass)

    def assertEmulatePress(self, event, key, mods=0, layer_state=0):
        self.assertEvent(event, row=key.row, col=key.col, pressed=True, mods=mods, layer_state=layer_state,
                         smtd_bypass=True)

    def assertEmulateRelease(self, event, key, mods=0, layer_state=0):
        self.assertEvent(event, row=key.row, col=key.col, pressed=False, mods=mods, layer_state=layer_state,
                         smtd_bypass=True)