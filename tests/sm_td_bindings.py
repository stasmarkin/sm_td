import ctypes
import os
import subprocess
import atexit
from typing import Dict, List, Optional, Tuple, Any


# Structure definitions
class CKeyPosition(ctypes.Structure):
    _fields_ = [
        ("row", ctypes.c_uint8),
        ("col", ctypes.c_uint8)
    ]


class CKeyEvent(ctypes.Structure):
    _fields_ = [
        ("key", CKeyPosition),
        ("pressed", ctypes.c_bool)
    ]


class CKeyRecord(ctypes.Structure):
    _fields_ = [
        ("event", CKeyEvent)
    ]

def create_ckeyrecord(row: int, col: int, pressed: bool) -> CKeyRecord:
    """Helper to create a keyrecord structure"""
    record = CKeyRecord()
    record.event.key.row = row
    record.event.key.col = col
    record.event.keycode = 0
    record.event.pressed = pressed
    return record

class CDeferredExecInfo(ctypes.Structure):
    _fields_ = [
        ("delay_ms", ctypes.c_uint32),
        ("callback", ctypes.c_void_p),  # Using void pointer for function pointer
        ("cb_arg", ctypes.c_void_p),
        ("active", ctypes.c_bool),
    ]


class CHistory(ctypes.Structure):
    _fields_ = [
        ("row", ctypes.c_uint8),
        ("col", ctypes.c_uint8),
        ("keycode", ctypes.c_uint16),
        ("pressed", ctypes.c_bool),
        ("mods", ctypes.c_uint8),
        ("layer_state", ctypes.c_uint32),
        ("smtd_bypass", ctypes.c_bool),
    ]



class Keycode:
    def __init__(self, smtd, value, row, col, layer):
        self.smtd = smtd
        self.value = value
        self.row = row
        self.col = col
        self.layer = layer
        self.pressed = False
        self.defer_idx = None

    def reset(self):
        self.pressed = False
        self.defer_idx = None

    def press(self):
        assert self.pressed == False
        assert self.smtd.get_layer_state() == self.layer
        self.pressed = True
        result, defer_idx = self.smtd.process_key_and_timeout(self, True)
        self.defer_idx = defer_idx
        return result

    def release(self):
        assert self.pressed == True
        self.pressed = False
        result, defer_idx = self.smtd.process_key_and_timeout(self, False)
        self.defer_idx = defer_idx
        return result

    def prolong(self):
        assert self.defer_idx is not None
        self.smtd.execute_deferred(self.defer_idx)
        self.defer_idx = None

    def try_prolong(self):
        if self.defer_idx is None: return
        self.smtd.execute_deferred(self.defer_idx, False)
        self.defer_idx = None

    def __str__(self):
        return self.value

    def __repr__(self):
        return self.value


class Key:
    def __init__(self, smtd, name, row, col, comment, all_keycodes):
        self.smtd = smtd
        self.name = name
        self.row = row
        self.col = col
        self.comment = comment
        self.all_keycodes = all_keycodes
        self.pressed = None
        self.released = None

    def reset(self):
        if self.pressed: self.pressed.reset()
        if self.released: self.released.reset()
        self.pressed = None
        self.released = None

    def press(self):
        assert self.pressed is None
        self.released = None
        self.pressed = self.current_keycode()
        return self.pressed.press()

    def current_keycode(self):
        layer = self.smtd.get_layer_state()
        for keycode in self.all_keycodes:
            if keycode.row == self.row and keycode.col == self.col and keycode.layer == layer:
                return keycode
        raise ValueError(f"No keycode for {self} on layer {layer}")

    def release(self):
        assert self.pressed is not None
        assert self.released is None
        result = self.pressed.release()
        self.released = self.pressed
        self.pressed = None
        return result

    def prolong(self):
        if self.pressed: return self.pressed.prolong()
        if self.released: return self.released.prolong()
        raise "both pressed and released are None"

    def try_prolong(self):
        if self.pressed: return self.pressed.try_prolong()
        if self.released: return self.released.try_prolong()
        raise "both pressed and released are None"

    def __repr__(self):
        return self.__str__()

    def __str__(self):
        return f"Key.{self.name} # {self.comment}"

class SmtdBindings:
    """Encapsulates all SMTD library bindings and functions"""

    def __init__(self, lib: ctypes.CDLL):
        self.lib = lib

    def process_key_and_timeout(self, keycode: Keycode, pressed: bool) -> Tuple[bool, Optional[int]]:
        execs_before = self.get_deferred_execs()
        record_ptr = ctypes.pointer(create_ckeyrecord(keycode.row, keycode.col, pressed))
        result = self.lib.process_smtd(ctypes.c_uint(keycode.value), record_ptr)
        execs_after = self.get_deferred_execs()
        execs_diff = execs_after[len(execs_before):]
        if len(execs_diff) == 0:
            return result, None
        return result, execs_diff[-1]["idx"]

    def set_bypass(self, enabled: bool) -> None:
        """Set the smtd_bypass flag"""
        self.lib.TEST_set_smtd_bypass(ctypes.c_bool(enabled))

    def reset(self) -> None:
        """Reset the test state"""
        self.lib.TEST_reset()

    def get_record_history(self) -> List[Dict[str, Any]]:
        """Get the history of key records processed"""
        records = (CHistory * 100)()  # MAX_RECORD_HISTORY is 100
        count = ctypes.c_uint8(0)
        self.lib.TEST_get_record_history(records, ctypes.byref(count))

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

    def get_deferred_execs(self) -> List[Dict[str, Any]]:
        """Get the list of deferred executions scheduled in the test environment"""
        execs_array = (CDeferredExecInfo * 100)()  # MAX_DEFERRED_EXECS is 100
        count = ctypes.c_uint8(0)
        self.lib.TEST_get_deferred_execs(execs_array, ctypes.byref(count))

        result = []
        for i in range(count.value):
            result.append({
                "idx": i + 1,
                "delay_ms": execs_array[i].delay_ms,
                "active": execs_array[i].active
            })
        return result

    def execute_deferred(self, idx: int, make_asserts: bool = True) -> None:
        """Execute a specific deferred execution by its id"""
        if make_asserts:
            assert self.get_deferred_execs()[idx - 1]["active"] == True
        if not self.get_deferred_execs()[idx - 1]["active"]:
            return
        self.lib.TEST_execute_deferred(ctypes.c_uint8(idx))
        assert self.get_deferred_execs()[idx - 1]["active"] == False

    def get_mods(self) -> int:
        """Get the current modifier state"""
        return self.lib.get_mods()

    def get_layer_state(self) -> int:
        """Get the current layer state"""
        return self.lib.TEST_get_layer_state()


# Compile and load the shared library
def load_smtd_lib(path: str) -> SmtdBindings:
    """Compile and load the sm_td shared library"""
    project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
    lib_path = os.path.join(project_root, "libsm_td.dylib")

    compile_cmd = (f"clang -shared "
                   f"-o {lib_path} "
                   f"-fPIC {os.path.join(project_root, path)} "
                   f"-I{project_root} ")

    print(f"Compiling sm_td library: {compile_cmd}")
    result = subprocess.run(compile_cmd, shell=True, stderr=subprocess.PIPE)

    if result.returncode != 0:
        print(f"Compilation failed: {result.stderr.decode()}")
        raise RuntimeError("Failed to compile sm_td library")

    # Load the compiled library
    lib: ctypes.CDLL = ctypes.CDLL(lib_path)

    # Register cleanup to remove the library file
    def cleanup() -> None:
        try:
            if os.path.exists(lib_path):
                os.remove(lib_path)
        except Exception as e:
            print(f"Failed to clean up shared library: {e}")

    atexit.register(cleanup)

    lib.process_smtd.argtypes = [ctypes.c_uint, ctypes.POINTER(CKeyRecord)]
    lib.process_smtd.restype = ctypes.c_bool

    lib.TEST_set_smtd_bypass.argtypes = [ctypes.c_bool]
    lib.TEST_set_smtd_bypass.restype = None

    lib.TEST_reset.argtypes = []
    lib.TEST_reset.restype = None

    lib.TEST_get_record_history.argtypes = [
        ctypes.POINTER(CHistory),  # out_records
        ctypes.POINTER(ctypes.c_uint8)  # out_count
    ]
    lib.TEST_get_record_history.restype = None

    lib.TEST_get_deferred_execs.argtypes = [
        ctypes.POINTER(CDeferredExecInfo),  # out_execs
        ctypes.POINTER(ctypes.c_uint8)  # out_count
    ]
    lib.TEST_get_deferred_execs.restype = None

    lib.TEST_execute_deferred.argtypes = [ctypes.c_uint8]  # deferred_token
    lib.TEST_execute_deferred.restype = None

    lib.get_mods.argtypes = []  # No arguments
    lib.get_mods.restype = ctypes.c_uint8  # Returns uint8_t

    lib.TEST_get_layer_state.argtypes = []
    lib.TEST_get_layer_state.restype = ctypes.c_uint8

    return SmtdBindings(lib)
