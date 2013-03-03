/* Model.c
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <ruby.h>
#include "ruby_compat.h"

#include <opdis/model.h>

#include "Model.h"

static VALUE symToSym, symToS;
static VALUE clsInsn, clsOp, clsReg, clsAbsAddr;
static VALUE clsRegOp, clsAbsAddrOp, clsAddrExprOp, clsImmOp;

#define IVAR(attr) "@" attr
#define SETTER(attr) attr "="

/* TODO: remove if not needed 
static VALUE str_to_sym( const char * str ) {
	VALUE var = rb_str_new_cstr(str);
	return rb_funcall(var, symToSym, 0);
}
*/

#define ALLOC_FIXED_INSN opdis_alloc_fixed(128, 32, 16, 32)

/* ---------------------------------------------------------------------- */
/* Supported architectures */

/* ---------------------------------------------------------------------- */
/* Shared Methods */

static VALUE cls_generic_to_s( VALUE instance ) {
	return rb_iv_get(instance, IVAR(GEN_ATTR_ASCII) );
}

/* ---------------------------------------------------------------------- */
/* Operand Base Class */

static VALUE cls_op_init(VALUE instance) {
	rb_iv_set(instance, IVAR(INSN_ATTR_FLAGS), rb_ary_new() );
	return instance;
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
	rb_define_method(clsOp, "initialize", cls_op_init, 0);
	rb_define_method(clsOp, "to_s", cls_generic_to_s, 0);

	define_op_constants();

	/* read-write attributes */
	rb_define_attr(clsOp, OP_ATTR_FLAGS, 1, 1);
	rb_define_attr(clsOp, GEN_ATTR_ASCII, 1, 1);
	rb_define_attr(clsOp, OP_ATTR_DATA_SZ, 1, 1);
}

/* ---------------------------------------------------------------------- */
/* Register Class */

static void set_ruby_reg_flags( VALUE instance, enum opdis_reg_flag_t val ) {
	VALUE flags = rb_iv_get(instance, IVAR(REG_ATTR_FLAGS) );
	if ( val & opdis_reg_flag_gen ) {
		rb_ary_push(flags, rb_str_new_cstr( REG_FLAG_GEN) );
	}
	if ( val & opdis_reg_flag_fpu ) {
		rb_ary_push(flags, rb_str_new_cstr( REG_FLAG_FPU) );
	}
	if ( val & opdis_reg_flag_gpu ) {
		rb_ary_push(flags, rb_str_new_cstr( REG_FLAG_GPU) );
	}
	if ( val & opdis_reg_flag_simd ) {
		rb_ary_push(flags, rb_str_new_cstr( REG_FLAG_SIMD) );
	}
	if ( val & opdis_reg_flag_task ) {
		rb_ary_push(flags, rb_str_new_cstr( REG_FLAG_TASK) );
	}
	if ( val & opdis_reg_flag_mem ) {
		rb_ary_push(flags, rb_str_new_cstr( REG_FLAG_MEM) );
	}
	if ( val & opdis_reg_flag_debug ) {
		rb_ary_push(flags, rb_str_new_cstr( REG_FLAG_DBG) );
	}
	if ( val & opdis_reg_flag_pc ) {
		rb_ary_push(flags, rb_str_new_cstr( REG_FLAG_PC) );
	}
	if ( val & opdis_reg_flag_flags ) {
		rb_ary_push(flags, rb_str_new_cstr( REG_FLAG_CC) );
	}
	if ( val & opdis_reg_flag_stack ) {
		rb_ary_push(flags, rb_str_new_cstr( REG_FLAG_STACK) );
	}
	if ( val & opdis_reg_flag_frame ) {
		rb_ary_push(flags, rb_str_new_cstr( REG_FLAG_FRAME) );
	}
	if ( val & opdis_reg_flag_seg ) {
		rb_ary_push(flags, rb_str_new_cstr( REG_FLAG_SEG) );
	}
	if ( val & opdis_reg_flag_zero ) {
		rb_ary_push(flags, rb_str_new_cstr( REG_FLAG_Z) );
	}
	if ( val & opdis_reg_flag_argsin ) {
		rb_ary_push(flags, rb_str_new_cstr( REG_FLAG_IN) );
	}
	if ( val & opdis_reg_flag_argsout ) {
		rb_ary_push(flags, rb_str_new_cstr( REG_FLAG_OUT) );
	}
	if ( val & opdis_reg_flag_locals ) {
		rb_ary_push(flags, rb_str_new_cstr( REG_FLAG_LOCALS) );
	}
	if ( val & opdis_reg_flag_return ) {
		rb_ary_push(flags, rb_str_new_cstr( REG_FLAG_RET) );
	}
}

static void set_c_reg_flags( opdis_reg_t * dest, VALUE reg ) {
	int i;
	struct RArray * flags = RARRAY( rb_iv_get(reg, IVAR(REG_ATTR_FLAGS)) );
	dest->flags = opdis_reg_flag_unknown;

	for ( i=0; i < RARRAY_LEN(flags); i++ ) {
		VALUE val = RARRAY_PTR(flags)[i];
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
	VALUE args[1] = {Qnil};
	VALUE var = rb_class_new_instance(0, args, clsReg);
	if ( var != Qnil ) {
		fill_ruby_reg(reg, var);
	}
	return var;
}

static void reg_to_c( VALUE reg, opdis_reg_t * dest ) {
	VALUE val;

	val = rb_iv_get(reg, IVAR(REG_ATTR_ID));
	dest->id = (val == Qnil) ? 0 : (unsigned char) NUM2UINT(val);

	val = rb_iv_get(reg, IVAR(REG_ATTR_SIZE));
	dest->size = (val == Qnil) ? 0 : (unsigned char) NUM2UINT(val);

	val = rb_iv_get(reg, IVAR(GEN_ATTR_ASCII));
	if ( val != Qnil ) {
		strncpy( dest->ascii, StringValueCStr(val), OPDIS_REG_NAME_SZ );
	}

	set_c_reg_flags( dest, reg );
}


/* ---------------------------------------------------------------------- */
/* Register Operand */

static VALUE reg_op_from_c( opdis_reg_t * reg ) {
	VALUE args[1] = {Qnil};
	VALUE var = rb_class_new_instance(0, args, clsRegOp);
	if ( var != Qnil ) {
		fill_ruby_reg(reg, var);
	}
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

static VALUE cls_reg_op_init(VALUE instance) {
	cls_op_init(instance);
	return cls_reg_init(instance);
}

static void init_reg_class( VALUE modOpdis ) {
	/* Register */
	clsReg = rb_define_class_under(modOpdis, "Register", rb_cObject);

	rb_define_method(clsReg, "initialize", cls_reg_init, 0);
	rb_define_method(clsReg, "to_s", cls_generic_to_s, 0);
	rb_define_attr(clsReg, GEN_ATTR_ASCII, 1, 0);
	rb_define_alias(clsReg, REG_ATTR_NAME, GEN_ATTR_ASCII );

	define_reg_constants( clsReg );
	init_reg_attributes( clsReg );

	/* Register Operand */
	clsRegOp = rb_define_class_under(modOpdis, "RegisterOperand", clsOp);
	rb_define_method(clsRegOp, "initialize", cls_reg_op_init, 0);
	rb_define_method(clsRegOp, "to_s", cls_generic_to_s, 0);
	rb_define_alias(clsRegOp, REG_ATTR_NAME, GEN_ATTR_ASCII );

	define_reg_constants( clsRegOp );
	init_reg_attributes( clsRegOp );
}

/* ---------------------------------------------------------------------- */
/* Absolute Address Operand Class */

static void fill_ruby_absaddr( opdis_abs_addr_t * addr, VALUE dest ) {
	rb_iv_set(dest, IVAR(ABS_ADDR_ATTR_SEG), reg_from_c(&addr->segment) );
	rb_iv_set(dest, IVAR(ABS_ADDR_ATTR_OFF), addr->offset );
}

static VALUE absaddr_from_c( opdis_abs_addr_t * addr ) {
	VALUE args[1] = {Qnil};
	VALUE var = rb_class_new_instance(0, args, clsAbsAddr);
	if ( var != Qnil ) {
		fill_ruby_absaddr(addr, var);
	}
	return var;
}

static void absaddr_to_c( VALUE addr, opdis_abs_addr_t * dest ) {
	reg_to_c( rb_iv_get(addr, IVAR(ABS_ADDR_ATTR_SEG)), &dest->segment );
	dest->offset = NUM2OFFT(rb_iv_get(addr, IVAR(ABS_ADDR_ATTR_OFF)));
}
 
/* ---------------------------------------------------------------------- */
/* Absolute Address Operand */

static VALUE absaddr_op_from_c( opdis_abs_addr_t * addr ) {
	VALUE args[1] = {Qnil};
	VALUE var = rb_class_new_instance(0, args, clsAbsAddrOp);
	if ( var != Qnil ) {
		fill_ruby_absaddr(addr, var);
	}
	return var;
}

static void init_abs_addr_attributes( VALUE class ) {
	rb_define_attr(class, ABS_ADDR_ATTR_SEG, 1, 1);
	rb_define_attr(class, ABS_ADDR_ATTR_OFF, 1, 1);
}

static VALUE cls_absaddr_to_s( VALUE instance ) {
	VALUE str;
	VALUE off = rb_funcall( rb_iv_get(instance, IVAR(ABS_ADDR_ATTR_OFF)),
				symToS, 1, INT2NUM(16) );
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
	rb_define_attr(clsAbsAddr, GEN_ATTR_ASCII, 1, 0);

	init_abs_addr_attributes( clsAbsAddr );

	/* Absolute Address Operand */
	clsAbsAddrOp = rb_define_class_under(modOpdis, "AbsoluteAddressOperand",
					     clsOp);
	rb_define_method(clsAbsAddrOp, "to_s", cls_absaddr_to_s, 0);
	init_abs_addr_attributes( clsAbsAddrOp );
}

/* ---------------------------------------------------------------------- */
/* Address Expression Class */

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

	if ( expr->elements & opdis_addr_expr_disp ) {
		if ( expr->elements & opdis_addr_expr_disp_abs ) {
			rb_iv_set(dest, IVAR(ADDR_EXP_ATTR_DISP), 
				  absaddr_from_c(&expr->displacement.a));
		} else if ( expr->elements & opdis_addr_expr_disp_s ) {
			rb_iv_set(dest, IVAR(ADDR_EXP_ATTR_DISP), 
				  LL2NUM(expr->displacement.s));
		} else {
			rb_iv_set(dest, IVAR(ADDR_EXP_ATTR_DISP), 
				  ULL2NUM(expr->displacement.u));
		} 
	}

	rb_iv_set( dest, IVAR(ADDR_EXP_ATTR_SHIFT), 
		   rb_str_new_cstr(get_expr_shift_str(expr->shift)) );
}

static VALUE addrexpr_op_from_c( opdis_addr_expr_t * expr ) {
	VALUE args[1] = {Qnil};
	VALUE var = rb_class_new_instance(0, args, clsAddrExprOp);
	if ( var != Qnil ) {
		fill_ruby_addrexpr( expr, var );
	}
	return var;
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
		if ( Qtrue == rb_obj_is_kind_of( val, clsAbsAddr ) ) {
			dest->elements |= opdis_addr_expr_disp_abs;
			absaddr_to_c(val, &dest->displacement.a);
		} else {
			dest->displacement.u = NUM2ULL(val);
			// TODO: int rb_cmpint(VALUE, VALUE, VALUE)
			dest->elements |= opdis_addr_expr_disp_u;
			//dest->elements |= ( (is_signed) ? 
			//		opdis_addr_expr_disp_s :
			//		opdis_addr_expr_disp_u );
		}
	}
}

static VALUE cls_addrexpr_to_s( VALUE instance ) {
	VALUE val, str;
	int needs_plus = 0;

	str = rb_str_new_cstr("");
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
		rb_str_concat( str, rb_funcall(val, symToS, 1, INT2NUM(16)) );
	}

	return str;
}

static void init_addrexpr_class( VALUE modOpdis ) {
	clsAddrExprOp = rb_define_class_under(modOpdis, 
					"AddressExpressionOperand", clsOp);
	rb_define_method(clsAddrExprOp, "to_s", cls_addrexpr_to_s, 0);

	/* read-write attributes */
	rb_define_attr(clsAddrExprOp, ADDR_EXP_ATTR_SHIFT, 1, 1);
	rb_define_attr(clsAddrExprOp, ADDR_EXP_ATTR_SCALE, 1, 1);
	rb_define_attr(clsAddrExprOp, ADDR_EXP_ATTR_INDEX, 1, 1);
	rb_define_attr(clsAddrExprOp, ADDR_EXP_ATTR_BASE, 1, 1);
	rb_define_attr(clsAddrExprOp, ADDR_EXP_ATTR_DISP, 1, 1);

	/* constants */
	rb_define_const(clsAddrExprOp, ADDR_EXP_SHIFT_LSL_NAME,
			rb_str_new_cstr(ADDR_EXP_SHIFT_LSL));
	rb_define_const(clsAddrExprOp, ADDR_EXP_SHIFT_LSR_NAME,
			rb_str_new_cstr(ADDR_EXP_SHIFT_LSR));
	rb_define_const(clsAddrExprOp, ADDR_EXP_SHIFT_ASL_NAME,
			rb_str_new_cstr(ADDR_EXP_SHIFT_ASL));
	rb_define_const(clsAddrExprOp, ADDR_EXP_SHIFT_ROR_NAME,
			rb_str_new_cstr(ADDR_EXP_SHIFT_ROR));
	rb_define_const(clsAddrExprOp, ADDR_EXP_SHIFT_RRX_NAME,
			rb_str_new_cstr(ADDR_EXP_SHIFT_RRX));
}
 
/* ---------------------------------------------------------------------- */
/* Immediate Operand */

static VALUE cls_imm_to_s(VALUE instance) {
	return rb_any_to_s(rb_iv_get(instance, IVAR(IMM_ATTR_VAL)));
}

static uint64_t imm_to_c( VALUE imm ) {
	return NUM2ULL(rb_iv_get(imm, IVAR(IMM_ATTR_UNSIGNED)));
}

static VALUE imm_op_from_c( opdis_op_t * op ) {
	VALUE args[1] = {Qnil};
	VALUE var = rb_class_new_instance(0, args, clsImmOp);
	if ( var == Qnil ) {
		return var;
	}

	rb_iv_set(var, IVAR(IMM_ATTR_VMA), 
		  ULL2NUM(op->value.immediate.vma) );
	rb_iv_set(var, IVAR(IMM_ATTR_SIGNED), 
		  LL2NUM(op->value.immediate.s) );
	rb_iv_set(var, IVAR(IMM_ATTR_UNSIGNED), 
		  ULL2NUM(op->value.immediate.u) );
	rb_iv_set(var, IVAR(IMM_ATTR_VAL), 
		  (op->flags & opdis_op_flag_signed) ?
			LL2NUM(op->value.immediate.s) : 
			ULL2NUM(op->value.immediate.u) );
	return var;
}

static void init_imm_class( VALUE modOpdis ) {
	clsImmOp = rb_define_class_under(modOpdis, "ImmediateOperand", clsOp);
	rb_define_singleton_method(clsImmOp, "to_s", cls_imm_to_s, 0);

	rb_define_attr(clsImmOp, IMM_ATTR_VAL, 1, 1);
	rb_define_attr(clsImmOp, IMM_ATTR_VMA, 1, 1);
	rb_define_attr(clsImmOp, IMM_ATTR_SIGNED, 1, 1);
	rb_define_attr(clsImmOp, IMM_ATTR_UNSIGNED, 1, 1);
}

/* ---------------------------------------------------------------------- */
/* Operand Factory */

static enum opdis_op_flag_t op_flags_code( VALUE op ) {
	int i;
	enum opdis_op_flag_t flags = opdis_op_flag_none;
	struct RArray * ary = RARRAY( rb_iv_get(op, IVAR(OP_ATTR_FLAGS)) );

	for ( i=0; i < RARRAY_LEN(ary); i++ ) {
		VALUE val = RARRAY_PTR(ary)[i];
		if (! strcmp(OP_FLAG_R, StringValueCStr(val)) ) {
			flags |= opdis_op_flag_r;
		} else if (! strcmp(OP_FLAG_W, StringValueCStr(val)) ) {
			flags |= opdis_op_flag_w;
		} else if (! strcmp(OP_FLAG_X, StringValueCStr(val)) ) {
			flags |= opdis_op_flag_x;
		} else if (! strcmp(OP_FLAG_SIGNED, StringValueCStr(val)) ) {
			flags |= opdis_op_flag_signed;
		} else if (! strcmp(OP_FLAG_ADDR, StringValueCStr(val)) ) {
			flags |= opdis_op_flag_address;
		} else if (! strcmp(OP_FLAG_IND, StringValueCStr(val)) ) {
			flags |= opdis_op_flag_indirect;
		}
	}

	return flags;
}

static void set_rb_op_flags( VALUE instance, enum opdis_op_flag_t val ) {
	VALUE flags = rb_iv_get(instance, IVAR(OP_ATTR_FLAGS) );
	
	if ( val & opdis_op_flag_r ) {
		rb_ary_push(flags, rb_str_new_cstr(OP_FLAG_R) );
	}
	if ( val & opdis_op_flag_w ) {
		rb_ary_push(flags, rb_str_new_cstr(OP_FLAG_W) );
	}
	if ( val & opdis_op_flag_x ) {
		rb_ary_push(flags, rb_str_new_cstr(OP_FLAG_X) );
	}
	if ( val & opdis_op_flag_signed ) {
		rb_ary_push(flags, rb_str_new_cstr(OP_FLAG_SIGNED) );
	}
	if ( val & opdis_op_flag_address ) {
		rb_ary_push(flags, rb_str_new_cstr(OP_FLAG_ADDR) );
	}
	if ( val & opdis_op_flag_indirect ) {
		rb_ary_push(flags, rb_str_new_cstr(OP_FLAG_IND) );
	}
}

static void op_to_c( VALUE op, opdis_op_t * dest ) {
	VALUE var;

	if ( Qtrue == rb_obj_is_kind_of( op, clsImmOp) ) {
		dest->category = opdis_op_cat_immediate;
		dest->value.immediate.u = imm_to_c( op );

	} else if ( Qtrue == rb_obj_is_kind_of( op, clsRegOp) ) {
		dest->category = opdis_op_cat_register;
		reg_to_c( op, &dest->value.reg );

	} else if ( Qtrue == rb_obj_is_kind_of( op, clsAddrExprOp) ) {
		dest->category = opdis_op_cat_expr;
		addrexpr_to_c( op, &dest->value.expr );

	} else if ( Qtrue == rb_obj_is_kind_of( op, clsAbsAddrOp) ) {
		dest->category = opdis_op_cat_absolute;
		absaddr_to_c( op, &dest->value.abs );
	}

	var = rb_iv_get(op, IVAR(GEN_ATTR_ASCII));
	opdis_op_set_ascii(dest, StringValueCStr(var));

	dest->data_size = (unsigned char) NUM2UINT(rb_iv_get(op, 
							IVAR(OP_ATTR_DATA_SZ)));
	dest->flags = op_flags_code( op );
}

static VALUE op_from_c( opdis_op_t * op ) {
	VALUE args[1] = {Qnil};
	VALUE dest;

	switch (op->category) {
		case opdis_op_cat_register:
			dest = reg_op_from_c(&op->value.reg); break;
		case opdis_op_cat_immediate:
			dest = imm_op_from_c(op); break;
		case opdis_op_cat_absolute:
			dest = absaddr_op_from_c(&op->value.abs); break;
		case opdis_op_cat_expr:
			dest = addrexpr_op_from_c(&op->value.expr); break;
		case opdis_op_cat_unknown:
		default:
			dest = rb_class_new_instance(0, args, clsOp);
	}

	if ( dest == Qnil ) {
		return dest;
	}

	rb_iv_set(dest, IVAR(GEN_ATTR_ASCII), rb_str_new_cstr(op->ascii));
	rb_iv_set(dest, IVAR(OP_ATTR_DATA_SZ), 
		  UINT2NUM((unsigned int) op->data_size));
	set_rb_op_flags( dest, op->flags );

	return dest;
}

/* ---------------------------------------------------------------------- */
/* Instruction Class */

static VALUE cls_insn_init(VALUE instance) {
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


static enum opdis_insn_cat_t insn_category_code( VALUE insn ) {
	VALUE val = rb_iv_get(insn, IVAR(INSN_ATTR_CATEGORY));
	if (! strcmp(INSN_CAT_CFLOW , StringValueCStr(val)) ) {
		return opdis_insn_cat_cflow;
	} else if (! strcmp(INSN_CAT_STACK , StringValueCStr(val)) ) {
		return opdis_insn_cat_stack;
	} else if (! strcmp(INSN_CAT_LOST , StringValueCStr(val)) ) {
		return opdis_insn_cat_lost;
	} else if (! strcmp(INSN_CAT_TEST , StringValueCStr(val)) ) {
		return opdis_insn_cat_test;
	} else if (! strcmp(INSN_CAT_MATH , StringValueCStr(val)) ) {
		return opdis_insn_cat_math;
	} else if (! strcmp(INSN_CAT_BIT , StringValueCStr(val)) ) {
		return opdis_insn_cat_bit;
	} else if (! strcmp(INSN_CAT_IO , StringValueCStr(val)) ) {
		return opdis_insn_cat_io;
	} else if (! strcmp(INSN_CAT_TRAP , StringValueCStr(val)) ) {
		return opdis_insn_cat_trap;
	} else if (! strcmp(INSN_CAT_PRIV , StringValueCStr(val)) ) {
		return opdis_insn_cat_priv;
	} else if (! strcmp(INSN_CAT_NOP , StringValueCStr(val)) ) {
		return opdis_insn_cat_nop;
	}

	return opdis_insn_cat_unknown;
}

static enum opdis_insn_subset_t insn_isa_code( VALUE insn ) {
	VALUE val = rb_iv_get(insn, IVAR(INSN_ATTR_ISA));
	if (! strcmp(INSN_ISA_FPU , StringValueCStr(val)) ) {
		return opdis_insn_subset_fpu;
	} else if (! strcmp(INSN_ISA_GPU , StringValueCStr(val)) ) {
		return opdis_insn_subset_gpu;
	} else if (! strcmp(INSN_ISA_SIMD , StringValueCStr(val)) ) {
		return opdis_insn_subset_simd;
	} else if (! strcmp(INSN_ISA_VM , StringValueCStr(val)) ) {
		return opdis_insn_subset_vm;
	}

	return opdis_insn_subset_gen;
}

static void insn_set_flags( opdis_insn_t * dest, VALUE insn ) {
	int i;
	struct RArray * ary = RARRAY( rb_iv_get(insn, IVAR(INSN_ATTR_FLAGS)) );

	dest->flags.cflow = opdis_cflow_flag_none;

	for ( i=0; i < RARRAY_LEN(ary); i++ ) {
		VALUE val = RARRAY_PTR(ary)[i];
		switch (dest->category) {
			case opdis_insn_cat_cflow:
				if (! strcmp(INSN_FLAG_CALL, 
					     StringValueCStr(val)) ) {
					dest->flags.cflow |= 
						opdis_cflow_flag_call;
				} else if (! strcmp(INSN_FLAG_CALLCC, 
						    StringValueCStr(val)) ) {
					dest->flags.cflow |= 
						opdis_cflow_flag_callcc;
				} else if (! strcmp(INSN_FLAG_JMP, 
						    StringValueCStr(val)) ) {
					dest->flags.cflow |= 
						opdis_cflow_flag_jmp;
				} else if (! strcmp(INSN_FLAG_JMPCC, 
						    StringValueCStr(val)) ) {
					dest->flags.cflow |= 
						opdis_cflow_flag_jmpcc;
				} else if (! strcmp(INSN_FLAG_RET, 
						    StringValueCStr(val)) ) {
					dest->flags.cflow |= 
						opdis_cflow_flag_ret;
				}
				break;
			case opdis_insn_cat_stack:
				if (! strcmp(INSN_FLAG_PUSH, 
					     StringValueCStr(val)) ) {
					dest->flags.stack |= 
						opdis_stack_flag_push;
				} else if (! strcmp(INSN_FLAG_POP, 
						    StringValueCStr(val)) ) {
					dest->flags.stack |= 
						opdis_stack_flag_pop;
				} else if (! strcmp(INSN_FLAG_FRAME, 
						    StringValueCStr(val)) ) {
					dest->flags.stack |= 
						opdis_stack_flag_frame;
				} else if (! strcmp(INSN_FLAG_UNFRAME, 
						    StringValueCStr(val)) ) {
					dest->flags.stack |= 
						opdis_stack_flag_unframe;
				}
				break;
			case opdis_insn_cat_bit:
				if (! strcmp(INSN_FLAG_AND, 
					     StringValueCStr(val)) ) {
					dest->flags.bit |= opdis_bit_flag_and;
				} else if (! strcmp(INSN_FLAG_OR, 
						    StringValueCStr(val)) ) {
					dest->flags.bit |= opdis_bit_flag_or;
				} else if (! strcmp(INSN_FLAG_XOR, 
						    StringValueCStr(val)) ) {
					dest->flags.bit |= opdis_bit_flag_xor;
				} else if (! strcmp(INSN_FLAG_NOT, 
						    StringValueCStr(val)) ) {
					dest->flags.bit |= opdis_bit_flag_not;
				} else if (! strcmp(INSN_FLAG_LSL, 
						    StringValueCStr(val)) ) {
					dest->flags.bit |= opdis_bit_flag_lsl;
				} else if (! strcmp(INSN_FLAG_LSR, 
						    StringValueCStr(val)) ) {
					dest->flags.bit |= opdis_bit_flag_lsr;
				} else if (! strcmp(INSN_FLAG_ASL, 
						    StringValueCStr(val)) ) {
					dest->flags.bit |= opdis_bit_flag_asl;
				} else if (! strcmp(INSN_FLAG_ASR, 
						    StringValueCStr(val)) ) {
					dest->flags.bit |= opdis_bit_flag_asr;
				} else if (! strcmp(INSN_FLAG_ROL, 
						    StringValueCStr(val)) ) {
					dest->flags.bit |= opdis_bit_flag_rol;
				} else if (! strcmp(INSN_FLAG_ROR, 
						    StringValueCStr(val)) ) {
					dest->flags.bit |= opdis_bit_flag_ror;
				} else if (! strcmp(INSN_FLAG_RCL, 
						    StringValueCStr(val)) ) {
					dest->flags.bit |= opdis_bit_flag_rcl;
				} else if (! strcmp(INSN_FLAG_RCR, 
						    StringValueCStr(val)) ) {
					dest->flags.bit |= opdis_bit_flag_rcr;
				}
				break;
			case opdis_insn_cat_io:
				if (! strcmp(INSN_FLAG_OUT, 
					     StringValueCStr(val)) ) {
					dest->flags.io |= opdis_io_flag_in;
				} else if (! strcmp(INSN_FLAG_IN, 
						    StringValueCStr(val)) ) {
					dest->flags.io |= opdis_io_flag_out;
				}
				break;
			default: break;
		}
	}
}

static enum opdis_insn_decode_t insn_status_code( VALUE instance ) {
	int i;
	enum opdis_insn_decode_t status = opdis_decode_invalid;
	struct RArray * ary = RARRAY( rb_iv_get(instance, 
						IVAR(INSN_ATTR_STATUS)) );

	for ( i=0; i < RARRAY_LEN(ary); i++ ) {
		VALUE val = RARRAY_PTR(ary)[i];
		if (! strcmp(INSN_DECODE_BASIC, StringValueCStr(val)) ) {
			status |= opdis_decode_basic;
		} else if (! strcmp(INSN_DECODE_MNEM, StringValueCStr(val)) ) {
			status |= opdis_decode_mnem;
		} else if (! strcmp(INSN_DECODE_OPS, StringValueCStr(val)) ) {
			status |= opdis_decode_ops;
		} else if (! strcmp(INSN_DECODE_MNEMFLG, 
				    StringValueCStr(val)) ) {
			status |= opdis_decode_mnem_flags;
		} else if (! strcmp(INSN_DECODE_OPFLG, 
				    StringValueCStr(val)) ) {
			status |= opdis_decode_op_flags;
		}
	}

	return status;
}

static void set_insn_status( VALUE instance, enum opdis_insn_decode_t val ) {
	VALUE status = rb_iv_get(instance, IVAR(INSN_ATTR_STATUS) );

	if ( val == opdis_decode_invalid ) {
		rb_ary_push(status, rb_str_new_cstr( INSN_DECODE_INVALID) );
		return;
	}

	if ( val & opdis_decode_basic ) {
		rb_ary_push(status, rb_str_new_cstr( INSN_DECODE_BASIC) );
	}
	if ( val & opdis_decode_mnem ) {
		rb_ary_push(status, rb_str_new_cstr( INSN_DECODE_MNEM) );
	}
	if ( val & opdis_decode_ops ) {
		rb_ary_push(status, rb_str_new_cstr( INSN_DECODE_OPS) );
	}
	if ( val & opdis_decode_mnem_flags ) {
		rb_ary_push(status, rb_str_new_cstr( INSN_DECODE_MNEMFLG) );
	}
	if ( val & opdis_decode_op_flags ) {
		rb_ary_push(status, rb_str_new_cstr( INSN_DECODE_OPFLG) );
	}
}

static void fill_ruby_insn( const opdis_insn_t * insn, VALUE dest ) {
	unsigned int i;
	char buf[128];
	VALUE ops = rb_iv_get(dest, IVAR(INSN_ATTR_OPERANDS) );

	set_insn_status( dest, insn->status );

	rb_iv_set(dest, IVAR(GEN_ATTR_ASCII), rb_str_new_cstr(insn->ascii));

	rb_iv_set(dest, IVAR(INSN_ATTR_OFFSET), OFFT2NUM(insn->offset));
	rb_iv_set(dest, IVAR(INSN_ATTR_VMA), OFFT2NUM(insn->vma));
	rb_iv_set(dest, IVAR(INSN_ATTR_SIZE), UINT2NUM(insn->size));
	rb_iv_set(dest, IVAR(INSN_ATTR_BYTES), 
		  rb_str_new((const char *) insn->bytes, insn->size ));

	rb_iv_set(dest, IVAR(INSN_ATTR_PREFIXES), 
		  rb_str_split(rb_str_new_cstr(insn->prefixes), " "));
	rb_iv_set(dest, IVAR(INSN_ATTR_MNEMONIC), 
		  rb_str_new_cstr(insn->mnemonic));
	rb_iv_set(dest, IVAR(INSN_ATTR_COMMENT), 
		  rb_str_new_cstr(insn->comment));

	buf[0] = '\0';
	opdis_insn_cat_str( insn, buf, 128 );
	rb_iv_set(dest, IVAR(INSN_ATTR_CATEGORY), rb_str_new_cstr(buf));

	buf[0] = '\0';
	opdis_insn_isa_str( insn, buf, 128 );
	rb_iv_set(dest, IVAR(INSN_ATTR_ISA), rb_str_new_cstr(buf));

	buf[0] = '\0';
	opdis_insn_flags_str( insn, buf, 128, "|" );
	rb_iv_set(dest, IVAR(INSN_ATTR_FLAGS), 
		  rb_str_split(rb_str_new_cstr(buf), "|"));

	rb_ary_clear(ops);
	for ( i=0; i < insn->num_operands; i++ ) {
		set_attr_if_alias( dest, IVAR(INSN_ATTR_TGT_IDX), i,
				   insn->operands[i], insn->target );
		set_attr_if_alias( dest, IVAR(INSN_ATTR_DEST_IDX), i,
				   insn->operands[i], insn->dest );
		set_attr_if_alias( dest, IVAR(INSN_ATTR_SRC_IDX), i,
				   insn->operands[i], insn->src );
				   
		rb_ary_push( ops, op_from_c(insn->operands[i]) );
	}
}

static VALUE insn_from_c( const opdis_insn_t * insn ) {
	VALUE args[1] = {Qnil};
	VALUE var = rb_class_new_instance(0, args, clsInsn);
	if ( var != Qnil ) {
		fill_ruby_insn( insn, var );
	}
	return var;
}

static void insn_to_c( VALUE insn, opdis_insn_t * dest ) {
	int i;
	VALUE var;
	struct RArray * ary;

	dest->status = insn_status_code(insn);

	var = rb_iv_get(insn, IVAR(GEN_ATTR_ASCII));
	opdis_insn_set_ascii(dest, StringValueCStr(var));

	var = rb_iv_get(insn, IVAR(INSN_ATTR_OFFSET));
	dest->offset = (opdis_off_t) NUM2ULL(var);

	var = rb_iv_get(insn, IVAR(INSN_ATTR_VMA));
	dest->vma = (opdis_vma_t) NUM2ULL(var);

	var = rb_iv_get(insn, IVAR(INSN_ATTR_SIZE));
	dest->size = (opdis_off_t) NUM2ULL(var);

	var = rb_iv_get(insn, IVAR(INSN_ATTR_BYTES));
	if (! dest->bytes ) {
		dest->bytes = calloc( 1, dest->size );
	}
	memcpy( dest->bytes, RSTRING_PTR(var), dest->size );

	ary = RARRAY( rb_iv_get(insn, IVAR(INSN_ATTR_PREFIXES)) );
	for ( i=0; i < RARRAY_LEN(ary); i++ ) {
		VALUE val = RARRAY_PTR(ary)[i];
		opdis_insn_add_prefix(dest, StringValueCStr(val));
	}

	var = rb_iv_get(insn, IVAR(INSN_ATTR_MNEMONIC));
	opdis_insn_set_mnemonic(dest, StringValueCStr(var));

	var = rb_iv_get(insn, IVAR(INSN_ATTR_COMMENT));
	opdis_insn_add_comment(dest, StringValueCStr(var));

	dest->category = insn_category_code(insn);
	dest->isa = insn_isa_code(insn);

	insn_set_flags(dest, insn);

	ary = RARRAY( rb_iv_get(insn, IVAR(INSN_ATTR_OPERANDS)) );
	for ( i=0; i < RARRAY_LEN(ary); i++ ) {
		VALUE val = RARRAY_PTR(ary)[i];
		opdis_op_t * op = opdis_insn_next_avail_op(dest);
		if (! op ) {
			op = opdis_op_alloc();
			opdis_insn_add_operand(dest, op);
		}

		op_to_c( val, op );

		if ( i == NUM2INT(rb_iv_get(insn, IVAR(INSN_ATTR_TGT_IDX))) ) {
			dest->target = op;
		}
		if ( i == NUM2INT(rb_iv_get(insn, IVAR(INSN_ATTR_DEST_IDX))) ) {
			dest->dest = op;
		}
		if ( i == NUM2INT(rb_iv_get(insn, IVAR(INSN_ATTR_SRC_IDX))) ) {
			dest->src = op;
		}
	}
}

static VALUE get_aliased_operand( VALUE instance, const char * alias ) {
	VALUE idx = rb_iv_get(instance, alias);
	VALUE ops = rb_iv_get(instance, IVAR(INSN_ATTR_OPERANDS) );

	return (idx == Qnil) ? idx : rb_ary_entry(ops, NUM2LONG(idx));
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

static int is_cflow_insn( VALUE instance ) {
	VALUE cat = rb_iv_get(instance, IVAR(INSN_ATTR_CATEGORY) );
	return (cat != Qnil && ! strcmp(INSN_CAT_CFLOW, StringValueCStr(cat))) ?
		1 : 0;
}

static int insn_has_flag( VALUE instance, const char * flg ) {
	VALUE flags = rb_iv_get(instance, IVAR(INSN_ATTR_FLAGS) );
	return (flags != Qnil && rb_ary_includes(flags, rb_str_new_cstr(flg))) ?
		1 : 0;
}

static VALUE cls_insn_branch( VALUE instance ) {
	if ( is_cflow_insn(instance) && 
	     (insn_has_flag(instance, INSN_FLAG_CALL) ||
	      insn_has_flag(instance, INSN_FLAG_CALLCC) ||
	      insn_has_flag(instance, INSN_FLAG_JMPCC) ||
	      insn_has_flag(instance, INSN_FLAG_JMP)) ) {
		return Qtrue;
	}

	return Qfalse;
}

static VALUE cls_insn_fallthrough( VALUE instance ) {
	if ( is_cflow_insn(instance) && 
	     (insn_has_flag(instance, INSN_FLAG_RET) ||
	      insn_has_flag(instance, INSN_FLAG_JMP)) ) {
		return Qfalse;
	}

	return Qtrue;
}

static void define_insn_constants() {
	rb_define_const(clsInsn, INSN_DECODE_INVALID_NAME,
			rb_str_new_cstr(INSN_DECODE_INVALID));
	rb_define_const(clsInsn, INSN_DECODE_BASIC_NAME,
			rb_str_new_cstr(INSN_DECODE_BASIC));
	rb_define_const(clsInsn, INSN_DECODE_MNEM_NAME,
			rb_str_new_cstr(INSN_DECODE_MNEM));
	rb_define_const(clsInsn, INSN_DECODE_OPS_NAME,
			rb_str_new_cstr(INSN_DECODE_OPS));
	rb_define_const(clsInsn, INSN_DECODE_MNEMFLG_NAME,
			rb_str_new_cstr(INSN_DECODE_MNEMFLG));
	rb_define_const(clsInsn, INSN_DECODE_OPFLG_NAME,
			rb_str_new_cstr(INSN_DECODE_OPFLG));

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
	rb_define_method(clsInsn, "initialize", cls_insn_init, 0);
	rb_define_method(clsInsn, "to_s", cls_generic_to_s, 0);
	rb_define_method(clsInsn, "branch?", cls_insn_branch, 0);
	rb_define_method(clsInsn, "fallthrough?", cls_insn_fallthrough, 0);

	define_insn_attributes();
	define_insn_constants();
}

/* ---------------------------------------------------------------------- */
/* Public API */

void Opdis_initModel( VALUE modOpdis ) {
	symToSym = rb_intern("to_sym");
	symToS = rb_intern("to_s");

	init_insn_class(modOpdis);
        init_op_class(modOpdis);
        init_imm_class(modOpdis);
        init_absaddr_class(modOpdis);
        init_addrexpr_class(modOpdis);
        init_reg_class(modOpdis);
}

VALUE Opdis_insnFromC( const opdis_insn_t * insn ) {
	VALUE var = Qnil; 
	if ( insn ) {
		var = insn_from_c(insn);
	}
	return var;
}

int Opdis_insnFillFromC( const opdis_insn_t * insn, VALUE dest ) {
	if (insn == NULL || dest == Qnil) {
		return 0;
	}
					      
	fill_ruby_insn(insn, dest);
	return 1;
}

int Opdis_insnToC( VALUE insn, opdis_insn_t * c_insn ) {
	if (insn == Qnil || c_insn == NULL) {
		return 0;
	}

	insn_to_c( insn, c_insn );

	return 1;
}
