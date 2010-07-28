/* Opcodes.h
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef OPCODES_RUBY_EXT_H
#define OPCODES_RUBY_EXT_H

/* Disassembler Class */
#define DIS_ATTR_OPTIONS "options"

#define DIS_FN_DIS_INSN "ext_disasm_insn"
#define DIS_FN_DIS_DIS "ext_disasm"

#define DIS_ARG_BFD "bfd"
#define DIS_ARG_ARCH "arch"
#define DIS_ARG_OPTS "opts"
#define DIS_ARG_VMA "vma"
#define DIS_ARG_BUFVMA "buffer_vma"
#define DIS_ARG_LENGTH "length"

/* Instruction info hash */
#define DIS_INSN_INFO_DELAY "branch_delay_insn"
#define DIS_INSN_INFO_DATA_SZ "data_size"
#define DIS_INSN_INFO_TYPE "type"
#define DIS_INSN_INFO_TGT "target"
#define DIS_INSN_INFO_TGT2 "target2"

#define DIS_INSN_VMA "vma"
#define DIS_INSN_SIZE "size"
#define DIS_INSN_INFO "info"
#define DIS_INSN_INSN "insn"

#define DIS_METHOD_ARCH "architectures"

#define DISASM_MAX_STR 64

/* Module and Class names */

#define OPCODES_MODULE_NAME "Opcodes"
#define DIS_CLASS_NAME "Disassembler"
void Init_OpcodesExt();

#endif
