/*
  SDL2_image test (SDL2 API) — drop-in replacement for SDL3-style test.
  Uses IMG_SavePNG / IMG_SaveJPG instead of SDL_IMAGE_SAVE_* enums.
*/

#include "SDL_image.h"
#include "SDL.h"
#include "SDL_test.h"

#if defined(SDL_FILESYSTEM_OS2) || defined(SDL_FILESYSTEM_WINDOWS)
static const char pathsep[] = "\\";
#elif defined(SDL_FILESYSTEM_RISCOS)
static const char pathsep[] = ".";
#else
static const char pathsep[] = "/";
#endif

#if defined(__APPLE__) && !defined(SDL_IMAGE_USE_COMMON_BACKEND)
# define USING_IMAGEIO 1
#else
# define USING_IMAGEIO 0
#endif

typedef enum { TEST_FILE_DIST, TEST_FILE_BUILT } TestFileType;

static SDL_bool GetStringBoolean(const char *value, SDL_bool defv) {
    if (!value || !*value) return defv;
    if (*value == '0' || SDL_strcasecmp(value, "false") == 0) return SDL_FALSE;
    return SDL_TRUE;
}

static char *GetTestFilename(TestFileType type, const char *file) {
    const char *env;
    char *base = NULL, *path = NULL;
    SDL_bool needPathSep = SDL_TRUE;

    env = SDL_getenv(type == TEST_FILE_DIST ? "SDL_TEST_SRCDIR" : "SDL_TEST_BUILDDIR");
    if (env) {
        base = SDL_strdup(env);
        if (!base) { SDL_OutOfMemory(); return NULL; }
    }
    if (!base) {
        base = SDL_GetBasePath();
        needPathSep = SDL_FALSE; /* SDL_GetBasePath() guarantees trailing sep */
    }
    if (base) {
        size_t len = SDL_strlen(base) + SDL_strlen(pathsep) + SDL_strlen(file) + 1;
        path = (char*)SDL_malloc(len);
        if (!path) { SDL_OutOfMemory(); return NULL; }
        if (needPathSep) SDL_snprintf(path, len, "%s%s%s", base, pathsep, file);
        else             SDL_snprintf(path, len, "%s%s", base, file);
        SDL_free(base);
    } else {
        path = SDL_strdup(file);
        if (!path) { SDL_OutOfMemory(); return NULL; }
    }
    return path;
}

static SDLTest_CommonState *state;

typedef struct {
    const char *name;
    const char *sample;
    const char *reference;
    int w, h;
    int tolerance;
    int initFlag;
    SDL_bool canLoad;
    SDL_bool canSave; /* SDL2: only JPG saving enabled here */
    int (SDLCALL * checkFunction)(SDL_RWops *src);
    SDL_Surface *(SDLCALL * loadFunction)(SDL_RWops *src);
} Format;

static const Format formats[] = {
    { "AVIF", "sample.avif", "sample.bmp", 23, 42, 300, IMG_INIT_AVIF,
    #ifdef LOAD_AVIF
      SDL_TRUE,
    #else
      SDL_FALSE,
    #endif
      SDL_FALSE,
      IMG_isAVIF, IMG_LoadAVIF_RW },

    { "BMP", "sample.bmp", "sample.png", 23, 42, 0, 0,
    #ifdef LOAD_BMP
      SDL_TRUE,
    #else
      SDL_FALSE,
    #endif
      SDL_FALSE,
      IMG_isBMP, IMG_LoadBMP_RW },

    { "CUR", "sample.cur", "sample.bmp", 23, 42, 0, 0,
    #ifdef LOAD_BMP
      SDL_TRUE,
    #else
      SDL_FALSE,
    #endif
      SDL_FALSE,
      IMG_isCUR, IMG_LoadCUR_RW },

    { "GIF", "palette.gif", "palette.bmp", 23, 42, 0, 0,
    #if USING_IMAGEIO || defined(LOAD_GIF)
      SDL_TRUE,
    #else
      SDL_FALSE,
    #endif
      SDL_FALSE,
      IMG_isGIF, IMG_LoadGIF_RW },

    { "ICO", "sample.ico", "sample.bmp", 23, 42, 0, 0,
    #ifdef LOAD_BMP
      SDL_TRUE,
    #else
      SDL_FALSE,
    #endif
      SDL_FALSE,
      IMG_isICO, IMG_LoadICO_RW },

    { "JPG", "sample.jpg", "sample.bmp", 23, 42, 100, IMG_INIT_JPG,
    #if (USING_IMAGEIO && defined(JPG_USES_IMAGEIO)) || defined(SDL_IMAGE_USE_WIC_BACKEND) || defined(LOAD_JPG)
      SDL_TRUE,
    #else
      SDL_FALSE,
    #endif
      /* SDL2_image provides IMG_SaveJPG; enable saving when JPEG is built */
    #ifdef LOAD_JPG
      SDL_TRUE,
    #else
      SDL_FALSE,
    #endif
      IMG_isJPG, IMG_LoadJPG_RW },

    /* JXL intentionally omitted for determinism */

    { "PCX", "sample.pcx", "sample.bmp", 23, 42, 0, 0,
    #ifdef LOAD_PCX
      SDL_TRUE,
    #else
      SDL_FALSE,
    #endif
      SDL_FALSE,
      IMG_isPCX, IMG_LoadPCX_RW },

    { "PNG", "sample.png", "sample.bmp", 23, 42, 0, IMG_INIT_PNG,
    #if (USING_IMAGEIO && defined(PNG_USES_IMAGEIO)) || defined(SDL_IMAGE_USE_WIC_BACKEND) || defined(LOAD_PNG)
      SDL_TRUE,
    #else
      SDL_FALSE,
    #endif
      /* Your libpng was built READ-ONLY → disable saving */
      SDL_FALSE,
      IMG_isPNG, IMG_LoadPNG_RW },

    { "PNM", "sample.pnm", "sample.bmp", 23, 42, 0, 0,
    #ifdef LOAD_PNM
      SDL_TRUE,
    #else
      SDL_FALSE,
    #endif
      SDL_FALSE,
      IMG_isPNM, IMG_LoadPNM_RW },

    { "QOI", "sample.qoi", "sample.bmp", 23, 42, 0, 0,
    #ifdef LOAD_QOI
      SDL_TRUE,
    #else
      SDL_FALSE,
    #endif
      SDL_FALSE,
      IMG_isQOI, IMG_LoadQOI_RW },

    { "SVG", "svg.svg", "svg.bmp", 32, 32, 100, 0,
    #ifdef LOAD_SVG
      SDL_TRUE,
    #else
      SDL_FALSE,
    #endif
      SDL_FALSE,
      IMG_isSVG, IMG_LoadSVG_RW },

    { "SVG-sized", "svg.svg", "svg64.bmp", 64, 64, 100, 0,
    #ifdef LOAD_SVG
      SDL_TRUE,
    #else
      SDL_FALSE,
    #endif
      SDL_FALSE,
      IMG_isSVG, IMG_LoadSVG_RW },

    { "SVG-class", "svg-class.svg", "svg-class.bmp", 82, 82, 0, 0,
    #ifdef LOAD_SVG
      SDL_TRUE,
    #else
      SDL_FALSE,
    #endif
      SDL_FALSE,
      IMG_isSVG, IMG_LoadSVG_RW },

    { "TGA", "sample.tga", "sample.bmp", 23, 42, 0, 0,
    #if USING_IMAGEIO || defined(LOAD_TGA)
      SDL_TRUE,
    #else
      SDL_FALSE,
    #endif
      SDL_FALSE,
      NULL, IMG_LoadTGA_RW },

    { "TIF", "sample.tif", "sample.bmp", 23, 42, 0, IMG_INIT_TIF,
    #if USING_IMAGEIO || defined(SDL_IMAGE_USE_WIC_BACKEND) || defined(LOAD_TIF)
      SDL_TRUE,
    #else
      SDL_FALSE,
    #endif
      SDL_FALSE, /* SDL2_image has no TIFF saving API */
      IMG_isTIF, IMG_LoadTIF_RW },

    { "WEBP", "sample.webp", "sample.bmp", 23, 42, 0, IMG_INIT_WEBP,
    #ifdef LOAD_WEBP
      SDL_TRUE,
    #else
      SDL_FALSE,
    #endif
      SDL_FALSE, /* SDL2_image has no WebP saving API */
      IMG_isWEBP, IMG_LoadWEBP_RW },

    { "XCF", "sample.xcf", "sample.bmp", 23, 42, 0, 0,
    #ifdef LOAD_XCF
      SDL_TRUE,
    #else
      SDL_FALSE,
    #endif
      SDL_FALSE,
      IMG_isXCF, IMG_LoadXCF_RW },

    { "XPM", "sample.xpm", "sample.bmp", 23, 42, 0, 0
    #ifdef LOAD_XPM
      , SDL_TRUE
    #else
      , SDL_FALSE
    #endif
      , SDL_FALSE
      , IMG_isXPM, IMG_LoadXPM_RW }
};

typedef enum { LOAD_CONVENIENCE = 0, LOAD_RW, LOAD_TYPED_RW, LOAD_FORMAT_SPECIFIC, LOAD_SIZED } LoadMode;

static SDL_bool ConvertToRgba32(SDL_Surface **surf_p) {
    if ((*surf_p)->format->format != SDL_PIXELFORMAT_RGBA32) {
        SDL_Surface *tmp = SDL_ConvertSurfaceFormat(*surf_p, SDL_PIXELFORMAT_RGBA32, 0);
        SDLTest_AssertCheck(tmp != NULL, "Convert RGBA32 (%s)", SDL_GetError());
        if (!tmp) return SDL_FALSE;
        SDL_FreeSurface(*surf_p);
        *surf_p = tmp;
    }
    return SDL_TRUE;
}

static void DumpPixels(const char *filename, SDL_Surface *surface) {
    const unsigned char *pixels = (const unsigned char*)surface->pixels, *p;
    size_t w, h, pitch, i, j;

    SDL_Log("%s:\n", filename);
    if (surface->format->palette) {
        size_t n = (size_t)surface->format->palette->ncolors;
        SDL_Log("  Palette:\n");
        for (i = 0; i < n; i++) {
            SDL_Log("    RGBA[0x%02x] = %02x%02x%02x%02x\n",
                    (unsigned)i,
                    surface->format->palette->colors[i].r,
                    surface->format->palette->colors[i].g,
                    surface->format->palette->colors[i].b,
                    surface->format->palette->colors[i].a);
        }
    }
    if (surface->w < 0 || surface->h < 0 || surface->pitch < 0) return;
    w = (size_t)surface->w; h = (size_t)surface->h; pitch = (size_t)surface->pitch;
    SDL_Log("  Pixels:\n");
    for (j = 0; j < h; j++) {
        SDL_Log("    ");
        for (i = 0; i < w; i++) {
            p = pixels + (j * pitch) + (i * surface->format->BytesPerPixel);
            switch (surface->format->BitsPerPixel) {
                case 8:  SDL_Log("%02x ", *p); break;
                case 16: SDL_Log("%02x%02x ", p[0], p[1]); break;
                case 24: SDL_Log("%02x%02x%02x ", p[0], p[1], p[2]); break;
                case 32: SDL_Log("%02x%02x%02x%02x ", p[0], p[1], p[2], p[3]); break;
                default: break;
            }
        }
        SDL_Log("\n");
    }
}

static void FormatLoadTest(const Format *fmt, LoadMode mode) {
    SDL_Surface *reference = NULL, *surface = NULL;
    SDL_RWops *src = NULL;
    char *filename = GetTestFilename(TEST_FILE_DIST, fmt->sample);
    char *refFilename = GetTestFilename(TEST_FILE_DIST, fmt->reference);
    int initResult = 0, diff;

    if (!SDLTest_AssertCheck(filename && refFilename, "Build filenames")) goto out;

    if (SDL_strcmp(fmt->reference, "sample.bmp") == 0) {
        reference = SDL_LoadBMP(refFilename);
        SDLTest_AssertCheck(reference != NULL, "Load reference BMP (%s)", SDL_GetError());
        if (!reference) goto out;
    } else if (SDL_strcmp(fmt->reference, "sample.png") == 0) {
    #ifdef LOAD_PNG
        reference = IMG_Load(refFilename);
        SDLTest_AssertCheck(reference != NULL, "Load reference PNG (%s)", SDL_GetError());
        if (!reference) goto out;
    #endif
    }

    if (fmt->initFlag) {
        initResult = IMG_Init(fmt->initFlag);
        SDLTest_AssertCheck(initResult != 0 && (initResult & fmt->initFlag),
                            "IMG_Init 0x%x -> 0x%x (%s)", fmt->initFlag, initResult, IMG_GetError());
        if (!initResult) goto out;
    }

    if (mode != LOAD_CONVENIENCE) {
        src = SDL_RWFromFile(filename, "rb");
        SDLTest_AssertCheck(src != NULL, "Open %s (%s)", filename, SDL_GetError());
        if (!src) goto out;
    }

    switch (mode) {
        case LOAD_CONVENIENCE: surface = IMG_Load(filename); break;
        case LOAD_RW:
            if (fmt->checkFunction) {
                SDL_RWops *ref_src = SDL_RWFromFile(refFilename, "rb");
                if (SDLTest_AssertCheck(ref_src != NULL, "Open %s (%s)", refFilename, SDL_GetError())) {
                    int chk = fmt->checkFunction(ref_src);
                    SDLTest_AssertCheck(!chk, "Ref is not %s (%d)", fmt->name, chk);
                    SDL_RWclose(ref_src);
                }
            }
            if (fmt->checkFunction) {
                int chk = fmt->checkFunction(src);
                SDLTest_AssertCheck(chk, "Detect %s (%d)", fmt->name, chk);
            }
            surface = IMG_Load_RW(src, SDL_TRUE); src = NULL; break;
        case LOAD_TYPED_RW: surface = IMG_LoadTyped_RW(src, SDL_TRUE, fmt->name); src = NULL; break;
        case LOAD_FORMAT_SPECIFIC: surface = fmt->loadFunction(src); break;
        case LOAD_SIZED:
            if (SDL_strcmp(fmt->name, "SVG-sized") == 0) {
                surface = IMG_LoadSizedSVG_RW(src, 64, 64);
            } break;
    }

    SDLTest_AssertCheck(surface != NULL, "Load %s (%s)", filename, IMG_GetError());
    if (!surface) goto out;

    SDLTest_AssertCheck(surface->w == fmt->w && surface->h == fmt->h,
                        "Expected %dx%d got %dx%d", fmt->w, fmt->h, surface->w, surface->h);

    if (GetStringBoolean(SDL_getenv("SDL_IMAGE_TEST_DEBUG"), SDL_FALSE)) DumpPixels(filename, surface);

    if (reference) {
        ConvertToRgba32(&reference);
        ConvertToRgba32(&surface);
        diff = SDLTest_CompareSurfaces(surface, reference, fmt->tolerance);
        SDLTest_AssertCheck(diff == 0, "Diff <= %d in %d px", fmt->tolerance, diff);
        if (diff != 0 || GetStringBoolean(SDL_getenv("SDL_IMAGE_TEST_DEBUG"), SDL_FALSE)) {
            DumpPixels(filename, surface);
            DumpPixels(refFilename, reference);
        }
    }

out:
    if (surface) SDL_FreeSurface(surface);
    if (reference) SDL_FreeSurface(reference);
    if (src) SDL_RWclose(src);
    if (refFilename) SDL_free(refFilename);
    if (filename) SDL_free(filename);
    if (initResult) IMG_Quit();
}

static void FormatSaveTest(const Format *fmt, SDL_bool rw) {
    char *refFilename = GetTestFilename(TEST_FILE_DIST, "sample.bmp");
    char filename[64] = {0};
    SDL_Surface *reference = NULL, *surface = NULL;
    SDL_RWops *dest = NULL;
    int initResult = 0, diff, result;

    SDL_snprintf(filename, sizeof(filename), "save%s.%s", rw ? "Rwops" : "", fmt->name);

    if (!SDLTest_AssertCheck(refFilename != NULL, "Build ref filename (%s)", SDL_GetError())) goto out;

    reference = SDL_LoadBMP(refFilename);
    if (!SDLTest_AssertCheck(reference != NULL, "Load reference BMP (%s)", SDL_GetError())) goto out;

    if (fmt->initFlag) {
        initResult = IMG_Init(fmt->initFlag);
        if (!SDLTest_AssertCheck(initResult != 0 && (initResult & fmt->initFlag),
                                 "IMG_Init 0x%x -> 0x%x (%s)", fmt->initFlag, initResult, IMG_GetError())) goto out;
    }

    if (SDL_strcmp(fmt->name, "PNG") == 0) {
        /* PNG saving disabled in this build (read-only libpng). */
        SDLTest_AssertCheck(SDL_FALSE, "PNG saving not supported in this build");
        goto out;
    } else if (SDL_strcmp(fmt->name, "JPG") == 0) {
        if (rw) {
            dest = SDL_RWFromFile(filename, "wb");
            SDLTest_AssertCheck(dest != NULL, "Open %s (%s)", filename, SDL_GetError());
            if (!dest) goto out;
            result = IMG_SaveJPG_RW(reference, dest, SDL_FALSE, 90);
            SDL_RWclose(dest);
        } else {
            result = IMG_SaveJPG(reference, filename, 90);
        }
        SDLTest_AssertCheck(result == 0, "Save %s (%s)", filename, IMG_GetError());
    } else {
        SDLTest_AssertCheck(SDL_FALSE, "How do I save %s?", fmt->name);
        goto out;
    }

    if (fmt->canLoad) {
        surface = IMG_Load(filename);
        if (!SDLTest_AssertCheck(surface != NULL, "Load saved file (%s)", IMG_GetError())) goto out;

        ConvertToRgba32(&reference);
        ConvertToRgba32(&surface);

        SDLTest_AssertCheck(surface->w == fmt->w && surface->h == fmt->h,
                            "Expected %dx%d got %dx%d", fmt->w, fmt->h, surface->w, surface->h);

        diff = SDLTest_CompareSurfaces(surface, reference, fmt->tolerance);
        SDLTest_AssertCheck(diff == 0, "Diff <= %d in %d px", fmt->tolerance, diff);
    }

out:
    if (surface) SDL_FreeSurface(surface);
    if (reference) SDL_FreeSurface(reference);
    if (refFilename) SDL_free(refFilename);
    if (initResult) IMG_Quit();
}

static void FormatTest(const Format *fmt) {
    SDL_bool forced;
    char envVar[64] = {0};

    SDL_snprintf(envVar, sizeof(envVar), "SDL_IMAGE_TEST_REQUIRE_LOAD_%s", fmt->name);
    forced = GetStringBoolean(SDL_getenv(envVar), SDL_FALSE);
    if (forced) SDLTest_AssertCheck(fmt->canLoad, "%s loading should be enabled", fmt->name);

    if (fmt->canLoad || forced) {
        SDLTest_Log("Testing ability to load format %s", fmt->name);
        if (SDL_strcmp(fmt->name, "SVG-sized") == 0) {
            FormatLoadTest(fmt, LOAD_SIZED);
        } else {
            FormatLoadTest(fmt, LOAD_CONVENIENCE);
            if (SDL_strcmp(fmt->name, "TGA") == 0) {
                SDLTest_Log("SKIP: Recognising %s by magic number is not supported", fmt->name);
            } else {
                FormatLoadTest(fmt, LOAD_RW);
            }
            FormatLoadTest(fmt, LOAD_TYPED_RW);
            if (fmt->loadFunction) FormatLoadTest(fmt, LOAD_FORMAT_SPECIFIC);
        }
    } else {
        SDLTest_Log("Format %s is not supported", fmt->name);
    }

    SDL_snprintf(envVar, sizeof(envVar), "SDL_IMAGE_TEST_REQUIRE_SAVE_%s", fmt->name);
    forced = GetStringBoolean(SDL_getenv(envVar), SDL_FALSE);
    if (forced) SDLTest_AssertCheck(fmt->canSave, "%s saving should be enabled", fmt->name);

    if (fmt->canSave || forced) {
        SDLTest_Log("Testing ability to save format %s", fmt->name);
        if (SDL_strcmp(fmt->name, "JPG") == 0) {
            FormatSaveTest(fmt, SDL_FALSE);
            FormatSaveTest(fmt, SDL_TRUE);
        } else {
            SDLTest_Log("Saving format %s is not supported in this SDL2 test", fmt->name);
        }
    } else {
        SDLTest_Log("Saving format %s is not supported", fmt->name);
    }
}

static int TestFormats(void *arg) {
    size_t i; (void)arg;
    for (i = 0; i < SDL_arraysize(formats); i++) FormatTest(&formats[i]);
    return TEST_COMPLETED;
}

static const SDLTest_TestCaseReference formatsTestCase =
    { TestFormats, "Images", "Load and save various image formats", TEST_ENABLED };

static const SDLTest_TestCaseReference *testCases[] = { &formatsTestCase, NULL };
static SDLTest_TestSuiteReference testSuite = { "img", NULL, testCases, NULL };
static SDLTest_TestSuiteReference *testSuites[] = { &testSuite, NULL };

static void quit(int rc) { SDLTest_CommonQuit(state); exit(rc); }

int main(int argc, char *argv[]) {
    int result, testIterations = 1, i, done;
    Uint64 userExecKey = 0;
    char *userRunSeed = NULL, *filter = NULL;
    SDL_Event event;

    state = SDLTest_CommonCreateState(argv, SDL_INIT_VIDEO);
    if (!state) return 1;

    for (i = 1; i < argc;) {
        int consumed = SDLTest_CommonArg(state, i);
        if (consumed == 0) {
            consumed = -1;
            if (SDL_strcasecmp(argv[i], "--iterations") == 0 && argv[i+1]) { testIterations = SDL_atoi(argv[i+1]); if (testIterations < 1) testIterations = 1; consumed = 2; }
            else if (SDL_strcasecmp(argv[i], "--execKey") == 0 && argv[i+1]) { SDL_sscanf(argv[i+1], "%" SDL_PRIu64, &userExecKey); consumed = 2; }
            else if (SDL_strcasecmp(argv[i], "--seed")    == 0 && argv[i+1]) { userRunSeed = SDL_strdup(argv[i+1]); consumed = 2; }
            else if (SDL_strcasecmp(argv[i], "--filter")  == 0 && argv[i+1]) { filter = SDL_strdup(argv[i+1]); consumed = 2; }
        }
        if (consumed < 0) {
        #if SDL_VERSION_ATLEAST(2,0,10)
            static const char *opts[] = {"[--iterations #]", "[--execKey #]", "[--seed string]", "[--filter suite|test]", NULL};
            SDLTest_CommonLogUsage(state, argv[0], opts);
        #else
            SDLTest_CommonUsage(state);
        #endif
            quit(1);
        }
        i += consumed;
    }

    if (!SDLTest_CommonInit(state)) quit(2);

    for (i = 0; i < state->num_windows; ++i) {
        SDL_Renderer *renderer = state->renderers[i];
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);
    }

    result = SDLTest_RunSuites(testSuites, (const char *)userRunSeed, userExecKey, (const char *)filter, testIterations);

    done = 0;
    for (i = 0; i < 100; i++) {
        while (SDL_PollEvent(&event)) SDLTest_CommonEvent(state, &event, &done);
        SDL_Delay(10);
    }

    SDL_free(userRunSeed);
    SDL_free(filter);
    //quit(result);
    return result;
}

/* vi: set ts=4 sw=4 expandtab: */
