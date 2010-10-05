/* Opcodes.c
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <dis-asm.h>
#include <ruby.h>

#include "ruby_compat.h"

#include "Opcodes.h"
#include "Arch.h"

#ifdef RUBY_18
#define IVAR(attr) attr
#else
#define IVAR(attr) "@" attr
#endif

static VALUE modOpcodes;
static VALUE clsDisasm;

static VALUE str_to_sym( const char * str ) {
	VALUE var = rb_str_new_cstr(str);
	return rb_funcall(var, rb_intern("to_sym"), 0);
}

static int generic_print_address_wrapper(bfd_vma vma, disassemble_info *info ) {
	generic_print_address(vma, info);
	return 1;
}

/* ---------------------------------------------------------------------- */
struct disasm_def_for_bfd {
	bfd * abfd;
	disassembler_ftype fn;
};

static int is_def_for_bfd( const Opcodes_disasm_def * def, void * arg ) {
	struct disasm_def_for_bfd * out = (struct disasm_def_for_bfd *) arg;
	if ( def->arch == out->abfd->arch_info->arch &&
	     def->arch == out->abfd->arch_info->mach ) {
		out->fn = def->fn;
		return 0;
	}

	return 1;	/* not found; continue */
}

/* return disassembler fn for BFD */
static disassembler_ftype fn_for_bfd( bfd * abfd ) {
	struct disasm_def_for_bfd arg = { abfd, NULL };
	const Opcodes_disasm_def * invalid = Opcodes_disasm_invalid();

	/* initialize to generic_print_address_wrapper */
	arg.fn = invalid->fn;

	Opcodes_disasm_iter( is_def_for_bfd, &arg );

	return arg.fn;
}

/* configure disassemble_info for specified architecture */
static void config_disasm_arch( struct disassemble_info * info, VALUE str ) {
	const Opcodes_disasm_def * def;
	const char * name = rb_string_value_cstr(&str);

	def = Opcodes_disasm_for_name( name );
	info->application_data = def->fn;
	info->arch = def->arch;
	info->mach = def->mach;
}

static int fill_disasm_def_array( const Opcodes_disasm_def * def, void * arg ) {
	VALUE * ary = (VALUE *) arg;
	const Opcodes_disasm_def * invalid = Opcodes_disasm_invalid();

	/* do not report the invalid architecture */
	if ( def != invalid ) {
		rb_ary_push( *ary, rb_str_new_cstr(def->name) );
	}

	return 1;
}

/* fill array with available disassemblers */
static void get_available_disassemblers( VALUE * ary ) {
	Opcodes_disasm_iter( fill_disasm_def_array, ary );
}

/* ---------------------------------------------------------------------- */
/* convert instruction type code to string */
static const char * insn_type_to_str( enum dis_insn_type t ) {
	const char *s;
	switch (t) {
		case dis_noninsn: s = "Invalid"; break;
		case dis_nonbranch: s = "Not branch"; break;
		case dis_branch: s = "Unconditional branch"; break;
		case dis_condbranch: s = "Conditional branch"; break;
		case dis_jsr: s = "Jump to subroutine"; break;
		case dis_condjsr: s = "Conditional jump to subroutine"; break;
		case dis_dref: s = "Data reference"; break;
		case dis_dref2: s = "Two data references"; break;
	}
	return s;
}

/* ---------------------------------------------------------------------- */
/* Disassembler class */

/* libopcodes callback */
static int disasm_fprintf( void * stream, const char * format, ... ) {
	char buf[DISASM_MAX_STR];
	int rv;
	VALUE ary = (VALUE) stream;

	va_list args;
	va_start (args, format);
	rv = vsnprintf( buf, DISASM_MAX_STR - 1, format, args );
	rb_ary_push( ary, rb_str_new_cstr(buf) ); 
        va_end (args);

	return rv;
}

/* fill instruction info hash based on disassemble_info */
static VALUE disasm_insn_info( struct disassemble_info * info ) {
	VALUE hash = rb_hash_new();

	if (! info->insn_info_valid ) {
		return hash;
	}

	rb_hash_aset( hash, str_to_sym(DIS_INSN_INFO_DELAY), 
		      INT2NUM((int) info->branch_delay_insns) );
	rb_hash_aset( hash, str_to_sym(DIS_INSN_INFO_DATA_SZ), 
		      INT2NUM((int) info->data_size) );
	rb_hash_aset( hash, str_to_sym(DIS_INSN_INFO_TYPE), 
		      rb_str_new_cstr(insn_type_to_str(info->insn_type)) );
	rb_hash_aset( hash, str_to_sym(DIS_INSN_INFO_TGT), 
		      INT2NUM(info->target) );
	rb_hash_aset( hash, str_to_sym(DIS_INSN_INFO_TGT2), 
		      INT2NUM(info->target2) );

	return hash;
}

/* disassemble a single instruction at address, returning a ruby hash. */
static VALUE disasm_insn( struct disassemble_info * info, bfd_vma vma,
			  unsigned int * length ) {
	disassembler_ftype fn;
	int size;
	VALUE ary = rb_ary_new();
	VALUE hash = rb_hash_new();

	if ( vma < info->buffer_vma ) {
		/* assume small VMAs are offsets into buffer */
		vma += info->buffer_vma;
	}

	if ( vma >= info->buffer_vma + info->buffer_length) {
		rb_raise(rb_eArgError, "VMA %d exceeds buffer length", 
			 (int) vma);
	}

	/* prepare info for insn disassmbly */
	info->insn_info_valid = 0;
	info->stream = (void *) ary;

	/* invoke disassembly */
	fn = (disassembler_ftype) info->application_data;
	size = fn( vma, info );

	/* increase # bytes disassembled */
	if ( length ) {
		*length += size;
	}

	/* fill output hash */
	rb_hash_aset( hash, str_to_sym(DIS_INSN_VMA), INT2NUM(vma) );
	rb_hash_aset( hash, str_to_sym(DIS_INSN_SIZE), INT2NUM(size) );
	rb_hash_aset( hash, str_to_sym(DIS_INSN_INFO), disasm_insn_info(info) );
	rb_hash_aset( hash, str_to_sym(DIS_INSN_INSN), rb_ary_dup(ary) );

	return hash;
}

/* support for different target types */
struct disasm_target {
	unsigned char * buf;
	unsigned int buf_len;
	asection * sec;
	asymbol * sym;
	bfd * abfd;
	unsigned int ruby_manages_buf;
};

/* fill disassemble_info struct based on contents of target struct */
static void config_libopcodes_for_target( struct disassemble_info * info, 
					  struct disasm_target * tgt ) {
	info->buffer_vma = info->buffer_length = 0;
	info->buffer = NULL;


	if ( tgt->sec ) {
		info->buffer_vma = tgt->sec->vma;
		info->buffer_length = tgt->sec->size;
		info->buffer = tgt->buf;

	} else if ( tgt->sym ) {
		unsigned int vma_off;
		symbol_info sym;
		asection * sec = tgt->sym->section;

		bfd_symbol_info(tgt->sym, &sym);

		if (! sym.value || (sym.value > sec->vma + sec->size ) ) {
			rb_raise(rb_eRuntimeError, "Invalid symbol value 0x%X",
				 ((unsigned long) sym.value));
		}
			
		vma_off = sym.value - sec->vma;

		/* disassembly buffer is set to start of symbol in section */
		info->buffer_vma = sym.value;
		info->buffer = &tgt->buf[vma_off];
		info->buffer_length = sec->size - vma_off;

	} else if ( tgt->buf ) {
		/* entire buffer is loaded at offset 9 */
		info->buffer_length = tgt->buf_len;
		info->buffer = tgt->buf;
	}

	if ( tgt->abfd && (! info->application_data ||
	     info->application_data == generic_print_address_wrapper) ) {
		info->application_data = fn_for_bfd( tgt->abfd );
	}

}

/* fill target struct based on ruby inout value */
static void load_target( VALUE tgt, struct disasm_target * dest ) {

	memset( dest, 0, sizeof(struct disasm_target) );

	if ( Qtrue == rb_obj_is_kind_of( tgt, rb_cString) ) {
		/* string of bytes */
		dest->buf = (unsigned char *) RSTRING_PTR(tgt);
		dest->buf_len = RSTRING_LEN(tgt);
		dest->ruby_manages_buf = 1;

	} else if ( Qtrue == rb_obj_is_kind_of( tgt, rb_cArray) ) {
		/* array of bytes */
		int i;
		dest->buf_len = RARRAY_LEN(tgt);
		dest->buf = calloc(dest->buf_len, 1);
		for( i=0; i < dest->buf_len; i++ ) {
			VALUE val = rb_ary_entry( tgt, i );
			dest->buf[i] = (unsigned char) NUM2UINT(val);
		}

	} else if ( Qtrue == rb_obj_is_kind_of( tgt, rb_cIO) ) {
		/* IO (file) object */
		VALUE str = rb_funcall( tgt, rb_intern("read"), 0 );
		dest->buf = (unsigned char*) RSTRING_PTR(str);
		dest->buf_len = RSTRING_LEN(str);
		dest->ruby_manages_buf = 1;

	} else if ( Qtrue == rb_obj_is_kind_of( tgt, 
	      				path2class("Bfd::Section") ) ) {
		/* BFD Section */
		Data_Get_Struct(tgt, asection, dest->sec);
		if ( dest->sec ) {
			dest->abfd = dest->sec->owner;
			/* load section contents */
			bfd_malloc_and_get_section( dest->abfd, dest->sec, 
						    &dest->buf );
		}

	} else if ( Qtrue == rb_obj_is_kind_of( tgt, 
	      				path2class("Bfd::Symbol") ) ) {
		/* BFD Symbol */
		Data_Get_Struct(tgt, asymbol, dest->sym);
		if ( dest->sym ) {
			dest->abfd = dest->sym->the_bfd;
			/* load contents of section containing symbol */
			bfd_malloc_and_get_section( dest->abfd, 
						    dest->sym->section, 
						    &dest->buf );
		}

	} else {
		rb_raise(rb_eArgError, 
			"Expecting IO, String, Bfd::Target,or Bfd::Section");
	}
}

/* free any memory allocated when loading target */
static void unload_target( struct disasm_target * tgt ) {
	if ( tgt->buf && ! tgt->ruby_manages_buf ) {
		free(tgt->buf);
	}
}

/* shared code for loading a target, configuring libopcodes, and getting
 * options */
static void disasm_init( struct disassemble_info * info, 
			 struct disasm_target * target, bfd_vma * vma, 
			 VALUE class, VALUE tgt, VALUE hash ) {
	const char *opts;
	bfd_vma vma_arg;
	VALUE var;

	load_target( tgt, target );
	config_libopcodes_for_target( info, target );

	/* override vma if caller requested it */
	vma_arg = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_BUFVMA), Qnil);
	if ( vma_arg != Qnil ) {
		info->buffer_vma = NUM2UINT(vma_arg);
	}
	
	/* disassembly options: offset/vma to disasm, vma of buffer */
	/* vma to disassemble */
	vma_arg = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_VMA), Qnil);
	*vma = (vma_arg == Qnil) ? info->buffer_vma : NUM2UINT(vma_arg);

	/* libopcodes disassembler options */
	var = rb_iv_get(class, IVAR(DIS_ATTR_OPTIONS));
	if ( var != Qnil ) {
		info->disassembler_options = StringValueCStr( var );
	}
}

/* disassemble a single instruction */
static VALUE cls_disasm_single(VALUE class, VALUE tgt, VALUE hash) {
	struct disassemble_info * info;
	struct disasm_target target;
	bfd_vma vma;
	VALUE result;

	Data_Get_Struct(class, struct disassemble_info, info);
	if (! info ) {
		rb_raise( rb_eRuntimeError, "Invalid disassemble_info" );
	}

	disasm_init( info, &target, &vma, class, tgt, hash );

	result = disasm_insn( info, vma, NULL );

	unload_target(&target);

	return result;
}

/* disassemble a buffer */
static VALUE cls_disasm_dis(VALUE class, VALUE tgt, VALUE hash) {
	struct disassemble_info * info;
	struct disasm_target target;
	unsigned int pos, length; 
	bfd_vma vma;
	VALUE ary;

	Data_Get_Struct(class, struct disassemble_info, info);
	if (! info ) {
		rb_raise( rb_eRuntimeError, "Invalid disassemble_info" );
	}

	disasm_init( info, &target, &vma, class, tgt, hash );

	/* length to disassemble to */
	length = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_LENGTH), Qnil);
	length = (length == Qnil) ? info->buffer_length : NUM2UINT(length);

	/* number of bytes disassembled */
	ary = rb_ary_new();
	for ( pos = 0; pos < length; ) {
		/* yes, pos is modified by disasm_insn. deal. */
		rb_ary_push(ary, disasm_insn( info, vma + pos, &pos ));
	}

	unload_target(&target);

	return ary;
}

/* return an array of supported architectures */
static VALUE cls_disasm_arch(VALUE class) {
	VALUE ary = rb_ary_new();
	get_available_disassemblers( &ary );
	return ary;
}


/* instantiate a new Disassembler object */
static VALUE cls_disasm_new(VALUE class, VALUE hash) {
	struct disassemble_info * info;
	VALUE instance, var;
	VALUE argv[1] = {Qnil};

	/* prepare disassemble_info struct */
	instance = Data_Make_Struct(class, struct disassemble_info, 0, free, 
				    info);
	var = rb_ary_new();
	init_disassemble_info(info, (void *) var, disasm_fprintf );
	rb_obj_call_init(instance, 0, argv);


	/* libopcodes disassembler options string */
	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_OPTS), 
			      rb_str_new_cstr(""));

	/* -- Get Disassembler Function (print-insn-*) */
	/* default to hex dump */
	info->application_data = generic_print_address_wrapper;
	info->disassembler_options = NULL;

	/* configure arch based on BFD, if provided */
	var = rb_hash_lookup(hash, str_to_sym(DIS_ARG_BFD));
	if ( var != Qnil) {
		if ( Qtrue == rb_obj_is_kind_of( var, 
			      path2class("Bfd::Target") ) ) {
			bfd * abfd;
			/* nasty nasty! touching other peoples' privates! */
			Data_Get_Struct(var, bfd, abfd);
			info->application_data = disassembler(abfd);
			info->arch = abfd->arch_info->arch;
			info->mach = abfd->arch_info->mach;
		} else {
			rb_raise(rb_eArgError, "Invalid :bfd argument");
		}
	}

	/* configura architecture manually, if provided */
	var = rb_hash_lookup(hash, str_to_sym(DIS_ARG_ARCH));
	if ( var != Qnil) {
		config_disasm_arch(info, var);
	}

	return instance;
}


static void init_disasm_class( VALUE modOpcodes ) {
	clsDisasm = rb_define_class_under(modOpcodes, DIS_CLASS_NAME,
					  rb_cObject);
	/* class methods */
	rb_define_singleton_method(clsDisasm, "ext_new", cls_disasm_new, 1);
	rb_define_singleton_method(clsDisasm, DIS_METHOD_ARCH, cls_disasm_arch,
				   0);
	
	/* instance attributes */
	rb_define_attr(clsDisasm, DIS_ATTR_OPTIONS, 1, 1);

	/* instance methods */
	rb_define_method(clsDisasm, DIS_FN_DIS_DIS, cls_disasm_dis, 2);
	rb_define_method(clsDisasm, DIS_FN_DIS_INSN, cls_disasm_single, 2);
}

/* ---------------------------------------------------------------------- */
/* Opcodes Module */

void Init_OpcodesExt() {
	modOpcodes = rb_define_module(OPCODES_MODULE_NAME);

	init_disasm_class(modOpcodes);
}
