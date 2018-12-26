About `Bus_module`
==================

Basic TLM 2.0 model of a memory mapped bus interconnect.

Features
--------
- TLM 2.0 base protocol compliant
- Supports `Config_extn`
- YAML memory map description
- No timing (currently)

Functional Description
----------------------

`Bus_module` is an implementation of a generic TLM bus interconnect component.
A memory map is constructed from a combbination of `Config_extn` probes in the
targets combined with a YAML memory map file.  Efforts are also made to
validate the map including checks for:

- No missing port information (lack of Config_extn data update)
- No overlapping address regions
- No loops

The memory map is constructed as follows:

1. Optionally, before `start_of_simulation()`, an implementation may modify
   configuration settings used in probing or alter the `memory_map.yaml` file.
2. At `start_of_simulation()`, `build_port_map()` is called.
3. YAML file is loaded.
4. For each bus probes its target socket connections using
   `transport_dbg()` to obtain Configuration data, which must include the target
   module's full `name()` and `kind()`. Data may include the target's depth.
   If not zero, the depth returned will be used in preference to any data in 
   the YAML file. This allows devices to be instantiated with different sizes.
5. Memory map is constructed by comparing the names in the port probes to the
   names found in the YAML file.
6. The resultant memory map is checked for overlapping data.

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
      | bus1| - - - Uses memory_map.yaml
      +--v--+
         v
         | level 1
   .-----x-----.----------.
   |           |          |
   |           |          V
+--v--+     +--v--+    +--v--+
|targ0|     |targ1|    | bus2| - - - Uses memory_map.yaml
+-----+     +-----+    +--v--+
                          v
                          | level 2
                    .-----x-----.
                    |           |
                 +--v--+     +--v--+
                 |targ2|     |targ3|
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
`Memory_map` reads YAML and provides address decode functionality.

Implementation Details
----------------------

{:TO BE SUPPLIED:}

### The end
<!-- vim:tw=78
-->
