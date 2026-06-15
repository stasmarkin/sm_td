# Dynamic release-term behavior: roll rhythm decides tap-tap vs hold-tap.
DEFERRED_EXEC_ENABLE = yes
OPT_DEFS += -DQMK_KEYBOARD_H=\"quantum.h\"
VPATH += $(TEST_PATH)/smtd_src
SRC += sm_td.c
SRC += smtd_hooks.c
