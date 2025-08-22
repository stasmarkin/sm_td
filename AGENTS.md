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

## Commit & Pull Request Guidelines
- Commits: concise, imperative; use conventional types (`feat:`, `fix:`, `docs:`, `test:`, `refactor:`, `build:`, `chore:`). Example: `feat(mt): eager mod-tap handling`.
- PRs include: linked issue, rationale, before/after behavior, updated docs (if user-facing), and test commands/suites run locally.
- Always run `python3 tests/run_tests.py` before submitting.

## Configuration Tips
- In QMK: set `DEFERRED_EXEC_ENABLE = yes` and define `MAX_DEFERRED_EXECUTORS` per docs.
- Tune via `SMTD_GLOBAL_*` and `on_smtd_action(...)`; document per-key overrides in `docs/`.
