import ctypes
import unittest
from unittest.mock import Mock, patch, call
import os
import sys
import subprocess
import tempfile
import atexit


class KeyPosition(ctypes.Structure):
    _fields_ = [
        ("row", ctypes.c_uint8),
        ("col", ctypes.c_uint8)
    ]

class KeyEvent(ctypes.Structure):
    _fields_ = [
        ("key", KeyPosition),
        ("pressed", ctypes.c_bool)
    ]

class KeyRecord(ctypes.Structure):
    _fields_ = [
        ("event", KeyEvent)
    ]

# Enum Values
SMTD_ACTION_TOUCH = 0
SMTD_ACTION_TAP = 1
SMTD_ACTION_HOLD = 2
SMTD_ACTION_RELEASE = 3

SMTD_RESOLUTION_UNCERTAIN = 0
SMTD_RESOLUTION_UNHANDLED = 1
SMTD_RESOLUTION_DETERMINED = 2

SMTD_TIMEOUT_TAP = 0
SMTD_TIMEOUT_SEQUENCE = 1
SMTD_TIMEOUT_FOLLOWING_TAP = 2
SMTD_TIMEOUT_RELEASE = 3

SMTD_FEATURE_AGGREGATE_TAPS = 0


# Function to load and initialize the shared library
def load_smtd_lib():
    """Compile and load the sm_td shared library"""
    lib_path = os.path.join(os.path.dirname(__file__), "libsm_td.dylib")

    # Compile command with SMTD_UNIT_TEST flag
    compile_cmd = (f"clang -shared "
                   f"-DSMTD_UNIT_TEST "
                   f"-o {lib_path} "
                   f"-fPIC sm_td.c "
                   f"-I. ")

    print(f"Compiling sm_td library: {compile_cmd}")
    result = subprocess.run(compile_cmd, shell=True, stderr=subprocess.PIPE)

    if result.returncode != 0:
        print(f"Compilation failed: {result.stderr.decode()}")
        raise RuntimeError("Failed to compile sm_td library")

    # Load the compiled library
    lib = ctypes.CDLL(lib_path)
    return lib, lib_path


# Test cases for sm_td.h
class TestSmTd(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # Compile and load the actual library
        cls.lib, cls.lib_path = load_smtd_lib()

        cls.lib.process_smtd.argtypes = [ctypes.c_uint, ctypes.POINTER(KeyRecord)]
        cls.lib.process_smtd.restype = ctypes.c_bool

        cls.lib.TEST_set_smtd_bypass.argtypes = [ctypes.c_bool]
        cls.lib.TEST_set_smtd_bypass.restype = None
        
        cls.lib.TEST_reset.argtypes = []
        cls.lib.TEST_reset.restype = None
        
        cls.lib.TEST_get_record_history.argtypes = [
            ctypes.POINTER(KeyRecord),  # out_records
            ctypes.POINTER(ctypes.c_uint8)  # out_count
        ]
        cls.lib.TEST_get_record_history.restype = None

    @staticmethod
    def create_keyrecord(row, col, pressed):
        """Helper to create a keyrecord structure"""
        record = KeyRecord()
        record.event.key.row = row
        record.event.key.col = col
        record.event.pressed = pressed
        return record

    def setUp(self):
        """Method to run before each test"""
        self.lib.TEST_reset()
        
    def test_process_smtd(self):
        """Test that process_smtd function from the actual library works"""
        # Setup
        keycode = 0x5A  # 'Z' key for example
        record = self.create_keyrecord(1, 2, True)  # Key press
        record_ptr = ctypes.pointer(record)

        # Call the actual library function
        result = self.lib.process_smtd(keycode, record_ptr)
        self.assertFalse(result, "process_smtd should block future key events")

        records = (KeyRecord * 50)()
        count = ctypes.c_uint8()
        self.lib.TEST_get_record_history(records, ctypes.byref(count))

        self.assertEquals(count.value, 1)
        self.assertEquals(records[0].event.key.row, 1)
        self.assertEquals(records[0].event.key.col, 2)
        self.assertEquals(records[0].event.pressed, True)


    def test_bypass_mode(self):
        """Test the actual bypass mode in the library"""
        # Setup
        keycode = 0x42  # 'B' key
        record = self.create_keyrecord(2, 2, True)
        record_ptr = ctypes.pointer(record)
        
        # Set the smtd_bypass variable to True directly
        # The variable needs to be defined as a global in the shared library
        self.lib.TEST_set_smtd_bypass(ctypes.c_bool(True))

        # Call process_smtd
        result = self.lib.process_smtd(keycode, record_ptr)
        
        # Reset bypass for other tests
        self.lib.TEST_set_smtd_bypass(ctypes.c_bool(False))
        
        self.assertTrue(result, "sm_td should return true in bypass mode")

        records = (KeyRecord * 50)()
        count = ctypes.c_uint8()
        self.lib.TEST_get_record_history(records, ctypes.byref(count))

        self.assertEquals(count.value, 0)

    def test_key_sequence(self):
        """Test a sequence of key presses and releases using the actual library"""
        # Press a key
        key_code = 0x41  # 'A'
        record_press = self.create_keyrecord(1, 1, True)
        record_press_ptr = ctypes.pointer(record_press)
        self.lib.process_smtd(key_code, record_press_ptr)

        # Release the key
        record_release = self.create_keyrecord(1, 1, False)
        record_release_ptr = ctypes.pointer(record_release)
        self.lib.process_smtd(key_code, record_release_ptr)

        # Get record history
        records = (KeyRecord * 50)()
        count = ctypes.c_uint8()
        self.lib.TEST_get_record_history(records, ctypes.byref(count))
        
        # Check the record history
        self.assertGreater(count.value, 0, "No records were saved")
        
        # Different events we should see
        touch_event = (key_code, SMTD_ACTION_TOUCH, 0)
        tap_or_release_event = [(key_code, SMTD_ACTION_TAP, 1), (key_code, SMTD_ACTION_RELEASE, 0)]

        on_smtd_calls = []  # fixme

        # Check that TOUCH happened
        self.assertIn(touch_event, on_smtd_calls)

        # Check that either TAP or RELEASE happened
        self.assertTrue(any(event in on_smtd_calls for event in tap_or_release_event),
                        f"Neither TAP nor RELEASE event found in {on_smtd_calls}")
                        
    def test_reset(self):
        """Test the TEST_reset function"""
        # First create some records
        key_code = 0x41  # 'A'
        record = self.create_keyrecord(1, 1, True)
        record_ptr = ctypes.pointer(record)
        self.lib.process_smtd(key_code, record_ptr)
        
        # Get record count
        records = (KeyRecord * 50)()
        count_before = ctypes.c_uint8()
        self.lib.TEST_get_record_history(records, ctypes.byref(count_before))
        
        # Reset
        self.lib.TEST_reset()
        
        # Check that records were cleared
        count_after = ctypes.c_uint8()
        self.lib.TEST_get_record_history(records, ctypes.byref(count_after))
        
        self.assertGreater(count_before.value, 0, "No records were created")
        self.assertEqual(count_after.value, 0, "Records were not cleared after reset")

if __name__ == "__main__":
    unittest.main()
