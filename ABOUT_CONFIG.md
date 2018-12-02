About Configuration
===================

The `Configuration` contains information about target module configuration including:

- Full hierarchical path name
- Class name (kind)
- Start address
- Depth

The `Config_extn` is used to carry the `Configuration` in transactions. This is
used for probing the bus targets and modifying configuration as needed.

The `Config_proxy` is to be used in situations where the target does not
internally support the `Configuration` concept. For example, acquired or legacy
TLM IP.

Usage Example
-------------

```cpp
{:CODE SNIPPETS:}
```

Implementation Details
----------------------

{:EXPLANATION OF STRUCTURES AND APPROACH:}

### end
