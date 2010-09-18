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


static VALUE str_to_sym( const char * str ) {
	VALUE var = rb_str_new_cstr(str);
	return rb_funcall(var, rb_intern("to_sym"), 0);
}

/* ---------------------------------------------------------------------- */
/* Target Class */

static void fill_arch_info( const struct bfd_arch_info * info, VALUE * hash ) {
	rb_hash_aset( *hash, str_to_sym(AINFO_MEMBER_BPW), 
		      INT2NUM(info->bits_per_word) );
	rb_hash_aset( *hash, str_to_sym(AINFO_MEMBER_BPA), 
		      INT2NUM(info->bits_per_address) );
	rb_hash_aset( *hash, str_to_sym(AINFO_MEMBER_BPB), 
		      INT2NUM(info->bits_per_byte) );
	rb_hash_aset( *hash, str_to_sym(AINFO_MEMBER_ALIGN), 
		      INT2NUM(info->section_align_power) );
	rb_hash_aset( *hash, str_to_sym(AINFO_MEMBER_ARCH), 
		      rb_str_new_cstr(bfd_printable_arch_mach(info->arch,
							      info->mach)) );
}

static VALUE cls_target_new(VALUE class, VALUE tgt, VALUE hash) {
	bfd * abfd;
	VALUE instance, var;
	VALUE argv[1] = { Qnil };

	if ( Qtrue == rb_obj_is_kind_of( tgt, rb_cIO) ) {
		int fd = NUM2INT(rb_funcall(tgt, rb_intern("fileno"), 0));
		VALUE rb_path = rb_funcall(tgt, rb_intern("path"), 0);
		char * path = StringValuePtr(rb_path);
		abfd = bfd_fdopenr (path, NULL, fd);

	} else if ( Qtrue == rb_obj_is_kind_of( tgt, rb_cString) ) {
		char * path = StringValuePtr(tgt);
		abfd = bfd_openr (path, NULL);

	} else {
		rb_raise(rb_eArgError, "Magic requires a path or IO object");
	}

	// TODO: process hash argument supporting flavour, arch, and mach info

	if (! abfd || abfd == (bfd *) bfd_error_invalid_target ) {
		bfd_error_type err = bfd_get_error();
		rb_raise(rb_eRuntimeError, "Magic error (%d): %s", err, 
			 bfd_errmsg(err) );
	}

	bfd_check_format( abfd, bfd_object );

	if ( bfd_get_flavour( abfd ) == bfd_target_unknown_flavour ) {
		/* we do not want to raise an exception, as the Magic is
		 * still usable, just empty */
		fprintf( stderr, "[Magic] Warning: unknown Magic flavour\n" );
	}

	instance = Data_Wrap_Struct(class, NULL, bfd_close, abfd);
	rb_obj_call_init(instance, 0, argv);

	/* set instance variables */
	rb_iv_set(instance, IVAR(TGT_ATTR_ID), INT2NUM(abfd->id) );
	rb_iv_set(instance, IVAR(TGT_ATTR_FILENAME), 
		  rb_str_new_cstr(abfd->filename) );
	rb_iv_set(instance, IVAR(TGT_ATTR_FORMAT), 
		  rb_str_new_cstr(bfd_format_string(abfd->format)) );
	rb_iv_set(instance, IVAR(TGT_ATTR_FMT_FLAGS), INT2NUM(abfd->flags) );
	rb_iv_set(instance, IVAR(TGT_ATTR_FLAVOUR), 
		  INT2NUM(abfd->xvec->flavour) );
	rb_iv_set(instance, IVAR(TGT_ATTR_TYPE), 
		  rb_str_new_cstr(abfd->xvec->name) );
	rb_iv_set(instance, IVAR(TGT_ATTR_TYPEFLAGS), 
		  INT2NUM(abfd->xvec->object_flags) );
	rb_iv_set(instance, IVAR(TGT_ATTR_ENDIAN), 
		  INT2NUM(abfd->xvec->byteorder) );
	rb_iv_set(instance, IVAR(TGT_ATTR_START_ADDR), 
		  SIZET2NUM(abfd->start_address) );
	rb_iv_set(instance, IVAR(TGT_ATTR_SECTIONS), Qnil); 
	rb_iv_set(instance, IVAR(TGT_ATTR_SYMBOLS), Qnil); 

	var = rb_hash_new();
	fill_arch_info( bfd_get_arch_info(abfd), &var );
	rb_iv_set(instance, IVAR(TGT_ATTR_ARCH_INFO), var );

	return instance;
}

static VALUE cls_target_sections(VALUE instance) {
	/* lazy-loading of section list */
	VALUE var = rb_iv_get(instance, IVAR(TGT_ATTR_SECTIONS));
	if ( var == Qnil ) {
		bfd * abfd;
		var = rb_hash_new();

		Data_Get_Struct(instance, bfd, abfd);
		bfd_map_over_sections( abfd, add_section_to_hash, &var );
		rb_iv_set(instance, IVAR(TGT_ATTR_SECTIONS), var); 
	}
	return var;
}

static void load_static_syms( bfd * abfd, VALUE * hash ) {
	size_t size;
	unsigned int i, num;
	asymbol ** syms;

	size = bfd_get_symtab_upper_bound(abfd);
	if ( size <= 0 ) {
		return;
	}

	syms = (struct bfd_symbol **) malloc(size);
	if (! syms ) {
		return;
	}

	num = bfd_canonicalize_symtab(abfd, syms);
	for ( i=0; i < num; i++ ) {
		add_symbol_to_hash( abfd, syms[i], hash, 0 );
	}

	free(syms);
}

static void load_dyn_syms( bfd * abfd, VALUE * hash ) {
	size_t size;
	unsigned int i, num;
	asymbol ** syms;

	size = bfd_get_dynamic_symtab_upper_bound(abfd);
	if ( size <= 0 ) {
		return;
	}

	syms = (asymbol **) malloc(size);
	if (! syms ) {
		return;
	}

	num = bfd_canonicalize_dynamic_symtab(abfd, syms);
	for ( i=0; i < num; i++ ) {
		add_symbol_to_hash( abfd, syms[i], hash, 1 );
	}

	free(syms);
}
		       
static VALUE cls_target_symbols(VALUE instance) {
	/* Lazy loading of symbols */
	VALUE var = rb_iv_get(instance, IVAR(TGT_ATTR_SYMBOLS));
	if ( var == Qnil ) {
		var = rb_hash_new();
		bfd * abfd;
		Data_Get_Struct(instance, bfd, abfd);
		if ( bfd_get_file_flags(abfd) & HAS_SYMS ) {
			/* GNU uses macros, not functions, for these Magic
			 * routines so we cannot abstract to fn pointers */
			load_static_syms( abfd, &var );
			load_dyn_syms( abfd, &var );
		}
		rb_iv_set(instance, IVAR(TGT_ATTR_SYMBOLS), var); 
	}
	return var;
}

static void init_target_class( VALUE modMagic ) {
	clsTarget = rb_define_class_under(modMagic, TARGET_CLASS_NAME, 
					  rb_cObject);
	rb_define_singleton_method(clsTarget, "ext_new", cls_target_new, 2);
	
	/* attributes (read-only) */
	rb_define_attr(clsTarget, TGT_ATTR_ID, 1, 0);
	rb_define_attr(clsTarget, TGT_ATTR_FILENAME, 1, 0);
	rb_define_attr(clsTarget, TGT_ATTR_FORMAT, 1, 0);
	rb_define_attr(clsTarget, TGT_ATTR_FMT_FLAGS, 1, 0);
	rb_define_attr(clsTarget, TGT_ATTR_FLAVOUR, 1, 0);
	rb_define_attr(clsTarget, TGT_ATTR_TYPE, 1, 0);
	rb_define_attr(clsTarget, TGT_ATTR_TYPEFLAGS, 1, 0);
	rb_define_attr(clsTarget, TGT_ATTR_ENDIAN, 1, 0);
	rb_define_attr(clsTarget, TGT_ATTR_START_ADDR, 1, 0);
	rb_define_attr(clsTarget, TGT_ATTR_ARCH_INFO, 1, 0);

	rb_define_method(clsTarget, TGT_ATTR_SECTIONS, cls_target_sections, 0);
	rb_define_method(clsTarget, TGT_ATTR_SYMBOLS, cls_target_symbols, 0);

	bfd_init();
}

/* ---------------------------------------------------------------------- */
/* Magic Module */

static const char * magic_for_io( magic_t magic, VALUE target ) {
	/* File : pass file descr to magic */
	int fd = NUM2INT(rb_funcall(target, idFileNo, 0));

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
	result = magic_buffer( magic_dict, buf, buf_len );

	free(buf);
	return result;
}

/*
 * Options:
 * 	mime
 * 	apple
 * 	devices
 * 	compress
 * Default options:
 * 	continue
 * 	raw
 * 	symlink
 * 	preserve atime
 */
static VALUE magic_method( VALUE mod, VALUE target, VALUE magic, 
			   VALUE options ) {
	const char * result = NULL;
	static magic_t magic_dict = NULL;
	int magic_options = MAGIC_CONTINUE | MAGIC_SYMLINK | MAGIC_RAW |
			    MAGIC_PRESERVE_ATIME;
	char * magic_file = StringValuePtr(magic_file);

	magic_dict = magic_open( MAGIC_NONE );

	// check options for mime, etc
	//
	// MAGIC_DEVICES ?
	// MAGIC_MIME
	// MAGIC_APPLE

	magic_setflags( magic_options );

	// load magic file
	// NOTE: ruby module should set this to internal file
	rb_thread_schedule();
	if ( 0 != magic_load(magic_dict, magic_file) ) {
		rb_raise(rb_eArgError, "Magic is not a valid magic file");
	}

	if ( Qtrue == rb_obj_is_kind_of( target, rb_cIO ) ) {
		result = magic_for_file( magic_dict, target );

	} else if ( Qtrue == rb_obj_is_kind_of( target, rb_cString ) ) {
		result = magic_for_string( magic_dict, target );

	} else if ( Qtrue == rb_obj_is_kind_of( target, rb_cArray ) ) {
		result = magic_for_array( magic_dict, target );

	} else {
		rb_raise(rb_eArgError, 
			 "Target must be a String, Array, or IO object");
	}

	magic_close(magic_dict);

	return  result ? rb_str_new_cstr(result) : Qnil;
}


static void init_magic( VALUE mod ) {

	rb_define_module_function( mod, name, magic_method, 1 ); 
}

void Init_MagicExt() {
	modMagic = rb_define_module(MAGIC_MODULE_NAME);
	init_magic(modMagic);
}
