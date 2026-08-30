# SDL2_GFX
Graphics primitives and surface functions for SDL2

## Disclaimer
I am not the author of this library!


I am in no way associated with the author (Andreas "aschiffler" Schiffler).

The original page for the library can be found at: https://www.ferzkopp.net/wordpress/2016/01/02/sdl_gfx-sdl2_gfx/


I needed this library for a separate project but couldn't find a usable binary, so I decided to compile it myself and update it to work on 64 bits windows systems.
May also add some other features at some point, but no promises.

What follows is the original README.

## Contact and License
Email aschiffler at ferzkopp dot net to contact the author 
or better check author's homepage at http://www.ferzkopp.net 
for the most up-to-date contact information.

This library is licenced under the zlib License, see the file LICENSE for details. 

## Introduction
The SDL2_gfx library provides the basic drawing functions such as lines,
circles or polygons provided by SDL_gfx on SDL2 against renderers of SDL2.

The current components of the SDL2_gfx library are:
- Graphic Primitives (SDL2_gfxPrimitives.h, SDL2_gfxPrimitives.c)
- Surface Rotozoomer (SDL2_rotozoom.h, SDL2_rotozoom.c)
- Framerate control (SDL2_framerate.h, SDL2_framerate.c)
- MMX image filters (SDL2_imageFilter.h, SDL2_imageFilter.c)
- Build-in 8x8 Font (SDL2_gfxPrimitives_font.h)

Note that SDL2_gfx is compatible with SDL version 2.0 (not SDL 1.2).

## Installation

### Unix/Linux
Use the standard autoconf/automake sequence to compile and install the library.
```
        ./autogen.sh    # (optional, recommended)
        ./configure
        make
        make install
```

#### Linker Configuration
The default location for the installation is /usr/local/lib and /usr/local/include. 
This libary path may need to be added to the file the linker configuration file:
```
        vi /etc/ld.so.conf
        ldconfig
```

#### Non-MMX Platforms
To build without MMX code enabled (i.e. ARM, PPC, AMD64 architectures):
```
        ./configure --disable-mmx
        make
        make install
```

### Windows (VS2022)
Open the SDL2_gfx.sln solution file, right click on the solution and choose 'Rebuild'.

The SDL folder must be placed in a directory alongside SDL2_gfx (or sdl2gfx-code) and build in the same configuration, i.e. Debug or Release, beforehand so the referenced SDL2.lib file can be found.

### Mac OSX 
The usual autotools build chain should be used. MacPorts or fink may be required.

Xcode is supported via templates. See Xcode.zip - this template only supports SDL2_gfx 
and not the tests. For this template, the Deployment Target (the lowest version to run on) 
is set to 10.11 and expects the SDL2.framework preinstalled in the default location: /Library/Frameworks.

## Test Programs
Change to the ./test directory and run
```
        ./autogen.sh
        ./configure
        make
```
to create several test programs for the libraries functions. This requires
the SDL2_gfx library to be previously compiled and installed.

See the source in the test/\*.c files for some sample code and implementation hints.

## Documentation
Please refer to the Doxygen-generated API documentation found in the
Docs/html folder as well as the test programs in the test folder.

## Change Log
Please refer to the ChangeLog file.