#pragma once

#include "test_common.h"

/* Dynamic release term ON (ratio 5, the default): the touch-release window is
 * min(p1, p2) / 5 clamped to [1ms .. RELEASE_TERM]. A fast roll (release rhythm
 * mirrors press rhythm) resolves tap-tap; a near-simultaneous release resolves
 * hold-tap. */
#define SMTD_GLOBAL_RELEASE_RATIO 5
