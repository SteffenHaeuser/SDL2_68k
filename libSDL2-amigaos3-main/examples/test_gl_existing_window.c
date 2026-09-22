/* Target smoke test: SDL wraps an application-owned Intuition window. */
#include <SDL.h>
#include <SDL_opengl.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <intuition/screens.h>

int main(int argc, char **argv)
{
    struct Screen *screen = NULL;
    struct Window *native = NULL;
    struct IntuiMessage *msg;
    SDL_Window *window = NULL;
    SDL_GLContext context = NULL;
    int done = 0, result = 1, width, height;
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;
    screen = LockPubScreen(NULL);
    if (!screen) goto cleanup;
    native = OpenWindowTags(NULL,
        WA_PubScreen, (ULONG)screen,
        WA_InnerWidth, 320, WA_InnerHeight, 240,
        WA_Title, (ULONG)"Host-owned SDL2 MiniGL window",
        WA_DragBar, TRUE, WA_DepthGadget, TRUE,
        WA_CloseGadget, TRUE, WA_SizeGadget, TRUE,
        WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_NEWSIZE,
        TAG_DONE);
    if (!native) goto cleanup;

    SDL_SetHint(SDL_HINT_VIDEO_FOREIGN_WINDOW_OPENGL, "1");
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    window = SDL_CreateWindowFrom(native);
    if (!window) goto cleanup;
    context = SDL_GL_CreateContext(window);
    if (!context) goto cleanup;
    /* Deleting a context must preserve the native window and SDL wrapper. */
    SDL_GL_DeleteContext(context);
    context = SDL_GL_CreateContext(window);
    if (!context) goto cleanup;

    while (!done) {
        /* SDL must leave this queue, and its messages, with the host. */
        SDL_PumpEvents();
        while ((msg = (struct IntuiMessage *)GetMsg(native->UserPort))) {
            if (msg->Class == IDCMP_CLOSEWINDOW) done = 1;
            ReplyMsg((struct Message *)msg);
        }
        SDL_GL_GetDrawableSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.08f, 0.28f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(window);
        SDL_Delay(10);
    }
    result = 0;
cleanup:
    if (result) SDL_Log("Existing-window smoke test failed: %s", SDL_GetError());
    if (context) SDL_GL_DeleteContext(context);
    if (window) SDL_DestroyWindow(window);
    if (native) {
        /* Must remain valid after SDL wrapper/context destruction. */
        SetWindowTitles(native, (CONST_STRPTR)"SDL detached", (CONST_STRPTR)~0UL);
        CloseWindow(native);
    }
    if (screen) UnlockPubScreen(NULL, screen);
    SDL_Quit();
    return result;
}
