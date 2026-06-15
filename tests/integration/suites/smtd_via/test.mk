# sm_td against a REAL VIA/Vial dynamic keymap (Option B): the suite reaches the
# genuine QMK chain keymap_key_to_keycode -> keycode_at_keymap_location ->
# dynamic_keymap_get_keycode -> emulated EEPROM, and writes keycodes at runtime
# via dynamic_keymap_set_keycode(). Proves sm_td resolves keycodes live from the
# dynamic keymap (so GUI remaps are picked up), not from the static keymaps[][].
#
# Requires two idempotent guards run.sh applies to the gitignored checkout:
#   - SMTD_USE_DYNAMIC_KEYMAP compiles out the harness's own keymap_key_to_keycode
#     override (test_fixture.cpp) so this suite's override (smtd_hooks.c) wins.
#   - an #ifndef guard around the test-harness TOTAL_EEPROM_BYTE_COUNT (default 32,
#     too small for a real dynamic keymap) so we can raise it below.

DEFERRED_EXEC_ENABLE = yes
DYNAMIC_KEYMAP_ENABLE = yes
SEND_STRING_ENABLE = yes

OPT_DEFS += -DQMK_KEYBOARD_H=\"quantum.h\"
OPT_DEFS += -DSMTD_USE_DYNAMIC_KEYMAP
OPT_DEFS += -DDYNAMIC_KEYMAP_ENABLE
OPT_DEFS += -DDYNAMIC_KEYMAP_LAYER_COUNT=2
OPT_DEFS += -DNUM_ENCODERS=0
OPT_DEFS += -DTOTAL_EEPROM_BYTE_COUNT=1024

VPATH += $(TEST_PATH)/smtd_src
SRC += sm_td.c
SRC += smtd_hooks.c

# DYNAMIC_KEYMAP_ENABLE=yes already pulls dynamic_keymap.c + nvm_dynamic_keymap.c
# into the build via QMK's feature rules — do NOT add them to SRC (duplicate symbols).
