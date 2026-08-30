# AmigaOS 3.x notes

This port targets classic m68k AmigaOS with `bsdsocket.library` and IPv4 only.

- `SDLNet_Init()` opens `bsdsocket.library` v4 and installs a socket errno pointer.
- `SDLNet_Quit()` closes the library when the init reference count reaches zero.
- sockets are closed with `CloseSocket()`.
- nonblocking mode uses `IoctlSocket(..., FIONBIO, ...)`.
- socket polling uses `WaitSelect()` instead of POSIX `select()`.
- name resolution uses the classic IPv4 `inet_addr()`, `gethostbyname()`, `gethostbyaddr()` and `Inet_NtoA()` APIs.
- `SDLNet_GetLocalAddresses()` currently returns zero on AmigaOS; the normal TCP/UDP and resolver APIs are unaffected.
- IPv6 is not compiled or used.

Build both CRT variants:

    make -f Makefile.amigaos3

Or individually:

    make -f Makefile.amigaos3 clib2
    make -f Makefile.amigaos3 libnix

Outputs:

    build/clib2/lib/libSDL2_net.a
    build/libnix/lib/libSDL2_net_libnix.a
