# Contributing to SM_TD (with or without an AI agent)

This guide explains how to set up the project, what the testing levels are, what
documentation a change must carry, and how to drive an AI coding agent so it can
contribute cleanly. It is self-contained: you should be able to go from a fresh
clone to a reviewable pull request using only this file.

`SM_TD` is a QMK user library (a community module) written in C11, with its test
suites written in Python and C++. There is no firmware to flash to use the test
loop — almost everything runs natively on your host machine.

> **Deep internals live in [`AGENTS.md`](AGENTS.md).** This file is the
> *contribution workflow*; `AGENTS.md` is the *engineering reference* (state
> machine, pipeline taps, chordal hold, known issues, crash write-ups, release
> process). When a section below says "see `AGENTS.md`", that is where the
> authoritative detail is.

---

## 1. Prerequisites

| Tool | Why | Notes |
|------|-----|-------|
| **Python 3.10+** | Unit-test layer (`ctypes` driver) | Must be on `PATH` |
| **`clang`** | Compiles `sm_td.c` into a per-suite shared library for the Python tests | macOS ships it; on Linux install `clang` |
| **`just`** | Task runner that wraps every build/test command | <https://github.com/casey/just> |
| **`make` + a C++ toolchain** | QMK-native integration tests (googletest) | Only needed for the `qmk` test layer |
| **`git`**, **`gh`** | Version control and GitHub operations | `gh` is used by the release flow |

You do **not** need a keyboard, a QMK toolchain install, or a flashed board to
develop and test. Real-hardware testing (level 4 below) is optional and only the
maintainer's setup is documented.

---

## 2. Get oriented: repository layout

```
sm_td/                     Core C module (the shipped artifact)
  sm_td.c / sm_td.h        Engine + public API and macros
  introspection.h          Version block (kept in lockstep with sm_td.c/.h)
  qmk_module.json          QMK community-module metadata
tests/
  run_tests.py             Python unit-test runner (discovers tests/unit/**)
  unit/                    Level 1: fast ctypes suites (one folder per feature)
    sm_td_bindings.{c,py}  QMK mocks + virtual clock that back the unit tests
    sm_td_assertions.py    Shared assertion helpers (Key, Register, EmulatePress…)
  integration/             Level 2: QMK-native googletest suites
    run.sh / fetch.sh      Download a real qmk_firmware and run a suite
    suites/smtd_*/         One overlay per suite (test.mk, config.h, *.cpp …)
    README.md              How the native harness is wired
docs/                      User documentation (numbered 000–090, see §5)
justfile                   Entry point for all build/test commands
.github/workflows/ci.yml   CI (runs the Python unit layer on macOS + Linux)
AGENTS.md                  Engineering reference (read this for internals)
README.md                  Project intro, install options, version roadmap
```

A change almost always touches `sm_td/` **and** at least one suite under
`tests/`. Doc-only and test-only changes are valid too.

---

## 3. Build and run

There is no standalone binary to build — `SM_TD` is consumed by QMK. "Building"
in this repo means compiling the engine inside one of the test harnesses, which
the test commands do for you.

* **Unit layer** auto-compiles `sm_td.c` with `clang -shared -fPIC` into a
  temporary `.dylib` (macOS) / `.so` (Linux) per suite. No manual step.
* **Integration layer** compiles `sm_td.c` together with a real `qmk_firmware`
  checkout into a native googletest executable via `make`.

How the module is consumed downstream (for context — you don't do this to test):

* As a **QMK community module** (recommended for users): `qmk_module.json` wires
  `process_record_sm_td`; requires `DEFERRED_EXEC_ENABLE = yes` and
  `MAX_DEFERRED_EXECUTORS` defined. See `README.md` and
  `docs/010_manual_installation_guide.md`.
* **Manually**: copy `sm_td.{c,h}` into a keymap, add `SRC += sm_td.c` and the
  deferred-exec settings, and call `process_smtd()` first in
  `process_record_user()`.

---

## 4. Testing levels

`SM_TD` has four layers of verification, from fastest/cheapest to slowest. Pick
the lowest layer that can prove your change, but a behavioral change to the
engine should land regression coverage at **level 1 and/or level 2**.

### Level 0 — Static / compile

The library is plain C11. The unit and integration layers both recompile the
engine, so a compile error surfaces the moment you run any test. There is no
separate lint step. Keep public headers warning-clean.

### Level 1 — Python unit tests (fast, the default loop)

`unittest` suites in `tests/unit/<feature>/` drive `process_smtd` directly via
`ctypes` against hand-written QMK mocks. Each feature folder holds:

* `layout.c` — a minimal mock keymap + `on_smtd_action(...)` defining the keys
  under test (built on top of `tests/unit/sm_td_bindings.c`).
* `test.py` — `unittest` cases using the helpers in
  `tests/unit/sm_td_assertions.py`.

Timeouts are simulated two ways: event-driven via `prolong()` in the key
helpers, or time-driven via the **virtual clock** `smtd.wait(ms)` (required for
dynamic-timeout tests — the clock only advances when you call `wait()`).

Run them:

```sh
just test python                                   # all unit suites
python3 tests/run_tests.py                          # same, directly
python3 -m unittest tests.unit.caps_word_enable.test  # one suite
```

This is the loop to run on **every** change — it is seconds-fast and is the only
layer CI runs. Always run it before opening a PR.

> **Known baseline failures:** 4 pre-existing FAILs
> (`test_stirred_long_mod_smtd_press2_fixed` in `caps_word_enable` and
> `complex_layout`) are a known prolong/defer assertion issue, unrelated to new
> work. Don't treat those as a regression you caused — but don't add more.

To add a unit suite, see `docs/090_test_template.md` (note: real suites live
under `tests/unit/<feature>/` and import from `tests.unit.sm_td_assertions`).

### Level 2 — QMK-native integration tests (real pipeline)

`tests/integration/suites/smtd_*/` compile `sm_td.c` against a **real,
unmodified `qmk/qmk_firmware`** checkout and run it through QMK's own googletest
harness (`action.c`, `action_tapping.c`, `deferred_exec.c`, layer state, etc.).
This is the layer that proves `SM_TD` behaves correctly inside the genuine
quantum pipeline — Caps Word visibility, dynamic keymaps (VIA/Vial), external
mods, chordal hold, and so on.

Run them (first run prompts for a checkout directory, ~2 GB per QMK version,
saved to gitignored `.env`):

```sh
just test qmk                      # all suites, default QMK version (0.33.5)
just test qmk full                 # one suite (smtd_full); smtd_ prefix optional
just test qmk 0.32.16              # all suites against a specific QMK version
just test qmk 0.32.16 full         # version + suite
just test qmk-matrix               # all pinned versions × all suites
just configure                     # change the QMK checkout directory
sh tests/integration/run.sh 0.33.5 smtd_full   # the raw script (respects SMTD_QMK_DIR)
```

Run this layer when your change touches the engine's interaction with QMK
itself, or when you add a suite. Two harness rules are easy to trip over:

* **Every test that touches the keyboard MUST declare a `TestDriver driver;`**
  live for the whole test body. Omitting it dangles QMK's host-driver pointer
  and crashes in `led_task` — this masquerades as an engine segfault. (See the
  crash write-up in `AGENTS.md`.)
* Each fixture must call `smtd_reset()` in `SetUp()` — `SM_TD` keeps global
  runtime state the QMK fixture does not clear between tests.

The suite layout and wiring are documented in
`tests/integration/README.md`. CI does **not** run this layer (it needs a QMK
checkout), so run it locally and state in the PR that you did.

### Level 3 — Version matrix

`just test qmk-matrix` runs the integration suites against every pinned QMK
version (see `QMK_VERSIONS` in the `justfile`). Run this when a change could be
sensitive to QMK API drift (the `test_common` API changes across versions).

### Level 4 — Real hardware (maintainer's setup, optional)

Documented in `AGENTS.md`: a `zsa/voyager` keymap with `CAPS_WORD`, `COMBO`,
`REPEAT_KEY`, and `UNICODE` enabled, with the module symlinked in for live
builds. Use this for changes that can't be faithfully reproduced in the
harnesses (e.g. action_tapping timing under `SMTD_ENABLE_QMK_TAPHOLD`). Most
contributors will not have this and should say so in the PR.

### Which level for which change

| Change | Run at minimum |
|--------|----------------|
| Docs only | none (proofread) |
| Engine behavior / new macro / bug fix | **level 1** regression test + `just test python`; add **level 2** if it involves the QMK pipeline |
| QMK-pipeline interaction (Caps Word, layers, VIA, external mods, chordal hold) | **level 1** + **level 2** |
| Anything touching version-sensitive QMK APIs | add **level 3** |
| Timing/latency that the harnesses can't model | note it, and **level 4** if you have hardware |

---

## 5. Documentation requirements

Code and docs ship together. Decide which of these your change touches and
update them in the same PR.

* **`docs/` (user-facing, numbered 000–090).** If your change is user-visible —
  a new macro, config flag, timeout knob, or behavior — update the matching
  guide:
  * new/changed customization macro or `on_smtd_action` behavior →
    `docs/050_customization.md` and an example in `docs/060_customization_examples.md`
  * timeout/release-term knobs → `docs/070_customization_timeouts.md`
  * feature flags (`SMTD_GLOBAL_*`, per-key features) → `docs/080_customization_features.md`
  * new limitation or divergence from QMK → `docs/030_known_problems.md`
  * install/upgrade impact → `docs/010_manual_installation_guide.md` /
    `docs/020_upgrade_instructions.md`
* **`AGENTS.md` (engineering reference).** Update it when you change internals,
  add a test suite, discover a non-obvious gotcha, or resolve/leave an open
  issue. This is where future contributors (and AI agents) learn *why* things
  are the way they are — keep it current.
* **`README.md`.** Only for feature-level or roadmap-level changes (the version
  bullet list is maintained during release, not per PR).
* **`docs/015_releases.md` (changelog) and version bumps.** These belong to the
  **release process**, not ordinary PRs. A release bumps `Version:`/`Date:` in
  **all three** of `sm_td/sm_td.c`, `sm_td/sm_td.h`, `sm_td/introspection.h` in
  lockstep, prepends a changelog entry, and moves the `(we are here)` marker in
  `README.md`. Do not bump versions in a feature PR unless you are cutting the
  release. Full steps are in `AGENTS.md` → *Release Process*.

Comment style for code: explain **why**, not **what**; keep public headers
minimal; mark owner-review items as `fixme-sm:`.

---

## 6. Coding style and conventions

* **Language:** C11 for `sm_td/`; Python 3.10+ for the unit tests; C++ for the
  integration suites.
* **Naming:** `snake_case` functions (`process_smtd`), `ALL_CAPS` macros
  (`SMTD_*`), public API prefixed `smtd_`/`SMTD_`, internal symbols `static`.
* **Formatting:** K&R braces, 4-space indents, spaces (not tabs) in new code.
* **Headers:** keep QMK includes behind guards; keep the public surface small.
* **macOS caveat:** the single-translation-unit test builds reject *undefined*
  weak function references — use `#ifdef` selection instead of a weak default in
  test layouts (see `AGENTS.md`).

---

## 7. Commit and pull request guidelines

* **Commits:** concise, imperative, conventional types —
  `feat:`, `fix:`, `docs:`, `test:`, `refactor:`, `build:`, `chore:`.
  Example: `feat(mt): eager mod-tap handling`.
* **Branch:** work on a feature branch; releases land via a `release-X.Y.Z`
  branch (see `AGENTS.md`).
* **Read the issue thread** when fixing a reported bug:
  `gh issue view <n> --repo stasmarkin/sm_td --comments`.

### PR checklist

- [ ] `just test python` is green (modulo the 4 known baseline FAILs).
- [ ] Added/updated a **level 1** regression test for behavioral changes.
- [ ] Ran **level 2** (`just test qmk`) if the change touches the QMK pipeline,
      and said so in the PR (CI does not run it).
- [ ] Updated the relevant `docs/` page(s) for user-visible changes.
- [ ] Updated `AGENTS.md` for internal changes / new gotchas / new suites.
- [ ] **No** version bumps or changelog edits unless cutting a release.
- [ ] PR body has: linked issue, rationale, before/after behavior, and the test
      commands/suites you ran locally.

---

## 8. Working with an AI coding agent

This project is deliberately set up so an AI agent can contribute without a human
re-explaining the codebase each time. To get good results:

1. **Point the agent at `AGENTS.md` first.** It is the curated context: layout,
   architecture, build/test commands, conventions, known issues, and detailed
   write-ups of past investigations. Most "how does X work / why is it like
   this" questions are already answered there. This `CONTRIBUTING.md` gives the
   workflow; `AGENTS.md` gives the depth.

2. **Make the agent run the test loop, not just edit.** The fast feedback loop is
   `just test python` (seconds). For pipeline-level changes, have it run
   `just test qmk <suite>`. An agent that edits without running the suite is not
   done. Treat a green run as the bar for "working", and require the agent to
   paste the command output.

3. **Demand a regression test with every behavioral change.** Use
   `docs/090_test_template.md` as the scaffold for a level-1 suite, or copy the
   nearest existing `tests/unit/<feature>/` or `tests/integration/suites/smtd_*/`
   suite. A change without a test that would have failed before it is incomplete.

4. **Feed the issue thread.** For a reported bug, give the agent the output of
   `gh issue view <n> --repo stasmarkin/sm_td --comments` so it fixes the actual
   reported behavior, not a guess.

5. **Watch the known traps.** When an agent works in the integration layer,
   verify it declared `TestDriver driver;` and called `smtd_reset()` — these two
   omissions produce crashes that *look* like engine bugs and have already
   burned real investigation time (documented in `AGENTS.md`). When it works in
   the unit layer, watch for the `#ifdef`-vs-weak-symbol macOS caveat.

6. **Keep docs in the same change.** Ask the agent to update the matching
   `docs/` page and `AGENTS.md` as part of the diff, per §5 — don't let it defer
   documentation to a follow-up.

7. **Verify, don't trust.** If the agent claims a suite name, file, flag, or QMK
   version, confirm it against the repo — names and counts drift. The PR
   checklist in §7 is the gate before merge regardless of who (or what) wrote the
   code.

---

## 9. Release process

Cutting releases is a maintainer task with its own checklist (version lockstep
across three files, changelog, `README.md` marker, tag, `gh release create` with
`sm_td.c`/`sm_td.h` attached as assets). It is documented in full in
`AGENTS.md` → *Release Process*. Ordinary contributions should **not** include
release commits.
