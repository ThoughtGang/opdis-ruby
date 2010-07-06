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

/* BFD Support (requires BFD gem) */
static VALUE clsBfd = Qnil;
static VALUE clsBfdSec = Qnil;
static VALUE clsBfdSym = Qnil;
#define BFD_TGT_PATH "Bfd::Target"
#define BFD_SEC_PATH "Bfd::Section"
#define BFD_SYM_PATH "Bfd::Symbol"
#define GET_BFD_CLASS(cls,name) (cls = cls == Qnil ? rb_path2class(name) : cls);

#define ALLOC_FIXED_INSN opdis_alloc_fixed(128, 32, 16, 32)

/* ---------------------------------------------------------------------- */
/* Supported architectures */

/* ---------------------------------------------------------------------- */
/* Shared Methods */

#define GEN_ATTR_ASCII "ascii"
static VALUE cls_generic_to_s( VALUE instance ) {
	return rb_iv_get(instance, IVAR(GEN_ATTR_ASCII) );
}

/* ---------------------------------------------------------------------- */
/* Operand Base Class */

static VALUE clsOp;

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
	rb_iv_set(instance, IVAR(INSN_ATTR_FLAGS), rb_ary_new() );
	return instance;
}

static void fill_ruby_op( opdis_op_t * op, VALUE dest ) {
	// TODO
}

static VALUE op_from_c( opdis_op_t * op ) {
	switch (op->category) {
		case opdis_op_cat_register:
		case opdis_op_cat_immediate:
		case opdis_op_cat_absolute:
		case opdis_op_cat_expr:
		case opdis_op_cat_unknown:
		default;
	}

	//VALUE var = rb_class_new(clsOp);
	//fill_ruby_op(op, var);
	return var;
}

static void op_to_c( VALUE op, opdis_op_t * dest ) {
	// TODO
}

static void define_op_constants() {
	rb_define_const(clsOp, OP_FLAG_R_NAME, rb_str_new_cstr(OP_FLAG_R));
	rb_define_const(clsOp, OP_FLAG_W_NAME, rb_str_new_cstr(OP_FLAG_W));
	rb_define_const(clsOp, OP_FLAG_X_NAME, rb_str_new_cstr(OP_FLAG_X));
	rb_define_const(clsOp, OP_FLAG_SIGNED_NAME,
			rb_str_new_cstr(OP_FLAG_SIGNED));
	rb_define_const(clsOp, OP_FLAG_ADDR_NAME,
			rb_str_new_cstr(OP_FLAG_ADDR));
	rb_define_const(clsOp, OP_FLAG_IND_NAME, rb_str_new_cstr(OP_FLAG_IND));
}

static void init_op_class( VALUE modOpdis ) {
	clsOp = rb_define_class_under(modOpdis, "Operand", rb_cObject);
	rb_define_method(clsOp, "initialize", cls_op_init, 1);
	rb_define_method(clsOp, "to_s", cls_generic_to_s, 0);

	define_op_constants();
	// TODO

	/* read-only attributes */
	//rb_define_attr(clsDisasm, DIS_ATTR_, 1, 0);

	/* setters */
	//rb_define_method(clsDisasm, DIS_ATTR_, cls_disasm_, 1);
}

/* ---------------------------------------------------------------------- */
/* Register Class */

static VALUE clsReg;

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


static void set_ruby_reg_flags( VALUE instance, enum opdis_reg_flag_t val ) {
	VALUE flags = rb_iv_get(instance, IVAR(REG_ATTR_FLAGS) );
	if ( val & opdis_reg_flag_gen ) {
		rb_arry_push(flags, rb_str_new_cstr( REG_FLAG_GEN) );
	}
	if ( val & opdis_reg_flag_fpu ) {
		rb_arry_push(flags, rb_str_new_cstr( REG_FLAG_FPU) );
	}
	if ( val & opdis_reg_flag_gpu ) {
		rb_arry_push(flags, rb_str_new_cstr( REG_FLAG_GPU) );
	}
	if ( val & opdis_reg_flag_simd ) {
		rb_arry_push(flags, rb_str_new_cstr( REG_FLAG_SIMD) );
	}
	if ( val & opdis_reg_flag_task ) {
		rb_arry_push(flags, rb_str_new_cstr( REG_FLAG_TASK) );
	}
	if ( val & opdis_reg_flag_mem ) {
		rb_arry_push(flags, rb_str_new_cstr( REG_FLAG_MEM) );
	}
	if ( val & opdis_reg_flag_debug ) {
		rb_arry_push(flags, rb_str_new_cstr( REG_FLAG_DBG) );
	}
	if ( val & opdis_reg_flag_pc ) {
		rb_arry_push(flags, rb_str_new_cstr( REG_FLAG_PC) );
	}
	if ( val & opdis_reg_flag_flags ) {
		rb_arry_push(flags, rb_str_new_cstr( REG_FLAG_CC) );
	}
	if ( val & opdis_reg_flag_stack ) {
		rb_arry_push(flags, rb_str_new_cstr( REG_FLAG_STACK) );
	}
	if ( val & opdis_reg_flag_frame ) {
		rb_arry_push(flags, rb_str_new_cstr( REG_FLAG_FRAME) );
	}
	if ( val & opdis_reg_flag_seg ) {
		rb_arry_push(flags, rb_str_new_cstr( REG_FLAG_SEG) );
	}
	if ( val & opdis_reg_flag_zero ) {
		rb_arry_push(flags, rb_str_new_cstr( REG_FLAG_Z) );
	}
	if ( val & opdis_reg_flag_argsin ) {
		rb_arry_push(flags, rb_str_new_cstr( REG_FLAG_IN) );
	}
	if ( val & opdis_reg_flag_argsout ) {
		rb_arry_push(flags, rb_str_new_cstr( REG_FLAG_OUT) );
	}
	if ( val & opdis_reg_flag_locals ) {
		rb_arry_push(flags, rb_str_new_cstr( REG_FLAG_LOCALS) );
	}
	if ( val & opdis_reg_flag_return ) {
		rb_arry_push(flags, rb_str_new_cstr( REG_FLAG_RET) );
	}
}

static void set_ruby_reg_flags( opdis_reg_t * dest, VALUE reg ) {
	int i, num_flags;
	VALUE flags = rb_iv_get(reg, IVAR(REG_ATTR_FLAGS) );
	dest->flags = opdis_reg_flag_unknown;

	num_flags = NUM2INT(rb_funcall(flags, rb_intern("size"), 0));
	for ( i=0; i < num_flags; i++ ) {
		VALUE val = rb_ary_entry( flags, i );
		if (! strcmp( REG_FLAG_GEN, StringValueCStr(val) ) ) {
			dest->flags |= opdis_reg_flag_gen;
		} else if (! strcmp( REG_FLAG_FPU, StringValueCStr(val) ) ) {
			dest->flags |= opdis_reg_flag_fpu;
		} else if (! strcmp( REG_FLAG_GPU, StringValueCStr(val) ) ) {
			dest->flags |= opdis_reg_flag_gpu;
		} else if (! strcmp( REG_FLAG_SIMD, StringValueCStr(val) ) ) {
			dest->flags |= opdis_reg_flag_simd;
		} else if (! strcmp( REG_FLAG_TASK, StringValueCStr(val) ) ) {
			dest->flags |= opdis_reg_flag_task;
		} else if (! strcmp( REG_FLAG_MEM, StringValueCStr(val) ) ) {
			dest->flags |= opdis_reg_flag_mem;
		} else if (! strcmp( REG_FLAG_DBG, StringValueCStr(val) ) ) {
			dest->flags |= opdis_reg_flag_debug;
		} else if (! strcmp( REG_FLAG_PC, StringValueCStr(val) ) ) {
			dest->flags |= opdis_reg_flag_pc;
		} else if (! strcmp( REG_FLAG_CC, StringValueCStr(val) ) ) {
			dest->flags |= opdis_reg_flag_flags;
		} else if (! strcmp( REG_FLAG_STACK, StringValueCStr(val) ) ) {
			dest->flags |= opdis_reg_flag_stack;
		} else if (! strcmp( REG_FLAG_FRAME, StringValueCStr(val) ) ) {
			dest->flags |= opdis_reg_flag_frame;
		} else if (! strcmp( REG_FLAG_SEG, StringValueCStr(val) ) ) {
			dest->flags |= opdis_reg_flag_seg;
		} else if (! strcmp( REG_FLAG_Z, StringValueCStr(val) ) ) {
			dest->flags |= opdis_reg_flag_zero;
		} else if (! strcmp( REG_FLAG_IN, StringValueCStr(val) ) ) {
			dest->flags |= opdis_reg_flag_argsin;
		} else if (! strcmp( REG_FLAG_OUT, StringValueCStr(val) ) ) {
			dest->flags |= opdis_reg_flag_argsout;
		} else if (! strcmp( REG_FLAG_LOCALS, StringValueCStr(val) ) ) {
			dest->flags |= opdis_reg_flag_locals;
		} else if (! strcmp( REG_FLAG_RET, StringValueCStr(val) ) ) {
			dest->flags |= opdis_reg_flag_return;
		}
	}
}

static void fill_ruby_reg( opdis_reg_t * reg, VALUE dest ) {
	rb_iv_set(dest, IVAR(REG_ATTR_ID), UINT2NUM(reg->id) );
	rb_iv_set(dest, IVAR(REG_ATTR_SIZE), UINT2NUM(reg->size) );
	rb_iv_set(dest, IVAR(GEN_ATTR_ASCII), rb_str_new_cstr(reg->ascii) );
	set_ruby_reg_flags(dest, reg->flags);
}

static VALUE reg_from_c( opdis_reg_t * reg ) {
	VALUE var = rb_class_new(clsReg);
	fill_ruby_reg(reg, var);
	return var;
}

static void reg_to_c( VALUE reg, opdis_reg_t * dest ) {
	VALUE val;

	val = rb_iv_get(dest, IVAR(REG_ATTR_ID));
	dest->id = (val == Qnil) ? 0 : (unsigned char) NUM2UINT(val);

	val = rb_iv_get(dest, IVAR(REG_ATTR_SIZE));
	dest->size = (val == Qnil) ? 0 : (unsigned char) NUM2UINT(size);

	val = rb_iv_get(dest, IVAR(GEN_ATTR_ASCII));
	if ( val != Qnil ) {
		strncpy( reg->ascii, StringValueCStr(val), OPDIS_REG_NAME_SZ );
	}

	set_c_reg_flags( dest, reg );
}


/* ---------------------------------------------------------------------- */
/* Register Operand */

static VALUE clsRegOp;

static VALUE reg_op_from_c( opdis_reg_t * reg ) {
	VALUE var = rb_class_new(clsRegOp);
	fill_ruby_reg(reg, var);
	return var;
}

static void init_reg_attributes( VALUE class ) {
	rb_define_attr(class, REG_ATTR_ID, 1, 0);
	rb_define_attr(class, GEN_ATTR_ASCII, 1, 0);
	rb_define_attr(class, REG_ATTR_FLAGS, 1, 0);
	rb_define_attr(class, REG_ATTR_SIZE, 1, 0);
}

static void define_reg_constants( VALUE class ) {
	rb_define_const(class, REG_FLAG_GEN_NAME,
			rb_str_new_cstr(REG_FLAG_GEN));
	rb_define_const(class, REG_FLAG_FPU_NAME,
			rb_str_new_cstr(REG_FLAG_FPU));
	rb_define_const(class, REG_FLAG_GPU_NAME,
			rb_str_new_cstr(REG_FLAG_GPU));
	rb_define_const(class, REG_FLAG_SIMD_NAME,
			rb_str_new_cstr(REG_FLAG_SIMD));
	rb_define_const(class, REG_FLAG_TASK_NAME,
			rb_str_new_cstr(REG_FLAG_TASK));
	rb_define_const(class, REG_FLAG_MEM_NAME,
			rb_str_new_cstr(REG_FLAG_MEM));
	rb_define_const(class, REG_FLAG_DBG_NAME,
			rb_str_new_cstr(REG_FLAG_DBG));
	rb_define_const(class, REG_FLAG_PC_NAME,
			rb_str_new_cstr(REG_FLAG_PC));
	rb_define_const(class, REG_FLAG_CC_NAME,
			rb_str_new_cstr(REG_FLAG_CC));
	rb_define_const(class, REG_FLAG_STACK_NAME,
			rb_str_new_cstr(REG_FLAG_STACK));
	rb_define_const(class, REG_FLAG_FRAME_NAME,
			rb_str_new_cstr(REG_FLAG_FRAME));
	rb_define_const(class, REG_FLAG_SEG_NAME,
			rb_str_new_cstr(REG_FLAG_SEG));
	rb_define_const(class, REG_FLAG_Z_NAME,
			rb_str_new_cstr(REG_FLAG_Z));
	rb_define_const(class, REG_FLAG_IN_NAME,
			rb_str_new_cstr(REG_FLAG_IN));
	rb_define_const(class, REG_FLAG_OUT_NAME,
			rb_str_new_cstr(REG_FLAG_OUT));
	rb_define_const(class, REG_FLAG_LOCALS_NAME,
			rb_str_new_cstr(REG_FLAG_LOCALS));
	rb_define_const(class, REG_FLAG_RET_NAME,
			rb_str_new_cstr(REG_FLAG_RET));
}

static VALUE cls_reg_init(VALUE instance) {
	rb_iv_set(instance, IVAR(REG_ATTR_FLAGS), rb_ary_new() );
	return instance;
}

static void init_reg_class( VALUE modOpdis ) {
	/* Register */
	clsReg = rb_define_class_under(modOpdis, "Register", rb_cObject);

	rb_define_method(clsReg, "initialize", cls_reg_init, 0);
	rb_define_method(clsReg, "to_s", cls_generic_to_s, 0);

	//rb_define_attr(clsDisasm, GEN_ATTR_ASCII, 1, 1);

	define_reg_constants( clsReg );
	init_reg_attributes( clsRegOp );

	/* Register Operand */
	clsRegOp = rb_define_class_under(modOpdis, "RegisterOperand", clsOp);
	rb_define_method(clsRegOp, "initialize", cls_reg_init, 0);
	rb_define_method(clsRegOp, "to_s", cls_generic_to_s, 0);

	define_reg_constants( clsRegOp );
	init_reg_attributes( clsRegOp );
}

/* ---------------------------------------------------------------------- */
/* Absolute Address Operand Class */

static VALUE clsAbsAddr;

#define ABS_ADDR_ATTR_SEG "segment"
#define ABS_ADDR_ATTR_OFF "offset"

static void fill_ruby_absaddr( opdis_abs_addr_t * addr, VALUE dest ) {
	rb_iv_set(dest, IVAR(ABS_ADDR_ATTR_SEG), reg_from_c(&addr->segment) );
	rb_iv_set(dest, IVAR(ABS_ADDR_ATTR_OFF), addr->offset );
}

static VALUE absaddr_from_c( opdis_abs_addr_t * addr ) {
	VALUE var = rb_class_new(clsAbsAddr);
	fill_ruby_absaddr(addr, var);
	return var;
}

static void absaddr_to_c( VALUE addr, opdis_abs_addr_t * dest ) {
	reg_to_c( rb_iv_get(expr, IVAR(ABS_ADDR_ATTR_SEG)), &dest->segment );
	dest->offset = NUM2OFF(rb_iv_get(expr, IVAR(ABS_ADDR_ATTR_OFF)));
}
 
/* ---------------------------------------------------------------------- */
/* Absolute Address Operand */

static VALUE clsAbsAddrOp;

static VALUE absaddr_op_from_c( opdis_abs_addr_t * addr ) {
	VALUE var = rb_class_new(clsAbsAddrOp);
	fill_ruby_absaddr(addr, var);
	return var;
}

static void init_abs_addr_attributes( VALUE class ) {
	rb_define_attr(class, ABS_ADDR_ATTR_SEG, 1, 1);
	rb_define_attr(class, ABS_ADDR_ATTR_OFF, 1, 1);
}

static void cls_absaddr_to_s( VALUE instance ) {
	VALUE str;
	// TODO: replace this with hex
	VALUE off = rb_any_to_s(rb_iv_get(instance, IVAR(ABS_ADDR_ATTR_OFF)));
	VALUE seg = rb_iv_get(instance, IVAR(ABS_ADDR_ATTR_SEG));

	if ( seg == Qnil ) {
		return off;
	}

	str = rb_str_dup(rb_iv_get(seg, IVAR(GEN_ATTR_ASCII)));
	rb_str_buf_cat_ascii(str, ":");
	rb_str_concat(str, off);
	return str;
}

static void init_absaddr_class( VALUE modOpdis ) {
	/* Absolute Address */
	clsAbsAddr = rb_define_class_under(modOpdis, "AbsoluteAddress", 
					   rb_cObject);
	rb_define_method(clsAbsAddr, "to_s", cls_absaddr_to_s, 0);

	init_abs_addr_attributes( clsAbsAddr );

	/* Absolute Address Operand */
	clsAbsAddrOp = rb_define_class_under(modOpdis, "AbsoluteAddressOperand",
					     clsOp);
	rb_define_method(clsAbsAddrOp, "to_s", cls_absaddr_to_s, 0);
	init_abs_addr_attributes( clsAbsAddrOp );
}

/* ---------------------------------------------------------------------- */
/* Address Expression Class */

static VALUE clsAddrExp;

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

static enum opdis_addr_expr_shift_t get_expr_shift( const char * str ) {
	if (! strcmp( ADDR_EXP_SHIFT_LSL, str ) ) {
		return opdis_addr_expr_lsl;
	} else if (! strcmp( ADDR_EXP_SHIFT_LSR, str ) ) {
		return opdis_addr_expr_lsr;
	} else if (! strcmp( ADDR_EXP_SHIFT_ROR, str ) ) {
		return opdis_addr_expr_ror;
	} else if (! strcmp( ADDR_EXP_SHIFT_RRX, str ) ) {
		return opdis_addr_expr_rrx;
	}

	return opdis_addr_expr_asl;
}

static const char * get_expr_shift_str( enum opdis_addr_expr_shift_t val ){
	const char * str = ADDR_EXP_SHIFT_ASL;

	switch (val) {
		case opdis_addr_expr_lsl: str = ADDR_EXP_SHIFT_LSL; break;
		case opdis_addr_expr_lsr: str = ADDR_EXP_SHIFT_LSR; break;
		case opdis_addr_expr_asl: str = ADDR_EXP_SHIFT_ASL; break;
		case opdis_addr_expr_ror: str = ADDR_EXP_SHIFT_ROR; break;
		case opdis_addr_expr_rrx: str = ADDR_EXP_SHIFT_RRX; break;
	}

	return str;
}


static void fill_ruby_addrexpr( opdis_addr_expr_t * expr, VALUE dest ) {
	
	if ( expr->elements & opdis_addr_expr_base ) {
		rb_iv_set(dest, IVAR(ADDR_EXP_ATTR_BASE), 
			  reg_from_c(&expr->base) );
	}

	if ( expr->elements & opdis_addr_expr_index ) {
		rb_iv_set(dest, IVAR(ADDR_EXP_ATTR_INDEX), 
			  reg_from_c(&expr->index) );
	}

	// TODO
	if ( expr->elements & opdis_addr_expr_disp ) {
		if ( expr->elements & opdis_addr_expr_disp_u ) {
			// fill disp w/ u
		} else if ( expr->elements & opdis_addr_expr_disp_s ) {
			// fill disp w/ s
		} else if ( expr->elements & opdis_addr_expr_disp_abs) {
			// fill disp w/ abs?
		} else {
			// invalid?
		} 
	}

	rb_iv_set( dest, IVAR(ADDR_EXP_ATTR_SHIFT), 
		   rb_str_new_cstr(get_expr_shift_str(expr->shift)) );

	return var;
}

static VALUE addrexpr_from_c( opdis_addr_expr_t * expr ) {
	VALUE var = rb_class_new(clsAddrExp);
	return fill_ruby_addrexpr( expr, var );
}

static void addrexpr_to_c( VALUE expr, opdis_addr_expr_t * dest ) {
	VALUE val;
	val = rb_iv_get(expr, IVAR(ADDR_EXP_ATTR_BASE));
	if ( val != Qnil ) {
		dest->elements |= opdis_addr_expr_base;
		reg_to_c( val, &dest->base );
	}

	val = rb_iv_get(expr, IVAR(ADDR_EXP_ATTR_INDEX));
	if ( val != Qnil ) {
		dest->elements |= opdis_addr_expr_index;
		reg_to_c( val, &dest->index );
	}

	val = rb_iv_get(expr, IVAR(ADDR_EXP_ATTR_SCALE));
	dest->scale = (val == Qnil) ? 1 : NUM2INT(val);

	val = rb_iv_get(expr, IVAR(ADDR_EXP_ATTR_SHIFT));
	dest->shift = (val == Qnil ) ? opdis_addr_expr_asl :
				       get_expr_shift(StringValueCStr(val));

	val = rb_iv_get(expr, IVAR(ADDR_EXP_ATTR_DISP));
	if ( val != Qnil ) {
		dest->elements |= opdis_addr_expr_disp;
		// if abs addr...
		// else if 32 or 64...
	}
}

static VALUE cls_addrexpr_to_s( VALUE instance ) {
	VALUE val, str;
	int needs_plus = 0;

	str = rb_string_new_cstr("");
	val = rb_iv_get(instance, IVAR(ADDR_EXP_ATTR_BASE));
	if ( val != Qnil ) {
		rb_str_concat(str, rb_iv_get(val, IVAR(GEN_ATTR_ASCII)));
		needs_plus = 1;
	}

	val = rb_iv_get(instance, IVAR(ADDR_EXP_ATTR_INDEX));
	if ( val != Qnil ) {
		VALUE v;
		if ( needs_plus ) {
			rb_str_buf_cat_ascii(str, "+");
			needs_plus = 0;
		}

		/* subexpression start */
		rb_str_buf_cat_ascii(str, "(");

		/* index register */
		rb_str_concat(str, rb_iv_get(val, IVAR(GEN_ATTR_ASCII)));

		/* scale operation */
		v = rb_iv_get(instance, IVAR(ADDR_EXP_ATTR_SHIFT));
		if ( v == Qnil ) {
			rb_str_buf_cat_ascii(str, "*");
		} else {
			rb_str_concat(str, v);
		}

		/* scale value */
		v = rb_iv_get(instance, IVAR(ADDR_EXP_ATTR_SCALE));
		if ( v == Qnil ) {
			rb_str_buf_cat_ascii(str, "1");
		} else {
			rb_str_concat(str, rb_any_to_s(v));
		}

		/* subexpression end */
		rb_str_buf_cat_ascii(str, ")");
		needs_plus = 1;
	}

	val = rb_iv_get(instance, IVAR(ADDR_EXP_ATTR_DISP));
	if ( val != Qnil ) {
		if ( needs_plus ) {
			rb_str_buf_cat_ascii(str, "+");
		}
		// TODO: hex str
		rb_str_concat(str, rb_any_to_s(val));
	}

	return str
}

static void init_addrexp_class( VALUE modOpdis ) {
	clsAddrExp = rb_define_class_under(modOpdis, "AddressExpression", 
					   clsOp);
	rb_singleton_method(clsAddrExp, "initialize", cls_addrexpr_init, 1);
	rb_define_method(clsAddrExp, "to_s", cls_addrexpr_to_s, 0);

	/* read-write attributes */
	rb_define_attr(clsAddrExp, ADDR_EXP_ATTR_SHIFT, 1, 1);
	rb_define_attr(clsAddrExp, ADDR_EXP_ATTR_SCALE, 1, 1);
	rb_define_attr(clsAddrExp, ADDR_EXP_ATTR_INDEX, 1, 1);
	rb_define_attr(clsAddrExp, ADDR_EXP_ATTR_BASE, 1, 1);
	rb_define_attr(clsAddrExp, ADDR_EXP_ATTR_DISP, 1, 1);

	/* constants */
	rb_define_const(clsAddrExp, ADDR_EXP_SHIFT_LSL_NAME,
			rb_str_new_cstr(ADDR_EXP_SHIFT_LSL));
	rb_define_const(clsAddrExp, ADDR_EXP_SHIFT_LSR_NAME,
			rb_str_new_cstr(ADDR_EXP_SHIFT_LSR));
	rb_define_const(clsAddrExp, ADDR_EXP_SHIFT_ASL_NAME,
			rb_str_new_cstr(ADDR_EXP_SHIFT_ASL));
	rb_define_const(clsAddrExp, ADDR_EXP_SHIFT_ROR_NAME,
			rb_str_new_cstr(ADDR_EXP_SHIFT_ROR));
	rb_define_const(clsAddrExp, ADDR_EXP_SHIFT_RRX_NAME,
			rb_str_new_cstr(ADDR_EXP_SHIFT_RRX));
}
 
/* ---------------------------------------------------------------------- */
/* Immediate Operand */

static VALUE clsImmOp;

#define IMM_ATTR_VAL "value"
#define IMM_ATTR_SIGNED "signed"
#define IMM_ATTR_UNSIGNED "unsigned"
#define IMM_ATTR_VMA "vma"

static VALUE cls_imm_init(VALUE instance, VALUE hash) {

	// TODO
	return instance;
}

static VALUE imm_from_c( opdis_op_t * op ) {
	VALUE var = rb_class_new(clsImmOp);

	rb_iv_set(var, IVAR(IMM_ATTR_VMA), op->value.immediate.vma );
	rb_iv_set(var, IVAR(IMM_ATTR_SIGNED), op->value.immediate.s );
	rb_iv_set(var, IVAR(IMM_ATTR_UNSIGNED), op->value.immediate.u );
	rb_iv_set(var, IVAR(IMM_ATTR_VAL), 
		  (op->flags & opdis_op_flag_signed) ?
			op->value.immediate.s : op->value.immediate.u );
	return var;
}

static void init_imm_class( VALUE modOpdis ) {
	clsAddrExp = rb_define_class_under(modOpdis, "Immediate", 
					   clsOp);
	rb_singleton_method(clsAddrExp, "initialize", cls_imm_init, 1);

	rb_define_attr(clsImmOp, IMM_ATTR_VAL, 1, 1);
	rb_define_attr(clsImmOp, IMM_ATTR_VMA, 1, 1);
	rb_define_attr(clsImmOp, IMM_ATTR_SIGNED, 1, 1);
	rb_define_attr(clsImmOp, IMM_ATTR_UNSIGNED, 1, 1);
}

/* ---------------------------------------------------------------------- */
/* Operand Factory */

static void set_op_flags( VALUE instance, enum opdis_op_flag_t val ) {
	VALUE flags = rb_iv_get(instance, IVAR(OP_ATTR_FLAGS) );
	if ( val & opdis_op_flag_r ) {
		rb_arry_push(status, rb_str_new_cstr(OP_FLAG_R) );
	}
	if ( val & opdis_op_flag_w ) {
		rb_arry_push(status, rb_str_new_cstr(OP_FLAG_W) );
	}
	if ( val & opdis_op_flag_x ) {
		rb_arry_push(status, rb_str_new_cstr(OP_FLAG_X) );
	}
	if ( val & opdis_op_flag_signed ) {
		rb_arry_push(status, rb_str_new_cstr(OP_FLAG_SIGNED) );
	}
	if ( val & opdis_op_flag_address ) {
		rb_arry_push(status, rb_str_new_cstr(OP_FLAG_ADDR) );
	}
	if ( val & opdis_op_flag_indirect ) {
		rb_arry_push(status, rb_str_new_cstr(OP_FLAG_IND) );
	}
}

static VALUE op_from_c( opdis_op_t * op ) {
	VALUE dest;
	switch (op->category) {
		case opdis_op_cat_register:
			dest = reg_from_c(&op->value.reg); break;
		case opdis_op_cat_immediate:
			dest = imm_from_c(op); break;
		case opdis_op_cat_absolute:
			dest = absaddr_from_c(&op->value.abs); break;
		case opdis_op_cat_expr:
			dest = addrexpr_from_c(&op->value.expr); break;
		case opdis_op_cat_unknown:
			dest = rb_class_new(clsOp);
		default;
	}

	rb_iv_set(dest, IVAR(GEN_ATTR_ASCII), rb_str_new_cstr(op->ascii));
	rb_iv_set(dest, IVAR(OP_ATTR_DATA_SZ), 
		  UINT2NUM((unsigned int) op->data_size));
	set_op_flags( dest, op->flags );

	return var;
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
	// TODO: off, vma, size, bytes
	/* set instance variables */
	//rb_iv_set(instance, IVAR(SYM_ATTR_NAME), rb_str_new_cstr(info.name) );

	rb_iv_set(instance, IVAR(INSN_ATTR_OPERANDS), rb_ary_new() );
	rb_iv_set(instance, IVAR(INSN_ATTR_STATUS), rb_ary_new() );

	return instance;
}

static void set_attr_if_alias( VALUE instance, const char * name, int idx, 
				opdis_op_t * a, opdis_op_t * b ) {
	if ( a && b && a == b ) {
		rb_iv_set(instance, name, INT2NUM(idx) );
	}
}

static void set_insn_status( VALUE instance, enum opdis_decode_t val ) {
	VALUE status = rb_iv_get(instance, IVAR(INSN_ATTR_STATUS) );
	if ( val & opdis_decode_invalid ) {
		rb_arry_push(status, rb_str_new_cstr( INSN_STATUS_INVALID) );
	}
	if ( val & opdis_decode_basic ) {
		rb_arry_push(status, rb_str_new_cstr( INSN_STATUS_BASIC) );
	}
	if ( val & opdis_decode_mnem ) {
		rb_arry_push(status, rb_str_new_cstr( INSN_STATUS_MNEM) );
	}
	if ( val & opdis_decode_ops ) {
		rb_arry_push(status, rb_str_new_cstr( INSN_STATUS_OPS) );
	}
	if ( val & opdis_decode_mnem_flags ) {
		rb_arry_push(status, rb_str_new_cstr( INSN_STATUS_MNEM_FLG) );
	}
	if ( val & opdis_decode_op_flags ) {
		rb_arry_push(status, rb_str_new_cstr( INSN_STATUS_OP_FLG) );
	}
}

static void fill_ruby_insn( opdis_insn_t * insn, VALUE dest ) {
	char buf[128];
	VALUE ops = rb_iv_get(dest, IVAR(INSN_ATTR_OPERANDS) );

	set_insn_status( dest, insn->status );

	rb_iv_set(dest, IVAR(GEN_ATTR_ASCII), rb_str_new_cstr(insn->ascii));

	rb_iv_set(dest, IVAR(INSN_ATTR_OFFSET), OFFT2NUM(insn->offset));
	rb_iv_set(dest, IVAR(INSN_ATTR_VMA), OFFT2NUM(insn->vma));
	rb_iv_set(dest, IVAR(INSN_ATTR_SIZE), UINT2NUM(insn->size));
	rb_iv_set(dest, IVAR(INSN_ATTR_BYTES), 
		  rb_str_new(insn->bytes, insn->size ));

	rb_iv_set(dest, IVAR(INSN_ATTR_PREFIXES), 
		  rb_str_new_cstr(insn->prefixes));
	rb_iv_set(dest, IVAR(INSN_ATTR_MNEMONIC), 
		  rb_str_new_cstr(insn->mnemonic));
	rb_iv_set(dest, IVAR(INSN_ATTR_COMMENT), 
		  rb_str_new_cstr(insn->comment));

	opdis_insn_cat_str( insn, buf, 128 );
	rb_iv_set(dest, IVAR(INSN_ATTR_CATEGORY), rb_str_new_cstr(buf));
	opdis_insn_isa_str( insn, buf, 128 );
	rb_iv_set(dest, IVAR(INSN_ATTR_ISA), rb_str_new_cstr(buf));

	opdis_insn_flags_str( insn, buf, 128 );
	rb_iv_set(dest, IVAR(INSN_ATTR_FLAGS), rb_str_new_cstr(buf));

	rb_array_clear(ops);
	for ( i=0; i < insn->num_operands; i++ ) {
		set_attr_if_alias( dest, IVAR(INSN_ATTR_TGT_IDX), i,
				   &insn->operands[i], insn->target );
		set_attr_if_alias( dest, IVAR(INSN_ATTR_DEST_IDX), i,
				   &insn->operands[i], insn->dest );
		set_attr_if_alias( dest, IVAR(INSN_ATTR_SRC_IDX), i,
				   &insn->operands[i], insn->src );
				   
		rb_arry_push( ops, op_from_c(&insn->operands[i]) );
	}
}

static VALUE insn_from_c( opdis_insn_t * insn ) {
	VALUE var = rb_class_new(clsInsn);
	fill_ruby_insn( insn, var );
	return var;
}

static void insn_to_c( VALUE insn, opdis_insn_t * dest ) {
	// TODO
}

static VALUE get_aliased_operand( VALUE instance, const char * alias ) {
	VALUE idx = rb_iv_get(instance, alias);
	VALUE ops = rb_iv_get(instance, IVAR(INSN_ATTR_OPERANDS) );
	return (idx == Qnil) ? idx : rb_array_entry(ops, NUM2LONG(idx));
}

static VALUE cls_insn_tgt( VALUE instance ) {
	return get_aliased_operand(instance, IVAR(INSN_ATTR_TGT_IDX) );
}

static VALUE cls_insn_dest( VALUE instance ) {
	return get_aliased_operand(instance, IVAR(INSN_ATTR_DEST_IDX) );
}

static VALUE cls_insn_src( VALUE instance ) {
	return get_aliased_operand(instance, IVAR(INSN_ATTR_SRC_IDX) );
}

static void define_insn_constants() {
	rb_define_const(clsInsn, INSN_STATUS_INVALID_NAME,
			rb_str_new_cstr(INSN_STATUS_INVALID));
	rb_define_const(clsInsn, INSN_STATUS_BASIC_NAME,
			rb_str_new_cstr(INSN_STATUS_BASIC));
	rb_define_const(clsInsn, INSN_STATUS_MNEM_NAME,
			rb_str_new_cstr(INSN_STATUS_MNEM));
	rb_define_const(clsInsn, INSN_STATUS_OPS_NAME,
			rb_str_new_cstr(INSN_STATUS_OPS));
	rb_define_const(clsInsn, INSN_STATUS_MNEM_FLG_NAME,
			rb_str_new_cstr(INSN_STATUS_MNEM_FLG));
	rb_define_const(clsInsn, INSN_STATUS_OP_FLG_NAME,
			rb_str_new_cstr(INSN_STATUS_OP_FLG));

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

static void define_insn_attributes() {

	/* read-write attributes */
	rb_define_attr(clsInsn, INSN_ATTR_STATUS, 1, 1);
	rb_define_attr(clsInsn, INSN_ATTR_PREFIXES, 1, 1);
	rb_define_attr(clsInsn, INSN_ATTR_MNEMONIC, 1, 1);
	rb_define_attr(clsInsn, INSN_ATTR_CATEGORY, 1, 1);
	rb_define_attr(clsInsn, INSN_ATTR_ISA, 1, 1);
	rb_define_attr(clsInsn, INSN_ATTR_FLAGS, 1, 1);
	rb_define_attr(clsInsn, INSN_ATTR_COMMENT, 1, 1);
	rb_define_attr(clsInsn, INSN_ATTR_OPERANDS, 1, 1);

	/* private attributes */
	// TODO : make private
	rb_define_attr(clsInsn, GEN_ATTR_ASCII, 1, 1);
	rb_define_attr(clsInsn, INSN_ATTR_TGT_IDX, 1, 1);
	rb_define_attr(clsInsn, INSN_ATTR_DEST_IDX, 1, 1);
	rb_define_attr(clsInsn, INSN_ATTR_SRC_IDX, 1, 1);

	/* read-only attributes */
	rb_define_attr(clsInsn, INSN_ATTR_OFFSET, 1, 0);
	rb_define_attr(clsInsn, INSN_ATTR_VMA, 1, 0);
	rb_define_attr(clsInsn, INSN_ATTR_SIZE, 1, 0);
	rb_define_attr(clsInsn, INSN_ATTR_BYTES, 1, 0);

	/* getters */
	rb_define_method(clsInsn, INSN_ATTR_TGT, cls_insn_tgt, 0);
	rb_define_method(clsInsn, INSN_ATTR_DEST, cls_insn_dest, 0);
	rb_define_method(clsInsn, INSN_ATTR_SRC, cls_insn_src, 0);
}

static void init_insn_class( VALUE modOpdis ) {
	clsInsn = rb_define_class_under(modOpdis, "Instruction", rb_cObject);
	rb_define_method(clsInsn, "initialize", cls_insn_new, 1);
	rb_define_method(clsInsn, "to_s", cls_generic_to_s, 0);
	// TODO:
	// is_branch?
	// fallthrough?

	define_insn_attributes();
	define_insn_constants();
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

	/* 1. get items from opdis_insn_buf_t */
	ary = rb_ary_new();
	for ( i = 0; i < in->item_count; i++ ) {
		rb_ary_push( ary, rb_str_new_cstr(in->items[i]) );
	}
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_ITEMS), ary );

	/* 2. get raw ASCII version of insn from libopcodes */
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_STR), 
		      rb_str_new_cstr(in->string) );

	/* 3. get instruction metadata set by libopcodes */
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
	opdis_insn_buf_t inbuf;
        opdis_byte_t * buf;
	opdis_off_t offset;
        opdis_vma_t vma;
	opdis_off_t length;
	opdis_insn_t * c_insn = ALLOC_FIXED_INSN();

	insn_to_c( insn, c_insn );

	/* get insn_buf_t from decoder instance; this saves some trouble */
	Data_Get_Struct( hash, opdis_insn_buf_t, inbuf );
	if (! inbuf ) {
		/* something went wrong: we weren't called from local_decoder */
		rb_raise( rb_eRuntimeError, "opdis_insn_buf_t not found" );
	}

	/* convert decoder arguments to C */
	var = rb_hash_lookup2(hash, str_to_sym(DECODER_MEMBER_VMA), Qfalse);
	vma = ( Qfalse != var ) ? NUM2UINT(var) : 0;

	var = rb_hash_lookup2(hash, str_to_sym(DECODER_MEMBER_OFF), Qfalse);
	offset = ( Qfalse != var ) ? NUM2UINT(var) : 0;

	var = rb_hash_lookup2(hash, str_to_sym(DECODER_MEMBER_LEN), Qfalse);
	length = ( Qfalse != var ) ? NUM2UINT(var) : 0;

	var = rb_hash_lookup2(hash, str_to_sym(DECODER_MEMBER_BUF), Qfalse);
	buf = ( Qfalse != var ) ? RSTRING_PTR(buf) : Qnil;

	/* invoke C decoder callback */
	rv = fn( inbuf, c_insn, buf, offset, vma, length );
	if ( rv ) {
		fill_ruby_insn( c_insn, insn );
	}

	opdis_insn_free(c_insn);

	return rv ? Qtrue : Qfalse;
}

static VALUE cls_decoder_decode( VALUE instance, VALUE insn, VALUE hash ) {
	invoke_builtin_decoder(opdis_default_decoder, insn, hash);
	return insn;
}

static void init_decoder_class( VALUE modOpdis ) {
	clsDecoder = rb_define_class_under(modOpdis, "InstructionDecoder", 
					   rb_cObject);
	rb_define_method(clsDecoder, DECODER_METHOD, cls_decoder_decode, 2);
}

/*      ----------------------------------------------------------------- */
/* 	X86 Decoder Class */

static VALUE clsX86Decoder, clsX86IntelDecoder;

static VALUE cls_x86decoder_decode( VALUE instance, VALUE insn, VALUE hash ) {
	invoke_builtin_decoder(opdis_x86_att_decoder, insn, hash);
	return insn;
}

static VALUE cls_x86inteldecoder_decode(VALUE instance, VALUE insn, VALUE hash){
	invoke_builtin_decoder(opdis_x86_intel_decoder, insn, hash);
	return insn;
}

static void init_x86decoder_class( VALUE modOpdis ) {
	/* AT&T Decoder */
	clsX86Decoder = rb_define_class_under(modOpdis, "X86Decoder", 
					      clsDecoder);
	rb_define_method(clsX86Decoder, DECODER_METHOD, 
			 cls_x86decoder_decode, 2);

	/* Intel Decoder */
	clsX86IntelDecoder = rb_define_class_under(modOpdis, "X86IntelDecoder", 
					           clsDecoder);
	rb_define_method(clsX86IntelDecoder, DECODER_METHOD, 
			 cls_x86inteldecoder_decode, 2);
}

/* ---------------------------------------------------------------------- */
/* VisitedAddr Class */

/* Handler class determines whether to continue ? */
static VALUE clsHandler;

#define HANDLER_METHOD "visited?"

static VALUE cls_handler_visited( VALUE instance, VALUE insn ) {
	int rv;
	opdist_t opdis;
	opdis_insn_t * c_insn = ALLOC_FIXED_INSN();

	insn_to_c( insn, c_insn );

	Data_Get_Struct(instance, opdis_t, opdis);
	if (! opdis ) {
		rb_raise(rb_eRuntimeError, "opdis_t not found in Handler");
	}

	rv = opdis_default_handler(c_insn, opdis);
	return rv ? Qtrue : Qfalse;
}

/* NOTE: this uses its own opdis_t with a visited_addr tree */
static VALUE cls_handler_new( VALUE class ) {
	VALUE argv[1] = {Qnil};
	opdist_t opdis = opdis_init();
	instance = Data_Wrap_Struct(class, NULL, opdis_term, opdis);
	rb_obj_call_init(instance, 0, argv);

	opdis->visited_addr = opdis_vma_tree_init();

	return init;
}

static void init_handler_class( VALUE modOpdis ) {

	clsHandler = rb_define_class_under(modOpdis, "VisitedAddressTracker", 
					   rb_cObject);
	rb_define_singleton_method(clsHandler, "new", cls_handler_new, 0);
	rb_define_method(clsHandler, HANDLER_METHOD, cls_handler_visited, 1);
}

/* ---------------------------------------------------------------------- */
/* Resolver Class */

static VALUE clsResolver;

#define RESOLVER_METHOD "resolve"

static VALUE cls_resolver_resolve( VALUE instance, VALUE insn ) {
	int rv;
	opdis_insn_t * c_insn = ALLOC_FIXED_INSN();

	insn_to_c( insn, c_insn );
	rv = opdis_default_resolver( c_insn, NULL );
	return INT2NUM(rv);
}

static void init_resolver_class( VALUE modOpdis ) {
	cls = rb_define_class_under(modOpdis, "AddressResolver", rb_cObject);
	rb_define_method(clsResolver, RESOLVER_METHOD, cls_resolver_resolve, 1);
}

/* ---------------------------------------------------------------------- */
/* Disasm Output Class */
/* This is basically a Hash of VMA => Instruction entries, with an @errors
 * attribute that gets filled with error messages from the disassembler */

static VALUE clsOutput;

#define OUT_ATTR_ERRORS "errors"
#define OUT_METHOD_CONTAIN "containing"

/* insn containing vma */
static VALUE cls_output_contain( VALUE instance, VALUE vma ) {
	// TODO
	// return insn containing vma
}

static VALUE cls_output_new( VALUE class ) {
	VALUE instance = rb_class_new(clsOutput);
	rb_iv_set(instance, IVAR(OUT_ATTR_ERRORS), rb_arry_new() );
	return instance;
}

static void init_output_class( VALUE modOpdis ) {
	clsOutput = rb_define_class_under(modOpdis, "Disassembly", rb_cHash);
	rb_define_singleton_method(clsOutput, "new", cls_output_new, 0);

	/* read-only attribute for error list */
	rb_define_attr(clsOutput, OUT_ATTR_ERRORS, 1, 0);

	/* setters */
	rb_define_method(clsOutput, OUT_METHOD_CONTAIN, cls_output_contain, 1);
}


/* ---------------------------------------------------------------------- */
/* Disassembler Architectures (internal) */
struct arch_def { 
	const char * name; 
	enum bfd_architecture arch;	/* BFD CPU architecture */
	unsigned long default_mach;	/* BFD machine type */
	disassembler_ftype fn;		/* libopcodes print_insn callback */
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

#define DIS_SYNTAX_ATT "att"
#define DIS_SYNTAX_INTEL "intel"

#define DIS_CONST_SYNTAXES "syntaxes"

/* list all recognized syntaxes */
static VALUE cls_disasm_syntaxes( VALUE class ) {
	VALUE ary = rb_ary_new();
	rb_ary_push( ary, rb_str_new_cstr(DIS_SYNTAX_ATT) );
	rb_ary_push( ary, rb_str_new_cstr(DIS_SYNTAX_INTEL) );
	return ary;
}

/* list all recognized architectures */
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

/* list all recognized disassembly algorithms */
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
	VALUE insn = insn_from_c(out);

	/* build hash containing insn info */
	fill_decoder_hash( &hash, in, buf, offset, vma, length );

	/* invoke decode method in Decoder object */
	VALUE var = rb_funcall(obj, rb_intern(DECODER_METHOD), 2, insn, hash);
	return (Qfalse == var || Qnil == var) ? 0 : 1;
}

static VALUE cls_disasm_set_decoder(VALUE instance, VALUE obj) {
	opdist_t  opdis;
	Data_Get_Struct(instance, opdis_t, opdis);

	/* 'nil' causes opdis to revert to default decoder */
	if ( Qnil == obj ) {
		opdis_set_decoder( opdis, opdis_default_decoder, NULL );
		return Qtrue;
	}

	/* objects without a 'decode' method cannot be decoders */
	if (! rb_respond_to(object, rb_intern(DECODER_METHOD)) ) {
		return Qfalse;
	}
	
	opdis_set_decoder( opdis, local_decoder, obj );
	rb_iv_set(instance, IVAR(DIS_ATTR_DECODER), obj );

	return Qtrue;
}

/* local insn handler object: this invokes the visited? method in the handler
 * object provided by the user. */
static int local_handler( const opdis_insn_t * i, void * arg ) {
	VALUE obj = (VALUE) arg;
	VALUE insn = insn_from_c(i);

	/* invoke visited? method in Handler object */
	VALUE var = rb_funcall(obj, rb_intern(HANDLER_METHOD), 1, insn);

	/* True means already visited, so continue = 0 */
	return (Qtrue == var) ? 0 : 1;
}

static VALUE cls_disasm_set_handler(VALUE instance, VALUE obj) {
	opdist_t  opdis;
	Data_Get_Struct(instance, opdis_t, opdis);

	/* nil causes opdis to revert to default decoder */
	if ( Qnil == obj ) {
		opdis_set_handler( opdis, opdis_default_handler, opdis );
		return Qtrue;
	}

	/* objects without a visited? method cannot be handlers */
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
	VALUE insn = insn_from_c(i);

	/* invoke resolve method in Resolver object */
	VALUE vma = rb_funcall(obj, rb_intern(RESOLVER_METHOD), 1, insn);

	return (Qnil == vma) ? OPDIS_INVALID_ADDR : (opdis_vma_t) NUM2UINT(vma);
}

static VALUE cls_disasm_set_resolver(VALUE instance, VALUE obj) {
	opdist_t  opdis;
	Data_Get_Struct(instance, opdis_t, opdis);

	/* nil causes opdis to revert to default resolver */
	if ( Qnil == obj ) {
		opdis_set_resolver( opdis, opdis_default_resolver, NULL );
		return Qtrue;
	}

	/* objects without a resolve method cannot be Resolvers */
	if (! rb_respond_to(object, rb_intern(RESOLVER_METHOD)) ) {
		return Qfalse;
	}
	
	opdis_set_resolver( opdis, local_resolver, obj );
	rb_iv_set(instance, IVAR(DIS_ATTR_RESOLVER), obj );

	return Qtrue;
}

static VALUE cls_disasm_get_debug(VALUE instance) {
	opdist_t  opdis;
	Data_Get_Struct(instance, opdis_t, opdis);
	return opdis->debug ? Qtrue : Qfalse;
}

static VALUE cls_disasm_set_debug(VALUE instance, VALUE enabled) {
	opdist_t  opdis;
	Data_Get_Struct(instance, opdis_t, opdis);
	opdis->debug = (enabled == Qtrue) ? 1 : 0;
	return Qtrue;
}

static VALUE cls_disasm_get_syntax(VALUE instance) {
	opdist_t  opdis;
	VALUE str;

	Data_Get_Struct(instance, opdis_t, opdis);

	if ( opdis->disassembler == print_insn_i386_intel ) {
		str = rb_str_new_cstr(DIS_SYNTAX_INTEL);
	} else {
		str = rb_str_new_cstr(DIS_SYNTAX_ATT);
	}

	return str;
}

static VALUE cls_disasm_set_syntax(VALUE instance, VALUE syntax) {
	opdist_t  opdis;
	enum opdis_x86_syntax_t syn;
	const char * str = StringValueCStr(rb_any_to_s(syntax));

	if (! strcmp(syntax, DIS_SYNTAX_INTEL) ) {
		syn = opdis_x86_syntax_intel;
	} else if (! strcmp(syntax, DIS_SYNTAX_ATT) ) {
		syn = opdis_x86_syntax_att;
	} else {
		rb_raise(rb_eArgError, "Syntax must be 'intel' or 'att'");
	}

	Data_Get_Struct(instance, opdis_t, opdis);
	opdis_set_x86_syntax(opdis, syn);

	return Qtrue;
}

static VALUE cls_disasm_get_arch(VALUE instance) {
	opdist_t  opdis;
	int i;
	VALUE str;

	Data_Get_Struct(instance, opdis_t, opdis);

	for ( i = 0; i < num_defs; i++ ) {
		struct disasm_def *def = &disasm_definitions[i];
		if ( def->arch == opdis->config.arch &&
		     def->mach == opdis->config.mach ) {
			return rb_str_new_cstr(def->name);
		}
	}

	return rb_str_new_cstr("unknown");
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

static VALUE cls_disasm_set_opts(VALUE instance) {
	opdist_t  opdis;
	Data_Get_Struct(instance, opdis_t, opdis);
	return rb_str_new_cstr(opdis->config.disassembler_options);
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

	/* opdis callbacks */
	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_RESOLVER), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_resolver(instance, var);

	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_HANDLER), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_handler(instance, var);

	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_DECODER), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_decoder(instance, var);

	/* opdis settings */
	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_SYNTAX), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_syntax(instance, var);

	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_DEBUG), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_debug(instance, var);

	/* libopcodes settings */
	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_OPTIONS), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_opts(instance, var);

	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_ARCH), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_arch(VALUE instance, VALUE arch);
}

/* local display handler for blocks: this yields the current insn to arg */
static void local_block_display( const opdis_insn_t * i, void * arg ) {
	VALUE block = (VALUE) arg;
	VALUE insn = insn_from_c(i);

	rb_funcall(block, rb_intern("call"), 1, insn);
}

/* local display handler: this adds instructions to a Disassembly object */
static void local_display( const opdis_insn_t * i, void * arg ) {
	VALUE output = (VALUE) arg;
	VALUE insn = insn_from_c(i);

	rb_hash_aset( output, INT2NUM(i->vma), insn );
}

/* local error handler: this appends errors to a ruby array in arg */
static void local_error( enum opdis_error_t error, const char * msg,
                         void * arg ) {
	const char * type;
	char buf[128];
	VALUE errors = (VALUE) arg;

	switch (error) {
		case opdis_error_bounds: type = DIS_ERR_BOUNDS; break;
		case opdis_error_invalid_insn: type = DIS_ERR_INVALID; break;
		case opdis_error_decode_insn: type = DIS_ERR_DECODE; break;
		case opdis_error_bfd: type = DIS_ERR_BFD; break;
		case opdis_error_max_items: type = DIS_ERR_MAX; break;
		case opdis_error_unknown: 
		default: type = DIS_ERR_UNK; break;
	}

	snprintf( buf, 127-1, "%s: $s", type, msg );
	str = rb_str_new_cstr(buf);

	/* append error message to error list */
	rb_funcall( errors, rb_intern("<<"), 1, rb_str_new_cstr(buf) );
}

static void config_buf_from_args( opdis_buf_t * buf, VALUE hash ) {
	VALUE var;

	/* buffer vma */
	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_VMA), Qfalse);
	if ( Qfalse != var && Qnil != var ) buf->vma = NUM2UINT(var);

	// TODO: other options? 
}

static opdis_buf_t opdis_buf_for_target( VALUE tgt, VALUE hash ) {
	unsigned char * buf = NULL;
	unsigned int buf_len = 0;
	opdis_buf_t obuf;

	/* String object containing bytes */
	if ( Qtrue == rb_obj_is_kind_of( tgt, rb_cString ) ) {
		buf = (unsigned char*) RSTRING_PTR(tgt);
		buf_len = RSTRING_LEN(tgt);

	/* Array object containing bytes */
	} else if ( Qtrue == rb_obj_is_kind_of( tgt, rb_cArray ) ) {
		int i;
		unsigned char * sbuf;
		buf_len = RARRAY_LEN(tgt);
		sbuf = alloca(buf_len);
		for( i=0; i < buf_len; i++ ) {
			VALUE val = rb_ary_entry( tgt, i );
			sbuf[i] = (unsigned char *) NUM2UINT(val);
		}

		buf = sbuf;

	/* IO object containing bytes */
	} else if ( Qtrue == rb_obj_is_kind_of( tgt, rb_cIO ) ) {
		VALUE str = rb_funcall( tgt, rb_intern("read"), 0 );
		buf = (unsigned char*) RSTRING_PTR(str);
		buf_len = RSTRING_LEN(str);

	} else {
		rb_raise(rb_eArgError, "Buffer must be a String, IO or Array");
	}

	if (! buf || ! buf_len ) {
		rb_raise(rb_eArgError, "Cannot disassemble empty buffer");
	}

	obuf = opdis_buf_alloc( buf_len, 0 );
	opdis_buf_fill( obuf, 0, buf, buf_len );

	/* apply target-specific args (vma, etc) */
	config_buf_from_args( buf, hash );

	return obuf;
}

struct OPDIS_TGT { bfd * abfd; asection * sec; asymbol * sym; opdis_buf_t buf };

static void load_target( opdist_t opdis, VALUE tgt, VALUE hash, 
			 struct OPDIS_TGT * out ) {

	/* Ruby Bfd::Target object */
	if ( Qnil != GET_BFD_CLASS(clsBfdTgt, BFD_TGT_PATH) &&
	     Qtrue == rb_obj_is_kind_of( tgt, clsBfdTgt ) ) {
		DataGetStruct(tgt, bfd, out->abfd );
	
	/* Ruby Bfd::Symbol object */
	} else if ( Qnil != GET_BFD_CLASS(clsBfdSym, BFD_SYM_PATH) &&
	     Qtrue == rb_obj_is_kind_of( tgt, clsBfdSym ) ) {
		asymbol * s;
		DataGetStruct(tgt, asymbol, out->sym );
		if ( out->sym ) {
			out->abfd = s->the_bfd;
		}

	/* Ruby Bfd::Section object */
	} else if ( Qnil != GET_BFD_CLASS(clsBfdSec, BFD_SEC_PATH) &&
	     Qtrue == rb_obj_is_kind_of( tgt, clsBfdSec ) ) {
		DataGetStruct(tgt, asection, out->sec );
		if ( out->sec ) {
			out->abfd = s->owner;
		}

	/* Other non-Bfd Ruby object */
	} else {
		out->buf = opdis_buf_for_target( tgt, hash );
	}

	/* Set arch, etc based on BFD info */
	if ( out->abfd ) {
		opdis_config_from_bfd( opdis, out->abfd );
	}
}

static void perform_disassembly( VALUE instance, opdist_t opdis, VALUE target,
				 VALUE hash ) {
	VALUE var;
	VALUE vma = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_VMA), INT2NUM(0));
	VALUE len = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_LEN), INT2NUM(0));
	const char * strategy = DIS_STRAT_LINEAR;
	struct OPDIS_TGT tgt = {0};

	/* load target based on its Ruby object type */
	load_target( opdis, target, hash, &tgt );

	/* apply general args (syntax, arch, etc), overriding Bfd config */
	cls_disasm_handle_args(instance, hash);

	/* get disassembly algorithm to use */
	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_STRATEGY), Qfalse);
	if ( Qfalse != var ) strategy = StringValueCStr(var);

	// TODO
	// rb_thread_schedule()
	// (in every callback?)

	/* Single instruction disassembly */
	if (! strcmp( strategy, DIS_STRAT_SINGLE ) ) {
		// TODO: alloc fixed insn
		opdis_insn_t * insn;

		if ( tgt.abfd ) {
			// not impl!
			//opdis_disasm_insn( opdis, tgt.abfd, vma, nsn );
		} else {
			opdis_disasm_insn( opdis, tgt.buf, vma, insn );
		}

	/* Linear disassembly */
	} else if (! strcmp( strategy, DIS_STRAT_LINEAR ) ) {
		if ( tgt.abfd ) {
		 	opdis_disasm_bfd_linear( opdis, tgt.bfd, vma, len );
		} else {
		 	opdis_disasm_linear( opdis, tgt.buf, vma, len );
		}

	/* Control Flow disassembly */
	} else if (! strcmp( strategy, DIS_STRAT_CFLOW ) ) {
		if ( tgt.abfd ) {
			opdis_disasm_bfd_cflow( opdis, tgt.abfd, vma );
		} else {
			opdis_disasm_cflow( opdis, tgt.buf, vma );
		}

	/* Control Flow disassembly of BFD symbol */
	} else if (! strcmp( strategy, DIS_STRAT_SYMBOL ) ) {
		if (! tgt.sym ) {
			rb_raise(rb_eArgError, "Bfd::Symbol required");
		}
		opdis_disasm_bfd_symbol( opdis, tgt.sym );

	/* Linear disassembly of BFD section */
	} else if (! strcmp( strategy, DIS_STRAT_SECTION ) ) {
		if (! tgt.sec ) {
			rb_raise(rb_eArgError, "Bfd::Section required");
		}
		opdis_disasm_bfd_section( opdis, tgt.sec );

	/* Control Flow disassembly of BFD entry point */
	} else if (! strcmp( strategy, DIS_STRAT_ENTRY ) ) {
		if (! tgt.abfd ) {
			rb_raise(rb_eArgError, "Bfd::Target required");
		}
		opdis_disasm_bfd_entry( opdis, tgt.abfd );
	} else {
		if ( tgt.buf ) {
			opdis_buf_free(tgt.buf);
		}
		rb_raise(rb_eArgError, "Unknown strategy '%s'", strategy);
	}

	if ( tgt.buf ) {
		opdis_buf_free(tgt.buf);
	}
}


/* Disassembler strategies produce blocks */
static VALUE cls_disasm_disassemble(VALUE instance, VALUE tgt, VALUE hash ) {
	VALUE output;
	opdist_t opdis;

	// TODO:  opdis_dupe in order to be threadsafe
	Data_Get_Struct(instance, opdis_t, opdis);

	/* yielding is easy */
	if ( rb_block_given_p() ) {
		// TODO: should errors raise an exception?
		opdis_set_display(opdis, local_block_display, rb_block_proc());
		return perform_disassembly( instance, opdis, tgt, hash );
	}

	/* ...otherwise we have to fill an output object */
	output = cls_output_new( clsOutput );

	opdis_set_display( opdis, local_display, output );
	opdis_set_error_reporter( opdis, local_error, 
				  rb_iv_get( output, IVAR(OUT_ATTR_ERRORS) ) );

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
	//TODO: is this viable? how does # of args get determined?
	//rb_obj_call_init(instance, 0, &Qnil);

	cls_disasm_handle_args(instance, hash);

	return instance;
}

static void define_disasm_constants() {
	/* Error types */
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

	/* Disassembly algorithms */
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

	/* Lists of symbolic constants */
	rb_define_singleton_method(clsDisasm, DIS_CONST_STRATEGIES, 
				   cls_disasm_strategies, 0);
	rb_define_singleton_method(clsDisasm, DIS_CONST_ARCHES, 
				   cls_disasm_architectures, 0);
	rb_define_singleton_method(clsDisasm, DIS_CONST_SYNTAXES, 
				   cls_disasm_syntaxes, 0);
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

	/* getters */
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
