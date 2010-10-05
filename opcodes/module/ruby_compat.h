/* ruby1.8_compat.h
 * Macros to make ruby1.8 look like ruby1.9.
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef RUBY_18_COMPAT_H
#define RUBY_18_COMPAT_H

#include <ruby.h>

#ifdef RUBY_18
#include <stdarg.h>
#define rb_str_new_cstr(arg) rb_str_new2(arg)

VALUE Opcodes_rb_hash_lookup2( VALUE, VALUE, VALUE );
#define rb_hash_lookup2( a1, a2, a3 ) Opcodes_rb_hash_lookup2(a1, a2, a3)
#endif

VALUE Opcodes_path2class(const char * path);
#define path2class(path) Opcodes_path2class(path)

#endif
