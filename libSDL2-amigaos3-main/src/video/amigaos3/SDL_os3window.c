#include "SDL_internal.h"

#if SDL_VIDEO_DRIVER_AMIGAOS3

#include "SDL_os3window.h"
#include "SDL_os3modes.h"
#if !defined(SDL_AMIGAOS3_SW_ONLY)
#include <proto/minigl.h>
#endif

static bool OS3_SetupData(SDL_Window *window, struct Window *syswin)
{
    SDL_WindowData *d = (SDL_WindowData *)SDL_calloc(1, sizeof(*d));
    if (!d) {
        return SDL_OutOfMemory();
    }

    d->sdlwin = window;
    d->syswin = syswin;
    d->screen = syswin ? syswin->WScreen : NULL;
    window->internal = d;
    return true;
}

static struct Window *OS3_OpenSystemWindow(SDL_VideoDevice *_this,
                                           SDL_Window *window,
                                           struct Screen *screen,
                                           bool fullscreen)
{
    struct Window *w;

    if (!screen) {
        SDL_SetError("AmigaOS3: no screen available");
        return NULL;
    }

    if (fullscreen) {
        w = OpenWindowTags(NULL,
            WA_CustomScreen, screen,
            WA_Left, 0,
            WA_Top, 0,
            WA_Width, screen->Width,
            WA_Height, screen->Height,
            WA_Title, window->title ? window->title : "SDL3",
            WA_IDCMP, OS3_IDCMP_FULLSCREEN,
            WA_Borderless, TRUE,
            WA_Backdrop, TRUE,
            WA_Activate, TRUE,
            WA_RMBTrap, TRUE,
            WA_ReportMouse, TRUE,
            WA_SimpleRefresh, TRUE,
            TAG_DONE);
    } else {
        SDL_VideoData *vd = (SDL_VideoData *)_this->internal;
        int inner_w = window->windowed.w > 0 ? window->windowed.w : window->w;
        int inner_h = window->windowed.h > 0 ? window->windowed.h : window->h;
        int left = window->windowed.x;
        int top = window->windowed.y;
        int max_inner_w;
        int max_inner_h;

        if (!vd || !vd->publicScreen) {
            SDL_SetError("AmigaOS3: no public screen available");
            return NULL;
        }

        max_inner_w = (int)screen->Width -
                      (int)screen->WBorLeft -
                      (int)screen->WBorRight;
        max_inner_h = (int)screen->Height -
                      (int)screen->WBorTop -
                      (int)screen->WBorBottom -
                      (int)screen->Font->ta_YSize - 1;
        if (max_inner_w < 64) max_inner_w = 64;
        if (max_inner_h < 64) max_inner_h = 64;
        if (inner_w > max_inner_w) inner_w = max_inner_w;
        if (inner_h > max_inner_h) inner_h = max_inner_h;

        if (left < 0 || left + inner_w > (int)screen->Width) left = 0;
        if (top < 0 || top + inner_h > (int)screen->Height) top = 0;

        w = OpenWindowTags(NULL,
            WA_CustomScreen, screen,
            WA_Left, left, WA_Top, top,
            WA_InnerWidth, inner_w, WA_InnerHeight, inner_h,
            WA_Title, window->title ? window->title : "SDL3",
            WA_IDCMP, OS3_IDCMP_WINDOWED,
            WA_Borderless, (window->flags & SDL_WINDOW_BORDERLESS) ? TRUE : FALSE,
            WA_CloseGadget, (window->flags & SDL_WINDOW_BORDERLESS) ? FALSE : TRUE,
            WA_DepthGadget, (window->flags & SDL_WINDOW_BORDERLESS) ? FALSE : TRUE,
            WA_DragBar, (window->flags & SDL_WINDOW_BORDERLESS) ? FALSE : TRUE,
            WA_SizeGadget, ((window->flags & SDL_WINDOW_RESIZABLE) &&
                            !(window->flags & SDL_WINDOW_BORDERLESS)) ? TRUE : FALSE,
            WA_Activate, TRUE,
            WA_ReportMouse, TRUE,
            WA_SimpleRefresh, TRUE,
            WA_AutoAdjust, TRUE,
            TAG_DONE);
    }

    return w;
}

bool OS3_CreateWindow(SDL_VideoDevice *_this, SDL_Window *window, SDL_PropertiesID props)
{
    SDL_VideoData *vd = (SDL_VideoData *)_this->internal;
    struct Window *w;
    (void)props;

    /*
     * MiniGL owns its native Window. SDL only allocates per-window data here;
     * OS3_GL_CreateContext() creates the Window later.
     */
    if (window->flags & SDL_WINDOW_OPENGL) {
        return OS3_SetupData(window, NULL);
    }

    if (!vd || !vd->publicScreen) {
        return SDL_SetError("AmigaOS3: no public screen available");
    }

    w = OS3_OpenSystemWindow(_this, window, vd->publicScreen, false);
    if (!w) {
        return false;
    }

    window->x = w->LeftEdge;
    window->y = w->TopEdge;
    window->w = w->GZZWidth;
    window->h = w->GZZHeight;

    return OS3_SetupData(window, w);
}

void OS3_DestroyWindow(SDL_VideoDevice *_this, SDL_Window *window)
{
    SDL_WindowData *d = window ? window->internal : NULL;
    (void)_this;

    if (!d) {
        return;
    }

    if (d->syswin && !d->minigl_owns_window) {
        CloseWindow(d->syswin);
    }

    SDL_free(d);
    window->internal = NULL;
}


static void OS3_ApplyWindowLimits(SDL_Window *window)
{
    SDL_WindowData *d = window ? window->internal : NULL;
    int minw = 1, minh = 1, maxw = -1, maxh = -1;

    if (!d || !d->syswin || d->minigl_owns_window) return;

    if (window->min_w > 0) minw = window->min_w + d->syswin->BorderLeft + d->syswin->BorderRight;
    if (window->min_h > 0) minh = window->min_h + d->syswin->BorderTop + d->syswin->BorderBottom;
    if (window->max_w > 0) maxw = window->max_w + d->syswin->BorderLeft + d->syswin->BorderRight;
    if (window->max_h > 0) maxh = window->max_h + d->syswin->BorderTop + d->syswin->BorderBottom;

    WindowLimits(d->syswin, minw, minh, maxw, maxh);
}

bool OS3_SetWindowPosition(SDL_VideoDevice *_this, SDL_Window *window)
{
    SDL_WindowData *d = window ? window->internal : NULL;
    (void)_this;
    if (!d || !d->syswin) return false;
    if (d->minigl_owns_window) return SDL_SetError("AmigaOS3: moving a MiniGL-owned window is unsupported");

    MoveWindow(d->syswin,
               window->pending.x - d->syswin->LeftEdge,
               window->pending.y - d->syswin->TopEdge);
    SDL_SendWindowEvent(window, SDL_EVENT_WINDOW_MOVED, d->syswin->LeftEdge, d->syswin->TopEdge);
    return true;
}

void OS3_SetWindowSize(SDL_VideoDevice *_this, SDL_Window *window)
{
    SDL_WindowData *d = window ? window->internal : NULL;
    int cw, ch;
    (void)_this;
    if (!d || !d->syswin) return;

    if (d->minigl_owns_window) {
#if !defined(SDL_AMIGAOS3_SW_ONLY)
        if (d->gl_context) mglResizeContext(window->pending.w, window->pending.h);
#endif
        return;
    }

    cw = d->syswin->GZZWidth;
    ch = d->syswin->GZZHeight;
    SizeWindow(d->syswin, window->pending.w - cw, window->pending.h - ch);
}

void OS3_SetWindowMinMaxSize(SDL_VideoDevice *_this, SDL_Window *window)
{
    (void)_this;
    OS3_ApplyWindowLimits(window);
}

bool OS3_GetWindowBordersSize(SDL_VideoDevice *_this, SDL_Window *window,
                              int *top, int *left, int *bottom, int *right)
{
    SDL_WindowData *d = window ? window->internal : NULL;
    (void)_this;
    if (!d || !d->syswin) return false;
    if (top) *top = d->syswin->BorderTop;
    if (left) *left = d->syswin->BorderLeft;
    if (bottom) *bottom = d->syswin->BorderBottom;
    if (right) *right = d->syswin->BorderRight;
    return true;
}

void OS3_MaximizeWindow(SDL_VideoDevice *_this, SDL_Window *window)
{
    SDL_WindowData *d = window ? window->internal : NULL;
    struct Screen *s;
    int w, h;
    (void)_this;
    if (!d || !d->syswin || d->minigl_owns_window) return;
    s = d->syswin->WScreen;
    w = s->Width - d->syswin->BorderLeft - d->syswin->BorderRight;
    h = s->Height - d->syswin->BorderTop - d->syswin->BorderBottom;
    MoveWindow(d->syswin, -d->syswin->LeftEdge, -d->syswin->TopEdge);
    SizeWindow(d->syswin, w - d->syswin->GZZWidth, h - d->syswin->GZZHeight);
    SDL_SendWindowEvent(window, SDL_EVENT_WINDOW_MAXIMIZED, 0, 0);
}

void OS3_MinimizeWindow(SDL_VideoDevice *_this, SDL_Window *window)
{
    SDL_WindowData *d = window ? window->internal : NULL;
    (void)_this;
    if (!d || !d->syswin) return;

    /* Classic Intuition has no generic hide/iconify call without Workbench
       AppIcon plumbing. WindowToBack is the safe native approximation. */
    WindowToBack(d->syswin);
    SDL_SendWindowEvent(window, SDL_EVENT_WINDOW_MINIMIZED, 0, 0);
}

void OS3_RestoreWindow(SDL_VideoDevice *_this, SDL_Window *window)
{
    SDL_WindowData *d = window ? window->internal : NULL;
    (void)_this;
    if (!d || !d->syswin) return;

    if ((window->flags & SDL_WINDOW_MAXIMIZED) && !d->minigl_owns_window) {
        MoveWindow(d->syswin,
                   window->floating.x - d->syswin->LeftEdge,
                   window->floating.y - d->syswin->TopEdge);
        SizeWindow(d->syswin,
                   window->floating.w - d->syswin->GZZWidth,
                   window->floating.h - d->syswin->GZZHeight);
    }

    WindowToFront(d->syswin);
    ActivateWindow(d->syswin);
    SDL_SendWindowEvent(window, SDL_EVENT_WINDOW_RESTORED, 0, 0);
}

static void OS3_RecreateWindow(SDL_VideoDevice *_this, SDL_Window *window)
{
    SDL_WindowData *d = window ? window->internal : NULL;
    SDL_VideoData *vd = (SDL_VideoData *)_this->internal;
    struct Window *nw;

    if (!d || !d->syswin || d->minigl_owns_window || (window->flags & SDL_WINDOW_FULLSCREEN)) {
        return;
    }

    if (window->surface) SDL_DestroyWindowSurface(window);
    CloseWindow(d->syswin);
    d->syswin = NULL;

    nw = OS3_OpenSystemWindow(_this, window, vd->publicScreen, false);
    if (nw) {
        d->syswin = nw;
        d->screen = nw->WScreen;
        OS3_ApplyWindowLimits(window);
    }
}

void OS3_SetWindowBordered(SDL_VideoDevice *_this, SDL_Window *window, bool bordered)
{
    (void)bordered;
    OS3_RecreateWindow(_this, window);
}

void OS3_SetWindowResizable(SDL_VideoDevice *_this, SDL_Window *window, bool resizable)
{
    (void)resizable;
    OS3_RecreateWindow(_this, window);
}

void OS3_SetWindowAlwaysOnTop(SDL_VideoDevice *_this, SDL_Window *window, bool on_top)
{
    SDL_WindowData *d = window ? window->internal : NULL;
    (void)_this;
    if (d && d->syswin && on_top) WindowToFront(d->syswin);
}

bool OS3_SetWindowMouseGrab(SDL_VideoDevice *_this, SDL_Window *window, bool grabbed)
{
    SDL_WindowData *d = window ? window->internal : NULL;
    (void)_this;
    if (!d || !d->syswin) return false;

    /*
     * Classic Intuition has no universal pointer confinement API.
     * RMBTrap + focus retention are still useful to games; true confinement
     * can be layered on input.device later.
     */
    if (grabbed) {
        d->syswin->Flags |= WFLG_RMBTRAP;
        ActivateWindow(d->syswin);
    } else {
        d->syswin->Flags &= ~WFLG_RMBTRAP;
    }
    return true;
}

bool OS3_SetWindowKeyboardGrab(SDL_VideoDevice *_this, SDL_Window *window, bool grabbed)
{
    SDL_WindowData *d = window ? window->internal : NULL;
    (void)_this;
    if (!d || !d->syswin) return false;
    if (grabbed) ActivateWindow(d->syswin);
    return true;
}

SDL_FullscreenResult OS3_SetWindowFullscreen(SDL_VideoDevice *_this,
                                              SDL_Window *window,
                                              SDL_VideoDisplay *display,
                                              SDL_FullscreenOp op)
{
    SDL_WindowData *d = window ? window->internal : NULL;
    SDL_VideoData *vd = (SDL_VideoData *)_this->internal;
    struct Window *w = NULL;
    struct Screen *screen = NULL;

    if (!d || !display) {
        SDL_SetError("AmigaOS3: invalid fullscreen transition");
        return SDL_FULLSCREEN_FAILED;
    }

    /*
     * MiniGL creates and owns both the GL window and fullscreen screen.
     * A GL window that has not got a context yet can safely enter/leave here;
     * OS3_GL_CreateContext() will inspect SDL_WINDOW_FULLSCREEN afterwards.
     * Recreating a live MiniGL context behind SDL's back would invalidate the
     * user's GL context, so don't do that.
     */
    if (window->flags & SDL_WINDOW_OPENGL) {
        if (d->gl_context) {
            if ((op == SDL_FULLSCREEN_OP_ENTER && !(window->flags & SDL_WINDOW_FULLSCREEN)) ||
                (op == SDL_FULLSCREEN_OP_LEAVE && (window->flags & SDL_WINDOW_FULLSCREEN))) {
                SDL_SetError("AmigaOS3: live MiniGL fullscreen transitions are not supported yet");
                return SDL_FULLSCREEN_FAILED;
            }
        }
        return SDL_FULLSCREEN_SUCCEEDED;
    }

    /* Any SDL window surface points at the old native window. */
    if (window->surface) {
        SDL_DestroyWindowSurface(window);
    }

    if (d->syswin) {
        CloseWindow(d->syswin);
        d->syswin = NULL;
        d->screen = NULL;
    }

    if (op == SDL_FULLSCREEN_OP_ENTER || op == SDL_FULLSCREEN_OP_UPDATE) {
        screen = OS3_OpenFullscreenScreen(_this, display);
        if (!screen) {
            return SDL_FULLSCREEN_FAILED;
        }

        w = OS3_OpenSystemWindow(_this, window, screen, true);
        if (!w) {
            OS3_CloseFullscreenScreen(_this, display);
            return SDL_FULLSCREEN_FAILED;
        }

        d->syswin = w;
        d->screen = screen;
        d->owns_screen = false; /* screen is owned by SDL_DisplayData */
        window->x = 0;
        window->y = 0;
        window->w = w->Width;
        window->h = w->Height;
        return SDL_FULLSCREEN_SUCCEEDED;
    }

    /* Leave fullscreen: native fullscreen window is gone, so Screen can close. */
    OS3_CloseFullscreenScreen(_this, display);

    if (!vd || !vd->publicScreen) {
        SDL_SetError("AmigaOS3: public screen unavailable while leaving fullscreen");
        return SDL_FULLSCREEN_FAILED;
    }

    w = OS3_OpenSystemWindow(_this, window, vd->publicScreen, false);
    if (!w) {
        return SDL_FULLSCREEN_FAILED;
    }

    d->syswin = w;
    d->screen = vd->publicScreen;
    window->x = w->LeftEdge;
    window->y = w->TopEdge;
    window->w = w->GZZWidth;
    window->h = w->GZZHeight;

    return SDL_FULLSCREEN_SUCCEEDED;
}

void OS3_SetWindowTitle(SDL_VideoDevice *_this, SDL_Window *window)
{
    SDL_WindowData *d = window->internal;
    (void)_this;
    if (d && d->syswin) {
        SetWindowTitles(d->syswin, (STRPTR)(window->title ? window->title : "SDL3"), (STRPTR)-1);
    }
}

void OS3_ShowWindow(SDL_VideoDevice *_this, SDL_Window *window)
{
    SDL_WindowData *d = window->internal;
    (void)_this;
    if (d && d->syswin) {
        WindowToFront(d->syswin);
        ActivateWindow(d->syswin);
    }
}

void OS3_HideWindow(SDL_VideoDevice *_this, SDL_Window *window)
{
    (void)_this;
    (void)window;
}

void OS3_RaiseWindow(SDL_VideoDevice *_this, SDL_Window *window)
{
    OS3_ShowWindow(_this, window);
}

#endif
