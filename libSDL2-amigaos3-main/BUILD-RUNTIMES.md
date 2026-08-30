# Separate clib2 / libnix builds

The two 68k runtime variants use completely separate build trees.

## clib2

    make -f Makefile

Outputs:

    build/clib2/libSDL2.a
    build/clib2/libSDL2_test.a

MiniGL smoke test:

    make -f Makefile gl-example

Output:

    build/clib2/examples/test_gl_minigl

## libnix

    make -f Makefile.libnix

Outputs:

    build/libnix/libSDL2.a
    build/libnix/libSDL2_test.a

MiniGL smoke test:

    make -f Makefile.libnix gl-example

Output:

    build/libnix/examples/test_gl_minigl

## Both

    make -f Makefile.all

Both GL tests:

    make -f Makefile.all gl-examples

Clean both build trees:

    make -f Makefile.all clean

The source tree itself is no longer populated with runtime-specific .o files.
