/* Opcodes.c
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <dis-asm.h>
#include <ruby.h>

#define IVAR(attr) "@" attr

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

static void get_available_disassemblers( VALUE * ary ) {
	int i;
	int num_defs = sizeof(disasm_definitions) / sizeof(struct disasm_def);

	for ( i = 0; i < num_defs; i++ ) {
		struct disasm_def *def = &disasm_definitions[i];
		rb_ary_push( *ary, rb_str_new_cstr(def->name) );
	}
}

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

static VALUE clsDisasm;

#define DIS_ATTR_OPTIONS "options"

// TODO: better names
#define DIS_FN_DIS_INSN "disasm_insn"
#define DIS_FN_DIS_SEC "disasm_section"

#define DIS_ARG_BFD "bfd"
#define DIS_ARG_ARCH "arch"
#define DIS_ARG_OPTS "opts"
#define DIS_ARG_VMA "vma"
#define DIS_ARG_BUFVMA "buffer_vma"

#define DIS_INSN_INFO_DELAY "branch_delay_insn"
#define DIS_INSN_INFO_DATA_SZ "data_size"
#define DIS_INSN_INFO_TYPE "type"
#define DIS_INSN_INFO_TGT "target"
#define DIS_INSN_INFO_TGT2 "target2"

#define DIS_INSN_VMA "vma"
#define DIS_INSN_SIZE "size"
#define DIS_INSN_INFO "info"
#define DIS_INSN_INSN "insn"

#define DISASM_MAX_STR 64
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

static VALUE disasm_insn( struct disassemble_info * info, bfd_vma vma ) {
	disassembler_ftype fn;
	int size;
	VALUE ary;
	VALUE hash = rb_hash_new();

	ary = (VALUE) info->stream;
	rb_ary_clear(ary);

	if ( vma < info->buffer_vma ) {
		/* assume small VMAs are offsets into buffer */
		vma += info->buffer_vma;
	}

	if ( vma >= info->buffer_vma + info->buffer_length) {
		rb_raise(rb_eArgError, "VMA %d exceeds buffer length", 
			 (int) vma);
	}

	fn = (disassembler_ftype) info->application_data;
	info->insn_info_valid = 0;
	size = fn( vma, info );

	rb_hash_aset( hash, str_to_sym(DIS_INSN_VMA), INT2NUM(vma) );
	rb_hash_aset( hash, str_to_sym(DIS_INSN_SIZE), INT2NUM(size) );
	rb_hash_aset( hash, str_to_sym(DIS_INSN_INFO), disasm_insn_info(info) );
	rb_hash_aset( hash, str_to_sym(DIS_INSN_INSN), rb_ary_dup(ary) );

	return hash;
}

static VALUE cls_disasm_dis(VALUE class, VALUE tgt, VALUE hash) {
	struct disassemble_info * info;
	bfd_vma vma;
	VALUE var;
	char * buf = NULL;
	unsigned int buf_len = 0;

	Data_Get_Struct(class, struct disassemble_info, info);

	/* disassembly options: offset/vma to disasm, vma of buffer */
	vma = NUM2INT( rb_hash_lookup2(hash, str_to_sym(DIS_ARG_VMA), 
		       INT2NUM(0)) );
	info->buffer_vma = NUM2INT( rb_hash_lookup2(hash, 
				    str_to_sym(DIS_ARG_BUFVMA), INT2NUM(0)) );
	var = rb_iv_get(class, IVAR(DIS_ATTR_OPTIONS));
	info->disassembler_options = StringValueCStr( var );

	if ( Qtrue == rb_obj_is_kind_of( tgt, rb_cString) ) {
		buf = RSTRING_PTR(tgt);
		buf_len = RSTRING_LEN(tgt);
	} else if ( Qtrue == rb_obj_is_kind_of( tgt, rb_cIO) ) {
		// TODO: read MAX_BYTES from file
		rb_raise(rb_eNotImpError, "File not supported");

	} else if ( Qtrue == rb_obj_is_kind_of( tgt, 
	      				rb_path2class("Bfd::Target") ) ) {
		bfd * abfd;
		Data_Get_Struct(tgt, bfd, abfd);
		// TODO: bfd->iovec
		rb_raise(rb_eNotImpError, "BFD tgt not supported");
	} else if ( Qtrue == rb_obj_is_kind_of( tgt, 
	      				rb_path2class("Bfd::Section") ) ) {
		// TODO: tgt.contents
		rb_raise(rb_eNotImpError, "BFD sec not supported");
	} else {
		rb_raise(rb_eArgError, 
			"Expecting IO, String, Bfd::Target,or Bfd::Section");
	}

	info->buffer = (bfd_byte *) buf;
	info->buffer_length = buf_len;

	return disasm_insn( info, vma );

}

static VALUE cls_disasm_sec(VALUE class, VALUE sec) {
	struct disassemble_info * info;
	VALUE var;

	Data_Get_Struct(class, struct disassemble_info, info);
// TODO: disassemble ( target ) <- takes block or returns all instructions
	if ( Qtrue != rb_obj_is_kind_of( sec, 
		      rb_path2class("Bfd::Section") ) ) {

		rb_raise(rb_eArgError, "Bfd::Section argument required");
	}

	var = rb_iv_get(class, IVAR(DIS_ATTR_OPTIONS));
	info->disassembler_options = StringValueCStr( var );

	// TODO : from 0 to size, disasm
	//return disasm_insn( info, vma );
	//bfd * abfd;
	/* nasty nasty! touching other peoples' privates! */
	//Data_Get_Struct(var, bfd, abfd);
	//info->application_data = disassembler(abfd);
	return Qnil;
}

static VALUE cls_disasm_arch(VALUE class) {
	VALUE ary = rb_ary_new();
	get_available_disassemblers( &ary );
	return ary;
}


static VALUE cls_disasm_new(VALUE class, VALUE hash) {
	struct disassemble_info * info;
	VALUE instance, var;
	VALUE argv[1] = {Qnil};

	instance = Data_Make_Struct(class, struct disassemble_info, 0, free, 
				    info);
	var = rb_ary_new();
	init_disassemble_info(info, (void *) var, disasm_fprintf );
	rb_obj_call_init(instance, 0, argv);


	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_OPTS), rb_str_new("",0));
	rb_iv_set(instance, IVAR(DIS_ATTR_OPTIONS), var );

	/* -- Get Disassembler Function (print-insn-*) */
	/* default to hex dump */
	info->application_data = generic_print_address_wrapper;
	info->disassembler_options = NULL;

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

	var = rb_hash_lookup(hash, str_to_sym(DIS_ARG_ARCH));
	if ( var != Qnil) {
		fprintf(stderr, "GOT ARCH ARG");
		config_disasm_arch(info, var);
	}

	return instance;
}


static void init_disasm_class( VALUE modOpcodes ) {
	clsDisasm = rb_define_class_under(modOpcodes, "Disassembler", 
					  rb_cObject);
	rb_define_singleton_method(clsDisasm, "new", cls_disasm_new, 1);
	rb_define_singleton_method(clsDisasm, "architectures", 
				   cls_disasm_arch, 0);
	
	/* attributes */
	rb_define_attr(clsDisasm, DIS_ATTR_OPTIONS, 1, 1);

	// TODO: make 2nd option 0 by default
	rb_define_method(clsDisasm, DIS_FN_DIS_INSN, cls_disasm_dis, 2);
	rb_define_method(clsDisasm, DIS_FN_DIS_SEC, cls_disasm_sec, 1);
}

/* ---------------------------------------------------------------------- */
/* Opcodes Module */

static VALUE modOpcodes;
void Init_Opcodes() {
	modOpcodes = rb_define_module("Opcodes");

	init_disasm_class(modOpcodes);
}
