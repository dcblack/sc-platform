About Power modeling
====================

Power class aids modeling of power domains.

First decide on names of power domains in your system. These should be simple
identifiers (e.g. `system`, `fpu`). There is no concept of sub-domains; however,
you could use underscores to create a pseudo hierarchy (e.g.
`manipulator_control`, `manipulator_engine`).

Controlling module
------------------

```cpp
auto cryptoPower { PowerDomain::create("crypto") };

cryptoPower.set_level(Power::gated);
...
cryptoPower.set_level(Power::fast);
```

Member module
-------------

```cpp
PowerDomain power{ "crypto" };

if( power.is_off() ) {
  trans.set_response
( TLM_GENERIC_ERROR_RESPONSE );
  return;
}

void power_method(  void ) {
    next_trigger( power.event() );
    switch( power.level() ) {
        case Power::fast: {
          // calculations specific to this level
        }
        case Power::slow: {
          // calculations specific to this level
        }
        case Power::gated: {
          // calculations specific to this level
        }
        case Power::off: {
          // calculations specific to this level
        }
    }
}

std::vector<sc_object> objects_vec = root.get_objects()

```

Implementation Details
----------------------

### The end
