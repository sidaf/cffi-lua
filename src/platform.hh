/* This contains assorted platform constants, defined in one place.
 *
 * The primary preference is config.h, generated by the buildsystem. There
 * are also fallbacks in place using different approaches, in order to
 * decouple the code from build system config as much as possible; the
 * goal is to make it buildable without any pre-configuration
 */

#ifndef PLATFORM_HH
#define PLATFORM_HH

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* host system endianness */

#if defined(HAVE_BIG_ENDIAN) || defined(__BIG_ENDIAN__) || \
    defined(__ARMEB__) || defined(__AARCH64EB__) || ( \
        defined(__BYTE_ORDER__) && \
        (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) \
    )
#  define FFI_BIG_ENDIAN 1
#elif defined(HAVE_ENDIAN_H)
#  include <endian.h>
#  if __BYTE_ORDER == __BIG_ENDIAN
#    define FFI_BIG_ENDIAN 1
#  else
#    define FFI_LITTLE_ENDIAN 1
#  endif
#else
#  define FFI_LITTLE_ENDIAN 1
#endif

/* OS; defined to be luajit compatible
 *
 * If undetected, it will still work, but the OS will be "Other"
 */

#define FFI_OS_OTHER   0
#define FFI_OS_WINDOWS 1
#define FFI_OS_LINUX   2
#define FFI_OS_OSX     3
#define FFI_OS_BSD     4
#define FFI_OS_POSIX   5

#if defined(_WIN32) && !defined(_XBOX_VER)
#  define FFI_OS FFI_OS_WINDOWS
#  define FFI_OS_NAME "Windows"
#elif defined(__linux__)
#  define FFI_OS FFI_OS_LINUX
#  define FFI_OS_NAME "Linux"
#elif defined(__MACH__) && defined(__APPLE__)
#  define FFI_OS FFI_OS_OSX
#  define FFI_OS_NAME "OSX"
#elif (defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || \
       defined(__NetBSD__) || defined(__OpenBSD__) || \
       defined(__DragonFly__)) && !defined(__ORBIS__)
#  define FFI_OS FFI_OS_BSD
#  define FFI_OS_NAME "BSD"
#elif (defined(__sun__) && defined(__svr4__)) || defined(__HAIKU__) || \
      defined(__CYGWIN__)
#  define FFI_OS FFI_OS_POSIX
#  define FFI_OS_NAME "POSIX"
#  ifdef __CYGWIN__
#    define FFI_OS_CYGWIN 1
#  endif
#else
#  define FFI_OS FFI_OS_OTHER
#  define FFI_OS_NAME "Other"
#endif

/* Arch; defined to be luajit compatible
 *
 * IF undetected, it will still work, but the arch will be "unknown"
 */

#define FFI_ARCH_UNKNOWN 0
#define FFI_ARCH_X86     1
#define FFI_ARCH_X64     2
#define FFI_ARCH_ARM     3
#define FFI_ARCH_ARM64   4
#define FFI_ARCH_PPC     5
#define FFI_ARCH_PPC64   6
#define FFI_ARCH_MIPS32  7
#define FFI_ARCH_MIPS64  8

#if defined(__i386) || defined(__i386__) || defined(_M_IX86)
#  define FFI_ARCH FFI_ARCH_X86
#  define FFI_ARCH_NAME "x86"
#elif defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || \
      defined(_M_AMD64)
#  define FFI_ARCH FFI_ARCH_X64
#  define FFI_ARCH_NAME "x64"
#elif defined(__arm__) || defined(__arm) || defined(__ARM__) || defined(__ARM)
#  define FFI_ARCH FFI_ARCH_ARM
#  if defined(FFI_BIG_ENDIAN)
#    define FFI_ARCH_NAME "armeb"
#  else
#    define FFI_ARCH_NAME "arm"
#  endif
#  ifdef __SOFTFP__
#    define FFI_ARCH_HAS_FPU 0
#  endif
#  if !__ARM_PCS_VFP
#    define FFI_ARCH_SOFTFP 1
#  endif
#elif defined(__aarch64__)
#  define FFI_ARCH FFI_ARCH_ARM64
#  if defined(FFI_BIG_ENDIAN)
#    define FFI_ARCH_NAME "arm64be"
#  else
#    define FFI_ARCH_NAME "arm64"
#  endif
#elif defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__) || \
      defined(__powerpc64) || defined(__ppc64) || defined(__PPC64) || \
      defined(__POWERPC64__) || defined(__POWERPC64) || defined(_M_PPC64)
#  define FFI_ARCH FFI_ARCH_PPC64
#  if defined(FFI_LITTLE_ENDIAN)
#    define FFI_ARCH_NAME "ppc64le"
#  else
#    define FFI_ARCH_NAME "ppc64"
#  endif
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__) || \
      defined(__powerpc) || defined(__ppc) || defined(__PPC) || \
      defined(__POWERPC__) || defined(__POWERPC) || defined(_M_PPC)
#  define FFI_ARCH FFI_ARCH_PPC
#  if defined(FFI_LITTLE_ENDIAN)
#    define FFI_ARCH_NAME "ppcle"
#  else
#    define FFI_ARCH_NAME "ppc"
#  endif
#elif defined(__mips64__) || defined(__mips64) || defined(__MIPS64__) || \
      defined(__MIPS64)
#  define FFI_ARCH FFI_ARCH_MIPS64
#  if __mips_isa_rev >= 6
#    if defined(FFI_LITTLE_ENDIAN)
#      define FFI_ARCH_NAME "mips64r6el"
#    else
#      define FFI_ARCH_NAME "mips64r6"
#    endif
#  else
#    if defined(FFI_LITTLE_ENDIAN)
#      define FFI_ARCH_NAME "mips64el"
#    else
#      define FFI_ARCH_NAME "mips64"
#    endif
#  endif
#elif defined(__mips__) || defined(__mips) || defined(__MIPS__) || \
      defined(__MIPS)
#  define FFI_ARCH FFI_ARCH_MIPS32
#  if __mips_isa_rev >= 6
#    if defined(FFI_LITTLE_ENDIAN)
#      define FFI_ARCH_NAME "mips32r6el"
#    else
#      define FFI_ARCH_NAME "mips32r6"
#    endif
#  else
#    if defined(FFI_LITTLE_ENDIAN)
#      define FFI_ARCH_NAME "mipsel"
#    else
#      define FFI_ARCH_NAME "mips"
#    endif
#  endif
#  ifdef __mips_soft_float
#    define FFI_ARCH_SOFTFP  1
#    define FFI_ARCH_HAS_FPU 0
#  else
#    define FFI_ARCH_SOFTFP  0
#    define FFI_ARCH_HAS_FPU 1
#  endif
#else
#  define FFI_ARCH FFI_ARCH_UNKNOWN
#  define FFI_ARCH_NAME "unknown"
#endif

#if FFI_ARCH == FFI_ARCH_ARM && defined(__ARM_EABI__)
#  define FFI_ARM_EABI 1
#endif

#if FFI_ARCH == FFI_ARCH_PPC && defined(__NO_FPRS__) && !defined(_SOFT_FLOAT)
#  define FFI_PPC_SPE 1
#endif

#ifndef FFI_ARCH_HAS_FPU
#if defined(_SOFT_FLOAT) || defined(_SOFT_DOUBLE)
#  define FFI_ARCH_HAS_FPU 0
#else
#  define FFI_ARCH_HAS_FPU 1
#endif
#endif

#ifndef FFI_ARCH_SOFTFP
#if defined(_SOFT_FLOAT) || defined(_SOFT_DOUBLE)
#  define FFI_ARCH_SOFTFP 1
#else
#  define FFI_ARCH_SOFTFP 0
#endif
#endif

#if FFI_OS == FFI_OS_WINDOWS || defined(FFI_OS_CYGWIN)
#  define FFI_WINDOWS_ABI 1
#endif

#ifdef _UWP
#  define FFI_WINDOWS_UWP 1
#  error "UWP support not yet implemented"
#endif

#if FFI_OS != FFI_OS_WINDOWS
#  define FFI_USE_DLFCN 1
#endif

/* some compiler bits */

#if defined(__GNUC__)
#if (__GNUC__ >= 5) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6))
#define FFI_DIAGNOSTIC_PRAGMA_GCC 1
#endif
#define WARN_UNUSED_RET __attribute__((warn_unused_result))
#define RESTRICT __restrict__
#else
#define WARN_UNUSED_RET
#define RESTRICT
#endif

#if defined(__clang__)
#define FFI_DIAGNOSTIC_PRAGMA_CLANG 1
#endif

/* MSVC warnings we don't care about/are irrelevant to us */

#ifdef _MSC_VER
/* conditional expression is constant */
#pragma warning(disable: 4127)
/* unsafe CRT, used only once */
#pragma warning(disable: 4996)
#endif

/* MSVC and clang */

#define _CRT_SECURE_NO_WARNINGS 1

#endif /* PLATFORM_HH */
