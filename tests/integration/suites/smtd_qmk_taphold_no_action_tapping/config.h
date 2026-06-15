#pragma once

#include "test_common.h"

/* Handle standard QMK MT()/LT() keycodes with sm_td timing. */
#define SMTD_ENABLE_QMK_TAPHOLD 1

/* A/B control: with NO_ACTION_TAPPING, QMK's action.c calls process_record
 * directly per event instead of routing MT()/LT() through action_tapping first.
 * sm_td then sees raw, real-time press/release events. Compare against the
 * sibling smtd_qmk_taphold suite (action_tapping enabled) to isolate the
 * SMTD_ENABLE_QMK_TAPHOLD divergence documented in AGENTS.md. */
#define NO_ACTION_TAPPING
