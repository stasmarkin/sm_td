# QMK-native test for sm_td chordal hold: SMTD_CHORDAL_HOLD on top of
# SMTD_ENABLE_QMK_TAPHOLD, compiled against the REAL quantum pipeline
# (action.c, action_tapping.c, deferred_exec.c).

DEFERRED_EXEC_ENABLE = yes

# sm_td.h does `#include QMK_KEYBOARD_H` in non-unit-test mode; there is no keyboard
# in a test build, so point it at the umbrella quantum header.
OPT_DEFS += -DQMK_KEYBOARD_H=\"quantum.h\"

# sm_td sources are symlinked into this dir as `smtd_src` by run.sh.
VPATH += $(TEST_PATH)/smtd_src
SRC += sm_td.c
SRC += smtd_hooks.c
