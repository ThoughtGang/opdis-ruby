/* Magic.c
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <magic.h>

#include <ruby.h>
#include "ruby_compat.h"

#include "Magic.h"

static VALUE modMagic;
static VALUE idFileNo;

static VALUE str_to_sym( const char * str ) {
	return rb_funcall(rb_str_new_cstr(str), rb_intern("to_sym"), 0);
}

/* ---------------------------------------------------------------------- */
/* Magic Module */

static const char * magic_for_io( magic_t magic, VALUE target ) {
	/* File : pass file descr to magic */
	/* NOTE: this closes the file descriptor! very bad! */
	int fd = NUM2INT(rb_funcall(target, idFileNo, 0));
	const char *m;

	rb_thread_schedule();
	return magic_descriptor( magic, fd );
}

static const char * magic_for_string( magic_t magic, VALUE target ) {
	/* String: pass C string to magic */
	rb_thread_schedule();
	return magic_buffer( magic, RSTRING_PTR(target), RSTRING_LEN(target) );
}

static const char * magic_for_array( magic_t magic, VALUE target ) {
	/* Array: generate string to pass to magic */
	int i;
	const char * result;
	size_t buf_len = RARRAY_LEN(target);
	unsigned char * buf = calloc(buf_len, 1);

	rb_thread_schedule();
	for ( i=0; i < buf_len; i++ ) {
		VALUE val = rb_ary_entry( target, i );
		buf[i] = (unsigned char) NUM2UINT(val);
	}
	result = magic_buffer( magic, buf, buf_len );

	free(buf);
	return result;
}

static void set_unless_false( VALUE options, const char *sym_name,
			      int flag, int * magic_options ) {
	VALUE option = rb_hash_lookup2(options, str_to_sym(sym_name), Qnil);
	if ( option == Qnil || option != Qfalse ) {
		*magic_options |= flag;
	}
}

static void set_if_present( VALUE options, const char *sym_name,
			    int flag, int * magic_options ) {
	VALUE option = rb_hash_lookup2(options, str_to_sym(sym_name), Qnil);
	if ( option != Qnil && option != Qfalse ) {
		*magic_options |= flag;
	}
}

static VALUE magic_method( VALUE mod, VALUE target, VALUE options ) {
	const char * result = NULL;
	static magic_t magic_dict = NULL;
	int flags = MAGIC_PRESERVE_ATIME;
	char * magic_file = NULL;
	VALUE val;

	/* get magic flags and open magic DB */
	/* default options (set unless user sets them to false) */
	set_unless_false( options, MAGIC_OPT_CONTINUE, MAGIC_CONTINUE, &flags );
	set_unless_false( options, MAGIC_OPT_SYMLINK, MAGIC_SYMLINK, &flags );
	set_unless_false( options, MAGIC_OPT_RAW, MAGIC_RAW, &flags );
	/* user-specified options */
	set_if_present( options, MAGIC_OPT_DEVICES, MAGIC_DEVICES, &flags );
	set_if_present( options, MAGIC_OPT_COMPRESS, MAGIC_COMPRESS, &flags );
	set_if_present( options, MAGIC_OPT_MIME, MAGIC_MIME_TYPE, &flags );
	set_if_present( options, MAGIC_OPT_CHARSET, MAGIC_MIME_ENCODING, 
			&flags );
	set_if_present( options, MAGIC_OPT_APPLE, MAGIC_APPLE, &flags );

	magic_dict = magic_open( flags );

	/* load magic file */
	val = rb_hash_lookup2(options, str_to_sym(MAGIC_OPT_MAGIC_FILE), Qnil);
	if ( val != Qnil ) {
		magic_file = StringValuePtr(val);
	}

	rb_thread_schedule();
	if ( 0 != magic_load(magic_dict, magic_file) ) {
		rb_raise(rb_eArgError, "Magic is not a valid magic file");
	}

	/* apply magic to input */
	if ( Qtrue == rb_obj_is_kind_of( target, rb_cIO ) ) {
		result = magic_for_io( magic_dict, target );

	} else if ( Qtrue == rb_obj_is_kind_of( target, rb_cString ) ) {
		result = magic_for_string( magic_dict, target );

	} else if ( Qtrue == rb_obj_is_kind_of( target, rb_cArray ) ) {
		result = magic_for_array( magic_dict, target );

	} else {
		rb_raise(rb_eArgError, 
			 "Target must be a String, Array, or IO object");
	}

	/* return result as a string */
	val = result ? rb_str_new_cstr(result) : Qnil;
	magic_close(magic_dict);	/* NOTE: This frees 'result' */

	return val;
}


static void init_magic( VALUE mod ) {
	rb_define_module_function( mod, MAGIC_METHOD_NAME, magic_method, 2 ); 
}

void Init_MagicExt() {
	idFileNo = rb_intern("fileno");
	modMagic = rb_define_module(MAGIC_MODULE_NAME);
	init_magic(modMagic);
}
