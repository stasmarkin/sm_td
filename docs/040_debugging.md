Preconditions:
- you need QMK version v0.22 or higher
- you must have `quantum/logging/print.h` file in QMK firmware


Instructions:
1. Add `CONSOLE_ENABLE = yes` and `COMMAND_ENABLE = yes` to `rules.mk`
2. Add `#define SMTD_DEBUG_ENABLED` into `config.h`
3. (optional) Cretate `char* keycode_to_string_user(uint16_t keycode)` function for better readability
4. Compile and flash
5. Run `qmk console -n -t` to see, what is going on