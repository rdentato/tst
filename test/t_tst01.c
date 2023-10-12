#include "tst.h"

tsttags(NoDB, FileOnly, SimpleRun);

int main(int argc, char *argv[])
{
    tstsettags(argc,argv);
    tstrun() {
      tstgroup(tsttag(NoDB) && !tsttag(SimpleRun)) {


      }
    }
}
