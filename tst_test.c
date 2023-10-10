#include "tst.h"  // Ensure the tst framework is included

void myData(FILE *f)
{
  fprintf(f,"\nMYDATA MYDATA MYDATA MYDATA MYDATA\n MYDATA MYDATA MYDATA MYDATA \n");
}

int main(int argc, char *argv[])
{
  tstrun("Primary Test Suite") {
    
    tstcase("Equality Checks %d, %d", 1, 1) {
      tstcheck(1 == 1, "Mismatch: %d != %d", 1, 1);
      tstcheck(1 != 1, "Failed on purpose");
    }
    
    tstcase("Time Complexity Analysis") {
      tstclk("Check counting time") {
        volatile int b = 1;
        // Code to analyze...
        for (int a = 1; a < 100 ; a++) b = a + b;
      }
    }
    
    tstcase("Grouped Checks: Edge Cases") {
      tstgroup(1 == 2, "Inequality" ) {  // Will be skipped!
        tstcheck(0 < 1, "0 should be less than 1");
        tstassert(1 >= 1, "1 should be equal to 1");
      }

      tstgroup(1 != 2,"Equality") {  // Will be executed!
        tstcheck(0 < 1, "0 should be less than 1");
        tstassert(1 >= 1, "1 should be equal to 1");
      }
    }
    
    tstdata("Useful Debug Data") {
       myData(stderr);
    }
    
    tstnote("Testing Complete. Review for any FAIL flags.");
  }
}