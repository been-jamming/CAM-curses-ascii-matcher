# CAM-curses-ascii-matcher
console graphics

Are you like me and hate GUI programming? Well I have the library for you! Introducing CAM (curses ASCII matcher), a library which hooks into curses and allows you to create virtual screens which are rendered as 8-color ASCII.

Included is an example `pong.c` program, which on my machine with mingw32 and pdcurses, I compile with 

```gcc CAM.c pong.c -lm -lpdcurses -Wall -O3 -o pong```

There is also a `mandafractal.c` mandelbrot explorer program that can be compiled the same way as `pong.c`

CAM should be compatible with any curses library.

More improvements to come soon, including optimized rectangle drawing, circle and line algorithms.

See https://pasteboard.co/IiuKu8y.png for terminal pong screenshot.
