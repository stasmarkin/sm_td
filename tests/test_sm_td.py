import unittest
import os
import sys

from tests.sm_td_bindings import *

# Add parent directory to path so we can import modules
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

# Import our bindings module
from tests import sm_td_bindings as smtd


class TestSmTd(unittest.TestCase):
    def setUp(self):
        """Method to run before each test"""
        reset()

    def test_process_smtd(self):
        """Test that process_smtd function from the actual library works"""
        self.assertFalse(Keycode.L0_KC0.press(), "process_smtd should block future key events")

        records = get_record_history()
        self.assertEqual(len(records), 1)
        self.assertEqual(records[0]["row"], 0)
        self.assertEqual(records[0]["col"], 0)
        self.assertEqual(records[0]["keycode"], 0)
        self.assertEqual(records[0]["pressed"], True)

    def test_bypass_mode(self):
        """Test the actual bypass mode in the library"""
        set_bypass(True)
        self.assertTrue(Keycode.L0_KC0.press(), "sm_td should return true in bypass mode")

        self.assertEqual(len(get_record_history()), 0)

    def test_reset(self):
        """Test the reset function"""
        Keycode.L0_KC0.press()
        records_before = get_record_history()

        reset()
        records_after = get_record_history()

        self.assertGreater(len(records_before), 0, "No records were created")
        self.assertEqual(len(records_after), 0, "Records were not cleared after reset")

    def test_generic_tap(self):
        """Test the generic tap function"""
        self.assertFalse(Keycode.L0_KC1.press(), "press should block future key events")
        records = get_record_history()
        self.assertEqual(len(records), 1)
        self.assertEqual(records[0]["row"], 0)
        self.assertEqual(records[0]["col"], 1)
        self.assertEqual(records[0]["keycode"], 0)
        self.assertEqual(records[0]["pressed"], True)

        self.assertFalse(Keycode.L0_KC1.release(), "release should block future key events")
        records = get_record_history()
        self.assertEqual(len(records), 2)
        self.assertEqual(records[1]["row"], 0)
        self.assertEqual(records[1]["col"], 1)
        self.assertEqual(records[1]["keycode"], 0)
        self.assertEqual(records[1]["pressed"], False)

    def test_basic_MT(self):
        """Test the basic MT function"""
        self.assertFalse(Keycode.L0_KC2.press(), "press should block future key events")
        records = get_record_history()
        self.assertEqual(len(records), 0)

        self.assertFalse(Keycode.L0_KC2.release(), "release should return true")
        records = get_record_history()
        self.assertEqual(len(records), 2, "tap should happer after release")
        self.assertEqual(records[0]["row"], 255)
        self.assertEqual(records[0]["col"], 255)
        self.assertEqual(records[0]["keycode"], Keycode.MACRO2.value)
        self.assertEqual(records[0]["pressed"], True)
        self.assertEqual(records[1]["row"], 255)
        self.assertEqual(records[1]["col"], 255)
        self.assertEqual(records[1]["keycode"], Keycode.MACRO2.value)
        self.assertEqual(records[1]["pressed"], False)


if __name__ == "__main__":
    unittest.main()
