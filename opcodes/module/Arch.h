/* Arch.h
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef OPCODES_EXT_ARCH_H
#define OPCODES_EXT_ARCH_H

#include <bfd.h>
#include <dis-asm.h>

/* Disassembler definitions for supported architectures */
typedef struct { 
	const char * name;		/* unique name for disassembler */ 
	enum bfd_architecture arch;	/* architecture from bfd.h */
	unsigned long mach;		/* machine from bfd.h or 0 */
	disassembler_ftype fn;		/* print_insn fn from dis-asm.h */
} Opcodes_disasm_def;

/* disassembler iterator callback function. Returns 0 if iteration should
 * halt; 1 otherwise. */
typedef int (*OPCODES_DISASM_ITER_FN) ( const Opcodes_disasm_def *, void * );

/* iterate over all available disassemblers, invoking 'fn' on each */
void Opcodes_disasm_iter( OPCODES_DISASM_ITER_FN fn, void * arg );

/* return the disassembler definition for 'name', or the invalid
 * definition. */
const Opcodes_disasm_def * Opcodes_disasm_for_name( const char * name );

/* return an invalid disassembler definition with safe values for
 * name and fn, and an architecture of bfd_arch_unknown. */
const Opcodes_disasm_def * Opcodes_disasm_invalid( void );

#endif
