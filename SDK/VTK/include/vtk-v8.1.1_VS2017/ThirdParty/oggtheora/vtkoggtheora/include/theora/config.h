/* KITWARE_OGGTHEORA_CHANGE - Stripped out unused defines (autoheader pain)
 * and made it work for universal builds on Mac OS X */

#ifndef THEORA_CONFIG_H
#define THEORA_CONFIG_H

/* KITWARE_OGGTHEORA_CHANGE - Added to mangle function names */
#include <vtkoggtheora/include/vtk_oggtheora_mangle.h>

/* Define to exclude floating point code from the build */
/* #undef THEORA_DISABLE_FLOAT */

/* make use of asm optimization */
#if !defined(__APPLE__)
/* #undef USE_ASM */
#elif !defined(THEORA_DISABLE_ASM)
#  undef USE_ASM
#  if defined(__i386__) || defined(__x86_64__)
#    define USE_ASM 1
#  endif
#endif

/* determine whether we should use x86_32 or x86_64 asm (if at all) */
#if defined(USE_ASM)
#  if !defined(__APPLE__)
/* #undef OC_X86_ASM */
/* #undef OC_X86_64_ASM */
#  else
#    if defined(__i386__)
#      define OC_X86_ASM 1
#    elif defined(__x86_64__)
#      define OC_X86_64_ASM 1
#    endif
#  endif
#endif

/* never use Cairo */
#undef HAVE_CAIRO

/* Version number of package */
#define VERSION "1.1.1"

#endif /* THEORA_CONFIG_H */
