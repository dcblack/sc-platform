About the bus
=============

`Bus_module` is an implementation of a generic TLM bus interconnect
component. A memory map is constructed either from a `Config_extn`
probed in the targets, or via a YAML memory map file.

Block diagram
-------------

```
+-----+     +-----+
|init0|     |init1|
+--v--+     +--v--+
   |           |
   '-----x-----'
         |
         v
      +--v--+
      | bus | - - Uses memory_map
      +--v--+
         v
         |
   .-----x-----.
   |           |
+--v--+     +--v--+
|targ0|     |targ1|
+-----+     +-----+
```

Usage Example
-------------

```cpp
struct Impl {
  // Declare
  std::unique_ptr< Init_module > init0;
  std::unique_ptr< Init_module > init1;
  std::unique_ptr< Bus_module  > bus;
  std::unique_ptr< Targ_module > targ0;
  std::unique_ptr< Targ_module > targ1;

  Impl( void ) { // Constructor
  {
    // Instantiate
    init0 = std::make_unique< Init_module >( "init0", ... );
    init1 = std::make_unique< Init_module >( "init1", ... );
    init0 = std::make_unique< Bus_module  >( "bus", ... );
    targ0 = std::make_unique< Targ_module >( "targ0", ... );
    targ1 = std::make_unique< Targ_module >( "targ1", ... );
    // Connect
    init0 -> init_socket.bind( bus   -> targ_socket );
    init1 -> init_socket.bind( bus   -> targ_socket );
    bus   -> init_socket.bind( targ0 -> targ_socket );
    bus   -> init_socket.bind( targ1 -> targ_socket );
  }
};
```

See also
--------

`Config_proxy` may be used to establish start address and depth of targets.

Implementation
--------------

{:TO BE SUPPLIED:}

### The end
