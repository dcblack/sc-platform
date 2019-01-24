About Asynchronous Channels
===========================

SystemC as a whole is not OS thread-safe. This document describes thread-safe mechanisms to address this problem.

Note: **WORK-IN-PROGRESS**

Features
--------
- C++11 condition variables to synchronize outgoing events
- IEEE 1666-2011 `async_request_update` - to synchronize incoming events
- C++11 `mutex` to secure modification of payloads
- C++11 `thread` to communicate outside the application
- C++11 `chrono` for wall clock time
- SystemC `sc_time` for simulated time since start of simulation
- TCP/IP sockets
- `async_payload` class to carry information

Requirements
------------
- SystemC 2.3.2 or better
- C++14 or better

Functional Description
----------------------

Two SystemC channels provide two-way communications between SystemC and external OS threads,
which in turn provide connectivity via TCP/IP sockets to/from external processes.

- `Async_payload` provides a common data structure for exchanges.
- `Async_tx_if` provides a common interface for transmission.
- `Async_rx_if` provides a common interface for receipt.
- `Async_if` combined interface.
- `Async_rx_channel` SystemC receives notifications and data from OS. Implements `Async_if`.
- `Async_tx_channel` SystemC transmits data with notifications to OS. Implements `Async_if`.
- `Tcpip_rx` provides an OS client thread to connect with an incoming TCP/IP socket.
- `Tcpip_tx` provides an OS client thread to connect with an outgoing TCP/IP socket.

Both channels implement queues, which may be fixed maximum depth or infinite if value of zero.

`Asynchronous payload`
-------------------------

Briefly, the data portions are roughly equivalent to:

```cpp
typedef struct
{
  uint64_t  id;   //< serial number incremented for every send from the originator
  uint64_t  orig; //< identifier
  uint64_t  dest; //< identifier
  uint64_t  time; //< senders time
  uint32_t  kind; //< enumerated values
  Data_t    data; //< payload data if any
} Async_payload_t;
```

For C++, appropriate constructors, and access methods will be created.


`Asynchronous Interfaces`
-------------------------

```cpp
Async_tx_if<T>
  notify( Async_payload<T>* );

Async_rx_base_if
  void wait_unless_available( void );
  bool is_empty() const;
  size_t available( void ) const;
  const sc_event& default_event( void ) const; // Only valid for SystemC side.
  void drop();

Async_rx_if<T>
  // Copy out data -- returns false if queue empty. Sets data_len to 0 if no data. Removes entry from queue if successful.
  bool nb_read( Async_payload<T>& the_payload );
  // Blocking call -- waits for data. Sets data_len to 0 if no data. Removes entry from queue.
  void read( Async_payload<T>& the_payload );
  // Get point to data -- returns false if queue empty. Does NOT complete transaction.
  bool nb_peek( Async_payload<T>*& payload_ptr ) const;
  // Blocking call to get pointer to data -- waits for data. Does NOT complete
  // transaction.
  void peek( Async_payload<T>*& payload_ptr ) const;
```

`Asynchronous Channels`
-------------------------

```cpp
#include <systemc>
#include "async/async_payload.hpp"
template<typename T>
struct Async_rx_channel
: Async_rx_if<T>, sc_prim_channel
{
private:
};
```

```cpp
#include <systemc>
template<typename T>
struct Async_tx_channel
: Async_tx_if<T>, sc_prim_channel
{
private:
};
```

`TCP/IP Interfaces`
-------------------

```cpp
Tcpip_tx_if<T>
  bool can_put();
  const sc_event& ok_to_put();
  void put( Async_payload& the_payload );
  bool nb_put( Async_payload& the_payload );
Tcpip_rx_if<T>
  bool can_get();
  const sc_event& ok_to_get();
  void get( Async_payload& the_payload );
  bool nb_get( Async_payload& the_payload );
```

`TCP/IP Channels`
-------------------


```cpp
template<typename T>
struct Tcpip_tx_channel
: Tcpip_tx_if<T>, sc_channel
{
  // Local channels
  Async_tx_channel tx;

  // Launch thread
  Tcpip_tx_module( int port );

  // Create socket and listen for connect - blocking
  // Packetize and send - blocking
  void OS_tx_thread( void );
};
```

```cpp
template<typename T>
struct Tcpip_rx_channel
: Tcpip_rx_if<T>, sc_channel
{
  // Local channels
  Async_tx_channel tx;

  // Launch thread
  Tcpip_rx_module( int port );

  // Create socket and listen for connect - blocking
  // receive, unpack and return - blocking
  void OS_rx_thread( void );
};
```

UML Sequence Diagrams
---------------------

```
+----------------------------------------------+  +----------------------+
|                                              |  |                      |
|                   Platform                   |  |    xterm             |
|          =========================           |  |    =====             |
|                      |                       |  |                      |
|                create tcp socket             |  |                      |
|                bind to address               |  |                      |
|                listen for connect            |  |                      |
|                      |                       |  |                      |
|                create OS_RX thread           |  |                      |
|                create OS_TX thread           |  |                      |
|                      |         |             |  |                      |
|                     ---        v             |  |                      |
|          SC_TX               OS_TX           |  |    NetRX             |
|          =====               =====           |  |    =====             |
|            |                   |             |  |      |               |
|            |                   |             |  |      |               |
|            |                   |             |  |      |               |
|            |             txque.read(v)       |  |      |               |
|  tx.put(data)                  :             |  |      |               |
|    v = new value(...)          :             |  |      |               |
|    txque.notify(v)             :             |  |      |               |
|      m_mtx.lock()              :             |  |      |               |
|      m_que.push_back(v)        :             |  |      |               |
|      m_mtx.unlock()            :             |  |      |               |
|      m_barrier.notify()------->|             |  |      |               |
|            |               m_mtx.lock();     |  |      |               |
|            |               v = m_que.front() |  |      |               |
|            |               m_que.pop_front() |  |      |               |
|            |               m_mtx.unlock();   |  |      |               |
|            |               p = packetize(v)  |  |      |               |
|            |                send(p)----------|~>|--receive(p)          |
|            |                   |             |  |  v=unpack(p)         |
|            |                   |             |  |  txaction(v)         |
|            |                   |             |  |      |               |
|            -                   -             |  |      -               |
|                                              |  |                      |
|          SC_RX               OS_RX           |  |    NetRX             |
|          =====               =====           |  |    =====             |
|            |                   |             |  |      |               |
|            |                   |             |  |      |               |
|  rx.get(data)                  |             |  |  "Input event"       |
|            :                   |             |  |  v = new value(...)  |
|            :                   |             |  |  p = packetize(v)    |
|            :             receive(p)<---------|<~|----send(p)           |
|            :             v = unpack(p)       |  |      |               |
|            :                   |             |  |      |               |
|            :             rxque.notify(v)     |  |      |               |
|            :               rxque.lock()      |  |      |               |
|            :               rxque.push_back(v)|  |      |               |
|            :               rxque.unlock()    |  |      |               |
|            :<--------------async_request_upda|  |te()  |               |
|            :                   |             |  |      |               |
|      rxque.lock()              :             |  |      |               |
|      v = rxque.front()         :             |  |      |               |
|      rxque.pop_front()         :             |  |      |               |
|      rxque.lock()              :             |  |      |               |
|      rxaction(v)               |             |  |      |               |
|            |                   |             |  |      |               |
|            -                   -             |  |      -               |
|                                              |  |                      |
+----------------------------------------------+  +----------------------+
```

Syntax
------

See header comments for information on methods.

Usage Example
-------------

```cpp
// Globally
#include <systemc>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
std::chrono::high_resolution_clock::time_point t0;
struct Barrier {
  void notify() {
    std::unique_lock<std::mutex> lck(mtx);
    ready = true;
    cv.notify_all();
  }
  void wait( void ) {
    std::unique_lock<std::mutex> lck(mtx);
    while(!ready) cv.wait(lck);
  }
  std::mutex              mtx;
  std::condition_variable cv;
  bool                    ready{ false};
  size_t                  threshold{ 1 };
};
Barrier tx_barrier;

  void Top_module::start_of_simulation( void ) {
    // Establish simulation start time
    t0 = high_resolution_clock::now();
    // Establish TCP/IP connection
    // - Create
    // - Bind address
    // - Listen
    // - Accept
  }

// Launch OS threads
auto rx_handle = std::thread( rx_osthread );
auto tx_handle = std::thread( tx_osthread );

// SystemC
// OS Threads
  void rx_osthread( void ) {
  }
  void tx_osthread( void ) {
    tx_barrier.wait();
  }
```

Testing
-----

{:***TO BE SUPPLIED*** -- POSSIBLY AS FOLLOWS -- DELETE THIS LINE WHEN CORRECT:}
```sh
g++ -std=c++14 -DASYNC_EXAMPLE -o async.exe async.cpp && ./async.exe
```

Files
-----

  Filename                 | Purpose                  
  ------------------------ | -------------------------
  `ABOUT_ASYNC.md`   | Documentation in markdown
  `async.hpp`        | {:COMMENT1:}             
  `async.cpp`        | {:COMMENT1:}             

Implementation Details
----------------------

{:EXPLANATION OF STRUCTURES AND APPROACH:}

See Also
--------

### The end
<!-- vim:tw=78
-->
