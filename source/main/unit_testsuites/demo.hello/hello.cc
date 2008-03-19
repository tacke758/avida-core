#ifndef __DEJAGNU_H__
#include "../dejagnu.h"
#endif

using namespace std;

int main(int argc, char *argv[]){
  TestState test;
  for(int i = 0; i < argc; i++) cout << argv[i] << " ";
  cout << endl;
  test.pass("pass message.");
  //test.fail("fail message.");
  //test.untested("untested message.");
  //test.unresolved("untested message.");
  //test.totals();
  sleep(1);
}
