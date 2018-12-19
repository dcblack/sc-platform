About the Report utilities
==========================

The report utilities are intended to simplify SystemC reporting and
provide more flexibility, while keeping the implementation lightweight.

Foremost are the streaming macros that all steaming syntax to be combined
with `SC_REPORT_*` macros.

Syntax
------

- `ASSERT(expression, message_stream)`
- `REPORT(message_type, message_stream);`
- `INFO(verbosity_level, message_stream);`
- `MESSAGE(message_stream);`
- `MEND(verbosity_level);`
- `RULER(char);`
- `TODO(message_stream);`
- `NOT_YET_IMPLEMENTED();`

Message types are: FATAL, ERROR, WARNING, and INFO

Verbosity levels are: ALWAYS, LOW, MEDIUM, HIGH, DEBUG

The MESSAGE macro doesn't output anything, but rather builds up
a message to be emitted by REPORT, INFO or MEND.

Assumes you define in every implementation file (i.e. .cpp):

```cpp
#include "report.hpp"
namespace { static const char* const MSGID{ "/Company/Group/Project/Module" }; }
```

For header files leave off the name space and put in the function
or define a class member (non-static). Must not allow to escape
the header. So a #define is inappropriate.

Usage Example
-------------

```cpp
#include "report.hpp"
ASSERT( n > -2, "Value of n may only be positive or -1. Currently " << n );
namespace { static const char* MSGID{ "/Doulos/Example/Report" }; }
REPORT(ERROR,"Data " << data << " doesn't match expected " << expected);
INFO(DEBUG,"Packet contains " << packet);
TODO("Fix report handler to remove blank line after REPORT_INFO");
NOT_YET_IMPLEMENTED();
MESSAGE( "Map contents:\n" );
for( const auto& v : my_map ) {
 MESSAGE( "  " << v.first << ": " << v.second << "\n" );
}
MEND( HIGH ); // or REPORT( WARNING, "" );
```

Implementation Details
----------------------

{:EXPLANATION OF STRUCTURES AND APPROACH TO BE SUPPLIED:}

### end
