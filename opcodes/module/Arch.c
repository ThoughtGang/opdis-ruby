/* Arch.c
 * Disassembler definitions for architectures supported by libopcodes
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */
#include <string.h>

#include "Arch.h"

static int generic_print_address_wrapper(bfd_vma vma, disassemble_info *info ) {
	generic_print_address(vma, info);
	return 1;
}

static const Opcodes_disasm_def disasm_definitions[] = {
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
		{"INVALID", bfd_arch_unknown, 0, generic_print_address_wrapper}
};

void Opcodes_disasm_iter( OPCODES_DISASM_ITER_FN fn, void * arg ) {
	int i;
	int num_defs = sizeof(disasm_definitions) / sizeof(Opcodes_disasm_def);

	if (! fn ) {
		return;
	}

	for ( i = 0; i < num_defs; i++ ) {
		const Opcodes_disasm_def *def = &disasm_definitions[i];
		if (! fn( def, arg ) ) {
			break;
		}
	}
}

struct disasm_find_name_arg {
	const char * name;
	const Opcodes_disasm_def * def;
};

static int is_named_def( const Opcodes_disasm_def * def, void * arg ) {
	struct disasm_find_name_arg * out = (struct disasm_find_name_arg *) arg;
	if (! strcmp( out->name, def->name ) ) {
		out->def = def;
		return 0;
	}

	return 1;
}

const Opcodes_disasm_def * Opcodes_disasm_for_name( const char * name ) {
	struct disasm_find_name_arg arg = { name, Opcodes_disasm_invalid() };
	Opcodes_disasm_iter( is_named_def, &arg );
	return arg.def;
}

const Opcodes_disasm_def * Opcodes_disasm_invalid( void ) {
	int num_defs = sizeof(disasm_definitions) / sizeof(Opcodes_disasm_def);
	return &disasm_definitions[num_defs-1];
}

