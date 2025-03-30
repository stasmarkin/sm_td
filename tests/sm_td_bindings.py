import ctypes
import os
import subprocess
import atexit
from enum import Enum


# Structure definitions
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


class DeferredExecInfo(ctypes.Structure):
    _fields_ = [
        ("delay_ms", ctypes.c_uint32),
        ("callback", ctypes.c_void_p),  # Using void pointer for function pointer
        ("cb_arg", ctypes.c_void_p),
        ("active", ctypes.c_bool),
    ]


class History(ctypes.Structure):
    _fields_ = [
        ("row", ctypes.c_uint8),
        ("col", ctypes.c_uint8),
        ("keycode", ctypes.c_uint16),
        ("pressed", ctypes.c_bool),
        ("mods", ctypes.c_uint8),
        ("layer_state", ctypes.c_uint32),
        ("smtd_bypass", ctypes.c_bool),
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


class Layer(Enum):
    L0 = 0
    L1 = 1
    L2 = 2


class Keycode(Enum):
    L0_KC0 = 100
    L0_KC1 = 101
    L0_KC2 = 102
    L0_KC3 = 103
    L0_KC4 = 104
    L0_KC5 = 105
    L0_KC6 = 106
    L0_KC7 = 107

    L1_KC0 = 200
    L1_KC1 = 201
    L1_KC2 = 202
    L1_KC3 = 203
    L1_KC4 = 204
    L1_KC5 = 205
    L1_KC6 = 206
    L1_KC7 = 207

    L2_KC0 = 300
    L2_KC1 = 301
    L2_KC2 = 302
    L2_KC3 = 303
    L2_KC4 = 304
    L2_KC5 = 305
    L2_KC6 = 306
    L2_KC7 = 307

    MACRO0 = 400
    MACRO1 = 401
    MACRO2 = 402
    MACRO3 = 403
    MACRO4 = 404
    MACRO5 = 405
    MACRO6 = 406
    MACRO7 = 407

    @classmethod
    def from_rowcol(cls, rowcol):
        return cls.from_layer_rowcol(get_layer_state(), rowcol)

    @classmethod
    def from_layer_rowcol(cls, layer, rowcol):
        return cls.from_value(100 + layer * 100 + rowcol[0] * 10 + rowcol[1])

    @classmethod
    def from_value(cls, value):
        """Get the enum member from its value"""
        for member in cls:
            if member.value == value:
                return member
        raise ValueError(f"No enum member with value {value}")

    @classmethod
    def reset(cls):
        for member in cls:
            member.__init__(member._value)

    def __init__(self, value):
        self._value = value
        self._pressed = False
        self._defer_idx = None

    def press(self):
        assert self._pressed == False
        assert get_layer_state() == self.layer()
        self._pressed = True
        result, defer_idx = process_key_and_timeout(self, True)
        self._defer_idx = defer_idx
        return result

    def release(self):
        assert self._pressed == True
        self._pressed = False
        result, defer_idx = process_key_and_timeout(self, False)
        self._defer_idx = defer_idx
        return result

    def prolong(self):
        assert self._defer_idx is not None
        execute_deferred(self._defer_idx)
        self._defer_idx = None

    def layer(self):
        """Get the layer of the keycode"""
        if self.value < 400:
            return -1 + self.value // 100
        raise "MACRO keycodes are not supported"

    # noinspection PyRedundantParentheses
    def rowcol(self):
        """Convert the keycode to row and column"""
        if self.value < 400:
            return (0, self.value % 100)
        raise "MACRO keycodes are not supported"

    def __str__(self):
        return self.name

    def __repr__(self):
        return self.name


# Compile and load the shared library
def _load_smtd_lib():
    """Compile and load the sm_td shared library"""
    project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
    lib_path = os.path.join(project_root, "libsm_td.dylib")

    # Compile command with SMTD_UNIT_TEST flag
    compile_cmd = (f"clang -shared "
                   f"-DSMTD_UNIT_TEST "
                   f"-o {lib_path} "
                   f"-fPIC {os.path.join(project_root, 'tests/test_mocks.c')} "
                   f"-I{project_root} ")

    print(f"Compiling sm_td library: {compile_cmd}")
    result = subprocess.run(compile_cmd, shell=True, stderr=subprocess.PIPE)

    if result.returncode != 0:
        print(f"Compilation failed: {result.stderr.decode()}")
        raise RuntimeError("Failed to compile sm_td library")

    # Load the compiled library
    lib = ctypes.CDLL(lib_path)

    # Register cleanup to remove the library file
    def cleanup():
        try:
            if os.path.exists(lib_path):
                os.remove(lib_path)
        except Exception as e:
            print(f"Failed to clean up shared library: {e}")

    atexit.register(cleanup)

    return lib, lib_path

# Load the library and set up function definitions
lib, lib_path = _load_smtd_lib()

# Define function prototypes
lib.process_smtd.argtypes = [ctypes.c_uint, ctypes.POINTER(KeyRecord)]
lib.process_smtd.restype = ctypes.c_bool

lib.TEST_set_smtd_bypass.argtypes = [ctypes.c_bool]
lib.TEST_set_smtd_bypass.restype = None

lib.TEST_reset.argtypes = []
lib.TEST_reset.restype = None

lib.TEST_get_record_history.argtypes = [
    ctypes.POINTER(History),  # out_records
    ctypes.POINTER(ctypes.c_uint8)  # out_count
]
lib.TEST_get_record_history.restype = None

lib.TEST_get_deferred_execs.argtypes = [
    ctypes.POINTER(DeferredExecInfo),  # out_execs
    ctypes.POINTER(ctypes.c_uint8)  # out_count
]
lib.TEST_get_deferred_execs.restype = None

lib.TEST_execute_deferred.argtypes = [ctypes.c_uint8]  # deferred_token
lib.TEST_execute_deferred.restype = None

lib.get_mods.argtypes = []  # No arguments
lib.get_mods.restype = ctypes.c_uint8  # Returns uint8_t

lib.TEST_get_layer_state.argtypes = []
lib.TEST_get_layer_state.restype = ctypes.c_uint8


# Helper functions
def create_keyrecord(row, col, pressed):
    """Helper to create a keyrecord structure"""
    record = KeyRecord()
    record.event.key.row = row
    record.event.key.col = col
    record.event.keycode = 0
    record.event.pressed = pressed
    return record


def process_key_and_timeout(keycode: Keycode, pressed: bool):
    execs_before = get_deferred_execs()
    record = create_keyrecord(0, int(keycode.name[5:]), pressed)
    record_ptr = ctypes.pointer(record)
    result = lib.process_smtd(ctypes.c_uint(keycode.value), record_ptr)
    execs_after = get_deferred_execs()
    execs_diff = execs_after[len(execs_before):]
    if len(execs_diff) is 0: return result, None
    return result, execs_diff[-1]["idx"]


def set_bypass(enabled):
    """Set the smtd_bypass flag"""
    lib.TEST_set_smtd_bypass(ctypes.c_bool(enabled))


def reset():
    """Reset the test state"""
    lib.TEST_reset()
    Keycode.reset()


def get_record_history():
    """Get the history of key records processed"""
    records = (History * 100)()  # MAX_RECORD_HISTORY is 100
    count = ctypes.c_uint8(0)
    lib.TEST_get_record_history(records, ctypes.byref(count))

    result = []
    for i in range(count.value):
        result.append({
            "row": records[i].row,
            "col": records[i].col,
            "keycode": records[i].keycode,
            "pressed": records[i].pressed,
            "mods": records[i].mods,
            "layer_state": records[i].layer_state,
            "smtd_bypass": records[i].smtd_bypass,
        })
    return result


def get_deferred_execs():
    """Get the list of deferred executions scheduled in the test environment"""
    execs_array = (DeferredExecInfo * 100)()  # MAX_DEFERRED_EXECS is 100
    count = ctypes.c_uint8(0)
    lib.TEST_get_deferred_execs(execs_array, ctypes.byref(count))

    result = []
    for i in range(count.value):
        result.append({
            "idx": i + 1,
            "delay_ms": execs_array[i].delay_ms,
            "active": execs_array[i].active
        })
    return result


def execute_deferred(idx):
    """Execute a specific deferred execution by its id"""
    assert get_deferred_execs()[idx - 1]["active"] == True
    lib.TEST_execute_deferred(ctypes.c_uint8(idx))
    assert get_deferred_execs()[idx - 1]["active"] == False


def get_mods():
    """Get the current modifier state"""
    return lib.get_mods()

def get_layer_state():
    """Get the current layer state"""
    return lib.TEST_get_layer_state()
