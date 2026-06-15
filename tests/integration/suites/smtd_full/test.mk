# Full sm_td engine against the real QMK pipeline: a rich on_smtd_action exercising
# SMTD_MT / SMTD_MTE / SMTD_LT / SMTD_TD / SMTD_TK / SMTD_TTO.

DEFERRED_EXEC_ENABLE = yes
OPT_DEFS += -DQMK_KEYBOARD_H=\"quantum.h\"

VPATH += $(TEST_PATH)/smtd_src
SRC += sm_td.c
SRC += smtd_hooks.c
