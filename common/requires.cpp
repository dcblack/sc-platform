#define REQUIRES_CPP 17
#define REQUIRES_SYSTEMC 20181013
#include "common/require_version.hpp"
#include "common/require_version.hpp"
#include <iostream>
using namespace std;

int main(void)
{
  cout << "CPP_VERSION is " << CPP_VERSION << endl;
  cout << "__cplusplus is " << __cplusplus << endl;
  cout << "SYSTEMC_VERSION is " << SYSTEMC_VERSION << endl;
  return 0;
}
