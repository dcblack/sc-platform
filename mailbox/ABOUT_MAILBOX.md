About Mailbox
=============

Implements a hardware mailbox for communications synchronization between
multiple processors possibly with different clocks.

Features
--------
* Two FIFOS
* Separate input and output target sockets
* Configurable addresses per socket
* Configurable FIFO depths (1..) and widths (1,2,4,8)
* Interrupts signal availability, completion and errors
* Status and Data register per interface

Block Diagram
-------------

```
Cpu        Bus        Mailbox    Bus        Cpu
+------+   +------+   +------+   +------+   +------+
|      |   |      |   |      |   |      |   |      |
| cpu1 >---> bus1 >---> mbox <---< bus2 <---< cpu2 |
|      |   |      |   |      |   |      |   |      |
+------+   +------+   +------+   +------+   +------+
```

Example Usage
-------------

```cpp
const size_t runs = 20;
struct Mesg {
  Data_t req;
  Data_t rsp;
};
void cpu1_thread( void )
{
  std::vector<Mesg> data{runs};
  randomize(data);
  Mailbox_api mbox{ BASE_ADDR1 };
  for( auto& req: data ) {
    if( mbox.state(READY) ) {
      mbox.send( req );
      mbox.wait(DONE);
      mbox.wait(INCOMING);
      mbox.read( rsp );
    }
  }
}
```

Implementation Details
----------------------

{:TBS:}

### The end
