/*
  SDL2 OpenGL backend -- AmigaOS 3.x minigl.library

  The current MiniGL API creates and owns the native Intuition Window as part
  of mglCreateContext(). SDL therefore allocates its SDL_Window first, and this
  backend attaches the MiniGL-created Window to OS3_WindowData after context
  creation. No MiniGL API extension is required.
*/

#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_AMIGAOS3 && defined(SDL_VIDEO_OPENGL)

#include <proto/minigl.h>

#include "SDL_os3video.h"
#include "SDL_os3window.h"
#include "SDL_os3opengl.h"

static int os3_minigl_open = 0;
static int os3_swap_interval = 0;

static void OS3_GL_SelectContext(SDL_GLContext context)
{
    if (MiniGLDispatch && MiniGLDispatch->currentContext) {
        *MiniGLDispatch->currentContext = (GLcontext)context;
    }
}

void OS3_GL_DefaultProfileConfig(_THIS, int *mask, int *major, int *minor)
{
    (void)_this;
    if (mask)  *mask = 0;
    if (major) *major = 1;
    if (minor) *minor = 2;
}

int OS3_GL_LoadLibrary(_THIS, const char *path)
{
    (void)_this;
    (void)path;

    if (os3_minigl_open) {
        return 0;
    }

    if (!MiniGLOpen()) {
        return SDL_SetError("OS3: cannot open minigl.library V12+");
    }

    os3_minigl_open = 1;
    return 0;
}

void OS3_GL_UnloadLibrary(_THIS)
{
    (void)_this;

    if (os3_minigl_open) {
        MiniGLClose();
        os3_minigl_open = 0;
    }
}

SDL_GLContext OS3_GL_CreateContext(_THIS, SDL_Window *window)
{
    OS3_WindowData *data;
    SDL_GLContext context;
    struct Window *iwin;
    int depth = 32;

    if (!window || !window->driverdata) {
        SDL_SetError("OS3: invalid SDL window for MiniGL context");
        return NULL;
    }

    data = (OS3_WindowData *)window->driverdata;
    if (data->gl_context) {
        SDL_SetError("OS3: this SDL window already has a MiniGL context");
        return NULL;
    }

    if (!os3_minigl_open) {
        if (OS3_GL_LoadLibrary(_this, NULL) < 0) {
            return NULL;
        }
    }

    /* Prefer the current display's native RTG depth when it is known. */
    if (window->display_index < _this->num_displays) {
        int bpp = SDL_BITSPERPIXEL(_this->displays[window->display_index].current_mode.format);
        if (bpp > 0) {
            depth = (bpp <= 16) ? 16 : 32;
        }
    }

    mglChoosePixelDepth(depth);
    mglChooseNumberOfBuffers(_this->gl_config.double_buffer ? 2 : 1);
    mglChooseWindowMode((window->flags & SDL_WINDOW_FULLSCREEN) ? GL_FALSE : GL_TRUE);

    context = (SDL_GLContext)mglCreateContext(0, 0, window->w, window->h);
    if (!context) {
        SDL_SetError("OS3: mglCreateContext(%d,%d) failed", window->w, window->h);
        return NULL;
    }

    iwin = (struct Window *)mglGetWindowHandle();
    if (!iwin) {
        mglDeleteContext();
        SDL_SetError("OS3: MiniGL context returned no Intuition window");
        return NULL;
    }

    data->window = iwin;
    data->screen = iwin->WScreen;
    data->is_opengl = 1;
    data->minigl_owns_window = 1;
    data->gl_context = context;
    data->is_fullscreen = (window->flags & SDL_WINDOW_FULLSCREEN) ? 1 : 0;

    /* SDL owns event translation. Subscribe the MiniGL Window to the same
     * IDCMP classes used by the normal AmigaOS3 SDL window backend. */
    ModifyIDCMP(iwin, data->is_fullscreen ? OS3_IDCMP_FULLSCREEN : OS3_IDCMP_WINDOWED);
    SetWindowTitles(iwin,
                    (CONST_STRPTR)(window->title ? window->title : "SDL"),
                    (CONST_STRPTR)~0UL);

    OS3_GL_SelectContext(context);
    return context;
}

int OS3_GL_MakeCurrent(_THIS, SDL_Window *window, SDL_GLContext context)
{
    OS3_WindowData *data;
    (void)_this;

    if (!context) {
        OS3_GL_SelectContext(NULL);
        return 0;
    }

    if (!window || !window->driverdata) {
        return SDL_SetError("OS3: MiniGL context requires its SDL window");
    }

    data = (OS3_WindowData *)window->driverdata;
    if (data->gl_context != context) {
        return SDL_SetError("OS3: MiniGL context belongs to a different SDL window");
    }

    OS3_GL_SelectContext(context);
    return 0;
}

void OS3_GL_GetDrawableSize(_THIS, SDL_Window *window, int *w, int *h)
{
    OS3_WindowData *data = window ? (OS3_WindowData *)window->driverdata : NULL;
    (void)_this;

    if (data && data->window) {
        if (w) *w = data->window->Width - data->window->BorderLeft - data->window->BorderRight;
        if (h) *h = data->window->Height - data->window->BorderTop - data->window->BorderBottom;
    } else {
        if (w) *w = window ? window->w : 0;
        if (h) *h = window ? window->h : 0;
    }
}

void OS3_GL_ResizeWindow(_THIS, SDL_Window *window, int w, int h)
{
    OS3_WindowData *data = window ? (OS3_WindowData *)window->driverdata : NULL;
    (void)_this;

    if (!data || !data->gl_context || w <= 0 || h <= 0) {
        return;
    }

    OS3_GL_SelectContext((SDL_GLContext)data->gl_context);
    mglResizeContext((GLsizei)w, (GLsizei)h);
}

int OS3_GL_SetSwapInterval(_THIS, int interval)
{
    (void)_this;

    /* Current MiniGL API has no explicit swap-interval control. Accept 0 and
     * 1 so SDL applications that request ordinary vsync/no-vsync can run;
     * mglSwitchDisplay() retains the backend's native presentation behavior. */
    if (interval != 0 && interval != 1) {
        return SDL_SetError("OS3: MiniGL supports swap interval 0 or 1 only");
    }
    os3_swap_interval = interval;
    return 0;
}

int OS3_GL_GetSwapInterval(_THIS)
{
    (void)_this;
    return os3_swap_interval;
}

int OS3_GL_SwapWindow(_THIS, SDL_Window *window)
{
    OS3_WindowData *data;
    (void)_this;

    if (!window || !window->driverdata) {
        return SDL_SetError("OS3: invalid SDL window for MiniGL swap");
    }

    data = (OS3_WindowData *)window->driverdata;
    if (!data->gl_context) {
        return SDL_SetError("OS3: SDL window has no MiniGL context");
    }

    OS3_GL_SelectContext((SDL_GLContext)data->gl_context);
    mglSwitchDisplay();
    return 0;
}

void OS3_GL_DeleteContext(_THIS, SDL_GLContext context)
{
    SDL_Window *window;
    (void)_this;

    if (!context) {
        return;
    }

    /* Clear the association before MiniGL destroys its native window. */
    for (window = _this->windows; window; window = window->next) {
        OS3_WindowData *data = (OS3_WindowData *)window->driverdata;
        if (data && data->gl_context == context) {
            data->gl_context = NULL;
            data->window = NULL;
            data->screen = NULL;
            data->minigl_owns_window = 0;
            break;
        }
    }

    OS3_GL_SelectContext(context);
    mglDeleteContext();
}

/* The MiniGL SDK exposes GL entry points as static inline dispatch wrappers.
 * Taking their address here gives SDL_GL_GetProcAddress() callable functions
 * whose implementation still dispatches through the current MiniGL context. */
typedef struct OS3_GLProcEntry {
    const char *name;
    void *address;
} OS3_GLProcEntry;

#define OS3_GLPROC(fn) { #fn, (void *)(fn) }

static const OS3_GLProcEntry os3_gl_procs[] = {
    OS3_GLPROC(glActiveTextureARB),
    OS3_GLPROC(glAlphaFunc),
    OS3_GLPROC(glArrayElement),
    OS3_GLPROC(glBegin),
    OS3_GLPROC(glBindTexture),
    OS3_GLPROC(glBlendFunc),
    OS3_GLPROC(glClear),
    OS3_GLPROC(glClearColor),
    OS3_GLPROC(glClearDepth),
    OS3_GLPROC(glColor3f),
    OS3_GLPROC(glColor3fv),
    OS3_GLPROC(glColor3ub),
    OS3_GLPROC(glColor3ubv),
    OS3_GLPROC(glColor4f),
    OS3_GLPROC(glColor4fv),
    OS3_GLPROC(glColor4ub),
    OS3_GLPROC(glColor4ubv),
    OS3_GLPROC(glColorMask),
    OS3_GLPROC(glColorPointer),
    OS3_GLPROC(glColorTable),
    OS3_GLPROC(glColorTableEXT),
    OS3_GLPROC(glCullFace),
    OS3_GLPROC(glDeleteTextures),
    OS3_GLPROC(glDepthFunc),
    OS3_GLPROC(glDepthMask),
    OS3_GLPROC(glDepthRange),
    OS3_GLPROC(glDisable),
    OS3_GLPROC(glDisableClientState),
    OS3_GLPROC(glDrawArrays),
    OS3_GLPROC(glDrawBuffer),
    OS3_GLPROC(glDrawElements),
    OS3_GLPROC(glEnable),
    OS3_GLPROC(glEnableClientState),
    OS3_GLPROC(glEnd),
    OS3_GLPROC(glFinish),
    OS3_GLPROC(glFlush),
    OS3_GLPROC(glFogf),
    OS3_GLPROC(glFogfv),
    OS3_GLPROC(glFogi),
    OS3_GLPROC(glFrontFace),
    OS3_GLPROC(glFrustum),
    OS3_GLPROC(glGenTextures),
    OS3_GLPROC(glGetBooleanv),
    OS3_GLPROC(glGetError),
    OS3_GLPROC(glGetFloatv),
    OS3_GLPROC(glGetIntegerv),
    OS3_GLPROC(glGetString),
    OS3_GLPROC(glHint),
    OS3_GLPROC(glIsEnabled),
    OS3_GLPROC(glLoadIdentity),
    OS3_GLPROC(glLoadMatrixd),
    OS3_GLPROC(glLoadMatrixf),
    OS3_GLPROC(glLockArrays),
    OS3_GLPROC(glMatrixMode),
    OS3_GLPROC(glMultiTexCoord2fARB),
    OS3_GLPROC(glMultiTexCoord2fvARB),
    OS3_GLPROC(glMultMatrixd),
    OS3_GLPROC(glMultMatrixf),
    OS3_GLPROC(glNormal3f),
    OS3_GLPROC(glOrtho),
    OS3_GLPROC(glPixelStorei),
    OS3_GLPROC(glPointSize),
    OS3_GLPROC(glPolygonMode),
    OS3_GLPROC(glPolygonOffset),
    OS3_GLPROC(glPopMatrix),
    OS3_GLPROC(glPushMatrix),
    OS3_GLPROC(glReadPixels),
    OS3_GLPROC(glRotated),
    OS3_GLPROC(glRotatef),
    OS3_GLPROC(glRotatefEXT),
    OS3_GLPROC(glRotatefEXTs),
    OS3_GLPROC(glScaled),
    OS3_GLPROC(glScalef),
    OS3_GLPROC(glScissor),
    OS3_GLPROC(glShadeModel),
    OS3_GLPROC(glTexCoord2f),
    OS3_GLPROC(glTexCoord2fv),
    OS3_GLPROC(glTexCoord4f),
    OS3_GLPROC(glTexCoord4fv),
    OS3_GLPROC(glTexCoordPointer),
    OS3_GLPROC(glTexEnvf),
    OS3_GLPROC(glTexEnvfv),
    OS3_GLPROC(glTexEnvi),
    OS3_GLPROC(glTexEnviv),
    OS3_GLPROC(glTexGeni),
    OS3_GLPROC(glTexImage2D),
    OS3_GLPROC(glTexParameterf),
    OS3_GLPROC(glTexParameteri),
    OS3_GLPROC(glTexSubImage2D),
    OS3_GLPROC(glTranslated),
    OS3_GLPROC(glTranslatef),
    OS3_GLPROC(glUnlockArrays),
    OS3_GLPROC(glVertex2f),
    OS3_GLPROC(glVertex2fv),
    OS3_GLPROC(glVertex3f),
    OS3_GLPROC(glVertex3fv),
    OS3_GLPROC(glVertex4f),
    OS3_GLPROC(glVertex4fv),
    OS3_GLPROC(glVertexPointer),
    OS3_GLPROC(glViewport),
    OS3_GLPROC(gluLookAt),
    OS3_GLPROC(gluPerspective),
    { NULL, NULL }
};

void *OS3_GL_GetProcAddress(_THIS, const char *proc)
{
    const OS3_GLProcEntry *entry;
    (void)_this;

    if (!proc) {
        return NULL;
    }

    for (entry = os3_gl_procs; entry->name; ++entry) {
        if (SDL_strcmp(entry->name, proc) == 0) {
            return entry->address;
        }
    }

    SDL_SetError("OS3: MiniGL entry point '%s' is not available", proc);
    return NULL;
}

#endif /* SDL_VIDEO_DRIVER_AMIGAOS3 && SDL_VIDEO_OPENGL */
