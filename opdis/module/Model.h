/* Model.h
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef OPDIS_RB_MODEL_H
#define OPDIS_RB_MODEL_H

#include <opdis/opdis.h>
#include <ruby.h>

/* Instruction Class */
#define INSN_DECODE_BASIC_NAME "DECODE_BASIC"
#define INSN_DECODE_BASIC "basic"
#define INSN_DECODE_MNEM_NAME "DECODE_MNEMONIC"
#define INSN_DECODE_MNEM "mnemonic"
#define INSN_DECODE_OPS_NAME "DECODE_OPERANDS"
#define INSN_DECODE_OPS "operands"
#define INSN_DECODE_MNEMFLG_NAME "DECODE_MNEMONIC_FLAGS"
#define INSN_DECODE_MNEMFLG "mnemonic flags"
#define INSN_DECODE_OPFLG_NAME "DECODE_OPERAND_FLAGS"
#define INSN_DECODE_OPFLG "operand flags"

#define INSN_ATTR_STATUS "status"
#define INSN_ATTR_OFFSET "offset"
#define INSN_ATTR_VMA "vma"
#define INSN_ATTR_SIZE "size"
#define INSN_ATTR_BYTES "bytes"
#define INSN_ATTR_PREFIXES "prefixes"
#define INSN_ATTR_MNEMONIC "mnemonic"
#define INSN_ATTR_CATEGORY "category"
#define INSN_ATTR_ISA "isa"
#define INSN_ATTR_FLAGS "flags"
#define INSN_ATTR_COMMENT "comment"
#define INSN_ATTR_OPERANDS "operands"
#define INSN_ATTR_TGT "target" // method
#define INSN_ATTR_DEST "dest" // method
#define INSN_ATTR_SRC "src" // method
#define INSN_ATTR_TGT_IDX "tgt_idx" // private
#define INSN_ATTR_DEST_IDX "dest_idx" // private
#define INSN_ATTR_SRC_IDX "src_idx" // private

#define INSN_STATUS_INVALID_NAME "INVALID"
#define INSN_STATUS_INVALID "invalid"
#define INSN_STATUS_BASIC_NAME "BASIC"
#define INSN_STATUS_BASIC "basic"
#define INSN_STATUS_MNEM_NAME "MNEMONIC"
#define INSN_STATUS_MNEM "mnemonic"
#define INSN_STATUS_OPS_NAME "OPERANDS"
#define INSN_STATUS_OPS "operands"
#define INSN_STATUS_MNEM_FLG_NAME "MNEMONIC_FLAGS"
#define INSN_STATUS_MNEM_FLG "mnemonic flags"
#define INSN_STATUS_OP_FLG_NAME "OPERAND_FLAGS"
#define INSN_STATUS_OP_FLG "operand flags"

#define INSN_ISA_GEN_NAME "ISA_GEN"
#define INSN_ISA_GEN "general"
#define INSN_ISA_FPU_NAME "ISA_FPU"
#define INSN_ISA_FPU "fpu"
#define INSN_ISA_GPU_NAME "ISA_GPU"
#define INSN_ISA_GPU "gpu"
#define INSN_ISA_SIMD_NAME "ISA_SIMD"
#define INSN_ISA_SIMD "simd"
#define INSN_ISA_VM_NAME "ISA_VM"
#define INSN_ISA_VM "vm"

#define INSN_CAT_CFLOW_NAME "CAT_CFLOW"
#define INSN_CAT_CFLOW "control-flow"
#define INSN_CAT_STACK_NAME "CAT_STACK"
#define INSN_CAT_STACK "stack"
#define INSN_CAT_LOST_NAME "CAT_LOADSTORE"
#define INSN_CAT_LOST "load/store"
#define INSN_CAT_TEST_NAME "CAT_TEST"
#define INSN_CAT_TEST "test"
#define INSN_CAT_MATH_NAME "CAT_MATH"
#define INSN_CAT_MATH "mathematic"
#define INSN_CAT_BIT_NAME "CAT_BIT"
#define INSN_CAT_BIT "bitwise"
#define INSN_CAT_IO_NAME "CAT_IO"
#define INSN_CAT_IO "i/o"
#define INSN_CAT_TRAP_NAME "CAT_TRAP"
#define INSN_CAT_TRAP "trap"
#define INSN_CAT_PRIV_NAME "CAT_PRIV"
#define INSN_CAT_PRIV "privileged"
#define INSN_CAT_NOP_NAME "CAT_NOP"
#define INSN_CAT_NOP "no-op"

#define INSN_FLAG_CALL_NAME "FLG_CALL"
#define INSN_FLAG_CALL "call"
#define INSN_FLAG_CALLCC_NAME "FLG_CALLCC"
#define INSN_FLAG_CALLCC "conditional call"
#define INSN_FLAG_JMP_NAME "JMP"
#define INSN_FLAG_JMP "jump"
#define INSN_FLAG_JMPCC_NAME "FLG_JMPCC"
#define INSN_FLAG_JMPCC "conditional jump"
#define INSN_FLAG_RET_NAME "FLG_RET"
#define INSN_FLAG_RET "return"
#define INSN_FLAG_PUSH_NAME "FLG_PUSH"
#define INSN_FLAG_PUSH "push"
#define INSN_FLAG_POP_NAME "FLG_POP"
#define INSN_FLAG_POP "pop"
#define INSN_FLAG_FRAME_NAME "FLG_FRAME"
#define INSN_FLAG_FRAME "enter frame"
#define INSN_FLAG_UNFRAME_NAME "FLG_UNFRAME"
#define INSN_FLAG_UNFRAME "exit frame"
#define INSN_FLAG_AND_NAME "FLG_AND"
#define INSN_FLAG_AND "bitwise and"
#define INSN_FLAG_OR_NAME "FLG_OR"
#define INSN_FLAG_OR "bitwise or"
#define INSN_FLAG_XOR_NAME "FLG_XOR"
#define INSN_FLAG_XOR "bitwise xor"
#define INSN_FLAG_NOT_NAME "FLG_NOT"
#define INSN_FLAG_NOT "bitwise not"
#define INSN_FLAG_LSL_NAME "FLG_LSL"
#define INSN_FLAG_LSL "logical shift left"
#define INSN_FLAG_LSR_NAME "FLG_LSR"
#define INSN_FLAG_LSR "logical shift right"
#define INSN_FLAG_ASL_NAME "FLG_ASL"
#define INSN_FLAG_ASL "arithmetic shift left"
#define INSN_FLAG_ASR_NAME "FLG_ASR"
#define INSN_FLAG_ASR "arithmetic shift right"
#define INSN_FLAG_ROL_NAME "FLG_ROL"
#define INSN_FLAG_ROL "rotate left"
#define INSN_FLAG_ROR_NAME "FLG_ROR"
#define INSN_FLAG_ROR "rotate right"
#define INSN_FLAG_RCL_NAME "FLG_RCR"
#define INSN_FLAG_RCL "rotate carry left"
#define INSN_FLAG_RCR_NAME "FLG_RCR"
#define INSN_FLAG_RCR "rotate carry right"
#define INSN_FLAG_OUT_NAME "FLG_OUT"
#define INSN_FLAG_OUT "input from port"
#define INSN_FLAG_IN_NAME "FLG_IN"
#define INSN_FLAG_IN "output to port"

/* Operand Base Class */

#define OP_ATTR_FLAGS "flags"
#define OP_ATTR_DATA_SZ "data_sz"

#define OP_FLAG_R_NAME "FLG_READ"
#define OP_FLAG_R "r"
#define OP_FLAG_W_NAME "FLG_WRITE"
#define OP_FLAG_W "w"
#define OP_FLAG_X_NAME "FLG_EXEC"
#define OP_FLAG_X "x"
#define OP_FLAG_SIGNED_NAME "FLG_SIGNED"
#define OP_FLAG_SIGNED "signed"
#define OP_FLAG_ADDR_NAME "FLG_ADDR"
#define OP_FLAG_ADDR "addr"
#define OP_FLAG_IND_NAME "FLG_INDIRECT"
#define OP_FLAG_IND "indirect_addr"

#define GEN_ATTR_ASCII "ascii"

/* Immediate Class */
#define IMM_ATTR_VAL "value"
#define IMM_ATTR_SIGNED "signed"
#define IMM_ATTR_UNSIGNED "unsigned"
#define IMM_ATTR_VMA "vma"

/* Address Expression Class */

#define ADDR_EXP_ATTR_SHIFT "shift"
#define ADDR_EXP_ATTR_SCALE "scale"
#define ADDR_EXP_ATTR_INDEX "index"
#define ADDR_EXP_ATTR_BASE "base"
#define ADDR_EXP_ATTR_DISP "disp"

#define ADDR_EXP_SHIFT_LSL_NAME "SHIFT_LSL"
#define ADDR_EXP_SHIFT_LSL "lsl"
#define ADDR_EXP_SHIFT_LSR_NAME "SHIFT_LSR"
#define ADDR_EXP_SHIFT_LSR "lsr"
#define ADDR_EXP_SHIFT_ASL_NAME "SHIFT_ASL"
#define ADDR_EXP_SHIFT_ASL "asl"
#define ADDR_EXP_SHIFT_ROR_NAME "SHIFT_ROR"
#define ADDR_EXP_SHIFT_ROR "ror"
#define ADDR_EXP_SHIFT_RRX_NAME "SHIFT_RRX"
#define ADDR_EXP_SHIFT_RRX "rrx"

/* Absolute Address Class */

#define ABS_ADDR_ATTR_SEG "segment"
#define ABS_ADDR_ATTR_OFF "offset"

/* Register Class */

#define REG_ATTR_FLAGS "flags"
#define REG_ATTR_ID "id"
#define REG_ATTR_SIZE "size"

#define REG_FLAG_GEN_NAME "FLG_GEN"
#define REG_FLAG_GEN "general purpose"
#define REG_FLAG_FPU_NAME "FLG_FPU"
#define REG_FLAG_FPU "fpu"
#define REG_FLAG_GPU_NAME "FLG_GPU"
#define REG_FLAG_GPU "gpu"
#define REG_FLAG_SIMD_NAME "FLG_SIMD"
#define REG_FLAG_SIMD "simd"
#define REG_FLAG_TASK_NAME "FLG_TASK"
#define REG_FLAG_TASK "task_mgt"
#define REG_FLAG_MEM_NAME "FLG_MEM"
#define REG_FLAG_MEM "memory_mgt"
#define REG_FLAG_DBG_NAME "FLG_DEBUG"
#define REG_FLAG_DBG "debug"
#define REG_FLAG_PC_NAME "FLG_PC"
#define REG_FLAG_PC "pc"
#define REG_FLAG_CC_NAME "FLG_FLAGS"
#define REG_FLAG_CC "flags"
#define REG_FLAG_STACK_NAME "FLG_STACK"
#define REG_FLAG_STACK "stack"
#define REG_FLAG_FRAME_NAME "FLG_FRAME"
#define REG_FLAG_FRAME "stack_frame"
#define REG_FLAG_SEG_NAME "FLG_SEGMENT"
#define REG_FLAG_SEG "segment"
#define REG_FLAG_Z_NAME "FLG_ZERO"
#define REG_FLAG_Z "zero"
#define REG_FLAG_IN_NAME "FLG_IN"
#define REG_FLAG_IN "args_in"
#define REG_FLAG_OUT_NAME "FLG_OUT"
#define REG_FLAG_OUT "args_out"
#define REG_FLAG_LOCALS_NAME "FLG_LOCALS"
#define REG_FLAG_LOCALS "locals"
#define REG_FLAG_RET_NAME "FLG_RET"
#define REG_FLAG_RET "return"

void Opdis_initModel( VALUE modOpdis );
VALUE Opdis_insnFromC( opdis_insn_t * insn );
int Opdis_insnToC( VALUE insn, opdis_insn_t * c_insn );

#endif
