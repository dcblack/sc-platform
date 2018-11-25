About the Cache
===============

This is a simple set associative cache. Cache size, Line size, set size (ways)
are all configurable, but will default to 64k, 32 bytes, 4 ways.
Transaction with a valid `Cache_extn` will cause caching; otherwise, the module is
a transparent interconnect. `IGNORE_COMMAND` with cache operations of clean or
invalidate will turn this into the target.

Block Diagram
-------------

Usage Example
-------------

Implementation Details
----------------------

### The end
