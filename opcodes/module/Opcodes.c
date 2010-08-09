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
/* Disassemblers */
struct disasm_def { 
	const char * name; 
	enum bfd_architecture arch;
	unsigned long default_mach;
	disassembler_ftype fn;
};

static struct disasm_def disasm_definitions[] = {
/* Goddamn GNU. They make it near-impossible to get a list of supported
 * architectures at build OR run time. */
	#ifdef ARCH_ALPHA
		{"alpha", bfd_arch_alpha, bfd_mach_alpha_ev4, print_insn_alpha},
		{"alphaev4", bfd_arch_alpha, bfd_mach_alpha_ev4, 
			print_insn_alpha},
		{"alphaev5", bfd_arch_alpha, bfd_mach_alpha_ev5, 
			print_insn_alpha},
		{"alphaev6", bfd_arch_alpha, bfd_mach_alpha_ev6, 
			print_insn_alpha},
	#endif
	#ifdef ARCH_ARM
		{"big_arm", bfd_arch_arm, bfd_mach_arm_umknown, 
			print_insn_big_arm},
		{"little_arm", bfd_arch_arm, bfd_mach_arm_unknown,
			print_insn_little_arm},
		// TODO: the other ARMs have to be big & little?
	#endif
	#ifdef ARCH_AVR
		{"avr", bfd_arch_avr, bfd_mach_avr1, print_insn_avr},
		{"avr1", bfd_arch_avr, bfd_mach_avr1, print_insn_avr},
		{"avr2", bfd_arch_avr, bfd_mach_avr2, print_insn_avr},
		{"avr25", bfd_arch_avr, bfd_mach_avr25, print_insn_avr},
		{"avr3", bfd_arch_avr, bfd_mach_avr3, print_insn_avr},
		{"avr31", bfd_arch_avr, bfd_mach_avr31, print_insn_avr},
		{"avr35", bfd_arch_avr, bfd_mach_avr35, print_insn_avr},
		{"avr4", bfd_arch_avr, bfd_mach_avr4, print_insn_avr},
		{"avr5", bfd_arch_avr, bfd_mach_avr5, print_insn_avr},
		{"avr51", bfd_arch_avr, bfd_mach_avr51, print_insn_avr},
		{"avr6", bfd_arch_avr, bfd_mach_avr6, print_insn_avr},
	#endif
	#ifdef ARCH_BFIN
		{"bfin", bfd_arch_bfin, bfd_mach_bfin, print_insn_bfin},
	#endif

	#ifdef ARCH_CR16
		{"cr16", bfd_arch_cr16, bfd_mach_cr16, print_insn_cr16},
		{"cr16c", bfd_arch_cr16c, bfd_mach_cr16c, print_insn_cr16},
	#endif
	#ifdef ARCH_CRX
		{"crx", bfd_arch_crx, bfd_mach_crx, print_insn_crx},
	#endif
	#ifdef ARCH_D10V
		{"d10v", bfd_arch_d10v, bfd_mach_d10v, print_insn_d10v},
		{"d10v2", bfd_arch_d10v, bfd_mach_d10v_ts2, print_insn_d10v},
		{"d10v3", bfd_arch_d10v, bfd_mach_d10v_ts3, print_insn_d10v},
	#endif
	#ifdef ARCH_D30V
		{"d30v", bfd_arch_d30v, 0, print_insn_d30v},
	#endif
	#ifdef ARCH_DLX
		{"dlx", bfd_arch_dlx, 0, print_insn_dlx},
	#endif
	#ifdef ARCH_FR30
		{"fr30", bfd_arch_fr30, bfd_mach_fr30, print_insn_fr30},
	#endif
	#ifdef ARCH_FRV
		{"frv", bfd_arch_frv, bfd_mach_frv, print_insn_frv},
		{"frvsimple", bfd_arch_frv, bfd_mach_frvsimple, print_insn_frv},
		{"fr300", bfd_arch_frv, bfd_mach_fr300, print_insn_frv},
		{"fr400", bfd_arch_frv, bfd_mach_fr400, print_insn_frv},
		{"fr450", bfd_arch_frv, bfd_mach_fr450, print_insn_frv},
		{"frvtomcat", bfd_arch_frv, bfd_mach_frvtomcat, print_insn_frv},
		{"fr500", bfd_arch_frv, bfd_mach_fr500, print_insn_frv},
		{"fr550", bfd_arch_frv, bfd_mach_fr550, print_insn_frv},
	#endif
	#ifdef ARCH_H8300
		{"h8300", bfd_arch_8300, bfd_mach_h8300, print_insn_h8300},
		{"h8300h", bfd_arch_8300, bfd_mach_h8300h, print_insn_h8300h},
		{"h8300hn", bfd_arch_8300, bfd_mach_h8300hn, print_insn_h8300h},
		{"h8300s", bfd_arch_8300, bfd_mach_h8300s, print_insn_h8300s},
		{"h8300sn", bfd_arch_8300, bfd_mach_h8300sn, print_insn_h8300s},
		{"h8300sx", bfd_arch_8300, bfd_mach_h8300sx, print_insn_h8300s},
		{"h8300sxn", bfd_arch_8300, bfd_mach_h8300sxn, 
			print_insn_h8300s},
	#endif
	#ifdef ARCH_H8500
		{"h8500", bfd_arch_h8500, 0, print_insn_h8500},
	#endif
	#ifdef ARCH_HPPA
		{"hppa", bfd_arch_hppa, bfd_mach_hppa10, print_insn_hppa},
		{"hppa11", bfd_arch_hppa, bfd_mach_hppa11, print_insn_hppa},
		{"hppa20", bfd_arch_hppa, bfd_mach_hppa20, print_insn_hppa},
		{"hppa20w", bfd_arch_hppa, bfd_mach_hppa20w, print_insn_hppa},
	#endif
	#ifdef ARCH_I370
		{"i370", bfd_arch_i370, 0, print_insn_i370},
	#endif
	#ifdef ARCH_I386
		{"8086", bfd_arch_i386, bfd_mach_i386_i8086, print_insn_i386},
		{"x86", bfd_arch_i386, bfd_mach_i386_i386, print_insn_i386},
		{"x86_att", bfd_arch_i386, bfd_mach_i386_i386, 
			    print_insn_i386_att},
		{"x86_intel", bfd_arch_i386, bfd_mach_i386_i386_intel_syntax, 
			      print_insn_i386_intel},
		{"x86_64", bfd_arch_i386, bfd_mach_x86_64, print_insn_i386},
		{"x86_64_att", bfd_arch_i386, bfd_mach_x86_64, print_insn_i386},
		{"x86_64_intel", bfd_arch_i386, bfd_mach_x86_64_intel_syntax, 
			         print_insn_i386},
	#endif
	#ifdef ARCH_i860
		{"i860", bfd_arch_i860, 0, print_insn_i860},
	#endif
	#ifdef ARCH_i960
		{"i960", bfd_arch_i960, bfd_mach_i960_core, print_insn_i960},
		{"i960_hx", bfd_arch_i960, bfd_mach_i960_hx, print_insn_i960},
	#endif
	#ifdef ARCH_ia64
		{"ia64", bfd_arch_ia64, bfd_mach_ia64_elf64, print_insn_ia64},
		{"ia64_32", bfd_arch_ia64, bfd_mach_ia64_elf32, 
			print_insn_ia64},
	#endif
	#ifdef ARCH_IP2K
		{"ip2k", bfd_arch_ip2k, bfd_mach_ip2022, print_insn_ip2k},
		{"ip2kext", bfd_arch_ip2k, bfd_mach_ip2022ext, print_insn_ip2k},
	#endif
	#ifdef ARCH_IQ2000
		{"iq2000", bfd_arch_iq2000, bfd_mach_iq2000, print_insn_iq2000},
		{"iq10", bfd_arch_iq2000, bfd_mach_iq10, print_insn_iq2000},
	#endif
	#ifdef ARCH_LM32
		{"lm32", bfd_arch_lm32, bfd_mach_lm32, print_insn_lm32},
	#endif
	#ifdef ARCH_M32C
		{"m32c", bfd_arch_m32c, bfd_mach_m32c, print_insn_m32c},
		{"m16c", bfd_arch_m32c, bfd_mach_m16c, print_insn_m32c},
	#endif
	#ifdef ARCH_M32R
		{"m32r", bfd_arch_m32r, bfd_mach_m32r, print_insn_m32r},
		{"m32rx", bfd_arch_m32r, bfd_mach_m32rx, print_insn_m32r},
		{"m32r2", bfd_arch_m32r, bfd_mach_m32r2, print_insn_m32r},
	#endif
	#ifdef ARCH_M68HC11
		{"m68hc11", bfd_arch_m68hc11, 0, print_insn_m68hc11},
	#endif
	#ifdef ARCH_M68HC12
		{"m68hc12", bfd_arch_m68hc12, bfd_mach_m6812_default, 
			    print_insn_m68hc12},
		{"m68hc12s", bfd_arch_m68hc12, bfd_mach_m6812s, 
			    print_insn_m68hc12},
	#endif
	#ifdef ARCH_M68K
		{"m68k", bfd_arch_m68k, bfd_mach_m68000, print_insn_m68k},
		{"m68008", bfd_arch_m68k, bfd_mach_m68008, print_insn_m68k},
		{"m68010", bfd_arch_m68k, bfd_mach_m68010, print_insn_m68k},
		{"m68020", bfd_arch_m68k, bfd_mach_m68020, print_insn_m68k},
		{"m68030", bfd_arch_m68k, bfd_mach_m68030, print_insn_m68k},
		{"m68040", bfd_arch_m68k, bfd_mach_m68040, print_insn_m68k},
		{"m68060", bfd_arch_m68k, bfd_mach_m68060, print_insn_m68k},
		{"m68060", bfd_arch_m68k, bfd_mach_m68060, print_insn_m68k},
		{"m68k_cpu32", bfd_arch_m68k, bfd_mach_cpu32, print_insn_m68k},
		{"m68k_fido", bfd_arch_m68k, bfd_mach_fido, print_insn_m68k},
	#endif
	#ifdef ARCH_M88K
		{"m88k", bfd_arch_m88k, 0, print_insn_m88k},
	#endif
	#ifdef ARCH_MAXQ
		{"maxq_big", bfd_arch_maxq, 0, print_insn_maxq_big},
		{"maxq_little", bfd_arch_maxq, 0, print_insn_maxq_little},
		{"maxq10_big", bfd_arch_maxq, bfd_mach_maxq10, 
			print_insn_maxq_big},
		{"maxq10_little", bfd_arch_maxq, bfd_mach_maxq10, 
			print_insn_maxq_little},
		{"maxq20_big", bfd_arch_maxq, bfd_mach_maxq20, 
			print_insn_maxq_big},
		{"maxq20_little", bfd_arch_maxq, bfd_mach_maxq20, 
			print_insn_maxq_little},
	#endif
	#ifdef ARCH_MCORE
		{"mcore", bfd_arch_mcore, 0, print_insn_mcore},
	#endif
	#ifdef ARCH_MEP
		{"mep", bfd_arch_mep, 0, print_insn_mep},
	#endif
	#ifdef ARCH_MICROBLAZE
		{"microblaze", bfd_arch_microblaze, print_insn_microblaze},
	#endif
	#ifdef ARCH_MIPS
		{"big_mips", bfd_arch_mips, 0, print_insn_big_mips},
		{"little_mips", bfd_arch_mips, 0, print_insn_little_mips},
	#endif
	#ifdef ARCH_MMIX
		{"mmix", bfd_arch_mmix, 0, print_insn_mmix},
	#endif
	#ifdef ARCH_MN10200
		{"mn10200", bfd_arch_mn10200, 0, print_insn_mn10200},
	#endif
	#ifdef ARCH_MN10300
		{"mn10300", bfd_arch_mn10300, bfd_mach_mn10300,
			print_insn_mn10300},
	#endif
	#ifdef ARCH_MOXIE
		{"moxie", bfd_arch_moxie, bfd_mach_moxie, print_insn_moxie},
	#endif
	#ifdef ARCH_MSP430
		{"msp430", bfd_arch_msp430, 0, print_insn_msp430},
	#endif
	#ifdef ARCH_MT
		{"mt", bfd_arch_mt, 0, print_insn_mt},
	#endif
	#ifdef ARCH_NS32K
		{"ns32k", bfd_arch_ns32k, 0, print_insn_ns32k},
	#endif
	#ifdef ARCH_OPENRISC
		{"openrisc", bfd_arch_openrisc, 0, print_insn_openrisc},
	#endif
	#ifdef ARCH_OR32
		{"big_or32", bfd_arch_or32, 0, print_insn_big_or32},
		{"little_or32", bfd_arch_or32, 0, print_insn_little_or32},
	#endif
	#ifdef ARCH_PDP11
		{"pdp11", bfd_arch_pdp11, 0, print_insn_pdp11},
	#endif
	#ifdef ARCH_PJ
		{"pj", bfd_arch_pj, 0, print_insn_pj},
	#endif
	#ifdef ARCH_POWERPC
		{"big_powerpc", bfd_arch_powerpc, 0, print_insn_big_powerpc},
		{"little_powerpc", bfd_arch_powerpc, 0, 
			print_insn_little_powerpc},
		{"big_powerpc32", bfd_arch_powerpc, bfd_mach_ppc, 
			print_insn_big_powerpc},
		{"little_powerpc32", bfd_arch_powerpc, bfd_mach_ppc, 
			print_insn_little_powerpc},
		{"big_powerpc64", bfd_arch_powerpc, bfd_mach_ppc64, 
			print_insn_big_powerpc},
		{"little_powerpc64", bfd_arch_powerpc, bfd_mach_ppc64, 
			print_insn_little_powerpc},
	#endif
	#ifdef ARCH_RS6000
		{"rs6000", bfd_arch_rs6000, bfd_mach_rs6k, print_insn_rs6000},
	#endif
	#ifdef ARCH_S390
		{"s390", bfd_arch_s390, 0, print_insn_s390},
	#endif
	#ifdef ARCH_SCORE
		{"big_score", bfd_arch_score, 0, print_insn_big_score},
		{"little_score", bfd_arch_score, 0, print_insn_little_score},
	#endif
	#ifdef ARCH_SH
		{"sh", bfd_arch_sh, bfd_mach_sh, print_insn_sh},
		{"sh2", bfd_arch_sh, bfd_mach_sh2, print_insn_sh},
		{"sh3", bfd_arch_sh, bfd_mach_sh3, print_insn_sh},
		{"sh4", bfd_arch_sh, bfd_mach_sh4, print_insn_sh},
		{"sh5", bfd_arch_sh, bfd_mach_sh5, print_insn_sh},
		{"sh_dsp", bfd_arch_sh, bfd_mach_sh_dsp, print_insn_sh},
		{"sh64", bfd_arch_sh, bfd_mach_sh, print_insn_sh64},
		{"sh64x_media", bfd_mach_sh, bfd_mach_sh, 
			print_insn_sh64x_media},
	#endif
	#ifdef ARCH_SPARC
		{"sparc", bfd_arch_sparc, bfd_mach_sparc, print_insn_sparc},
		{"sparclite", bfd_arch_sparc, bfd_mach_sparc_sparclite, 
			print_insn_sparc},
		{"sparclitev9", bfd_arch_sparc, bfd_mach_sparc_v9, 
			print_insn_sparc},
	#endif
	#ifdef ARCH_SPU
		{"spu", bfd_arch_spu, bfd_mach_spu, print_insn_spu},
	#endif
	#ifdef ARCH_TIC30
		{"tic30", bfd_arch_tic30, 0, print_insn_tic30},
	#endif
	#ifdef ARCH_TIC4X
		{"tic3x", bfd_arch_tic4x, bfd_mach_tic3x, print_insn_tic4x},
		{"tic4x", bfd_arch_tic4x, bfd_mach_tic4x, print_insn_tic4x},
	#endif
	#ifdef ARCH_TIC54X
		{"tic54x", bfd_arch_tic54x, 0, print_insn_tic54x},
	#endif
	#ifdef ARCH_TIC80
		{"tic80", bfd_arch_tic80, 0, print_insn_tic80},
	#endif
	#ifdef ARCH_V850
		{"v850", bfd_arch_v850, bfd_mach_v850, print_insn_v850},
	#endif
	#ifdef ARCH_VAX
		{"vax", bfd_arch_vax, 0, print_insn_vax},
	#endif
	#ifdef ARCH_W65
		{"w65", bfd_arch_w65, 0, print_insn_w65},
	#endif
	#ifdef ARCH_XC16X
		{"xc16x", bfd_arch_xc16x, bfd_mach_xc16x, print_insn_xc16x},
	#endif
	#ifdef ARCH_XSTORMY16
		{"xstormy16", bfd_arch_xstormy16, bfd_mach_xstormy16, 
			print_insn_xstormy16},
	#endif
	#ifdef ARCH_XTENSA
		{"xtensa", bfd_arch_xtensa, bfd_mach_xtensa, print_insn_xtensa},
	#endif
	#ifdef ARCH_Z80
		{"z80", bfd_arch_z80, bfd_mach_z80full, print_insn_z80},
		{"z80strict", bfd_arch_z80, bfd_mach_z80strict, print_insn_z80},
		{"r800", bfd_arch_z80, bfd_mach_r800, print_insn_z80},
	#endif
	#ifdef ARCH_Z8K
		{"z8001", bfd_arch_z8k, bfd_mach_z8001, print_insn_z8001},
		{"z8002", bfd_arch_z8k, bfd_mach_z8002, print_insn_z8002},
	#endif
		/* NULL entry to ensure table ends up ok */
		{"INVALID", bfd_arch_unknown, 0, print_insn_alpha}
};

/* return disassembler fn for BFD */
static disassembler_ftype fn_for_bfd( bfd * abfd ) {
	int i;
	int num_defs = sizeof(disasm_definitions) / sizeof(struct disasm_def);

	for ( i = 0; i < num_defs; i++ ) {
		struct disasm_def *def = &disasm_definitions[i];
		if ( def->arch == abfd->arch_info->arch &&
		     def->arch == abfd->arch_info->mach ) {
			return def->fn;
		}
	}

	return generic_print_address_wrapper;
}

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
	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_OPTS), rb_str_new2(""));
	rb_iv_set(instance, IVAR(DIS_ATTR_OPTIONS), var );

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
