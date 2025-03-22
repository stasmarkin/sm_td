import unittest
import os
import sys

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
        keycode = 0x5A  # 'Z' key for example
        
        result = smtd.process_key(keycode, 1, 2, True)  # Key press
        self.assertFalse(result, "process_smtd should block future key events")

        records = smtd.get_record_history()
        self.assertEqual(len(records), 1)
        self.assertEqual(records[0]["row"], 1)
        self.assertEqual(records[0]["col"], 2)
        self.assertEqual(records[0]["pressed"], True)


    def test_bypass_mode(self):
        """Test the actual bypass mode in the library"""
        keycode = 0x42  # 'B' key
        smtd.set_bypass(True)

        result = smtd.process_key(keycode, 2, 2, True)
        self.assertTrue(result, "sm_td should return true in bypass mode")

        records = smtd.get_record_history()
        self.assertEqual(len(records), 0)

    def test_reset(self):
        """Test the reset function"""
        key_code = 0x41  # 'A'
        smtd.process_key(key_code, 1, 1, True)
        records_before = smtd.get_record_history()

        smtd.reset()
        records_after = smtd.get_record_history()
        
        self.assertGreater(len(records_before), 0, "No records were created")
        self.assertEqual(len(records_after), 0, "Records were not cleared after reset")


if __name__ == "__main__":
    unittest.main()
