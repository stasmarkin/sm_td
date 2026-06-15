#pragma once

#include "test_common.h"

/* Fixed release term (ratio 0) so tap/hold/roll resolve purely on the press/idle
 * pattern the tests drive — dynamic release-term timing is covered separately in
 * the smtd_dynamic suite. */
#define SMTD_GLOBAL_RELEASE_RATIO 0
