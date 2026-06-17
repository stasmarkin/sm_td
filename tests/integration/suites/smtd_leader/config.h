#pragma once

#include "test_common.h"

/* Fixed release term so tap/hold resolution is timing-independent. */
#define SMTD_GLOBAL_RELEASE_RATIO 0

/* Generous leader window so the whole driven sequence fits before timeout. */
#define LEADER_TIMEOUT 1000
