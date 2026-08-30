# AmigaOS3 SDL2 MiniGL backend (experimental)

This tree adds classic OpenGL support to the AmigaOS3 SDL2 video driver through
`minigl.library`, without extending the current MiniGL API.

## Current model

MiniGL currently creates and owns the native Intuition Window together with its
GL context. Therefore an `SDL_WINDOW_OPENGL` window is created in two stages:

1. `OS3_CreateWindow()` creates only SDL/driver bookkeeping.
2. `SDL_GL_CreateContext()` calls `mglCreateContext()`.
3. `mglGetWindowHandle()` is stored in `OS3_WindowData` so SDL's existing IDCMP
   event pump can use the MiniGL-created window.

Software SDL windows continue to use the existing CyberGraphX/P96/AGA path.

## Implemented SDL GL hooks

- GL_LoadLibrary / GL_UnloadLibrary -> MiniGLOpen / MiniGLClose
- GL_CreateContext -> mglCreateContext
- GL_DeleteContext -> mglDeleteContext
- GL_MakeCurrent -> MiniGLDispatch->currentContext
- GL_SwapWindow -> mglSwitchDisplay
- GL_GetDrawableSize
- GL_SetSwapInterval / GL_GetSwapInterval (0/1 accepted; MiniGL has no explicit control)
- GL_GetProcAddress for all entry points present in the supplied MiniGL dispatch SDK
- GL_DefaultProfileConfig -> OpenGL 1.2

## Important limitations

- The MiniGL public SDK currently depends on the separately installed `mgl/`
  headers/backend header layout. This tree assumes those headers are already in
  the compiler include path, as in the current MiniGL development setup.
- MiniGL owns the native GL window. Runtime switching of an existing GL window
  between fullscreen and windowed mode cannot be implemented correctly until a
  MiniGL attach-window/recreate-window API exists. Initial windowed/fullscreen
  selection is passed to `mglChooseWindowMode()` before context creation.
- A MiniGL context is tied to the SDL_Window that created it. `SDL_GL_MakeCurrent`
  can switch between contexts, but does not move one context to another window.
- `SDL_GL_SetSwapInterval(0/1)` records the requested value only; presentation
  behavior remains whatever `mglSwitchDisplay()`/the backend provides.
- This adds an OpenGL window/context backend; it does not add an SDL_Renderer
  hardware-accelerated OpenGL renderer. `SDL_CreateRenderer` remains software.
- MiniGL support is enabled for the 68k Makefile only. `Makefile.wos` remains
  buildable but does not enable this 68k `minigl.library` backend.

## Build

Normal library build (existing project workflow):

    make -f Makefile

or inside the toolchain environment:

    make -f Makefile

The static library intentionally does not embed `libminigl.a`. Applications that
use OpenGL must link MiniGL as well.

## GL smoke test

No data files are required. Build with:

    make -f Makefile gl-example

or inside the toolchain environment:

    make -f Makefile gl-example

This creates:

    examples/test_gl_minigl

The program opens a 640x480 SDL_WINDOW_OPENGL window, creates a MiniGL context,
renders a rotating RGB triangle and swaps with SDL_GL_SwapWindow(). Escape or a
close-window event exits.


## Local toolchain

The 68k Makefile builds directly with the locally installed `m68k-amigaos-gcc`, `m68k-amigaos-ar`, and `m68k-amigaos-ranlib`. Docker is not used by the main SDL2/MiniGL build. Run `make -f Makefile setup-toolchain` to check that the required commands are in `PATH`.
