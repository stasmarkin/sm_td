# Repository Guidelines

## Project Structure & Module Organization
- `sm_td/`: Core C module for QMK (`sm_td.c`, `sm_td.h`, `qmk_module.json`).
- `tests/`: Python `unittest` suites compile and call the C library via `ctypes` and `clang` (see `tests/sm_td_bindings.py`). Feature folders (e.g., `caps_word_enable/`) contain `layout.c` and `test.py`.
- `docs/`: User docs and customization guides.
- `keyboards/`: Example QMK integration assets.

## Architecture
- Entrypoint: `process_smtd(...)` interprets tap vs. hold and schedules deferred actions.
- Hooks: implement `on_smtd_action(...)` and configure `SMTD_GLOBAL_*` macros for timing/features.
- Tests: Python drives C via `ctypes`; timeouts simulated with `prolong()` in key helpers.
- Key output ("pipeline taps", since #23 fix): `SMTD_TAP_16`/`SMTD_REGISTER_16`/`SMTD_UNREGISTER_16` call `smtd_tap_code16`/`smtd_register_code16`/`smtd_unregister_code16`. When the sent key equals the keymap keycode at the pressed position (and `use_cl != false`), the key goes through `smtd_emulate_key` -> full `process_record()` pipeline (Caps Word, combos, etc. see it); otherwise a manual `process_caps_word()` shim runs before a direct `tap_code16` send. Controlled by `SMTD_GLOBAL_PIPELINE_TAPS` (default true) and per-key `SMTD_FEATURE_PIPELINE_TAPS`.
- `smtd_executing_state` tracks the state whose action is running (set in `smtd_execute_action`); `state->emulated_register` pairs an emulated register with an emulated release even if the layer changed in between. `smtd_emulate_key` saves/restores `smtd_bypass` because it can be called while bypass is already set.

## Build, Test, and Development Commands
- Run all tests: `python3 tests/run_tests.py`
- Run a suite: `python3 -m unittest tests/caps_word_enable/test.py`
- Discover: `python3 -m unittest discover tests -p 'test*.py'`
Notes:
- Tests auto-compile a per-suite shared library with `clang -shared -fPIC` (macOS: `.dylib`, Linux: `.so`). No manual `cmake` step is needed for tests.
- Prereqs: Python 3.10+, `clang` on PATH.

## Coding Style & Naming Conventions
- C11 for `sm_td`; Python for tests.
- C: snake_case for functions (`process_smtd`), ALL_CAPS for macros (`SMTD_*`), K&R braces, 4-space indents (prefer spaces in new code).
- Headers: keep QMK includes behind guards; use `static` for internal symbols; prefix public API with `smtd_`/`SMTD_`.

## Testing Guidelines
- Framework: Python `unittest` driving the C library via `ctypes`.
- Structure: each feature folder has `layout.c` and `test_*.py`; use helpers in `tests/sm_td_assertions.py`.
- Naming: discovered by `test*.py`; keep scenario names descriptive.
- Determinism: seed randomness in tests that sample (e.g., `random.seed(…)` in `setUp`) to stabilize failures.
- Known baseline failures (pre-existing): `test_stirred_long_mod_smtd_press2_fixed` in `caps_word_enable` and `complex_layout` (4 FAILs total) - prolong/defer assertion issue, unrelated to recent work.
- Mocks (`tests/sm_td_bindings.c`) mirror QMK essentials, including Caps Word state and weak mods; suites with `CAPS_WORD_ENABLE` must define `caps_word_press_user` in their `layout.c`. History `mods` field records `current_mods | weak_mods`; emulated `process_record` events are recorded with `keycode=65535` plus row/col.
- macOS dylib linking rejects `__attribute__((weak))` *undefined* function references - use `#ifdef` selection instead of weak defaults inside the single-TU test build.

## Real-Hardware Testing (owner's setup)
- Firmware repo: `../qmk_firmware_zsa` (keyboard `zsa/voyager`, keymap `sm`); `modules/stasmarkin/sm_td/` there is symlinked to this repo's `sm_td/`, so builds pick up local changes directly.
- Build/flash: `cd ../qmk_firmware_zsa && just build` / `just flash` (DFU). That keymap enables CAPS_WORD, COMBO, REPEAT_KEY, UNICODE - a good integration stress test.

## Known Issues & Backlog
- Divergence from QMK (intentional, YAGNI): real QMK turns Caps Word off when a non-shift mod-tap is held; sm_td's MT hold (`register_mods`) stays invisible to Caps Word.
- `fixme-sm` in `smtd_emulate_key`: emulating keycodes absent from the keymap could use combo-style records (`record->keycode`, requires `COMBO_ENABLE`/`REPEAT_KEY_ENABLE`).
- Issue #45 (dynamic timeouts): replace fixed `SMTD_TIMEOUT_RELEASE` with a rhythm-derived threshold (`p3 << p2`, ratio `SMTD_GLOBAL_RELEASE_RATIO`). PR #54 attempts this but is broken (absolute timestamps instead of durations, leaked deferred execs, swapped callbacks, C++ in C debug macro, no tests) - do not merge as-is.
- Useful: `gh issue view <n> --repo stasmarkin/sm_td --comments` to read issue threads.

## Commit & Pull Request Guidelines
- Commits: concise, imperative; use conventional types (`feat:`, `fix:`, `docs:`, `test:`, `refactor:`, `build:`, `chore:`). Example: `feat(mt): eager mod-tap handling`.
- PRs include: linked issue, rationale, before/after behavior, updated docs (if user-facing), and test commands/suites run locally.
- Always run `python3 tests/run_tests.py` before submitting.

## Configuration Tips
- In QMK: set `DEFERRED_EXEC_ENABLE = yes` and define `MAX_DEFERRED_EXECUTORS` per docs.
- Tune via `SMTD_GLOBAL_*` and `on_smtd_action(...)`; document per-key overrides in `docs/`.
