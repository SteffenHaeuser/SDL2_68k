#include "SDL.h"
#include <stdio.h>

static double dabs_local(double x)
{
    return (x < 0.0) ? -x : x;
}

static int report(const char *name, double got, double expected, double tolerance)
{
    const double err = dabs_local(got - expected);
    const int ok = (err <= tolerance);

    /* Avoid %f/%g here; float printf itself is not part of this test. */
    printf("%-8s %s\n", name, ok ? "OK" : "FAIL");
    fflush(stdout);
    return ok ? 0 : 1;
}

#define RUN_TEST(label, expression, expected, tolerance) \
    do { \
        double value; \
        printf("TEST %-8s ...\n", label); \
        fflush(stdout); \
        value = (expression); \
        fails += report(label, value, expected, tolerance); \
    } while (0)

int main(int argc, char **argv)
{
    int fails = 0;
    const double pi = 3.1415926535897932384626433832795;
    (void)argc;
    (void)argv;

    puts("SDL2 AmigaOS3 math wrapper test");
    fflush(stdout);

    RUN_TEST("sin",    SDL_sin(pi / 6.0),              0.5,                0.00002);
    RUN_TEST("cos",    SDL_cos(pi / 3.0),              0.5,                0.00002);
    RUN_TEST("tan",    SDL_tan(pi / 4.0),              1.0,                0.000001);
    RUN_TEST("atan",   SDL_atan(1.0),                  pi / 4.0,           0.000001);
    RUN_TEST("atan2",  SDL_atan2(1.0, 1.0),            pi / 4.0,           0.000001);
    RUN_TEST("ceil",   SDL_ceil(1.25),                 2.0,                0.0);
    RUN_TEST("floor",  SDL_floor(1.75),                1.0,                0.0);
    RUN_TEST("fmod",   SDL_fmod(7.0, 2.0),             1.0,                0.000001);
    RUN_TEST("exp",    SDL_exp(1.0),                   2.718281828459045,  0.000001);
    RUN_TEST("log",    SDL_log(2.718281828459045),     1.0,                0.000001);
    RUN_TEST("log10",  SDL_log10(1000.0),              3.0,                0.000001);
    RUN_TEST("pow",    SDL_pow(2.0, 10.0),             1024.0,             0.000001);
    RUN_TEST("sqrt",   SDL_sqrt(2.0),                  1.4142135623730951, 0.000001);

    RUN_TEST("sin-",   SDL_sin(-pi / 2.0),            -1.0,                0.00002);
    RUN_TEST("cospi",  SDL_cos(pi),                    -1.0,                0.00002);
    RUN_TEST("atan2Q", SDL_atan2(1.0, -1.0),           3.0 * pi / 4.0,     0.000001);
    RUN_TEST("floor-", SDL_floor(-1.25),               -2.0,                0.0);
    RUN_TEST("ceil-",  SDL_ceil(-1.25),                -1.0,                0.0);

    printf("RESULT: %s (%d failures)\n", fails ? "FAIL" : "OK", fails);
    return fails ? 1 : 0;
}
