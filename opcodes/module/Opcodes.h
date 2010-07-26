/* Opcodes.h
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef OPCODES_RUBY_EXT_H
#define OPCODES_RUBY_EXT_H

#define DIS_ATTR_OPTIONS "options"

#define DIS_FN_DIS_INSN "disasm_insn"
#define DIS_FN_DIS_DIS "disasm"

#define DIS_ARG_BFD "bfd"
#define DIS_ARG_ARCH "arch"
#define DIS_ARG_OPTS "opts"
#define DIS_ARG_VMA "vma"
#define DIS_ARG_BUFVMA "buffer_vma"
#define DIS_ARG_LENGTH "length"

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

void Init_Opcodes();

#endif
