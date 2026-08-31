/* SDL3 build configuration for AmigaOS 3.x / 68k. Initial port. */
#ifndef SDL_build_config_amigaos3_h_
#define SDL_build_config_amigaos3_h_
#define SDL_build_config_h_
#include <SDL3/SDL_platform_defines.h>

#define HAVE_LIBC 1
#define HAVE_STDARG_H 1
#define HAVE_STDDEF_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDIO_H 1
#define HAVE_STRING_H 1
#define HAVE_CTYPE_H 1
#define HAVE_MATH_H 1
#define HAVE_FLOAT_H 1
#define HAVE_LIMITS_H 1
#define HAVE_MALLOC 1
#define HAVE_CALLOC 1
#define HAVE_REALLOC 1
#define HAVE_FREE 1
#define HAVE_MEMSET 1
#define HAVE_MEMCPY 1
#define HAVE_MEMMOVE 1
#define HAVE_MEMCMP 1
#define HAVE_STRLEN 1
#define HAVE_STRDUP 1
#define HAVE_STRCHR 1
#define HAVE_STRRCHR 1
#define HAVE_STRSTR 1
#define HAVE_STRCMP 1
#define HAVE_STRNCMP 1
#define HAVE_STRCASECMP 1
#define HAVE_STRNCASECMP 1
#define HAVE_STRTOL 1
#define HAVE_STRTOUL 1
#define HAVE_STRTOD 1
#define HAVE_ATOI 1
#define HAVE_ATOF 1
#define HAVE_QSORT 1
#define HAVE_ABS 1
#define HAVE_SNPRINTF 1
#define HAVE_VSNPRINTF 1
#define HAVE_SSCANF 1
#define HAVE_ACOS 1
#define HAVE_ASIN 1
#define HAVE_ATAN 1
#define HAVE_ATAN2 1
#define HAVE_EXP 1
#define HAVE_FABS 1
#define HAVE_FLOOR 1
#define HAVE_FMOD 1
#define HAVE_LOG 1
#define HAVE_POW 1
#define HAVE_SQRT 1
#define HAVE_TAN 1

#define SDL_MATH_AMIGAOS3_TABLES 1

#define SDL_VIDEO_DRIVER_AMIGAOS3 1
#define SDL_VIDEO_DRIVER_DUMMY 1
#if !defined(SDL_AMIGAOS3_SW_ONLY)
#define SDL_VIDEO_OPENGL 1
#endif
#define SDL_VIDEO_RENDER_SW 1
#define SDL_AUDIO_DRIVER_AMIGAOS3 1
#define SDL_JOYSTICK_AMIGAOS3 1
#define SDL_HAPTIC_DISABLED 1
#define SDL_SENSOR_DISABLED 1
#define SDL_TRAY_DUMMY 1
#define SDL_THREAD_AMIGAOS3 1
#define SDL_LOADSO_DUMMY 1
#define SDL_PROCESS_DUMMY 1
#define SDL_FILESYSTEM_AMIGAOS3 1
#define SDL_FSOPS_AMIGAOS3 1
#define SDL_TIME_AMIGAOS3 1
#define SDL_TIMER_AMIGAOS3 1
#define SDL_GPU_DISABLED 1

#endif
