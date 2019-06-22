# CAM-curses-ascii-matcher
console graphics

Are you like me and hate GUI programming? Well I have the library for you! Introducing CAM (curses ASCII matcher), a library which hooks into curses and allows you to create virtual screens which are rendered as 8-color ASCII.

Included is an example `pong.c` program, which on my machine with mingw32 and pdcurses, I compile with 

```gcc CAM.c pong.c -lm -lpdcurses -Wall -O3 -o pong```

There is also a `mandafractal.c` mandelbrot explorer program that can be compiled the same way as `pong.c`

CAM should be compatible with any curses library.

More improvements to come soon, including circle and line algorithms.

See https://pasteboard.co/IiuKu8y.png for terminal pong screenshot.

## How to use

CAM interfaces with curses-type libraries to render the ascii-matched screen. These "virtual screens" are defined using the `CAM_screen` datatype. A `CAM_screen` contains the width of the virtual screen, a height, the window to which it should print, the current characters/colors being displayed, and the contents of the virtual screen which has been rendered to.

The width and height of the screen are initially defined in characters. The width of the virtual screen is 8 times the width of the screen in characters. Likewise, the height of the virtual screen is 13 times the height of the screen in characters. These dimensions cannot be changed for the sake of optimizations.

## Functions
```unsigned char CAM_init(unsigned int start_color);```

Initializes CAM. CAM needs to define 64 color pairs (for the 8x8=64 color combinations) to render the characters in color. `start_color` defines the first color pair available to CAM to use. CAM will populate color pairs `start_color`, `start_color + 1`, ... `start_color + 63`. For instance, if `CAM_init(1)` is called, then color pairs `1`, `2`, ... all the way to `64` will be used by CAM. Remember that color pair `0` is an invalid color pair, so `start_color` should be at least 1. If an error is encountered when initializing CAM, a non-zero value is returned. Otherwise, `0` is returned.

```CAM_screen *CAM_screen_create(WINDOW *parent, unsigned int char_width, unsigned int char_height);```

Creates a `CAM_screen` and returns a pointer to the allocated screen. This handle is used to draw to the created screen in all subsequent updates to the screen. This handle should be freed with the `CAM_screen_free` function. `parent` defines the curses window to which the text should be rendered. If using the entire screen, this value can be `stdscr`. `char_width` is the number of characters wide the screen should be. The pixel width of the screen will be `8*char_width`. `char_height` is the number of characters tall the screen should be. The pixel height of the screen will be `13*char_height`.

```void CAM_screen_free(CAM_screen *s);```

Frees the `CAM_screen` associated with the handle `s`. Must be called before the end of the program to free the memory in use by a `CAM_screen`.

```void CAM_set_pix(CAM_screen *s, unsigned int x, unsigned int y, unsigned char color);```

Sets a pixel on the screen `s`. `x` is the x coordinate of the pixel to be set on the virtual screen. `y` is the y coordinate of the pixel to be set. The coordinates are not given in character coordinates, but virtual screen coordinates. That means that the coordinate `(2, 2)` is still a pixel within the top left character. `color` is the color that the pixel should be set to. Valid colors are colors `0` through `7`. One can use the constants such as `COLOR_WHITE` given by the curses library to define the color. If the coordinate lies off of the screen, the results of this function are undefined.

```unsigned char CAM_get_pix(CAM_screen *s, unsigned int x, unsigned int y);```

Returns the color of a pixel on screen `s`. `x` is the x coordinate of the pixel to be tested, and `y` is the y coordinate. The colors of the pixels are undefined when the screen is created, even if the screen displays as black when the screen is updated. A pixel's color is only defined once it has been drawn to using a function such as `CAM_fill`.

```void CAM_fill(CAM_screen *s, unsigned int color);```

Fills a screen `s` with the color `color`. This function is much more efficient than calling `CAM_rect` to fill the entire screen. This can be used at the beginning of a frame update for a game to clear the screen before redrawing.

```void CAM_fill_char(CAM_screen *s, unsigned int char_x, unsigned int char_y, unsigned char color);```

Fills a character at position `char_x` and `char_y` with the color `color`. Note that this is a character position, not a virtual screen position, which means that the coordinate `(1, 0)` would fill the character directly to the right of the top left character.

```void CAM_rect(CAM_screen *s, int x0, int y0, int x1,  int y1, unsigned char color);```

Fills an axis-aligned rectangle in screen `s` with color `color`. The top left corner is defined by the x coordinate `x0` and the y coordinate `y0`. The bottom right corner is defined by the x coordinate `x1` and the y coordinate `y1`. If any of the rectangle lies off of the screen, the results of this function are undefined.

```void CAM_update_char(CAM_screen *s, unsigned int char_x, unsigned int char_y);```

Matches a character on screen `s` at character coordinates `char_x` and `char_y`. Matching is done by best fit; the character, foreground, and background color are chosen from the displayable characters such that the number of pixels on the virtual screen which match the pixels displayed on the console (as far as the program can guess, see `font.h`) is maximized. 

```void CAM_update(CAM_screen *s);```

For a given screen `s`, the function updates every character using `CAM_update_char` and displays the characters in the parent `WINDOW` starting from the top left character. The window still needs to be refreshed using the curses library to display the results.
