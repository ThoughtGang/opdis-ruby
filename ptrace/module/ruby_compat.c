/* ruby_compat.c
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef RUBY_19

#include <ruby.h>

VALUE Bfd_rb_hash_lookup2( VALUE hash, VALUE key, VALUE def ) {
	VALUE v = rb_hash_lookup(hash, key);
	return (v == Qnil) ? def : v;
}

#endif
