About Enumerations
==================

This is a quick description of features in the _enum.hpp files, which
implement enumeration classes including full string I/O. There is only
one new line. These are all generated from the cpp-enum template.

Features
--------
- Enumeration classes for safer enumeration.
- Streaming I/O and string conversion support.

Requirements
------------
- SystemC 2.3.2 or better
- C++14 or better

Usage Example
-------------

```cpp
#include <test_enum>
...
Test t1{ Test::TRIVIAL };
std::cout << "Test is " << t1 << std::endl;
...
std::cout << "Available tests:" << std::endl;
for( auto t : Test ) {
  std::cout << "  " << t << std::endl;
}
...
#include <string>
string s { "TRIVIAL" };
Test   t2;
if( is_Test( s ) t2 = to_Test( t2 );
```

### The end
<!-- vim:tw=78
-->
