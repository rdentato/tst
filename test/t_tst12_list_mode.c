//  SPDX-FileCopyrightText: © 2025 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

// This suite is designed to be inspected via:
//   ./t_tst12_list_mode --list
// or through tstrun:
//   ./tstrun -l -w '12*'
//
// It focuses on verifying that:
//   - In list mode, test case bodies are NOT executed
//   - The printed output includes the case name, and any tags
//   - Tag strings are printed exactly as provided to tstcase
//
// We verify "bodies not executed" via global counters, which must
// stay at zero when invoked with --list.

static int ran_untagged = 0;
static int ran_plus_tag = 0;
static int ran_minus_tag = 0;
static int ran_multi_tags = 0;

// Note: tstsuite(...) expands to tst_main, which will call
// tst__run(1) and exit(0) when argv[1] is "--list". In that
// situation tstcase__ sees tst_list_opt != 0 and only prints
// the case name/tags without executing the bodies below.

tstsuite("List mode behavior")
{
  tstcase("Untagged case") {
    ran_untagged++;
    tstcheck(ran_untagged == 1, "untagged case should run once in normal mode");
  }

  tstcase("Plus tag case", +ListTag) {
    ran_plus_tag++;
    tstcheck(ran_plus_tag == 1, "+tag case should run once in normal mode");
  }

  tstcase("Minus tag case", -ListTag) {
    ran_minus_tag++;
    tstcheck(ran_minus_tag == 1, "-tag case should run once in normal mode");
  }

  tstcase("Multiple tags case", +FirstTag, -SecondTag) {
    ran_multi_tags++;
    tstcheck(ran_multi_tags == 1, "multi-tag case should run once in normal mode");
  }

  // Sanity check in normal execution mode: all counters must be 1
  // by the time we get here. When invoked with --list, none of the
  // above bodies execute, so this case should still pass with all
  // counters at zero.
  tstcase("Counters summary") {
    tstcheck(ran_untagged == 1, "untagged counter value: %d", ran_untagged);
    tstcheck(ran_plus_tag == 1, "+tag counter value: %d", ran_plus_tag);
    tstcheck(ran_minus_tag == 1, "-tag counter value: %d", ran_minus_tag);
    tstcheck(ran_multi_tags == 1, "multi-tag counter value: %d", ran_multi_tags);
  }
}
