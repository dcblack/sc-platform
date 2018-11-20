#define REQUIRES_CPP 17
#include "require_cxx_version.hpp"
#include "require_cxx_version.hpp"
#include <iostream>
using namespace std;

int main(void)
{
  cout << "CPP_VERSION is " << CPP_VERSION << endl;
  cout << "__cplusplus is " << __cplusplus << endl;
  return 0;
}
