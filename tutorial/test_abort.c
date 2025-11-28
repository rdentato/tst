#include "tst.h"
tstsuite("Test Abort") {
  tstcase("Abort test") {
    tstassert(0, "This will abort");
    tstcheck(1, "This never runs");
  }
}
