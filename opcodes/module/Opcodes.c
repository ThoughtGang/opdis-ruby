/* Opcodes.c
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <dis-asm.h>

#include "Opcodes.h"

#define IVAR(attr) "@" attr

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
/* Disassemblers */
struct disasm_def { 
	const char * name; 
	enum bfd_architecture arch;
	unsigned long default_mach;
	disassembler_ftype fn;
};

static struct disasm_def disasm_definitions[] = {
// TODO: fix conditional compilation, set arch and mach for all entries
#if 0
// Goddamn GNU. They make it impossible to get a list of supported
// architectures at build OR run time.
	#ifdef ARCH_alpha
		{"alpha", print_insn_alpha},
	#endif
	#ifdef ARCH_avr
		{"avr", print_insn_avr},
	#endif
	#ifdef ARCH_bfin
		{"bfin", print_insn_bfin},
	#endif
	#ifdef ARCH_arm
		{"big_arm", print_insn_big_arm},
		{"little_arm", print_insn_little_arm},
	#endif
	#ifdef ARCH_cr16
		{"cr16", print_insn_cr16},
	#endif
	#ifdef ARCH_crx
		{"crx", print_insn_crx},
	#endif
	#ifdef ARCH_d10v
		{"d10v", print_insn_d10v},
	#endif
	#ifdef ARCH_d30v
		{"d30v", print_insn_d30v},
	#endif
	#ifdef ARCH_dlx
		{"dlx", print_insn_dlx},
	#endif
	#ifdef ARCH_fr30
		{"fr30", print_insn_fr30},
	#endif
	#ifdef ARCH_frv
		{"frv", print_insn_frv},
	#endif
	#ifdef ARCH_h8300
		{"h8300", print_insn_h8300},
		{"h8300h", print_insn_h8300h},
		{"h8300s", print_insn_h8300s},
	#endif
	#ifdef ARCH_h8500
		{"h8500", print_insn_h8500},
	#endif
	#ifdef ARCH_hppa
		{"hppa", print_insn_hppa},
	#endif
	#ifdef ARCH_i370
		{"i370", print_insn_i370},
	#endif
#endif

		{"8086", bfd_arch_i386, bfd_mach_i386_i8086, print_insn_i386},
		{"x86", bfd_arch_i386, bfd_mach_i386_i386, print_insn_i386},
		{"x86_att", bfd_arch_i386, bfd_mach_i386_i386, 
			    print_insn_i386_att},
		{"x86_intel", bfd_arch_i386, bfd_mach_i386_i386_intel_syntax, 
			      print_insn_i386_intel},
		{"x86_64", bfd_arch_i386, bfd_mach_x86_64, print_insn_i386},
		{"x86_64_att", bfd_arch_i386, bfd_mach_x86_64, print_insn_i386},
		{"x86_64_intel", bfd_arch_i386, bfd_mach_x86_64_intel_syntax, 
			         print_insn_i386}
#if 0
// Ditto.
	#ifdef ARCH_i386
		{"i386", print_insn_i386},
		{"i386_att", print_insn_i386_att},
		{"i386_intel", print_insn_i386_intel},
	#endif
	#ifdef ARCH_i860
		{"i860", print_insn_i860},
	#endif
	#ifdef ARCH_i960
		{"i960", print_insn_i960},
	#endif
	#ifdef ARCH_ia64
		{"ia64", print_insn_ia64},
	#endif
	#ifdef ARCH_ip2k
		{"ip2k", print_insn_ip2k},
	#endif
	#ifdef ARCH_iq2000
		{"iq2000", print_insn_iq2000},
	#endif
	#ifdef ARCH_lm32
		{"lm32", print_insn_lm32},
	#endif
	#ifdef ARCH_m32c
		{"m32c", print_insn_m32c},
	#endif
	#ifdef ARCH_m32r
		{"m32r", print_insn_m32r},
	#endif
	#ifdef ARCH_m68hc11
		{"m68hc11", print_insn_m68hc11},
	#endif
	#ifdef ARCH_m68hc12
		{"m68hc12", print_insn_m68hc12},
	#endif
	#ifdef ARCH_m68k
		{"m68k", print_insn_m68k},
	#endif
	#ifdef ARCH_m88k
		{"m88k", print_insn_m88k},
	#endif
	#ifdef ARCH_maxq
		{"maxq_big", print_insn_maxq_big},
		{"maxq_little", print_insn_maxq_little},
	#endif
	#ifdef ARCH_mcore
		{"mcore", print_insn_mcore},
	#endif
	#ifdef ARCH_mep
		{"mep", print_insn_mep},
	#endif
	#ifdef ARCH_microblaze
		{"microblaze", print_insn_microblaze},
	#endif
	#ifdef ARCH_mips
		{"big_mips", print_insn_big_mips},
		{"little_mips", print_insn_little_mips},
	#endif
	#ifdef ARCH_mmix
		{"mmix", print_insn_mmix},
	#endif
	#ifdef ARCH_mn10200
		{"mn10200", print_insn_mn10200},
	#endif
	#ifdef ARCH_mn10300
		{"mn10300", print_insn_mn10300},
	#endif
	#ifdef ARCH_moxie
		{"moxie", print_insn_moxie},
	#endif
	#ifdef ARCH_msp430
		{"msp430", print_insn_msp430},
	#endif
	#ifdef ARCH_mt
		{"mt", print_insn_mt},
	#endif
	#ifdef ARCH_ns32k
		{"ns32k", print_insn_ns32k},
	#endif
	#ifdef ARCH_openrisc
		{"openrisc", print_insn_openrisc},
	#endif
	#ifdef ARCH_or32
		{"big_or32", print_insn_big_or32},
		{"little_or32", print_insn_little_or32},
	#endif
	#ifdef ARCH_pdp11
		{"pdp11", print_insn_pdp11},
	#endif
	#ifdef ARCH_pj
		{"pj", print_insn_pj},
	#endif
	#ifdef ARCH_powerpc
		{"big_powerpc", print_insn_big_powerpc},
		{"little_powerpc", print_insn_little_powerpc},
	#endif
	#ifdef ARCH_rs6000
		{"rs6000", print_insn_rs6000},
	#endif
	#ifdef ARCH_s390
		{"s390", print_insn_s390},
	#endif
	#ifdef ARCH_score
		{"big_score", print_insn_big_score},
		{"little_score", print_insn_little_score},
	#endif
	#ifdef ARCH_sh
		{"sh", print_insn_sh},
		{"sh64", print_insn_sh64},
		{"sh64x_media", print_insn_sh64x_media},
	#endif
	#ifdef ARCH_sparc
		{"sparc", print_insn_sparc},
	#endif
	#ifdef ARCH_spu
		{"spu", print_insn_spu},
	#endif
	#ifdef ARCH_tic30
		{"tic30", print_insn_tic30},
	#endif
	#ifdef ARCH_tic4x
		{"tic4x", print_insn_tic4x},
	#endif
	#ifdef ARCH_tic54x
		{"tic54x", print_insn_tic54x},
	#endif
	#ifdef ARCH_tic80
		{"tic80", print_insn_tic80},
	#endif
	#ifdef ARCH_v850
		{"v850", print_insn_v850},
	#endif
	#ifdef ARCH_vax
		{"vax", print_insn_vax},
	#endif
	#ifdef ARCH_w65
		{"w65", print_insn_w65},
	#endif
	#ifdef ARCH_xc16x
		{"xc16x", print_insn_xc16x},
	#endif
	#ifdef ARCH_xstormy16
		{"xstormy16", print_insn_xstormy16},
	#endif
	#ifdef ARCH_xtensa
		{"xtensa", print_insn_xtensa},
	#endif
	#ifdef ARCH_z80
		{"z80", print_insn_z80},
	#endif
	#ifdef ARCH_z8k
		{"z8001", print_insn_z8001},
		{"z8002", print_insn_z8002}
	#endif
#endif
};

/* configure disassemble_info for specified architecture */
static void config_disasm_arch( struct disassemble_info * info, VALUE str ) {
	int i;
	int num_defs = sizeof(disasm_definitions) / sizeof(struct disasm_def);
	const char * name = rb_string_value_cstr(&str);

	info->application_data = generic_print_address_wrapper;
	info->arch = bfd_arch_unknown;
	info->mach = 0;

	for ( i = 0; i < num_defs; i++ ) {
		struct disasm_def *def = &disasm_definitions[i];
		if (! strcmp(name, def->name) ) {
			info->application_data = def->fn;
			info->arch = def->arch;
			info->mach = def->default_mach;
		}
	}

}

/* fill array with available disassemblers */
static void get_available_disassemblers( VALUE * ary ) {
	int i;
	int num_defs = sizeof(disasm_definitions) / sizeof(struct disasm_def);

	for ( i = 0; i < num_defs; i++ ) {
		struct disasm_def *def = &disasm_definitions[i];
		rb_ary_push( *ary, rb_str_new_cstr(def->name) );
	}
}

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

/* disassemble a single instruction at address, returning a ruby hash.
 * NOTE: this assumes disassemble_info.application_data is used to
 *       keep track of the number of bytes processed so far. */
static VALUE disasm_insn( struct disassemble_info * info, bfd_vma vma ) {
	disassembler_ftype fn;
	int size;
	VALUE ary = rb_ary_new();;
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
	info->application_data += size;

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
};

/* fill disassemble_info struct based on contents of target struct */
static void config_libopcodes_for_target( struct disassemble_info * info, 
					  struct disasm_target * tgt ) {
	info->buffer_vma = info->buffer_length = 0;
	info->buffer = NULL;

	if ( tgt->buf ) {
		info->buffer_length = tgt->buf_len;
		info->buffer = tgt->buf;
	} else if ( tgt->sec ) {
		info->buffer_vma = tgt->sec->vma;
		info->buffer_length = tgt->sec->size;
		info->buffer = tgt->sec->contents;
	} else if ( tgt->sym ) {
		rb_raise(rb_eNotImpError, "BFD sym not supported");
	} else if ( tgt->abfd ) {
		info->buffer_length = tgt->buf_len;
		info->buffer = tgt->buf;
		rb_raise(rb_eNotImpError, "BFD tgt not supported");
	}
}

/* fill target struct based on ruby inout value */
static void load_target( VALUE tgt, struct disasm_target * dest ) {

	memset( dest, 0, sizeof(struct disasm_target) );

	if ( Qtrue == rb_obj_is_kind_of( tgt, rb_cString) ) {
		/* string of bytes */
		dest->buf = (unsigned char *) RSTRING_PTR(tgt);
		dest->buf_len = RSTRING_LEN(tgt);

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

	} else if ( Qtrue == rb_obj_is_kind_of( tgt, 
	      				rb_path2class("Bfd::Target") ) ) {
		/* BFD Target */
		Data_Get_Struct(tgt, bfd, dest->abfd);

	} else if ( Qtrue == rb_obj_is_kind_of( tgt, 
	      				rb_path2class("Bfd::Section") ) ) {
		/* BFD Section */
		Data_Get_Struct(tgt, asection, dest->sec);
		if ( dest->sec ) {
			dest->abfd = dest->sec->owner;
		}
		/* force loading of section contents */
		rb_funcall( tgt, rb_intern("contents"), 0 );

	} else if ( Qtrue == rb_obj_is_kind_of( tgt, 
	      				rb_path2class("Bfd::Symbol") ) ) {
		/* BFD Symbol */
		Data_Get_Struct(tgt, asymbol, dest->sym);
		if ( dest->sym ) {
			dest->abfd = dest->sym->the_bfd;
		}

	} else {
		rb_raise(rb_eArgError, 
			"Expecting IO, String, Bfd::Target,or Bfd::Section");
	}
}

/* free any memory allocated when loading target */
static void unload_target( struct disasm_target * tgt ) {
	if ( tgt->buf ) {
		free(tgt->buf);
	}
}

/* shared code for loading a target, configuring libopcodes, and getting
 * options */
static void disasm_init( struct disassemble_info * info, 
			 struct disasm_target * target, bfd_vma * vma, 
			 VALUE class, VALUE tgt, VALUE hash ) {
	bfd_vma vma_arg;
	VALUE var;

	load_target( tgt, target );
	config_libopcodes_for_target( info, target );

	/* override vma if caller requested it */
	vma_arg = NUM2INT( rb_hash_lookup2(hash, 
		           str_to_sym(DIS_ARG_BUFVMA), Qnil) );
	if ( vma_arg != Qnil ) {
		info->buffer_vma = vma_arg;
	}
	
	/* disassembly options: offset/vma to disasm, vma of buffer */
	/* vma to disassemble */
	vma_arg = NUM2INT( rb_hash_lookup2(hash, str_to_sym(DIS_ARG_VMA), 
		        Qnil) );
	*vma = (vma_arg == Qnil) ? info->buffer_vma : vma_arg;

	/* libopcodes disassembler options */
	var = rb_iv_get(class, IVAR(DIS_ATTR_OPTIONS));
	info->disassembler_options = StringValueCStr( var );
}

/* disassemble a single instruction */
static VALUE cls_disasm_single(VALUE class, VALUE tgt, VALUE hash) {
	struct disassemble_info * info;
	struct disasm_target target;
	bfd_vma vma;
	VALUE result;

	Data_Get_Struct(class, struct disassemble_info, info);

	disasm_init( info, &target, &vma, class, tgt, hash );

	result = disasm_insn( info, vma );

	unload_target(&target);

	return result;
}

/* disassemble a buffer */
static VALUE cls_disasm_dis(VALUE class, VALUE tgt, VALUE hash) {
	struct disassemble_info * info;
	struct disasm_target target;
	unsigned int length; 
	bfd_vma vma;
	VALUE ary;

	Data_Get_Struct(class, struct disassemble_info, info);

	disasm_init( info, &target, &vma, class, tgt, hash );


	/* length to disassemble to */
	length = NUM2UINT( rb_hash_lookup2(hash, 
			   str_to_sym(DIS_ARG_LENGTH), Qnil) );
	length = (length == Qnil) ? info->buffer_length : length;

	/* number of bytes disassembled */
	ary = rb_ary_new();
	for ( info->application_data = 0; 
	      info->application_data < (void *) length; ) {
		rb_ary_push(ary, disasm_insn( info, vma ));
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
	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_OPTS), rb_str_new("",0));
	rb_iv_set(instance, IVAR(DIS_ATTR_OPTIONS), var );

	/* -- Get Disassembler Function (print-insn-*) */
	/* default to hex dump */
	info->application_data = generic_print_address_wrapper;
	info->disassembler_options = NULL;

	/* configure arch based on BFD, if provided */
	var = rb_hash_lookup(hash, str_to_sym(DIS_ARG_BFD));
	if ( var != Qnil) {
		if ( Qtrue == rb_obj_is_kind_of( var, 
			      rb_path2class("Bfd::Target") ) ) {
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
	clsDisasm = rb_define_class_under(modOpcodes, "Disassembler", 
					  rb_cObject);
	/* class methods */
	rb_define_singleton_method(clsDisasm, "new", cls_disasm_new, 1);
	rb_define_singleton_method(clsDisasm, "architectures", 
				   cls_disasm_arch, 0);
	
	/* instance attributes */
	rb_define_attr(clsDisasm, DIS_ATTR_OPTIONS, 1, 1);

	/* instance methods */
	rb_define_method(clsDisasm, DIS_FN_DIS_DIS, cls_disasm_dis, 2);
	rb_define_method(clsDisasm, DIS_FN_DIS_INSN, cls_disasm_single, 2);
}

/* ---------------------------------------------------------------------- */
/* Opcodes Module */

void Init_Opcodes() {
	modOpcodes = rb_define_module("Opcodes");

	init_disasm_class(modOpcodes);
}
