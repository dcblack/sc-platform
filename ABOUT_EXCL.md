About Excl
==========

The `Excl_filt` (exclusives filter) implements the read-lock/conditional-modify
concept.  For write operations, this would be identical to the
load-lock/store-compare operation also known as exclusive monitor by Arm with
a twist. Store can be any operation other than `read`.

Implementation makes use of an ignorable auto-extension, `Excl_extn`.

Adding an `Excl_extn` to a transaction makes it an exclusive operation. The
extension may be marked shared, in which case a global filter needs to be
setup.

This is a filter in the sense that it prevents exclusive non-read transactions
from passing further unless the exclusive status for the specified address has
been set.

To support the global concept, when a transaction marked as exclusive fails,
a fake transport debug transaction is created and forwarded on to pass the
exclusives status to potentially subsequent global filters.

Block Diagram
-------------

```
          "Local"       "Shared"  "Global"  
Cpu1      Excl_filt     Bus       Excl_filt   Memory
+------+   +------+     +------+   +------+   +------+
|      |   | cpu1 |     |      |   | xram |   |      |
| cpu1 >---> excl >-----> xbus >---> excl >---> xram |
|      |   |      |  ^  |      |   |      |   |      |
+------+   +------+  |  +------+   +------+   +------+
                     |  
          "Local"    |  
Cpu2      Excl_filt  |  
+------+   +------+  |  
|      |   | cpu2 |  |  
| cpu2 >---> excl >--'
|      |   |      |     
+------+   +------+     
```

Limitations
-----------
Only works as a 1:1 filter. In other words, this requires
a point-to-point connection.

Usage Example
-------------

```cpp
Top:
  std::unique_ptr < Cpu_module    > cpu1;
  std::unique_ptr < Cpu_module    > cpu2;
  std::unique_ptr < Excl_filt     > cpu1_excl;
  std::unique_ptr < Excl_filt     > cpu2_excl;
  std::unique_ptr < Bus_module    > xbus;
  std::unique_ptr < Memory_module > xram;
  std::unique_ptr < Excl_filt     > xram_excl;
  ...
  cpu1      = std::make_unique < Cpu_module    > ( "cpu1" );
  cpu2      = std::make_unique < Cpu_module    > ( "cpu2" );
  xbus      = std::make_unique < Bus_module    > ( "xbus" );
  xram_excl = std::make_unique < Excl_module   > ( "xram_excl" , 4, 256 );
  cpu1_excl = std::make_unique < Excl_module   > ( "cpu1_excl" , 4, 256, &xram_excl );
  cpu2_excl = std::make_unique < Excl_module   > ( "cpu2_excl" , 4, 256, &xram_excl );
  xram      = std::make_unique < Memory_module > ( "xram" , RAM_DEPTH, RAM_BASE,  Access::RW, 16,  8 );
  ...                            
  cpu1      -> init_socket.bind( cpu1_excl -> targ_socket );
  cpu1_excl -> init_socket.bind( xbus      -> targ_socket );
  cpu2      -> init_socket.bind( cpu2_excl -> targ_socket );
  cpu2_excl -> init_socket.bind( xbus      -> targ_socket );
  xbus      -> init_socket.bind( xram_excl -> targ_socket );
  xram_excl -> init_socket.bind( xram      -> targ_socket );
  
Initiator:
  #include "excl_extn.hpp"
  // Add  extension to invoke
  bool shared = false;
  Excl_extn* extn{ new Excl_extn(shared) };
  trans->set_auto_extension( extn );
  ...
  transport( trans, ... )
  ...
  if( extn->succeeded() ) ...
  else

Target:
  #include "excl_extn.hpp"
  Depth_t m_excl_size{ 4 };
  Depth_t m_excl_depth{ 256 };
  assert( isPowerOfTwo(m_excl_size) and isPowerOfTwo(m_excl_depth) );
  ...
  // If extension exists, update monitor
  Excl_extn* extn{trans.get_extension<Excl_extn>()};
  if( extn != nullptr ) {
    if( not extn->shared() ) {
      Addr_t  excl_mask = (((1u<<bits(excl_locks))-1) << bits(excl_size));
      if( extn->exclusive( trans, mon[trans.get_address()&erg] ) ) {
        trans.set_response( TLM_OK_RESPONSE );
        return; //early
      }
    }
    else { // shared
    }
  }
  // otherwise continue to complete the operation
```

Implementation Details
----------------------

{:TBS:}

### The end
