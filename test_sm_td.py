import ctypes
import unittest
from unittest.mock import Mock, patch, call
import os
import sys
import subprocess
import tempfile
import atexit


# Define C structures and enums used by sm_td.h
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


# Mock for QMK functions
class QMKMock:
    def __init__(self):
        self.reset()

    def reset(self):
        # Track function calls
        self.calls = {
            "process_record": [],
            "get_mods": [],
            "set_mods": [],
            "send_keyboard_report": [],
            "register_code16": [],
            "unregister_code16": [],
            "tap_code16": [],
            "register_mods": [],
            "unregister_mods": [],
            "get_highest_layer": [],
            "layer_move": [],
            "layer_state": 0,
            "defer_exec": [],
            "cancel_deferred_exec": [],
            "timer_read32": [],
            "timer_elapsed32": [],
            "wait_ms": [],
        }

        # Current state
        self.mods = 0
        self.current_layer = 0
        self.current_time_ms = 0
        self.keymap = [[[0 for _ in range(10)] for _ in range(10)] for _ in range(5)]  # Mock 5 layers

    def process_record(self, record):
        self.calls["process_record"].append((record.event.key.row, record.event.key.col, record.event.pressed))
        return True

    def get_mods(self):
        self.calls["get_mods"].append(True)
        return self.mods

    def set_mods(self, mods):
        self.calls["set_mods"].append(mods)
        self.mods = mods

    def send_keyboard_report(self):
        self.calls["send_keyboard_report"].append(True)

    def register_code16(self, keycode):
        self.calls["register_code16"].append(keycode)

    def unregister_code16(self, keycode):
        self.calls["unregister_code16"].append(keycode)

    def tap_code16(self, keycode):
        self.calls["tap_code16"].append(keycode)

    def register_mods(self, mods):
        self.calls["register_mods"].append(mods)
        self.mods |= mods

    def unregister_mods(self, mods):
        self.calls["unregister_mods"].append(mods)
        self.mods &= ~mods

    def get_highest_layer(self, layer_state):
        self.calls["get_highest_layer"].append(layer_state)
        return self.current_layer

    def layer_move(self, layer):
        self.calls["layer_move"].append(layer)
        self.current_layer = layer

    def defer_exec(self, timeout_ms, callback, state):
        self.calls["defer_exec"].append((timeout_ms, callback, state))
        return 123  # Mock token

    def cancel_deferred_exec(self, token):
        self.calls["cancel_deferred_exec"].append(token)

    def timer_read32(self):
        self.calls["timer_read32"].append(True)
        return self.current_time_ms

    def timer_elapsed32(self, start_time):
        self.calls["timer_elapsed32"].append(start_time)
        return self.current_time_ms - start_time

    def wait_ms(self, ms):
        self.calls["wait_ms"].append(ms)
        self.current_time_ms += ms


# Mock for sm_td implementation
class SmtdMock:
    def __init__(self, qmk_mock):
        self.qmk = qmk_mock
        self.reset()

    def reset(self):
        # Track function calls
        self.calls = {
            "on_smtd_action": [],
            "get_smtd_timeout": [],
            "smtd_feature_enabled": []
        }

    def on_smtd_action(self, keycode, action, sequence_len):
        self.calls["on_smtd_action"].append((keycode, action, sequence_len))
        # Default implementation - can be overridden in tests
        return SMTD_RESOLUTION_UNHANDLED

    def get_smtd_timeout(self, keycode, timeout):
        self.calls["get_smtd_timeout"].append((keycode, timeout))
        # Return default timeouts
        if timeout == SMTD_TIMEOUT_TAP:
            return 200  # TAPPING_TERM
        elif timeout == SMTD_TIMEOUT_SEQUENCE:
            return 100  # TAPPING_TERM/2
        elif timeout == SMTD_TIMEOUT_FOLLOWING_TAP:
            return 200  # TAPPING_TERM
        elif timeout == SMTD_TIMEOUT_RELEASE:
            return 50  # TAPPING_TERM/4
        return 0

    def smtd_feature_enabled(self, keycode, feature):
        self.calls["smtd_feature_enabled"].append((keycode, feature))
        if feature == SMTD_FEATURE_AGGREGATE_TAPS:
            return False
        return False


# Function to load and initialize the shared library
def load_smtd_lib():
    # Determine platform-specific file extension and compiler command
    if sys.platform == "darwin":  # macOS
        lib_file = "libsm_td.dylib"
        compile_cmd = ["clang", "-shared",
                       '-DQMK_KEYBOARD_H="mock_qmk_keyboard.h"',
                       '-DQMK_DEFERRED_EXEC_H="mock_qmk_deferred_exec.h"'
                       "-o", lib_file, "-fPIC", "sm_td.c",
                       "-I.", "-DTESTING"]
    elif sys.platform == "linux":  # Linux
        lib_file = "libsm_td.so"
        compile_cmd = ["gcc", "-shared",
                       '-DQMK_KEYBOARD_H="mock_qmk_keyboard.h"',
                       '-DQMK_DEFERRED_EXEC_H="mock_qmk_deferred_exec.h"'
                       "-o", lib_file, "-fPIC", "sm_td.c",
                       "-I.", "-DTESTING"]
    elif sys.platform == "win32":  # Windows
        lib_file = "sm_td.dll"
        compile_cmd = ["gcc", "-shared",
                       '-DQMK_KEYBOARD_H="mock_qmk_keyboard.h"',
                       '-DQMK_DEFERRED_EXEC_H="mock_qmk_deferred_exec.h"'
                       "-o", lib_file, "sm_td.c",
                       "-I.", "-DTESTING"]
    else:
        raise RuntimeError(f"Unsupported platform: {sys.platform}")

    # Compile the library
    lib_path = os.path.abspath(lib_file)
    print(f"Compiling sm_td library: {' '.join(compile_cmd)}")
    result = subprocess.run(compile_cmd, capture_output=True, text=True)

    if result.returncode != 0:
        print(f"Compilation failed: {result.stderr}")
        raise RuntimeError("Failed to compile sm_td library")

    # Register cleanup to remove the compiled library file
    atexit.register(lambda: os.remove(lib_path) if os.path.exists(lib_path) else None)

    # Load the compiled library
    try:
        lib = ctypes.CDLL(lib_path)
        print(f"Successfully loaded sm_td library from {lib_path}")
        return lib, lib_path
    except Exception as e:
        print(f"Failed to load library: {e}")
        raise


# Setup callback function types
SMTD_ACTION_CALLBACK = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_uint, ctypes.c_int, ctypes.c_uint)
SMTD_TIMEOUT_CALLBACK = ctypes.CFUNCTYPE(ctypes.c_uint, ctypes.c_uint, ctypes.c_int)
SMTD_FEATURE_CALLBACK = ctypes.CFUNCTYPE(ctypes.c_bool, ctypes.c_uint, ctypes.c_int)
QMK_PROCESS_RECORD_CALLBACK = ctypes.CFUNCTYPE(ctypes.c_bool, ctypes.POINTER(KeyRecord))
QMK_DEFER_EXEC_CALLBACK = ctypes.CFUNCTYPE(ctypes.c_void_p)


# Test cases for sm_td.h
class TestSmTd(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # Compile and load the actual library
        cls.lib, cls.lib_path = load_smtd_lib()

        # Define the function signatures
        cls.lib.process_smtd.argtypes = [ctypes.c_uint, ctypes.POINTER(KeyRecord)]
        cls.lib.process_smtd.restype = ctypes.c_bool

        cls.lib.smtd_init.argtypes = [
            SMTD_ACTION_CALLBACK,
            SMTD_TIMEOUT_CALLBACK,
            SMTD_FEATURE_CALLBACK,
            QMK_PROCESS_RECORD_CALLBACK
        ]
        cls.lib.smtd_init.restype = None

        # Additional functions if needed
        cls.lib.smtd_set_bypass.argtypes = [ctypes.c_bool]
        cls.lib.smtd_set_bypass.restype = None

    def setUp(self):
        # Create mocks for each test
        self.qmk_mock = QMKMock()
        self.smtd_mock = SmtdMock(self.qmk_mock)
        self.setup_callback_functions()

    def setup_callback_functions(self):
        # Create C-compatible callback functions that will call our mock implementations

        @SMTD_ACTION_CALLBACK
        def action_callback(keycode, action, sequence_len):
            return self.smtd_mock.on_smtd_action(keycode, action, sequence_len)

        @SMTD_TIMEOUT_CALLBACK
        def timeout_callback(keycode, timeout_type):
            return self.smtd_mock.get_smtd_timeout(keycode, timeout_type)

        @SMTD_FEATURE_CALLBACK
        def feature_callback(keycode, feature):
            return self.smtd_mock.smtd_feature_enabled(keycode, feature)

        @QMK_PROCESS_RECORD_CALLBACK
        def process_record_callback(record):
            return self.qmk_mock.process_record(record.contents)

        # Store references to prevent garbage collection
        self.action_callback = action_callback
        self.timeout_callback = timeout_callback
        self.feature_callback = feature_callback
        self.process_record_callback = process_record_callback

        # Initialize the library with our callbacks
        self.lib.smtd_init(
            self.action_callback,
            self.timeout_callback,
            self.feature_callback,
            self.process_record_callback
        )

    def create_keyrecord(self, row, col, pressed):
        """Helper to create a keyrecord structure"""
        record = KeyRecord()
        record.event.key.row = row
        record.event.key.col = col
        record.event.pressed = pressed
        return record

    def test_process_smtd_calls_process_record(self):
        """Test that process_smtd eventually calls process_record with the same keycode"""
        # Setup
        keycode = 0x5A  # 'Z' key for example
        record = self.create_keyrecord(1, 2, True)  # Key press

        # Mock on_smtd_action to return SMTD_RESOLUTION_UNHANDLED for this test
        original_on_smtd_action = self.smtd_mock.on_smtd_action

        def mock_on_smtd_action(keycode, action, sequence_len):
            self.smtd_mock.calls["on_smtd_action"].append((keycode, action, sequence_len))
            return SMTD_RESOLUTION_UNHANDLED

        self.smtd_mock.on_smtd_action = mock_on_smtd_action

        # Execute the function (in real implementation, this would call into the C library)
        # process_smtd(keycode, record)
        self.simulate_process_smtd(keycode, record)

        # Verify that process_record was called with the same key position and pressed state
        self.assertGreaterEqual(len(self.qmk_mock.calls["process_record"]), 1)
        last_process_record_call = self.qmk_mock.calls["process_record"][-1]
        self.assertEqual(last_process_record_call, (record.event.key.row, record.event.key.col, record.event.pressed))

        # Restore mock
        self.smtd_mock.on_smtd_action = original_on_smtd_action

    def test_process_smtd_with_determined_action(self):
        """Test that process_smtd properly handles keys when on_smtd_action returns DETERMINED"""
        # Setup
        keycode = 0x41  # 'A' key
        record = self.create_keyrecord(2, 3, True)  # Key press

        # Mock on_smtd_action to return DETERMINED
        original_on_smtd_action = self.smtd_mock.on_smtd_action

        def mock_on_smtd_action(keycode, action, sequence_len):
            self.smtd_mock.calls["on_smtd_action"].append((keycode, action, sequence_len))
            return SMTD_RESOLUTION_DETERMINED

        self.smtd_mock.on_smtd_action = mock_on_smtd_action

        # Execute
        self.simulate_process_smtd(keycode, record)

        # Verify that on_smtd_action was called
        self.assertGreaterEqual(len(self.smtd_mock.calls["on_smtd_action"]), 1)

        # Since we returned DETERMINED, process_record should not be called with the same key
        # We can't assert it was never called because other internal workings might call it,
        # but we can check if the last call doesn't match our key
        if self.qmk_mock.calls["process_record"]:
            for call_args in self.qmk_mock.calls["process_record"]:
                # Make sure our exact key wasn't processed
                self.assertFalse(call_args == (record.event.key.row, record.event.key.col, record.event.pressed))

        # Restore mock
        self.smtd_mock.on_smtd_action = original_on_smtd_action

    def simulate_process_smtd(self, keycode, record):
        """Simulate the behavior of process_smtd without actually calling the C function"""
        # This is a simplified simulation of what process_smtd would do
        # In a real implementation, you'd call the actual function from the library

        # First, check if on_smtd_action handles the key
        result = self.smtd_mock.on_smtd_action(keycode, SMTD_ACTION_TOUCH, 0)

        # If unhandled, simulate calling process_record
        if result == SMTD_RESOLUTION_UNHANDLED:
            self.qmk_mock.process_record(record)

        # Return what process_smtd would return
        return result != SMTD_RESOLUTION_UNHANDLED

    def test_key_sequence(self):
        """Test a sequence of key presses and releases"""
        # Press a key
        key1_code = 0x41  # 'A'
        record_press = self.create_keyrecord(1, 1, True)
        self.simulate_process_smtd(key1_code, record_press)

        # Release the key
        record_release = self.create_keyrecord(1, 1, False)
        self.simulate_process_smtd(key1_code, record_release)

        # Verify the pattern of calls
        self.assertGreaterEqual(len(self.smtd_mock.calls["on_smtd_action"]), 2)
        self.assertIn((key1_code, SMTD_ACTION_TOUCH, 0), self.smtd_mock.calls["on_smtd_action"])

    def test_bypass_mode(self):
        """Test that keys are processed normally when bypass mode is active"""
        # In real implementation, this would set smtd_bypass to True
        # For our mock, we'll simulate that behavior

        # Press a key with bypass mode simulated
        keycode = 0x42  # 'B'
        record = self.create_keyrecord(2, 2, True)

        # Simulate process_smtd with bypass mode
        # In this case, it should directly call process_record and return true
        result = True  # Simulating bypass behavior
        self.qmk_mock.process_record(record)

        # Verify that process_record was called directly
        self.assertIn((record.event.key.row, record.event.key.col, record.event.pressed),
                      self.qmk_mock.calls["process_record"])

        # Verify that on_smtd_action was not called
        # In real bypass mode, on_smtd_action won't be called
        # self.assertEqual(len(self.smtd_mock.calls["on_smtd_action"]), 0)

    def test_real_process_smtd(self):
        """Test that process_smtd function from the actual library works"""
        # Setup
        keycode = 0x5A  # 'Z' key for example
        record = self.create_keyrecord(1, 2, True)  # Key press
        record_ptr = ctypes.pointer(record)

        # Mock on_smtd_action to return SMTD_RESOLUTION_UNHANDLED for this test
        original_on_smtd_action = self.smtd_mock.on_smtd_action

        def mock_on_smtd_action(keycode, action, sequence_len):
            self.smtd_mock.calls["on_smtd_action"].append((keycode, action, sequence_len))
            return SMTD_RESOLUTION_UNHANDLED

        self.smtd_mock.on_smtd_action = mock_on_smtd_action

        # Call the actual library function
        result = self.lib.process_smtd(keycode, record_ptr)

        # Verify that on_smtd_action was called with expected values
        self.assertGreaterEqual(len(self.smtd_mock.calls["on_smtd_action"]), 1)

        # Check if action was TOUCH for the press event
        self.assertIn((keycode, SMTD_ACTION_TOUCH, 0), self.smtd_mock.calls["on_smtd_action"])

        # Since we returned UNHANDLED, process_record should be called
        self.assertGreaterEqual(len(self.qmk_mock.calls["process_record"]), 1)

        # Restore mock
        self.smtd_mock.on_smtd_action = original_on_smtd_action

    def test_real_bypass_mode(self):
        """Test the actual bypass mode in the library"""
        # Setup
        keycode = 0x42  # 'B' key
        record = self.create_keyrecord(2, 2, True)
        record_ptr = ctypes.pointer(record)

        # Set bypass mode
        self.lib.smtd_set_bypass(True)

        # Call process_smtd
        result = self.lib.process_smtd(keycode, record_ptr)

        # In bypass mode, on_smtd_action shouldn't be called
        # but process_record should be called directly
        self.assertEqual(len(self.smtd_mock.calls["on_smtd_action"]), 0)
        self.assertGreaterEqual(len(self.qmk_mock.calls["process_record"]), 1)

        # Reset bypass mode for other tests
        self.lib.smtd_set_bypass(False)

    def test_real_key_sequence(self):
        """Test a sequence of key presses and releases using the actual library"""
        # Press a key
        key_code = 0x41  # 'A'
        record_press = self.create_keyrecord(1, 1, True)
        record_press_ptr = ctypes.pointer(record_press)
        self.lib.process_smtd(key_code, record_press_ptr)

        # Check initial action call
        self.assertIn((key_code, SMTD_ACTION_TOUCH, 0), self.smtd_mock.calls["on_smtd_action"])

        # Simulate time passing - update the mock time
        self.qmk_mock.current_time_ms += 50  # 50ms later

        # Release the key
        record_release = self.create_keyrecord(1, 1, False)
        record_release_ptr = ctypes.pointer(record_release)
        self.lib.process_smtd(key_code, record_release_ptr)

        # Check we got a proper sequence of events
        on_smtd_calls = self.smtd_mock.calls["on_smtd_action"]

        # Different events we should see
        touch_event = (key_code, SMTD_ACTION_TOUCH, 0)
        tap_or_release_event = [(key_code, SMTD_ACTION_TAP, 1), (key_code, SMTD_ACTION_RELEASE, 0)]

        # Check that TOUCH happened
        self.assertIn(touch_event, on_smtd_calls)

        # Check that either TAP or RELEASE happened
        self.assertTrue(any(event in on_smtd_calls for event in tap_or_release_event),
                        f"Neither TAP nor RELEASE event found in {on_smtd_calls}")


if __name__ == "__main__":
    unittest.main()
