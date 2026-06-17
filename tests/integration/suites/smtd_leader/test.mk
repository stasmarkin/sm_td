# sm_td + real QMK Leader Key (issue #29): a leader sequence must be able to
# observe sm_td-resolved tap keys.

DEFERRED_EXEC_ENABLE = yes
LEADER_ENABLE = yes
OPT_DEFS += -DQMK_KEYBOARD_H=\"quantum.h\"

VPATH += $(TEST_PATH)/smtd_src
SRC += sm_td.c
SRC += smtd_hooks.c
