# New Test Folder Template

Use this template to add a focused test suite for a new behavior (e.g., a tap/hold tweak or LT/MT interaction).

## Directory Layout
- Create `tests/<feature_name>/` with:
  - `layout.c` — minimal C layout + `on_smtd_action(...)` setup
  - `test.py` — Python `unittest` cases using shared helpers

## layout.c (minimal skeleton)
```c
// tests/<feature_name>/layout.c
#define SMTD_UNIT_TEST
#define MATRIX_ROWS 1
#define MATRIX_COLS 3
#define TAPPING_TERM 200
#include "../sm_td_bindings.c"

enum LAYERS { L0 = 0 };

enum KEYCODES { K_A = 100, K_B, K_C };

uint16_t const keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [L0] = { K_A, K_B, K_C },
};

smtd_resolution on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        // Example: SMTD_MT(K_A, KC_LEFT_CTRL)
    }
    return SMTD_RESOLUTION_UNHANDLED;
}

uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) { return get_smtd_timeout_default(timeout); }
bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature) { return smtd_feature_enabled_default(keycode, feature); }
```

## test.py (minimal skeleton)
```python
# tests/<feature_name>/test.py
import unittest
from tests.sm_td_assertions import *

smtd = load_smtd_lib('tests/<feature_name>/layout.c')

class TestFeature(SmTdAssertions):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.smtd = smtd

    def setUp(self):
        super().setUp()
        reset()

    def test_basic(self):
        # Example: press/release a key and assert
        # self.assertFalse(KeyA.press())
        # self.assertFalse(KeyA.release())
        pass

if __name__ == '__main__':
    unittest.main()
```

## Run
- Single suite: `python3 -m unittest tests/<feature_name>/test.py`
- All tests: `python3 tests/run_tests.py`

Notes
- Tests auto-compile with `clang -shared -fPIC` into a temporary `.dylib` (macOS) or `.so` (Linux).
- Reuse helpers from `tests/sm_td_assertions.py` for clear, consistent assertions.
