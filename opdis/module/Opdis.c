/* opdis.c
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

/* Usage:
 * 	require 'Opdis'
 * 	t = Bfd.new( filename )
 * 	o = Opdis.new()
 * 	o.disassemble( t, strategy: o.STRAT_ENTRY ).each do |i|
 * 		...
 * 	end
 * 	insns = o.disassemble( t, strategy: o.STRAT_CFLOW, start: 0 )
 * 	insns = o.disassemble( t, start: 0, len : 100 )
 */
#include <ruby.h>

#include <opdis.h>


#define IVAR(attr) "@" attr
#define SETTER(attr) attr "="

static VALUE str_to_sym( const char * str ) {
	VALUE var = rb_str_new_cstr(str);
	return rb_funcall(var, rb_intern("to_sym"), 0);
}

/* BFD Support (require BFD gem) */
static VALUE clsBfd = Qnil;
static VALUE clsBfdSec = Qnil;
static VALUE clsBfdSym = Qnil;
#define BFD_TGT_PATH "Bfd::Target"
#define BFD_SEC_PATH "Bfd::Section"
#define BFD_SYM_PATH "Bfd::Symbol"
#define GET_BFD_CLASS(cls,name) (cls = cls == Qnil ? rb_path2class(name) : cls);

/* ---------------------------------------------------------------------- */
/* Supported architectures */

/* ---------------------------------------------------------------------- */
/* Register Class */

static VALUE clsReg;

#define REG_ATTR_ASCII "ascii"
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

static void define_reg_constants() {
	rb_define_const(clsREG, REG_FLAG_GEN_NAME,
			rb_str_new_cstr(REG_FLAG_GEN));
	rb_define_const(clsREG, REG_FLAG_FPU_NAME,
			rb_str_new_cstr(REG_FLAG_FPU));
	rb_define_const(clsREG, REG_FLAG_GPU_NAME,
			rb_str_new_cstr(REG_FLAG_GPU));
	rb_define_const(clsREG, REG_FLAG_SIMD_NAME,
			rb_str_new_cstr(REG_FLAG_SIMD));
	rb_define_const(clsREG, REG_FLAG_TASK_NAME,
			rb_str_new_cstr(REG_FLAG_TASK));
	rb_define_const(clsREG, REG_FLAG_MEM_NAME,
			rb_str_new_cstr(REG_FLAG_MEM));
	rb_define_const(clsREG, REG_FLAG_DBG_NAME,
			rb_str_new_cstr(REG_FLAG_DBG));
	rb_define_const(clsREG, REG_FLAG_PC_NAME,
			rb_str_new_cstr(REG_FLAG_PC));
	rb_define_const(clsREG, REG_FLAG_CC_NAME,
			rb_str_new_cstr(REG_FLAG_CC));
	rb_define_const(clsREG, REG_FLAG_STACK_NAME,
			rb_str_new_cstr(REG_FLAG_STACK));
	rb_define_const(clsREG, REG_FLAG_FRAME_NAME,
			rb_str_new_cstr(REG_FLAG_FRAME));
	rb_define_const(clsREG, REG_FLAG_SEG_NAME,
			rb_str_new_cstr(REG_FLAG_SEG));
	rb_define_const(clsREG, REG_FLAG_Z_NAME,
			rb_str_new_cstr(REG_FLAG_Z));
	rb_define_const(clsREG, REG_FLAG_IN_NAME,
			rb_str_new_cstr(REG_FLAG_IN));
	rb_define_const(clsREG, REG_FLAG_OUT_NAME,
			rb_str_new_cstr(REG_FLAG_OUT));
	rb_define_const(clsREG, REG_FLAG_LOCALS_NAME,
			rb_str_new_cstr(REG_FLAG_LOCALS));
	rb_define_const(clsREG, REG_FLAG_RET_NAME,
			rb_str_new_cstr(REG_FLAG_RET));
}

static VALUE cls_reg_init(VALUE instance, VALUE hash) {
	// TODO
	/* set instance variables */
	//rb_iv_set(instance, IVAR(SYM_ATTR_NAME), rb_str_new_cstr(info.name) );
	return instance;
}

static void fill_ruby_reg( opdis_reg_t * reg, VALUE dest ) {
	// TODO
}

static VALUE reg_from_c( opdis_reg_t * reg ) {
	VALUE var = rb_class_new(clsOp);
	fill_ruby_reg(reg, var);
	return var;
}

static void reg_to_c( VALUE reg, opdis_reg_t * dest ) {
	// TODO
}

static void init_reg_class( VALUE modOpdis ) {
	clsReg = rb_define_class_under(modOpdis, "Register", rb_cObject);

	rb_define_method(clsReg, "initialize", cls_reg_init, 1);

	/* read-write attributes */
	rb_define_attr(clsDisasm, DIS_ATTR_ASCII, 1, 1);
	rb_define_attr(clsDisasm, DIS_ATTR_FLAGS, 1, 1);
	rb_define_attr(clsDisasm, DIS_ATTR_ID, 1, 1);
	rb_define_attr(clsDisasm, DIS_ATTR_SIZE, 1, 1);

	define_reg_constants();
}

/* ---------------------------------------------------------------------- */
/* Absolute Address Operand Class */

static VALUE clsAbsAddr;

#define ABS_ADDR_ATTR_SEG "segment"
#define ABS_ADDR_ATTR_OFF "offset"

static VALUE cls_absaddr_init(VALUE instance, VALUE hash) {
	// TODO
	/* set instance variables */
	//rb_iv_set(instance, IVAR(SYM_ATTR_NAME), rb_str_new_cstr(info.name) );
	return instance;
}

static void fill_ruby_absaddr( opdis_abs_addr_t * addr, VALUE dest ) {
	// TODO
}

static VALUE absaddr_from_c( opdis_abs_addr_t * addr ) {
	VALUE var = rb_class_new(clsAbsAddr);
	fill_ruby_absaddr(addr, var);
	return var;
}

static void absaddr_to_c( VALUE addr, opdis_abs_addr_t * dest ) {
	// TODO
}

static void init_absaddr_class( VALUE modOpdis ) {
	clsAbsAddr = rb_define_class_under(modOpdis, "AbsoluteAddress", 
					   rb_cObject);
	rb_define_method(clsAbsAddr, "initialize", cls_absaddr_init, 1);
	
	// TODO

	/* read-only attributes */
	//rb_define_attr(clsDisasm, DIS_ATTR_, 1, 0);

	/* setters */
	//rb_define_method(clsDisasm, DIS_ATTR_, cls_disasm_, 1);
}
 
/* ---------------------------------------------------------------------- */
/* Address Expression Class */

static VALUE clsAddrExp;

#define ADDR_EXP_ATTR_SHIFT "shift"
#define ADDR_EXP_ATTR_SCALE "scale"
#define ADDR_EXP_ATTR_INDEX "index"
#define ADDR_EXP_ATTR_BASE "base"
#define ADDR_EXP_ATTR_DISP "disp"

static VALUE cls_addrexpr_init(VALUE instance, VALUE hash) {
	// TODO
	/* set instance variables */
	//rb_iv_set(instance, IVAR(SYM_ATTR_NAME), rb_str_new_cstr(info.name) );
	return instance;
}

static void fill_ruby_addrexpr( opdis_addr_expr_t * expr, VALUE dest ) {
	// TODO
}

static VALUE addrexpr_from_c( opdis_addr_expr_t * expr ) {
	VALUE var = rb_class_new(clsAddrExp);
	fill_ruby_addrexpr(expr, var);
	return var;
}

static void addrexpr_to_c( VALUE expr, opdis_addr_expr_t * dest ) {
	// TODO
}

static void init_addrexp_class( VALUE modOpdis ) {
	clsAddrExp = rb_define_class_under(modOpdis, "AddressExpression", 
					   rb_cObject);
	rb_singleton_method(clsAddrExp, "initialize", cls_addrexpr_init, 1);

	// TODO
	/* read-only attributes */
	//rb_define_attr(clsDisasm, DIS_ATTR_, 1, 0);

	/* setters */
	//rb_define_method(clsDisasm, DIS_ATTR_, cls_disasm_, 1);
}

/* ---------------------------------------------------------------------- */
/* Operand Class */

static VALUE clsOp;

#define OP_ATTR_ASCII "ascii"
#define OP_ATTR_FLAGS "flags"
#define OP_ATTR_VALUE "value"
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

static VALUE cls_op_init(VALUE instance, VALUE hash) {
	// TODO
	/* set instance variables */
	//rb_iv_set(instance, IVAR(SYM_ATTR_NAME), rb_str_new_cstr(info.name) );
	return instance;
}

static void fill_ruby_op( opdis_op_t * op, VALUE dest ) {
	// TODO
}

static VALUE op_from_c( opdis_op_t * op ) {
	VALUE var = rb_class_new(clsOp);
	fill_ruby_op(op, var);
	return var;
}

static void op_to_c( VALUE op, opdis_op_t * dest ) {
	// TODO
}

static void define_op_constants() {
	rb_define_const(clsOp, OP_FLAG_R_NAME,
			rb_str_new_cstr(OP_FLAG_R));
	rb_define_const(clsOp, OP_FLAG_W_NAME,
			rb_str_new_cstr(OP_FLAG_W));
	rb_define_const(clsOp, OP_FLAG_X_NAME,
			rb_str_new_cstr(OP_FLAG_X));
	rb_define_const(clsOp, OP_FLAG_SIGNED_NAME,
			rb_str_new_cstr(OP_FLAG_SIGNED));
	rb_define_const(clsOp, OP_FLAG_ADDR_NAME,
			rb_str_new_cstr(OP_FLAG_ADDR));
	rb_define_const(clsOp, OP_FLAG_IND_NAME,
			rb_str_new_cstr(OP_FLAG_IND));
}

static void init_op_class( VALUE modOpdis ) {
	clsOp = rb_define_class_under(modOpdis, "Operand", rb_cObject);
	rb_define_method(clsOp, "initialize", cls_op_init, 1);

	// TODO

	/* read-only attributes */
	//rb_define_attr(clsDisasm, DIS_ATTR_, 1, 0);

	/* setters */
	//rb_define_method(clsDisasm, DIS_ATTR_, cls_disasm_, 1);
}

/* ---------------------------------------------------------------------- */
/* Instruction Class */

static VALUE clsInsn;

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
#define INSN_FLAG_CALLCC "callcc"
#define INSN_FLAG_JMP_NAME "JMP"
#define INSN_FLAG_JMP "jmp"
#define INSN_FLAG_JMPCC_NAME "FLG_JMPCC"
#define INSN_FLAG_JMPCC "jmpcc"
#define INSN_FLAG_RET_NAME "FLG_RET"
#define INSN_FLAG_RET "ret"
#define INSN_FLAG_PUSH_NAME "FLG_PUSH"
#define INSN_FLAG_PUSH "push"
#define INSN_FLAG_POP_NAME "FLG_POP"
#define INSN_FLAG_POP "pop"
#define INSN_FLAG_FRAME_NAME "FLG_FRAME"
#define INSN_FLAG_FRAME "frame"
#define INSN_FLAG_UNFRAME_NAME "FLG_UNFRAME"
#define INSN_FLAG_UNFRAME "unframe"
#define INSN_FLAG_AND_NAME "FLG_AND"
#define INSN_FLAG_AND "and"
#define INSN_FLAG_OR_NAME "FLG_OR"
#define INSN_FLAG_OR "or"
#define INSN_FLAG_XOR_NAME "FLG_XOR"
#define INSN_FLAG_XOR "xor"
#define INSN_FLAG_NOT_NAME "FLG_NOT"
#define INSN_FLAG_NOT "not"
#define INSN_FLAG_LSL_NAME "FLG_LSL"
#define INSN_FLAG_LSL "lsl"
#define INSN_FLAG_LSR_NAME "FLG_LSR"
#define INSN_FLAG_LSR "lsr"
#define INSN_FLAG_ASL_NAME "FLG_ASL"
#define INSN_FLAG_ASL "asl"
#define INSN_FLAG_ASR_NAME "FLG_ASR"
#define INSN_FLAG_ASR "asr"
#define INSN_FLAG_ROL_NAME "FLG_ROL"
#define INSN_FLAG_ROL "rol"
#define INSN_FLAG_ROR_NAME "FLG_ROR"
#define INSN_FLAG_ROR "ror"
#define INSN_FLAG_RCL_NAME "FLG_RCR"
#define INSN_FLAG_RCL "rcl"
#define INSN_FLAG_RCR_NAME "FLG_RCR"
#define INSN_FLAG_RCR "rcr"
#define INSN_FLAG_OUT_NAME "FLG_OUT"
#define INSN_FLAG_OUT "in"
#define INSN_FLAG_IN_NAME "FLG_IN"
#define INSN_FLAG_IN "out"

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

static VALUE cls_insn_init(VALUE instance, VALUE hash) {
	// TODO
	/* set instance variables */
	//rb_iv_set(instance, IVAR(SYM_ATTR_NAME), rb_str_new_cstr(info.name) );
	return instance;
}

static void fill_ruby_insn( opdis_insn_t * insn, VALUE dest ) {
	// TODO
}

static VALUE insn_from_c( opdis_insn_t * insn ) {
	VALUE var = rb_class_new(clsInsn);
	fill_ruby_insn( insn, var );
	return var;
}

static void insn_to_c( VALUE insn, opdis_insn_t * dest ) {
	// TODO
}

static void define_op_constants() {
	rb_define_const(clsInsn, INSN_ISA_GEN_NAME,
			rb_str_new_cstr(INSN_ISA_GEN));
	rb_define_const(clsInsn, INSN_ISA_FPU_NAME,
			rb_str_new_cstr(INSN_ISA_FPU));
	rb_define_const(clsInsn, INSN_ISA_GPU_NAME,
			rb_str_new_cstr(INSN_ISA_GPU));
	rb_define_const(clsInsn, INSN_ISA_SIMD_NAME,
			rb_str_new_cstr(INSN_ISA_SIMD));
	rb_define_const(clsInsn, INSN_ISA_VM_NAME,
			rb_str_new_cstr(INSN_ISA_VM));

	rb_define_const(clsInsn, INSN_CAT_CFLOW_NAME,
			rb_str_new_cstr(INSN_CAT_CFLOW));
	rb_define_const(clsInsn, INSN_CAT_STACK_NAME,
			rb_str_new_cstr(INSN_CAT_STACK));
	rb_define_const(clsInsn, INSN_CAT_LOST_NAME,
			rb_str_new_cstr(INSN_CAT_LOST));
	rb_define_const(clsInsn, INSN_CAT_TEST_NAME,
			rb_str_new_cstr(INSN_CAT_TEST));
	rb_define_const(clsInsn, INSN_CAT_MATH_NAME,
			rb_str_new_cstr(INSN_CAT_MATH));
	rb_define_const(clsInsn, INSN_CAT_BIT_NAME,
			rb_str_new_cstr(INSN_CAT_BIT));
	rb_define_const(clsInsn, INSN_CAT_IO_NAME,
			rb_str_new_cstr(INSN_CAT_IO));
	rb_define_const(clsInsn, INSN_CAT_TRAP_NAME,
			rb_str_new_cstr(INSN_CAT_TRAP));
	rb_define_const(clsInsn, INSN_CAT_PRIV_NAME,
			rb_str_new_cstr(INSN_CAT_PRIV));
	rb_define_const(clsInsn, INSN_CAT_NOP_NAME,
			rb_str_new_cstr(INSN_CAT_NOP));

	rb_define_const(clsInsn, INSN_FLAG_CALL_NAME,
			rb_str_new_cstr(INSN_FLAG_CALL));
	rb_define_const(clsInsn, INSN_FLAG_CALLCC_NAME,
			rb_str_new_cstr(INSN_FLAG_CALLCC));
	rb_define_const(clsInsn, INSN_FLAG_JMP_NAME,
			rb_str_new_cstr(INSN_FLAG_JMP));
	rb_define_const(clsInsn, INSN_FLAG_JMPCC_NAME,
			rb_str_new_cstr(INSN_FLAG_JMPCC));
	rb_define_const(clsInsn, INSN_FLAG_RET_NAME,
			rb_str_new_cstr(INSN_FLAG_RET));
	rb_define_const(clsInsn, INSN_FLAG_PUSH_NAME,
			rb_str_new_cstr(INSN_FLAG_PUSH));
	rb_define_const(clsInsn, INSN_FLAG_POP_NAME,
			rb_str_new_cstr(INSN_FLAG_POP));
	rb_define_const(clsInsn, INSN_FLAG_FRAME_NAME,
			rb_str_new_cstr(INSN_FLAG_FRAME));
	rb_define_const(clsInsn, INSN_FLAG_UNFRAME_NAME,
			rb_str_new_cstr(INSN_FLAG_UNFRAME));
	rb_define_const(clsInsn, INSN_FLAG_AND_NAME,
			rb_str_new_cstr(INSN_FLAG_AND));
	rb_define_const(clsInsn, INSN_FLAG_OR_NAME,
			rb_str_new_cstr(INSN_FLAG_OR));
	rb_define_const(clsInsn, INSN_FLAG_XOR_NAME,
			rb_str_new_cstr(INSN_FLAG_XOR));
	rb_define_const(clsInsn, INSN_FLAG_NOT_NAME,
			rb_str_new_cstr(INSN_FLAG_NOT));
	rb_define_const(clsInsn, INSN_FLAG_LSL_NAME,
			rb_str_new_cstr(INSN_FLAG_LSL));
	rb_define_const(clsInsn, INSN_FLAG_LSR_NAME,
			rb_str_new_cstr(INSN_FLAG_LSR));
	rb_define_const(clsInsn, INSN_FLAG_ASL_NAME,
			rb_str_new_cstr(INSN_FLAG_ASL));
	rb_define_const(clsInsn, INSN_FLAG_ASR_NAME,
			rb_str_new_cstr(INSN_FLAG_ASR));
	rb_define_const(clsInsn, INSN_FLAG_ROL_NAME,
			rb_str_new_cstr(INSN_FLAG_ROL));
	rb_define_const(clsInsn, INSN_FLAG_ROR_NAME,
			rb_str_new_cstr(INSN_FLAG_ROR));
	rb_define_const(clsInsn, INSN_FLAG_RCL_NAME,
			rb_str_new_cstr(INSN_FLAG_RCL));
	rb_define_const(clsInsn, INSN_FLAG_RCR_NAME,
			rb_str_new_cstr(INSN_FLAG_RCR));
	rb_define_const(clsInsn, INSN_FLAG_IN_NAME,
			rb_str_new_cstr(INSN_FLAG_IN));
	rb_define_const(clsInsn, INSN_FLAG_OUT_NAME,
			rb_str_new_cstr(INSN_FLAG_OUT));
}

static void init_insn_class( VALUE modOpdis ) {
	clsInsn = rb_define_class_under(modOpdis, "Instruction", rb_cObject);
	rb_define_method(clsInsn, "initialize", cls_insn_new, 1);

	/* read-only attributes */
	//rb_define_attr(clsDisasm, DIS_ATTR_, 1, 0);

	/* setters */
	//rb_define_method(clsDisasm, DIS_ATTR_, cls_disasm_, 1);
}

/* ---------------------------------------------------------------------- */
/* Decoder Class */

static VALUE clsDecoder;

#define DECODER_METHOD "decode"

/* info provided to decoder */
#define DECODER_MEMBER_VMA "vma"
#define DECODER_MEMBER_OFF "offset"
#define DECODER_MEMBER_LEN "size"
#define DECODER_MEMBER_BUF "buffer"
#define DECODER_MEMBER_ITEMS "items"
#define DECODER_MEMBER_STR "raw_insn"
#define DECODER_MEMBER_DELAY "branch_delay"
#define DECODER_MEMBER_DATA "data_size"
#define DECODER_MEMBER_TYPE "type"
#define DECODER_MEMBER_TGT "target"
#define DECODER_MEMBER_TGT2 "target2"

static void fill_decoder_hash( VALUE * hash, const opdis_insn_buf_t in, 
                               const opdis_byte_t * buf, opdis_off_t offset,
                               opdis_vma_t vma, opdis_off_t length ) {
	int i;
	VALUE ary;
	char info = in->insn_info_valid;

	/* instruction location and size */
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_VMA), INT2NUM(vma) );
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_OFF), INT2NUM(offset) );
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_LEN), INT2NUM(length) );

	/* target buffer */
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_BUF), 
		      rb_str_new( (const char *) buf, offset + length ) );
	
	/* decode instruction as provided by libopcodes */
	ary = rb_ary_new();
	for ( i = 0; i < in->item_count; i++ ) {
		rb_ary_push( rb_str_new_cstr(in->items[i]) );
	}

	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_ITEMS), ary );

	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_STR), 
		      rb_str_new_cstr(in->string) );

	/* instruction metadata set by libopcodes */
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_DELAY), 
		      info ? INT2NUM(in->branch_delay_insns) : Qnil);
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_DATA), 
		      info ? INT2NUM(in->data_size) : Qnil);
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_TYPE), 
		      info ? INT2NUM((int) in->insn_type) : Qnil);
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_TGT), 
		      info ? INT2NUM(in->target) : Qnil);
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_TGT2), 
		      info ? INT2NUM(in->target2) : Qnil );

	/* here we cheat and store insn_buf in hash in case one of the local
	 * decoder base classes gets called */
	instance = Data_Wrap_Struct(*hash, NULL, NULL, in);
}

static int invoke_builtin_decoder( OPDIS_DECODER fn, VALUE insn, VALUE hash ) {
	opdis_insn_t c_insn = {0};
	opdis_insn_buf_t inbuf;
        opdis_byte_t * buf;
	opdis_off_t offset;
        opdis_vma_t vma;
	opdis_off_t length;
	
	cls_insn_to_c( insn, *c_insn );
	Data_Get_Struct( hash, opdis_insn_buf_t, inbuf );

	var = rb_hash_lookup2(hash, str_to_sym(DECODER_MEMBER_VMA), Qfalse);
	vma = ( Qfalse != var ) ? NUM2UINT(var) : 0;

	var = rb_hash_lookup2(hash, str_to_sym(DECODER_MEMBER_OFF), Qfalse);
	offset = ( Qfalse != var ) ? NUM2UINT(var) : 0;

	var = rb_hash_lookup2(hash, str_to_sym(DECODER_MEMBER_LEN), Qfalse);
	length = ( Qfalse != var ) ? NUM2UINT(var) : 0;

	var = rb_hash_lookup2(hash, str_to_sym(DECODER_MEMBER_BUF), Qfalse);
	buf = ( Qfalse != var ) ? RSTRING_PTR(buf) : Qnil;

	// TODO
	if ( fn( inbuf, c_insn, buf, offset, vma, length ) ) {
		// copy: insn_to_ruby( c_insn, insn );
		return Qtrue;
	}

	return Qfalse;
}

static VALUE cls_decoder_decode( VALUE instance, VALUE insn, VALUE hash ) {
	// TODO
	// invoke default decoder
	// return decoded insn
}

static void init_decoder_class( VALUE modOpdis ) {
	clsDecoder = rb_define_class_under(modOpdis, "InstructionDecoder", 
					   rb_cObject);
	// TODO
	//rb_define_singleton_method(clsDisasm, "new", cls_disasm_new, 1);
	/* setters */
	//rb_define_method(clsDisasm, DIS_ATTR_, cls_disasm_, 1);
}

/*      ----------------------------------------------------------------- */
/* 	X86 Decoder Class */

static VALUE clsX86Decoder;

static VALUE cls_x86decoder_decode( VALUE instance, VALUE insn, VALUE hash ) {
	// TODO
	// invoke default x86 decoder
	// return decoded insn
}

static void init_x86decoder_class( VALUE modOpdis ) {
	clsX86Decoder = rb_define_class_under(modOpdis, "X86Decoder", 
					      clsDecoder);
	// TODO
	//rb_define_singleton_method(clsDisasm, "new", cls_disasm_new, 1);
	/* setters */
	//rb_define_method(clsDisasm, DIS_ATTR_, cls_disasm_, 1);
}

/* ---------------------------------------------------------------------- */
/* VisitedAddr Class */

/* Handler class determines whether to continue ? */
static VALUE clsHandler;

#define HANDLER_METHOD "visited?"

static VALUE cls_handler_visited( VALUE instance, VALUE insn ) {
	// TODO
	// return t or f if insn has been visited
}

static void init_handler_class( VALUE modOpdis ) {
	clsHandler = rb_define_class_under(modOpdis, "VisitedAddressTracker", 
					   rb_cObject);
	// TODO
	//rb_define_singleton_method(clsDisasm, "new", cls_disasm_new, 1);
	//rb_define_singleton_method(clsDisasm, "new", cls_disasm_new, 1);
	/* setters */
	//rb_define_method(clsDisasm, DIS_ATTR_, cls_disasm_, 1);
}

/* ---------------------------------------------------------------------- */
/* Resolver Class */

static VALUE clsResolver;

#define RESOLVER_METHOD "resolve"

static VALUE cls_resolver_resolve( VALUE instance, VALUE insn ) {
	// TODO
	// return vma for insn operand
}

static void init_resolver_class( VALUE modOpdis ) {
	cls = rb_define_class_under(modOpdis, "AddressResolver", rb_cObject);
	// TODO
	//rb_define_singleton_method(clsDisasm, "new", cls_disasm_new, 1);
	/* setters */
	//rb_define_method(clsDisasm, DIS_ATTR_, cls_disasm_, 1);
}

/* ---------------------------------------------------------------------- */
/* Disasm Output Class */

static VALUE clsOutput;

#define OUT_ATTR_ERRORS "errors"

static VALUE cls_output_contain( VALUE instance, VALUE vma ) {
	// TODO
	// return insn containing vma
}

static VALUE cls_output_new( VALUE class ) {
	// TODO
	// attr ro errors = []
}

static void init_output_class( VALUE modOpdis ) {
	clsOutput = rb_define_class_under(modOpdis, "Disassembly", rb_cHash);
	// TODO
	// attr rw errors
	//rb_define_singleton_method(clsDisasm, "new", cls_disasm_new, 1);
	/* setters */
	//rb_define_method(clsDisasm, DIS_ATTR_, cls_disasm_, 1);
}


/* ---------------------------------------------------------------------- */
/* Disassembler Architectures (internal) */
struct arch_def { 
	const char * name; 
	enum bfd_architecture arch;
	unsigned long default_mach;
	disassembler_ftype fn;
};

static struct arch_def arch_definitions[] = {
	// TODO: arm, ia64, ppc, sparc, etc

	{"8086", bfd_arch_i386, bfd_mach_i386_i8086, print_insn_i386},
	{"x86", bfd_arch_i386, bfd_mach_i386_i386, print_insn_i386},
	{"x86_att", bfd_arch_i386, bfd_mach_i386_i386, print_insn_i386_att},
	{"x86_intel", bfd_arch_i386, bfd_mach_i386_i386_intel_syntax, 
		      print_insn_i386_intel},
	{"x86_64", bfd_arch_i386, bfd_mach_x86_64, print_insn_i386},
	{"x86_64_att", bfd_arch_i386, bfd_mach_x86_64, print_insn_i386},
	{"x86_64_intel", bfd_arch_i386, bfd_mach_x86_64_intel_syntax, 
			         print_insn_i386}
};




/* ---------------------------------------------------------------------- */
/* Disassembler Class */

static VALUE clsDisasm;

/* method names */
#define DIS_METHOD_DISASM "disassemble"
#define DIS_ALIAS_DISASM "disasm"

/* attribute names */
#define DIS_ATTR_DECODER "insn_decoder"
#define DIS_ATTR_HANDLER "addr_tracker"
#define DIS_ATTR_RESOLVER "resolver"
#define DIS_ATTR_DEBUG "debug"
#define DIS_ATTR_SYNTAX "syntax"
#define DIS_ATTR_ARCH "arch"
#define DIS_ATTR_OPTS "opcodes_options"

/* argument (hash) names */
#define DIS_ARG_DECODER DIS_ATTR_DECODER 
#define DIS_ARG_HANDLER DIS_ATTR_HANDLER
#define DIS_ARG_RESOLVER DIS_ATTR_RESOLVER
#define DIS_ARG_SYNTAX "syntax"
#define DIS_ARG_DEBUG "debug"
#define DIS_ARG_OPTIONS "options"
#define DIS_ARG_STRATEGY "strategy"
#define DIS_ARG_ARCH "arch"
#define DIS_ARG_VMA "vma"
#define DIS_ARG_OFFSET "offset"
#define DIS_ARG_LEN "length"
#define DIS_ARG_BUFVMA "buffer_vma"

/* constants */
#define DIS_ERR_BOUNDS_NAME "ERROR_BOUNDS"
#define DIS_ERR_BOUNDS "Bounds exceeded"
#define DIS_ERR_INVALID_NAME "ERROR_INVALID_INSN"
#define DIS_ERR_INVALID "Invalid instruction"
#define DIS_ERR_DECODE_NAME "ERROR_DECODE_INSN"
#define DIS_ERR_DECODE "Decoder error"
#define DIS_ERR_BFD_NAME "ERROR_BFD"
#define DIS_ERR_BFD "Bfd error"
#define DIS_ERR_MAX_NAME "ERROR_MAX_ITEMS"
#define DIS_ERR_MAX "Max insn items error"
#define DIS_ERR_UNK "Unknown error"
#define DIS_STRAT_SINGLE_NAME "STRATEGY_SINGLE"
#define DIS_STRAT_SINGLE "single"
#define DIS_STRAT_LINEAR_NAME "STRATEGY_LINEAR"
#define DIS_STRAT_LINEAR "linear"
#define DIS_STRAT_CFLOW_NAME "STRATEGY_CFLOW"
#define DIS_STRAT_CFLOW "cflow"
#define DIS_STRAT_SYMBOL_NAME "STRATEGY_SYMBOL"
#define DIS_STRAT_SYMBOL "symbol"
#define DIS_STRAT_SECTION_NAME "STRATEGY_SECTION"
#define DIS_STRAT_SECTION "section"
#define DIS_STRAT_ENTRY_NAME "STRATEGY_ENTRY"
#define DIS_STRAT_ENTRY "entry"

#define DIS_CONST_STRATEGIES "strategies"

#define DIS_CONST_ARCHES "architectures"


static VALUE cls_disasm_architectures( VALUE class ) {
	VALUE ary = rb_ary_new();
	int i;
	int num_defs = sizeof(arch_definitions) / sizeof(struct arch_def);

	for ( i = 0; i < num_defs; i++ ) {
		struct arch_def *def = &arch_definitions[i];
		rb_ary_push( ary, rb_str_new_cstr(def->name) );
	}

	return ary;
}

static VALUE cls_disasm_strategies( VALUE class ) {
	VALUE ary = rb_ary_new();
	rb_ary_push( rb_str_new_cstr(DIS_STRAT_SINGLE) );
	rb_ary_push( rb_str_new_cstr(DIS_STRAT_LINEAR) );
	rb_ary_push( rb_str_new_cstr(DIS_STRAT_CFLOW) );
	rb_ary_push( rb_str_new_cstr(DIS_STRAT_SYMBOL) );
	rb_ary_push( rb_str_new_cstr(DIS_STRAT_SECTION) );
	rb_ary_push( rb_str_new_cstr(DIS_STRAT_ENTRY) );
	return ary;
}

/* local decoder callback: this calls the decode method in the object provided
 * by the user. */
static int local_decoder( const opdis_insn_buf_t in, opdis_insn_t * out,
                          const opdis_byte_t * buf, opdis_off_t offset,
                          opdis_vma_t vma, opdis_off_t length, void * arg ) {
	VALUE obj = (VALUE) arg;
	VALUE hash = rb_hash_new();
	VALUE insn = insn_to_ruby(out);

	/* build hash containing insn info */
	fill_decoder_hash( &hash, in, buf, offset, vma, length );

	VALUE var = rb_funcall(obj, rb_intern(DECODER_METHOD), 2, insn, hash);
	return (Qfalse == var || Qnil == var) ? 0 : 1;
}

static VALUE cls_disasm_set_decoder(VALUE instance, VALUE obj) {
	opdist_t  opdis;
	Data_Get_Struct(instance, opdis_t, opdis);

	if ( Qnil == obj ) {
		opdis_set_decoder( opdis, opdis_default_decoder, NULL );
		return Qtrue;
	}

	if (! rb_respond_to(object, rb_intern(DECODER_METHOD)) ) {
		return Qfalse;
	}
	
	opdis_set_decoder( opdis, local_decoder, obj );

	return Qtrue;
}

/* local insn handler object: this invokes the visited? method in the handler
 * object provided by the user. */
static int local_handler( const opdis_insn_t * i, void * arg ) {
	VALUE obj = (VALUE) arg;
	VALUE insn = insn_to_ruby(i);
	VALUE var = rb_funcall(obj, rb_intern(HANDLER_METHOD), 1, insn);
	/* True means already visited, so continue = 0 */
	return (Qtrue == var) ? 0 : 1;
}

static VALUE cls_disasm_set_handler(VALUE instance, VALUE obj) {
	opdist_t  opdis;
	Data_Get_Struct(instance, opdis_t, opdis);

	if ( Qnil == obj ) {
		opdis_set_handler( opdis, opdis_default_handler, opdis );
		return Qtrue;
	}

	if (! rb_respond_to(object, rb_intern(HANDLER_METHOD)) ) {
		return Qfalse;
	}

	opdis_set_handler( opdis, local_handler, obj );
	rb_iv_set(instance, IVAR(DIS_ATTR_HANDLER), obj );

	return Qtrue;
}

/* local resolver callback: this invokes the ruby resolve method in the object
 * provided by the user */
static opdis_vma_t local_resolver ( const opdis_insn_t * i, void * arg ) {
	VALUE obj = (VALUE) arg;
	VALUE insn = insn_to_ruby(i);
	VALUE vma = rb_funcall(obj, rb_intern(RESOLVER_METHOD), 1, insn);
	return (Qnil == vma) ? OPDIS_INVALID_ADDR : (opdis_vma_t) NUM2UINT(vma);
}

static VALUE cls_disasm_set_resolver(VALUE instance, VALUE obj) {
	opdist_t  opdis;
	Data_Get_Struct(instance, opdis_t, opdis);

	if ( Qnil == obj ) {
		opdis_set_resolver( opdis, opdis_default_resolver, NULL );
		return Qtrue;
	}

	if (! rb_respond_to(object, rb_intern(RESOLVER_METHOD)) ) {
		return Qfalse;
	}
	
	opdis_set_resolver( opdis, local_resolver, obj );
	rb_iv_set(instance, IVAR(DIS_ATTR_RESOLVER), obj );

	return Qtrue;
}

static VALUE cls_disasm_set_debug(VALUE instance, VALUE enabled) {
	opdist_t  opdis;
	Data_Get_Struct(instance, opdis_t, opdis);
	opdis->debug = (enabled == Qtrue) ? 1 : 0;
	return Qtrue;
}

static VALUE cls_disasm_set_syntax(VALUE instance, VALUE syntax) {
	opdist_t  opdis;
	Data_Get_Struct(instance, opdis_t, opdis);
	// TODO
// set x86syntax
	return Qtrue;
}

static VALUE cls_disasm_set_arch(VALUE instance, VALUE arch) {
	opdist_t  opdis;
	struct disassemble_info * info;
	int i;
	int num_defs = sizeof(arch_definitions) / sizeof(struct arch_def);
	const char * name = rb_string_value_cstr(&arch);

	Data_Get_Struct(instance, opdis_t, opdis);
	info = &opdis->config;

	info->application_data = def[0]->fn;	// no NULL pointers here, suh!
	info->arch = bfd_arch_unknown;
	info->mach = 0;

	for ( i = 0; i < num_defs; i++ ) {
		struct disasm_def *def = &disasm_definitions[i];
		if (! strcmp(name, def->name) ) {
			info->application_data = def->fn;
			info->arch = def->arch;
			info->mach = def->default_mach;
			rb_iv_set(instance, IVAR(DIS_ATTR_ARCH), arch);
			return Qtrue;
		}
	}

	return Qfalse;
}

static VALUE cls_disasm_set_opts(VALUE instance, VALUE opts) {
	char * str;
	opdist_t  opdis;
	Data_Get_Struct(instance, opdis_t, opdis);

	str = StringValueCStr(rb_any_to_s(opts));
	opdis_set_disassembler_options( opdis, str );
	return Qtrue;
}

/* get opdis options from an argument hash */
static void cls_disasm_handle_args( VALUE instance, VALUE hash ) {
	VALUE var;

	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_RESOLVER), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_resolver(instance, var);

	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_HANDLER), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_handler(instance, var);

	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_DECODER), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_decoder(instance, var);

	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_SYNTAX), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_syntax(instance, var);

	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_DEBUG), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_debug(instance, var);

	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_OPTIONS), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_opts(instance, var);

	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_ARCH), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_arch(VALUE instance, VALUE arch);
}

/* local display handler for blocks: this yields the current insn to arg */
static void local_block_display( const opdis_insn_t * i, void * arg ) {
	VALUE block = (VALUE) arg;
	VALUE insn = insn_to_ruby(i);

	rb_funcall(block, rb_intern("call"), 1, insn);
}

/* local display handler: this adds instructions to a Disassembly object */
static void local_display( const opdis_insn_t * i, void * arg ) {
	VALUE output = (VALUE) arg;
	VALUE insn = insn_to_ruby(i);
	rb_hash_aset( output, INT2NUM(i->vma), insn );
}

/* local error handler: this appends errors to a ruby array in arg */
static void local_error( enum opdis_error_t error, const char * msg,
                         void * arg ) {
	VALUE str, output = (VALUE) arg;
	const char * type;
	char buf[128];

	switch (error) {
		case opdis_error_bounds: type = DIS_ERR_BOUNDS; break;
		opdis_error_invalid_insn: type = DIS_ERR_INVALID; break;
		opdis_error_decode_insn: type = DIS_ERR_DECODE; break;
		opdis_error_bfd: type = DIS_ERR_BFD; break;
		opdis_error_max_items: type = DIS_ERR_MAX; break;
		opdis_error_unknown: 
		default: type = DIS_ERR_UNK; break;
	}

	snprintf( buf, 127-1, "%s: $s", type, msg );
	str = rb_str_new_cstr(buf);
	rb_funcall(object, rb_intern("<<"), 1, str)
}

static void config_buf_from_args( opdis_buf_t * buf, VALUE hash ) {
	VALUE var;

	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_VMA), Qfalse);
	if ( Qfalse != var && Qnil != var ) buf->vma = NUM2UINT(var);

	// TODO: other options? 
}

static opdis_buf_t opdis_buf_for_target( VALUE tgt, VALUE hash ) {
	// TODO
	if ( Qtrue == rb_obj_is_kind_of( tgt, rb_cString ) ) {
		// str 2 buf
	} else if ( Qtrue == rb_obj_is_kind_of( tgt, rb_cArray ) ) {
		// array to buf
	} else if ( Qtrue == rb_obj_is_kind_of( tgt, rb_cIO ) ) {
		// read file to buf
	} else {
		// raise
	}

	/* apply target-specific args (vma, etc) */
	config_buf_from_args( buf, hash );
}

struct OPDIS_TGT { bfd * abfd; asection * sec; asymbol * sym; opdis_buf_t buf };

static void load_target( opdist_t opdis, VALUE tgt, VALUE hash, 
			 struct OPDIS_TGT * out ) {

	if ( Qnil != GET_BFD_CLASS(clsBfdTgt, BFD_TGT_PATH) &&
	     Qtrue == rb_obj_is_kind_of( tgt, clsBfdTgt ) ) {
		DataGetStruct(tgt, bfd, out->abfd );
	} else if ( Qnil != GET_BFD_CLASS(clsBfdSym, BFD_SYM_PATH) &&
	     Qtrue == rb_obj_is_kind_of( tgt, clsBfdSym ) ) {
		asymbol * s;
		DataGetStruct(tgt, asymbol, out->sym );
		if ( out->sym ) {
			out->abfd = s->the_bfd;
		}
	} else if ( Qnil != GET_BFD_CLASS(clsBfdSec, BFD_SEC_PATH) &&
	     Qtrue == rb_obj_is_kind_of( tgt, clsBfdSec ) ) {
		DataGetStruct(tgt, asection, out->sec );
		if ( out->sec ) {
			out->abfd = s->owner;
		}
	} else {
		out->buf = opdis_buf_for_target( tgt, hash );
	}

	if ( out->abfd ) {
		opdis_config_from_bfd( opdis, out->abfd );
	}
}

static int perform_disassembly( VALUE instance, opdist_t opdis, VALUE target,
				VALUE hash ) {
	VALUE var;
	VALUE vma = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_VMA), INT2NUM(0));
	VALUE len = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_LEN), INT2NUM(0));
	const char * strategy = DIS_STRAT_LINEAR;
	struct OPDIS_TGT tgt = {0};

	/* apply general args (syntax, arch, etc) */
	cls_disasm_handle_args(instance, hash);
	
	load_target( opdis, target, hash, &tgt );

	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_STRATEGY), Qfalse);
	if ( Qfalse != var ) strategy = StringValueCStr(var);

	// TODO
	// rb_thread_schedule()
	// (in every callback?)

	if (! strcmp( strategy, DIS_STRAT_SINGLE ) ) {
		// TODO: alloc fixed insn
		opdis_insn_t * insn;

		if ( tgt.abfd ) {
			// not impl!
			//opdis_disasm_insn( opdis, tgt.abfd, vma, nsn );
		} else {
			opdis_disasm_insn( opdis, tgt.buf, vma, insn );
		}

	} else if (! strcmp( strategy, DIS_STRAT_LINEAR ) ) {
		if ( tgt.abfd ) {
		 	opdis_disasm_bfd_linear( opdis, tgt.bfd, vma, len );
		} else {
		 	opdis_disasm_linear( opdis, tgt.buf, vma, len );
		}

	} else if (! strcmp( strategy, DIS_STRAT_CFLOW ) ) {
		if ( tgt.abfd ) {
			opdis_disasm_bfd_cflow( opdis, tgt.abfd, vma );
		} else {
			opdis_disasm_cflow( opdis, tgt.buf, vma );
		}

	} else if (! strcmp( strategy, DIS_STRAT_SYMBOL ) ) {
		if (! tgt.sym ) {
			rb_raise(rb_eArgError, "Bfd::Symbol required");
		}
		opdis_disasm_bfd_symbol( opdis, tgt.sym );

	} else if (! strcmp( strategy, DIS_STRAT_SECTION ) ) {
		if (! tgt.sec ) {
			rb_raise(rb_eArgError, "Bfd::Section required");
		}
		opdis_disasm_bfd_section( opdis, tgt.sec );

	} else if (! strcmp( strategy, DIS_STRAT_ENTRY ) ) {
		if (! tgt.abfd ) {
			rb_raise(rb_eArgError, "Bfd::Target required");
		}
		opdis_disasm_bfd_entry( opdis, tgt.abfd );
	} else {
		rb_raise(rb_eArgError, "Unknown strategy '%s'", strategy);
	}
}


/* Disassembler strategies produce blocks */
static VALUE cls_disasm_disassemble(VALUE instance, VALUE tgt, VALUE hash ) {
	VALUE output, errors;
	opdist_t opdis;

	// TODO:  opdis_dupe in order to be threadsafe
	Data_Get_Struct(instance, opdis_t, opdis);

	/* yielding is easy */
	if ( rb_block_given_p() ) {
		opdis_set_display(opdis, local_block_display, rb_block_proc());
		return perform_disassembly( instance, opdis, tgt, hash );
	}

	/* otherwise we have to fill an output object */
	output = cls_output_new( clsOutput );
	errors = rb_iv_get( output, IVAR(OUT_ATTR_ERRORS) );

	opdis_set_display(opdis, local_display, output);
	opdis_set_error_reporter(opdis, local_error, errors);

	perform_disassembly( instance, opdis, tgt, hash );

	return output;
}

/* new: takes hash of arguments */
static VALUE cls_disasm_new(VALUE class, VALUE hash) {
	VALUE instance, var;
	VALUE argv[1] = { Qnil };
	opdist_t  opdis = opdis_init();

	instance = Data_Wrap_Struct(class, NULL, opdis_term, opdis);
	rb_obj_call_init(instance, 0, argv);
	//rb_obj_call_init(instance, 0, &Qnil);

	cls_disasm_handle_args(instance, hash);

	return instance;
}

static void define_disasm_constants() {
	rb_define_const(clsDisasm, DIS_ERR_BOUNDS_NAME,
			rb_str_new_cstr(DIS_ERR_BOUNDS));
	rb_define_const(clsDisasm, DIS_ERR_INVALID_NAME,
			rb_str_new_cstr(DIS_ERR_INVALID));
	rb_define_const(clsDisasm, DIS_ERR_DECODE_NAME,
			rb_str_new_cstr(DIS_ERR_DECODE));
	rb_define_const(clsDisasm, DIS_ERR_BFD_NAME,
			rb_str_new_cstr(DIS_ERR_BFD));
	rb_define_const(clsDisasm, DIS_ERR_MAX_NAME,
			rb_str_new_cstr(DIS_ERR_MAX));

	rb_define_const(clsDisasm, DIS_STRAT_SINGLE_NAME,
			rb_str_new_cstr(DIS_STRAT_SINGLE));
	rb_define_const(clsDisasm, DIS_STRAT_LINEAR_NAME,
			rb_str_new_cstr(DIS_STRAT_LINEAR));
	rb_define_const(clsDisasm, DIS_STRAT_CFLOW_NAME,
			rb_str_new_cstr(DIS_STRAT_CFLOW));
	rb_define_const(clsDisasm, DIS_STRAT_SYMBOL_NAME,
			rb_str_new_cstr(DIS_STRAT_SYMBOL));
	rb_define_const(clsDisasm, DIS_STRAT_SECTION_NAME,
			rb_str_new_cstr(DIS_STRAT_SECTION));
	rb_define_const(clsDisasm, DIS_STRAT_ENTRY_NAME,
			rb_str_new_cstr(DIS_STRAT_ENTRY));

	rb_define_singleton_method(clsDisasm, DIS_CONST_STRATEGIES, 
				   cls_disasm_strategies, 0);
	rb_define_singleton_method(clsDisasm, DIS_CONST_ARCHES, 
				   cls_disasm_architectures, 0);
}

static void init_disasm_class( VALUE modOpdis ) {
	clsDisasm = rb_define_class_under(modOpdis, "Disassembler", rb_cObject);
	rb_define_singleton_method(clsDisasm, "new", cls_disasm_new, 1);

	/* read-only attributes */
	rb_define_attr(clsDisasm, DIS_ATTR_DECODER, 1, 0);
	rb_define_attr(clsDisasm, DIS_ATTR_HANDLER, 1, 0);
	rb_define_attr(clsDisasm, DIS_ATTR_RESOLVER, 1, 0);

	/* setters */
	rb_define_method(clsDisasm, SETTER(DIS_ATTR_DECODER), 
			 cls_disasm_set_decoder, 1);
	rb_define_method(clsDisasm, SETTER(DIS_ATTR_HANDLER), 
			 cls_disasm_set_handler, 1);
	rb_define_method(clsDisasm, SETTER(DIS_ATTR_RESOLVER), 
			 cls_disasm_set_resolver, 1);
	rb_define_method(clsDisasm, SETTER(DIS_ATTR_DEBUG), 
			 cls_disasm_set_debug, 1);
	rb_define_method(clsDisasm, SETTER(DIS_ATTR_SYNTAX), 
			 cls_disasm_set_syntax, 1);
	rb_define_method(clsDisasm, SETTER(DIS_ATTR_ARCH), 
			 cls_disasm_set_arch, 1);
	rb_define_method(clsDisasm, SETTER(DIS_ATTR_OPTS), 
			 cls_disasm_set_opts, 1);
	rb_define_method(clsDisasm, DIS_ATTR_DEBUG, cls_disasm_get_debug, 0);
	rb_define_method(clsDisasm, DIS_ATTR_SYNTAX, cls_disasm_get_syntax, 0);
	rb_define_method(clsDisasm, DIS_ATTR_ARCH, cls_disasm_get_arch, 0);
	rb_define_method(clsDisasm, DIS_ATTR_OPTS, cls_disasm_get_opts, 0);

	/* methods */
	rb_define_method(clsDisasm, DIS_METHOD_DISASM, cls_disasm_disassemble, 
			 2);

	/* aliases */
	rb_define_alias(clsDisasm, DIS_ALIAS_DISASM, DIS_METHOD_DISASM );

	define_disasm_constants();
}

/* ---------------------------------------------------------------------- */
/* Opdis Module */

static VALUE modOpdis;
void Init_Opdis() {
	modOpdis = rb_define_module("Opdis");

	init_disasm_class(modOpdis);
	init_tgt_class(modOpdis);
	init_bfddis_class(modOpdis);
	init_output_class(modOpdis);
	init_resolver_class(modOpdis);
	init_handler_class(modOpdis);
	init_decoder_class(modOpdis);
	init_insn_class(modOpdis);
	init_op_class(modOpdis);
	init_absaddr_class(modOpdis);
	init_addrexp_class(modOpdis);
	init_reg_class(modOpdis);
}
