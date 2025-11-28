#include "tst.h"

// void test_helper() {
//     tstcheck(0);
// }
// 
// tstsuite("Test") {
//     tstcase("Case 0") {
//         test_helper();
//     }
// }

#include "tst.h"

int test_helper_func() {
    return(0);
}

void test_helper_case();

  tstsuite("Test") {
    tstcase("Case 0") {
        tstcheck(test_helper_func());
    }

    test_helper_case();
}

void test_helper_case() {
    tstcase("Case 1") {
        tstcheck(0);
    };
}

