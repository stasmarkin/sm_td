"""Regression tests for sm_td layer interaction (GitHub issues #57 and #44).

Since 0.6.2 SMTD_LT activates/deactivates its layer with QMK-native
layer_on/layer_off instead of the home-grown LAYER_PUSH/LAYER_RESTORE, which did
layer_move and wiped the whole layer stack on release. These tests reproduce the
two user-facing consequences of that old behaviour and assert the fixed one.

Run against 0.6.1 (commit 6684a52) both tests fail; against 0.6.2 they pass.
"""

try:
    from tests.unit.sm_td_assertions import *
except ImportError:
    from sm_td_assertions import *

smtd = load_smtd_lib('tests/unit/layer_features/layout.c')


class TestLayerFeatures(SmTdAssertions):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.smtd = smtd

    def setUp(self):
        super().setUp()
        reset()

    def test_57_toggle_layer_survives_smtd_lt_release(self):
        """#57: a layer toggled on (TG/TO) while an SMTD_LT is held must stay on
        after the SMTD_LT is released. Old layer_move reset the whole stack and
        dropped the toggled layer."""
        # hold SMTD_LT(L0 -> L4)
        LT_C.press()
        LT_C.prolong()
        self.assertEqual(smtd.get_layer_state(), L4, "SMTD_LT hold should activate L4")

        # press a TG(L5)-style key while on L4 -> L5 toggled on, L4 still held
        TG.press()
        TG.release()
        self.assertEqual(smtd.get_layer_state(), L5, "TG should toggle L5 on")

        # release the SMTD_LT: only L4 must go away, L5 must remain
        LT_C.release()
        self.assertEqual(smtd.get_layer_state(), L5,
                         "toggled L5 must survive SMTD_LT release (regression #57)")

        # cleanup: toggle L5 back off so tearDown sees a clean layer state
        TG.press()
        TG.release()
        self.assertEqual(smtd.get_layer_state(), L0)

    def test_44_tri_layer_activates_with_two_smtd_lt(self):
        """#44: holding two SMTD_LT keys (L1 and L2) must leave both layers on so
        the tri-layer hook can light up L3. Old layer_move kept only the last
        layer, so the two never coexisted and tri-layer never fired."""
        # hold SMTD_LT(L0 -> L1)
        LT_A.press()
        LT_A.prolong()
        self.assertEqual(smtd.get_layer_state(), L1, "first SMTD_LT hold should activate L1")

        # hold SMTD_LT(L1 -> L2); L1 + L2 active -> tri-layer adds L3 (highest)
        LT_B.press()
        LT_B.prolong()
        self.assertEqual(smtd.get_layer_state(), L3,
                         "two SMTD_LT layers must coexist and trigger tri-layer L3 (regression #44)")

        # release in reverse; tri-layer L3 drops as soon as L2 leaves
        LT_B.release()
        self.assertEqual(smtd.get_layer_state(), L1, "releasing L2 should drop tri-layer L3")

        LT_A.release()
        self.assertEqual(smtd.get_layer_state(), L0)


# Layers (mirror layout.c)
L0, L1, L2, L3, L4, L5 = 0, 1, 2, 3, 4, 5

# Keycodes (mirror layout.c enum values)
LT_A_KC, LT_B_KC, LT_C_KC, TG_L5_KC = 100, 101, 102, 103

# Keycode objects: (value, row, col, layer) — one per (key position, layer) pressed
lt_a_l0 = Keycode(smtd, LT_A_KC, 0, 0, L0)
lt_b_l1 = Keycode(smtd, LT_B_KC, 0, 1, L1)
lt_c_l0 = Keycode(smtd, LT_C_KC, 0, 2, L0)
tg_l4 = Keycode(smtd, TG_L5_KC, 0, 3, L4)
tg_l5 = Keycode(smtd, TG_L5_KC, 0, 3, L5)

all_keycodes = [lt_a_l0, lt_b_l1, lt_c_l0, tg_l4, tg_l5]

# Keys
LT_A = Key(smtd, 'LT_A', 0, 0, "SMTD_LT(LT_A, L1)", all_keycodes)
LT_B = Key(smtd, 'LT_B', 0, 1, "SMTD_LT(LT_B, L2)", all_keycodes)
LT_C = Key(smtd, 'LT_C', 0, 2, "SMTD_LT(LT_C, L4)", all_keycodes)
TG = Key(smtd, 'TG', 0, 3, "TG(L5) plain key", all_keycodes)

all_keys = [LT_A, LT_B, LT_C, TG]


def reset():
    for keycode in all_keycodes:
        keycode.reset()
    for key in all_keys:
        key.reset()
    smtd.reset()


if __name__ == "__main__":
    unittest.main()
