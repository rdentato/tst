#include "tst.h"
#include "functions.h"

int f(int n, const char *s) {return 1; }// Always pass 

tstrun("Sections") {
  tstcase("Sections") {
    int a;
    a = 5;
    tstsection("Change to 9") {
      tstcheck(a == 5);
      a = 9;
      tstcheck(a == 9);
    }
    tstsection("Change to 8") {
      tstcheck(a == 5);
      a = 8;
      tstcheck(a == 8);
    }
    tstcheck(a != 5);
  }

  tstcase("Data diven (static)") {
    
    struct {int n; const char *s;} tstdata[] = {
             {123, "pippo"},
             {431, "pluto"},
             { 93, "topolino"}
    };

    tstsection("My check") {
      tstnote("Checking <%d,%s>",tstcurdata.n, tstcurdata.s);
      tstcheck(f(tstcurdata.n , tstcurdata.s));
    }

  }

  srand(time(0));
  tstcase("Data diven (random)") {
    
    // Use some random data
    int tstdata[4];
    for (int k=0; k<4; k++) tstdata[k] = 8-(rand() & 0x0F);

    tstsection("In thje range [-10, 10]") {
      tstnote("Checking %d",tstcurdata);
      tstcheck(-10 <= tstcurdata && tstcurdata <= 10);
    }

  }
}
