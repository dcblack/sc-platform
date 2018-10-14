Index
=====

+ [About the Project](#AbtMe)
+ [The Grand Design](#GrandDesign)
  - [Block Diagram](#BlkDiag)
  - [Memory Map](#MemMap)
  - [Design Notes](#DNotes)
  - [To Do List](#ToDo)
+ [Rules, Conventions, and Guidelines](#CRules)
+ [Instructions for Building](#HowTo)
+ [About Markdown](#Mrkdown)

# <a name="AbtMe"></a>About the Project

This directory contains a basic empty SystemC design with a single top-level
module and minimal main implementation that provides an execution summary.

This project has several goals:

1. Provide an example of a complete TLM-2 virtual platform for study.
2. Provide examples of different techniques of modeling.
3. Provide the basis for "Easier SystemC" templates.

At any point in time, this project represents a set of ideas in the making, and
not expected to be complete. Various authors may contribute to this effort, and
as such, coding guidelines have bee provided here-in. It is important that this
code be *model code* and meet high levels of documentation and formatting to
make it easy to understand. Comment blocks are highly encouraged.

# <a name="GrandDesign"></a>The Grand Design

Several extras have been added including:

- `report.h` is provided to simplify message reporting. See `report.h` comments
  for more information.
- `config.h` and `config.cpp` provide a generalized configuration mechanism that
  may be used to configure memory maps and other features.
- `proxy.h` and `proxy.cpp` supply a proxy for modules that do not have `config`
  built in.

## <a name="BlkDiag"></a>Block Diagram

- Names inside the boxes are instance names.
- Names outside the boxes are part of a module name.
  
```
Top
+-------------------------------------------------------------+
|                                                             |
|  Cpu       Dma       IrqCtrl   Gpio      Console            |
|  +-----+   +-----+   +-----+   +-----+   +-----+            |
|  | cpu |   | dma !   | pic !   | gio !   | con !            |
|  +--v--+   +-v-^-+   +--^--+   +--^--+   +--^--+            |
|     |        | |        |         |         |               |
|  Mmu|        | |        |         |         |      Environ  |
|  +--v--+     | |        |         |         |      +-----+  |
|  ! mmu <-----' '--------x---------x----x----x    .-> env |  |
|  +--v--+                |         |    |    |    | +-----+  |
|     |      Mouse        |         |    |    |    |          |
|  +--v--+   +-----+   +--v--+   +--v--+ |    |    |          |
|  |cache|   | ptr |   | ddr |   |flash| |    |    |          |
|  +--v--+ | +--^--+   +-----+   +-----+ |    |    |          |
|     |    |    |      Memory    Flash   |    |    |          |
|     |    |    |                        |    |    |          |
|  Bus|    | Usb|      Disk      Bus     |    |    | Gps      |
|  +--v--+ | +--^--+   +-----+   +-----+ | +--v--+ | +-----+  | 
|  | nth | | | usb !   | dsk !   | sth >-x ! spi >-'-> gps |  | 
|  +--v--+ | +--^--+   +--^--+   +--^--+ | +-----+   +-----+  | 
|     |    |    |         |         |    | SBus               |
|     |    |    |         |         |    |                    |
|     x----x----x---------x---------x    '----x---------.     |
|     |         |         |         |         |         |     |
|     |         |         |         |         |         |     |
|     |         |         |         |         |         |     |
|  +--v--+   +--v--+   +--v--+   +--v--+   +--v--+   +--v--+  |
|  | ram |   | rom |   | net !   | vid !   | tmr !   | ser !  |
|  +-----+   +-----+   +-----+   +-----+   +-----+   +-----+  |
|  Memory    Memory    Wifi      Video     Timer     Usart    |
|                                                             |
+-------------------------------------------------------------+
```

## <a name="MemMap"></a>Memory Map
----------

| Block | Base Address | Bus | Size | Irq | Module   |
| ----- | :----------- | --- | ---: | :-: | :-----   |
| cpu   | 0xF000'0000  |  -  |  128 |  -  | Cpu      |
| rom   | 0x0000'0000  | NTH |  16M |  -  | Memory   |
| ram   | 0x1000'0000  | NTH | 512K |  -  | Memory   |
| ddr   | 0x2000'0000  | NTH |   1G |  -  | Memory   |
| tmr   | 0x4000'0000  | sth |   32 |  1  | Timer    |
| gio   | 0x4000'1000  | sth |   32 |  2  | Gpio     |
| con   | 0x4000'2000  | sth |   32 |  3  | Console  |
| pic   | 0x4000'3000  | sth |   1K |  4  | IrqCtrl  |
| dma   | 0x4000'4000  | sth |   32 |  0  | Dma      |
| flash | 0x4000'5000  | sth |   4K | 12  | Flash    |
| spi   | 0x4000'6000  | sth |   32 |  5  | SBus     |
| ser   | 0x4000'7000  | sth |   32 |  6  | Usart    |
| dsk   | 0x4008'0000  | NTH |   1K |  7  | DiskC    |
| net   | 0x4008'1000  | NTH |   32 |  8  | Wifi     |
| mmu   | 0x4008'2000  | NTH |   32 |  9  | Mmu      |
| usb   | 0x4008'3000  | NTH |   32 | 10  | Usb      |
| flash | 0x8000'0000  | sth |   1G |  7  | Flash    |
| vid   | 0xC000'0000  | NTH |  16M | 11  | Video    |
| gps   | spi:0x0      | spi |   32 |  -  | Gps      |
| env   | spi:0x100    | spi |   32 |  -  | Environ  |
| ptr   | usb:0        | usb |   32 |  -  | Mouse    |
| nth   | 0x0000'0000  |  -  |   4G |  -  | Bus      |
| sth   | 0x4000'0000  |  -  |  64K |  -  | Bus      |
| cache | n/a          |  -  |    - |  -  | Cache    |

## <a name="DNotes"></a>Design Notes

- Main buses are designated as North (nth), and South (sth)
- North bus is closest to the "main" processor (cpu)
- System clock 400MHz
- Processor is RISC w/ FPU (Arm CM4F)
- NTH is 32 bits 100MHz
- STH is 16 bits  50MHz
- rom is a type flash (fast write)
- flash is paged slower flash with separate controls
- Gpio has 4 LEDs, 8 toggles, 2 momentary
- Terminal communicates via TCP sockets
- Video reads/writes files
- Wifi sends/receives from web
- Timer is expandable
- Memory has read-only option & supports DMI
- Mmu has zero latency
- DiskCtrl reads/writes real files
- Dma has configurable # channels (1-16)
- Pic accepts 256 sources, simple priority
- Serial I/O configurable baud
- Eventually Video or Wifi may become a hierarchical sub-system
- Eventually one of the buses will become customized
- Order of development is TBD, but simplest first

## <a name="ToDo"></a>To Do List

In order of priority:

1. Add `Timer_module` and South Bus instantiation. Will include `no_clock`.
   2. Create base timer
   2. Add base no_clock
   2. Add South Bus
1. Add `Global` class to replace `g_` variables
1. Add `Pic_module`
1. Add reset capability
1. Add power-down capability (?use CCI?)
1. Add timing to AT mode of `Bus_module` with analysis port support
1. Add yaml support for configuration

Optional:

1. Consider refactor `Cpu_module` to use PIMPL and separate API and tests.
1. Consider refactor `Memory_module` to use PIMPL

# <a name="CRules"></a>Rules, Conventions, and Guidelines

- Documentation will be maintained in GitHub style Markdown
- C++14 standard will be used.
- Header filenames have suffix `.hpp`.
- Compiled C++ filenames have suffix `.cpp`.
- Raw pointers and arrays will be avoided. Instead use `std::unique_ptr<>`,
  `std::unique_ptr<>`, `std::vector<>`, etc. when possible.
- `cstdint` will be preferred over built-in types.
- Destructors will be defined. `default` is allowed if no allocations need to be
  managed.
- TLM Base Protocol will be used.
- Underscores will be used to separate words.
- User-defined types (e.g. classes, structs, enums) will start.
  with use Name case.
- Methods and variables will be lowercase.
- UPPERCASE may be used for labels, enumeration values and constants only.
- `and` will be preferred over `&&`.
- `or` will be preferred over `||`.
- `not` will be preferred over `!`.
- Leading or trailing underscores `_` will NEVER be used together (i.e. `a_`
  and `_a` are legal, but `_a_` is not).

| Pre/Suf   | Use                         |
| -------   | ---                         |
| `_module` | sc_module                   |
| `_extn`   | tlm_extension               |
| `_t`      | typedef                     |
| `_thread` | SC_THREAD                   |
| `_method` | SC_METHOD                   |
| `_socket` | TLM socket                  |
| `_port`   | sc_port                     |
| `m_`      | class member attributes     |
| `s_`      | class static attributes     |
| `g_`      | global variables            |
| `get_`    | const accessor method       |
| `set_`    | modifying accessor method   |

# <a name="HowTo"></a>Instructions for Building

Cmake is used to configure.

Linux/OSX
---------
1. Open a terminal and navigate to the directory where this file is located.
2. Configure by typeing `cmake .`
3. Build code with `make` or `make all`
4. Alternately, build and run with `make run`
5. To pass arguments to run, use `env ARGS="-your args" make run`
6. To cleanup after running, use `make clean`

# <a name="Mrkdown"></a>About Markdown

This document was created as a MarkDown text file.

- Best viewed with a Markdown rendering tool.
- Or generate HTML, PDF or DOC using pandoc <http://pandoc.org/installing.html>
- Do not edit derived files
- For apps that view/edit markdown see <https://github.com/karthik/markdown_science/wiki/Tools-to-support-your-markdown-authoring>.
- Same syntax as used for GitHub README's etc. Very popular amoung programmers.

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
