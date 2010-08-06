/* ruby_compat.c
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <ruby.h>

#ifdef RUBY_18

VALUE Opdis_rb_hash_lookup2( VALUE hash, VALUE key, VALUE def ) {
	VALUE v = rb_hash_lookup(hash, key);
	return (v == Qnil) ? def : v;
}

#endif

/* ---------------------------------------------------------------------- */
/* rb_path2class from variable.c reimplemented to NOT throw exceptions
 * when a class isn't found. */
VALUE Opdis_path2class(const char * path) {
	const char *pbeg, *p;
	ID id;
	VALUE c = rb_cObject;

	pbeg = p = path;

	if (path[0] == '#') {
		rb_raise(rb_eArgError, "can't retrieve anonymous class %s", 
			 path);
	}

	while (*p) {
#ifdef RUBY_18
		VALUE str;
		while (*p && *p != ':') p++;
		str = rb_str_new(pbeg, p-pbeg);
		id = rb_intern(RSTRING(str)->ptr);
#endif

#ifdef RUBY_19
		while (*p && *p != ':') p++;
		id = rb_intern2(pbeg, p-pbeg);
#endif
		if (p[0] == ':') {
			if (p[1] != ':') {
				rb_raise(rb_eArgError, 
					 "undefined class/module %.*s", 
					 (int)(p-path), path);
			}
			p += 2;
			pbeg = p;
		}

		if (!rb_const_defined(c, id)) {
			return Qnil;
		}

		c = rb_const_get_at(c, id);
		switch (TYPE(c)) {
			case T_MODULE:
			case T_CLASS:
			break;
			default:
			rb_raise(rb_eTypeError, 
				 "%s does not refer to class/module", path);
			}
		}

	return c;
}
