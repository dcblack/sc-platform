About `Fanout` (Signal Expander )
=================================

Expand (or fan out) `sc_port<sc_signal<VECTOR_TYPE>>` to
`sc_vector<sc_port<BIT_TYPE>>`.

Features
--------
- Not all expanded bits need to be connected

Requirements
------------
- SystemC 2.3.2 or better
- C++14 or better

Functional Description
----------------------

The `Fanout` channel allows you to expand a signal port into its individual bits.
The performance cost of doing this is relatively high, so don't overuse
this class. You do not have to connect all the ports.

This only works to convert `sc_port< sc_signal_if< T1 > >`, where T1 is an
indexable type such that `typeid( T1[0] ) == typeid( T2 )` and allow
assignment in both directions.

Examples would be:

| T1           | T2           |
| ----------   | ----------   |
| `sc_int<N>`  | `sc_int<1>`  |
| `sc_uint<N>` | `sc_uint<1>` |
| `sc_fix<N..` | `sc_bit`     |
| `sc_bv<N>`   | `sc_bit`     |
| `sc_lv<N>`   | `sc_logic`   |
| `bitset<N>`  | `bool`       |

Block diagram
-------------

```
 m1                 fo     
+--------+          +---------+
| Module |          | Fanout  |         s0
|        |          | <T1,T2> |port[0]  +---------------+
|        |          |         P-------->I sc_signal<T2> |
|        |          |         |         +---------------+
|        |          |         |         s1
|        |p    xport|         |port[1]  +---------------+
|        P--------->X         P-------->I sc_signal<T2> |
|        |          |         |         +---------------+
|        |          |         |         s2
|        |          |         |port[2]  +---------------+
|        |          |         P-------->I sc_signal<T2> |
|        |          |         |         +---------------+
|        |          |         |
+--------+          +---------+
```

Syntax
------

This channel supports the methods of `sc_signal_inout_if`.

Usage Example
-------------

```cpp
using T1 = sc_int<3>;
using T2 = sc_int<1>;
SC_MODULE( Module ) {
  sc_port< sc_signal_if< T1 > > p;
  // or sc_inout< T1 > p;
  ...
};
SC_MODULE( Top ) {
  Module        m1{ "m1" };
  Fanout<T1,T2> fo{ "fo" };
  sc_signal<T1> s0{"s0"}, s1{"s1"}, s2{"s2"};
  SC_CTOR( Top ) {
    p1.p.bind( fo.xport );
    fo.port[0].bind( s0 );
    fo.port[1].bind( s1 );
    fo.port[2].bind( s2 );
  }
};
```

Note that we could use `sc_vector< sc_signal<T2> >` in `Top`; however,
I wanted to illustrate that it is not necessary.

Testing
-----

{:***TO BE SUPPLIED*** -- POSSIBLY AS FOLLOWS -- DELETE THIS LINE WHEN CORRECT:}
```sh
g++ -std=c++14 -DPORTEX_EXAMPLE -o portex.exe portex.cpp && ./portex.exe
```

Files
-----

  Filename                 | Purpose                  
  ------------------------ | -------------------------
  `ABOUT_PORTEX.md`   | Documentation in markdown
  `portex.hpp`        | {:COMMENT1:}             

Implementation Details
----------------------

{:EXPLANATION OF STRUCTURES AND APPROACH:}

See Also
--------

### The end
<!-- vim:tw=78
-->
