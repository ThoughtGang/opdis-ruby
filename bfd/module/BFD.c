/* Bfd.c
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <bfd.h>

#include <ruby.h>
#include "ruby_compat.h"

#include "BFD.h"

#define IVAR(attr) "@" attr

static VALUE symFileno;
static VALUE symPath;

static VALUE modBfd;
static VALUE clsTarget;
static VALUE clsSection;
static VALUE clsSymbol;


static VALUE str_to_sym( const char * str ) {
	VALUE var = rb_str_new_cstr(str);
	return rb_funcall(var, rb_intern("to_sym"), 0);
}

/* ---------------------------------------------------------------------- */
/* Symbol Class */

static VALUE symbol_new(bfd * abfd, asymbol *s, char is_dynamic) {
	symbol_info info;
	VALUE class, instance;
	VALUE argv[1] = { Qnil };

	class = rb_class_new(clsSymbol);
	instance = Data_Wrap_Struct(class, NULL, NULL, s);
	rb_obj_call_init(instance, 0, argv);

	bfd_symbol_info(s, &info);

	/* set instance variables */
	rb_iv_set(instance, IVAR(SYM_ATTR_NAME), rb_str_new_cstr(info.name) );
	rb_iv_set(instance, IVAR(SYM_ATTR_TYPE), INT2NUM((int) info.type) );
	rb_iv_set(instance, IVAR(SYM_ATTR_VALUE), SIZET2NUM(info.value) );
	rb_iv_set(instance, IVAR(SYM_ATTR_FLAGS), INT2NUM(s->flags) );
	rb_iv_set(instance, IVAR(SYM_ATTR_BIND), 
		  rb_str_new_cstr( (is_dynamic ? SYM_BIND_DYNAMIC : 
				  		 SYM_BIND_STATIC) ) );
	if ( s->section ) {
		rb_iv_set(instance, IVAR(SYM_ATTR_SECTION), 
			  rb_str_new_cstr(s->section->name)); 
	}

	return instance;
}

static void add_symbol_to_hash( bfd * abfd, asymbol * s, PTR data,
			        char is_dynamic ) {
	VALUE sym;
	VALUE * hash = (VALUE *) data;

	if (! abfd || ! s ) {
		return;
	}

	sym = symbol_new(abfd, s, is_dynamic);
	rb_hash_aset( *hash, rb_iv_get(sym, IVAR(SYM_ATTR_NAME)), sym);
}

static void init_symbol_class( VALUE modBfd ) {
	/* NOTE: Symbol does not support instantiation via .new() */
	clsSymbol = rb_define_class_under(modBfd, SYMBOL_CLASS_NAME, 
					  rb_cObject);
	
	/* attributes (read-only) */
	rb_define_attr(clsSymbol, SYM_ATTR_NAME, 1, 0);
	rb_define_attr(clsSymbol, SYM_ATTR_VALUE, 1, 0);
	rb_define_attr(clsSymbol, SYM_ATTR_FLAGS, 1, 0);
	rb_define_attr(clsSymbol, SYM_ATTR_SECTION, 1, 0);
	rb_define_attr(clsSymbol, SYM_ATTR_BIND, 1, 0);

	/* constants */
	rb_define_const(clsSymbol, SYM_BIND_DYN_NAME,
			rb_str_new_cstr(SYM_BIND_DYNAMIC));
	rb_define_const(clsSymbol, SYM_BIND_STAT_NAME,
			rb_str_new_cstr(SYM_BIND_STATIC));
}

/* ---------------------------------------------------------------------- */
/* Section Class */

// TODO: relocs/relfilepos, lines/linefilepos

static VALUE cls_section_contents(VALUE instance) {
	/* lazy-loading of section list */
	VALUE var = rb_iv_get(instance, IVAR(SEC_ATTR_CONTENTS));
	if ( var == Qnil ) {
		unsigned char * buf;
		unsigned int size;
		asection * sec;

		var = Qnil;
		Data_Get_Struct(instance, asection, sec);
		size = bfd_section_size( sec->owner, sec );
		buf = calloc( size, 1 );
		if ( buf && 
		     bfd_get_section_contents(sec->owner, sec, buf, 0, size) ) {
			var = rb_str_new( (const char *) buf, size );
			rb_iv_set(instance, IVAR(SEC_ATTR_CONTENTS), var); 
		}
	}
	return var;
}

static VALUE section_new(bfd * abfd, asection *s) {
	VALUE class, instance;
	VALUE argv[1] = { Qnil };

	class = rb_class_new(clsSection);
	instance = Data_Wrap_Struct(class, NULL, NULL, s);
	rb_obj_call_init(instance, 0, argv);

	/* set instance variables */
	rb_iv_set(instance, IVAR(SEC_ATTR_ID), INT2NUM(s->id) );
	rb_iv_set(instance, IVAR(SEC_ATTR_NAME), rb_str_new_cstr(s->name) );
	rb_iv_set(instance, IVAR(SEC_ATTR_INDEX), INT2NUM(s->index) );
	rb_iv_set(instance, IVAR(SEC_ATTR_FLAGS), INT2NUM(s->flags) );
	rb_iv_set(instance, IVAR(SEC_ATTR_VMA), SIZET2NUM(s->vma) );
	rb_iv_set(instance, IVAR(SEC_ATTR_LMA), SIZET2NUM(s->lma) );
	rb_iv_set(instance, IVAR(SEC_ATTR_SIZE), 
		  SIZET2NUM(bfd_section_size(abfd, s)) );
	rb_iv_set(instance, IVAR(SEC_ATTR_ALIGN), INT2NUM(s->alignment_power) );
	rb_iv_set(instance, IVAR(SEC_ATTR_FPOS), SIZET2NUM(s->filepos) );
	rb_iv_set(instance, IVAR(SEC_ATTR_CONTENTS), Qnil); 

	return instance;
}

static void add_section_to_hash( bfd * abfd, asection * s, PTR data ) {
	VALUE sec;
	VALUE * hash = (VALUE *) data;
	if (! abfd || ! s ) {
		return;
	}
	sec = section_new(abfd, s);
	rb_hash_aset( *hash, rb_iv_get(sec, IVAR(SEC_ATTR_NAME)), sec);
}

static void init_section_class( VALUE modBfd ) {
	/* NOTE: Section does not support instantiation via .new() */
	clsSection = rb_define_class_under(modBfd, SECTION_CLASS_NAME, 
					   rb_cObject);
	
	/* attributes (read-only) */
	rb_define_attr(clsSection, SEC_ATTR_ID, 1, 0);
	rb_define_attr(clsSection, SEC_ATTR_NAME, 1, 0);
	rb_define_attr(clsSection, SEC_ATTR_INDEX, 1, 0);
	rb_define_attr(clsSection, SEC_ATTR_FLAGS, 1, 0);
	rb_define_attr(clsSection, SEC_ATTR_VMA, 1, 0);
	rb_define_attr(clsSection, SEC_ATTR_LMA, 1, 0);
	rb_define_attr(clsSection, SEC_ATTR_SIZE, 1, 0);
	rb_define_attr(clsSection, SEC_ATTR_ALIGN, 1, 0);
	rb_define_attr(clsSection, SEC_ATTR_FPOS, 1, 0);
	rb_define_attr(clsSection, SEC_ATTR_SYM, 1, 0);

	rb_define_method(clsSection, SEC_ATTR_CONTENTS, cls_section_contents, 
			 0);
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

static VALUE new_target_for_bfd(VALUE class, bfd * abfd, VALUE hash) {
	VALUE instance, var;
	VALUE argv[1] = { Qnil };
	// TODO: process hash argument supporting flavour, arch, and mach info

	if (! abfd || abfd == (bfd *) bfd_error_invalid_target ) {
		bfd_error_type err = bfd_get_error();
		rb_raise(rb_eRuntimeError, "BFD error (%d): %s", err, 
			 bfd_errmsg(err) );
	}

	bfd_check_format( abfd, bfd_object );

	if ( bfd_get_flavour( abfd ) == bfd_target_unknown_flavour ) {
		/* we do not want to raise an exception, as the BFD is
		 * still usable, just empty */
		fprintf( stderr, "[BFD] Warning: unknown BFD flavour\n" );
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

static VALUE cls_target_new(VALUE class, VALUE tgt, VALUE hash) {
	bfd * abfd = NULL;

	if ( rb_respond_to( tgt, symFileno ) ) {
		int fd;
		VALUE fd_val = rb_funcall(tgt, symFileno, 0);
		VALUE rb_path = rb_funcall(tgt, symPath, 0);
		char * path = StringValuePtr(rb_path);
		if ( Qnil == fd_val ) {
			rb_raise(rb_eArgError, "Invalid fileno() in IO object");
		}
		fd = NUM2INT(fd_val);
		abfd = bfd_fdopenr (path, NULL, fd);
		if (! abfd ) {
			bfd_error_type err = bfd_get_error();
			rb_raise(rb_eRuntimeError, "BFD error (%d) in open: %s",
				 err, bfd_errmsg(err) );
		}

	} else if ( Qtrue == rb_obj_is_kind_of( tgt, rb_cString) ) {
		char * path = StringValuePtr(tgt);
		abfd = bfd_openr (path, NULL);

	} else {
		rb_raise(rb_eArgError, "Bfd requires a path or IO object");
	}

	return new_target_for_bfd( class, abfd, hash );
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
			/* GNU uses macros, not functions, for these BFD
			 * routines so we cannot abstract to fn pointers */
			load_static_syms( abfd, &var );
			load_dyn_syms( abfd, &var );
		}
		rb_iv_set(instance, IVAR(TGT_ATTR_SYMBOLS), var); 
	}
	return var;
}

static void init_target_class( VALUE modBfd ) {
	clsTarget = rb_define_class_under(modBfd, TARGET_CLASS_NAME, 
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
/* BFD Module */

void Init_BFDext() {
	symFileno = rb_intern("fileno");
	symPath = rb_intern("path");

	modBfd = rb_define_module(BFD_MODULE_NAME);

	init_target_class(modBfd);
	init_section_class(modBfd);
	init_symbol_class(modBfd);
}
