About the MMU
=============

This is a simplistic design of a Memory Mapping Unit with very granular
mappings to ensure decent performance. On reset, it will
read a default mapping if present; otherwise it will simply
pass all addresses out the **P**rimary socket. When properly configured
some addresses may be redirected out the **S**econdary port.
There may be a performance advantage when using the two ports and
one bypasses the other in the architecture (e.g. a cache).

Features
--------
- Can be used for Memory Protection (no mapping)

Block diagram
-------------

```
   |
   v
+--v--+
| Mmu |
+--v--+
   v
   |
```

Implementation
--------------

When enabled lookup table will translate upper bits and obtain attributes as
follows:

- Cacheable transactions will have a `Cache_extn` attached.
- Secure transactions will have `Secure_extn` attached.
- Shared transactions will have `Shared_extn` attached.
- Read/write violations will result in an `Exception_extn`.
- Routing may be based on the type of memory
- `Trans_extn` may be added for attributes and bus error tracking
- Functional bus errors may result if commands are inconsistent with permissions
  per level of read/write/execute.

```cpp
enum DevKind { MEMORY, DEVICE, TCM };
struct Trans_t
{
  int level;
};
struct Mmu_t
{
  bool      active { false  };
  Addr_t    start  { 0u     };
  Addr_t    depth  { 0u     };
  DevKind   devknd { MEMORY };
  uint8_t   cached { 0u     };
  bool      shared { false  };
  bool      secure { false  };
  bitset<8> rdPerm { 0xF    };
  bitset<8> wrPerm { 0xF    };
  bitset<8> exPerm { 0xF    };
};
```

### The end
