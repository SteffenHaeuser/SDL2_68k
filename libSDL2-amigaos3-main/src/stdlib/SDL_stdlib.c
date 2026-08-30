/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2026 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#if defined(__clang_analyzer__) && !defined(SDL_DISABLE_ANALYZE_MACROS)
#define SDL_DISABLE_ANALYZE_MACROS 1
#endif

#include "../SDL_internal.h"

/* This file contains portable stdlib functions for SDL */

#include "SDL_stdinc.h"
#include "../libm/math_libm.h"

#ifdef SDL_MATH_AMIGAOS3_TABLES

/*
 * AmigaOS3/m68k fixed-table trigonometry.
 *
 * The target C library sin()/cos() path has been observed to produce
 * corrupted results.  Use a 257-entry quarter-wave Q30 table instead.
 *
 * Quadrant symmetry gives 1024 logical samples over one revolution and
 * linear interpolation gives sufficient accuracy for SDL's general-purpose
 * trigonometric wrappers without depending on the broken runtime functions.
 */
#define SDL_OS3_TRIG_STEPS       1024
#define SDL_OS3_TRIG_QUARTER     256
#define SDL_OS3_TRIG_Q30_SCALE   1073741824.0
#define SDL_OS3_TWO_PI_D         6.283185307179586476925286766559
#define SDL_OS3_HALF_PI_D        1.5707963267948966192313216916398

static const Sint32 SDL_os3_sin_q30[257] = {
    0, 6588356, 13176464, 19764076, 26350943, 32936819, 39521455, 46104602,
    52686014, 59265442, 65842639, 72417357, 78989349, 85558366, 92124163, 98686491,
    105245103, 111799753, 118350194, 124896179, 131437462, 137973796, 144504935, 151030634,
    157550647, 164064728, 170572633, 177074115, 183568930, 190056834, 196537583, 203010932,
    209476638, 215934457, 222384147, 228825464, 235258165, 241682010, 248096755, 254502159,
    260897982, 267283981, 273659918, 280025552, 286380643, 292724951, 299058239, 305380268,
    311690799, 317989595, 324276419, 330551034, 336813204, 343062693, 349299266, 355522689,
    361732726, 367929144, 374111709, 380280190, 386434353, 392573967, 398698801, 404808624,
    410903207, 416982319, 423045732, 429093217, 435124548, 441139496, 447137835, 453119340,
    459083786, 465030947, 470960600, 476872522, 482766489, 488642281, 494499676, 500338453,
    506158392, 511959275, 517740883, 523502998, 529245404, 534967884, 540670223, 546352205,
    552013618, 557654248, 563273883, 568872310, 574449320, 580004702, 585538248, 591049748,
    596538995, 602005783, 607449906, 612871159, 618269338, 623644239, 628995660, 634323400,
    639627258, 644907034, 650162530, 655393548, 660599890, 665781362, 670937767, 676068911,
    681174602, 686254647, 691308855, 696337036, 701339000, 706314559, 711263525, 716185713,
    721080937, 725949013, 730789757, 735602987, 740388522, 745146182, 749875788, 754577161,
    759250125, 763894504, 768510122, 773096806, 777654384, 782182683, 786681534, 791150767,
    795590213, 799999706, 804379079, 808728167, 813046808, 817334838, 821592095, 825818421,
    830013654, 834177638, 838310216, 842411232, 846480531, 850517961, 854523370, 858496606,
    862437520, 866345964, 870221790, 874064853, 877875009, 881652112, 885396022, 889106597,
    892783698, 896427186, 900036924, 903612776, 907154608, 910662286, 914135678, 917574653,
    920979082, 924348837, 927683790, 930983817, 934248793, 937478595, 940673101, 943832191,
    946955747, 950043650, 953095785, 956112036, 959092290, 962036435, 964944360, 967815955,
    970651112, 973449725, 976211688, 978936898, 981625251, 984276646, 986890984, 989468165,
    992008094, 994510675, 996975812, 999403415, 1001793390, 1004145648, 1006460100, 1008736660,
    1010975242, 1013175761, 1015338134, 1017462281, 1019548121, 1021595575, 1023604567, 1025575020,
    1027506862, 1029400018, 1031254418, 1033069992, 1034846671, 1036584389, 1038283080, 1039942680,
    1041563127, 1043144360, 1044686319, 1046188946, 1047652185, 1049075980, 1050460278, 1051805027,
    1053110176, 1054375676, 1055601479, 1056787540, 1057933813, 1059040255, 1060106826, 1061133483,
    1062120190, 1063066909, 1063973603, 1064840240, 1065666786, 1066453210, 1067199483, 1067905576,
    1068571464, 1069197120, 1069782521, 1070327646, 1070832474, 1071296985, 1071721163, 1072104991,
    1072448455, 1072751542, 1073014240, 1073236540, 1073418433, 1073559913, 1073660973, 1073721611,
    1073741824
};

static Sint32 SDL_OS3_SinSample(int index)
{
    int i = index & (SDL_OS3_TRIG_STEPS - 1);

    if (i <= SDL_OS3_TRIG_QUARTER) {
        return SDL_os3_sin_q30[i];
    }
    if (i <= (SDL_OS3_TRIG_QUARTER * 2)) {
        return SDL_os3_sin_q30[(SDL_OS3_TRIG_QUARTER * 2) - i];
    }
    if (i <= (SDL_OS3_TRIG_QUARTER * 3)) {
        return -SDL_os3_sin_q30[i - (SDL_OS3_TRIG_QUARTER * 2)];
    }
    return -SDL_os3_sin_q30[SDL_OS3_TRIG_STEPS - i];
}

static double SDL_OS3_TableSin(double x)
{
    double reduced;
    double phase;
    double frac;
    int index;
    Sint32 a;
    Sint32 b;

    /*
     * Use SDL2's bundled libm range reducer too, avoiding the platform fmod()
     * for this implementation.
     */
    reduced = SDL_uclibc_fmod(x, SDL_OS3_TWO_PI_D);
    if (reduced < 0.0) {
        reduced += SDL_OS3_TWO_PI_D;
    }

    phase = reduced * ((double)SDL_OS3_TRIG_STEPS / SDL_OS3_TWO_PI_D);
    index = (int)phase;
    frac = phase - (double)index;

    a = SDL_OS3_SinSample(index);
    b = SDL_OS3_SinSample(index + 1);

    return ((double)a + ((double)(b - a) * frac)) /
           SDL_OS3_TRIG_Q30_SCALE;
}

#endif /* SDL_MATH_AMIGAOS3_TABLES */

double
SDL_atan(double x)
{
#if defined(HAVE_ATAN)
    return atan(x);
#else
    return SDL_uclibc_atan(x);
#endif
}

float SDL_atanf(float x)
{
#if defined(HAVE_ATANF)
    return atanf(x);
#else
    return (float)SDL_atan((double)x);
#endif
}

double
SDL_atan2(double y, double x)
{
#if defined(HAVE_ATAN2)
    return atan2(y, x);
#else
    return SDL_uclibc_atan2(y, x);
#endif
}

float SDL_atan2f(float y, float x)
{
#if defined(HAVE_ATAN2F)
    return atan2f(y, x);
#else
    return (float)SDL_atan2((double)y, (double)x);
#endif
}

double
SDL_acos(double val)
{
#if defined(HAVE_ACOS)
    return acos(val);
#else
    double result;
    if (val == -1.0) {
        result = M_PI;
    } else {
        result = SDL_atan(SDL_sqrt(1.0 - val * val) / val);
        if (result < 0.0)
        {
            result += M_PI;
        }
    }
    return result;
#endif
}

float SDL_acosf(float val)
{
#if defined(HAVE_ACOSF)
    return acosf(val);
#else
    return (float)SDL_acos((double)val);
#endif
}

double
SDL_asin(double val)
{
#if defined(HAVE_ASIN)
    return asin(val);
#else
    double result;
    if (val == -1.0) {
        result = -(M_PI / 2.0);
    } else {
        result = (M_PI / 2.0) - SDL_acos(val);
    }
    return result;
#endif
}

float SDL_asinf(float val)
{
#if defined(HAVE_ASINF)
    return asinf(val);
#else
    return (float)SDL_asin((double)val);
#endif
}

double
SDL_ceil(double x)
{
#if defined(HAVE_CEIL)
    return ceil(x);
#else
    double integer = SDL_floor(x);
    double fraction = x - integer;
    if (fraction > 0.0) {
        integer += 1.0;
    }
    return integer;
#endif /* HAVE_CEIL */
}

float SDL_ceilf(float x)
{
#if defined(HAVE_CEILF)
    return ceilf(x);
#else
    return (float)SDL_ceil((double)x);
#endif
}

double
SDL_copysign(double x, double y)
{
#if defined(HAVE_COPYSIGN)
    return copysign(x, y);
#elif defined(HAVE__COPYSIGN)
    return _copysign(x, y);
#elif defined(__WATCOMC__) && defined(__386__)
    /* this is nasty as hell, but it works.. */
    unsigned int *xi = (unsigned int *)&x,
                 *yi = (unsigned int *)&y;
    xi[1] = (yi[1] & 0x80000000) | (xi[1] & 0x7fffffff);
    return x;
#else
    return SDL_uclibc_copysign(x, y);
#endif /* HAVE_COPYSIGN */
}

float SDL_copysignf(float x, float y)
{
#if defined(HAVE_COPYSIGNF)
    return copysignf(x, y);
#else
    return (float)SDL_copysign((double)x, (double)y);
#endif
}

double
SDL_cos(double x)
{
#if defined(SDL_MATH_AMIGAOS3_TABLES)
    return SDL_OS3_TableSin(x + SDL_OS3_HALF_PI_D);
#elif defined(HAVE_COS)
    return cos(x);
#else
    return SDL_uclibc_cos(x);
#endif
}

float SDL_cosf(float x)
{
#if defined(HAVE_COSF)
    return cosf(x);
#else
    return (float)SDL_cos((double)x);
#endif
}

double
SDL_exp(double x)
{
#if defined(HAVE_EXP)
    return exp(x);
#else
    return SDL_uclibc_exp(x);
#endif
}

float SDL_expf(float x)
{
#if defined(HAVE_EXPF)
    return expf(x);
#else
    return (float)SDL_exp((double)x);
#endif
}

double
SDL_fabs(double x)
{
#if defined(HAVE_FABS)
    return fabs(x);
#else
    return SDL_uclibc_fabs(x);
#endif
}

float SDL_fabsf(float x)
{
#if defined(HAVE_FABSF)
    return fabsf(x);
#else
    return (float)SDL_fabs((double)x);
#endif
}

double
SDL_floor(double x)
{
#if defined(HAVE_FLOOR)
    return floor(x);
#else
    return SDL_uclibc_floor(x);
#endif
}

float SDL_floorf(float x)
{
#if defined(HAVE_FLOORF)
    return floorf(x);
#else
    return (float)SDL_floor((double)x);
#endif
}

double
SDL_trunc(double x)
{
#if defined(HAVE_TRUNC)
    return trunc(x);
#else
    if (x >= 0.0f) {
        return SDL_floor(x);
    } else {
        return SDL_ceil(x);
    }
#endif
}

float SDL_truncf(float x)
{
#if defined(HAVE_TRUNCF)
    return truncf(x);
#else
    return (float)SDL_trunc((double)x);
#endif
}

double
SDL_fmod(double x, double y)
{
#if defined(HAVE_FMOD)
    return fmod(x, y);
#else
    return SDL_uclibc_fmod(x, y);
#endif
}

float SDL_fmodf(float x, float y)
{
#if defined(HAVE_FMODF)
    return fmodf(x, y);
#else
    return (float)SDL_fmod((double)x, (double)y);
#endif
}

double
SDL_log(double x)
{
#if defined(HAVE_LOG)
    return log(x);
#else
    return SDL_uclibc_log(x);
#endif
}

float SDL_logf(float x)
{
#if defined(HAVE_LOGF)
    return logf(x);
#else
    return (float)SDL_log((double)x);
#endif
}

double
SDL_log10(double x)
{
#if defined(HAVE_LOG10)
    return log10(x);
#else
    return SDL_uclibc_log10(x);
#endif
}

float SDL_log10f(float x)
{
#if defined(HAVE_LOG10F)
    return log10f(x);
#else
    return (float)SDL_log10((double)x);
#endif
}

double
SDL_pow(double x, double y)
{
#if defined(HAVE_POW)
    return pow(x, y);
#else
    return SDL_uclibc_pow(x, y);
#endif
}

float SDL_powf(float x, float y)
{
#if defined(HAVE_POWF)
    return powf(x, y);
#else
    return (float)SDL_pow((double)x, (double)y);
#endif
}

double
SDL_round(double arg)
{
#if defined HAVE_ROUND
    return round(arg);
#else
    if (arg >= 0.0) {
        return SDL_floor(arg + 0.5);
    } else {
        return SDL_ceil(arg - 0.5);
    }
#endif
}

float SDL_roundf(float arg)
{
#if defined HAVE_ROUNDF
    return roundf(arg);
#else
    return (float)SDL_round((double)arg);
#endif
}

long SDL_lround(double arg)
{
#if defined HAVE_LROUND
    return lround(arg);
#else
    return (long)SDL_round(arg);
#endif
}

long SDL_lroundf(float arg)
{
#if defined HAVE_LROUNDF
    return lroundf(arg);
#else
    return (long)SDL_round((double)arg);
#endif
}

double
SDL_scalbn(double x, int n)
{
#if defined(HAVE_SCALBN)
    return scalbn(x, n);
#elif defined(HAVE__SCALB)
    return _scalb(x, n);
#elif defined(HAVE_LIBC) && defined(HAVE_FLOAT_H) && (FLT_RADIX == 2)
    /* from scalbn(3): If FLT_RADIX equals 2 (which is
     * usual), then scalbn() is equivalent to ldexp(3). */
    return ldexp(x, n);
#else
    return SDL_uclibc_scalbn(x, n);
#endif
}

float SDL_scalbnf(float x, int n)
{
#if defined(HAVE_SCALBNF)
    return scalbnf(x, n);
#else
    return (float)SDL_scalbn((double)x, n);
#endif
}

double
SDL_sin(double x)
{
#if defined(SDL_MATH_AMIGAOS3_TABLES)
    return SDL_OS3_TableSin(x);
#elif defined(HAVE_SIN)
    return sin(x);
#else
    return SDL_uclibc_sin(x);
#endif
}

float SDL_sinf(float x)
{
#if defined(HAVE_SINF)
    return sinf(x);
#else
    return (float)SDL_sin((double)x);
#endif
}

double
SDL_sqrt(double x)
{
#if defined(HAVE_SQRT)
    return sqrt(x);
#else
    return SDL_uclibc_sqrt(x);
#endif
}

float SDL_sqrtf(float x)
{
#if defined(HAVE_SQRTF)
    return sqrtf(x);
#else
    return (float)SDL_sqrt((double)x);
#endif
}

double
SDL_tan(double x)
{
#if defined(HAVE_TAN)
    return tan(x);
#else
    return SDL_uclibc_tan(x);
#endif
}

float SDL_tanf(float x)
{
#if defined(HAVE_TANF)
    return tanf(x);
#else
    return (float)SDL_tan((double)x);
#endif
}

int SDL_abs(int x)
{
#if defined(HAVE_ABS)
    return abs(x);
#else
    return (x < 0) ? -x : x;
#endif
}

#if defined(HAVE_CTYPE_H)
int SDL_isalpha(int x)
{
    return isalpha(x);
}
int SDL_isalnum(int x) { return isalnum(x); }
int SDL_isdigit(int x) { return isdigit(x); }
int SDL_isxdigit(int x) { return isxdigit(x); }
int SDL_ispunct(int x) { return ispunct(x); }
int SDL_isspace(int x) { return isspace(x); }
int SDL_isupper(int x) { return isupper(x); }
int SDL_islower(int x) { return islower(x); }
int SDL_isprint(int x) { return isprint(x); }
int SDL_isgraph(int x) { return isgraph(x); }
int SDL_iscntrl(int x) { return iscntrl(x); }
int SDL_toupper(int x) { return toupper(x); }
int SDL_tolower(int x) { return tolower(x); }
#else
int SDL_isalpha(int x)
{
    return (SDL_isupper(x)) || (SDL_islower(x));
}
int SDL_isalnum(int x) { return (SDL_isalpha(x)) || (SDL_isdigit(x)); }
int SDL_isdigit(int x) { return ((x) >= '0') && ((x) <= '9'); }
int SDL_isxdigit(int x) { return (((x) >= 'A') && ((x) <= 'F')) || (((x) >= 'a') && ((x) <= 'f')) || (SDL_isdigit(x)); }
int SDL_ispunct(int x) { return (SDL_isgraph(x)) && (!SDL_isalnum(x)); }
int SDL_isspace(int x) { return ((x) == ' ') || ((x) == '\t') || ((x) == '\r') || ((x) == '\n') || ((x) == '\f') || ((x) == '\v'); }
int SDL_isupper(int x) { return ((x) >= 'A') && ((x) <= 'Z'); }
int SDL_islower(int x) { return ((x) >= 'a') && ((x) <= 'z'); }
int SDL_isprint(int x) { return ((x) >= ' ') && ((x) < '\x7f'); }
int SDL_isgraph(int x) { return (SDL_isprint(x)) && ((x) != ' '); }
int SDL_iscntrl(int x) { return (((x) >= '\0') && ((x) <= '\x1f')) || ((x) == '\x7f'); }
int SDL_toupper(int x) { return ((x) >= 'a') && ((x) <= 'z') ? ('A' + ((x) - 'a')) : (x); }
int SDL_tolower(int x) { return ((x) >= 'A') && ((x) <= 'Z') ? ('a' + ((x) - 'A')) : (x); }
#endif

/* This file contains a portable memcpy manipulation function for SDL */

void *SDL_memcpy(SDL_OUT_BYTECAP(len) void *dst, SDL_IN_BYTECAP(len) const void *src, size_t len)
{
#ifdef __GNUC__
    /* Presumably this is well tuned for speed.
       On my machine this is twice as fast as the C code below.
     */
    return __builtin_memcpy(dst, src, len);
#elif defined(HAVE_MEMCPY)
    return memcpy(dst, src, len);
#elif defined(HAVE_BCOPY)
    bcopy(src, dst, len);
    return dst;
#else
    /* GCC 4.9.0 with -O3 will generate movaps instructions with the loop
       using Uint32* pointers, so we need to make sure the pointers are
       aligned before we loop using them.
     */
    if (((uintptr_t)src & 0x3) || ((uintptr_t)dst & 0x3)) {
        /* Do an unaligned byte copy */
        Uint8 *srcp1 = (Uint8 *)src;
        Uint8 *dstp1 = (Uint8 *)dst;

        while (len--) {
            *dstp1++ = *srcp1++;
        }
    } else {
        size_t left = (len % 4);
        Uint32 *srcp4, *dstp4;
        Uint8 *srcp1, *dstp1;

        srcp4 = (Uint32 *)src;
        dstp4 = (Uint32 *)dst;
        len /= 4;
        while (len--) {
            *dstp4++ = *srcp4++;
        }

        srcp1 = (Uint8 *)srcp4;
        dstp1 = (Uint8 *)dstp4;
        switch (left) {
        case 3:
            *dstp1++ = *srcp1++;
        case 2:
            *dstp1++ = *srcp1++;
        case 1:
            *dstp1++ = *srcp1++;
        }
    }
    return dst;
#endif /* __GNUC__ */
}

void *SDL_memset(SDL_OUT_BYTECAP(len) void *dst, int c, size_t len)
{
#if defined(HAVE_MEMSET)
    return memset(dst, c, len);
#else
    size_t left;
    Uint32 *dstp4;
    Uint8 *dstp1 = (Uint8 *)dst;
    Uint8 value1;
    Uint32 value4;

    /* The value used in memset() is a byte, passed as an int */
    c &= 0xff;

    /* The destination pointer needs to be aligned on a 4-byte boundary to
     * execute a 32-bit set. Set first bytes manually if needed until it is
     * aligned. */
    value1 = (Uint8)c;
    while ((uintptr_t)dstp1 & 0x3) {
        if (len--) {
            *dstp1++ = value1;
        } else {
            return dst;
        }
    }

    value4 = ((Uint32)c | ((Uint32)c << 8) | ((Uint32)c << 16) | ((Uint32)c << 24));
    dstp4 = (Uint32 *)dstp1;
    left = (len % 4);
    len /= 4;
    while (len--) {
        *dstp4++ = value4;
    }

    dstp1 = (Uint8 *)dstp4;
    switch (left) {
    case 3:
        *dstp1++ = value1;
    case 2:
        *dstp1++ = value1;
    case 1:
        *dstp1++ = value1;
    }

    return dst;
#endif /* HAVE_MEMSET */
}

#if defined(HAVE_CTYPE_H) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
int SDL_isblank(int x)
{
    return isblank(x);
}
#else
int SDL_isblank(int x)
{
    return ((x) == ' ') || ((x) == '\t');
}
#endif

/* vi: set ts=4 sw=4 expandtab: */
