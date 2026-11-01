#ifndef __CGL_COMMON_H
#define __CGL_COMMON_H

#include <stdint.h>
#include <stdbool.h>

/* clang-format off */
#define __cgl_macro_for_each_primitive(macro) \
	macro(char, c)                        \
	macro(signed char, sc)                \
	macro(unsigned char, uc)              \
	macro(short, s)                       \
	macro(unsigned short, us)             \
	macro(int, i)                         \
	macro(unsigned int, ui)               \
	macro(long, l)                        \
	macro(unsigned long, ul)              \
	macro(long long, ll)                  \
	macro(unsigned long long, ull)        \
	macro(bool, b)                        \
	macro(float, f)                       \
	macro(double, d)                      \
	macro(long double, ld)

#define __cgl_macro_for_each_type(macro)      \
	__cgl_macro_for_each_primitive(macro) \
	macro(void *, v)
/* clang-format on */

#endif
