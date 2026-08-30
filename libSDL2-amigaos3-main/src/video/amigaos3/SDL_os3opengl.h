/*
  SDL2 OpenGL backend -- AmigaOS 3.x minigl.library
*/
#ifndef SDL_os3opengl_h_
#define SDL_os3opengl_h_

#include "../../SDL_internal.h"
#include "../SDL_sysvideo.h"

#if SDL_VIDEO_DRIVER_AMIGAOS3 && defined(SDL_VIDEO_OPENGL)

extern int OS3_GL_LoadLibrary(_THIS, const char *path);
extern void OS3_GL_UnloadLibrary(_THIS);
extern void *OS3_GL_GetProcAddress(_THIS, const char *proc);
extern SDL_GLContext OS3_GL_CreateContext(_THIS, SDL_Window *window);
extern int OS3_GL_MakeCurrent(_THIS, SDL_Window *window, SDL_GLContext context);
extern void OS3_GL_GetDrawableSize(_THIS, SDL_Window *window, int *w, int *h);
extern int OS3_GL_SetSwapInterval(_THIS, int interval);
extern int OS3_GL_GetSwapInterval(_THIS);
extern int OS3_GL_SwapWindow(_THIS, SDL_Window *window);
extern void OS3_GL_DeleteContext(_THIS, SDL_GLContext context);
extern void OS3_GL_DefaultProfileConfig(_THIS, int *mask, int *major, int *minor);
extern void OS3_GL_ResizeWindow(_THIS, SDL_Window *window, int w, int h);

#endif
#endif
