#include <SDL.h>
#include <SDL_opengl.h>

static void draw_triangle(float angle)
{
    glViewport(0, 0, 640, 480);
    glClearColor(0.06f, 0.08f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.4, 1.4, -1.05, 1.05, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(angle, 0.0f, 0.0f, 1.0f);

    glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.15f, 0.10f);
        glVertex2f( 0.0f,  0.75f);
        glColor3f(0.10f, 1.0f, 0.20f);
        glVertex2f(-0.70f, -0.55f);
        glColor3f(0.15f, 0.25f, 1.0f);
        glVertex2f( 0.70f, -0.55f);
    glEnd();
}

int main(int argc, char **argv)
{
    SDL_Window *window;
    SDL_GLContext context;
    SDL_Event event;
    int running = 1;
    float angle = 0.0f;
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return 10;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

    window = SDL_CreateWindow("SDL2 + minigl.library",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              640, 480,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 20;
    }

    context = SDL_GL_CreateContext(window);
    if (!context) {
        SDL_Log("SDL_GL_CreateContext failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 30;
    }

    SDL_GL_SetSwapInterval(0);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN &&
                       event.key.keysym.sym == SDLK_ESCAPE) {
                running = 0;
            }
        }

        draw_triangle(angle);
        SDL_GL_SwapWindow(window);

        angle += 1.0f;
        if (angle >= 360.0f) {
            angle -= 360.0f;
        }
        SDL_Delay(16);
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
