# span

span (styled as-is) is a minimal UI library in C, targetted at embedded, low-power and scanline-based systems. It provides a highly modular API, allowing for touchscreen interfaces, as well as button-based interfaces and static UIs.

The features provided include (but are not limited to):
*	Fully open-source, under the BSD 3-clause license.
*	Includes a dynamic animation backend, with multiple lerping functions provided.
*	Provides remarkable drawing speeds (about 10 FPS on a 20 MHz Cortex-M33 microcontroller, bottlenecked by the 10 MHz SPI connection to the 240x280 screen used for testing).
*	Has a basic event-driven backend for interfaces (allowing for enter/leave/press events on elements).
*	Includes a text rendering backend with support for a dynamic range of font resolutions (tested from 6x8 to 80x160 per character).
*	Has an icon rendering backend, with support for 1-bit masks and full-color icons.
*	Lacks any dependencies, even on the C standard library (only requires the `stdbool.h`, `stdint.h` and `stddef.h` headers).

## How do I build the demo?

A basic, SDL2-based demo is included (`demo.c`), simulating a smartwatch UI. It can be tested on linux by running the following commands:

```sh
bash build.sh
./demo
```

## How do I include it in my projects?

Only the source-code and the header are needed, provided at `include/span.h` and `span/span.c`. The library has been tested under C23-compliant compilers, but should build under C99-compliant ones. Note that the source-code assumes the header is added to the include path.

## How do I do X with the library?

Open an issue, and I will be more than willing to help!
