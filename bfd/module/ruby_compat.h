/* ruby_compat.h
 * Macros to make ruby1.8 look like ruby1.9.
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef RUBY_COMPAT_H
#define RUBY_COMPAT_H

#ifdef RUBY_18
#include <stdarg.h>
#define rb_str_new_cstr(arg) rb_str_new2(arg)

#define rb_hash_lookup2( a1, a2, a3 ) Bfd_rb_hash_lookup2(a1, a2, a3)

#if SIZEOF_SIZE_T > SIZEOF_LONG && defined(HAVE_LONG_LONG)
# define SIZET2NUM(v) ULL2NUM(v)
#elif SIZEOF_SIZE_T == SIZEOF_LONG
# define SIZET2NUM(v) ULONG2NUM(v)
#else                                                                           
# define SIZET2NUM(v) UINT2NUM(v)
#endif

#endif
#endif
