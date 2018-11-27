Table of Contents
=================

+ [About the Project](#AbtMe)
+ [The Grand Design](#GrandDesign)
  - [Block Diagram](#BlkDiag)
  - [Memory Map](#MemMap)
  - [Design Notes](#DNotes)
  - [Module Status](#Stats)
  - [To Do List](#ToDo)
+ [Rules, Conventions, and Guidelines](#CRules)
+ [Instructions for Building](#HowTo)
+ [About Markdown](#Mrkdown)

# <a name="AbtMe"></a>About the Project

This directory contains a basic SystemC design using TLM-2.0 with a single
top-level module and a practical main implementation that validates each step,
and provides an execution summary including timing/performance information.
If errors are detected, main exists with a non-zero status.

This project has several goals:

1. Provide an example of a complete TLM-2 virtual platform for study using modern C++.
2. Provide examples of different techniques of modeling.
  1. Loosely-timed and Approximately-timed models of initiators, interconnect and targets
  2. The absence of real clocks using the `no_clock` channel and the global distribution mechanism
  3. Resets and Power domain modeling
  4. Processor implementation with an ISS and support for interrupts
3. Provide the basis for "Easier SystemC" templates.
4. Several approaches to configuration

At any point in time, this project represents a set of ideas in the making, and
not expected to be complete. Various authors may contribute to this effort, and
as such, coding guidelines have bee provided here-in. It is important that this
code be *model code* and meet high levels of documentation and formatting to
make it easy to understand. Comment blocks are highly encouraged.

# <a name="GrandDesign"></a>The Grand Design

## <a name="BlkDiag"></a>Block Diagram

- Names inside the boxes are instance names.
- Names outside the boxes are part of a module name.
- Blocks with an exclamation mark (`!`), either generate or receive interrupts.
- Dotted boxes indicate a power or reset domain


```
Top
+--------------------------------------------------------------+
| .--------------.                                           r0|
| | Proc         |                                             |
|.|..........    |                                             |
|:| prc   r1:    |                                             |
|:|         :    |                                             |
|:|         :....|.....                                        |
|:| Cpu     : Dma|  r2: Pic       Gpio      Crypto             |
|:| +-----+ : +--^--+ : +-----+   +-----+   +-----+            |
|:| ! cpu ! : | dma ! : ! pic !   | gio !   |crypt!            |
|:| +--v-v+ : +--^--+ : +--^--+   +--^--+   +--^--+            |
|:|    | |  :....|....:    |         |         |     SBus      |
|:|    | |  :    |         |         |         |     +-----+   |
|:'--->| |  :    '---------x---------x---------x-----> spi !   |
|:     | '--:---.          |         |         |     +--v--+   |
|:  Mmu|    :   |          |         |         |        |      |
|:  +--v--+ :+--v--+    +--v--+   +--v--+      |   .----'      |
|:  ! mmu ! :| tcm |    | ssd |   |flash|      |   | Environ   |
|:  +v---v+ :+-----+    +-----+   +-----+      |   | +-----+   |
|:  P|  S|  :Memory     Ssd       Flash        |   x-> env |   |
|:   |   |  :                                  |   | +-----+   |
|:.--'   |  ...............................    |   |           |
|:| Cache|  : Ssd   r4: Mouse r5: Keybd r6:    |   | +-----+   |
|:| +----v+ : +-----+ : +-----+ : +-----+ :    |   '-> gps |   |
|:| | l2c | : | thb | : | ptr | : | kbd | :    |     +-----+   |
|:| +--v--+ : +--^--+ : +--^--+ : +--^--+ :    |     Gps       |
|:|    |    :....|....:....|....:....|....:    |               |
|:'--->|    :    '---------x---------'         |               |
|:.....|....:              |                   |               |
|   Bus|      Disk      Usb|      Bus          |     Memory    |
|   +--V--+   +-----+   +--^--+   +-----+      |     +-----+   |
|   | nth |   | dsk !   | usb !   | sth >------x-----> ddr |   |
|   +--V--+   +--^--+   +--^--+   +--^--+      |     +-----+   |
|      |         |         |         |         |               |
|      x---------x---------x---------x         x--------.      |
|      |         |         |         |         |        |      |
|      |         |    .....|.........|.....    |        |      |
|      |         |    :    |         |  r3:    |        |      |
|   +--v--+   +--v--+ : +--v--+   +--v--+ : +--v--+  +--v--+   |
|   | ram |   | rom | : | net !   | vid ! : | tmr !  | ser !   |
|   +-----+   +-----+ : +-----+   +-----+ : +-----+  +-----+   |
|   Memory    Memory  : Wifi      Video   : Timer    Usart     |
|                     :...................:                    |
|                                                              |
+--------------------------------------------------------------+
```

## <a name="MemMap"></a>Memory Map
----------

| Block | Base Address | Bus | Size | Irq | Module   |
| ----- | :----------- | --- | ---: | --: | :-----   |
| cpu   | 0xF000'0000  |  -  |  128 |  -  | Cpu      |
| rom   | 0x0000'0000  | NTH |  96M |  -  | Memory   |
| ram   | 0x1000'0000  | NTH | 128K |  -  | Memory   |
| ddr   | 0x2000'0000  | NTH | 256M |  -  | Memory   |
| tmr   | 0x4000'0000  | sth |   32 |  1  | Timer    |
| gio   | 0x4000'1000  | sth |   32 |  2  | Gpio     |
| crypt | 0x4000'2000  | sth |   2K |  3  | Crypto   |
| pic   | 0x4000'3000  | sth |   1K |  4  | IrqCtrl  |
| dma   | 0x4000'4000  | sth |   32 |  0  | Dma      |
| flash | 0x4000'5000  | sth |   4K | 12  | Flash    |
| spi   | 0x4000'6000  | sth |   32 |  5  | SBus     |
| ser   | 0x4000'7000  | sth |   32 |  6  | Usart    |
| dsk   | 0x4008'0000  | NTH |   1K |  7  | DiskC    |
| net   | 0x4008'1000  | NTH |   32 |  8  | Wifi     |
| mmu   | 0x4008'2000  | NTH |   32 |  9  | Mmu      |
| usb   | 0x4008'3000  | NTH |   32 | 10  | Usb      |
| ssd   | 0x6000'0000  | sth |   2M | 13  | Ssd      |
| flash | 0x8000'0000  | sth |   1G |  7  | Flash    |
| vid   | 0xC000'0000  | NTH |  16M | 11  | Video    |
| gps   | spi:0x0      | spi |   32 |  -  | Gps      |
| env   | spi:0x100    | spi |   32 |  -  | Environ  |
| ptr   | usb:0        | usb |   32 |  -  | Mouse    |
| kbd   | usb:1        | usb |   32 |  -  | Keyboard |
| thb   | usb:2        | usb |  32G |  -  | Ssd      |
| nth   | 0x0000'0000  |  -  |   4G |  -  | Bus      |
| sth   | 0x4000'0000  |  -  |  64K |  -  | Bus      |
| l2c   | n/a          |  -  |    - |  -  | Cache    |

## <a name="DNotes"></a>Design Notes

- Main buses are designated as North (nth), and South (sth)
- North bus is closest to the "main" processor (cpu)
- System clock 400MHz
- Processor is RISC (cm0+ or possibly w/ FPU e.g. Arm cm4f)
- Cpu tasks provide initiator behaviors (e.g. memory test, processor emulation)
- NTH is 32 bits 100MHz
- STH is 16 bits  50MHz
- rom is a type flash (fast write)
- flash is paged slower flash with separate controls
- Gpio has 4 LEDs, 8 toggles, 2 momentary
- Terminal communicates via TCP sockets
- Video reads/writes files
- Wifi sends/receives from web
- Timer is expandable (1-16)
- Memory has read-only option & supports DMI
- Mmu has zero latency
- DiskCtrl reads/writes real files
- Dma has configurable # channels (1-16)
- Pic accepts 256 sources, simple priority
- Serial I/O configurable baud
- Eventually Video or Wifi may become a hierarchical sub-system
- Eventually one of the buses will become customized
- Order of development is TBD, but simplest first
- Four power regions labeled r0 through r6
- One hierarchical boundary in the `Proc_module` represented by r1

## <a name="Stats"></a>Module Status

Each module will its status noted here. The following states are allowed:

- Thought - placeholder from this document not yet tackled
- Concept - documentation has begun
- Started - initial coding (some files in various states)
- Basic - basic functionality achieved with minimal coding
- Tested - extensive testing completed

| Class or Module     | Short Description                            | About | State   |
| :------------------ | :------------------------------------------- | :---: | :------ |
| `Bus_module`        | Interconnect                                 |   Y   | Basic   |
| `Cpu_module`        | Initiator                                    |   Y   | Basic   |
| `Memory_module`     | Target RAM                                   |   Y   | Basic   |
| `Timer_module`      | Target timer                                 |   Y   | Basic   |
| `no_clock`          | Substitute clock                             |   Y   | Basic   |
| `Mailbox_module`    | Hardware communication between processors    |   Y   | Concept |
| `Excl_proxy`        | Implements non-locking bus mutual exclusion. |   Y   | Started |
| `Excl_extn`         | Request mutual exclusion.                    |   Y   | Started |
| `Dma_module`        | Target/Initiator                             |   Y   | Concept |
| `Pic_module`        | Priority Interrupt Controller                |   Y   | Started |
| `Power` class       | Model power regions                          |   Y   | Started |
| `Reset` class       | Model resets                                 |   Y   | Concept |
| `Report` routines   | Simplify reporting                           |   Y   | Basic   |
| `Secure_extn`       | Identify transactions to secure areas.       |   Y   | Started |
| `Task_manager`      | Access tasks by name                         |   Y   | Concept |
| `Wallclock`         | Platform independent timing                  |   Y   | Concept |
| `Cache_module`      | Implement an associative cache               |   Y   | Concept |
| `Cache_extn`        | Used to perform cache actions                |   Y   | Started |
| `Mmu_module`        | MMU/MPU implementation                       |   Y   | Started |
| `Config_extn`       | Used to interrogate instances                |   Y   | Basic   |
| `armv6m` task       | Cpu task to emulate Arm Cortex-M0+ ISS       |   Y   | Started |
| `memory_test`       | Cpu task to test memory block                |   Y   | Basic   |
| `timer_test`        | Cpu task to test timer  block                |   Y   | Basic   |
| `Timer` behavior    | Behavior of timer used by `Timer_module`     |   Y   | Basic   |
| `Timer_api`         | API used `Timer_test`                        |   Y   | Basic   |
| `hexfile` utils     | Save/load/dump hex data                      |   Y   | Basic   |
| `Signal` class      | Specify how to handle CTRL-C                 |   Y   | Basic   |
| `Interrupt` channel | Carry interrupts between modules             |   Y   | Concept |
| `Summary` class     | Summarize execution                          |  TBS  | Concept |
| `sc_main`           | Beyond basics.                               |  TBS  | Concept |
| `Top_module`        | Module connectivity                          |  TBS  | Concept |
| `Proc_module`       | Hierarchical TLM                             |  TBS  | Thought |
| `Disk_module`       | Disk simulation using file system            |  TBS  | Thought |
| `SBus_module`       | Serial interconnect (SPI or I2C)             |  TBS  | Thought |
| `Crypto_module`     | Cryptography unit                            |  TBS  | Thought |
| `Gpio_module`       | General purpose I/O                          |  TBS  | Thought |
| `Ssd_module`        | Solid state memory (possibly sophisticated)  |  TBS  | Thought |
| `Flash_module`      | Flash memory unit (various types modeled)    |  TBS  | Thought |
| `Environ_module`    | Environment sensors (temp, magnetic, ...)    |  TBS  | Thought |
| `Usb_module`        | USB interconnect                             |  TBS  | Thought |
| `Video_module`      | Video display (possibly GPU too)             |  TBS  | Thought |
| `Usart_module`      | Serial port                                  |  TBS  | Thought |
| `Gps_module`        | GPS location                                 |  TBS  | Thought |
| `Wifi_module`       | WiFi using network to simulate               |  TBS  | Thought |

## <a name="ToDo"></a>To Do List

In order of priority:

1. Add `Timer_module` and South Bus instantiation. Will include `no_clock`.
   1. Create base timer - DONE
   2. Add base `no_clock` - DONE
   3. Add South Bus
2. Add `Pic_module`
3. Add `Dma_module`
4. Add power-down capability (?use CCI?) with reset
5. Add timing to AT mode of `Bus_module` with analysis port support
6. Add yaml or jason support for configuration
7. Implement proxy interconnect
8. Implement a CPU emulation (e.g. armv6m)

Optional:

1. Add fancy report handler with XML option and expectations for error injection
2. Implement one module as RTL and provide an example adaptor
3. Consider refactor `Cpu_module` to use PIMPL and separate API and tests.
4. Consider refactor `Memory_module` to use PIMPL
5. Add a shell interface and a scripting language for use in a CPU thread. LUA, Python or TCL.
6. Add a `Mailbox_module` (2 FIFO\'s)
7. Add a `Stack_module` (LIFO).
8. Add a `SystemManager_module` to manage clocks/power/resets
9. Add `Global` class to replace `g_` variables

# <a name="CRules"></a>Rules, Conventions, and Guidelines

- Documentation will be maintained in GitHub style Markdown
   + ABOUT_{:NAME:}.md will document each module
- C++14 standard will be the minimum C++ standard level used.
- Header filenames have suffix `.hpp` unless purely C11 compatible.
- Compiled C++ filenames have suffix `.cpp` unless purely C11 compatible.
- Raw pointers and arrays will be avoided. Instead use `std::unique_ptr<>`,
  `std::unique_ptr<>`, `std::vector<>`, etc. when possible. References are
  better than pointers of any type.
- `cstdint` will be preferred over built-in types.
- Destructors will be defined. `default` is allowed if no allocations need to be
  managed.
- TLM Base Protocol will be preferred.
- Ignorable extensions will be preferred.
- Underscores will be used to separate words.
- User-defined types (e.g. classes, structs, enums) will start.
  with use Name case.
- `enum class` will be preferred over simple `enum`
- Methods and variables will be lowercase.
- UPPERCASE may be used for labels, enumeration values and constants only.
- `and` will be preferred over `&&`.
- `or` will be preferred over `||`.
- `not` will be preferred over `!`.
- Leading or trailing underscores `\_` will NEVER be used together (i.e. `a_`
  and `_a` are legal, but `_a_` is not).

| Pre/Suf   | Use                             |
| --------- | ------------------------------- |
| `_api`    | initiator methods/constants     |
| `_beh`    | behavioral implementation       |
| `_delay`  | relative simulation `sc_time`   |
| `_extn`   | `tlm_extension`                 |
| `_if`     | interface class for channels    |
| `_method` | `SC_METHOD`                     |
| `_module` | `sc_module`                     |
| `_port`   | `sc_port`                       |
| `_rtl`    | RTL implementation              |
| `_socket` | TLM socket                      |
| `_t`      | `typedef`                       |
| `_thread` | `SC_THREAD`                     |
| `_time`   | absolute simulation `sc_time`   |
| `_xport`  | `sc_export`
| `g_`      | global variables                |
| `get_`    | const accessor method           |
| `is_`     | returns bool status             |
| `m_`      | class member attributes         |
| `_ptr`    | pointer to object               |
| `s_`      | class static attributes         |
| `set_`    | modifying accessor method       |

## Extras

Numerous extras have been added including:

- `report.h` is provided to simplify message reporting. See `report.h` comments
  for more information.
- `config.h` and `config.cpp` provide a generalized configuration mechanism that
  may be used to configure memory maps and other features.
- `proxy.h` and `proxy.cpp` supply a proxy for modules that do not have `config`
  built in.

# <a name="HowTo"></a>Instructions for Building

Dependencies
------------

+ Cmake is used to configure.
+ SystemC version 2.3.2 or better
+ C++14 or better
+ Boost libraries

If you would like to contribute, you should also have:

+ git
+ astyle

Linux/OSX
---------
1. Open a terminal and navigate into the `build/` directory located where this file is located.
2. Configure by typing `cmake ..`
3. Build code with `make` or `make all`
4. Alternately, build and run with `make run`
5. To pass arguments to run, use `env ARGS="-your args" make run`
6. To cleanup after running, use `make clean`

A slightly more automated approach uses some scripts located in the `bin/` directory.
A `setup.profile` bash script should be sourced to set environment variables; however, it will
need some tweaking. For one thing, it uses `modulecmd` to setup C++ (clang), SystemC and
Boost environment.

Windows
-------
*To be determined*

# <a name="Mrkdown"></a>About Markdown

This document was created as a MarkDown text file.

- Best viewed with a Markdown rendering tool.
- Or generate HTML, PDF or DOC using pandoc <http://pandoc.org/installing.html>
- Do not edit derived files
- For apps that view/edit markdown see <https://github.com/karthik/markdown_science/wiki/Tools-to-support-your-markdown-authoring>.
- Same syntax as used for GitHub README's etc. Very popular among programmers.

Reasons for GitHub Markdown include:

- Very simple - relatively intuitive and easy to learn
- No special tools - works for any text editor across platforms (Linux, Windows, Mac, iPhone)
- Supports code syntax highlighting
- Easy to integrate into web-pages
- Easy to convert to Word, PDF, Pod, Tex, HTML
- Wide spread support in the programming community
- Small files
- VERY portable

--------------------------------------------------------------------------------
### The end
