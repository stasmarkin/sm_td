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
        smtd.reset()

    def test_process_smtd(self):
        """Test that process_smtd function from the actual library works"""
        self.assertFalse(Keycode.L0_KC4.press(), "process_smtd should block future key events")

        records = smtd.get_record_history()
        self.assertEqual(len(records), 1)
        self.assertEqual(records[0]["row"], 0)
        self.assertEqual(records[0]["col"], 4)
        self.assertEqual(records[0]["pressed"], True)

    def test_bypass_mode(self):
        """Test the actual bypass mode in the library"""
        smtd.set_bypass(True)

        self.assertTrue(Keycode.L0_KC0.press(), "sm_td should return true in bypass mode")

        records = smtd.get_record_history()
        self.assertEqual(len(records), 0)

    def test_reset(self):
        """Test the reset function"""
        Keycode.L0_KC0.press()
        records_before = smtd.get_record_history()

        smtd.reset()
        records_after = smtd.get_record_history()

        self.assertGreater(len(records_before), 0, "No records were created")
        self.assertEqual(len(records_after), 0, "Records were not cleared after reset")

    def test_generic_tap(self):
        """Test the generic tap function"""

        pass


if __name__ == "__main__":
    unittest.main()
