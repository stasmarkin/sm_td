# Repository Guidelines

## Project Structure & Module Organization
- `sm_td/`: Core C module for QMK (`sm_td.c`, `sm_td.h`, `qmk_module.json`).
- `tests/`: Python `unittest` suites compile and call the C library via `ctypes` and `clang` (see `tests/sm_td_bindings.py`). Feature folders (e.g., `caps_word_enable/`) contain `layout.c` and `test.py`.
- `docs/`: User docs and customization guides.
- `keyboards/`: Example QMK integration assets.

## Architecture
- Entrypoint: `process_smtd(...)` interprets tap vs. hold and schedules deferred actions.
- Hooks: implement `on_smtd_action(...)` and configure `SMTD_GLOBAL_*` macros for timing/features.
- Tests: Python drives C via `ctypes`; timeouts simulated with `prolong()` in key helpers (event-driven) or with the virtual clock `smtd.wait(ms)` (time-driven, required for dynamic timeouts).
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
- Virtual clock (since #45 work): `mock_time_ms` in `tests/sm_td_bindings.c` backs `timer_read32`/`timer_elapsed32`; `defer_exec` records `deadline_ms`; `TEST_advance_time(ms)` / Python `smtd.wait(ms)` advances time and fires due deferred execs in deadline order. The clock stands still unless `wait()` is called, so legacy `prolong()`-driven tests are unaffected. NB: `CDeferredExecInfo` in `sm_td_bindings.py` must mirror the C struct layout exactly (array stride), update both together.
- One test.py may load several layouts (`load_smtd_lib` hashes the path per dylib) - see `tests/dynamic_release/test.py` with four layouts: ratio 5 (SMTD_MT + SMTD_LT + per-key release-term cap), ratio 0 fallback, ratio 1 clamp, and SMTD_ENABLE_QMK_TAPHOLD with raw MT()/LT() under dynamic timings. Also covers 3-key rolls (each state takes p1/p2 from its immediate follower) and multi-tap sequences (p1 measured from the latest touch; sequence expiry resets tap_count).
- `layout_fixed.c`/`layout_ratio1.c` are sed-generated snapshots of `layout.c` (see git history for the command) - regenerate them when editing `layout.c`.

## Real-Hardware Testing (owner's setup)
- Firmware repo: `../qmk_firmware_zsa` (keyboard `zsa/voyager`, keymap `sm`); `modules/stasmarkin/sm_td/` there is symlinked to this repo's `sm_td/`, so builds pick up local changes directly.
- Build/flash: `cd ../qmk_firmware_zsa && just build` / `just flash` (DFU). That keymap enables CAPS_WORD, COMBO, REPEAT_KEY, UNICODE - a good integration stress test.

## Known Issues & Backlog
- Divergence from QMK (intentional, YAGNI): real QMK turns Caps Word off when a non-shift mod-tap is held; sm_td's MT hold (`register_mods`) stays invisible to Caps Word.
- `fixme-sm` in `smtd_emulate_key`: emulating keycodes absent from the keymap could use combo-style records (`record->keycode`, requires `COMBO_ENABLE`/`REPEAT_KEY_ENABLE`).
- Issue #45 (dynamic timeouts) IMPLEMENTED (2026-06-13, unreleased): `smtd_compute_release_term` derives the touch-release window as `min(p1, p2) / SMTD_GLOBAL_RELEASE_RATIO` (default 5), clamped to [1ms .. per-key SMTD_TIMEOUT_RELEASE]; ratio 0 restores the fixed timeout. Owner explicitly chose the min(p1,p2) "AND" rule (hold iff p3 << p1 AND p3 << p2): in rolls the release rhythm mirrors the press rhythm (p3 ~= p1), so p1 acts as a roll veto. Accepted trade-off: the combo-like case `p1 ~= p3 << p2` resolves as tap-tap (switching to a p2-only rule = one-line change in that function). PR #54 attempted this and is broken - do not merge.
- Useful: `gh issue view <n> --repo stasmarkin/sm_td --comments` to read issue threads.
- Issue #40 (VIA/Vial support) research (2026-06-13): with `SMTD_ENABLE_QMK_TAPHOLD` sm_td already works with VIA/Vial-remapped MT()/LT() — dynamic keymap stores plain 16-bit keycodes and overrides `keymap_key_to_keycode()` (which sm_td.c uses for stack resolution), so GUI remaps are picked up with no recompile. Custom sm_td keycodes must be based at `QK_KB_0` (0x7E00, 32 named slots) + a per-keyboard `customKeycodes` array in via JSON / vial.json to show names in the GUI (SAFE_RANGE/QK_USER codes still work but display as hex via the "Any" key). Runtime timeout config would need SMTD_GLOBAL_* `#define`s converted to runtime vars + VIA v3 custom menu (`via_custom_value_command_kb`, per-keyboard JSON); Vial's QMK Settings tab is NOT third-party extensible (requires upstreaming into vial-qmk + vial-gui, as Getreuer did for Chordal Hold in Vial 0.7.4). No community tap-hold lib has GUI integration today; Vial's own dynamic Tap Dance is admitted by Vial docs to be poor for HRM.
- QMK-native testing research (2026-06-13, deep-research + local exploration of `../qmk_firmware_zsa`): integrating sm_td into QMK's REAL googletest harness is feasible with an additive overlay, no QMK fork needed. Key facts: (a) `builddefs/testlist.mk` auto-discovers full tests via `find tests/ -name test.mk`, so dropping `tests/<name>/{test.mk,config.h,*.cpp}` into a qmk_firmware checkout registers a suite; (b) `test.mk` can do `SRC += .../sm_td.c` + `VPATH +=` — `build_full_test.mk` compiles it together with the REAL `$(QUANTUM_SRC)` (action.c, action_tapping.c, caps_word, deferred_exec) into a native host gcc/g++ executable (`PLATFORM:=TEST`, GDB-debuggable); (c) test API: `TestFixture` (`set_keymap`, `idle_for(ms)`, `run_one_scan_loop`), `KeymapKey(layer, col, row, kc)` (NB: col before row), gmock `TestDriver` + `EXPECT_REPORT` asserts real HID reports; closest precedent: `tests/tap_hold_configurations/chordal_hold/` (Getreuer's PR #24560); (d) BLOCKER: community-module codegen (`community_modules.c/h`, introspection) lives only in `build_keyboard.mk` — `build_test.mk` has zero module support, so module hooks (`*_sm_td` callbacks via qmk_module.json) won't auto-wire in tests; needs manual glue/stubs or calling `process_smtd` directly from a test shim; (e) docs.qmk.fm/unit_testing is stale (~2017 text \"full integration test not yet possible\" + references to pre-builddefs file layout) — trust repo code, not docs; (f) CI matrix over QMK versions = checkout tags + symlink overlay into `tests/` + `make test:<name>`; test_common API drifts across versions (KeymapKey/chordal-hold era changes), pre-0.28 tags have no module system at all; (g) no known precedent of a community module tested inside the QMK harness — sm_td would likely be first.
- CAVEAT for SMTD_ENABLE_QMK_TAPHOLD on real firmware (2026-06-13, unverified on hardware): QMK's own action_tapping intercepts MT()/LT() BEFORE process_record_user (quantum/action.c:133-134 — pre_process_record_quantum, then action_tapping_process; process_record(&tapping_key) fires only after QMK's own tap/hold resolution). So sm_td receives MT/LT events time-compressed and pre-resolved: release-rhythm timing is destroyed, holds get double latency (TAPPING_TERM + sm_td TAP_TERM). Tests don't catch this (bindings call process_record directly, no action_tapping mock); owner's voyager keymap has no MT()/LT(). Fixes: document `NO_ACTION_TAPPING` as a requirement (action.c then calls process_record directly), or intercept MT/LT in pre_process_record (NB: v0.5.5 moved module install OFF pre_process_record for a reason — check before reverting).

## Commit & Pull Request Guidelines
- Commits: concise, imperative; use conventional types (`feat:`, `fix:`, `docs:`, `test:`, `refactor:`, `build:`, `chore:`). Example: `feat(mt): eager mod-tap handling`.
- PRs include: linked issue, rationale, before/after behavior, updated docs (if user-facing), and test commands/suites run locally.
- Always run `python3 tests/run_tests.py` before submitting.

## Configuration Tips
- In QMK: set `DEFERRED_EXEC_ENABLE = yes` and define `MAX_DEFERRED_EXECUTORS` per docs.
- Tune via `SMTD_GLOBAL_*` and `on_smtd_action(...)`; document per-key overrides in `docs/`.
